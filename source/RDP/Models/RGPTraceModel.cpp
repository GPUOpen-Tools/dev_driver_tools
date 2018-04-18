//=============================================================================
/// Copyright (c) 2016-2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  The model used to store RGP trace capture settings.
//=============================================================================

#include <QDateTime>
#include <QDir>
#include <QThread>
#include <QVariant>

#include "ApplicationSettingsModel.h"
#include "DeveloperPanelModel.h"
#include "RGPClientProcessorThread.h"
#include "RGPTraceModel.h"
#include "RGPRecentTraceListModel.h"
#include "../RDPDefinitions.h"
#include "../Settings/RDPSettings.h"
#include "../Util/RDPUtil.h"
#include "../../DevDriverComponents/inc/protocols/rgpClient.h"
#include "../../DevDriverComponents/inc/protocols/rgpProtocol.h"
#include "../../DevDriverComponents/inc/protocols/driverControlClient.h"

using namespace DevDriver;
using namespace DevDriver::RGPProtocol;
using namespace DevDriver::DriverControlProtocol;

//-----------------------------------------------------------------------------
/// Constructor for RGPTraceModel.
/// \param pPanelModel The main Panel model used to maintain a connection to RDS.
/// \param modelCount the number of models required.
//-----------------------------------------------------------------------------
RGPTraceModel::RGPTraceModel(DeveloperPanelModel* pPanelModel, ApplicationSettingsModel* pApplicationSettingsModel, uint32_t modelCount)
    : DriverProtocolModel(pPanelModel, modelCount)
    , m_pSettingsModel(pApplicationSettingsModel)
{
    m_traceOutputPath.clear();
    m_pRecentTracesModel = new RGPRecentTraceListModel();

    // Register the following types with the signal/slot system for use as signal parameters.
    int id = qRegisterMetaType<DevDriver::Result>();
    id = qRegisterMetaType<RgpTraceFileInfo>();
    Q_UNUSED(id);
}

//-----------------------------------------------------------------------------
/// Destructor for RGPTraceModel.
//-----------------------------------------------------------------------------
RGPTraceModel::~RGPTraceModel()
{
    SAFE_DELETE(m_pRecentTracesModel);
}

//-----------------------------------------------------------------------------
/// Initialize default values within the interface.
//-----------------------------------------------------------------------------
void RGPTraceModel::InitializeDefaults()
{
    // The "Profiling target status" section is empty when RDP first starts up.
    ClearProfilingTargetStatus();

    static RDPSettings& rdpSettings = RDPSettings::Get();

    // Does the trace output path still exist? If it doesn't, fall back to the default path.
    QString outputPath = rdpSettings.GetRGPTraceOutputPath();
    QDir outputDirectory(outputPath);
    if (!outputDirectory.exists())
    {
        outputPath = rdpSettings.GetDefaultTraceOutputPath();
        rdpSettings.SetRGPTraceOutputPath(outputPath);
        rdpSettings.SaveSettings();

        std::string outputPathString = outputPath.toStdString();
        RDPUtil::DbgMsg("[RDP] RGP profile output directory no longer exists. Using default output path %s", outputPathString.c_str());
    }

    Update(RGP_TRACE_CONTROLS_OUTPUT_PATH_STRING, outputPath);

    // Make sure that the path to RGP is still valid.
    QString rgpPath = rdpSettings.GetPathToRGP();
    bool filepathExists = ToolUtil::CheckFilepathExists(rgpPath);
    if (!filepathExists)
    {
        // If the existing filepath isn't valid, reset it to an empty path.
        rgpPath.clear();
        rdpSettings.SetPathToRGP(rgpPath);
        rdpSettings.SaveSettings();
    }
    Update(RGP_TRACE_CONTROLS_RGP_FILEPATH_STRING, rgpPath);
}

