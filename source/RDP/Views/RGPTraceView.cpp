//=============================================================================
/// Copyright (c) 2016-2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  An interface used to collect RGP traces.
//=============================================================================

#include <QDesktopServices>
#include <QFile>
#include <QFileDialog>
#include <QInputDialog>
#include <QLineEdit>
#include <QMenu>
#include <QProcess>
#include <QCompleter>
#include <QFileSystemModel>
#include "RGPTraceView.h"
#include "ui_RGPTraceView.h"
#include "../../Common/Util/SystemKeyboardHook.h"
#include <ScalingManager.h>
#include "../Models/ApplicationSettingsModel.h"
#include "../Models/DeveloperPanelModel.h"
#include "../Models/RGPTraceModel.h"
#include "../Models/RGPRecentTraceListModel.h"
#include "../Settings/RDPSettings.h"
#include "../Util/RDPUtil.h"
#include "../RDPDefinitions.h"
#include "../../Common/RestoreCursorPosition.h"
#include <QtUtil.h>

/// Button constants for missing RGP executable dialog
enum RGPMissingExeButton
{
    BUTTON_BROWSE,
    BUTTON_REVERT,
    BUTTON_CANCEL
};

const QString s_LINE_EDIT_WARNING_STYLESHEET = "border: 1px solid red";
const QString s_MISSING_FOLDER_WARNING = "Folder not found";
const QString s_MISSING_PROFILER_WARNING = "Profiler not found";

//-----------------------------------------------------------------------------
/// Constructor for the RGP Trace View interface.
/// \param pPanelModel The main Panel model used to maintain a connection to RDS.
/// \param pApplicationSettingsModel The instance holding the model data for this tab.
/// \param pParent The parent widget for the RGP Trace View.
//-----------------------------------------------------------------------------
RGPTraceView::RGPTraceView(DeveloperPanelModel* pPanelModel, ApplicationSettingsModel* pApplicationSettingsModel, QWidget* pParent)
    : QWidget(pParent)
    , ui(new Ui::RGPTraceView)
    , m_pRGPTraceModel(nullptr)
    , m_pApplicationSettingsModel(pApplicationSettingsModel)
    , m_pProgressWidget(nullptr)
    , m_targetApplicationIsProfilable(false)
    , m_traceInProgress(false)

{
    ui->setupUi(this);

    // Set white background for this pane.
    ToolUtil::SetWidgetBackgroundColor(this, Qt::white);

    if (pApplicationSettingsModel != nullptr)
    {
        m_pRGPTraceModel = pApplicationSettingsModel->GetRgpTraceModel();
        if (m_pRGPTraceModel != nullptr)
        {
            pPanelModel->RegisterProtocolModel(MAIN_PANEL_MODEL_RGP, m_pRGPTraceModel);

            m_pRGPTraceModel->InitializeModel(ui->processName, RGP_TRACE_CONTROLS_PROCESS_NAME, "text");
            m_pRGPTraceModel->InitializeModel(ui->processId, RGP_TRACE_CONTROLS_PROCESS_ID, "text");
            m_pRGPTraceModel->InitializeModel(ui->api, RGP_TRACE_CONTROLS_PROCESS_API, "text");
            m_pRGPTraceModel->InitializeModel(ui->clientId, RGP_TRACE_CONTROLS_PROCESS_CLIENT_ID, "text");
            m_pRGPTraceModel->InitializeModel(ui->traceOutputDirectoryTextbox, RGP_TRACE_CONTROLS_OUTPUT_PATH_STRING, "text");
            m_pRGPTraceModel->InitializeModel(ui->enableDetailedProfilingDataCheckbox, RGP_TRACE_CONTROLS_RGP_DETAILED_TRACE_DATA, "checked");
            m_pRGPTraceModel->InitializeModel(ui->allowComputePresentsCheckbox, RGP_TRACE_CONTROLS_RGP_ALLOW_COMPUTE_PRESENTS, "checked");
            m_pRGPTraceModel->InitializeModel(ui->rgpExecutablePathTextbox, RGP_TRACE_CONTROLS_RGP_FILEPATH_STRING, "text");

            m_pRGPTraceModel->InitializeDefaults();

            // Set the model for the Recent Traces listview.
            RGPRecentTraceListModel* pRecentTracesListModel = m_pRGPTraceModel->GetRecentTraceListModel();
            QtCommon::QtUtil::ApplyStandardTableStyle(ui->recentTracesListView);
            ui->recentTracesListView->setModel(pRecentTracesListModel);

            // Set up column widths.
            AdjustTableColumns();

            // Setup signal/slot connections.
            connect(ui->collectDataButton, &QPushButton::clicked, this, &RGPTraceView::OnCollectTraceClicked);
            connect(ui->browseOutputDirectoryButton, &QPushButton::clicked, this, &RGPTraceView::OnBrowseTraceDirectoryClicked);
            connect(ui->browseToRGPButton, &QPushButton::clicked, this, &RGPTraceView::OnBrowseToRGPButtonClicked);
            connect(ui->traceOutputDirectoryTextbox, &QLineEdit::textChanged, this, &RGPTraceView::OnTraceDirectoryTextboxChanged);
            connect(ui->rgpExecutablePathTextbox, &QLineEdit::textChanged, this, &RGPTraceView::OnRGPFilepathTextboxChanged);
            connect(ui->openInRGPButton, &QPushButton::clicked, this, &RGPTraceView::OnOpenInRGPClicked);
            connect(ui->recentTracesListView, &QTreeView::doubleClicked, this, &RGPTraceView::OnRecentTraceDoubleClicked);
            connect(ui->enableDetailedProfilingDataCheckbox, &QCheckBox::stateChanged, this, &RGPTraceView::OnCollectDetailedTraceDataChanged);
            connect(ui->allowComputePresentsCheckbox, &QCheckBox::stateChanged, this, &RGPTraceView::OnAllowComputePresentsChanged);
            connect(pRecentTracesListModel, &RGPRecentTraceListModel::rowsInserted, this, &RGPTraceView::OnTraceAdded);
            connect(m_pRGPTraceModel, &RGPTraceModel::CurrentlyCollectingTrace, this, &RGPTraceView::OnTraceCollectionStatusUpdated);
            connect(m_pRGPTraceModel, &RGPTraceModel::UpdateCollectRGPTraceButton, this, &RGPTraceView::OnUpdateCollectRGPTraceButton);
            connect(pPanelModel, &DeveloperPanelModel::Disconnected, this, &RGPTraceView::OnRDSDisconnect);
            connect(SystemKeyboardHook::GetInstance(), &SystemKeyboardHook::HotKeyPressed, this, &RGPTraceView::OnHotKeyPressed);

            // Enable hot key for capturing traces.
            SystemKeyboardHook::GetInstance()->SetHotKey(gs_CAPTURE_TRACE_HOTKEY, Qt::ShiftModifier | Qt::ControlModifier);
            SystemKeyboardHook::GetInstance()->Connect();

            // Hide options that are shown only in internal builds.
            ui->internalProfilingOptionsPane->hide();

            // Hide hotkey label if not enabled.
            if (SystemKeyboardHook::GetInstance()->Enabled() == false)
            {
                ui->hotkeyLabel->hide();
            }

            UpdateTraceDirectoryStatus(m_pRGPTraceModel->GetTraceOutputPath());
            UpdateRgpExecutablePathStatus(m_pRGPTraceModel->GetPathToRgp());

            // Connect a ContextMenu handler to the Recent Traces table.
            connect(ui->recentTracesListView, &QTreeView::customContextMenuRequested, this, &RGPTraceView::OnShowRecentTracesContextMenu);
            ui->recentTracesListView->setContextMenuPolicy(Qt::CustomContextMenu);

            // Disable "Open profile" button for now
            ui->openInRGPButton->setEnabled(false);
        }
    }
}

