//=============================================================================
/// Copyright (c) 2016-2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Connection Settings model class definition.
//=============================================================================

#include <QCoreApplication>
#include <QDir>
#include <QModelIndex>
#include <QThread>
#include <QTimer>
#include "ConnectionSettingsModel.h"
#include "DeveloperPanelModel.h"
#include "ConnectionAttemptWorker.h"
#include "gpuopen.h"
#include "../RDPDefinitions.h"
#include "../Util/RDPUtil.h"
#include "../../Common/Process.h"
#include "../../Common/Util/SingleApplicationInstance.h"
#include "../../DevDriverComponents/inc/devDriverClient.h"
#include "../../DevDriverComponents/inc/msgChannel.h"
#include "../../DevDriverComponents/inc/protocols/settingsClient.h"
#include "../../DevDriverComponents/inc/protocols/driverControlClient.h"
#include "../../DevDriverComponents/inc/protocols/loggingClient.h"
#include "../../DevDriverComponents/inc/protocols/rgpClient.h"

using namespace DevDriver;

const static unsigned int s_INVALID_PORT = 0;

//-----------------------------------------------------------------------------
/// Constructor for ConnectionSettingsModel.
/// \param pPanelModel The main Panel model used to maintain a connection to RDS.
/// \param modelCount the number of models required.
//-----------------------------------------------------------------------------
ConnectionSettingsModel::ConnectionSettingsModel(DeveloperPanelModel* pPanelModel, uint32_t modelCount)
    : DriverProtocolModel(pPanelModel, modelCount)
    , m_clientCreateInfo({})
    , m_pRdsProcess(nullptr)
    , m_lastLocalPort(0)
{

    // Initialize all client communication create info
    m_clientCreateInfo.rdsInfo.initialFlags = static_cast<DevDriver::StatusFlags>(ClientStatusFlags::DeveloperModeEnabled) | static_cast<DevDriver::StatusFlags>(ClientStatusFlags::HaltOnConnect);

    Platform::Strncpy(m_clientCreateInfo.rdsInfo.clientDescription, gs_PRODUCT_NAME_STRING.toStdString().c_str(), sizeof(m_clientCreateInfo.rdsInfo.clientDescription));
    m_clientCreateInfo.rdsInfo.componentType = Component::Tool;
    m_clientCreateInfo.rdsInfo.createUpdateThread = true;

    // Set up thread and worker
    m_pWorkerThread = new QThread();
    m_pWorker = new ConnectionAttemptWorker(GetPanelModel());
    m_pWorker->moveToThread(m_pWorkerThread);

    // Signal passthrough
    connect(m_pWorker, &ConnectionAttemptWorker::ConnectionAttemptFinished, this, &ConnectionSettingsModel::ConnectionAttemptFinished);
    connect(GetPanelModel(), &DeveloperPanelModel::Connected, this, &ConnectionSettingsModel::Connected);

    // RDS disconnect handler
    connect(GetPanelModel(), &DeveloperPanelModel::Disconnected, this, &ConnectionSettingsModel::OnRDSDisconnected);
}

//-----------------------------------------------------------------------------
/// Destructor for ConnectionSettingsModel.
//-----------------------------------------------------------------------------
ConnectionSettingsModel::~ConnectionSettingsModel()
{

    // Send a terminate message so the server cleans itself up properly.
    TerminateLocalRds();

    // Close any existing client connections.
    DisconnectFromClient();
}

//-----------------------------------------------------------------------------
/// Was an RDS process spawned from this instance of RDP
/// return true if this instance of RDP spawned RDS, false if not
//-----------------------------------------------------------------------------
bool ConnectionSettingsModel::CreatedRdsProcess()
{
    return m_pRdsProcess != nullptr;
}

//-----------------------------------------------------------------------------
/// Update the model data.
/// \param modelIndex The model index to update.
/// \param value The new model value.
//-----------------------------------------------------------------------------
void ConnectionSettingsModel::Update(ConnectionSettingsControls modelIndex, const QVariant& value)
{
    SetModelData(modelIndex, value);

    switch (modelIndex)
    {
    case CONNECTION_SETTINGS_SERVER_HOST_STRING:
        {
            const QString hostString = value.toString();

            RDPSettings::Get().SetConnectionHost(hostString);

            SetHostIP(hostString);
        }
        break;
    case CONNECTION_SETTINGS_SERVER_PORT_STRING:
        {
            const QString& portString = value.toString();
            uint port = portString.toUInt();
            SetHostPort(port);

            RDPSettings::Get().SetConnectionPort(portString);
        }
        break;
    default:
        Q_ASSERT(false);
        break;
    }
}

//-----------------------------------------------------------------------------
/// Update the internal connection info to match the incoming info from a recent connection.
/// \param connectionInfo The info necessary to connect to RDS.
//-----------------------------------------------------------------------------
void ConnectionSettingsModel::SetConnectionInfo(const RDSConnectionInfo& connectionInfo)
{
    SetHostIP(connectionInfo.ipString);
    SetHostPort(connectionInfo.port);
}