//-----------------------------------------------------------------------------
/// Update the model value for the incoming index.
/// \param modelIndex The index for the updated interface element.
/// \param value The new value for the updated model.
//-----------------------------------------------------------------------------
void RGPTraceModel::Update(RgpTraceControls modelIndex, const QVariant& value)
{
    SetModelData(modelIndex, value);

    switch (modelIndex)
    {
    case RGP_TRACE_CONTROLS_PROCESS_NAME:
        {
            m_profiledProcessName = value.toString();
        }
        break;
    case RGP_TRACE_CONTROLS_PROCESS_ID:
        {
            m_processId = value.toString();
        }
        break;
    case RGP_TRACE_CONTROLS_PROCESS_API:
        {
            m_api = value.toString();
        }
        break;
    case RGP_TRACE_CONTROLS_PROCESS_CLIENT_ID:
        {
            m_clientId = value.toString();
        }
        break;
    case RGP_TRACE_CONTROLS_OUTPUT_PATH_STRING:
        {
            m_traceOutputPath = value.toString();
            RDPSettings::Get().SetRGPTraceOutputPath(m_traceOutputPath);
        }
        break;
    case RGP_TRACE_CONTROLS_RGP_DETAILED_TRACE_DATA:
        {
            bool checked = value.toBool();
            RDPSettings::Get().SetRGPDetailedInstructionData(checked);
        }
        break;
    case RGP_TRACE_CONTROLS_RGP_ALLOW_COMPUTE_PRESENTS:
        {
            bool checked = value.toBool();
            RDPSettings::Get().SetRGPAllowComputePresents(checked);
        }
        break;
    case RGP_TRACE_CONTROLS_RGP_FILEPATH_STRING:
        {
            m_pathToRgp = value.toString();
            RDPSettings::Get().SetPathToRGP(m_pathToRgp);
        }
        break;
    default:
        Q_ASSERT(false);
        break;
    }
}

//-----------------------------------------------------------------------------
/// Handler used to trigger the collection of an RGP trace.
//-----------------------------------------------------------------------------
void RGPTraceModel::CollectRGPTrace()
{
    RGPClient* pRGPClient = nullptr;

    bool gotRgpClient = GetClientByType(Protocol::RGP, (DevDriver::IProtocolClient**)(&pRGPClient));
    if (gotRgpClient && (pRGPClient != nullptr))
    {
        // First check to make sure the application can be profiled.
        bool canProfileApplication = CanProfileApplication(pRGPClient);

        if (canProfileApplication)
        {
            // Emit a signal that lets the UI know that a trace is being collected.
			// The "Capture trace" button will be disabled while a trace is being collected.
            emit CurrentlyCollectingTrace(true);

            // Create a new background thread and worker to handle the RGP trace request.
            m_pRgpClientProcessorThread = new QThread;

            m_pRgpClientProcessorThread->setObjectName("TraceWorkerThread");
            m_pRequestWorker = new RGPClientProcessorThread(GetPanelModel(), GetConnectedClientId());

            m_pRequestWorker->moveToThread(m_pRgpClientProcessorThread);

            // Connect signals in the request processor to slots in this model.
            connect(m_pRgpClientProcessorThread, &QThread::started, m_pRequestWorker, &RGPClientProcessorThread::OnProcessRequest);
            connect(m_pRequestWorker, &RGPClientProcessorThread::ExecuteTraceFinished, this, &RGPTraceModel::OnExecuteTraceFinished);
            connect(m_pRequestWorker, &RGPClientProcessorThread::TraceProgressInfoUpdated, this, &RGPTraceModel::OnTraceProgressInfoUpdated);

            // Generate a timestamp to append to the trace filename.
            QDateTime rightNow = QDateTime::currentDateTime();
            QString localTime = rightNow.toString("yyyyMMdd-HHmmss");

            // Append the timestamp as a suffix to the end of the generated trace filename.
            const QString traceFilename = GenerateTraceFilename(localTime);

            DevDriver::RGPProtocol::BeginTraceInfo beginTraceInfo = {};
            beginTraceInfo.parameters.flags = {};

            // Only let the user collect detailed trace data in internal builds.
            bool collectDetailedTraceData = false;
            bool allowComputePresents = false;

            // Configure the trace parameter flags. These are chosen by the user within RDP's Profiling tab.
            beginTraceInfo.parameters.flags.enableInstructionTokens = collectDetailedTraceData;
            beginTraceInfo.parameters.flags.allowComputePresents = allowComputePresents;

            beginTraceInfo.parameters.numPreparationFrames = gs_NUM_PREPARATION_FRAMES;

            // Provide the request parameters to the thread worker, and start execution of the thread.
            m_pRequestWorker->ExecuteTraceArguments(traceFilename, rightNow, beginTraceInfo);

            m_pRgpClientProcessorThread->start();
        }
        else
        {
            RDPUtil::DbgMsg("[RDP] Failed to capture profile, because RGPClient wasn't ready.");
        }
    }
    else
    {
        RDPUtil::DbgMsg("[RDP] Failed to acquire client for profiling.");
    }
}