//-----------------------------------------------------------------------------
/// Destructor for the RGP Trace View interface.
//-----------------------------------------------------------------------------
RGPTraceView::~RGPTraceView()
{
    SystemKeyboardHook::GetInstance()->Disconnect();
    SAFE_DELETE(m_pRGPTraceModel);

    delete ui;
}

//-----------------------------------------------------------------------------
/// Update the connected ClientId in the model.
/// \param clientId The new clientId the view is connected to.
//-----------------------------------------------------------------------------
void RGPTraceView::OnHotKeyPressed(uint key)
{
    Q_UNUSED(key);
    if ( (ui->collectDataButton->isEnabled()) && (!m_traceInProgress) )
    {
        RDPUtil::DbgMsg("[RDP] Hot key pressed - capture profile");
        OnCollectTraceClicked(true);
    }
}

//-----------------------------------------------------------------------------
/// Update the connected ClientId in the model.
/// \param clientId The new clientId the view is connected to.
//-----------------------------------------------------------------------------
void RGPTraceView::OnClientIdUpdated(DevDriver::ClientId clientId)
{
    m_pRGPTraceModel->SetConnectedClientId(clientId);
}

//-----------------------------------------------------------------------------
/// Open a File Browse window to choose where trace files will be dumped to.
/// \param checked A bool indicating if the button is checked or not.
//-----------------------------------------------------------------------------
void RGPTraceView::OnBrowseTraceDirectoryClicked(bool checked)
{
    Q_UNUSED(checked);

    // Fill the dialog with the existing trace output path.
    const QString& outputPathDir = m_pRGPTraceModel->GetTraceOutputPath();

    // Open a new folder selection dialog so the user can choose a new output directory.
    const QString& updatedTraceDirectory = QDir::toNativeSeparators(QFileDialog::getExistingDirectory(this, gs_BROWSE_TRACE_DIRECTORY_CAPTION_TEXT,
        outputPathDir, QFileDialog::Option::ShowDirsOnly | QFileDialog::Option::DontResolveSymlinks));

    // Only update the trace path if the user's chosen path is valid - empty string indicates a cancel operation
    if (!updatedTraceDirectory.isEmpty())
    {
        // Make sure the user has write permissions for this location
        if (!IsDirectoryWritable(updatedTraceDirectory))
        {
            // Display a pop up message to indicate the error
            ShowDirectoryNotWritableNotification(updatedTraceDirectory);

            return;
        }

        // Don't bother updating if it's the same thing.
        if (updatedTraceDirectory.compare(outputPathDir) != 0)
        {
            // Set the RGP Trace model's output directory to the one the user just selected.
            m_pRGPTraceModel->Update(RGP_TRACE_CONTROLS_OUTPUT_PATH_STRING, updatedTraceDirectory);
        }
    }
}

//-----------------------------------------------------------------------------
/// Open a File Browse window to choose where the local RGP installation lives.
/// \param checked A bool indicating if the button is checked or not.
//-----------------------------------------------------------------------------
void RGPTraceView::OnBrowseToRGPButtonClicked(bool checked)
{
    Q_UNUSED(checked);

    // Fill the dialog with the existing trace output path.
    const QString& lastRgpPath = RDPSettings::Get().GetPathToRGP();

    // Open a new folder selection dialog so the user can choose a new output directory.
    const QString& rgpInstallPath = QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, gs_BROWSE_RGP_INSTALL_PATH, lastRgpPath));

    if (!rgpInstallPath.isEmpty())
    {
        if (lastRgpPath.compare(rgpInstallPath) != 0)
        {
            // Keep the user's last directory to start at next time.
            RDPSettings::Get().SetPathToRGP(rgpInstallPath);

            m_pRGPTraceModel->Update(RGP_TRACE_CONTROLS_RGP_FILEPATH_STRING, rgpInstallPath);
        }
    }
}

