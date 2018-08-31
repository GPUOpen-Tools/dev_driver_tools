//=============================================================================
/// Copyright (c) 2016-2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of the Connection Settings panel interface.
//=============================================================================

#ifndef _CONNECTION_SETTINGS_VIEW_H_
#define _CONNECTION_SETTINGS_VIEW_H_

#include "../Settings/RDPSettings.h"

#include <QThread>
#include <QWidget>

namespace Ui {
class ConnectionSettingsView;
}

class ActiveApplicationsTableView;
class DeveloperPanelModel;
class ConnectionSettingsModel;
class RecentConnectionsView;
class NewConnectionView;
class SetupTargetApplicationView;
class ConnectionStatusView;
class ConnectionLogView;

class ConnectionSettingsView : public QWidget
{
    Q_OBJECT
public:
    explicit ConnectionSettingsView(DeveloperPanelModel* pPanelModel, ConnectionLogView* pConnectionLogView, QWidget* pParent = nullptr);
    virtual ~ConnectionSettingsView();

    virtual void OnClientIdUpdated(DevDriver::ClientId clientId);

    SetupTargetApplicationView* GetSetupTargetApplicationView();
    void SetDisconnectButtonEnabled(bool enabled);
    bool CreatedRdsProcess();

    bool m_disableRdsDisconnectNotification;                            ///< Flag that indicates user initiated the disconnect (don't display message if true).

signals:
    void ConnectionStatusUpdated(bool connected, const QString& hostConnectionString);
    void NewConnectionAdded(const RDSConnectionInfo& connectionInfo);
    void LostRDSConnection();

public slots:
    void OnConnectClicked();
    void OnDisconnectClicked();

private slots:
    void OnTerminateClicked();
    void OnRDSConnected();
    void OnRDSDisconnected();
    void OnShowLog();
    void OnHideLog();
    void OnRecentConnectionSelected(const RDSConnectionInfo& selectedConnectionInfo);
    void OnRecentConnectionDoubleClicked(const RDSConnectionInfo& selectedConnectionInfo);
    void OnConnectionAttemptStopPressed();
    void OnConnectionAttemptFinished(int result);
    void OnConnectionAttemptUpdate();

private:
    void AttemptConnection();
    void ClearVisibleViews();
    void ConnectionAttemptDone();
    void SetupPage(const std::vector<QWidget*>& pageList);
    void ToggleConnectionAttemptControls(bool enabled);

    Ui::ConnectionSettingsView *ui;

    ConnectionStatusView*           m_pConnectionStatusView;            ///< The connection status view
    RecentConnectionsView*          m_pRecentConnectionsView;           ///< The recent connections view
    NewConnectionView*              m_pNewConnectionView;               ///< The new connections view
    SetupTargetApplicationView*     m_pSetupTargetApplicationView;      ///< The setup target application view
    ActiveApplicationsTableView*    m_pActiveApplicationsTableView;     ///< The table containing the list of active applications.
    ConnectionSettingsModel*        m_pConnectionSettingsModel;         ///< The Connection Settings model backend.
    std::vector<QWidget*>           m_preConnectionViews;               ///< Views to be displayed when not connected to RDS
    std::vector<QWidget*>           m_connectedViews;                   ///< Views to be displayed when connected to RDS
    std::vector<QWidget*>           m_connectionLogViews;               ///< Views to be displayed when viewing the connection log
    std::vector<QWidget*>           m_visibleViews;                     ///< The list of views currently on the pane
    DeveloperPanelModel*            m_pPanelModel;                      ///< The main Developer Panel model.

    QTimer*                         m_pConnectionAttemptUpdateTimer;    ///< The timer used to update how long is left in the connection attempt
    int                             m_remainingAttemptTimeMsecs;        ///< Remaining connection attempt time in milliseconds
};

#endif // _CONNECTION_SETTINGS_VIEW_H_
