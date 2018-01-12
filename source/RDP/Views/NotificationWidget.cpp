//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A notification overlay that can be used to display messages to
///         the user on top of the RDP window contents.
//=============================================================================

#include "NotificationWidget.h"
#include "ui_NotificationWidget.h"
#include "../Util/RDPUtil.h"
#include "../../Common/DriverToolsDefinitions.h"
#include <ScalingManager.h>
#include <QButtonGroup>
#include <QPushButton>

/// An array of strings used for the button choices.
/// Must match NotificationWidget::Button declaration.
static const QString kButtonStrings[] =
{
    QString("Unset"),
    QString("OK"),
    QString("Yes"),
    QString("No"),
    QString("Browse..."),
    QString("Revert"),
    QString("Cancel"),
    QString("Max"),
};

//-----------------------------------------------------------------------------
/// Constructor for the ContainerWidget overlay.
/// \param pParent The parent widget for the container.
//-----------------------------------------------------------------------------
ContainerWidget::ContainerWidget(QWidget* pParent) :
    QWidget(pParent),
    m_pBackgroundWidget(nullptr),
    m_pOverlayWidget(nullptr)
{
    this->setObjectName("OverlayContainerWidget");
    this->setLayout(pParent->layout());
}

//-----------------------------------------------------------------------------
/// Set the size of the container widget.
/// \param pObject The object used to determine what size the container should change to.
//-----------------------------------------------------------------------------
void ContainerWidget::SetSize(QObject* pObject)
{
    if (pObject->isWidgetType())
    {
        // Make the contents match the container's size.
        static_cast<QWidget*>(pObject)->setGeometry(rect());
    }
}

//-----------------------------------------------------------------------------
/// Set the background widget to be displayed within the container.
/// \param pBackgroundWidget The background widget to be displayed within the container.
//-----------------------------------------------------------------------------
void ContainerWidget::SetBackgroundWidget(QWidget* pBackgroundWidget)
{
    m_pBackgroundWidget = pBackgroundWidget;
    m_pBackgroundWidget->setParent(this);
}

//-----------------------------------------------------------------------------
/// Set the widget to be overlaid on top of the background widget in the container.
/// \param pOverlay The widget to be used as an overlay.
//-----------------------------------------------------------------------------
void ContainerWidget::SetOverlayWidget(QWidget* pOverlay)
{
    m_pOverlayWidget = pOverlay;
}

//-----------------------------------------------------------------------------
/// Show the overlay widget on top of the background widget.
//-----------------------------------------------------------------------------
void ContainerWidget::ShowOverlay()
{
    SetEnableBlur(true);
    m_pOverlayWidget->show();
    m_pBackgroundWidget->setDisabled(true);
}

//-----------------------------------------------------------------------------
/// Hide the overlay widget, and return control to the background widget.
//-----------------------------------------------------------------------------
void ContainerWidget::HideOverlay()
{
    m_pOverlayWidget->hide();
    m_pBackgroundWidget->setDisabled(false);
    SetEnableBlur(false);
}

//-----------------------------------------------------------------------------
/// Callback that gets invoked when a resize event is emitted for the container.
/// \param pEvent The resize event argument.
//-----------------------------------------------------------------------------
void ContainerWidget::resizeEvent(QResizeEvent* pEvent)
{
    Q_UNUSED(pEvent);

    for (auto pChild : children())
    {
        SetSize(pChild);
    }
}

//-----------------------------------------------------------------------------
/// Toggle a blur filter to be used when rendering the background widget while the overlay is active.
/// \param enable The flag that determines whether or not the background widget will be blurred.
//-----------------------------------------------------------------------------
void ContainerWidget::SetEnableBlur(bool enable)
{
    if (enable)
    {
        QGraphicsBlurEffect* pBackgroundBlur = new QGraphicsBlurEffect();
        pBackgroundBlur->setBlurRadius(15.0f);
        m_pBackgroundWidget->setGraphicsEffect(pBackgroundBlur);
    }
    else
    {
        // The blur effect doesn't need to be deleted manually.
        // Qt will delete it under the hood before using the null effect.
        m_pBackgroundWidget->setGraphicsEffect(nullptr);
    }
}

//-----------------------------------------------------------------------------
/// Constructor for the NotificationWidget overlay.
/// \param pParent The parent widget for the overlay.
//-----------------------------------------------------------------------------
NotificationWidget::NotificationWidget(ContainerWidget* pParent) :
    QWidget(pParent),
    ui(new Ui::NotificationWidget),
    m_pParentContainer(pParent),
    m_result(Unset),
    m_showDoNotAskAgain(false)
{
    ui->setupUi(this);

    NewParentSet();

    // Add each of the buttons to a button group.
    m_pOptionsGroup = new QButtonGroup(this);
}

//-----------------------------------------------------------------------------
/// Destructor for the NotificationWidget overlay.
//-----------------------------------------------------------------------------
NotificationWidget::~NotificationWidget()
{
    DestroyButtons();
    SAFE_DELETE(m_pOptionsGroup);
    SAFE_DELETE(ui);
}

