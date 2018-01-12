//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Implementation for the connection attempt worker.
//=============================================================================

#include "ConnectionAttemptWorker.h"

#include <QTimer>
#include <QThread>
#include "../Util/RDPUtil.h"

const static int s_CONNECTION_ATTEMPT_SLEEP_INTERVAL = 250;

//-----------------------------------------------------------------------------
/// Constructor.
/// \param pPanelModel A pointer to the RDP panel model.
//-----------------------------------------------------------------------------
ConnectionAttemptWorker::ConnectionAttemptWorker(DeveloperPanelModel* pPanelModel) :
    m_pPanelModel(pPanelModel)
{
}

//-----------------------------------------------------------------------------
/// Destructor.
//-----------------------------------------------------------------------------
ConnectionAttemptWorker::~ConnectionAttemptWorker()
{
}

//-----------------------------------------------------------------------------
/// Attempt to establish a connection to the RDS client until stopped. Once
/// the worker has been placed into it's own thread use the following call to
/// invoke this method:
///
/// QMetaObject::invokeMethod(pWorker, "AttemptConnection", Qt::QueuedConnection);
//-----------------------------------------------------------------------------
void ConnectionAttemptWorker::AttemptConnection()
{
    while (true)
    {
        bool connectSuccessful = m_pPanelModel->InitializeConnectionToRDS();
        if (connectSuccessful)
        {
            RDPUtil::DbgMsg("[RDP] Established connection to RDS within timeout.");
            RDPUtil::DbgMsg("[RDP] Connected successfully");
            emit ConnectionAttemptFinished(ATTEMPT_RESULT_SUCCESS);
            return;
        }

        // Stop on thread interruption request
        if (QThread::currentThread()->isInterruptionRequested())
        {
            RDPUtil::DbgMsg("[RDS] Failed to connect RDP to RDS. Connection request stopped.");
            emit ConnectionAttemptFinished(ATTEMPT_RESULT_STOPPED);
            return;
        }

        // Sleep a little
        QThread::msleep(s_CONNECTION_ATTEMPT_SLEEP_INTERVAL);
    }
}
