//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
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

/// A vector of ClientIds, with the last Ids being the most recent seen by a halted process.
typedef QVector< DevDriver::ClientId > ClientIdVector;

/// A helper structure used to track info for each halted processes the panel has seen.
class ProcessInfoModel
{
public:
    ProcessInfoModel() {}
    ProcessInfoModel(const QString& name, const QString& description, DevDriver::ProcessId processId);
    ~ProcessInfoModel();

    void SetConnectedStatus(bool isConnected) { m_isConnected = isConnected; }
    void SetProfilingStatus(bool isProfilingEnabled) {m_isProfilingEnabled = isProfilingEnabled; }
    void SetDriverInitializedStatus(bool isInitialized) { m_isDriverInitialized = isInitialized; }
    void SetDriverSettings(const DriverSettingsMap& driverSettingsMap) { m_driverSettings = driverSettingsMap; }
    DevDriver::ClientId GetMostRecentClientId() const;
    bool HasSeenClientId(DevDriver::ClientId clientId) const;
    void UpdateClientId(DevDriver::ClientId newClientId);

    bool GetConnectedStatus() const { return m_isConnected; }
    const QString& GetAPI() const;
    bool GetProfilingStatus() const { return m_isProfilingEnabled; }
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
    bool m_isConnected;                     ///< A flag to track if this process is still known to be alive.
    bool m_isProfilingEnabled;              ///< A flag to indicate if the process is enabled for profiling..
    bool m_isDriverInitialized;             ///< A flag to track if the driver has been fully initialized in the process.
};

// Let Qt know about this class type so it can be used as a parameter for signals/slots
Q_DECLARE_METATYPE(ProcessInfoModel)

#endif // _PROCESS_INFO_MODEL_H_
