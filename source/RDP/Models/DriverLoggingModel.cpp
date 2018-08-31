//=============================================================================
/// Copyright (c) 2017-2018 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  The model responsible for updating the driver log messages interface.
//=============================================================================

#include <QThread>

#include "DriverLoggingModel.h"
#include "gpuopen.h"
#include "DriverLogfileModel.h"
#include "DriverLogBackgroundWorker.h"
#include "../Util/RDPUtil.h"
#include "../../DevDriverComponents/inc/protocols/loggingClient.h"

using namespace DevDriver;
using namespace DevDriver::LoggingProtocol;

//-----------------------------------------------------------------------------
/// Constructor for DriverLoggingModel.
/// \param pPanelModel The main Panel model used to maintain a connection to RDS.
/// \param modelCount the number of models required.
//-----------------------------------------------------------------------------
DriverLoggingModel::DriverLoggingModel(DeveloperPanelModel* pPanelModel, uint32_t modelCount)
    : DriverProtocolModel(pPanelModel, modelCount)
    , m_pDriverLogWorkerThread(nullptr)
    , m_pLogReaderWorker(nullptr)
    , m_pLogfileModel(nullptr)
    , m_logWorkerInitialized(false)
{
}

//-----------------------------------------------------------------------------
/// Destructor for DriverLoggingModel.
//-----------------------------------------------------------------------------
DriverLoggingModel::~DriverLoggingModel()
{
    StopLogReaderWorker();

    SAFE_DELETE(m_pLogfileModel);
}

//-----------------------------------------------------------------------------
/// Initialize the driver logging client and background worker.
/// \returns True when the logging client and worker have initialized successfully, and false if it failed.
//-----------------------------------------------------------------------------
bool DriverLoggingModel::InitializeLogging()
{
    LoggingClient* pLoggingClient = nullptr;

    bool gotClient = GetClientByType(Protocol::Logging, (DevDriver::IProtocolClient**)(&pLoggingClient));
    if (gotClient && pLoggingClient != nullptr)
    {
        m_pDriverLogWorkerThread = new QThread;
        if (m_pDriverLogWorkerThread != nullptr)
        {
            m_pLogReaderWorker = new DriverLogBackgroundWorker();
            if (m_pLogReaderWorker != nullptr)
            {
                ResetLogfileModel();

                bool sessionInitialized = m_pLogReaderWorker->InitializeLogReader(pLoggingClient, m_pLogfileModel);
                if (sessionInitialized)
                {
                    m_logWorkerInitialized = true;

                    RDPUtil::DbgMsg("[RDP] Driver logger background worker started.");
                }
                else
                {
                    RDPUtil::DbgMsg("[RDP] Failed to initialize driver log reader.");
                }
            }
        }
        else
        {
            RDPUtil::DbgMsg("[RDP] Failed to create driver log reader thread.");
        }
    }
    else
    {
        RDPUtil::DbgMsg("[RDP] Failed to retrieve Logging client instance.");
    }

    return m_logWorkerInitialized;
}

//-----------------------------------------------------------------------------
/// Start the background worker to process incoming log messages.
//-----------------------------------------------------------------------------
void DriverLoggingModel::StartLogReaderWorker()
{
    m_pLogReaderWorker->moveToThread(m_pDriverLogWorkerThread);
    connect(m_pDriverLogWorkerThread, &QThread::started, m_pLogReaderWorker, &DriverLogBackgroundWorker::ReadIncomingDriverLogMessages);
    connect(m_pDriverLogWorkerThread, &QThread::finished, m_pLogReaderWorker, &DriverLogBackgroundWorker::ThreadFinished, Qt::DirectConnection);

    m_pDriverLogWorkerThread->start();
}

//-----------------------------------------------------------------------------
/// Stop the background worker from reading incoming log messages.
//-----------------------------------------------------------------------------
void DriverLoggingModel::StopLogReaderWorker()
{
    if (m_logWorkerInitialized)
    {
        // Signal the background log reader to stop processing, and then wait for the worker thread to exit.
        m_pLogReaderWorker->EmitStopProcessingLogMessages();
        m_pDriverLogWorkerThread->terminate();

        // Wait for the worker thread to be finished executing before destroying everything.
        m_pDriverLogWorkerThread->wait();

        RDPUtil::DbgMsg("[RDP] Driver logger background worker stopped.");

        // Destroy the background thread and worker, but keep the existing log model.
        // The user should be able to inspect it after disabling reading new messages.
        SAFE_DELETE(m_pLogReaderWorker);
        SAFE_DELETE(m_pDriverLogWorkerThread);

        m_logWorkerInitialized = false;
    }
}

//-----------------------------------------------------------------------------
/// Update the model value for the incoming index.
/// \param modelIndex The index for the updated interface element.
/// \param value The new value for the updated model.
//-----------------------------------------------------------------------------
void DriverLoggingModel::Update(DriverLoggerControls modelIndex, const QVariant& value)
{
    SetModelData(modelIndex, value);

    // @TODO: Implement log filtering controls: Message category, priority, and type.
    switch (modelIndex)
    {
    case DRIVER_LOGGER_CONTROLS_DEBUG_LEVEL:
        {
        }
        break;
    default:
        Q_ASSERT(false);
        break;
    }
}

//-----------------------------------------------------------------------------
/// Reset log file data. Clear out any existing log data and reinitialize an empty log model.
//-----------------------------------------------------------------------------
void DriverLoggingModel::ResetLogfileModel()
{
    // Either create a new empty log model, or reset the existing model.
    if (m_pLogfileModel == nullptr)
    {
        m_pLogfileModel = new DriverLogfileModel;
    }
    else
    {
        m_pLogfileModel->ClearLogfile();
    }
}

