//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Implementation for the connection attempt worker.
//=============================================================================

#ifndef _CONNECTION_ATTEMPT_WORKER_H_
#define _CONNECTION_ATTEMPT_WORKER_H_

#include <QObject>
#include "DeveloperPanelModel.h"

/// Connection attempt result constants
enum ConnectionAttemptResult
{
    ATTEMPT_RESULT_SUCCESS,
    ATTEMPT_RESULT_STOPPED
};

/// Connection attempt worker used to run the connection request loop in its' own thread
class ConnectionAttemptWorker : public QObject
{
    Q_OBJECT
public:
    explicit ConnectionAttemptWorker(DeveloperPanelModel* pPanelModel);
    virtual ~ConnectionAttemptWorker();

private:
    DeveloperPanelModel* m_pPanelModel;         ///< Panel model pointer

signals:
    void ConnectionAttemptFinished(int result);

public slots:
    void AttemptConnection();
};

#endif // _CONNECTION_ATTEMPT_WORKER_H_
