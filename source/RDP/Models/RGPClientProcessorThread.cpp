//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A background worker responsible for executing an RGPClient's requests.
//=============================================================================

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include "DeveloperPanelModel.h"
#include "RGPClientProcessorThread.h"
#include "../Settings/RDPSettings.h"
#include "../Util/RDPUtil.h"
#include "../../DevDriverComponents/inc/protocols/rgpClient.h"
#include "../../DevDriverComponents/inc/protocols/driverControlClient.h"

/// For now RDP will only affect the first GPU. mGPU is not handled yet.
static const int kGPUIndex = 0;

/// The clock mode to use while collecting a trace.
static const DevDriver::DriverControlProtocol::DeviceClockMode kTraceClockMode = DevDriver::DriverControlProtocol::DeviceClockMode::Peak;

/// A static instance for the active working collecting a trace.
static RGPClientProcessorThread* s_pProcessorThreadInstance = nullptr;

//-----------------------------------------------------------------------------
/// Constructor for RGPClientProcessorThread.
//-----------------------------------------------------------------------------
RGPClientProcessorThread::RGPClientProcessorThread(DeveloperPanelModel* pDeveloperPanelModel, DevDriver::ClientId clientId)
    : m_connectedClient(clientId)
    , m_requestType(RGP_CLIENT_REQUEST_NONE)
    , m_pPanelModel(pDeveloperPanelModel)
    , m_pRgpClient(nullptr)
    , m_pDriverControlClient(nullptr)
    , m_traceAborted(false)
{
    memset(&m_traceParameters, 0, sizeof(DevDriver::RGPProtocol::BeginTraceInfo));
    m_traceContext = {};
    m_traceFileInfo = {};

    s_pProcessorThreadInstance = this;
}

//-----------------------------------------------------------------------------
/// Destructor for the RGPClientProcessorThread worker type.
//-----------------------------------------------------------------------------
RGPClientProcessorThread::~RGPClientProcessorThread()
{
    // Clean up the trace file if it's currently open.
    CloseActiveTraceFile();
}

//-----------------------------------------------------------------------------
/// Attempt to establish a connection with RGP and DriverControl clients.
/// \returns The result of the connection attempt.
//-----------------------------------------------------------------------------
DevDriver::Result RGPClientProcessorThread::ConnectProtocolClients()
{
    using namespace DevDriver;
    DevDriver::Result connectResult = DevDriver::Result::Error;

    // Try to acquire an RGPClient to send requests with.
    ChannelContext& channelContext = m_pPanelModel->GetChannelContext();

    if (channelContext.pClient != nullptr)
    {
        m_pRgpClient = channelContext.pClient->AcquireProtocolClient<DevDriver::Protocol::RGP>();
        Q_ASSERT(m_pRgpClient != nullptr);
        connectResult = m_pRgpClient->Connect(m_connectedClient);
        if (connectResult != Result::Success)
        {
            RDPUtil::DbgMsg("[RDP] Failed to connect RGPClient to collect profile.");
        }

        m_pDriverControlClient = channelContext.pClient->AcquireProtocolClient<DevDriver::Protocol::DriverControl>();
        Q_ASSERT(m_pDriverControlClient != nullptr);
        connectResult = m_pDriverControlClient->Connect(m_connectedClient);
        if (connectResult != Result::Success)
        {
            RDPUtil::DbgMsg("[RDP] Failed to connect DriverControlClient to set profiling clock mode.");
        }
        Q_ASSERT(connectResult == Result::Success);
    }
    else
    {
        // The ChannelContext Client was null? Something's very wrong.
        Q_ASSERT(false);
    }

    return connectResult;
}

