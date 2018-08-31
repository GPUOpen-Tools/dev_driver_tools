//=============================================================================
/// Copyright (c) 2016-2018 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief The Developer Panel Model used to communicate with the Radeon Developer Service.
//=============================================================================

#include <QStandardItemModel>
#include <QList>
#include <QThread>
#include <algorithm>
#include <chrono>
#include <ctime>

#include "DeveloperPanelModel.h"
#include "ApplicationSettingsModel.h"
#include "ConnectionSettingsModel.h"
#include "DriverSettingsModel.h"
#include "DriverMessageProcessorThread.h"
#include "SetupTargetApplicationModel.h"
#include "ConnectionStatusWorker.h"
#include "../AppSettings/ApplicationSettingsFile.h"
#include "../Settings/RDPSettings.h"
#include "../Util/RDPUtil.h"
#include "../../DevDriverComponents/inc/protocols/driverControlClient.h"
#include "../../DevDriverComponents/inc/protocols/loggingClient.h"
#include "../../DevDriverComponents/inc/protocols/rgpClient.h"
#include "../../DevDriverComponents/inc/protocols/ddURIClient.h"
#include "../../DevDriverComponents/inc/msgChannel.h"

using namespace DevDriver;
using namespace DevDriver::DriverControlProtocol;
using namespace DevDriver::SettingsProtocol;

/// The maximum amount of time to wait for the driver to initialize.
static const DevDriver::uint32 kDriverInitializationTimeoutMilliseconds = 3000;

//-----------------------------------------------------------------------------
/// Constructor for DeveloperPanelModel.
//-----------------------------------------------------------------------------
DeveloperPanelModel::DeveloperPanelModel()
    : m_channelContext({})
    , m_connectedToRds(false)
    , m_pMessageProcessorThread(nullptr)
    , m_pMessageProcessorWorker(nullptr)
    , m_pPanelSettingsModel(nullptr)
    , m_pTargetApplicationModel(nullptr)
{
    // Register the ProcessInfoModel class so it can be used as a parameter for
    // signals/slots. Data type also needs to be declared as a metatype using
    // Q_DECLARE_METATYPE (see definition of ProcessInfoModel)
    int id = qRegisterMetaType<ProcessInfoModel>();
    Q_UNUSED(id);

    // Allow the DeveloperPanelModel to trigger popups when the profiler is already in use when a new application starts.
    connect(this, &DeveloperPanelModel::ProfilerAlreadyInUse, this, &DeveloperPanelModel::OnProfilerAlreadyInUse);
    connect(this, &DeveloperPanelModel::DisplayUnsupportedASICNotification, this, &DeveloperPanelModel::OnDisplayUnsupportedASICNotification);
}

//-----------------------------------------------------------------------------
/// Destructor for DeveloperPanelModel.
//-----------------------------------------------------------------------------
DeveloperPanelModel::~DeveloperPanelModel()
{
    Disconnect();

    SAFE_DELETE(m_pPanelSettingsModel);
    SAFE_DELETE(m_pTargetApplicationModel);
}

//-----------------------------------------------------------------------------
/// Let the developer panel model know about the Target application model
/// \param pTargetApplicationModel Pointer to the target application model
//-----------------------------------------------------------------------------
void DeveloperPanelModel::SetTargetApplicationsModel(SetupTargetApplicationModel* pTargetApplicationModel)
{
    m_pTargetApplicationModel = pTargetApplicationModel;
}

//-----------------------------------------------------------------------------
/// Attempt to initialize a connection to the Developer Service.
/// \returns True if the connection was established successfully; false if it failed.
//-----------------------------------------------------------------------------
bool DeveloperPanelModel::InitializeConnectionToRDS()
{
    using namespace DevDriver;

    Result result = Result::Error;

    ConnectionSettingsModel* pConnectionSettingsModel = static_cast<ConnectionSettingsModel*>(GetProtocolModel(MAIN_PANEL_MODEL_CONNECTION_SETTINGS));
    if (pConnectionSettingsModel != nullptr)
    {
        const RDSConnectionInfo& createInfoCopy = pConnectionSettingsModel->GetConnectionCreateInfo();

        // Create the main DevDriverClient instance. Use this to communicate with each halted developer mode process.
        DevDriverClient* pDriverClient = new DevDriverClient(GenericAllocCb, createInfoCopy.rdsInfo);
        if (pDriverClient != nullptr)
        {
            m_channelContext.pClient = pDriverClient;
            m_channelContext.exitRequested = false;

            result = m_channelContext.pClient->Initialize();

            if (result == Result::Success)
            {
                // Set up message processor thread
                m_pMessageProcessorThread = new QThread;
                if (m_pMessageProcessorThread != nullptr)
                {
                    m_pMessageProcessorWorker = new DriverMessageProcessorThread(&m_channelContext, this);

                    m_pMessageProcessorWorker->moveToThread(m_pMessageProcessorThread);
                    connect(m_pMessageProcessorThread, SIGNAL(started()), m_pMessageProcessorWorker, SLOT(StartMessageProcessingLoop()));
                    connect(m_pMessageProcessorThread, SIGNAL(finished()), m_pMessageProcessorWorker, SLOT(ThreadFinished()), Qt::DirectConnection);

                    m_pMessageProcessorThread->start();
                }

                // Set up connection status thread
                m_pConnectionStatusThread = new QThread();
                if (m_pConnectionStatusThread != nullptr)
                {
                    m_pConnectionStatusWorker = new ConnectionStatusWorker(&m_channelContext);
                    m_pConnectionStatusWorker->moveToThread(m_pConnectionStatusThread);

                    connect(m_pConnectionStatusWorker, &ConnectionStatusWorker::ClientDisconnected, this, &DeveloperPanelModel::Disconnect);

                    // Start the thread and worker loop
                    m_pConnectionStatusThread->start();
                    m_pConnectionStatusWorker->StartConnectionStatusLoop();
                }

                m_connectedToRds = true;

                emit Connected();
            }
        }
    }

    return (result == Result::Success);
}