//-----------------------------------------------------------------------------
/// Update the RGP collection interface controls to enable/disable based on
/// whether or not collecting a trace is possible.
//-----------------------------------------------------------------------------
void RGPTraceView::UpdateTraceCollectionControls()
{
    // The "Collect Trace" button should only be enabled if there's an
    // active target application with profiling enabled, and if no trace
    // is actively being captured.
    bool shouldEnableTraceControls = m_targetApplicationIsProfilable && !m_traceInProgress;

    // The "Capture RGP trace" button is only available to click when profiling is
    // enabled and there isn't an active trace being collected.
    bool wasEnabled = ui->collectDataButton->isEnabled();
    ui->collectDataButton->setEnabled(shouldEnableTraceControls);

    // Only report the state of the capture trace button if the enabled state changes.
    if (wasEnabled != shouldEnableTraceControls)
    {
        if (shouldEnableTraceControls)
        {
            RDPUtil::DbgMsg("[RDP] Capture profile button is enabled because the target application is profilable and there is no profile in progress.");
        }
        else
        {
            QString disabledReason;
            if (!m_targetApplicationIsProfilable)
            {
                disabledReason += "the application is not profilable";
            }

            if (m_traceInProgress)
            {
                if (!disabledReason.isEmpty())
                {
                    disabledReason += " and ";
                }
                disabledReason += "there is an active profile in progress";
            }

            std::string disabledReasonString = disabledReason.toStdString();
            RDPUtil::DbgMsg("[RDP] Capture profile button has been disabled because %s.", disabledReasonString.c_str());
        }
    }

    ui->hotkeyLabel->setEnabled(shouldEnableTraceControls);

    // Disable the disconnect button while there is a trace in progress
    RDPUtil::SetDisconnectButtonEnabled(!m_traceInProgress);

    // The Capture button and the progress bar are swapped depending on if there's a trace in progress.
    if (m_traceInProgress)
    {
        ShowProgressWidget();
    }
    else
    {
        HideProgressWidget();
    }
}

//-----------------------------------------------------------------------------
/// Handler invoked when the user clicks the "Collect Trace" button.
/// \param checked A bool indicating if the button is checked or not.
//-----------------------------------------------------------------------------
void RGPTraceView::OnCollectTraceClicked(bool checked)
{
    Q_UNUSED(checked);

    //  Before collecting the trace, make sure that the target folder is writable
    if (IsDirectoryWritable(m_pRGPTraceModel->GetTraceOutputPath()))
    {
        m_pRGPTraceModel->CollectRGPTrace();
    }
    else
    {
        // Display a pop up message to indicate the error
        ShowDirectoryNotWritableNotification(m_pRGPTraceModel->GetTraceOutputPath());
    }
}

//-----------------------------------------------------------------------------
/// Handler invoked when the user alters the check state on the "Detailed trace data" checkbox.
/// \param checkState The new check state of the checkbox.
//-----------------------------------------------------------------------------
void RGPTraceView::OnCollectDetailedTraceDataChanged(int checkState)
{
    bool checked = (checkState == Qt::Checked);
    if (checked)
    {
        ui->detailedTraceDataLabel->setText(gs_ON_TEXT);
    }
    else
    {
        ui->detailedTraceDataLabel->setText(gs_OFF_TEXT);
    }
    m_pRGPTraceModel->Update(RGP_TRACE_CONTROLS_RGP_DETAILED_TRACE_DATA, checked);
}

//-----------------------------------------------------------------------------
/// Handler invoked when the user alters the check state on the "Allow compute presents" checkbox.
/// \param checkState The new check state of the checkbox.
//-----------------------------------------------------------------------------
void RGPTraceView::OnAllowComputePresentsChanged(int checkState)
{
    bool checked = (checkState == Qt::Checked);
    if (checked)
    {
        ui->allowComputePresentsValueLabel->setText(gs_ON_TEXT);
    }
    else
    {
        ui->allowComputePresentsValueLabel->setText(gs_OFF_TEXT);
    }
    m_pRGPTraceModel->Update(RGP_TRACE_CONTROLS_RGP_ALLOW_COMPUTE_PRESENTS, checked);
}

//-----------------------------------------------------------------------------
/// Handler invoked when the text in the trace output path is changed.
/// \param pathString The updated string provided by the Output Path textbox.
//-----------------------------------------------------------------------------
void RGPTraceView::OnTraceDirectoryTextboxChanged(const QString& pathString)
{
    RestoreCursorPosition cacheCursorPosition(ui->traceOutputDirectoryTextbox);
    UpdateTraceDirectoryStatus(pathString);
    m_pRGPTraceModel->Update(RGP_TRACE_CONTROLS_OUTPUT_PATH_STRING, pathString);
}

//-----------------------------------------------------------------------------
/// Handler invoked when the text in the RGP filepath textbox is changed.
/// \param filepathString The updated filepath to the local RGP install executable.
//-----------------------------------------------------------------------------
void RGPTraceView::OnRGPFilepathTextboxChanged(const QString& filepathString)
{
    RestoreCursorPosition cacheCursorPosition(ui->rgpExecutablePathTextbox);
    UpdateRgpExecutablePathStatus(filepathString);
    m_pRGPTraceModel->Update(RGP_TRACE_CONTROLS_RGP_FILEPATH_STRING, filepathString);
}

