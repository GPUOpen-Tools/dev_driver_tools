//=============================================================================
/// Copyright (c) 2016-2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Connection Settings model class definition.
//=============================================================================

#ifndef _CONNECTION_SETTINGS_MODEL_H_
#define _CONNECTION_SETTINGS_MODEL_H_

#include "DriverProtocolModel.h"
#include "../RDPDefinitions.h"
#include "../Settings/RDPSettings.h"
#include "../../DevDriverComponents/inc/gpuopen.h"
#include "../../DevDriverComponents/inc/msgTransport.h"
#include "../../DevDriverComponents/inc/devDriverClient.h"
#include "ConnectionAttemptWorker.h"

class Process;

/// An enumeration used to represent controls in the Connection Settings interface.
enum ConnectionSettingsControls
{
    CONNECTION_SETTINGS_SERVER_HOST_STRING, ///< The server host textbox.
    CONNECTION_SETTINGS_SERVER_PORT_STRING, ///< The connection port textbox.

    CONNECTION_SETTINGS_COUNT,
};

/// Model class for the Connection Settings tab.
class ConnectionSettingsModel : public DriverProtocolModel
{
    Q_OBJECT
public:
    explicit ConnectionSettingsModel(DeveloperPanelModel* pPanelModel, uint32_t modelCount);
    virtual ~ConnectionSettingsModel();

    void Update(ConnectionSettingsControls modelIndex, const QVariant& value);
    void SetConnectionInfo(const RDSConnectionInfo& connectionInfo);
    void InitializeDefaults();
    void InitializeConnection();
    void DisconnectFromClient();
    void StopConnectionAttempt();
    bool CreatedRdsProcess();

    const RDSConnectionInfo& GetConnectionCreateInfo() const { return m_clientCreateInfo; }
    const QString GetConnectionEndpointString() const;

private:
    void AttemptConnection();
    bool LaunchLocalRDS(unsigned int portNumber);
    void TerminateLocalRds();
    void SetHostIP(const QString& hostIP);
    void SetHostPort(uint16_t port);

    RDSConnectionInfo m_clientCreateInfo;   ///< Create info with client settings.
    Process* m_pRdsProcess;                 ///< RDS process pointer.
    QThread* m_pWorkerThread;               ///< Worker thread used when trying to establish a connection
    ConnectionAttemptWorker* m_pWorker;     ///< Worker object used when trying to establish a connection
    unsigned int m_lastLocalPort;

signals:
    void ConnectionAttemptFinished(int result);
    void RemainingTimeUpdated(int remainingTimeMsecs);
    void Connected();
    void Disconnected();

public slots:
    void OnRDSDisconnected();
};

#endif // _CONNECTION_SETTINGS_MODEL_H_
