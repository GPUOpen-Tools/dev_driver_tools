//=============================================================================
/// Copyright (c) 2016-2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Widget to represent an editable settings item in the DriverSettingsView
//=============================================================================

#include "DriverSettingItemWidget.h"

#include <QVBoxLayout>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include <float.h>
#include <QtGlobal>

#include "../Util/RDPUtil.h"
#include "../../DevDriverComponents/inc/protocols/settingsClient.h"

using namespace DevDriver::SettingsProtocol;

// Stylesheet for the show full description button
const static QString s_SHOW_DESCRIPTION_CHECKBOX_STYLESHEET(
    "QCheckBox::indicator { width: 12px; height: 12px; }"
    "QCheckBox::indicator:checked { image: url(:/images/PullDownOn_Gray.svg); }"
    "QCheckBox::indicator:unchecked { image: url(:/images/PullDownOff_Gray.svg); }"
    "QCheckBox::indicator:checked:hover { image: url(:/images/PullDownOn_Black.svg); }"
    "QCheckBox::indicator:unchecked:hover { image: url(:/images/PullDownOff_Black.svg); }"
);

// Stylesheet for the restore default setting button
const static QString s_RESTORE_DEFAULT_SETTING_BUTTON_STYLESHEET(
    "QPushButton { min-width: 20px; padding: 0px; }"
);

const static QString s_RESTORE_DEFAULT_SETTING_TOOLTIP("Restore default value");
const static QString s_RESTORE_DEFAULT_SETTING_ICON(":/images/ResetIcon.svg");
const static int s_EDIT_WIDGET_HEIGHT = 20;
const static QSize s_RESTORE_DEFAULT_SETTING_BUTTON_SIZE = QSize(s_EDIT_WIDGET_HEIGHT, s_EDIT_WIDGET_HEIGHT);

//-----------------------------------------------------------------------------
/// Constructor.
/// \param categoryName The category name of the setting for this widget.
/// \param setting The driver setting structure for this widget.
//-----------------------------------------------------------------------------
DriverSettingItemWidget::DriverSettingItemWidget(const QString& categoryName, const DevDriver::SettingsProtocol::Setting& setting, QWidget* pParent) :
    QWidget(pParent),
    m_setting(setting),
    m_categoryName(categoryName)
{
    // Prepare description text
    FixNewlineCharacters(m_setting.description, m_descriptionTextFull);
    m_descriptionTextFirstLine = QString(m_setting.description).split("\\r").at(0);

    // Create widgets
    m_pTitleLabel = new QLabel(m_setting.name, this);
    m_pDescriptionLabel = new QLabel(m_descriptionTextFirstLine, this);
    m_pShowFullDescriptionButton = new QCheckBox("", this);
    m_pRestoreDefaultSettingButton = new QPushButton(QIcon(s_RESTORE_DEFAULT_SETTING_ICON), "", this);
    m_pEditWidget = CreateEditWidget(); // Edit widget type depends on setting type, so use a helper function

    // Title label setup
    QFont font;
    font.setBold(true);
    font.setPointSize(10);
    m_pTitleLabel->setFont(font);

    // Edit widget setup
    m_pEditWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_pShowFullDescriptionButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);
    m_pEditWidget->setMaximumHeight(s_EDIT_WIDGET_HEIGHT);
    m_pEditWidget->setMinimumHeight(s_EDIT_WIDGET_HEIGHT);

    // Description button setup
    m_pShowFullDescriptionButton->setStyleSheet(s_SHOW_DESCRIPTION_CHECKBOX_STYLESHEET);

    // If the first line is the full description, disable the description show button
    if (m_descriptionTextFull.compare(m_descriptionTextFirstLine) == 0)
    {
        m_pShowFullDescriptionButton->hide();
    }

    // Restore to default setting button setup
    m_pRestoreDefaultSettingButton->setMaximumSize(s_RESTORE_DEFAULT_SETTING_BUTTON_SIZE);
    m_pRestoreDefaultSettingButton->setStyleSheet(s_RESTORE_DEFAULT_SETTING_BUTTON_STYLESHEET);
    m_pRestoreDefaultSettingButton->setToolTip(s_RESTORE_DEFAULT_SETTING_TOOLTIP);
    m_pRestoreDefaultSettingButton->hide();

    // Signal/slot connection
    connect(m_pRestoreDefaultSettingButton, &QPushButton::clicked, this, &DriverSettingItemWidget::OnRestoreDefaultButtonPressed);
    connect(m_pShowFullDescriptionButton, &QCheckBox::stateChanged, this, &DriverSettingItemWidget::OnFullDescriptionButtonPressed);

    // Layout items
    QVBoxLayout* pLayout = new QVBoxLayout(this);
    pLayout->setContentsMargins(0, 0, 0, 10);
    pLayout->setSpacing(5);

    QHBoxLayout* pDescLayout = new QHBoxLayout();
    pDescLayout->setContentsMargins(0, 0, 0, 0);
    pDescLayout->setSpacing(6);
    pDescLayout->addWidget(m_pDescriptionLabel, 0, Qt::AlignLeft);
    pDescLayout->addWidget(m_pShowFullDescriptionButton, 1, Qt::AlignLeft | Qt::AlignTop);

    QHBoxLayout* pEditLayout = new QHBoxLayout();
    pEditLayout->setContentsMargins(0, 0, 0, 0);
    pEditLayout->addWidget(m_pEditWidget);
    pEditLayout->addWidget(m_pRestoreDefaultSettingButton);
    pEditLayout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

    pLayout->addWidget(m_pTitleLabel, 0, Qt::AlignLeft);
    pLayout->addLayout(pDescLayout);
    pLayout->addLayout(pEditLayout);

    // Make view reflect initial data
    SetViewFromModel();
}