//-----------------------------------------------------------------------------
/// Update the RGP executable path status label.  Displays warning if filename path is invalid.
/// \param filenamePathString The filename and path where RGP should exist.
//-----------------------------------------------------------------------------
void RGPTraceView::UpdateRgpExecutablePathStatus(const QString& filenamePathString)
{
    if (ValidateFilenamePath(filenamePathString, ValidateExecutable) == true)
    {
        ui->rgpExecutablePathTextbox->setStyleSheet("");
        ui->profilerPathStatusLabel->setText("");
    }
    else
    {
        ui->rgpExecutablePathTextbox->setStyleSheet(s_LINE_EDIT_WARNING_STYLESHEET);
        ui->profilerPathStatusLabel->setText(s_MISSING_PROFILER_WARNING);
    }
}

//-----------------------------------------------------------------------------
/// Update the trace directory status label.  Displays warning if directory is invalid.
/// \param pathString The folder to be tested.
//-----------------------------------------------------------------------------
void RGPTraceView::UpdateTraceDirectoryStatus(const QString& pathString)
{
    if (ValidateFilenamePath(pathString, ValidateDirectory) == true)
    {
        ui->traceOutputDirectoryTextbox->setStyleSheet("");
        ui->profileDirectoryStatusLabel->setText("");
    }
    else
    {
        ui->traceOutputDirectoryTextbox->setStyleSheet(s_LINE_EDIT_WARNING_STYLESHEET);
        ui->profileDirectoryStatusLabel->setText(s_MISSING_FOLDER_WARNING);
    }
}

//-----------------------------------------------------------------------------
/// Handler invoked when the "Open profile" button is clicked.
/// \param checked A bool indicating if the button is checked or not.
//-----------------------------------------------------------------------------
void RGPTraceView::OnOpenInRGPClicked(bool checked)
{
    Q_UNUSED(checked);

    const QModelIndex& currentIndex = ui->recentTracesListView->selectionModel()->currentIndex();
    if (currentIndex.isValid())
    {
        bool opened = OpenRecentTraceAtModelIndex(currentIndex);
        if (!opened)
        {
            RDPUtil::DbgMsg("[RDP] Failed to open the profile in the Radeon GPU Profiler.");
        }
    }
}

//-----------------------------------------------------------------------------
/// Handler invoked when a recently collected trace file is double clicked.
/// \param index The index of the model that was double clicked.
//-----------------------------------------------------------------------------
void RGPTraceView::OnRecentTraceDoubleClicked(const QModelIndex& index)
{
    if (index.isValid())
    {
        bool opened = OpenRecentTraceAtModelIndex(index);
        if (!opened)
        {
            RDPUtil::DbgMsg("[RDP] Failed to open the profile in the Radeon GPU Profiler.");
        }
    }
}

//-----------------------------------------------------------------------------
/// Check if a trace file exists, when given the trace's row in the recent trace table.
/// \param recentTraceRow The row index of the trace in the recent traces table.
/// \param traceFileInfo The file info for the recently collected trace.
/// \returns true if the trace file still exists on disk, and false if it can't be found.
//-----------------------------------------------------------------------------
bool RGPTraceView::DoesRecentTraceExistOnDisk(int recentTraceRow, RgpTraceFileInfo& traceFileInfo)
{
    bool fileExists = false;

    bool gotInfo = m_pRGPTraceModel->GetRecentTraceListModel()->GetTraceInfoByIndex(recentTraceRow, traceFileInfo);
    if (gotInfo)
    {
        // If we got the file info, check that the file still exists on disk.
        QFileInfo fileInfo(traceFileInfo.fullPathToFile);
        if (fileInfo.exists())
        {
            fileExists = true;
        }
    }
    else
    {
        RDPUtil::DbgMsg("[RDP] Failed to retrieve info for trace at row %d.", recentTraceRow);
    }

    return fileExists;
}

//-----------------------------------------------------------------------------
/// Open the "Collecting RGP trace..." panel.
//-----------------------------------------------------------------------------
void RGPTraceView::ShowProgressWidget()
{
    // If non-null, the progress meter is already visible.
    if (m_pProgressWidget == nullptr)
    {
        ui->captureAndProgressWidgetLayout->removeWidget(ui->collectDataButton);
        ui->collectDataButton->hide();
        ui->hotkeyLabel->hide();

        // Create the capture progress widget and add it to the tracing interface.
        m_pProgressWidget = new CaptureProgressWidget(this);
        ScalingManager::Get().RegisterObject(m_pProgressWidget);
        ui->captureAndProgressWidgetLayout->addWidget(m_pProgressWidget);
        m_pProgressWidget->show();

        // Connect the slot to update the progress UI based on signals from the underlying worker in the model.
        connect(m_pRGPTraceModel, &RGPTraceModel::TraceProgressInfoUpdated, m_pProgressWidget, &CaptureProgressWidget::OnTraceProgressUpdated);

        // Connect the cancel button to the model to cancel a trace in progress.
        connect(m_pProgressWidget, &CaptureProgressWidget::TraceCancelled, m_pRGPTraceModel, &RGPTraceModel::OnTraceRequestCanceled);
    }
}

//-----------------------------------------------------------------------------
/// Close the "Collecting RGP trace..." panel.
//-----------------------------------------------------------------------------
void RGPTraceView::HideProgressWidget()
{
    if (m_pProgressWidget != nullptr)
    {
        ui->captureAndProgressWidgetLayout->removeWidget(m_pProgressWidget);
        SAFE_DELETE(m_pProgressWidget);
        ui->captureAndProgressWidgetLayout->addWidget(ui->collectDataButton);
        ui->captureAndProgressWidgetLayout->addWidget(ui->hotkeyLabel);
        ui->collectDataButton->show();

        // show hotkey label if enabled
        if (SystemKeyboardHook::GetInstance()->Enabled() == true)
        {
            ui->hotkeyLabel->show();
        }
    }
}