//-----------------------------------------------------------------------------
/// Send a signal to the connected RDS to initiate termination of the process.
/// \returns True if the request was executed correctly.
//-----------------------------------------------------------------------------
bool DeveloperPanelModel::TerminateConnectedRDS()
{
    ChannelContext& channelContext = GetChannelContext();
    if (channelContext.pClient != nullptr)
    {
        DevDriver::IMsgChannel* pMessageChannel = channelContext.pClient->GetMessageChannel();
        if (pMessageChannel == nullptr)
        {
            return false;
        }

        using namespace DevDriver::URIProtocol;
        URIClient* pURIClient = static_cast<URIClient*>(channelContext.pClient->AcquireProtocolClient<DevDriver::Protocol::URI>());
        if (pURIClient == nullptr)
        {
            return false;
        }

        // Build a filter to find a ClientId for the RDS we're connected to.
        DevDriver::ClientMetadata filter = {};
        filter.clientType = Component::Server;
        ClientId rdsClientId = {};

        Result gotClientId = pMessageChannel->FindFirstClient(filter, &rdsClientId);
        if (gotClientId == DevDriver::Result::Success)
        {
            // Attempt to establish a connection to RDS with a URIClient.
            Result connectResult = pURIClient->Connect(rdsClientId);
            if (connectResult == Result::Success)
            {
                // Send the terminate command.
                const char* pTerminateMessage = "command://terminate";

                ResponseHeader responseHeader = {};
                DevDriver::Result result = pURIClient->RequestURI(pTerminateMessage, &responseHeader);
                if (result != Result::Success)
                {
                    RDPUtil::DbgMsg("[RDP] Failed to send RDS terminate request.");
                }

                pURIClient->Disconnect();
            }
            else
            {
                RDPUtil::DbgMsg("[RDP] Failed to connect URIClient to send RDS terminate request.");
            }
        }
        else
        {
            RDPUtil::DbgMsg("[RDP] Failed to find RDS ClientId on connected message channel.");
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
/// Disconnect and destroy the Panel communication interface.
//-----------------------------------------------------------------------------
void DeveloperPanelModel::Disconnect()
{
    if (m_connectedToRds)
    {
        // Set rds connection state (this is done before eveything else so that any
        // threads which require connection components know not to do anything with them
        // while they are being destroyed)
        m_connectedToRds = false;

        // Stop and destroy the background message processing thread.
        m_channelContext.exitRequested = true;
        m_pMessageProcessorThread->quit();
        m_pMessageProcessorThread->wait();
        m_pConnectionStatusThread->quit();
        m_pConnectionStatusThread->wait();
        SAFE_DELETE(m_pMessageProcessorThread);
        SAFE_DELETE(m_pMessageProcessorWorker);
        SAFE_DELETE(m_pConnectionStatusThread);
        SAFE_DELETE(m_pConnectionStatusWorker);

        // Destroy the connected ChannelContext to RDS.
        m_channelContext.pClient->Destroy();
        SAFE_DELETE(m_channelContext.pClient);
        memset(&m_channelContext, 0, sizeof(ChannelContext));

        // Client disconnected handling - handles disconnection actions for all connected processes
        for (int processInfoIndex = 0; processInfoIndex < m_processInfoList.size(); ++processInfoIndex)
        {
            ProcessInfoModel& infoModel = m_processInfoList[processInfoIndex];
            infoModel.SetConnectedStatus(false);

            // Update process running status
            emit UpdateClientRunStatus(infoModel, false);

            DevDriver::ProcessId profiledProcessId = FindProfileEnabledProcess();
            if (profiledProcessId == infoModel.GetProcessId())
            {
                infoModel.SetProfilingStatus(false);
            }
        }

        RDPUtil::DbgMsg("[RDP] Disconnected from RDS");
        emit Disconnected();
    }
}

//-----------------------------------------------------------------------------
/// Add a new ClientId to the list of known clients that we can communicate with.
/// \param srcClientId The ClientId of the new client connected in developer mode.
//-----------------------------------------------------------------------------
void DeveloperPanelModel::AddClientId(DevDriver::ClientId srcClientId)
{
    // Add the new ClientId to the panel model's list of known ClientIds.
    m_knownClientIdList.push_back(srcClientId);
}

//-----------------------------------------------------------------------------
/// Add the incoming client to the list of known process info.
/// \param srcClientId The ClientId for the new client.
/// \param processName The name string of the process.
/// \param processId The process Id for the new process.
/// \param clientDescription The description string for the new client.
//-----------------------------------------------------------------------------
void DeveloperPanelModel::AddClientInfo(DevDriver::ClientId srcClientId, const QString& processName, DevDriver::ProcessId processId, const QString& clientDescription)
{
    RDPUtil::DbgMsg("[RDP] Processing halted client with id %u: %s:%u - %s", srcClientId, processName.toStdString().c_str(), processId, clientDescription.toStdString().c_str());

    // look to see if the clientID has been seen before for this processId. If it's been seen before, it could be being processed
    // in a different thread so ignore it if so.
    bool seenClientBefore = false;
    for (int processIndex = 0; processIndex < m_processInfoList.size(); ++processIndex)
    {
        ProcessInfoModel& processInfoFromList = m_processInfoList[processIndex];
        if (processInfoFromList.GetProcessId() == processId)
        {
            if (processInfoFromList.HasSeenClientId(srcClientId) == true)
            {
                RDPUtil::DbgMsg("[RDP] Seen ClientId %d for process %s", srcClientId, processName.toStdString().c_str());
                seenClientBefore = true;
                break;
            }
        }
    }

    if (seenClientBefore == false)
    {
        // Create a ProcessInfoModel with the process info, and then update with the ClientId it's using.
        ProcessInfoModel processInfo(processName, clientDescription, processId);
        processInfo.UpdateClientId(srcClientId);
        RDPUtil::DbgMsg("[RDP] Updated %s ClientId to %d", processName.toStdString().c_str(), srcClientId);

        // Connect a DriverControlClient to the application to avoid allowing a timeout to resume it.
        DriverControlClient* pDriverControlClient = ConnectDriverControlClient(processInfo);

        bool wasBlacklisted = false;
        if (RDPSettings::Get().CheckBlacklistMatch(processName))
        {
            RDPUtil::DbgMsg("[RDP] Process %s blacklisted, no action taken", processName.toStdString().c_str());
            wasBlacklisted = true;
        }
        else
        {
            // We only need to filter a process if it's new, or if the ClientId has been updated.
            bool shouldFilterProcess = false;

            // Has RDP already seen this process? Ignore repeated requests from the same process.
            bool hasProcessInfo = HasProcessInfo(processInfo);
            if (!hasProcessInfo)
            {
                 // Add the process info to the list for the first time.
                m_processInfoList.push_back(processInfo);
                shouldFilterProcess = true;
            }
            else
            {
                bool updatedClientId = TryUpdateClientId(processInfo);
                if (updatedClientId)
                {
                    shouldFilterProcess = true;
                }
            }

            if (shouldFilterProcess)
            {
                // The ClientId has been updated for the process. It's still actively running, so update the status.
                emit UpdateClientRunStatus(processInfo, true);

                // Filter each process by Target Executable filename.
                FilterHaltedProcess(srcClientId, processInfo);
            }
        }

        if (pDriverControlClient != nullptr)
        {
            // Resume each halted process after we're finished applying settings.
            ResumeHaltedProcess(pDriverControlClient, processInfo);

            // If the process was blacklisted, don't wait for the driver to be initialized- just move on.
            if (wasBlacklisted == false)
            {
                // Wait for the driver to be initialized inside the application.
                WaitForDriverInitialization(pDriverControlClient, processInfo);
            }

            // Disconnect the driver control client since we're done with it now.
            DisconnectDriverControlClient(pDriverControlClient);
        }
        else
        {
            const QString& processNameQString = processInfo.GetProcessName();
            std::string processNameString = processNameQString.toStdString();
            RDPUtil::DbgMsg("[RDP] Couldn't filter halted process '%s' because DriverControlClient failed to connect.", processNameString.c_str());
        }
    }
}

//-----------------------------------------------------------------------------
/// Handle cases where RDS has sent an explicit "Client Disconnected" message.
/// This lets RDP know that a developer mode process has been shut down.
/// \param srcClientId The ClientId for the developer mode process that has been shut down.
//-----------------------------------------------------------------------------
void DeveloperPanelModel::ClientDisconnected(DevDriver::ClientId srcClientId)
{
    // Step through each ProcessInfoModel to find one with a matching ClientId.
    // This ClientId is now invalid, and the ProcessInfoModel is in a disconnected state.
    for (int processInfoIndex = 0; processInfoIndex < m_processInfoList.size(); ++processInfoIndex)
    {
        ProcessInfoModel& infoModel = m_processInfoList[processInfoIndex];
        bool matchesProcess = infoModel.HasSeenClientId(srcClientId);
        if (matchesProcess)
        {
            infoModel.SetConnectedStatus(srcClientId, false);
            infoModel.SetDriverInitializedStatus(false);

            emit UpdateClientRunStatus(infoModel, false);
            emit UpdateDriverInitializedStatus(infoModel, false);

            // Get the most recent client id that is still connected, just in case the
            // current client (srcClientId) is the one that is enabled for profiling
            const ClientId lastClientId = infoModel.GetMostRecentClientId(true);
            if (lastClientId != 0)
            {
                m_pPanelSettingsModel->SetConnectedClientId(lastClientId);
            }

            DevDriver::ProcessId profiledProcessId = FindProfileEnabledProcess();
            if (profiledProcessId == infoModel.GetProcessId())
            {
                infoModel.SetProfilingStatus(srcClientId, false);
                emit ProfiledProcessInfoUpdate(infoModel);
            }
            break;
        }
    }
}

//-----------------------------------------------------------------------------
/// Search for a process in the list that has profiling enabled.
/// \returns The process ID of the application currently being profiled.
/// zero is returned if a process with profiling enabled is not found.
//-----------------------------------------------------------------------------
DevDriver::ProcessId DeveloperPanelModel::FindProfileEnabledProcess()
{
    for (int processInfoIndex = 0; processInfoIndex < m_processInfoList.size(); ++processInfoIndex)
    {
        if (m_processInfoList[processInfoIndex].GetProfilingStatus())
        {
            return m_processInfoList[processInfoIndex].GetProcessId();
        }
    }
    return 0;
}

//-----------------------------------------------------------------------------
/// If necessary, update the ClientId for the incoming ProcessInfo.
/// \param processInfo The ProcessInfo for the process being filtered.
/// \returns True if the ClientId was updated for the process.
//-----------------------------------------------------------------------------
bool DeveloperPanelModel::TryUpdateClientId(ProcessInfoModel& processInfo)
{
    bool clientIdUpdated = false;

    // Step through the vector of info for each halted process that RDP has seen.
    for (int processIndex = 0; processIndex < m_processInfoList.size(); ++processIndex)
    {
        ProcessInfoModel& processInfoModel = m_processInfoList[processIndex];

        // Have we already seen the given process?
        const ClientId potentiallyNewClientId = processInfo.GetMostRecentClientId();
        if (processInfo.GetProcessId() == processInfoModel.GetProcessId())
        {
            // Is the process communicating with RDP using an updated ClientId?
            bool hasAlreadySeenClientId = processInfoModel.HasSeenClientId(potentiallyNewClientId);
            if (!hasAlreadySeenClientId)
            {
                processInfoModel.UpdateClientId(potentiallyNewClientId);
                processInfo = processInfoModel;
                clientIdUpdated = true;
                break;
            }
        }
    }

    return clientIdUpdated;
}

//-----------------------------------------------------------------------------
/// Filter each detected Developer Mode process. Apply any required settings
/// and attempt to resume them.
/// \param srcClientId The ClientId for the new client.
/// \param processInfo The info for the new process.
//-----------------------------------------------------------------------------
void DeveloperPanelModel::FilterHaltedProcess(const DevDriver::ClientId srcClientId, const ProcessInfoModel& processInfo)
{
    RDPUtil::DbgMsg("[RDP] Filtered halted process with ProcessId = %u", processInfo.GetProcessId());
    DevDriver::ProcessId profiledProcessId = FindProfileEnabledProcess();

    // Locate the matching processInfo in the vector and use this when making modifications.
    for (int processIndex = 0; processIndex < m_processInfoList.size(); ++processIndex)
    {
        ProcessInfoModel& processInfoFromList = m_processInfoList[processIndex];
        if (processInfoFromList.GetProcessId() == processInfo.GetProcessId())
        {
            // 1. If profiling should be enabled for this process, do it here.
            bool enabledSuccessfully = TryEnableProfiling(processInfoFromList);
            if ( enabledSuccessfully && ((profiledProcessId == 0) || (profiledProcessId == processInfo.GetProcessId())) )
            {
                RDPUtil::DbgMsg("[RDP] Set profiling flag for ProcessId = %u (client ID %u) to true.", processInfoFromList.GetProcessId(), srcClientId);
                processInfoFromList.SetProfilingStatus(srcClientId, true);
                emit ProfiledProcessInfoUpdate(processInfoFromList);
                // 2. Populate the ProcessInfo instance with driver settings data.
                PopulateProcessDriverSettings(processInfoFromList);

                // 3. Attempt to populate the global settings file for the first time if necessary.
                PopulateGlobalSettingsCache(processInfoFromList);

                // 4.  Apply global and user-defined driver setting overrides.
                ApplyDriverSettingOverrides(processInfoFromList);
            }
            else if (profiledProcessId != 0)
            {
                bool duplicateProcessName = false;

                int profiledProcessIndex = GetProcessInfoModelIndexByProcessId(profiledProcessId);

                Q_ASSERT(profiledProcessId != -1);

                if (profiledProcessIndex != -1)
                {
                    ProcessInfoModel& processBeingProfiled = m_processInfoList[profiledProcessIndex];
                    duplicateProcessName = processInfo.GetProcessName().compare(processBeingProfiled.GetProcessName()) == 0;

                    if (duplicateProcessName)
                    {
                        // Display a message box warning if multiple target app instances are detected.
                        emit MultipleProfilerTargetsStarted(processBeingProfiled);
                        RDPUtil::DbgMsg("[RGP] Unable to enable profiling for target executable '%s' with ProcessId %u.  ProcessId %u is already profiling.", processInfo.GetProcessName().toStdString().c_str(), processInfo.GetProcessId(), profiledProcessId);
                    }
                    else
                    {
                        emit ProfilerAlreadyInUse(processBeingProfiled);
                    }
                }
            }

            return;
        }
    }
}

//-----------------------------------------------------------------------------
/// Display the "GPU not supported" notification to the user.
/// In this case, the hardware is too old to collect valid traces.
//-----------------------------------------------------------------------------
void DeveloperPanelModel::OnDisplayUnsupportedASICNotification()
{
    RDPUtil::ShowNotification(gs_PROFILING_NOT_SUPPORTED_TITLE, gs_PROFILING_NOT_SUPPORTED_TEXT, NotificationWidget::Button::Ok);
}

//-----------------------------------------------------------------------------
/// Show the "Profiler is already in use" notificaiton popup.
/// \param processInfo The info for the process that is already being profiled.
//-----------------------------------------------------------------------------
void DeveloperPanelModel::OnProfilerAlreadyInUse(const ProcessInfoModel& processInfo)
{
    // Put up an warning message
    QString message = QString(gs_PROFILER_ALREADY_IN_USE_MSG).arg(processInfo.GetProcessName().toStdString().c_str()).arg(processInfo.GetProcessId());
    RDPUtil::ShowNotification(gs_PROFILER_ALREADY_IN_USE_TITLE, message, NotificationWidget::Button::Ok);
}

//-----------------------------------------------------------------------------
/// Slot to handle retrieving the process information for the process currently
/// being profiled.
/// \param processInfo The info for the profiled process.  If there is no process
///  being profiled, processInfo is set to an empty processInfoModel.
//-----------------------------------------------------------------------------
void DeveloperPanelModel::OnProfiledTargetInfoQuery(ProcessInfoModel& processInfo)
{
    static ProcessInfoModel processNone;
    for (int processInfoIndex = 0; processInfoIndex < m_processInfoList.size(); ++processInfoIndex)
    {
        if (m_processInfoList[processInfoIndex].GetProfilingStatus())
        {
            processInfo = m_processInfoList[processInfoIndex];
            return;
        }
    }
    processInfo = processNone;
}

//-----------------------------------------------------------------------------
/// Apply the global RDP settings and user defined application overrides.
/// \param processInfo The halted process to apply the driver settings to.
//-----------------------------------------------------------------------------
bool DeveloperPanelModel::ApplyDriverSettingOverrides(const ProcessInfoModel& processInfo)
{
    bool appliedChanges = false;

    // Step through each Target Executable row, and check if the executable name matches the new halted process.
    QAbstractItemModel* pTargetExecutableTable = m_pTargetApplicationModel->GetTableModel();
    int numRows = pTargetExecutableTable->rowCount();

    for (int targetRow = 0; targetRow < numRows; ++targetRow)
    {
        if (m_pTargetApplicationModel->IsExecutableMatchingAtRow(targetRow, processInfo.GetProcessName()) == true)
        {

            QModelIndex enableProfilingIndex = pTargetExecutableTable->index(targetRow, TARGET_APPLICATION_TABLE_COLUMN_APPLY_SETTINGS);
            const QVariant& checkState = pTargetExecutableTable->data(enableProfilingIndex, Qt::CheckStateRole);

            // Apply driver setting changes only if the user has specified that they should be applied when seen.
            if (checkState == Qt::Checked)
            {
                // Determine the set of driver settings will be updated using RDP's global overrides.
                DriverSettingsMap globalOverridesMap;
                GetOverriddenSettings(processInfo, globalOverridesMap);

                if (!globalOverridesMap.empty())
                {
                    ChannelContext& channelContext = GetChannelContext();
                    if (channelContext.pClient == nullptr)
                    {
                        return false;
                    }

                    SettingsClient* pSettingsClient = static_cast<SettingsClient*>(channelContext.pClient->AcquireProtocolClient<DevDriver::Protocol::Settings>());
                    if (pSettingsClient == nullptr)
                    {
                        return false;
                    }

                    // Convert to a cstring so we can print it.
                    std::string processName = processInfo.GetProcessName().toStdString();
                    const char* pProcessName = processName.c_str();

                    const ClientId currentClientId = processInfo.GetMostRecentClientId();
                    RDPUtil::DbgMsg("[RDP] Attempting to apply app-specific settings for Process %d %s, currentCLientId %d", processInfo.GetProcessId(), pProcessName, currentClientId);
                    DevDriver::Result connectResult = pSettingsClient->Connect(currentClientId);
                    if (connectResult == Result::Success)
                    {
                        appliedChanges = ApplySettingsMap(pSettingsClient, globalOverridesMap);
                        pSettingsClient->Disconnect();
                    }

                    // Release the SettingsClient after applying required settings.
                    channelContext.pClient->ReleaseProtocolClient(pSettingsClient);

                    if (!appliedChanges)
                    {
                        RDPUtil::DbgMsg("[RDP] Failed to apply application setting overrides to %s.", pProcessName);
                    }
                }
            }
        }
    }

    return appliedChanges;
}

//-----------------------------------------------------------------------------
/// Populate the incoming ProcessInfo instance with driver settings info.
/// \param processInfo The process to populate driver settings for.
//-----------------------------------------------------------------------------
void DeveloperPanelModel::PopulateProcessDriverSettings(ProcessInfoModel& processInfo)
{
    DriverSettingsMap driverSettings;
    GetProcessDriverSettings(processInfo, driverSettings);

    processInfo.SetDriverSettings(driverSettings);
}

//-----------------------------------------------------------------------------
/// Populate the global settings cache if necessary.
/// \param processInfo The processInfo for the client to use to collect the global settings data.
//-----------------------------------------------------------------------------
void DeveloperPanelModel::PopulateGlobalSettingsCache(ProcessInfoModel& processInfo)
{
    // Retrieve the global application settings file. These are the default settings chosen by the user.
    ApplicationSettingsModel* pGlobalAppSettings = m_pPanelSettingsModel;
    if (pGlobalAppSettings != nullptr)
    {
        ApplicationSettingsFile* pSettingsFile = pGlobalAppSettings->GetSettingsFile();
        Q_ASSERT(pSettingsFile != nullptr);

        // Is the global application settings empty? It might be initializing for the first time.
        // @TODO: This should ultimately work in the following way:
        // 1. Retrieve the first set of settings.
        // 2. If we have cached global settings on disk, compare them to the set that were just retrieved.
        // 3. Update the cached global settings with additional settings/changes in type/etc for next time.
        const DriverSettingsMap& driverSettings = pSettingsFile->GetDriverSettings();
        if (driverSettings.empty())
        {
            // Copy the process's driver settings into the global settings cache for initial population.
            DriverSettingsModel* pDriverSettingsModel = pGlobalAppSettings->GetDriverSettingsModel();

            // Update the global driver settings using the ProcessInfo's settings map.
            pDriverSettingsModel->UpdateDriverSettings(processInfo.GetDriverSettings());

            // Once the global settings have been populated, save the file to disk for next time.
            RDPSettings::Get().WriteApplicationSettingsFile(pSettingsFile);

            // Let the main window know that the global settings cache has been updated.
            emit DriverSettingsPopulated(0);
        }
    }
}

//-----------------------------------------------------------------------------
/// Retrieve a map of driver settings based on the incoming ProcessInfo instance.
/// \param processInfo The process to collect DriverSettings for.
/// \param driverSettings The map of driver settings collected from the process.
//-----------------------------------------------------------------------------
void DeveloperPanelModel::GetProcessDriverSettings(const ProcessInfoModel& processInfo, DriverSettingsMap& driverSettings)
{
    QVector<SettingCategory> categories;

    ChannelContext& channelContext = GetChannelContext();
    if (channelContext.pClient != nullptr)
    {
        SettingsClient* pSettingsClient = static_cast<SettingsClient*>(channelContext.pClient->AcquireProtocolClient<DevDriver::Protocol::Settings>());
        if (pSettingsClient != nullptr)
        {
            const ClientId currentClientId = processInfo.GetMostRecentClientId();

            // Attempt to connect the SettingsClient to RDS.
            DevDriver::Result connectResult = pSettingsClient->Connect(currentClientId);
            if (connectResult == Result::Success)
            {
                uint32 numCategories = 0;
                Result result = pSettingsClient->QueryNumCategories(&numCategories);
                if (result == Result::Success && numCategories > 0)
                {
                    SettingCategory* pCategories = new SettingCategory[numCategories];
                    result = pSettingsClient->QueryCategories(pCategories, numCategories);
                    if (result == Result::Success)
                    {
                        for (size_t index = 0; index < numCategories; ++index)
                        {
                            // Add a new SettingTreeNode for every new setting category.
                            SettingCategory& currentCategory = pCategories[index];
                            categories.push_back(currentCategory);
                        }
                    }
                }

                // Ask for the entire list of settings, but only display a subset based on the selected category.
                uint32 numSettings = 0;
                Result numSettingsResult = pSettingsClient->QueryNumSettings(&numSettings);
                if (numSettingsResult == Result::Success)
                {
                    if (numSettings > 0)
                    {
                        SettingsProtocol::Setting* pSettings = new SettingsProtocol::Setting[numSettings];
                        result = pSettingsClient->QuerySettings(pSettings, numSettings);
                        if (result == Result::Success)
                        {
                            // Add a new SettingTreeNode for every new setting category.
                            for (uint32 settingIndex = 0; settingIndex < numSettings; ++settingIndex)
                            {
                                SettingsProtocol::Setting* pCurrentSetting = pSettings + settingIndex;

                                SettingCategory& category = categories[pCurrentSetting->categoryIndex];
                                const QString categoryString = QString(category.name);

                                // Add the setting to the output map.
                                driverSettings[categoryString].push_back(*pCurrentSetting);
                            }
                        }
                        else
                        {
                            RDPUtil::DbgMsg("[RDP] Failed to query driver settings.");
                        }

                        // Destroy the settings array, since they were copied into the ProcessInfo.
                        SAFE_DELETE_ARRAY(pSettings);
                    }
                    else
                    {
                        RDPUtil::DbgMsg("[RDP] Found 0 settings.");
                    }
                }
                else
                {
                    RDPUtil::DbgMsg("[RDP] Failed to query number of driver settings.");
                }

                pSettingsClient->Disconnect();
            }
            else
            {
                RDPUtil::DbgMsg("[RDP] Failed to connect SettingsClient to query driver settings.");
            }

            // Release the SettingsClient after applying required settings.
            channelContext.pClient->ReleaseProtocolClient(pSettingsClient);
        }
    }
}

//-----------------------------------------------------------------------------
/// This function checks if the new halted process should have profiling enabled. If
/// it should be enabled, attempt to do so.
/// \param processInfo The processInfo of the new halted process.
/// \returns True if profiling was enabled, and false if it didn't need to be enabled, or failed.
//-----------------------------------------------------------------------------
bool DeveloperPanelModel::TryEnableProfiling(const ProcessInfoModel& processInfo)
{
    bool profilingEnabled = false;

    DevDriver::ProcessId profiledProcessId = FindProfileEnabledProcess();
    if ( (profiledProcessId != 0) && (processInfo.GetProcessId() != profiledProcessId) )
    {
        return false;
    }

    // Step through each Target Executable row, and check if the executable name matches the new halted process.
    QAbstractItemModel* pTargetExecutableTable = m_pTargetApplicationModel->GetTableModel();
    int numRows = pTargetExecutableTable->rowCount();

    const QString processName = processInfo.GetProcessName();
    for (int targetRow = 0; targetRow < numRows; ++targetRow)
    {
        if (m_pTargetApplicationModel->IsExecutableMatchingAtRow(targetRow, processName) == true)
        {
            QModelIndex enableProfilingIndex = pTargetExecutableTable->index(targetRow, TARGET_APPLICATION_TABLE_COLUMN_ENABLE_PROFILING);
            const QVariant& checkState = pTargetExecutableTable->data(enableProfilingIndex, Qt::CheckStateRole);

            if (checkState == Qt::Checked)
            {
                // Use the last known ClientId to connect the ApplicationSettingsModel clients internally.
                const ClientId lastClientId = processInfo.GetMostRecentClientId();

                // If profiling was enabled successfully for the incoming process,
                // update the Panel model with the process's ClientId and process name.
                m_pPanelSettingsModel->SetConnectedClientId(lastClientId);
                m_pPanelSettingsModel->SetConnectedProcessName(processInfo.GetProcessName());

                // Enable profiling as early as possible in the global model.
                profilingEnabled = EnableProfiling(processInfo);
                if (profilingEnabled == false)
                {
                    // Supposed to enable profiling for the app, but it's not supported.
                    emit DisplayUnsupportedASICNotification();
                    RDPUtil::DbgMsg("[RDP] Can't enable profiling for client ID %d", lastClientId);
                }
                else
                {
                    RDPUtil::DbgMsg("[RDP] Can enable profiling for client ID %d", lastClientId);
                }

                break;
            }
        }
    }

    return profilingEnabled;
}

//-----------------------------------------------------------------------------
/// Enable RGP profiling for the halted process with the given clientId.
/// \param processInfo The processInfo of the halted process to resume.
/// \returns True if profiling was enabled, and false if it failed.
//-----------------------------------------------------------------------------
bool DeveloperPanelModel::EnableProfiling(const ProcessInfoModel& processInfo)
{
    using namespace DevDriver::RGPProtocol;

    Result profilingEnabledResult = Result::Error;

    ChannelContext& channelContext = GetChannelContext();
    if (channelContext.pClient != nullptr)
    {
        RGPClient* pRgpClient = static_cast<RGPClient*>(channelContext.pClient->AcquireProtocolClient<DevDriver::Protocol::RGP>());
        if (pRgpClient != nullptr)
        {
            const ClientId currentClientId = processInfo.GetMostRecentClientId();

            DevDriver::Result connectResult = pRgpClient->Connect(currentClientId);
            if (connectResult == Result::Success)
            {
                RDPUtil::DbgMsg("[RDP] EnableProfiling() for clientId %d.", currentClientId);

                DevDriver::RGPProtocol::ProfilingStatus profilingStatus = DevDriver::RGPProtocol::ProfilingStatus::NotAvailable;
                if (pRgpClient->QueryProfilingStatus(&profilingStatus) == DevDriver::Result::Success)
                {
                    if (profilingStatus == DevDriver::RGPProtocol::ProfilingStatus::Enabled)
                    {
                        RDPUtil::DbgMsg("[RDP] Profiling is already enabled on client id %u.", currentClientId);
                    }
                    else if (profilingStatus == DevDriver::RGPProtocol::ProfilingStatus::Available)
                    {
                        profilingEnabledResult = pRgpClient->EnableProfiling();
                        pRgpClient->Disconnect();
                    }
                }
            }

            // Release the client after attempting to enable profiling.
            channelContext.pClient->ReleaseProtocolClient(pRgpClient);
        }
    }

    bool profilingEnabled = (profilingEnabledResult == Result::Success);
    if (profilingEnabled)
    {
        std::string processName = processInfo.GetProcessName().toStdString();
        RDPUtil::DbgMsg("[RDP] Enabled profiling for target executable '%s', ProcessId = %u.", processName.c_str(), processInfo.GetProcessId());
    }
    return profilingEnabled;
}

//-----------------------------------------------------------------------------
/// Collect the map of global driver settings changes to apply.
/// \param processInfo The processInfo of the halted process to resume.
/// \param globalOverrides The map that overidden settings will be added to.
//-----------------------------------------------------------------------------
void DeveloperPanelModel::GetOverriddenSettings(const ProcessInfoModel& processInfo, DriverSettingsMap& globalOverrides)
{
    // Retrieve the global application settings file. These are the global setting overrides chosen by the user.
    ApplicationSettingsModel* pGlobalAppSettings = m_pPanelSettingsModel;
    if (pGlobalAppSettings != nullptr)
    {
        ApplicationSettingsFile* pSettingsFile = pGlobalAppSettings->GetSettingsFile();
        Q_ASSERT(pSettingsFile != nullptr);

        const DriverSettingsMap& processSettings = processInfo.GetDriverSettings();
        pSettingsFile->GetSettingsMapDelta(processSettings, globalOverrides);
    }
}

//-----------------------------------------------------------------------------
/// Apply the given map of settings using the given client.
/// \param pSettingsClient The settings client used to apply settings.
/// \param driverSettingsMap The map of settings to apply with the client.
/// \returns True if the settings were applied successfully, and false if they weren't.
//-----------------------------------------------------------------------------
bool DeveloperPanelModel::ApplySettingsMap(DevDriver::SettingsProtocol::SettingsClient* pSettingsClient, const DriverSettingsMap& driverSettingsMap)
{
    bool settingsApplied = true;

    DriverSettingsMap::const_iterator categoryIter;
    DriverSettingsMap::const_iterator firstCategoryIter = driverSettingsMap.begin();
    DriverSettingsMap::const_iterator lastCategoryIter = driverSettingsMap.end();
    for (categoryIter = firstCategoryIter; categoryIter != lastCategoryIter; ++categoryIter)
    {
        const QString categoryString = categoryIter.key();
        const DriverSettingVector& settingsVector = categoryIter.value();

        DriverSettingVector::const_iterator settingIter;
        for (settingIter = settingsVector.begin(); settingIter != settingsVector.end(); ++settingIter)
        {
            const SettingValue& pendingValue = (*settingIter).value;

            const QString& settingName = settingIter->name;
            std::string settingNameString = settingName.toStdString();
            const char* pSettingName = settingNameString.c_str();
            Result setResult = pSettingsClient->SetSetting(pSettingName, &pendingValue);
            if (setResult != Result::Success)
            {
                // If a single setting fails to be set, consider the whole thing failed.
                settingsApplied = false;
            }
            else
            {
                RDPUtil::DbgMsg("[RDP] Setting '%s' applied successfully", pSettingName);
            }
        }
    }

    return settingsApplied;
}

//-----------------------------------------------------------------------------
/// Apply global and application-specific settings to the given halted process.
/// \param processInfo The processInfo of the halted process to resume.
/// \returns True if settings were applied successfully, and false if it failed.
//-----------------------------------------------------------------------------
#if 0
bool DeveloperPanelModel::ApplyApplicationSettings(const ProcessInfoModel& processInfo, ApplicationSettingsModel* pSettingsModel)
{
    bool appliedGlobalSettings = false;

    // Retrieve the global application settings file. These are the default settings chosen by the user.
    if (pSettingsModel != nullptr)
    {
        ApplicationSettingsFile* pSettingsFile = pSettingsModel->GetSettingsFile();
        Q_ASSERT(pSettingsFile != nullptr);

        const DriverSettingsMap& applicationSettingsMap = pSettingsFile->GetDriverSettings();
        if (!applicationSettingsMap.empty())
        {
            // Convert to a cstring so we can print it.
            std::string processName = processInfo.GetProcessName().toStdString();
            const char* pProcessName = processName.c_str();

            ApplicationSettingsFile* pGlobalSettingsFile = m_pPanelSettingsModel->GetSettingsFile();
            Q_ASSERT(pGlobalSettingsFile != nullptr);

            DriverSettingsMap differentSettings;
            bool hasDifferences = pGlobalSettingsFile->GetSettingsDelta(pSettingsFile, differentSettings);
            if (hasDifferences)
            {
                ChannelContext& channelContext = GetChannelContext();
                if (channelContext.pClient == nullptr)
                {
                    return false;
                }

                SettingsClient* pSettingsClient = static_cast<SettingsClient*>(channelContext.pClient->AcquireProtocolClient<DevDriver::Protocol::Settings>());
                if (pSettingsClient == nullptr)
                {
                    return false;
                }

                RDPUtil::DbgMsg("[RDP] Attempting to apply app-specific settings for Process %d %s", processInfo.GetProcessId(), pProcessName);
                const ClientId currentClientId = processInfo.GetMostRecentClientId();

                DevDriver::Result connectResult = pSettingsClient->Connect(currentClientId);
                if (connectResult == Result::Success)
                {
                    appliedGlobalSettings = ApplySettingsMap(pSettingsClient, differentSettings);

                    if (!appliedGlobalSettings)
                    {
                        RDPUtil::DbgMsg("[RDP] Failed to apply application setting overrides to %s.", pProcessName);
                    }
                }
            }
            else
            {
                RDPUtil::DbgMsg("[RDP] No application-specific settings to apply to Process %d %s.", processInfo.GetProcessId(), pProcessName);
            }
        }
    }

    return appliedGlobalSettings;
}
#endif // 0

//-----------------------------------------------------------------------------
/// Connect a driver control client to the new process.
/// \param newProcessInfo The ProcessInfo  for the new process.
/// \returns A driver control client connected to the given process, or null if it failed to connect.
//-----------------------------------------------------------------------------
DriverControlClient* DeveloperPanelModel::ConnectDriverControlClient(const ProcessInfoModel& processInfo)
{
    using namespace DevDriver::DriverControlProtocol;

    DriverControlClient* pDriverControlClient = nullptr;

    ChannelContext& channelContext = GetChannelContext();
    if (channelContext.pClient != nullptr)
    {
        pDriverControlClient = static_cast<DriverControlClient*>(channelContext.pClient->AcquireProtocolClient<DevDriver::Protocol::DriverControl>());
        DD_ASSERT(pDriverControlClient != nullptr);
        if (pDriverControlClient != nullptr)
        {
            const ClientId currentClientId = processInfo.GetMostRecentClientId();

            DevDriver::Result connectResult = pDriverControlClient->Connect(currentClientId);
            if (connectResult != Result::Success)
            {
                // The client failed to connect to the given process. Release the client and return null - the client allocation is managed internally for us.
                channelContext.pClient->ReleaseProtocolClient(pDriverControlClient);
                pDriverControlClient = nullptr;
            }
        }
    }

    // Stringify the process name so we can print in the DbgMsg below.
    const std::string& processNameString = processInfo.GetProcessName().toStdString();

    if (pDriverControlClient == nullptr)
    {
        RDPUtil::DbgMsg("[RDP] Failed to connect DriverControlClient to process '%s', ProcessId = %u", processNameString.c_str(), processInfo.GetProcessId());
    }
    else
    {
        RDPUtil::DbgMsg("[RDP] Connected DriverControlClient to process '%s', ProcessId = %u", processNameString.c_str(), processInfo.GetProcessId());
    }

    return pDriverControlClient;
}

//-----------------------------------------------------------------------------
/// Disconnect the given DriverControlClient.
/// \param pDriverControlClient The DriverControlClient instance to release.
//-----------------------------------------------------------------------------
void DeveloperPanelModel::DisconnectDriverControlClient(DriverControlClient* pDriverControlClient)
{
    bool validClient = pDriverControlClient != nullptr;
    Q_ASSERT(validClient);
    if (validClient)
    {
        if (pDriverControlClient->IsConnected())
        {
            pDriverControlClient->Disconnect();
            ChannelContext& channelContext = GetChannelContext();
            if (channelContext.pClient != nullptr)
            {
                channelContext.pClient->ReleaseProtocolClient(pDriverControlClient);
            }
        }
        else
        {
            RDPUtil::DbgMsg("[RDP] Attempted to disconnect from DriverControlClient that was already disconnected.");
        }
    }
    else
    {
        RDPUtil::DbgMsg("[RDP] Attempted to disconnect from invalid DriverControlClient.");
    }
}

//-----------------------------------------------------------------------------
/// Resume a halted process.
/// \param processInfo The processInfo of the halted process to resume.
/// \returns True if the process was resumed successfully, and false if it failed.
//-----------------------------------------------------------------------------
bool DeveloperPanelModel::ResumeHaltedProcess(DriverControlClient* pDriverControlClient, const ProcessInfoModel& processInfo)
{
    bool resumeSuccessful = false;

    if (pDriverControlClient != nullptr)
    {
        // Resume the application before executing a trace.
        if (pDriverControlClient->IsConnected())
        {
            Result resumeResult = pDriverControlClient->ResumeDriver();
            if (resumeResult == Result::Success)
            {
                RDPUtil::DbgMsg("[RDP] Resumed execution of process '%s', ProcessId = %u. Disconnect client.", processInfo.GetProcessName().toStdString().c_str(), processInfo.GetProcessId());
                resumeSuccessful = true;
            }
            else if (resumeResult == Result::NotReady)
            {
                RDPUtil::DbgMsg("[RDP] Resume driver timed out on client");
            }
            else
            {
                RDPUtil::DbgMsg("[RDP] Failed to resume driver on client");
            }
        }
        else
        {
            RDPUtil::DbgMsg("[RDP] ResumeHaltedProcess failed as DriverControlClient is not connected");
        }
    }

    if (!resumeSuccessful)
    {
        RDPUtil::DbgMsg("[RDP] Failed to resume execution of process '%s', ProcessId = %u", processInfo.GetProcessName().toStdString().c_str(), processInfo.GetProcessId());
    }

    return resumeSuccessful;
}

//-----------------------------------------------------------------------------
/// Wait for the application's driver to be fully initialized.
/// \param pDriverControlClient The DriverControlClient used to wait for the driver to initialize.
/// \param processInfo The ProcessInfoModel instance to update the state for.
/// \returns The error code returned from the attempt to wait for driver initialization.
//-----------------------------------------------------------------------------
DevDriver::Result DeveloperPanelModel::WaitForDriverInitialization(DevDriver::DriverControlProtocol::DriverControlClient* pDriverControlClient, ProcessInfoModel& processInfo)
{
    const QString& processName = processInfo.GetProcessName();
    std::string processNameString = processName.toStdString();

    DevDriver::Result initResult = pDriverControlClient->WaitForDriverInitialization(kDriverInitializationTimeoutMilliseconds);
    if (initResult != Result::Success)
    {
        switch (initResult)
        {
        case Result::VersionMismatch:
            {
                RDPUtil::DbgMsg("[RDP] Wait for driver initialization not supported on target client %s's protocol version.", processNameString.c_str());
            }
            break;
        case Result::Unavailable:
            {
                RDPUtil::DbgMsg("[RDP] Wait for driver initialization not available on target client %s.", processNameString.c_str());
            }
            break;
        case Result::NotReady:
            {
                RDPUtil::DbgMsg("[RDP] Wait for driver initialization timed out on target client %s.", processNameString.c_str());
            }
            break;
        case Result::Error:
            {
                RDPUtil::DbgMsg("[RDP] Wait for driver initialization in process '%s' failed.", processNameString.c_str());
            }
            break;
        default:
            {
                Q_ASSERT(false);
                QString resultString = ToolUtil::GetResultString(initResult);
                std::string stdResultString = resultString.toStdString();
                RDPUtil::DbgMsg("[RDP] Wait for driver initialization failed on target client %s with code '%s'.", processNameString.c_str(), stdResultString.c_str());
            }
        }
    }
    else
    {
        processInfo.SetDriverInitializedStatus(true);
        emit UpdateDriverInitializedStatus(processInfo, true);
        RDPUtil::DbgMsg("[RDP] Driver initialized successfully on target client %s.", processNameString.c_str());
    }

    return initResult;
}

//-----------------------------------------------------------------------------
/// Check if RDP has already discovered the process with the given ClientId.
/// \param clientId The client Id for the process.
/// \returns True if the process is already known, and false if it isn't.
//-----------------------------------------------------------------------------
bool DeveloperPanelModel::HasProcessInfo(const ProcessInfoModel& incomingProcessInfo)
{
    bool alreadySeenProcess = false;

    // Step through the vector of info for each halted process that RDP has seen.
    for (int processIndex = 0; processIndex < m_processInfoList.size(); ++processIndex)
    {
        ProcessInfoModel& processInfo = m_processInfoList[processIndex];

        // Have we already seen the given process?
        const ClientId potentiallyNewClientId = incomingProcessInfo.GetMostRecentClientId();
        if (processInfo.GetProcessId() == incomingProcessInfo.GetProcessId())
        {
            // We've already seen the process.
            alreadySeenProcess = true;
            break;
        }
    }

    return alreadySeenProcess;
}

//-----------------------------------------------------------------------------
/// Register a driver protocol model with the model map.
/// \param modelType The type of model being registered.
/// \param pDriverModel A pointer to the model with the given type.
//-----------------------------------------------------------------------------
void DeveloperPanelModel::RegisterProtocolModel(MainPanelModels modelType, DriverProtocolModel* pDriverModel)
{
    m_modelMap.insert({(uint8_t)modelType, pDriverModel});
}

//-----------------------------------------------------------------------------
/// Unregister the model with the given type. The model will be destroyed elsewhere.
/// \param modelType The model type to unregister from the main panel model.
//-----------------------------------------------------------------------------
bool DeveloperPanelModel::UnregisterModel(MainPanelModels modelType)
{
    PanelModelMap::iterator modelIter = m_modelMap.find(modelType);
    if (modelIter != m_modelMap.end())
    {
        m_modelMap.erase(modelIter);
        return true;
    }

    RDPUtil::DbgMsg("[RDP] Failed to unregister protocol model with type %d", modelType);
    return false;
}


//-----------------------------------------------------------------------------
/// Retrieve a registered model instance based on the incoming model type.
/// \param modelType The type of model instance to retrieve.
/// \returns A registered model instance of the requested type, or nullptr if there isn't one.
//-----------------------------------------------------------------------------
DriverProtocolModel* DeveloperPanelModel::GetProtocolModel(MainPanelModels modelType)
{
    PanelModelMap::iterator modelIter = m_modelMap.find(modelType);
    if (modelIter != m_modelMap.end())
    {
        return modelIter->second;
    }
    return nullptr;
}

//-----------------------------------------------------------------------------
/// Add a new application settings instance to the Panel model.
/// \returns A new Application Settings model instance.
//-----------------------------------------------------------------------------
ApplicationSettingsModel* DeveloperPanelModel::AddNewApplicationSettings(ApplicationSettingsFile* pAppSettingsFile)
{
    ApplicationSettingsModel* pSettingsModel = nullptr;
    RDPApplicationSettingsFile* pApplicationSettingsFileInfo = nullptr;

    // Create a new settings file on disk for this application.
    if (pAppSettingsFile == nullptr)
    {
        // Create a new application settings file if this is a new tab.
        pApplicationSettingsFileInfo = RDPSettings::Get().CreateAppSettingsFile();
        RDPSettings::Get().SaveSettings();
    }
    else
    {
        pApplicationSettingsFileInfo = pAppSettingsFile->GetFileInfo();
    }

    if (pApplicationSettingsFileInfo != nullptr)
    {
        pSettingsModel = new ApplicationSettingsModel(this, APPLICATION_SETTINGS_CONTROLS_COUNT);
        pSettingsModel->InitializeFromFile(pApplicationSettingsFileInfo);

        ApplicationSettingsFile* pSettingsFile = pSettingsModel->GetSettingsFile();
        if (pSettingsFile != nullptr)
        {
            pSettingsFile->SetFileInfo(pApplicationSettingsFileInfo);
        }

        m_pPanelSettingsModel = pSettingsModel;
    }

    Q_ASSERT(m_pPanelSettingsModel != nullptr);

    return pSettingsModel;
}

//-----------------------------------------------------------------------------
/// Retrieve the list index of the ProcessInfoModel based on the provided ProcessId.
/// \param processId The process Id to search for in the list of known processes.
/// \returns The Index within m_processInfoList for the process with the given ProcessId.
//-----------------------------------------------------------------------------
int DeveloperPanelModel::GetProcessInfoModelIndexByProcessId(DevDriver::ProcessId processId) const
{
    for (int processIndex = 0; processIndex < m_processInfoList.size(); ++processIndex)
    {
        if (m_processInfoList[processIndex].GetProcessId() == processId)
        {
            return processIndex;
        }
    }

    return -1;
}

//-----------------------------------------------------------------------------
/// Get a pointer to URI protocol client
/// \return a valid pointer, else null
//-----------------------------------------------------------------------------
DevDriver::URIProtocol::URIClient* DeveloperPanelModel::GetUriClient()
{
    DevDriver::URIProtocol::URIClient* pClient = nullptr;

    ChannelContext& channelContext = GetChannelContext();
    if (channelContext.pClient != nullptr)
    {
        DevDriver::IMsgChannel* pMessageChannel = channelContext.pClient->GetMessageChannel();

        if (pMessageChannel != nullptr)
        {
            using namespace DevDriver::URIProtocol;
            pClient = static_cast<DevDriver::URIProtocol::URIClient*>(channelContext.pClient->AcquireProtocolClient<DevDriver::Protocol::URI>());
        }
    }

    return pClient;
}