//-----------------------------------------------------------------------------
/// Initialize the default model values.
//-----------------------------------------------------------------------------
void ConnectionSettingsModel::InitializeDefaults()
{
    // Start with the hardcoded product defaults, and then override them with all of the user's changes.
    unsigned int defaultPort = gs_DEFAULT_CONNECTION_PORT;
    QString defaultHost = gs_LOCAL_HOST;

    static RDPSettings& rdpSettings = RDPSettings::Get();
    defaultPort = rdpSettings.GetConnectionPort();
    defaultHost = rdpSettings.GetConnectionHost();

    Update(CONNECTION_SETTINGS_SERVER_PORT_STRING, QString::number(defaultPort));
    Update(CONNECTION_SETTINGS_SERVER_HOST_STRING, defaultHost);
}

//-----------------------------------------------------------------------------
/// Attempt to initialize a connection to the Developer Service. This spawns a
/// worker thread to attempt to establish a connection and then returns. Use
/// the ConnectionAttemptFinished signal to determine when the attempt finishes
/// and if it was successful.
//-----------------------------------------------------------------------------
void ConnectionSettingsModel::InitializeConnection()
{
    DeveloperPanelModel* pPanelModel = GetPanelModel();
    if (pPanelModel != nullptr)
    {
        // If RDP is already connected, early out.
        bool alreadyConnectedToRDS = pPanelModel->IsConnectedToRDS();
        if (alreadyConnectedToRDS)
        {
            return;
        }

        // Save the host IP and port in the parent connection info so the full info can be added to the recent connections table.
        m_clientCreateInfo.port = m_clientCreateInfo.rdsInfo.connectionInfo.port;
        m_clientCreateInfo.ipString = m_clientCreateInfo.rdsInfo.connectionInfo.hostname;

        // Check the IP given by the user and attempt to determine if it's a local or remote connection.
        if (strcmp(m_clientCreateInfo.rdsInfo.connectionInfo.hostname, gs_LOCAL_HOST_IP.toStdString().c_str()) == 0)
        {
            m_clientCreateInfo.rdsInfo.connectionInfo.type = TransportType::Local;

            // Replace the hostname with the user-visible "localhost" string to display in the recent connections table.
            m_clientCreateInfo.ipString = gs_LOCAL_HOST;

            // Use default connection info when connecting to localhost.
            m_clientCreateInfo.rdsInfo.connectionInfo = kDefaultNamedPipe;
        }
        else
        {
            m_clientCreateInfo.rdsInfo.connectionInfo.type = TransportType::Remote;
        }

        // When the user will connect to the local machine, start the RDS process automatically.
        if (m_clientCreateInfo.rdsInfo.connectionInfo.type == TransportType::Local)
        {
            // Check if RDS is already running locally.
            bool rdsAlreadyRunning = ((SingleApplicationInstance*)QApplication::instance())->IsInstanceRunning(gs_RDS_APPLICATION_GUID);

            // Only start a new RDS process if it isn't already running, or if the local port has changed
            if (!rdsAlreadyRunning || (m_clientCreateInfo.rdsInfo.connectionInfo.port != m_lastLocalPort && m_lastLocalPort != 0))
            {
                // RDS isn't already running. Launch it automatically before we attempt to connect.
                RDPUtil::DbgMsg("[RDP] Attempting to start Radeon Developer Service locally.");
                bool rdsLaunched = LaunchLocalRDS(m_clientCreateInfo.rdsInfo.connectionInfo.port);
                if (rdsLaunched)
                {
                    m_lastLocalPort = m_clientCreateInfo.rdsInfo.connectionInfo.port;
                }
                else
                {
                    RDPUtil::DbgMsg("[RDP] Failed to launch Radeon Developer Service locally.");
                }
            }
        }

        AttemptConnection();
    }
}

//-----------------------------------------------------------------------------
/// Disconnect the model's communication client.
//-----------------------------------------------------------------------------
void ConnectionSettingsModel::DisconnectFromClient()
{
    DeveloperPanelModel* pPanelModel = GetPanelModel();
    if (pPanelModel != nullptr && pPanelModel->IsConnectedToRDS())
    {
        pPanelModel->Disconnect();
    }
}

//-----------------------------------------------------------------------------
/// Attempt to connect RDP to RDS until it's successful, or until the request
/// is stopped. This spawns a worker thread to do the request loop, so this
/// function will return without waiting for connection success.
//-----------------------------------------------------------------------------
void ConnectionSettingsModel::AttemptConnection()
{
    // Start the thread
    m_pWorkerThread->start();

    // Start the worker
    QMetaObject::invokeMethod(m_pWorker, "AttemptConnection", Qt::QueuedConnection);
}

//-----------------------------------------------------------------------------
/// Stop trying to establish a connection to RDS. This tells the connection
/// worker to stop execution when it is safe to do so.
//-----------------------------------------------------------------------------
void ConnectionSettingsModel::StopConnectionAttempt()
{
    m_pWorkerThread->quit();
    m_pWorkerThread->requestInterruption();
}