//-----------------------------------------------------------------------------
/// Compute the size in MB when given a byte count.
/// \param fileSizeInBytes The size of a file in bytes.
/// \returns The size of the file in MB.
//-----------------------------------------------------------------------------
float RGPTraceModel::ComputeFileSizeInMB(size_t fileSizeInBytes)
{
    return  static_cast<float>(fileSizeInBytes) / 1024.0f / 1024.0f;
}

//-----------------------------------------------------------------------------
/// Destroy the background thread request worker objects.
//-----------------------------------------------------------------------------
void RGPTraceModel::DestroyWorkerThread()
{
    if (m_pRgpClientProcessorThread != nullptr)
    {
        m_pRgpClientProcessorThread->terminate();
        m_pRgpClientProcessorThread->wait();

        SAFE_DELETE(m_pRequestWorker);

        SAFE_DELETE(m_pRgpClientProcessorThread);
    }

    emit CurrentlyCollectingTrace(false);
}

//-----------------------------------------------------------------------------
/// Check if the connected application can be profiled.
/// \param pRgpClient The RGPClient connected to the application.
/// \returns True if the application can be profiled, and false if it can't.
//-----------------------------------------------------------------------------
bool RGPTraceModel::CanProfileApplication(RGPClient* pRgpClient)
{
    ProfilingStatus profilingStatus = ProfilingStatus::NotAvailable;

    if (pRgpClient != nullptr && pRgpClient->IsConnected())
    {
        // No need to check the return value since the function won't modify
        // the parameter if it fails.
        pRgpClient->QueryProfilingStatus(&profilingStatus);
    }

    return (profilingStatus == DevDriver::RGPProtocol::ProfilingStatus::Enabled);
}

//-----------------------------------------------------------------------------
/// Clear out the profiling target status interface elements so that they contain dashes.
//-----------------------------------------------------------------------------
void RGPTraceModel::ClearProfilingTargetStatus()
{
    // Empty out all of the labels that indicate which process is connected.
    Update(RGP_TRACE_CONTROLS_PROCESS_NAME, gs_DASH_TEXT);
    Update(RGP_TRACE_CONTROLS_PROCESS_ID, gs_DASH_TEXT);
    Update(RGP_TRACE_CONTROLS_PROCESS_API, gs_DASH_TEXT);
    Update(RGP_TRACE_CONTROLS_PROCESS_CLIENT_ID, gs_DASH_TEXT);
}

//-----------------------------------------------------------------------------
/// Generate a new RGP trace filename based on the target executable filename and a timestamp.
/// \returns A newly generated RGP trace filename string.
//-----------------------------------------------------------------------------
const QString RGPTraceModel::GenerateTraceFilename(const QString& timestampSuffix) const
{
    // The filename starts with the the executable filename.
    const QString& executableNameString = m_pSettingsModel->GetFullTargetExecutableProcessName();
    Q_ASSERT(executableNameString.isEmpty() == false);

    const QStringList& executableNameSplit = executableNameString.split(".");
    const QString& applicationFilename = executableNameSplit.first();

    QString filenameString = applicationFilename;
    filenameString.append(gs_DASH_TEXT);
    filenameString.append(timestampSuffix);

    // Append the extension onto the full filepath.
    filenameString.append(gs_RGP_TRACE_EXTENSION);

    // Generate a path starting at the user-specified root, and include the executable name.
    QDir traceFilepath(m_traceOutputPath);
    QString fullTraceFilepath = traceFilepath.filePath(filenameString);

    return fullTraceFilepath;
}

