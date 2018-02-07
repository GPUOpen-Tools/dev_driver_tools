//=============================================================================
/// Copyright (c) 2016-2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of the Connection Settings panel interface.
//=============================================================================

#include <QAbstractButton>
#include <QHeaderView>
#include <QPushButton>
#include <QLineEdit>
#include <QTreeView>

#include "ActiveApplicationsTableView.h"
#include "ConnectionSettingsView.h"
#include "ConnectionStatusView.h"
#include "RecentConnectionsView.h"
#include "NewConnectionView.h"
#include "SetupTargetApplicationView.h"
#include "ui_ConnectionSettingsView.h"
#include "../Models/ConnectionSettingsModel.h"
#include "../Models/DeveloperPanelModel.h"
#include "../RDPDefinitions.h"
#include "../Util/RDPUtil.h"
#include "../../Common/ToolUtil.h"
#include <ScalingManager.h>

const static int s_CONNECTION_ATTEMPT_UPDATE_INTERVAL_MSECS = 1000;

//-----------------------------------------------------------------------------
/// Constructor for the ConnectionSettingsView. Initialize the interface defaults.
/// \param pPanelModel The main Panel model used to maintain a connection to RDS.
/// \param pParent The parent widget for the Connection Settings View.
//-----------------------------------------------------------------------------
ConnectionSettingsView::ConnectionSettingsView(DeveloperPanelModel* pPanelModel, QWidget* pParent) :
    QWidget(pParent),
    m_disableRdsDisconnectNotification(false),
    ui(new Ui::ConnectionSettingsView),
    m_pPanelModel(pPanelModel)
{
    ui->setupUi(this);

    // Set white background for this pane
    ToolUtil::SetWidgetBackgroundColor(this, Qt::white);

    // Create a model for the connection settings widgets, and bind the controls in the view.
    m_pConnectionSettingsModel = new ConnectionSettingsModel(pPanelModel, CONNECTION_SETTINGS_COUNT);

    // Setup the connection view objects
    m_pConnectionStatusView = new ConnectionStatusView(this);
    m_pRecentConnectionsView = new RecentConnectionsView(this);
    m_pNewConnectionView = new NewConnectionView(m_pConnectionSettingsModel, this);
    m_pSetupTargetApplicationView = new SetupTargetApplicationView(pPanelModel, this);
    m_pActiveApplicationsTableView = new ActiveApplicationsTableView(pPanelModel, m_pSetupTargetApplicationView, this);

    // Insert the required views into the pane and hide others that aren't required yet
    ui->connectionStatusViewLayout->addWidget(m_pConnectionStatusView);

    m_page1Views.push_back(m_pRecentConnectionsView);
    m_page1Views.push_back(m_pNewConnectionView);
    m_page2Views.push_back(m_pSetupTargetApplicationView);
    m_page2Views.push_back(m_pActiveApplicationsTableView);
    m_pSetupTargetApplicationView->hide();
    m_pActiveApplicationsTableView->hide();

    SetupPage(m_page1Views);

    // Handle the "Connect" button in the Recent Connections View being clicked.
    connect(m_pRecentConnectionsView, &RecentConnectionsView::ConnectionRequested, this, &ConnectionSettingsView::OnRecentConnectionDoubleClicked);
    connect(m_pRecentConnectionsView, &RecentConnectionsView::ConnectionSelected, this, &ConnectionSettingsView::OnRecentConnectionSelected);

    // Connect this and the RecentConnectionsView interface to update the connections list when the user adds a new one.
    connect(this, &ConnectionSettingsView::NewConnectionAdded, m_pRecentConnectionsView, &RecentConnectionsView::OnNewConnectionAdded);

    pPanelModel->RegisterProtocolModel(MAIN_PANEL_MODEL_CONNECTION_SETTINGS, m_pConnectionSettingsModel);

    m_pConnectionSettingsModel->InitializeDefaults();

    // Connect/disconnect action signals and slots
    connect(static_cast<QAbstractButton*>(m_pNewConnectionView->GetConnectButton()), &QAbstractButton::clicked, this, &ConnectionSettingsView::OnConnectClicked);
    connect(static_cast<QLineEdit*>(m_pNewConnectionView->GetIPAddressLineEdit()), &QLineEdit::returnPressed, this, &ConnectionSettingsView::OnConnectClicked);
    connect(static_cast<QLineEdit*>(m_pNewConnectionView->GetPortNumberLineEdit()), &QLineEdit::returnPressed, this, &ConnectionSettingsView::OnConnectClicked);
    connect(m_pConnectionStatusView, &ConnectionStatusView::DisconnectPressed, this, &ConnectionSettingsView::OnDisconnectClicked);

    // Connected/disconnected state signals and slots
    connect(m_pConnectionSettingsModel, &ConnectionSettingsModel::Connected, this, &ConnectionSettingsView::OnRDSConnected);
    connect(m_pConnectionSettingsModel, &ConnectionSettingsModel::Disconnected, this, &ConnectionSettingsView::OnRDSDisconnected);

    // Connection attempt signals/slots
    connect(m_pConnectionSettingsModel, &ConnectionSettingsModel::ConnectionAttemptFinished, this, &ConnectionSettingsView::OnConnectionAttemptFinished);
    connect(m_pConnectionStatusView, &ConnectionStatusView::StopPressed, this, &ConnectionSettingsView::OnConnectionAttemptStopPressed);

    // Select first row in recent connections - this has to be done after setting up
    // signals/slots so that the effects of selecting a row are applied
    m_pRecentConnectionsView->SelectRow(0);

    m_pConnectionAttemptUpdateTimer = new QTimer();
    m_pConnectionAttemptUpdateTimer->setInterval(s_CONNECTION_ATTEMPT_UPDATE_INTERVAL_MSECS);
    connect(m_pConnectionAttemptUpdateTimer, &QTimer::timeout, this, &ConnectionSettingsView::OnConnectionAttemptUpdate);
}