//-----------------------------------------------------------------------------
/// Destructor.
//-----------------------------------------------------------------------------
DriverSettingItemWidget::~DriverSettingItemWidget()
{
    delete m_pTitleLabel;
    delete m_pDescriptionLabel;
    delete m_pEditWidget;
    delete m_pShowFullDescriptionButton;
    delete m_pRestoreDefaultSettingButton;
}

//-----------------------------------------------------------------------------
/// Setting edited SLOT - Used to handle what happens when the setting is edited.
//-----------------------------------------------------------------------------
void DriverSettingItemWidget::OnSettingEdited()
{
    // Update data to reflect user input
    SetModelFromView();

    // Indicate this widget's setting has changed
    emit SettingChanged(m_categoryName, m_setting);
}

//-----------------------------------------------------------------------------
/// Handler invoked when the show full description arrow is toggled.
/// \param state The checkbox state.
//-----------------------------------------------------------------------------
void DriverSettingItemWidget::OnFullDescriptionButtonPressed(int state)
{
    // Set description text based on state
    if (state == STATE_HIDE_DESCRIPTION)
    {
        m_pDescriptionLabel->setText(m_descriptionTextFirstLine);
    }
    else
    {
        m_pDescriptionLabel->setText(m_descriptionTextFull);
    }
}

//-----------------------------------------------------------------------------
/// Handler invoked when the restore to default button is pressed.
//-----------------------------------------------------------------------------
void DriverSettingItemWidget::OnRestoreDefaultButtonPressed()
{
    // Set default value
    m_setting.value = m_setting.defaultValue;

    // Update view to reflect data changes
    SetViewFromModel();

    // Indicate this widget's setting has changed
    emit SettingChanged(m_categoryName, m_setting);
}

//-----------------------------------------------------------------------------
/// Qt enter event handler - Determines what to do when the mouse enters this
/// widget.
/// \param pEvent The qt event being handled.
//-----------------------------------------------------------------------------
void DriverSettingItemWidget::enterEvent(QEvent* pEvent)
{
    m_pRestoreDefaultSettingButton->show();
    QWidget::enterEvent(pEvent);
}

//-----------------------------------------------------------------------------
/// Qt leave event handler - Determines what to do when the mouse leaves this
/// widget.
/// \param pEvent The qt event being handled.
//-----------------------------------------------------------------------------
void DriverSettingItemWidget::leaveEvent(QEvent* pEvent)
{
    m_pRestoreDefaultSettingButton->hide();
    QWidget::leaveEvent(pEvent);
}

//-----------------------------------------------------------------------------
/// Create the appropriate edit widget based on the setting type.
/// \return The newly created widget.
//-----------------------------------------------------------------------------
QWidget* DriverSettingItemWidget::CreateEditWidget()
{
    QWidget* pEditWidget;

    // Create edit widget based on setting type
    switch (m_setting.type)
    {
        case SettingType::Boolean:
        {
            // Create a checkbox widget for boolean type
            QCheckBox* pCheckBox = new QCheckBox(this);
            pEditWidget = pCheckBox;

            // Setting edit indicated by checkbox state change
            connect(pCheckBox, &QCheckBox::stateChanged, this, &DriverSettingItemWidget::OnSettingEdited);
        }
        break;

        // Default to integer type to display the data when type isn't known
        case SettingType::Integer:
        case SettingType::Hex:
        case SettingType::Unknown:
        default:
        {
            // Create spinbox widget for integer type
            QSpinBox* pSpinBox = new QSpinBox(this);
            pEditWidget = pSpinBox;

            // Initialize spinbox
            pSpinBox->setMaximum(INT32_MAX);
            pSpinBox->setMinimum(INT32_MIN);

            // Setting edit indicated by spinbox value change
            connect(pSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnSettingEdited()));
        }
        break;

        case SettingType::UnsignedInteger:
        {
            // Create spinbox widget for unsigned integer type
            QSpinBox* pSpinBox = new QSpinBox(this);
            pEditWidget = pSpinBox;

            // Initialize spinbox
            pSpinBox->setMaximum(INT32_MAX);
            pSpinBox->setMinimum(0);

            // Setting edit indicated by spinbox value change
            connect(pSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnSettingEdited()));
        }
        break;

        case SettingType::Float:
        {
            // Create double spinbox widget for float type
            QDoubleSpinBox* pSpinBox = new QDoubleSpinBox(this);
            pEditWidget = pSpinBox;

            // Initialize double spinbox
            pSpinBox->setMaximum(FLT_MAX);
            pSpinBox->setMinimum(FLT_MIN);

            // Setting edit indicated by spinbox value change
            connect(pSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnSettingEdited()));
        }
        break;

        case SettingType::String:
        {
            // Create line edit widget for string type
            QLineEdit* pLineEdit = new QLineEdit(this);
            pEditWidget = pLineEdit;

            // Setting edit indicated by finishing line edit
            connect(pLineEdit, &QLineEdit::editingFinished, this, &DriverSettingItemWidget::OnSettingEdited);
        }
        break;
    }

    return pEditWidget;
}