//-----------------------------------------------------------------------------
/// Open a recently collected trace in RGP based on the given recent trace model index.
/// \param recentTraceIndex The model index of the recently collected RGP trace.
/// \returns True if RGP successfully opened the target file.
//-----------------------------------------------------------------------------
bool RGPTraceView::OpenRecentTraceAtModelIndex(const QModelIndex& recentTraceIndex)
{
    bool opened = false;
    int traceRow = recentTraceIndex.row();

    RGPRecentTraceListModel* pRecentTraceModel = m_pRGPTraceModel->GetRecentTraceListModel();

    RgpTraceFileInfo recentTraceInfo = {};
    if (pRecentTraceModel->GetTraceInfoByIndex(traceRow, recentTraceInfo))
    {
        // Attempt to open a new instance of RGP using the selected trace file as an argument.
        QString rgpFilename = RDPSettings::Get().GetPathToRGP();

        // If RGP executable does not exist, put up a message box
        QFileInfo rgpFile(rgpFilename);
        if (!rgpFile.exists())
        {
            // Display missing executable message box and determine user action
            NotificationWidget::Button resultButton = RDPUtil::ShowNotification(
                gs_RGP_EXE_DIALOG_TITLE,
                gs_RGP_EXE_MISSING_MESSAGE.arg(rgpFilename),
                NotificationWidget::Button::Browse | NotificationWidget::Button::Revert | NotificationWidget::Button::Cancel, NotificationWidget::Button::Cancel);

            // Determine what to do based on the button pressed
            switch (resultButton)
            {
                // Browse for executable
                case NotificationWidget::Button::Browse:
                {
                    // Create a file dialog to get a file path
                    const QString& lastRgpPath = RDPSettings::Get().GetPathToRGP();
                    rgpFilename = QFileDialog::getOpenFileName(this, gs_BROWSE_RGP_INSTALL_PATH, lastRgpPath);
                    rgpFile.setFile(rgpFilename);

                    // Check that the path isn't empty
                    if (!rgpFilename.isEmpty())
                    {
                        RDPSettings::Get().SetPathToRGP(rgpFilename);
                        m_pRGPTraceModel->Update(RGP_TRACE_CONTROLS_RGP_FILEPATH_STRING, rgpFilename);
                    }
                    else
                    {
                        return false;
                    }
                }
                break;

                // Revert to default executable path
                case NotificationWidget::Button::Revert:
                {
                    // Get file at default path
                    rgpFilename = RDPUtil::GetDefaultRGPPath();
                    rgpFile.setFile(rgpFilename);

                    // If default still doesn't exist display an error message box
                    if (rgpFile.exists())
                    {
                        RDPSettings::Get().SetPathToRGP(rgpFilename);
                        m_pRGPTraceModel->Update(RGP_TRACE_CONTROLS_RGP_FILEPATH_STRING, rgpFilename);
                    }
                    else
                    {
                        RDPUtil::ShowNotification(gs_RGP_EXE_DIALOG_TITLE, gs_DEFAULT_RGP_EXE_MISSING_MESSAGE.arg(rgpFilename), NotificationWidget::Button::Ok);
                        return false;
                    }
                }
                break;

                // Cancel
                case NotificationWidget::Button::Cancel:
                default:
                {
                    return false;
                }
            }
        }

        // If RGP executable name is missing, display a message stating so.
        if (!rgpFile.isFile())
        {
            RDPUtil::ShowNotification(gs_RGP_EXE_NAME_MISSING_DIALOG_TITLE, gs_RGP_EXE_NAME_MISSING_MESSAGE_1 + gs_RGP_EXE_NAME_MISSING_MESSAGE_2, NotificationWidget::Button::Ok);
            return false;
        }

        // Check that the paths to RGP and the file being opened are valid.
        bool rgpExecutablePathExists = ToolUtil::CheckFilepathExists(rgpFilename);

        // Verify that the trace file exists on disk.
        QFileInfo fileInfo(recentTraceInfo.fullPathToFile);
        bool validPathToTraceFile = fileInfo.exists();

        // If the path isn't valid, the user has moved/deleted it outside of RDP, and the file can't be opened.
        if (!validPathToTraceFile)
        {
            // File the existed previously, but doesn't anymore. Display a message and remove from the recent traces table.
            RDPUtil::ShowNotification(gs_RECENT_CONNECTIONS_PROFILE_DOESNT_EXIST_TITLE, gs_RGP_TRACE_FILE_MISSING_TEXT, NotificationWidget::Button::Ok);

            RemoveRecentTraceRow(traceRow);
            opened = false;
        }

        if (rgpExecutablePathExists && validPathToTraceFile && !rgpFilename.isEmpty())
        {
            QStringList rgpArgs;
            rgpArgs << recentTraceInfo.fullPathToFile;

            QProcess* pRgpProcess = new QProcess(this);
            if (pRgpProcess != nullptr)
            {
                opened = pRgpProcess->startDetached(rgpFilename, rgpArgs);
                if (!opened)
                {
                    const std::string& recentTraceFilenameString = recentTraceInfo.fullPathToFile.toStdString();
                    RDPUtil::DbgMsg("[RDP] Failed to launch the Radeon GPU Profiler with profile %s.", recentTraceFilenameString.c_str());
                }
            }
        }
    }

    return opened;
}