//-----------------------------------------------------------------------------
/// Destructor for the ConnectionSettingsView widget.
//-----------------------------------------------------------------------------
ConnectionSettingsView::~ConnectionSettingsView()
{
    SAFE_DELETE(m_pConnectionSettingsModel);
    SAFE_DELETE(ui);
}

//-----------------------------------------------------------------------------
/// Was an RDS process spawned from this instance of RDP
/// return true if this instance of RDP spawned RDS, false if not
//-----------------------------------------------------------------------------
bool ConnectionSettingsView::CreatedRdsProcess()
{
    return m_pConnectionSettingsModel->CreatedRdsProcess();
}

//-----------------------------------------------------------------------------
/// Update the connected Client Id for any models that are connected to clients.
/// \param clientId The client Id for the client.
//-----------------------------------------------------------------------------
void ConnectionSettingsView::OnClientIdUpdated(DevDriver::ClientId clientId)
{
    m_pConnectionSettingsModel->SetConnectedClientId(clientId);
}

//-----------------------------------------------------------------------------
/// Handler for the "Connect" action - handles everything that needs to happen
/// when the user initiates a connection request.
//-----------------------------------------------------------------------------
void ConnectionSettingsView::OnConnectClicked()
{
    // Only attempt a connection if the IP address is valid
    if (m_pNewConnectionView->IsIPAddressValid())
    {
        // Attempt connection
        AttemptConnection();
    }
}

//-----------------------------------------------------------------------------
/// Handler for the disconnect action - handles everything that needs to happen
/// when the user clicks disconnect.
//-----------------------------------------------------------------------------
void ConnectionSettingsView::OnDisconnectClicked()
{
    // Prevent message box from being displayed for the RDS disconnect.
    m_disableRdsDisconnectNotification = true;

    bool shouldDisconnect = true;
    RDPSettings& rdpSettings = RDPSettings::Get();
    bool showConfirmation = rdpSettings.ShowConfirmationWhenDisconnecting();
    if (showConfirmation)
    {
        const QString& connectionEndpoint = m_pConnectionSettingsModel->GetConnectionEndpointString();
        QString disconnectText = gs_DISCONNECT_CONFIRMATION_TEXT.arg(connectionEndpoint);

        NotificationWidget::Button userResult = RDPUtil::ShowNotification(gs_DISCONNECT_CONFIRMATION_TITLE, disconnectText, NotificationWidget::Button::Yes | NotificationWidget::Button::No, showConfirmation);

        // If the user doesn't want to see this notification again, update the change in the settings file.
        if (showConfirmation == false)
        {
            rdpSettings.SetShowDisconnectConfirmation(false);
        }

        if (userResult == NotificationWidget::Button::No)
        {
            // Stop the disconnection if the user changes their mind.
            shouldDisconnect = false;
        }
    }

    if (shouldDisconnect)
    {
        // Disconnect from the client
        m_pConnectionSettingsModel->DisconnectFromClient();
    }
}

//-----------------------------------------------------------------------------
/// Handler invoked when the terminate RDS button has been clicked.
//-----------------------------------------------------------------------------
void ConnectionSettingsView::OnTerminateClicked()
{
    // Prevent the "Disconnected from RDS" message box from being displayed after the disconnect.
    m_disableRdsDisconnectNotification = true;

    // Send a request to terminate the connected RDS process.
    m_pPanelModel->TerminateConnectedRDS();

    // Disconnect from the client.
    m_pConnectionSettingsModel->DisconnectFromClient();
}

