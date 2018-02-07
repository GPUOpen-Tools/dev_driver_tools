//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Implementation of clock mode UI
//=============================================================================

#include "ClocksView.h"
#include "ui_ClocksView.h"

#include "ClockModeWidget.h"
#include "../Models/ClocksTabModel.h"
#include "../Models/DeveloperPanelModel.h"
#include "../../Common/ToolUtil.h"
#include "../../Common/DriverToolsDefinitions.h"
#include <ScalingManager.h>
#include "../Settings/RDPSettings.h"
#include "../Util/RDPUtil.h"
#include <QPushButton>
#include <QThread>

//-----------------------------------------------------------------------------
/// Constructor.
/// \param pPanelModel The main DeveloperPanelModel instance for RDP.
/// \param pParent The parent widget for this view.
//-----------------------------------------------------------------------------
ClocksView::ClocksView(DeveloperPanelModel* pPanelModel, QWidget* pParent)
    : QWidget(pParent),
    ui(new Ui::ClocksView)
{
    ui->setupUi(this);

    // Set white background for this pane.
    ToolUtil::SetWidgetBackgroundColor(this, Qt::white);

    m_pClockTabModel = new ClocksTabModel(pPanelModel, 0);
    if (m_pClockTabModel != nullptr)
    {
        InitializeClockModeWidgets();
        InitializeInterface();
    }

    // Attach a handler that's invoked whenever the target process's driver initialization status changes.
    connect(pPanelModel, &DeveloperPanelModel::UpdateDriverInitializedStatus, this, &ClocksView::OnDriverInitializedStatusUpdated);
}

//-----------------------------------------------------------------------------
/// Destructor.
//-----------------------------------------------------------------------------
ClocksView::~ClocksView()
{
    for (int modeIndex = 0; modeIndex < m_modeWidgets.size(); ++modeIndex)
    {
        SAFE_DELETE(m_modeWidgets[modeIndex]);
    }

    SAFE_DELETE(m_pClockButtonGroup);
    SAFE_DELETE(ui);

    SAFE_DELETE(m_pClockTabModel);
}

//-----------------------------------------------------------------------------
/// Handler used to update the Clocks view when a target process's driver has been initialized.
/// \param processInfo The ProcessInfoModel instance describing the target process.
/// \param isInitialized A flag used to indicate if the target application's driver has been fully initialized.
//-----------------------------------------------------------------------------
void ClocksView::OnDriverInitializedStatusUpdated(const ProcessInfoModel& processInfo, bool isInitialized)
{
    if (isInitialized)
    {
        bool modelInitialized = (m_pClockTabModel != nullptr);
        Q_ASSERT(modelInitialized);

        if (modelInitialized)
        {
            DevDriver::ClientId clientId = processInfo.GetMostRecentClientId();
            m_pClockTabModel->SetConnectedClientId(clientId);

            InitializeInterface();

            // Let the user know that an attempt will be made to collect clock frequencies.
            const QString& processName = processInfo.GetProcessName();
            RDPUtil::DbgMsg("[RDP] Driver has been initialized within %s process. Attempting to retrieve device clock frequencies.", processName.toStdString().c_str());

            // Use the first connected process's ClientId.
            bool frequenciesCollected = m_pClockTabModel->CollectClockValues(clientId);
            if (frequenciesCollected)
            {
                RDPUtil::DbgMsg("[RDP] Successfully collected device clock frequencies.");
            }
            else
            {
                RDPUtil::DbgMsg("[RDP] Failed to collect device clock frequencies.");
            }

            RDPUtil::DbgMsg("[RDP] Reverting to user's clock mode.");

            // Apply the user's clock mode choice using the connected application.
            DevDriver::DriverControlProtocol::DeviceClockMode userClockMode = RDPSettings::Get().GetUserClockMode();
            m_pClockTabModel->SetClockMode(userClockMode);
        }
    }
    else
    {
        // The process disconnected, so make sure it doesn't try to communicate with the Client anymore.
        m_pClockTabModel->SetConnectedClientId(0);
    }
}