//-----------------------------------------------------------------------------
/// A handler invoked when a profilable target executable has been detected,
/// and profiling was successfully enabled.
/// \param processInfoModel The Process Info for the new halted process.
//-----------------------------------------------------------------------------
void RGPTraceView::OnProfilingTargetUpdated(const ProcessInfoModel& processInfoModel)
{
    QString processNameString     = gs_DASH_TEXT;
    QString processIdString       = gs_DASH_TEXT;
    QString processAPIString      = gs_DASH_TEXT;
    QString processClientIdString = gs_DASH_TEXT;

    bool traceCollectionEnabled = processInfoModel.GetConnectedStatus();
    if (traceCollectionEnabled)
    {
        processNameString = processInfoModel.GetProcessName();
        processIdString = QString::number(processInfoModel.GetProcessId());

        // Parse the process description to extract the graphics API.
        processAPIString = processInfoModel.GetAPI();

        processClientIdString = QString::number(processInfoModel.GetMostRecentClientId());

        // Flip to the profiling tab.
        RDPUtil::OpenProfilingTab();
    }

    // Only display the current process info and allow the user to collect a trace if the application was successfully enabled for profiling.
    m_pRGPTraceModel->Update(RGP_TRACE_CONTROLS_PROCESS_NAME, processNameString);
    m_pRGPTraceModel->Update(RGP_TRACE_CONTROLS_PROCESS_ID, processIdString);
    m_pRGPTraceModel->Update(RGP_TRACE_CONTROLS_PROCESS_API, processAPIString);
    m_pRGPTraceModel->Update(RGP_TRACE_CONTROLS_PROCESS_CLIENT_ID, processClientIdString);

    m_targetApplicationIsProfilable = traceCollectionEnabled;
    UpdateTraceCollectionControls();
}