//-----------------------------------------------------------------------------
/// Update the view to reflect the model data. This determines the correct type
/// of edit widget based on the setting type and updates it accordingly.
//-----------------------------------------------------------------------------
void DriverSettingItemWidget::SetViewFromModel()
{
    switch (m_setting.type)
    {
        // Boolean corresponds to QCheckBox widget
        case SettingType::Boolean:
            static_cast<QCheckBox*>(m_pEditWidget)->setChecked(m_setting.value.boolValue);
            break;

        // Integer corresponds to QSpinBox widget
        case SettingType::Integer:
            static_cast<QSpinBox*>(m_pEditWidget)->setValue(m_setting.value.integerValue);
            break;

        // Unsigned integer corresponds to QSpinBox widget
        case SettingType::UnsignedInteger:
            static_cast<QSpinBox*>(m_pEditWidget)->setValue(m_setting.value.unsignedIntegerValue);
            break;

        // Float corresponds to QDoubleSpinBox widget
        case SettingType::Float:
            static_cast<QDoubleSpinBox*>(m_pEditWidget)->setValue(m_setting.value.floatValue);
            break;

        // String corresponds to QLineEdit widget
        case SettingType::String:
            static_cast<QLineEdit*>(m_pEditWidget)->setText(m_setting.value.stringValue);
            break;

        // Hex values use a spin box to edit the value for now.
        case SettingType::Hex:
            static_cast<QSpinBox*>(m_pEditWidget)->setValue(m_setting.value.hexValue);
            break;

        default:
            Q_ASSERT(false);
            break;
    }
}

//-----------------------------------------------------------------------------
/// Update the model data to reflect the view. This determines the correct type
/// of edit widget based on the setting type and updates the model data from the
/// edit widget.
//-----------------------------------------------------------------------------
void DriverSettingItemWidget::SetModelFromView()
{
    switch (m_setting.type)
    {
        // Boolean corresponds to QCheckBox widget
        case SettingType::Boolean:
            m_setting.value.boolValue = static_cast<QCheckBox*>(m_pEditWidget)->isChecked();
            break;

        // Integer corresponds to QSpinBox widget
        case SettingType::Integer:
            m_setting.value.integerValue = static_cast<QSpinBox*>(m_pEditWidget)->value();
            break;

        // Unsigned integer corresponds to QSpinBox widget
        case SettingType::UnsignedInteger:
            m_setting.value.unsignedIntegerValue = static_cast<QSpinBox*>(m_pEditWidget)->value();
            break;

        // Float corresponds to QDoubleSpinBox widget
        case SettingType::Float:
            m_setting.value.floatValue = static_cast<QDoubleSpinBox*>(m_pEditWidget)->value();
            break;

        // String corresponds to QLineEdit widget
        case SettingType::String:
            {
                std::string text = static_cast<QLineEdit*>(m_pEditWidget)->text().toStdString();

                // Validate the incoming string. If larger than kSmallStringSize, warn the user.
                bool validString = text.length() < kSmallStringSize;
                if (validString)
                {
                    DevDriver::Platform::Strncpy(m_setting.value.stringValue, text.c_str(), sizeof(char) * kSmallStringSize);
                }
                else
                {
                    RDPUtil::DbgMsg("[RDP] New string for setting '%s' is too large.", m_setting.name);
                }
            }
            break;

        default:
            Q_ASSERT(false);
            break;
    }
}

//-----------------------------------------------------------------------------
/// Fix incoming newline characters. The setting descriptions come with '\\' and
/// 'n' baked in as characters rather than a '\n' character, so replace those
/// instances with the actual newline and carriage return characters.
/// \param input The "dirty" input string.
/// \param output The "clean" output string.
//-----------------------------------------------------------------------------
void DriverSettingItemWidget::FixNewlineCharacters(const QString& input, QString& output)
{
    output = input;

    // Regular expressions for \n and \r
    QRegExp newlineRegex(QString("\\\\n"));
    QRegExp returnRegex(QString("\\\\r"));

    // Replace with actual characters
    output.replace(newlineRegex, QString('\n'));
    output.replace(returnRegex, QString('\r'));
}
