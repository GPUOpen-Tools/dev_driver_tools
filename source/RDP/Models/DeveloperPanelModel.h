//=============================================================================
/// Copyright (c) 2017-2018 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief The Developer Panel Model used to communicate with the Radeon Developer Service.
//=============================================================================

#ifndef _DEVELOPER_PANEL_MODEL_H_
#define _DEVELOPER_PANEL_MODEL_H_

#include <QObject>
#include <QVector>
#include <QSharedPointer>
#include <QString>

#include "ProcessInfoModel.h"
#include "../AppSettings/ApplicationSettingsFile.h"
#include "../../Common/DriverToolsDefinitions.h"
#include "../../Common/ddMemAlloc.h"
#include "../../Common/ModelViewMapper.h"
#include "../../DevDriverComponents/inc/protocolClient.h"
#include "../../DevDriverComponents/inc/devDriverClient.h"
#include "../../DevDriverComponents/inc/protocols/settingsClient.h"
#include <unordered_map>

namespace DevDriver
{
    class DevDriverClient;
    class BaseProtocolClient;

    namespace DriverControlProtocol
    {
        class DriverControlClient;
    }
}

class ApplicationSettingsFile;
class ApplicationSettingsModel;
class DriverMessageProcessorThread;
class DriverProtocolModel;
class SetupTargetApplicationModel;
class ConnectionStatusWorker;

/// An enum listing the main models controlled by this Developer Panel model.
enum MainPanelModels
{
    MAIN_PANEL_MODEL_CONNECTION_SETTINGS,
    MAIN_PANEL_MODEL_DRIVER_CONTROL,
    MAIN_PANEL_MODEL_DRIVER_SETTINGS,
    MAIN_PANEL_MODEL_DRIVER_LOGGING,
    MAIN_PANEL_MODEL_RGP,
};

/// The custom allocator to use for all RDP memory allocations.
static const DevDriver::AllocCb GenericAllocCb =
{
    nullptr,
    &ddMemAlloc::GenericAlloc,
    &ddMemAlloc::GenericFree
};

/// Struct that represents a named protocol client
struct ProtocolClientEntry
{
    char name[256];                                 ///< The name string of the protocol client.
    DevDriver::IProtocolClient* pProtocolClient;    ///< A pointer to the driver protocol client.
};

/// Struct to track message channel data, will be passed to message channel create to be used as our context.
struct ChannelContext
{
    DevDriver::DevDriverClient* pClient;    ///< Pointer to the DevDriverClient used for communication.
    DevDriver::ClientId connectedClientId;  ///< The client Id of the client the panel is connected to.
    bool exitRequested;                     ///< A flag that indicates if RDP is intending to shut down.
};

/// A vector of Application-specific settings models.
typedef QVector<ApplicationSettingsModel*> AppSettingsModelVector;

/// Class to hold and control the Developer Panel's main communication channel.
class DeveloperPanelModel : public QObject
{
    Q_OBJECT
public:
    DeveloperPanelModel();
    virtual ~DeveloperPanelModel();
    bool InitializeConnectionToRDS();
    bool TerminateConnectedRDS();
    void Disconnect();
    void AddClientId(DevDriver::ClientId srcClientId);
    void AddClientInfo(DevDriver::ClientId srcClientId, const QString& processName, DevDriver::ProcessId processId, const QString& clientDescription);
    void ClientDisconnected(DevDriver::ClientId srcClientId);
    void FilterHaltedProcess(const DevDriver::ClientId srcClientId, const ProcessInfoModel& processInfo);
    bool ApplyDriverSettingOverrides(const ProcessInfoModel& processInfo);
    void PopulateProcessDriverSettings(ProcessInfoModel& processInfo);
    void PopulateGlobalSettingsCache(ProcessInfoModel& processInfo);
    void GetProcessDriverSettings(const ProcessInfoModel& processInfo, DriverSettingsMap& driverSettings);