//-----------------------------------------------------------------------------
/// Handle cases where a new parent has been set for the notification overlay.
//-----------------------------------------------------------------------------
void NotificationWidget::NewParentSet()
{
    if (!parent())
    {
        return;
    }
}

//-----------------------------------------------------------------------------
/// Set the title text within the notification overlay.
/// \param titleText The text to insert into the title label.
//-----------------------------------------------------------------------------
void NotificationWidget::SetTitle(const QString& titleText)
{
    ui->titleLabel->setText(titleText);
}

//-----------------------------------------------------------------------------
/// Set the message text within the notification overlay.
/// \param text The text to insert into the notification text label.
//-----------------------------------------------------------------------------
void NotificationWidget::SetText(const QString& text)
{
    ui->notificationTextLabel->setText(text);
}

//-----------------------------------------------------------------------------
/// Set the buttons to display within the notification overlay.
/// \param buttons A bitwise'd field of NotificationWidget::Button.
/// \param defaultButtonBitIndex An enum button bit index for the default button
/// (zero indicates use the highest bit value from the buttons parameter).
//-----------------------------------------------------------------------------
void NotificationWidget::SetButtons(uint buttons, uint defaultButtonBitIndex)
{
    m_buttons = buttons;
    m_defaultButtonBitIndex = defaultButtonBitIndex;
    InitializeButtons();
}

//-----------------------------------------------------------------------------
/// Toggle the visibility of the "Don't ask me again" checkbox.
/// \param showCheckbox If true, the checkbox will be visible. If false, it will be hidden.
//-----------------------------------------------------------------------------
void NotificationWidget::ShowDoNotAsk(bool showCheckbox)
{
    if (showCheckbox)
    {
        ui->dontAskAgainButton->show();
    }
    else
    {
        ui->dontAskAgainButton->hide();
    }
}

//-----------------------------------------------------------------------------
/// Is the "Do not ask me again" checkbox checked?
/// \returns True if the "Do not ask me again" checkbox is checked, and false if it's not.
//-----------------------------------------------------------------------------
bool NotificationWidget::GetIsDoNotAskChecked() const
{
    return ui->dontAskAgainButton->isChecked();
}

//-----------------------------------------------------------------------------
/// Handler invoked when the Notification overlay is shown.
/// \param pEvent The info for the show event.
//-----------------------------------------------------------------------------
void NotificationWidget::showEvent(QShowEvent* pEvent)
{
    Q_UNUSED(pEvent);

    // Reset the button result state before showing the overlay (potentially again).
    m_result = Unset;
}

//-----------------------------------------------------------------------------
/// Initialize all of the buttons displayed within the overlay.
//-----------------------------------------------------------------------------
void NotificationWidget::InitializeButtons()
{
    // Clear out the buttons from previous popups.
    DestroyButtons();

    ScalingManager& scalingManager = ScalingManager::Get();

    // Step through each button option and check if the button is required.
    int maxSteps = log2(static_cast<int>(Button::Button_Max) - 1);
    QPushButton* pButton = nullptr;
    QPushButton* pDefaultButton = nullptr;

    for (int buttonBitIndex = 0; buttonBitIndex <= maxSteps; ++buttonBitIndex)
    {
        uint buttonBit = (1 << buttonBitIndex);
        if (static_cast<uint>(m_buttons & buttonBit) == buttonBit)
        {
            const QString& buttonText = kButtonStrings[buttonBitIndex];

            pButton = new QPushButton(buttonText, this);
            pButton->setAutoDefault(true);
            if (m_defaultButtonBitIndex & buttonBit)
            {
                pDefaultButton = pButton;
            }

            scalingManager.RegisterObject(pButton);

            ui->buttonHost->addWidget(pButton);
            m_pOptionsGroup->addButton(pButton, buttonBit);
            m_buttonOptions.push_back(pButton);
        }
    }

    if (pButton != nullptr)
    {
        if (pDefaultButton == nullptr)
        {
            // use the last button added if a default has not been defined.
            pDefaultButton = pButton;
        }
        pDefaultButton->setFocus();
    }

    // Connect the new buttons to the button handler using the overloaded signal syntax.
    connect(m_pOptionsGroup, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked),
        this, &NotificationWidget::OnButtonClicked);
}

//-----------------------------------------------------------------------------
/// Clean up all option buttons added to the overlay.
//-----------------------------------------------------------------------------
void NotificationWidget::DestroyButtons()
{
    for (int buttonIndex = 0; buttonIndex < m_buttonOptions.size(); ++buttonIndex)
    {
        QPushButton* pOption = m_buttonOptions[buttonIndex];

        m_pOptionsGroup->removeButton(pOption);
        ui->buttonHost->removeWidget(pOption);

        // Destroy each previously created button.
        SAFE_DELETE(pOption);
    }

    m_buttonOptions.clear();
}

//-----------------------------------------------------------------------------
/// Handler invoked when the notification overlay's buttons are clicked.
/// \param buttonId The ID of the button that was clicked.
//-----------------------------------------------------------------------------
void NotificationWidget::OnButtonClicked(int buttonId)
{
    // The ID of the clicked button is stored as the result.
    m_result = static_cast<Button>(buttonId);

    this->hide();
}
