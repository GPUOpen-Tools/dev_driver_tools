//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief The model used to manage the values for each device clock mode.
//=============================================================================

#include <cmath>                // for round

#include "ClocksTabModel.h"
#include "DeviceClockModeModel.h"
#include "../Settings/RDPSettings.h"
#include "../Util/RDPUtil.h"
#include "../../DevDriverComponents/inc/protocols/driverControlClient.h"
#include <QVariant>

/// mGPU is not supported at the moment, so just target GPU 0 for now.
static const int kGpuIndex = 0;

//-----------------------------------------------------------------------------
/// Constructor for ClocksTabModel.
/// \param pPanelModel The main DeveloperPanelModel instance for RDP.
/// \param modelCount The number of models used registered with the interface.
//-----------------------------------------------------------------------------
ClocksTabModel::ClocksTabModel(DeveloperPanelModel* pPanelModel, uint32_t modelCount)
    : DriverProtocolModel(pPanelModel, modelCount)
    , m_pDriverControlClient(nullptr)
{
}

//-----------------------------------------------------------------------------
/// Destructor for ClocksTabModel.
//-----------------------------------------------------------------------------
ClocksTabModel::~ClocksTabModel()
{
    for (int modelIndex = 0; modelIndex < m_clockModeModels.size(); ++modelIndex)
    {
        SAFE_DELETE(m_clockModeModels[modelIndex]);
    }
}

//-----------------------------------------------------------------------------
/// Create a new instance of an individual GPU clock mode model.
/// \param properties The properties structure used to create a new clock mode model instance.
/// \returns A new instance of a clock mode model, holding GPU frequencies.
//-----------------------------------------------------------------------------
DeviceClockModeModel* ClocksTabModel::CreateClockModeModel(const ModeProperties& properties)
{
    DeviceClockModeModel* pDeviceClockModeModel = new DeviceClockModeModel(properties, CLOCK_MODE_CONTROLS_COUNT);

    if (pDeviceClockModeModel != nullptr)
    {
        m_clockModeModels.push_back(pDeviceClockModeModel);
    }

    return pDeviceClockModeModel;
}

//-----------------------------------------------------------------------------
/// Collect GPU clock values for each clock mode that RDP exposes.
/// \param connectedClientId The ClientId for the client used to retrieve GPU clock values.
/// \returns True if the clock frequency values were retrieved successfully, and false if the request failed somehow.
//-----------------------------------------------------------------------------
bool ClocksTabModel::CollectClockValues(DevDriver::ClientId connectedClientId)
{
    bool clocksRetrieved = true;

    // If there's a bogus ClientId, do nothing.
    if (connectedClientId == 0)
    {
        RDPUtil::DbgMsg("[RDP] Failed to collect device clock frequencies due to invalid ClientId.");
        return false;
    }

    using namespace DevDriver;
    using namespace DevDriver::DriverControlProtocol;

    float shaderClock;
    float memoryClock;

    SetConnectedClientId(connectedClientId);

    // Obtain an instance of DriverControlClient to use for all clock setting/querying operations below.
    DriverControlClient* pDriverControlClient = nullptr;
    bool gotDriverControlClient = GetClientByType(Protocol::DriverControl, (DevDriver::IProtocolClient**)(&pDriverControlClient));
    if (gotDriverControlClient && (pDriverControlClient != nullptr))
    {
        float shaderClockMax = 0;
        float memoryClockMax = 0;
        DevDriver::Result gotMaxClocks = pDriverControlClient->QueryMaxDeviceClock(kGpuIndex, &shaderClockMax, &memoryClockMax);
        if (gotMaxClocks == Result::Success)
        {
            RDPUtil::DbgMsg("[RDP] Successfully queried maximum device clocks.");
        }
        else
        {
            QString resultString = ToolUtil::GetResultString(gotMaxClocks);
            RDPUtil::DbgMsg("[RDP] Failed to retrieve GPU max device clock frequencies with result code '%s'.", resultString.toStdString().c_str());
            clocksRetrieved = false;
        }

        // Step through each clock widget and update the view with the collected frequency.
        for (int clockModeIndex = 0; clockModeIndex < m_clockModeModels.size(); ++clockModeIndex)
        {
            // Retrieve clock mode frequencies from each of the specified clock modes.
            DeviceClockModeModel* pClockModeModel = m_clockModeModels[clockModeIndex];
            Q_ASSERT(pClockModeModel != nullptr);

            // Switch the current GPU mode before querying the clock values.
            const ModeProperties& clockModeProperties = pClockModeModel->GetModeProperties();

            // Attempt to set the clock mode before querying the frequency for each mode.
            DevDriver::Result setClock = pDriverControlClient->SetDeviceClockMode(kGpuIndex, clockModeProperties.m_clockMode);
            QString clockModeString = RDPUtil::GetClockModeAsString(clockModeProperties.m_clockMode);
            if (setClock != Result::Success)
            {
                QString resultString = ToolUtil::GetResultString(setClock);
                RDPUtil::DbgMsg("[RDP] Failed to set device clock mode to %s when querying GPU frequencies. Result code '%s'",
                    clockModeString.toStdString().c_str(), resultString.toStdString().c_str());

                clocksRetrieved = false;
            }
            else
            {
                RDPUtil::DbgMsg("[RDP] Successfully set device clock mode to %s to query frequency.", clockModeString.toStdString().c_str());

                // Get the shader and memory clocks for the currently set mode.
                shaderClock = 0;
                memoryClock = 0;
                DevDriver::Result gotClocks = pDriverControlClient->QueryDeviceClock(kGpuIndex, &shaderClock, &memoryClock);
                if (gotClocks == Result::Success)
                {
                    RDPUtil::DbgMsg("[RDP] Successfully queried device clock frequencies for %s mode.", clockModeString.toStdString().c_str());

                    shaderClock = round(shaderClock);
                    memoryClock = round(memoryClock);
                    pClockModeModel->Update(CLOCK_MODE_CONTROLS_BASE_SHADER_CLOCK, shaderClock);
                    pClockModeModel->Update(CLOCK_MODE_CONTROLS_BASE_MEMORY_CLOCK, memoryClock);
                }
                else
                {
                    RDPUtil::DbgMsg("[RDP] Failed to retrieve GPU device frequency for %s mode.", clockModeString.toStdString().c_str());
                    clocksRetrieved = false;
                }
            }

            if (gotMaxClocks == Result::Success)
            {
                shaderClockMax = round(shaderClockMax);
                memoryClockMax = round(memoryClockMax);
                pClockModeModel->Update(CLOCK_MODE_CONTROLS_MAX_SHADER_CLOCK, shaderClockMax);
                pClockModeModel->Update(CLOCK_MODE_CONTROLS_MAX_MEMORY_CLOCK, memoryClockMax);
            }
            else
            {
                RDPUtil::DbgMsg("[RDP] Failed to retrieve GPU max clock frequencies.");
                clocksRetrieved = false;
            }
        }

        // After stepping through each mode and collecting values, reset to the user's chosen clock mode.
        DevDriver::DriverControlProtocol::DeviceClockMode userClockMode = RDPSettings::Get().GetUserClockMode();

        DevDriver::Result setUserClock = pDriverControlClient->SetDeviceClockMode(kGpuIndex, userClockMode);
        if (setUserClock != Result::Success)
        {
            RDPUtil::DbgMsg("[RDP] Failed to restore device clock mode to user's choice.");
        }

        // Release the DriverControlClient after we're done with it.
        ReleaseClient(pDriverControlClient);
    }
    else
    {
        RDPUtil::DbgMsg("[RDP] Failed to connect DriverControlClient used to collect GPU clocks.");
        clocksRetrieved = false;
    }

    return clocksRetrieved;
}

