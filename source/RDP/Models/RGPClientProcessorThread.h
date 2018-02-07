//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A background worker responsible for executing an RGPClient's requests.
//=============================================================================

#ifndef _RGP_CLIENT_PROCESSOR_THREAD_H_
#define _RGP_CLIENT_PROCESSOR_THREAD_H_

#include <QDateTime>
#include "RGPTraceModel.h"

namespace DevDriver
{
    namespace RGPProtocol
    {
        class RGPClient;
    }
    namespace DriverControlProtocol
    {
        class DriverControlClient;
    }
}

class QFile;

/// An enumeration that dictates the request types available with the RGP client.
enum RgpClientRequest
{
    RGP_CLIENT_REQUEST_NONE,
    RGP_CLIENT_REQUEST_EXECUTE_TRACE,
};

/// An enumeration used to determine why a profile-in-progress was aborted.
enum ProfileAbortedReason
{
    PROFILE_ABORTED_REASON_NONE,
    PROFILE_ABORTED_REASON_USER_CANCELLED_TRACE,
    PROFILE_ABORTED_REASON_LOW_DISK_SPACE,
};

/// Register types for use as parameters within the signal/slot system.
Q_DECLARE_METATYPE(DevDriver::Result);
Q_DECLARE_METATYPE(RgpTraceFileInfo);

struct TraceContext
{
    QFile* pTraceFile;                          ///< The file handle used to write the trace to disk.
    size_t totalTraceSizeInBytes;               ///< The final total size of the RGP trace file in bytes.
    size_t totalReceivedSize;                   ///< The total size of the trace received thus far.
    size_t lastChunkReceivedSize;               ///< The size of the last received chunk.
    size_t bytesPerSec;                         ///< The last computed receive rate in MB.
    DevDriver::uint64 numChunks;                ///< The number of chunks received for the trace file.
    DevDriver::uint64 lastStatusUpdateTimeInMs; ///< The timestamp for the last time we received a chunk.
    float updateRateAccumulator;                ///< The total size in MB accumulated in the trace transfer.
    DevDriver::uint32 totalUpdateRateValues;    ///< The count of how many seconds have elapsed during trace transfer.
    ProfileAbortedReason abortedReason;         ///< The reason why a profile in progress was aborted.
};

/// A background worker responsible for executing an RGPClient's requests.
class RGPClientProcessorThread : public QObject
{
    Q_OBJECT
public:
    explicit RGPClientProcessorThread(DeveloperPanelModel* pDeveloperPanelModel, DevDriver::ClientId clientId);
    virtual ~RGPClientProcessorThread();

    void ExecuteTraceArguments(const QString& traceFilepath, const QDateTime& creationTime, DevDriver::RGPProtocol::BeginTraceInfo& traceInfo);
    void EmitTraceProgressUpdate();
    void SetProfileAborted(ProfileAbortedReason abortedReason);
    const TraceContext& GetTraceContext() const { return m_traceContext; }

signals:
    void TraceProgressInfoUpdated(quint64 receivedBytes, quint64 traceSizeInBytes, quint64 receiveRateInMb);
    void ExecuteTraceFinished(DevDriver::Result result, const RgpTraceFileInfo& traceInfo);
    void QueryProfilingStatusFinished(DevDriver::Result result, DevDriver::RGPProtocol::ProfilingStatus* pStatus);
    void EnableProfilingFinished(DevDriver::Result result);

public slots:
    void OnProcessRequest();

private:
    DevDriver::Result ConnectProtocolClients();
    void DisconnectClients();
    DevDriver::Result ExecuteTraceRequest();
    bool BeginWriteRGPTraceFile();
    bool EndWriteRGPTraceFile();
    void CloseActiveTraceFile(bool removeFile = false);
    DevDriver::Result SetTracingClocks();
    DevDriver::Result RevertToApplicationClocks();

    TraceContext m_traceContext;                                    ///< A context that holds incoming RGP chunks for this request.
    DevDriver::RGPProtocol::BeginTraceInfo m_traceParameters;       ///< The parameters to use when collecting a new trace.
    RgpTraceFileInfo m_traceFileInfo;                               ///< The file info for the trace collected by this worker.
    DevDriver::ClientId m_connectedClient;                          ///< The CliendId used to connect the RGPClient.
    RgpClientRequest m_requestType;                                 ///< The type of request being handled in this worker.
    DeveloperPanelModel* m_pPanelModel;                             ///< The main Panel model, holding the RDS connection info.
    DevDriver::RGPProtocol::RGPClient* m_pRgpClient;                ///< The RGPClient to use for requests in this worker.
    DevDriver::DriverControlProtocol::DriverControlClient* m_pDriverControlClient;  ///< The DriverControlClient used to set clock mode.
    bool m_traceAborted;                                            ///< A flag that tracks whether or not to continue transferring trace data.
};

#endif // _RGP_CLIENT_PROCESSOR_THREAD_H_
