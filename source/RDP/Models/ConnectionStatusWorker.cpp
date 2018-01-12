//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Implementation for the worker which executes the connection status
/// loop to continually check that RDS is still connected.
//=============================================================================

#include "ConnectionStatusWorker.h"

#include <QThread>
#include "DeveloperPanelModel.h"
#include "../Util/RDPUtil.h"

const static int s_CONNECTION_CHECK_INTERVAL_MSECS = 250;

//-----------------------------------------------------------------------------
/// Constructor.
/// \param pChannelContext The channel context for the connected client.
//-----------------------------------------------------------------------------
ConnectionStatusWorker::ConnectionStatusWorker(ChannelContext* pChannelContext) :
    QObject(),
    m_pChannelContext(pChannelContext)
{
}

//-----------------------------------------------------------------------------
/// Destructor.
//-----------------------------------------------------------------------------
ConnectionStatusWorker::~ConnectionStatusWorker()
{
}

//-----------------------------------------------------------------------------
/// Start the connection status loop inside this worker's thread. This is a helper
/// function which queues the connection status loop slot for execution in this
/// worker's thread.
//-----------------------------------------------------------------------------
void ConnectionStatusWorker::StartConnectionStatusLoop()
{
    QMetaObject::invokeMethod(this, "ConnectionStatusLoop", Qt::QueuedConnection);
}

//-----------------------------------------------------------------------------
/// Connection status loop which continually checks that there is a connection
/// to RDS. When RDS disconnects, this loop ends and the ClientDisconnected
/// signal is emitted.
//-----------------------------------------------------------------------------
void ConnectionStatusWorker::ConnectionStatusLoop()
{
    bool connected = true;

    while (connected && !m_pChannelContext->exitRequested)
    {
        // Check connection status
        DevDriver::DevDriverClient* pClient = m_pChannelContext->pClient;
        connected = pClient->IsConnected();

        // Sleep
        QThread::msleep(s_CONNECTION_CHECK_INTERVAL_MSECS);
    }

    // If not connected, emit the client disconnected signal
    if (!connected)
    {
        RDPUtil::DbgMsg("[RDP] Lost connection to RDS");
        emit ClientDisconnected();
    }
}