//-----------------------------------------------------------------------------
/// Set the current GPU clock mode.
/// \param clockMode The clock mode to switch to.
/// \returns The result of the set operation.
//-----------------------------------------------------------------------------
DevDriver::Result ClocksTabModel::SetClockMode(DevDriver::DriverControlProtocol::DeviceClockMode clockMode)
{
    DevDriver::Result setResult = DevDriver::Result::Error;

    if (GetConnectedClientId() != 0)
    {
        bool connected = ConnectDriverControlClient();
        if (connected)
        {
            // Attempt to set the clock mode
            setResult = m_pDriverControlClient->SetDeviceClockMode(kGpuIndex, clockMode);
            if (setResult != DevDriver::Result::Success)
            {
                RDPUtil::DbgMsg("[RDP] Failed to set clock mode to query GPU frequencies.");
            }
            else
            {
                QString clockModeString = RDPUtil::GetClockModeAsString(clockMode);
                std::string clockModeStdString = clockModeString.toStdString();
                RDPUtil::DbgMsg("[RDP] Set clock mode to %s.", clockModeStdString.c_str());
            }

            DisconnectDriverControlClient();
        }
    }
    else
    {
        QString clockModeString = RDPUtil::GetClockModeAsString(clockMode);
        std::string clockModeStdString = clockModeString.toStdString();
        RDPUtil::DbgMsg("[RDP] Clock mode switching to %s on next app connect.", clockModeStdString.c_str());
    }

    return setResult;
}

//-----------------------------------------------------------------------------
/// Connect the DriverControlClient used to get and set GPU clock modes.
/// \returns True if the client was connected successfully, and false if it failed.
//-----------------------------------------------------------------------------
bool ClocksTabModel::ConnectDriverControlClient()
{
    Q_ASSERT(m_pDriverControlClient == nullptr);

    bool connected = false;
    bool gotDriverControlClient = GetClientByType(DevDriver::Protocol::DriverControl, (DevDriver::IProtocolClient**)(&m_pDriverControlClient));
    if (gotDriverControlClient && (m_pDriverControlClient != nullptr))
    {
        connected = true;
    }

    return connected;
}

//-----------------------------------------------------------------------------
/// Disconnect the DriverControlClient used within the clocks tab.
//-----------------------------------------------------------------------------
void ClocksTabModel::DisconnectDriverControlClient()
{
    if (m_pDriverControlClient != nullptr && m_pDriverControlClient->IsConnected())
    {
        m_pDriverControlClient->Disconnect();
        ReleaseClient(m_pDriverControlClient);
        m_pDriverControlClient = nullptr;
    }
}