//-----------------------------------------------------------------------------
/// Disconnect the RGPClient used to make requests in this background worker.
//-----------------------------------------------------------------------------
void RGPClientProcessorThread::DisconnectClients()
{
    // Do nothing if RDS is disconnected
    if (!m_pPanelModel->IsConnectedToRDS())
    {
        return;
    }

    if (m_pRgpClient == nullptr && m_pDriverControlClient != nullptr)
    {
        return;
    }

    if (m_pRgpClient->IsConnected())
    {
        // Release the client used to make the request.
        ChannelContext& channelContext = m_pPanelModel->GetChannelContext();

        Q_ASSERT(channelContext.pClient != nullptr);
        channelContext.pClient->ReleaseProtocolClient(m_pRgpClient);
    }

    if (m_pDriverControlClient->IsConnected())
    {
        // Release the client used to make the request.
        ChannelContext& channelContext = m_pPanelModel->GetChannelContext();

        Q_ASSERT(channelContext.pClient != nullptr);
        channelContext.pClient->ReleaseProtocolClient(m_pDriverControlClient);
    }
}

//-----------------------------------------------------------------------------
/// Execute the RGP trace capture request. Write the trace data if it was successful.
/// \returns The DevDriver::Result code of the trace exection request.
//-----------------------------------------------------------------------------
DevDriver::Result RGPClientProcessorThread::ExecuteTraceRequest()
{
    using namespace DevDriver;

    // Set the GPU clock mode before starting a trace.
    Result setClocks = SetTracingClocks();

    Result requestResult = m_pRgpClient->BeginTrace(m_traceParameters);

    Result restoredClocks = Result::Error;

    if (requestResult == Result::Success)
    {
        RDPUtil::DbgMsg("[RDP] Profiling began successfully.");

        // Clear out the existing trace context info before starting to collect new trace data.
        m_traceContext = {};

        uint32 numChunks = 0;
        requestResult = m_pRgpClient->EndTrace(&numChunks, &m_traceContext.totalTraceSizeInBytes);

        // Revert the clock mode to the RDP default after tracing.
        if (setClocks == Result::Success && m_pPanelModel->IsConnectedToRDS())
        {
            restoredClocks = RevertToApplicationClocks();
        }

        if (((requestResult == Result::Success) || (requestResult == Result::Unavailable)) && m_pPanelModel->IsConnectedToRDS())
        {
            // Looks like the trace was successful, so start transferring the chunks.
            emit TraceProgressInfoUpdated(0, m_traceContext.totalTraceSizeInBytes, 0);

            // Only try to write the trace file if the trace executed correctly.
            bool beganTraceWrite = BeginWriteRGPTraceFile();
            if (beganTraceWrite)
            {
                // Read chunks until we hit the end of the stream.
                do
                {
                    // Check if the user wants to cancel collecting the trace.
                    if (m_traceAborted || !m_pPanelModel->IsConnectedToRDS())
                    {
                        requestResult = Result::Aborted;
                        break;
                    }

                    requestResult = m_pRgpClient->ReadTraceDataChunk();
                } while (requestResult == Result::Success);

                if (requestResult == Result::EndOfStream)
                {
                    // If the result is EndOfStream, it means that all of the trace chunks were transferred.
                    // Just return Success to better indicate that the trace was successful.
                    requestResult = Result::Success;

                    // Due to differences in how RGPClient works between versions,
                    // reporting the total trace can happen in different ways.
                    if (m_traceContext.totalTraceSizeInBytes != 0)
                    {
                        m_traceFileInfo.totalBytes = m_traceContext.totalTraceSizeInBytes;
                    }
                    else
                    {
                        m_traceFileInfo.totalBytes = m_traceContext.totalReceivedSize;
                    }
                    EndWriteRGPTraceFile();
                }
                else
                {
                    // Don't report an error if the user purposely canceled collecting the trace.
                    if (!m_traceAborted)
                    {
                        RDPUtil::DbgMsg("[RDP] Error retrieving profile data!");
                    }

                    // The trace file write was started, but didn't complete successfully. Delete the partially written file.
                    CloseActiveTraceFile(true);
                }
            }
        }
    }
    else
    {
        const QString resultAsString = ToolUtil::GetResultString(requestResult);
        RDPUtil::DbgMsg("Failed to begin profile. Result = %s", resultAsString.toStdString().c_str());

        // Looks like tracing failed- Attempt to revert the clock state to what it was originally.
        if (restoredClocks != Result::Success)
        {
            restoredClocks = RevertToApplicationClocks();
            if (restoredClocks != Result::Success)
            {
                RDPUtil::DbgMsg("Failed to restore GPU clocks to default after profiling.");
            }
        }
    }

    // Signal to the RGPTraceModel that the trace is finished.
    emit ExecuteTraceFinished(requestResult, m_traceFileInfo);

    return requestResult;
}

