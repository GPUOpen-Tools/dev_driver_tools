//=============================================================================
/// Copyright (c) 2017-2018 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A background worker thread responsible for processing incoming driver log messages.
//=============================================================================

#ifndef _DRIVER_LOG_BACKGROUND_WORKER_H_
#define _DRIVER_LOG_BACKGROUND_WORKER_H_

#include <QObject>
#include <QVector>
#include "../../DevDriverComponents/inc/protocols/loggingClient.h"

namespace DevDriver
{
    namespace LoggingProtocol
    {
        class LoggingClient;
    }
}

class DriverLogfileModel;

/// A background worker thread responsible for processing incoming driver log messages.
class DriverLogBackgroundWorker : public QObject
{
    Q_OBJECT
public:
    DriverLogBackgroundWorker();
    virtual ~DriverLogBackgroundWorker();

    bool InitializeLogReader(DevDriver::LoggingProtocol::LoggingClient* pLoggingClient, DriverLogfileModel* pDriverLogfileModel);

signals:
    void EmitStopProcessingLogMessages();

public slots:
    void ReadIncomingDriverLogMessages();
    void StopProcessingLogMessages();
    void ThreadFinished();

private:
    DevDriver::LoggingProtocol::LoggingClient* m_pLoggingClient;    ///< The logging client instance used to read log messages.
    DriverLogfileModel* m_pDriverLogfileModel;                      ///< The logfile model to insert log lines into.
    bool m_retrievingLogMessages;                                   ///< The flag used to track if reading log messages is enabled.
};

#endif // _DRIVER_LOG_BACKGROUND_WORKER_H_