    bool TryEnableProfiling(const ProcessInfoModel& processInfo);
    bool EnableProfiling(const ProcessInfoModel& processInfo);
    void GetOverriddenSettings(const ProcessInfoModel& processInfo, DriverSettingsMap& globalOverrides);
    bool ApplySettingsMap(DevDriver::SettingsProtocol::SettingsClient* pSettingsClient, const DriverSettingsMap& driverSettingsMap);
    bool ApplyApplicationSettings(const ProcessInfoModel& processInfo, ApplicationSettingsModel* pSettingsModel);
    bool ResumeHaltedProcess(DevDriver::DriverControlProtocol::DriverControlClient* pDriverControlClient, const ProcessInfoModel& processInfo);
    DevDriver::Result WaitForDriverInitialization(DevDriver::DriverControlProtocol::DriverControlClient* pDriverControlClient, ProcessInfoModel& processInfo);
    DevDriver::DriverControlProtocol::DriverControlClient* ConnectDriverControlClient(const ProcessInfoModel& processInfo);
    void DisconnectDriverControlClient(DevDriver::DriverControlProtocol::DriverControlClient* pDriverControlClient);
    bool HasProcessInfo(const ProcessInfoModel& processInfo);
    bool TryUpdateClientId(ProcessInfoModel& processInfo);
    DevDriver::ProcessId FindProfileEnabledProcess();
    ChannelContext& GetChannelContext() { return m_channelContext; }
    void RegisterProtocolModel(MainPanelModels modelType, DriverProtocolModel* pDriverModel);
    bool UnregisterModel(MainPanelModels modelType);
    DevDriver::URIProtocol::URIClient* GetUriClient();

    DriverProtocolModel* GetProtocolModel(MainPanelModels modelType);
    bool IsConnectedToRDS() const { return m_connectedToRds; }

    ApplicationSettingsModel* AddNewApplicationSettings(ApplicationSettingsFile* pAppSettingsFile = nullptr);

    void SetTargetApplicationsModel(SetupTargetApplicationModel* targetApplicationModel);

public slots:
    void OnDisplayUnsupportedASICNotification();
    void OnProfiledTargetInfoQuery(ProcessInfoModel& processInfo);
    void OnProfilerAlreadyInUse(const ProcessInfoModel& processInfo);

signals:
    void DisplayUnsupportedASICNotification();
    void UpdateClientRunStatus(const ProcessInfoModel& processInfo, bool isActive);
    void UpdateDriverInitializedStatus(const ProcessInfoModel& processInfo, bool isInitialized);
    void ConnectedToHaltedTargetApplication(ApplicationSettingsModel* pSettingsModel);
    void DriverSettingsPopulated(int modelIndex);
    void ProfiledProcessInfoUpdate(const ProcessInfoModel& processInfo);
    void MultipleProfilerTargetsStarted(const ProcessInfoModel& profiledProcessInfo);
    void ProfilerAlreadyInUse(const ProcessInfoModel& profiledProcessInfo);
    void Connected();
    void Disconnected();

private:
    typedef std::unordered_map<uint8_t, DriverProtocolModel*> PanelModelMap;    ///< A helper used to provide a generic way to retrieve driver models.
    int GetProcessInfoModelIndexByProcessId(DevDriver::ProcessId processId) const;

    PanelModelMap m_modelMap;                                  ///< A map to associate a driver model type with a model instance.
    ChannelContext m_channelContext;                           ///< Structure used to store service connection information.
    bool m_connectedToRds;                                     ///< A flag used to check if there's an active connection to an RDS instance.

    QThread* m_pMessageProcessorThread;                        ///< The QThread responsible for processing new DevDriver messages.
    QThread* m_pConnectionStatusThread;                        ///< The QThread responsible for checking the connection status with RDS.
    DriverMessageProcessorThread* m_pMessageProcessorWorker;   ///< The worker which processes DevDriver messages.
    ConnectionStatusWorker* m_pConnectionStatusWorker;         ///< The worker which performs connection status checking.
    QVector<ProcessInfoModel> m_processInfoList;               ///< A vector of info structures retrieved from each known process.
    QVector<DevDriver::ClientId> m_knownClientIdList;          ///< A list of client Id's discovered through ping responses.

    ApplicationSettingsModel* m_pPanelSettingsModel;           ///< The Panel's single settings model instance.
    SetupTargetApplicationModel* m_pTargetApplicationModel;    ///< Reference to the target application model.
};

#endif // _DEVELOPER_PANEL_MODEL_H_
