//=============================================================================
/// Copyright (c) 2017-2018 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A helper structure used to track info for each halted processes the panel has seen.
//=============================================================================

#ifndef _PROCESS_INFO_MODEL_H_
#define _PROCESS_INFO_MODEL_H_

#include <QObject>
#include <QVector>
#include "../AppSettings/ApplicationSettingsFile.h"
#include "../../DevDriverComponents/inc/gpuopen.h"

// The status for each client in a process
struct ClientStatus
{
    ClientStatus();
    ClientStatus(DevDriver::ClientId clientId, bool connected, bool IsProfilingEnabled);

    DevDriver::ClientId m_clientID;             ///< The client ID
    bool                m_connected;            ///< The client ID's connection status (connected or disconnected)
    bool                m_isProfilingEnabled;   ///< A flag to indicate if the process is enabled for profiling for this clientID
};

/// A vector of Client status comprising of the client Id and its current connection status, with the
/// last one being the most recent seen by a halted process.
typedef QVector< ClientStatus > ClientIdVector;

/// A helper structure used to track info for each halted processes the panel has seen.
class ProcessInfoModel
{
public:
    ProcessInfoModel() {}
    ProcessInfoModel(const QString& name, const QString& description, DevDriver::ProcessId processId);
    ~ProcessInfoModel();

    void SetConnectedStatus(const bool isConnected);
    void SetConnectedStatus(const DevDriver::ClientId clientId, const bool isConnected);
    void SetProfilingStatus(bool isProfilingEnabled);
    void SetProfilingStatus(const DevDriver::ClientId clientId, const bool isProfilingEnabled);
    void SetDriverInitializedStatus(bool isInitialized) { m_isDriverInitialized = isInitialized; }
    void SetDriverSettings(const DriverSettingsMap& driverSettingsMap) { m_driverSettings = driverSettingsMap; }
    DevDriver::ClientId GetMostRecentClientId(const bool getConnectedOnly = false) const;
    bool HasSeenClientId(const DevDriver::ClientId clientId) const;
    void UpdateClientId(DevDriver::ClientId newClientId);

    bool GetConnectedStatus() const;
    const QString& GetAPI() const;
    bool GetProfilingStatus() const;
    bool GetDriverInitializedStatus() const { return m_isDriverInitialized; }
    const QString& GetProcessName() const { return m_processName; }
    const QString& GetProcessDescription() const { return m_processDescription; }
    DevDriver::ProcessId GetProcessId() const { return m_processId; }
    const DriverSettingsMap& GetDriverSettings() const { return m_driverSettings; }

private:
    QString m_processName;                  ///< The executable name of the process.
    QString m_processDescription;           ///< The description of the attachable process.
    DevDriver::ProcessId m_processId;       ///< The process Id.
    ClientIdVector m_clientIds;             ///< The driver's client Id for the process.
    DriverSettingsMap m_driverSettings;     ///< The collection of settings retrieved from the process when it first started.
    QVector<QString> m_categoryStrings;     ///< The vector of category strings the driver settings fit into.
    bool m_isDriverInitialized;             ///< A flag to track if the driver has been fully initialized in the process.
};

// Let Qt know about this class type so it can be used as a parameter for signals/slots
Q_DECLARE_METATYPE(ProcessInfoModel)
Q_DECLARE_METATYPE(DevDriver::ProcessId)

#endif // _PROCESS_INFO_MODEL_H_