//-----------------------------------------------------------------------------
/// Process the RGPClient request.
//-----------------------------------------------------------------------------
void RGPClientProcessorThread::OnProcessRequest()
{
    DevDriver::Result connectionResult = ConnectProtocolClients();

    if (connectionResult == DevDriver::Result::Success)
    {
        DevDriver::Result requestResult = DevDriver::Result::Error;

        switch (m_requestType)
        {
        case RGP_CLIENT_REQUEST_EXECUTE_TRACE:
            {
                if (m_pRgpClient != nullptr && m_pRgpClient->IsConnected())
                {
                    requestResult = ExecuteTraceRequest();
                }
            }
            break;
        default:
            // This type of request is unhandled at the moment.
            Q_ASSERT(false);
        }

        Q_UNUSED(requestResult);
        DisconnectClients();
    }
    else
    {
        RDPUtil::DbgMsg("[RDP] Failed to connect new RGPClient with ClientId %u", m_connectedClient);
    }
}

//-----------------------------------------------------------------------------
/// Handler invoked when the user wishes to cancel a trace that's in-progress.
/// \param The reason why the profile collection was aborted.
//-----------------------------------------------------------------------------
void RGPClientProcessorThread::SetProfileAborted(ProfileAbortedReason abortedReason)
{
    // Set the flag used to abort collecting the trace in progress.
    m_traceAborted = true;
    m_traceContext.abortedReason = abortedReason;
}

//-----------------------------------------------------------------------------
/// A callback handler invoked for each new RGP Trace data chunk recieved by the Panel.
/// \param pChunk The new chunk of RGP trace file data to append.
/// \param pUserData A userdata pointer holding info for the in-progress trace.
//-----------------------------------------------------------------------------
void TraceDataChunkReceivedCallback(const DevDriver::RGPProtocol::TraceDataChunk* pChunk, void* pUserdata)
{
    TraceContext* pTraceContext = reinterpret_cast<TraceContext*>(pUserdata);
    if (pTraceContext != nullptr)
    {
        if (pTraceContext->pTraceFile != nullptr && pTraceContext->pTraceFile->isOpen())
        {
            // Write the latest chunk into the trace file.
            qint64 chunkSize = pChunk->dataSize;

            // We should never get chunks with no data
            DD_ASSERT(chunkSize > 0);

            pTraceContext->numChunks++;

            // Update the status message at a fixed interval.
            const DevDriver::uint64 currentTimeInMs = DevDriver::Platform::GetCurrentTimeInMs();

            // If this is the first chunk, initialize the status update time.
            if (pTraceContext->lastStatusUpdateTimeInMs == 0)
            {
                pTraceContext->lastStatusUpdateTimeInMs = currentTimeInMs;
                RDPUtil::DbgMsg("[RDP] Receiving Profiling Data...");
            }

            pTraceContext->lastChunkReceivedSize = chunkSize;
            pTraceContext->totalReceivedSize += chunkSize;

            const qint64 bytesWritten = pTraceContext->pTraceFile->write(reinterpret_cast<const char*>(pChunk->data), chunkSize);

            // Verify that all bytes in the latest chunk were written correctly. Forget about the current trace if there was a problem.
            if (bytesWritten != chunkSize || bytesWritten == -1)
            {
                RDPUtil::DbgMsg("[RDP] Profile collection failed due to lack of disk space.");
                s_pProcessorThreadInstance->SetProfileAborted(PROFILE_ABORTED_REASON_LOW_DISK_SPACE);
            }
        }

        if (s_pProcessorThreadInstance != nullptr)
        {
            s_pProcessorThreadInstance->EmitTraceProgressUpdate();
        }
    }
}

