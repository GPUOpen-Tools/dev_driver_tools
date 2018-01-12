//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A widget used to display the frequencies for a given clock mode.
//=============================================================================

#include "ClockModeWidget.h"
#include "ui_ClockModeWidget.h"
#include "../Models/DeviceClockModeModel.h"
#include "../RDPDefinitions.h"
#include "../../Common/DriverToolsDefinitions.h"

/// A skeleton stylesheet for the clock mode's image, where the On/Off/Hover state will be replaced.
static const QString kClockModeImageStylesheet =
"QCheckBox::indicator "
"{ "
"    width: 249px; "
"    height: 155px; "
"} "
" "
"QCheckBox::indicator::unchecked "
"{ "
"    image: url(:/images/%1.png); "
"} "
" "
"QCheckBox::indicator::checked "
"{ "
"    image: url(:/images/%2.png); "
"} "
" "
"QCheckBox::indicator::unchecked:hover "
"{ "
"    image: url(:/images/%3.png); "
"} ";

//-----------------------------------------------------------------------------
/// Constructor for the ClockModeWidget type.
/// \param pClockModeModel The mode model holding this widget's data.
/// \param usesStableClocks A flag used to indicate if the clock mode uses stable clock frequencies.
/// \param pParent The parent widget.
//-----------------------------------------------------------------------------
ClockModeWidget::ClockModeWidget(DeviceClockModeModel* pClockModeModel, bool usesStableClocks, QWidget* pParent) :
    QWidget(pParent),
    ui(new Ui::ClockModeWidget),
    m_pClockModeModel(pClockModeModel)
{
    ui->setupUi(this);

    if (m_pClockModeModel != nullptr)
    {
        m_pClockModeModel->InitializeModel(ui->deviceClockName,     CLOCK_MODE_CONTROLS_MODE_NAME,          "text");
        m_pClockModeModel->InitializeModel(ui->clockDescription,    CLOCK_MODE_CONTROLS_MODE_DESCRIPTION,   "text");
        m_pClockModeModel->InitializeModel(ui->currentShaderClock,  CLOCK_MODE_CONTROLS_BASE_SHADER_CLOCK,  "text");
        m_pClockModeModel->InitializeModel(ui->currentMemoryClock,  CLOCK_MODE_CONTROLS_BASE_MEMORY_CLOCK,  "text");
        m_pClockModeModel->InitializeModel(ui->maxShaderClock,      CLOCK_MODE_CONTROLS_MAX_SHADER_CLOCK,   "text");
        m_pClockModeModel->InitializeModel(ui->maxMemoryClock,      CLOCK_MODE_CONTROLS_MAX_MEMORY_CLOCK,   "text");

        // If the mode uses stable clocks, replace several label strings.
        if (usesStableClocks)
        {
            // Change the "Base" clock header to "Fixed," as the clock frequency won't change in this mode.
            ui->baseHeaderLabel->setText(gs_CLOCKS_FIXED_FREQUENCY_HEADER);

            // Hide the "Max" column widgets since the frequency won't deviate from the "Fixed" value.
            ui->clockValuesGrid->removeWidget(ui->maxClockHeaderLabel);
            ui->clockValuesGrid->removeWidget(ui->maxShaderClock);
            ui->clockValuesGrid->removeWidget(ui->maxMemoryClock);
            ui->maxClockHeaderLabel->hide();
            ui->maxShaderClock->hide();
            ui->maxMemoryClock->hide();
        }

        m_pClockModeModel->InitializeDefaults();
    }
}

//-----------------------------------------------------------------------------
/// Destructor for the ClockModeWidget.
//-----------------------------------------------------------------------------
ClockModeWidget::~ClockModeWidget()
{
    SAFE_DELETE(ui);
}

//-----------------------------------------------------------------------------
/// Get the widget's instance of the button used to set the clock mode.
/// \returns The widget instance used to set the clock mode.
//-----------------------------------------------------------------------------
QCheckBox* ClockModeWidget::GetClockButton() const
{
    if (ui != nullptr)
    {
        return ui->clockWidgetButton;
    }

    // The ClockModeWidget ui file is probably broken if this happens.
    Q_ASSERT(false);
    return nullptr;
}

//-----------------------------------------------------------------------------
/// Set the images by a given prefix. The On/Off/Hover strings will be generated using the prefix.
/// \param filenamePrefix The image filename prefix. Full image filenames are generated based on it.
//-----------------------------------------------------------------------------
void ClockModeWidget::SetModeImage(const QString& filenamePrefix)
{
    // Each clock mode has an Off/On/Hover image. Generate the proper resource filename for each widget.
    QString offString = filenamePrefix + "ClockOff";
    QString onString = filenamePrefix + "ClockOn";
    QString hoverString = filenamePrefix + "ClockHover";

    // Start with the base stylesheet and replace portions with the correct image prefix.
    QString newStylesheet = kClockModeImageStylesheet.arg(offString, onString, hoverString);

    // Set the On/Off/Hover images for the button by replacing the stylesheet.
    ui->clockWidgetButton->setStyleSheet(newStylesheet);
}