//-----------------------------------------------------------------------------
/// Handler invoked when the RGPClient's ExecuteTrace call is finished and the
/// RGP trace data is ready to be written to disk.
/// \param result The return value of the call to ExecuteTrace.
//-----------------------------------------------------------------------------
void RGPTraceModel::OnExecuteTraceFinished(DevDriver::Result result, const RgpTraceFileInfo& traceInfo)
{
    // If the capture was successful, write the trace file to disk. Otherwise report the error.
    if (result == Result::Success)
    {
        m_pRecentTracesModel->AddRecentTraceFile(traceInfo.fullPathToFile, traceInfo.totalBytes, traceInfo.traceCreationTimestamp);
    }
    else
    {
        const QString resultString = ToolUtil::GetResultString(result);
        RDPUtil::DbgMsg("[RDP] Failed to finish executing profile with code '%s'.", resultString.toStdString().c_str());
        RDPUtil::ShowNotification(gs_RGP_PROFILE_FAILED_TITLE, gs_RGP_PROFILE_FAILED_ERROR.arg(resultString), NotificationWidget::Button::Ok);

        // The trace failed to finish writing correctly- but why?
        const TraceContext& traceContext = m_pRequestWorker->GetTraceContext();
        if (traceContext.abortedReason != PROFILE_ABORTED_REASON_NONE)
        {
            switch (traceContext.abortedReason)
            {
            case PROFILE_ABORTED_REASON_NONE:
                {
                    RDPUtil::DbgMsg("[RDP] All trace chunks written successfully.");
                }
                break;
            case PROFILE_ABORTED_REASON_USER_CANCELLED_TRACE:
                {
                    RDPUtil::DbgMsg("[RDP] User canceled profile collection.");
                }
                break;
            case PROFILE_ABORTED_REASON_LOW_DISK_SPACE:
                {
                    // Let the user know that the trace failed because there's no more disk space.
                    RDPUtil::DbgMsg("[RDP] Failed to complete writing profile due to lack of disk space.");
                    RDPUtil::ShowNotification(gs_RGP_PROFILE_FAILED_TITLE, gs_RGP_PROFILE_FAILED_NO_SPACE_TEXT, NotificationWidget::Button::Ok);
                }
                break;
            default:
                // If this assert fires, the switch needs to updated with a newly added trace error code.
                Q_ASSERT(false);
            }
        }
    }

    DestroyWorkerThread();
}

//-----------------------------------------------------------------------------
/// Handler invoked when the user unchecks the currently selected application
//-----------------------------------------------------------------------------
void RGPTraceModel::OnApplicationUnchecked()
{
    ClearProfilingTargetStatus();

    // Disable the Collect RGP trace button as well
    emit UpdateCollectRGPTraceButton(false);
}

//-----------------------------------------------------------------------------
/// Handler invoked when the trace in progress has a new transfer info update.
/// \param receivedBytes The number of bytes that have been transferred.
/// \param traceSizeInBytes The total byte count in the trace file.
/// \param receiveRateInMb The trace data transfer rate in MB/sec/
//-----------------------------------------------------------------------------
void RGPTraceModel::OnTraceProgressInfoUpdated(quint64 receivedBytes, quint64 traceSizeInBytes, quint64 receiveRateInMb)
{
    // Emit the same signal so that the view can be updated.
    emit TraceProgressInfoUpdated(receivedBytes, traceSizeInBytes, receiveRateInMb);
}

//-----------------------------------------------------------------------------
/// Handler invoked when the user wnats to cancel collecting the trace in progress.
//-----------------------------------------------------------------------------
void RGPTraceModel::OnTraceRequestCanceled()
{
    if (m_pRgpClientProcessorThread != nullptr)
    {
        m_pRequestWorker->SetProfileAborted(PROFILE_ABORTED_REASON_USER_CANCELLED_TRACE);
    }
}