//-----------------------------------------------------------------------------
/// ContextMenu handler invoked when the user right-clicks on the Recent Traces table.
/// \param pos The position of the click.
//-----------------------------------------------------------------------------
void RGPTraceView::OnShowRecentTracesContextMenu(const QPoint& pos)
{
    // Get global position
    QPoint globalPos = QCursor::pos();

    // Get the index of the clicked cell.
    QModelIndex selectedCellIndex = ui->recentTracesListView->indexAt(pos);

    // Did the user select a valid row in the recent traces list?
    bool validRow = selectedCellIndex.isValid();

    // Create Context Menu. The menu options are only enabled if the user right-clicked a valid row.
    QMenu menu;

    // Add the "Open profile" entry.
    QAction* pOpenInRGPAction = menu.addAction(gs_RECENT_TRACE_CONTEXT_MENU_OPEN_TEXT);
    pOpenInRGPAction->setEnabled(validRow);

    // Add the "Show in File Browser" entry.
    QAction* pOpenInFileBrowserAction = menu.addAction(gs_RECENT_TRACE_CONTEXT_MENU_SHOW_IN_FILE_BROWSER);
    pOpenInFileBrowserAction->setEnabled(validRow);

    // Add the "Rename trace file" entry.
    QAction* pRenameFileAction = menu.addAction(gs_RECENT_TRACE_CONTEXT_MENU_RENAME_TEXT);
    pRenameFileAction->setEnabled(validRow);

    // Add the "Delete trace file" entry.
    QAction* pDeleteAction = menu.addAction(gs_RECENT_TRACE_CONTEXT_MENU_DELETE_TEXT);
    pDeleteAction->setEnabled(validRow);

    // Execute menu actions
    QAction* pAction = menu.exec(globalPos);

    // Only open the ContextMenu if the user selected a row that has something in it.
    if (validRow && (pAction != nullptr))
    {
        // Regardless of which cell was clicked, if it was valid, get the index of the 0th column instead.
        int selectedRow = selectedCellIndex.row();

        // Check if the given trace exists. If it doesn't, remove the row and display a message.
        RgpTraceFileInfo traceFileInfo = {};
        bool traceFileExists = DoesRecentTraceExistOnDisk(selectedRow, traceFileInfo);

        selectedCellIndex = selectedCellIndex.sibling(selectedRow, RECENT_TRACE_COLUMN_FILEPATH);

        if (pAction == pOpenInRGPAction)
        {
            if (traceFileExists)
            {
                OpenRecentTraceAtModelIndex(selectedCellIndex);
            }
            else
            {
                // The trace file doesn't exist on disk anymore.
                RDPUtil::ShowNotification(gs_RECENT_CONNECTIONS_PROFILE_DOESNT_EXIST_TITLE, gs_RGP_TRACE_FILE_MISSING_TEXT, NotificationWidget::Button::Ok);
            }
        }
        else if (pAction == pOpenInFileBrowserAction)
        {
            if (traceFileExists)
            {
                // Open file directory.
                QFileInfo fileInfo(traceFileInfo.fullPathToFile);
                QString dirPath = fileInfo.absoluteDir().absolutePath();

                // Opening a directory url launches the OS specific window manager.
                QDesktopServices::openUrl(QUrl::fromLocalFile(dirPath));
            }
            else
            {
                RDPUtil::ShowNotification(gs_RECENT_CONNECTIONS_PROFILE_DOESNT_EXIST_TITLE, gs_RGP_TRACE_FILE_MISSING_FILE_BROWSER_FAILED, NotificationWidget::Button::Ok);
            }
        }
        else if (pAction == pRenameFileAction)
        {
            if (traceFileExists)
            {
                bool renameSuccessful = false;
                while (!renameSuccessful)
                {
                    QFileInfo selectedTraceFileInfo(ui->recentTracesListView->model()->data(selectedCellIndex).toString());
                    const QString& existingFilenameOnly = selectedTraceFileInfo.fileName();

                    // Open a "Rename this trace file" popup where the user will choose a new filename.
                    QString newFilename = QInputDialog::getText(this, gs_RECENT_TRACE_CONTEXT_MENU_RENAME_TITLE, gs_RECENT_TRACE_CONTEXT_MENU_RENAME_MESSAGE, QLineEdit::Normal, existingFilenameOnly);

                    // Trim all leading and trailing whitespace from the file.
                    newFilename = newFilename.trimmed();

                    if (newFilename.isEmpty())
                    {
                        return;
                    }

                    // If the user didn't add the extension to the end of the filename, help them out by adding it for them.
                    if (newFilename.endsWith(gs_RGP_TRACE_EXTENSION, Qt::CaseInsensitive) == false)
                    {
                        newFilename.append(gs_RGP_TRACE_EXTENSION);
                    }

                    // If the user typed in the name of an existing file, disallow the operation
                    // and put up a message box
                    const QString& path = m_pRGPTraceModel->GetTraceOutputPath();
                    QString fileName = path + QDir::separator() + newFilename;
                    QFileInfo newFileInfo(fileName);
                    if (newFileInfo.exists())
                    {
                        RDPUtil::ShowNotification(gs_RGP_TRACE_DUPLICATE_RENAME_DIALOG_TITLE, gs_RGP_TRACE_DUPLICATE_RENAME_DIALOG_MESSAGE, NotificationWidget::Button::Ok);

                        // Add a message to the log window
                        RDPUtil::DbgMsg("[RDP] Failed to rename %s to %s", existingFilenameOnly.toStdString().c_str(), newFilename.toStdString().c_str());

                        continue;
                    }

                    RgpTraceFileInfo openedTraceInfo = {};
                    bool gotInfo = m_pRGPTraceModel->GetRecentTraceListModel()->GetTraceInfoByIndex(selectedRow, openedTraceInfo);
                    if (gotInfo)
                    {
                        const QString& traceFilepath(openedTraceInfo.fullPathToFile);

                        QFileInfo existingTraceInfo(traceFilepath);
                        QString absolutePathNewFilename = existingTraceInfo.absoluteDir().absolutePath();
                        absolutePathNewFilename.append(QDir::separator());
                        absolutePathNewFilename.append(newFilename);
                        absolutePathNewFilename = QDir::toNativeSeparators(absolutePathNewFilename);

                        QFile existingTracefile(traceFilepath);
                        renameSuccessful = existingTracefile.rename(absolutePathNewFilename);
                        if (renameSuccessful)
                        {
                            RDPUtil::DbgMsg("[RDP] Successfully renamed %s to %s", existingFilenameOnly.toStdString().c_str(), newFilename.toStdString().c_str());

                            ui->recentTracesListView->model()->data(selectedCellIndex).setValue(newFilename);
                            absolutePathNewFilename = QDir::fromNativeSeparators(absolutePathNewFilename);

                            // If the file was renamed successfully, rename it in the table.
                            m_pRGPTraceModel->GetRecentTraceListModel()->RenameTraceFile(selectedRow, absolutePathNewFilename);

                            // Also resize the columns since the new file name could be longer than others.
                            int numRows = m_pRGPTraceModel->GetRecentTraceListModel()->rowCount();
                            QtCommon::QtUtil::AutoAdjustTableColumns(ui->recentTracesListView, numRows, 10);
                        }
                        else
                        {
                            // Add a message to the log window
                            RDPUtil::DbgMsg("[RDP] Failed to rename %s to %s", existingFilenameOnly.toStdString().c_str(), newFilename.toStdString().c_str());
                        }
                    }
                }
            }
            else
            {
                // The user wanted to rename the trace file, but it doesn't exist anymore. Display a message, and remove the file from the recent traces table.
                RDPUtil::ShowNotification(gs_RECENT_CONNECTIONS_PROFILE_DOESNT_EXIST_TITLE, gs_RECENT_CONNECTIONS_PROFILE_DOESNT_EXIST_RENAME_TEXT, NotificationWidget::Button::Ok);
            }
        }
        else if (pAction == pDeleteAction)
        {
            if (traceFileExists)
            {
                bool deleteSuccessful = false;

                RgpTraceFileInfo openedTraceInfo = {};
                bool gotInfo = m_pRGPTraceModel->GetRecentTraceListModel()->GetTraceInfoByIndex(selectedRow, openedTraceInfo);
                if (gotInfo)
                {
                    const QString& traceFilepath(openedTraceInfo.fullPathToFile);

                    QFileInfo existingTraceInfo(traceFilepath);

                    const QString& existingFilenameOnly = existingTraceInfo.absoluteFilePath();

                    // Attempt to delete the file the user pointed to.
                    QFile existingTracefile(traceFilepath);
                    bool deleted = existingTracefile.remove();
                    if (deleted)
                    {
                        // If the file was deleted successfully, remove the item from the table.
                        RemoveRecentTraceRow(selectedRow);
                        RDPUtil::DbgMsg("[RDP] Successfully deleted %s.", existingFilenameOnly.toStdString().c_str());
                        deleteSuccessful = true;
                    }
                }

                if (!deleteSuccessful)
                {
                    RDPUtil::DbgMsg("[RDP] Failed to delete profile.");
                }
            }
            else
            {
                RDPUtil::ShowNotification(gs_RECENT_CONNECTIONS_PROFILE_DOESNT_EXIST_TITLE, gs_RECENT_CONNECTIONS_PROFILE_DOESNT_EXIST_DELETE_TEXT, NotificationWidget::Button::Ok);
            }
        }

        // If the file doesn't exist anymore for whatever reason, remove it from the recent traces table.
        if (!traceFileExists)
        {
            RemoveRecentTraceRow(selectedRow);
        }
    }
}

//-----------------------------------------------------------------------------
/// Handler for newly addded traces (from the model's rowsInserted signal).  Forces
/// first item in the list to be visible if it has been scrolled out of view.
/// \param parent The model's parent index.
/// \start The first item added to the model.
/// \end The last item added to the model.
//-----------------------------------------------------------------------------
void RGPTraceView::OnTraceAdded(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);
    ui->recentTracesListView->scrollToTop();
    SelectRecentTraceRow(0);

    // Enable the "Open profile" button
    ui->openInRGPButton->setEnabled(true);

    // Adjust the column widths so the new trace can be seen
    AdjustTableColumns();
}

