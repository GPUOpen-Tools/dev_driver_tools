//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A background worker thread responsible for processing incoming driver log messages.
//=============================================================================

#include <QCoreApplication>
#include "DriverLogBackgroundWorker.h"
#include "DeveloperPanelModel.h"
#include "DriverLogfileModel.h"
#include "../RDPDefinitions.h"
#include "../../DevDriverComponents/inc/util/vector.h"

using namespace DevDriver;
using namespace DevDriver::LoggingProtocol;

//-----------------------------------------------------------------------------
/// Constructor for DriverLogBackgroundWorker.
//-----------------------------------------------------------------------------
DriverLogBackgroundWorker::DriverLogBackgroundWorker()
    : m_pLoggingClient(nullptr)
    , m_pDriverLogfileModel(nullptr)
    , m_retrievingLogMessages(false)
{
    connect(this, SIGNAL(EmitStopProcessingLogMessages()), this, SLOT(StopProcessingLogMessages()));
}

//-----------------------------------------------------------------------------
/// Destructor for DriverLogBackgroundWorker.
//-----------------------------------------------------------------------------
DriverLogBackgroundWorker::~DriverLogBackgroundWorker()
{
}

//-----------------------------------------------------------------------------
/// Initialize the driver log reader background worker.
/// \param pLoggingClient The driver logging client instance to read log messages with.
/// \param pDriverLogfileModel The model to update with new log lines.
/// \returns True when the worker is initialized successfully, and false if it failed.
//-----------------------------------------------------------------------------
bool DriverLogBackgroundWorker::InitializeLogReader(LoggingClient* pLoggingClient, DriverLogfileModel* pDriverLogfileModel)
{
    // @HACK: Temporarily disable the log functionality while the protocol client is being reworked.
    Q_UNUSED(pLoggingClient);
    Q_UNUSED(pDriverLogfileModel);

    if (pLoggingClient != nullptr && pLoggingClient->IsConnected())
    {
        m_pLoggingClient = pLoggingClient;

#ifdef ENABLE_LOGGING_SYSTEM
        if ((pLoggingClient->EnableLogging() == Result::Success) && (pDriverLogfileModel != nullptr))
        {
            m_pDriverLogfileModel = pDriverLogfileModel;
            m_retrievingLogMessages = true;
        }
#endif // ENABLE_LOGGING_SYSTEM

    }

    return m_retrievingLogMessages;
}

//-----------------------------------------------------------------------------
/// Read incoming driver log messages.
//-----------------------------------------------------------------------------
void DriverLogBackgroundWorker::ReadIncomingDriverLogMessages()
{
    while (m_retrievingLogMessages)
    {
        if (m_pLoggingClient != nullptr && m_pLoggingClient->IsConnected())
        {
            DevDriver::Vector<LogMessage> logLines(GenericAllocCb);
            Result result = m_pLoggingClient->ReadLogMessages(logLines);
            size_t numLines = logLines.Size();
            if (result == Result::Success && numLines > 0)
            {
                for (uint32 messageIndex = 0; messageIndex < numLines; ++messageIndex)
                {
                    const QString& logLine = QString(logLines[messageIndex].message);
                    m_pDriverLogfileModel->AddLogLine(logLine);
                }
            }
        }
        else
        {
            m_retrievingLogMessages = false;
        }

        // This thread needs to process any incoming events in order to signal completion.
        QCoreApplication::processEvents();
    }
}

//-----------------------------------------------------------------------------
/// A slot that disables reading new log messages from the connected RDS.
//-----------------------------------------------------------------------------
void DriverLogBackgroundWorker::StopProcessingLogMessages()
{
    if (m_retrievingLogMessages)
    {
        m_retrievingLogMessages = false;
    }
}

//-----------------------------------------------------------------------------
/// Handle shutting down the log worker when the thread is finished.
//-----------------------------------------------------------------------------
void DriverLogBackgroundWorker::ThreadFinished()
{
    if (m_pLoggingClient != nullptr && m_pLoggingClient->IsConnected())
    {
        m_pLoggingClient->DisableLogging();
        m_retrievingLogMessages = false;
    }
}