//-----------------------------------------------------------------------------
/// Handler for when the user selects one of the connection items in the recent
/// connections view
/// \param selectedConnectionInfo The connection info for the selected row in the recent connections table.
//-----------------------------------------------------------------------------
void ConnectionSettingsView::OnRecentConnectionSelected(const RDSConnectionInfo& selectedConnectionInfo)
{
    m_pConnectionSettingsModel->Update(CONNECTION_SETTINGS_SERVER_HOST_STRING, selectedConnectionInfo.ipString);
    m_pConnectionSettingsModel->Update(CONNECTION_SETTINGS_SERVER_PORT_STRING, QString::number(selectedConnectionInfo.port));
}

//-----------------------------------------------------------------------------
/// Handler for when the user double clicks one of the connection items in the recent
/// connections view.
/// \param selectedConnectionInfo The connection info for the selected row in the recent connections table.
//-----------------------------------------------------------------------------
void ConnectionSettingsView::OnRecentConnectionDoubleClicked(const RDSConnectionInfo& selectedConnectionInfo)
{
    m_pConnectionSettingsModel->SetConnectionInfo(selectedConnectionInfo);
    AttemptConnection();
}

//-----------------------------------------------------------------------------
/// Attempt to establish a connection to RDS. If it fails, open a popup with a "Failed" message.
//-----------------------------------------------------------------------------
void ConnectionSettingsView::AttemptConnection()
{
    // Disable any ui elements that should be inactive while connection attempt is happening
    ToggleConnectionAttemptControls(false);

    // Get the connection endpoint string to display to the user
    const QString& connectionEndpointString = m_pConnectionSettingsModel->GetConnectionEndpointString();

    // Switch to connection attempt status
    m_pConnectionStatusView->SetHostConnectionString(connectionEndpointString);
    m_pConnectionStatusView->SetConnectionStatus(ATTEMPT);

    // Start connection attempt
    m_pConnectionSettingsModel->InitializeConnection();

    // Start connection attempt ui control loop
    m_pConnectionAttemptUpdateTimer->start();
    m_remainingAttemptTimeMsecs = gs_CONNECTION_TIMEOUT_PERIOD;
    m_pConnectionStatusView->SetRemainingTime(m_remainingAttemptTimeMsecs);
}

//-----------------------------------------------------------------------------
/// Connection attempt update call - updates the remaining time value and
/// determines if the attempt has timed out.
//-----------------------------------------------------------------------------
void ConnectionSettingsView::OnConnectionAttemptUpdate()
{
    // Decrement time remaining
    m_remainingAttemptTimeMsecs -= s_CONNECTION_ATTEMPT_UPDATE_INTERVAL_MSECS;

    if (m_remainingAttemptTimeMsecs > 0)
    {
        m_pConnectionStatusView->SetRemainingTime(m_remainingAttemptTimeMsecs);
    }
    else
    {
        // Connection attempt finished with timeout/failure
        m_pConnectionSettingsModel->StopConnectionAttempt();
        m_pConnectionStatusView->SetConnectionStatus(DISCONNECTED);
        RDPUtil::ShowNotification(gs_CONNECTION_ATTEMPT_FAILED_TITLE, gs_CONNECTION_ATTEMPT_FAILED_TEXT, NotificationWidget::Button::Ok);
        m_pRecentConnectionsView->SelectRow(0);

        ConnectionAttemptDone();
    }
}

//-----------------------------------------------------------------------------
/// Handler for when the user presses the stop button during a connection
/// attempt.
//-----------------------------------------------------------------------------
void ConnectionSettingsView::OnConnectionAttemptStopPressed()
{
    // Connection attempt finished with stop request
    m_pConnectionSettingsModel->StopConnectionAttempt();
    m_pConnectionStatusView->SetConnectionStatus(DISCONNECTED);

    ConnectionAttemptDone();
}

//-----------------------------------------------------------------------------
/// Handler for when a connection attempt finishes.
/// \param result The ConnectionAttemptResult constant indicating the result of
/// the connection attempt.
//-----------------------------------------------------------------------------
void ConnectionSettingsView::OnConnectionAttemptFinished(int result)
{
    // Connection attempt finished with success
    if (result == ATTEMPT_RESULT_SUCCESS)
    {
        const RDSConnectionInfo& newConnectionInfo = m_pConnectionSettingsModel->GetConnectionCreateInfo();
        emit NewConnectionAdded(newConnectionInfo);

        ConnectionAttemptDone();
    }
}

