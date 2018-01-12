//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  The configuration window is where the user can alter RDS configuration.
//=============================================================================

#include <QEvent>
#include <QKeyEvent>

#include "ConfigurationWindow.h"
#include "ui_ConfigurationWindow.h"
#include "../Models/ConfigurationWindowModel.h"
#include "../../Common/DriverToolsDefinitions.h"
#include "../../Common/RestoreCursorPosition.h"
#include "../../Common/ToolUtil.h"

static const QString s_REMOTE_LISTEN_PORT_LABEL("Remote listen port:");

//-----------------------------------------------------------------------------
/// Constructor for the Configuration Window class.
/// \param pParent The parent widget for this control.
//-----------------------------------------------------------------------------
ConfigurationWindow::ConfigurationWindow(QDialog* pParent) :
    QDialog(pParent),
    ui(new Ui::ConfigurationWindow),
    m_pWindowIcon(nullptr)
{
    ui->setupUi(this);

    // Set the window's flags
    setWindowFlags(Qt::WindowCloseButtonHint |
                   Qt::WindowTitleHint |
                   Qt::WindowSystemMenuHint);

    // Set white background for this pane
    ToolUtil::SetWidgetBackgroundColor(this, Qt::white);

    // Allow the user to change the listen port initially.
    EnableChangingPort(true);

    ui->versionLabelData->setText(DEV_DRIVER_TOOLS_VERSION_STRING);
    ui->buildLabelData->setText(QString::number(DEV_DRIVER_TOOLS_BUILD_NUMBER));
    ui->buildDateLabelData->setText(DEV_DRIVER_TOOLS_BUILD_DATE_STRING);

    m_pWindowIcon = new QIcon(":/images/RDS_Icon.png");
    if (m_pWindowIcon != nullptr)
    {
        setWindowIcon(*m_pWindowIcon);
    }

    m_pConfigurationModel = new ConfigurationWindowModel();
    m_pConfigurationModel->InitializeModel(ui->listenPortTextbox, RDS_CONFIGURATION_CONTROLS_LISTEN_PORT, "text");
    m_pConfigurationModel->InitializeDefaults();

    connect(ui->restoreDefaultSettingButton, &QPushButton::clicked, this, &ConfigurationWindow::OnDefaultListenPortClicked);
    connect(ui->listenPortTextbox, &QLineEdit::textEdited, this, &ConfigurationWindow::OnListenPortTextChanged);
    ui->listenPortTextbox->setValidator(new QIntValidator(0, gs_MAX_LISTEN_PORT));
#ifdef Q_OS_WIN
    ui->listenPortHeader->setText(s_REMOTE_LISTEN_PORT_LABEL);
#endif
    qApp->installEventFilter(this);
}

//-----------------------------------------------------------------------------
/// Destructor for the Configuration Window.
//-----------------------------------------------------------------------------
ConfigurationWindow::~ConfigurationWindow()
{
    SAFE_DELETE(ui);
}

//-----------------------------------------------------------------------------
/// Toggle the user's ability to alter the listen port.
/// \param enabled A flag that indicates if the user is allowed to alter the listen port.
//-----------------------------------------------------------------------------
void ConfigurationWindow::EnableChangingPort(bool enabled)
{
    // Hide the port change warning label.
    ui->portWarningLabel->setVisible(!enabled);
    ui->listenPortTextbox->setEnabled(enabled);
    ui->restoreDefaultSettingButton->setEnabled(enabled);
}

//-----------------------------------------------------------------------------
/// Handler invoked when the main RDS listener port has been updated.
/// \param port The port that RDS will use to listen for incoming connections.
//-----------------------------------------------------------------------------
void ConfigurationWindow::OnListenPortUpdated(unsigned int port)
{
    const QString portString = QString::number(port);
    ui->listenPortTextbox->setText(portString);
}

//-----------------------------------------------------------------------------
/// Handler invoked when the user alters the text in the listen port textbox.
/// \param portText The updated port number string.
//-----------------------------------------------------------------------------
void ConfigurationWindow::OnListenPortTextChanged(const QString& portText)
{
    RestoreCursorPosition cacheCursorPosition(ui->listenPortTextbox);
    m_pConfigurationModel->Update(RDS_CONFIGURATION_CONTROLS_LISTEN_PORT, portText);

    emit ListenEndpointUpdated();
}

//-----------------------------------------------------------------------------
/// Filter events sent to this widget and child widgets.
/// \param pTarget Pointer to the destination object of the event.
/// \param pEvent Pointer to the event object.
//-----------------------------------------------------------------------------
bool ConfigurationWindow::eventFilter(QObject* pTarget, QEvent* pEvent)
{
    bool eventHandled = false;
    if (pEvent != nullptr)
    {
        if (pEvent->type() == QEvent::KeyPress)
        {
            QKeyEvent* pKeyEvent = static_cast<QKeyEvent*>(pEvent);
            if ((pKeyEvent != nullptr) && (pKeyEvent->key() == Qt::Key_Escape))
            {
                eventHandled = true;
                close();
            }
        }
    }

    if (eventHandled == false)
    {
        eventHandled = QDialog::eventFilter(pTarget, pEvent);
    }

    return eventHandled;
}

/// Handler invoked when the user presses the button to reset the listen port textbox.
//-----------------------------------------------------------------------------
void ConfigurationWindow::OnDefaultListenPortClicked()
{
    const QString portString = QString::number(gs_DEFAULT_CONNECTION_PORT);
    ui->listenPortTextbox->setText(portString);
    OnListenPortTextChanged(portString);
}
