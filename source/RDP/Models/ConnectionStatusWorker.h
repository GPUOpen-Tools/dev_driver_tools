//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Implementation for the worker which executes the connection status
/// loop to continually check that RDS is still connected.
//=============================================================================

#ifndef _CONNECTION_STATUS_WORKER_H_
#define _CONNECTION_STATUS_WORKER_H_

#include <QObject>

struct ChannelContext;

/// Worker which executes the connection status loop
class ConnectionStatusWorker : public QObject
{
    Q_OBJECT
public:
    explicit ConnectionStatusWorker(ChannelContext* pChannelContext);
    virtual ~ConnectionStatusWorker();
    void StartConnectionStatusLoop();

private:
    ChannelContext* m_pChannelContext;      ///< The ChannelContext used to send messages (also contains client information)

public slots:
    void ConnectionStatusLoop();

signals:
    void ClientDisconnected();
};

#endif // _CONNECTION_STATUS_WORKER_H_