//-----------------------------------------------------------------------------
/// Execute an RGP trace using the given trace parameters.
/// \param traceFilepath The full filepath to the trace file to be written.
/// \param creationTime The DateTime when the trace file was created.
/// \param traceInfo A structure containing the user's RGP trace execution parameters.
//-----------------------------------------------------------------------------
void RGPClientProcessorThread::ExecuteTraceArguments(const QString& traceFilepath, const QDateTime& creationTime, DevDriver::RGPProtocol::BeginTraceInfo& traceInfo)
{
    m_requestType = RGP_CLIENT_REQUEST_EXECUTE_TRACE;
    m_traceFileInfo.fullPathToFile = traceFilepath;

    QFileInfo traceFileInfo(traceFilepath);
    m_traceFileInfo.fileToDisplay = traceFileInfo.fileName();
    m_traceFileInfo.traceCreationTimestamp = creationTime.toTime_t();

    // Copy the incoming trace info structure so the arguments can be used in the upcoming request execution.
    memcpy(&m_traceParameters, &traceInfo, sizeof(DevDriver::RGPProtocol::BeginTraceInfo));

    // This worker class will handle receiving new RGP chunks.
    DevDriver::RGPProtocol::ChunkCallbackInfo traceChunkCallbackInfo = {};
    traceChunkCallbackInfo.chunkCallback = &TraceDataChunkReceivedCallback;
    traceChunkCallbackInfo.pUserdata = &m_traceContext;

    // Set up the callback info to point to this worker.
    m_traceParameters.callbackInfo = traceChunkCallbackInfo;
}

//-----------------------------------------------------------------------------
/// Emit a progress updated signal with the current trace context's transfer info.
//-----------------------------------------------------------------------------
void RGPClientProcessorThread::EmitTraceProgressUpdate()
{
    emit TraceProgressInfoUpdated(m_traceContext.totalReceivedSize, m_traceContext.totalTraceSizeInBytes, m_traceContext.bytesPerSec);
}

//-----------------------------------------------------------------------------
/// Create the file to write the RGP trace data chunks to. File will be closed when all chunks are tranferred.
/// \returns True if the trace file was created and opened successfully.
//-----------------------------------------------------------------------------
bool RGPClientProcessorThread::BeginWriteRGPTraceFile()
{
    bool bFileReady = false;

    const QString& traceFilename = m_traceFileInfo.fullPathToFile;
    std::string filepathString = traceFilename.toStdString();

    // Make sure that the directory for the file is created before writing.
    QFileInfo fileInfo(traceFilename);
    QDir filepathDir = fileInfo.absoluteDir();

    // Does RDP need to create the directory where the trace file will be dumped to?
    bool pathCreated = true;
    if (!filepathDir.exists())
    {
        pathCreated = filepathDir.mkdir(filepathDir.absolutePath());
        if (pathCreated)
        {
            RDPUtil::DbgMsg("[RDP] Created profile output path %s", filepathDir.absolutePath().toLatin1().data());
        }
        else
        {
            RDPUtil::DbgMsg("[RDP] Failed to create profile output path %s", filepathDir.absolutePath().toLatin1().data());
        }
    }

    if (pathCreated)
    {
        m_traceContext.pTraceFile = new QFile(traceFilename);
        bool openedSuccessfully = m_traceContext.pTraceFile->open(QFile::WriteOnly);

        if (openedSuccessfully)
        {
            RDPUtil::DbgMsg("[RDP] Created profile %s", filepathString.c_str());
            bFileReady = true;
        }
        else
        {
            RDPUtil::DbgMsg("[RDP] Failed to open profile for writing at $s.", traceFilename.toStdString().c_str());
        }
    }
    else
    {
        RDPUtil::DbgMsg("[RDP] Failed to create profile filepath at %s.", traceFilename.toStdString().c_str());
    }

    return bFileReady;
}

//-----------------------------------------------------------------------------
/// Finalize writing the RGP chunk data and close the trace file.
/// \returns True if the file was written successfully.
//-----------------------------------------------------------------------------
bool RGPClientProcessorThread::EndWriteRGPTraceFile()
{
    bool wroteTraceFile = false;

    // Don't attempt to write anything if the user aborted collecting the trace.
    if (!m_traceAborted)
    {
        std::string traceFilepath = m_traceContext.pTraceFile->fileName().toStdString();

        if (m_traceContext.pTraceFile->isOpen())
        {
            // Report that we're done writing chunks to the trace file.
            RDPUtil::DbgMsg("[RDP] Completed writing %u chunks of profile data to %s", m_traceContext.numChunks, traceFilepath.c_str());

            // Close the trace file.
            CloseActiveTraceFile();
            wroteTraceFile = true;
        }
        else
        {
            RDPUtil::DbgMsg("[RDP] Failed to write profile because file at %s was not open.", traceFilepath.c_str());
        }
    }
    else
    {
        RDPUtil::DbgMsg("[RDP] User canceled writing profile file.");
    }

    return wroteTraceFile;
}

