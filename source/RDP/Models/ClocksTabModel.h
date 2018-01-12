//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief The model used to manage the values for each device clock mode.
//=============================================================================

#ifndef _CLOCKS_TAB_MODEL_H_
#define _CLOCKS_TAB_MODEL_H_

#include "DriverProtocolModel.h"
#include "DeviceClockModeModel.h"
#include "ProcessInfoModel.h"
#include "../../DevDriverComponents/inc/protocols/driverControlClient.h"

class DeviceClockModeModel;

/// Clock mode enumeration used to id buttons in clock button group
enum ClockModeType
{
    CLOCK_WIDGET_TYPE_NORMAL,
    CLOCK_WIDGET_TYPE_STABLE,
    // Peak clocks are only available in internal builds.
    CLOCK_WIDGET_TYPE_COUNT
};

/// The set of clock modes available to choose from within the clocks view.
static const ModeProperties kClockModeProperties[CLOCK_WIDGET_TYPE_COUNT] =
{
    { gs_CLOCKS_MODE_NAME_TEXT_NORMAL,  gs_CLOCKS_MODE_DESCRIPTION_TEXT_NORMAL, "Default",      DevDriver::DriverControlProtocol::DeviceClockMode::Default },
    { gs_CLOCKS_MODE_NAME_TEXT_STABLE,  gs_CLOCKS_MODE_DESCRIPTION_TEXT_STABLE, "Profiling",    DevDriver::DriverControlProtocol::DeviceClockMode::Profiling },
    // Only allow the user to choose PEAK clock mode when using an internal build.
};

/// The model used to manage the values for each device clock mode.
class ClocksTabModel : public DriverProtocolModel
{
    Q_OBJECT
public:
    explicit ClocksTabModel(DeveloperPanelModel* pPanelModel, uint32_t modelCount);
    virtual ~ClocksTabModel();

    DeviceClockModeModel* CreateClockModeModel(const ModeProperties& properties);
    bool CollectClockValues(DevDriver::ClientId connectedClientId);
    DevDriver::Result SetClockMode(DevDriver::DriverControlProtocol::DeviceClockMode clockMode);

private:
    bool ConnectDriverControlClient();
    void DisconnectDriverControlClient();

    QList<DeviceClockModeModel*> m_clockModeModels; ///< A list containing info for device clock mode.
    DevDriver::DriverControlProtocol::DriverControlClient* m_pDriverControlClient;    ///< Client used to get and set clock modes.
};

#endif // _CLOCKS_TAB_MODEL_H_