//-----------------------------------------------------------------------------
/// Initialize each clock widget within the tab interface.
//-----------------------------------------------------------------------------
void ClocksView::InitializeClockModeWidgets()
{
    ScalingManager& scalingManager = ScalingManager::Get();

    // Set up clock button group
    m_pClockButtonGroup = new QButtonGroup(this);
    connect(m_pClockButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(OnClockButtonPressed(int)));

    // Step through each clock mode we want to query and add a widget for it.
    int startMode = CLOCK_WIDGET_TYPE_NORMAL;
    int endMode = CLOCK_WIDGET_TYPE_COUNT;
    for (int clockModeIndex = startMode; clockModeIndex < endMode; ++clockModeIndex)
    {
        ClockModeType widgetType = static_cast<ClockModeType>(clockModeIndex);

        const ModeProperties& properties = kClockModeProperties[clockModeIndex];

        // Create a new model used to store the mode's frequency data.
        DeviceClockModeModel* pClockModeModel = m_pClockTabModel->CreateClockModeModel(properties);
        if (pClockModeModel != nullptr)
        {
            // In modes other than Normal, the clock values don't fluctuate, and only display a single "Fixed" frequency column.
            bool modeUsesStableClocks = (clockModeIndex != CLOCK_WIDGET_TYPE_NORMAL);

            ClockModeWidget* pModeWidget = new ClockModeWidget(pClockModeModel, modeUsesStableClocks, this);

            // Create a new clock mode widget and replace the images using the given mode prefix.
            pModeWidget->SetModeImage(properties.m_imagePrefix);
            m_modeWidgets.push_back(pModeWidget);

            // Register the new widget with the scaling manager so it resizes properly.
            scalingManager.RegisterObject(pModeWidget);

            // Add the clock mode's widget to the clock tab interface.
            ui->clockModesGrid->addWidget(pModeWidget, 0, clockModeIndex);

            // Add the mode widget's clock button to the group of mode buttons.
            m_pClockButtonGroup->addButton(pModeWidget->GetClockButton(), widgetType);
        }
    }
}

//-----------------------------------------------------------------------------
/// Initialize the clock interface when it first starts up.
//-----------------------------------------------------------------------------
void ClocksView::InitializeInterface()
{
    if (m_modeWidgets.empty())
    {
        return;
    }

    // Retrieve the user's clock mode saved in the settings file.
    DevDriver::DriverControlProtocol::DeviceClockMode userClockMode = RDPSettings::Get().GetUserClockMode();

    // Step through the list of clock modes and find the user's choice
    for (int modeIndex = 0; modeIndex < m_modeWidgets.size(); ++modeIndex)
    {
        const ModeProperties& properties = kClockModeProperties[modeIndex];
        if (properties.m_clockMode == userClockMode)
        {
            // Activate the widget associated with the user's choice of clock mode.
            ClockModeWidget* pWidget = m_modeWidgets[modeIndex];
            if (pWidget != nullptr)
            {
                pWidget->GetClockButton()->click();
                break;
            }
        }
    }
}

//-----------------------------------------------------------------------------
/// Handler invoked when a clock button is pressed. Use the id to determine which
/// specific button was pressed.
/// \param id The id of the button in the button group (buttons are given meaningful
/// ClockMode enum values as ids).
//-----------------------------------------------------------------------------
void ClocksView::OnClockButtonPressed(int id)
{
    using namespace DevDriver::DriverControlProtocol;

    DeviceClockMode clockMode = DeviceClockMode::Default;

    if (id >= 0 && id < CLOCK_WIDGET_TYPE_COUNT)
    {
        clockMode = kClockModeProperties[id].m_clockMode;
    }
    else
    {
        // If this asserts, a new clock mode has been added, but the clocks array hasn't been updated.
        Q_ASSERT(false);
        RDPUtil::DbgMsg("[RDP] Failed to set user's selected clock mode.");
    }

    // Only change the clock mode if it's different than the current clock mode.
    RDPSettings& rdpSettings = RDPSettings::Get();
    if (rdpSettings.GetUserClockMode() != clockMode)
    {
        // Set the clock mode on the GPU.
        m_pClockTabModel->SetClockMode(clockMode);

        // Save the clock mode choice in the RDP settings.
        rdpSettings.SetUserClockMode(clockMode);
    }
}