//-----------------------------------------------------------------------------
/// Handler invoked when the RGPTraceModel either starts or finishes collecting an RGP trace.
/// \param traceIsBeingCollected A flag that indicates the current state of RGP Trace collection.
//-----------------------------------------------------------------------------
void RGPTraceView::OnTraceCollectionStatusUpdated(bool traceIsBeingCollected)
{
    m_traceInProgress = traceIsBeingCollected;

    UpdateTraceCollectionControls();
}

//-----------------------------------------------------------------------------
/// Adjust the column widths of the target applications table
//-----------------------------------------------------------------------------
void RGPTraceView::AdjustTableColumns()
{
    int numRows = m_pRGPTraceModel->GetRecentTraceListModel()->rowCount();
    QtCommon::QtUtil::AutoAdjustTableColumns(ui->recentTracesListView, numRows, 10);
}

//-----------------------------------------------------------------------------
/// Remove the given trace row in the Recent Traces table. Update the UI based on the new row count.
/// \param rowIndex The index of the row to be removed.
//-----------------------------------------------------------------------------
void RGPTraceView::RemoveRecentTraceRow(int rowIndex)
{
    m_pRGPTraceModel->GetRecentTraceListModel()->RemoveRecentTraceRow(rowIndex);

    // Are there any more trace files left?
    bool noTracesLeft = m_pRGPTraceModel->GetRecentTraceListModel()->rowCount() == 0;

    // Does the user have any trace rows selected?
    bool noRowsSelected = ui->recentTracesListView->selectionModel()->selectedRows().size() == 0;

    if (noTracesLeft || noRowsSelected)
    {
        // Disable the "Open trace" button if there's nothing to open, or nothing is selected.
        ui->openInRGPButton->setEnabled(false);
    }
}

//-----------------------------------------------------------------------------
/// Select a numbered row in the recent trace list.
/// \param row The row number to select.
//-----------------------------------------------------------------------------
void RGPTraceView::SelectRecentTraceRow(int row)
{
    QItemSelectionModel* pSelectionModel = ui->recentTracesListView->selectionModel();
    QAbstractItemModel* pDataModel = ui->recentTracesListView->model();

    // Do nothing if row is out of range or models are null
    if (row < 0 || row > pDataModel->rowCount() - 1 ||
        pSelectionModel == nullptr ||
        pDataModel == nullptr)
    {
        return;
    }

    QItemSelectionModel::SelectionFlags flags = QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows;
    pSelectionModel->setCurrentIndex(pDataModel->index(row, 0), flags);
}

//-----------------------------------------------------------------------------
/// Handler invoked when the Collect RGP trace button needs enabled/disabled
/// \param enable The enable/disable boolean
//-----------------------------------------------------------------------------
void RGPTraceView::OnUpdateCollectRGPTraceButton(bool enable)
{
    ui->collectDataButton->setEnabled(enable);
    ui->hotkeyLabel->setEnabled(enable);
}

//-----------------------------------------------------------------------------
/// Handler invoked when the connection to RDS ends.
//-----------------------------------------------------------------------------
void RGPTraceView::OnRDSDisconnect()
{
    // "Trace in progress" and "application profilable" flags are forced off
    m_traceInProgress = false;
    m_targetApplicationIsProfilable = false;

    // Update view to to empty state
    m_pRGPTraceModel->ClearProfilingTargetStatus();
    UpdateTraceCollectionControls();
}

//-----------------------------------------------------------------------------
/// Check to see if the target directory is writable.
/// \param traceDirectory The name of the target directory
//-----------------------------------------------------------------------------
bool RGPTraceView::IsDirectoryWritable(const QString& traceDirectory) const
{
    bool result = false;

    // See if the user can write to the selected folder
    QFile tempFile(traceDirectory + "/tempFile");
    result = tempFile.open(QIODevice::WriteOnly);
    if (result == true)
    {
        // Remove the temp file before moving on
        tempFile.remove();
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Display an error message for directory not writable error.
/// \pararm traceDirectory The location of the chosen directory.
//-----------------------------------------------------------------------------
void RGPTraceView::ShowDirectoryNotWritableNotification(const QString& traceDirectory) const
{
    // Put up a message box stating the target directory is not writable
    RDPUtil::ShowNotification(gs_RGP_DIR_NOT_WRITABLE_TITLE, gs_RGP_DIR_NOT_WRITABLE_MESSAGE.arg(traceDirectory), NotificationWidget::Button::Cancel);
}

//-----------------------------------------------------------------------------
/// Validate that a filename path exists.
/// \pararm filepathString The filename and/or path to varify.
/// \param mode Type of validation to perform (directory, filename path or executable)
/// \return If the path/filename exist then return true, otherwise return false.
//-----------------------------------------------------------------------------
bool RGPTraceView::ValidateFilenamePath(const QString& filePathString, PathValidationMode mode)
{
    QFileInfo pathInfo(filePathString);
    bool result = false;
    const bool validPath = pathInfo.isDir();
    const bool validFilename = pathInfo.isFile();
    const bool exists = pathInfo.exists();
    const bool validExecutable = pathInfo.isExecutable();

	switch(mode)
    {
        case ValidateDirectory:
        {
            if ((validPath == true) && (exists == true))
            {
                result = true;
            }
            break;
        }

        case ValidateFile:
        {
            if ((exists == true) && (validFilename == true))
            {
                result = true;
            }
            break;
        }

        case ValidateExecutable:
        {
            if ((exists == true) && (validFilename == true) && (validExecutable == true))
            {
                result = true;
            }
            break;
        }

        default:
            // Invalid PathValidationMode
            Q_ASSERT(false);
    }
    return result;
}