//-----------------------------------------------------------------------------
/// Use RDP to launch a new instance of the RDS process.
/// \param port The port number to try to launch RDS with. An invalid port number
/// (0 and anything above 65535) will be ignored and RDS will launch as normal,
/// using the port from its' settings file.
/// \returns True when a new RDS instance was launched successfully, and false if it failed.
//-----------------------------------------------------------------------------
bool ConnectionSettingsModel::LaunchLocalRDS(unsigned int portNumber = s_INVALID_PORT)
{
    // Clean up RDS if RDP has previously launched it.
    if (m_pRdsProcess != nullptr)
    {
        RDPUtil::DbgMsg("[RDP] Terminating existing RDS process.");
        TerminateLocalRds();
    }

    QString rdsExecutableFilename = gs_RDS_EXECUTABLE_FILENAME;
#if WIN32
    rdsExecutableFilename.append(".exe");
#endif

    QDir executableDirectory = QCoreApplication::applicationDirPath();
    QString rdsExecutablePath = executableDirectory.filePath(rdsExecutableFilename);
    QString rdsArgs = QString("--port %1").arg(portNumber);

    RDPUtil::DbgMsg("[RDP] Launching local RDS.");

    // Start a new RDS instance.
    m_pRdsProcess = new Process();
    DWORD errorCode = m_pRdsProcess->Create(rdsExecutablePath, executableDirectory.absolutePath(), rdsArgs, this);

    // Error code of 0 indicates successful start
    bool startedRds = (errorCode == 0);
    if (!startedRds)
    {
        RDPUtil::DbgMsg("[RDP] Failed to launch local RDS process with error code %d.", startedRds);
    }

    return startedRds;
}

//-----------------------------------------------------------------------------
/// Terminate the local RDS process if it was started by RDP.
//-----------------------------------------------------------------------------
void ConnectionSettingsModel::TerminateLocalRds()
{
    if (m_pRdsProcess != nullptr)
    {
        // Establish a connection, send the terminate signal, then clean up the process.
        GetPanelModel()->TerminateConnectedRDS();

        bool terminatedSuccessfully = m_pRdsProcess->Terminate();
        RDPUtil::DbgMsg("[RDP] Local service process termination %s.", terminatedSuccessfully ? "was successful" : "failed");

        SAFE_DELETE(m_pRdsProcess);
        RDPUtil::DbgMsg("[RDP] Successfully terminated local RDS launched by RDP.");
    }
}

//-----------------------------------------------------------------------------
/// Set the RDS host IP address in the DevDriver connection info structure.
/// \param hostIP The IP where the RDS process is running.
//-----------------------------------------------------------------------------
void ConnectionSettingsModel::SetHostIP(const QString& hostIP)
{
    std::string hostStdString = hostIP.toStdString();

    // If the hostString is "localhost", set it to "127.0.0.1"
    if (hostIP.compare(gs_LOCAL_HOST) == 0)
    {
        hostStdString = gs_LOCAL_HOST_IP.toStdString();
    }

    Platform::Strncpy(m_clientCreateInfo.rdsInfo.connectionInfo.hostname, hostStdString.c_str(), kMaxStringLength);
}

//-----------------------------------------------------------------------------
/// Set the port to connect to RDS with.
/// \param port The port to connect to RDS with.
//-----------------------------------------------------------------------------
void ConnectionSettingsModel::SetHostPort(uint16_t port)
{
    m_clientCreateInfo.rdsInfo.connectionInfo.port = port;
}

//-----------------------------------------------------------------------------
/// Get the hostname string. This returns the hostname as a string, formatted
/// as "IP:PORT"
/// \return The hostname string.
//-----------------------------------------------------------------------------
const QString ConnectionSettingsModel::GetConnectionEndpointString() const
{
    // Append the Hostname:Port together to display to the user.
    QString hostnameString(m_clientCreateInfo.rdsInfo.connectionInfo.hostname);
    if (hostnameString.isEmpty())
    {
        // If the hostname string is empty, the user is connecting to localhost.
        hostnameString.append(gs_LOCAL_HOST);
    }

#ifdef Q_OS_WIN
    if (hostnameString != gs_LOCAL_HOST)
    {
        hostnameString.append(":");
        hostnameString.append(QString::number(m_clientCreateInfo.rdsInfo.connectionInfo.port));
    }
#else
    hostnameString.append(":");
    hostnameString.append(QString::number(m_clientCreateInfo.rdsInfo.connectionInfo.port));
#endif
    return hostnameString;
}

//-----------------------------------------------------------------------------
/// Handler for when RDS is disconnected.
//-----------------------------------------------------------------------------
void ConnectionSettingsModel::OnRDSDisconnected()
{
    // If connected using a local transport type, restore clientCreateInfo ip to local host ip
    if (m_clientCreateInfo.rdsInfo.connectionInfo.type == TransportType::Local)
    {
        Platform::Strncpy(m_clientCreateInfo.rdsInfo.connectionInfo.hostname,
            gs_LOCAL_HOST_IP.toStdString().c_str(), sizeof(char) * kMaxStringLength);
    }

    emit Disconnected();
}