//-----------------------------------------------------------------------------
/// Close the trace file handle for the trace being written (if there is one).
//-----------------------------------------------------------------------------
void RGPClientProcessorThread::CloseActiveTraceFile(bool removeFile)
{
    if (m_traceContext.pTraceFile != nullptr)
    {
        if (m_traceContext.pTraceFile->isOpen())
        {
            m_traceContext.pTraceFile->close();
        }

        // Can optionally remove the file from disk (in cases where it was only partially written).
        if (removeFile)
        {
            m_traceContext.pTraceFile->remove();
        }

        SAFE_DELETE(m_traceContext.pTraceFile);
    }
}

//-----------------------------------------------------------------------------
/// Set the GPU clock mode to be used when collecting an RGP trace.
/// \returns Result::Success if the clock mode was set correctly, and an error code if it failed.
//-----------------------------------------------------------------------------
DevDriver::Result RGPClientProcessorThread::SetTracingClocks()
{
    using namespace DevDriver;

    Result setClockResult = Result::Error;

    if (m_pDriverControlClient != nullptr && m_pDriverControlClient->IsConnected())
    {
        // RDP explicitly sets the GPU's clock mode to ensure timing accuracy while collecting a trace.
        setClockResult = m_pDriverControlClient->SetDeviceClockMode(kGPUIndex, kTraceClockMode);
        if (setClockResult == Result::Success)
        {
            QString clockModeQString = RDPUtil::GetClockModeAsString(kTraceClockMode);
            std::string clockModeString = clockModeQString.toStdString();
            RDPUtil::DbgMsg("[RDP] Set clock mode to %s for profiling.", clockModeString.c_str());
        }
    }
    else
    {
        RDPUtil::DbgMsg("[RDP] Didn't set clock for profiling because DriverControlClient wasn't connected.");
    }

    if (setClockResult != Result::Success)
    {
        QString clockModeQString = RDPUtil::GetClockModeAsString(kTraceClockMode);
        std::string clockModeString = clockModeQString.toStdString();
        RDPUtil::DbgMsg("[RDP] Failed to set GPU clocks to %s for profiling.", clockModeString.c_str());
    }

    return setClockResult;
}

//-----------------------------------------------------------------------------
/// Restore the GPU clock mode to the user's chosen clock setting after tracing is finished.
/// \returns Result::Success if the clock mode was restored correctly, and an error code if it failed.
//-----------------------------------------------------------------------------
DevDriver::Result RGPClientProcessorThread::RevertToApplicationClocks()
{
    using namespace DevDriver;

    Result setClockResult = Result::Error;

    if (m_pDriverControlClient != nullptr && m_pDriverControlClient->IsConnected())
    {
        // After tracing, restore the GPU clocks to the user's clock choice from the settings file.
        DevDriver::DriverControlProtocol::DeviceClockMode userClockMode = RDPSettings::Get().GetUserClockMode();
        setClockResult = m_pDriverControlClient->SetDeviceClockMode(kGPUIndex, userClockMode);
        if (setClockResult == Result::Success)
        {
            QString clockModeQString = RDPUtil::GetClockModeAsString(userClockMode);
            std::string clockModeString = clockModeQString.toStdString();
            RDPUtil::DbgMsg("[RDP] Reverted clock after profiling to %s.", clockModeString.c_str());
        }
    }
    else
    {
        RDPUtil::DbgMsg("[RDP] Didn't revert from profiling clocks because DriverControlClient wasn't connected.");
    }

    if (setClockResult != Result::Success)
    {
        RDPUtil::DbgMsg("[RDP] Failed to restore GPU clocks to user selection after profiling.");
    }

    return setClockResult;
}
