//=============================================================================
/// Copyright (c) 2016-2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  The model used to store RGP trace capture settings.
//=============================================================================

#ifndef _RGP_TRACE_MODEL_H_
#define _RGP_TRACE_MODEL_H_

#include <deque>
#include "DriverProtocolModel.h"
#include "../../DevDriverComponents/inc/protocols/rgpClient.h"

class ApplicationSettingsModel;
class RGPClientProcessorThread;
class RGPRecentTraceListModel;

/// An enumeration representing the controls displayed in the RGP Trace interface.
enum RgpTraceControls
{
    RGP_TRACE_CONTROLS_PROCESS_NAME,                ///< The filename of the executable targeted for profiling.
    RGP_TRACE_CONTROLS_PROCESS_ID,                  ///< The ProcessId being targeted for profiling.
    RGP_TRACE_CONTROLS_PROCESS_API,                 ///< The API string for the target process.
    RGP_TRACE_CONTROLS_PROCESS_CLIENT_ID,           ///< The DevDriver Client Id for the target process.
    RGP_TRACE_CONTROLS_OUTPUT_PATH_STRING,          ///< The trace output directory text.
    RGP_TRACE_CONTROLS_RGP_DETAILED_TRACE_DATA,     ///< A checkbox used to determine if detailed instruction tokens are included in the trace.
    RGP_TRACE_CONTROLS_RGP_ALLOW_COMPUTE_PRESENTS,  ///< A checkbox used to determine if compute queue presents are allowed.
    RGP_TRACE_CONTROLS_RGP_FILEPATH_STRING,         ///< The filepath to the installed RGP executable on the local machine.
    RGP_TRACE_CONTROLS_COUNT,                       ///< The number of controls displayed in the interface.
};

/// A structure used to store RGP trace metadata info.
struct RgpTraceFileInfo
{
    QString fullPathToFile;             ///< The full path to the trace file.
    QString fileToDisplay;              ///< The trace filename only.
    size_t totalBytes;                  ///< The total filesize of the trace file in bytes.
    time_t traceCreationTimestamp;      ///< A timestamp collected when the trace was captured.
};

/// The model used to store RGP trace capture settings.
class RGPTraceModel : public DriverProtocolModel
{
    Q_OBJECT
public:
    explicit RGPTraceModel(DeveloperPanelModel* pPanelModel, ApplicationSettingsModel* pApplicationSettingsModel, uint32_t modelCount);
    virtual ~RGPTraceModel();

    void InitializeDefaults();
    void Update(RgpTraceControls modelIndex, const QVariant& value);
    void CollectRGPTrace();
    void DestroyWorkerThread();
    static float ComputeFileSizeInMB(size_t fileSizeInBytes);
    void ClearProfilingTargetStatus();

    const QString& GetTraceOutputPath() const { return m_traceOutputPath; }
    const QString& GetPathToRgp() const { return m_pathToRgp; }
    ApplicationSettingsModel* GetApplicationSettingsModel() const { return m_pSettingsModel; }
    RGPRecentTraceListModel* GetRecentTraceListModel() const { return m_pRecentTracesModel; }

signals:
    void CurrentlyCollectingTrace(bool currentlyCollecting);
    void UpdateCollectRGPTraceButton(bool enable);
    void TraceProgressInfoUpdated(quint64 receivedBytes, quint64 traceSizeInBytes, quint64 receiveRateInMb);

public slots:
    void OnApplicationUnchecked();
    void OnTraceProgressInfoUpdated(quint64 receivedBytes, quint64 traceSizeInBytes, quint64 receiveRateInMb);
    void OnTraceRequestCanceled();

private:
    bool CanProfileApplication(DevDriver::RGPProtocol::RGPClient* pRgpClient);
    const QString GenerateTraceFilename(const QString& timestampSuffix) const;

private slots:
    void OnExecuteTraceFinished(DevDriver::Result result, const RgpTraceFileInfo& traceInfo);

private:
    QString m_profiledProcessName;                  ///< The current profiling target's executable filename.
    QString m_processId;                            ///< The current profiling target's process Id.
    QString m_api;                                  ///< The current profiling target's graphics API.
    QString m_clientId;                             ///< The current profiling target's DevDriver Client Id.
    QString m_traceOutputPath;                      ///< The directory that the trace files will be dumped to.
    QString m_pathToRgp;                            ///< The filepath to the installed RGP executable on the local machine.
    ApplicationSettingsModel* m_pSettingsModel;     ///< The settings file where the chosen settings are serialized.
    RGPRecentTraceListModel* m_pRecentTracesModel;  ///< The recent trace files ListView model.
    RGPClientProcessorThread* m_pRequestWorker;     ///< A thread worker object used to make requests with RGPClient.
    QThread* m_pRgpClientProcessorThread;           ///< The thread responsible for the RGPClient request thread worker.
};

#endif // _RGP_TRACE_MODEL_H_
