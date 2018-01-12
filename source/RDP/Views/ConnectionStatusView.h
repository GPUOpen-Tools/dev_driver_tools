//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A small control responsible for showing the connection status to RDS.
//=============================================================================

#ifndef _CONNECTION_STATUS_VIEW_H_
#define _CONNECTION_STATUS_VIEW_H_

#include <QWidget>
#include "../Settings/RDPSettings.h"

namespace Ui {
class ConnectionStatusView;
}

/// Connection status constants
enum ConnectionStatus
{
    DISCONNECTED,
    ATTEMPT,
    CONNECTED
};

/// A small control responsible for showing the connection status to RDS.
class ConnectionStatusView : public QWidget
{
    Q_OBJECT

public:
    explicit ConnectionStatusView(QWidget* pParent = 0);
    virtual ~ConnectionStatusView();
    void SetConnectionStatus(ConnectionStatus status);
    void SetHostConnectionString(const QString& hostConnectionString);
    void SetRemainingTime(int remainingTimeMsecs);
    void SetDisconnectButtonEnabled(bool enabled);

    ConnectionStatus GetConnectionStatus();

signals:
    void DisconnectPressed();
    void StopPressed();

private:
    void Update();

    Ui::ConnectionStatusView *ui;           ///< Qt ui component
    ConnectionStatus m_connectionStatus;    ///< Current connection status
    QString m_hostConnectionString;         ///< Hostname string displayed during connected and connection attempt state
    QString m_remainingTimeString;          ///< Remaining time string displayed during connection attempt state
};

#endif // _CONNECTION_STATUS_VIEW_H_