//-----------------------------------------------------------------------------
/// Connection attempt finished call - call this after the connection attempt
/// has finished to restore ui elements and kill the attempt ui update thread.
//-----------------------------------------------------------------------------
void ConnectionSettingsView::ConnectionAttemptDone()
{
    // Stop connection attempt ui loop
    m_pConnectionAttemptUpdateTimer->stop();

    // Re-enable ui elements that were disabled for connection attempt
    ToggleConnectionAttemptControls(true);
}

//-----------------------------------------------------------------------------
/// Handler invoked when RDP connects to RDS - updates the view components
/// to the connected state.
//-----------------------------------------------------------------------------
void ConnectionSettingsView::OnRDSConnected()
{
    // Allow message box to be displayed when RDS disconnects.
    m_disableRdsDisconnectNotification = false;

    // Get the connection endpoint string to display to the user
   const QString& connectionEndpointString = m_pConnectionSettingsModel->GetConnectionEndpointString();

    // Disable the first screen and progress to the next.
    SetupPage(m_page2Views);
    m_pConnectionStatusView->SetHostConnectionString(connectionEndpointString);
    m_pConnectionStatusView->SetConnectionStatus(CONNECTED);

    emit ConnectionStatusUpdated(true, connectionEndpointString);
}

//-----------------------------------------------------------------------------
/// Handler invoked when RDP disconnects from RDS - updates the view components
/// to the disconnected state.
//-----------------------------------------------------------------------------
void ConnectionSettingsView::OnRDSDisconnected()
{
    // Go back to the first screen
    SetupPage(m_page1Views);
    m_pConnectionStatusView->SetConnectionStatus(DISCONNECTED);

    // RDP disconnected from RDS. There's no connected hostname anymore.
    emit ConnectionStatusUpdated(false, "");

    if (!m_disableRdsDisconnectNotification)
    {
        // If the notification is not disabled, this disconnect is caused
        // by RDS, so emit the signal to indicate we lost connection with RDS.
        emit LostRDSConnection();

        // Show the notification box.
        RDPUtil::ShowNotification(gs_CONNECTION_LOST_TITLE, gs_CONNECTION_LOST_TEXT, NotificationWidget::Button::Ok);
    }
}

//-----------------------------------------------------------------------------
/// Remove any currently visible views in the connection view layout and hide
/// them.
//-----------------------------------------------------------------------------
void ConnectionSettingsView::ClearVisibleViews()
{
    for (auto& it : m_visibleViews)
    {
        ui->connectionViewLayout->removeWidget(it);
        it->hide();
    }
    m_visibleViews.clear();
}

//-----------------------------------------------------------------------------
/// Setup the views which go into the connection view layout. These views
/// change depending on whether the user is connected or not.
/// \param Reference to the list of view widgets to be added
//-----------------------------------------------------------------------------
void ConnectionSettingsView::SetupPage(const std::vector<QWidget*>& pageList)
{
    ClearVisibleViews();
    for (auto& it : pageList)
    {
        ui->connectionViewLayout->addWidget(it);
        it->show();
        m_visibleViews.push_back(it);
    }
}

//-----------------------------------------------------------------------------
/// Toggle the user's ability to click controls responsible for establishing a connection.
/// \param enabled A flag used to determine whether to enable or disable connection-attempt controls.
//-----------------------------------------------------------------------------
void ConnectionSettingsView::ToggleConnectionAttemptControls(bool enabled)
{
    QPushButton* pConnectButton = static_cast<QPushButton*>(m_pNewConnectionView->GetConnectButton());
    pConnectButton->setEnabled(enabled);

    QTreeView* pRecentConnectionsList = static_cast<QTreeView*>(m_pRecentConnectionsView->GetRecentConnectionsTable());
    pRecentConnectionsList->setEnabled(enabled);

    // Toggle the enabledness controls in the recent connections view.
    m_pRecentConnectionsView->ToggleDisabledControlsWhileConnecting(enabled);

    m_pNewConnectionView->ToggleDisabledControlsWhileConnecting(enabled);
}

//-----------------------------------------------------------------------------
/// Return the SetupTargetApplicationView object pointer.
/// \return SetupTargetApplicationView pointer
//-----------------------------------------------------------------------------
SetupTargetApplicationView* ConnectionSettingsView::GetSetupTargetApplicationView()
{
    return m_pSetupTargetApplicationView;
}

//-----------------------------------------------------------------------------
/// Set the enabled state of the disconnect button in the connection status view.
/// \param enabled Whether the button should be enabled.
//-----------------------------------------------------------------------------
void ConnectionSettingsView::SetDisconnectButtonEnabled(bool enabled)
{
    m_pConnectionStatusView->SetDisconnectButtonEnabled(enabled);
}
