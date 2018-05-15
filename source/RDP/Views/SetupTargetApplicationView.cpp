//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of the Setup Target Application panel interface.
//=============================================================================

#include <QStandardItemModel>
#include <QFileDialog>
#include <QFontMetrics>
#include <QGridLayout>
#include <QSpacerItem>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QMimeData>

#include "SetupTargetApplicationView.h"
#include "ui_SetupTargetApplicationView.h"
#include "../RDPDefinitions.h"
#include "../Settings/RDPSettings.h"
#include "../Util/RDPUtil.h"
#include "../Models/SetupTargetApplicationModel.h"
#include "../Models/DeveloperPanelModel.h"
#include <QtUtil.h>

//-----------------------------------------------------------------------------
/// Explicit constructor
/// \param pPanelModel The panel model.
/// \param pParent The parent widget.
//-----------------------------------------------------------------------------
SetupTargetApplicationView::SetupTargetApplicationView(DeveloperPanelModel* pPanelModel, QWidget* pParent) :
    QWidget(pParent),
    ui(new Ui::SetupTargetApplicationView),
    m_pSetupTargetApplicationModel(nullptr),
    m_traceInProgress(false)
{
    ui->setupUi(this);

    // Create a model for the table view.
    m_pSetupTargetApplicationModel = new SetupTargetApplicationModel();
    Q_ASSERT(m_pSetupTargetApplicationModel != nullptr);
    QtCommon::QtUtil::ApplyStandardTableStyle(ui->TargetApplicationList);
    ui->TargetApplicationList->setModel(m_pSetupTargetApplicationModel->GetTableModel());
    ui->TargetApplicationList->SetTargetApplicationModel(m_pSetupTargetApplicationModel);
    Q_ASSERT(pPanelModel);
    pPanelModel->SetTargetApplicationsModel(m_pSetupTargetApplicationModel);

    // Set up signals/slots.
    connect(ui->addToListButton, &QPushButton::clicked, this, &SetupTargetApplicationView::AddToList);
    connect(ui->targetExeLineEdit, &QLineEdit::returnPressed, this, &SetupTargetApplicationView::OnReturnPressedOnExecutableLine);
    connect(ui->removeFromListButton, &QPushButton::clicked, this, &SetupTargetApplicationView::RemoveFromList);
    connect(ui->targetExeBrowseButton, &QPushButton::clicked, this, &SetupTargetApplicationView::OnTargetExeBrowseButtonPressed);
    connect(ui->TargetApplicationList, &AppListTreeView::clicked, this, &SetupTargetApplicationView::OnApplicationSelected);
    connect(ui->targetExeLineEdit, &QLineEdit::textChanged, this, &SetupTargetApplicationView::OnTargetExeLineEditTextChanged);

    // Disable the remove from list button for now
    ui->removeFromListButton->setEnabled(false);

    // Disable the add to list button for now
    ui->addToListButton->setEnabled(false);

    // Enable sorting for target application list
    ui->TargetApplicationList->setSortingEnabled(true);
    ui->TargetApplicationList->sortByColumn(TARGET_APPLICATION_TABLE_COLUMN_EXECUTABLE_NAME, Qt::AscendingOrder);

    // Update the model.
    m_pSetupTargetApplicationModel->Update();
    AdjustTableColumns();

    setAcceptDrops(true);
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
SetupTargetApplicationView::~SetupTargetApplicationView()
{
    SAFE_DELETE(ui);
    SAFE_DELETE(m_pSetupTargetApplicationModel);
}

//-----------------------------------------------------------------------------
/// Slot to handle what happens when the user clicks the 'Add to list'
/// button
/// \param clicked Whether the button is clicked
//-----------------------------------------------------------------------------
void SetupTargetApplicationView::AddToList(bool clicked)
{
    Q_UNUSED(clicked);
    QString applicationFilepath = ui->targetExeLineEdit->text();

    if (applicationFilepath.isEmpty())
    {
        RDPUtil::ShowNotification(gs_PRODUCT_NAME_STRING, gs_NO_FILE_SPECIFIED_TEXT, NotificationWidget::Button::Ok);
    }
    else
    {
        // Remove any leading and trailing whitespace from the specified executable.
        applicationFilepath = applicationFilepath.trimmed();

        AddExecutableToList(applicationFilepath);
    }
}

//-----------------------------------------------------------------------------
/// Add the given executable filename to the Target Applications table.
/// \param executableFilename The executable filename to add to the target list.
/// \returns True if the new item was added to the list successfully, and false if it failed.
//-----------------------------------------------------------------------------
bool SetupTargetApplicationView::AddExecutableToList(const QString& executableFilename)
{
    bool addedSuccessfully = m_pSetupTargetApplicationModel->AddApplication(executableFilename);
    if (addedSuccessfully == false)
    {
        RDPUtil::ShowNotification(gs_PRODUCT_NAME_STRING, gs_DUPLICATE_FILE_TEXT, NotificationWidget::Button::Ok);
    }
    else
    {
        // The executable was added successfully, so clear out the textbox.
        ui->targetExeLineEdit->setText("");
        AdjustTableColumns();

        // Select the last application added by the user.
        QModelIndex firstRow = ui->TargetApplicationList->model()->index(0, 0);
        ui->TargetApplicationList->setCurrentIndex(firstRow);

        // Enable the "Remove from list" button
        ui->removeFromListButton->setEnabled(true);
    }

    return addedSuccessfully;
}

//-----------------------------------------------------------------------------
/// Handler invoked when the 'Remove from list' button is clicked.
/// \param clicked Flag that indicates if the button is checked.
//-----------------------------------------------------------------------------
void SetupTargetApplicationView::RemoveFromList(bool clicked)
{
    Q_UNUSED(clicked);

    QModelIndex selectedRowIndex = ui->TargetApplicationList->currentIndex();
    if (selectedRowIndex.isValid())
    {
        int selectedRow = selectedRowIndex.row();
        int sourceModelRow = m_pSetupTargetApplicationModel->MapToSourceModelRow(ui->TargetApplicationList->currentIndex());
        RDPSettings& rdpSettings = RDPSettings::Get();
        // If a trace is currently being collected or this application is currently being profiled, disallow this operation.
        if (m_traceInProgress || (rdpSettings.isAllowTargetApplicationProfiling(sourceModelRow) && !GetSetupTargetApplicationModel()->ActivelyProfiledApplication().isEmpty()) )
        {
            RDPUtil::ShowNotification(gs_DELETE_WHILE_PROFILING_TITLE, gs_DELETE_WHILE_PROFILING_MSG, NotificationWidget::Button::Ok);
            return;
        }

        // Pop up a confirmation dialog box
        NotificationWidget::Button resultButton =
            RDPUtil::ShowNotification(gs_DELETE_APPLICATION_TITLE, gs_DELETE_APPLICATION, NotificationWidget::Button::Yes | NotificationWidget::Button::No, NotificationWidget::Button::No);

        if (resultButton == NotificationWidget::Button::Yes)
        {
            // Get the name of the application being removed
            QString executableName;
            bool result = m_pSetupTargetApplicationModel->GetExecutableNameAtRow(sourceModelRow, executableName);

            m_pSetupTargetApplicationModel->RemoveApplication(selectedRow);
            AdjustTableColumns();

            // Get the model
            QAbstractItemModel* pModel = m_pSetupTargetApplicationModel->GetTableModel();

            // Get the data from the first row/column
            QString text = pModel->data(pModel->index(0, 0)).toString();

            // If the table is empty, disable the "Remove from list" button
            if (text.isEmpty())
            {
                ui->removeFromListButton->setEnabled(false);
            }

            // emit a signal to indicate that an app was removed
            if (result == true)
            {
                emit ApplicationRemovedFromList(executableName);
            }
        }
    }
}

//-----------------------------------------------------------------------------
/// Handler invoked when the user presses the "Enter" key while the target executable textbox has focus.
//-----------------------------------------------------------------------------
void SetupTargetApplicationView::OnReturnPressedOnExecutableLine()
{
    // Add whatever is in the textbox as the next target executable.
    AddToList(false);
}

//-----------------------------------------------------------------------------
/// Target exe browse button pressed SLOT - Determines what to do when the
/// browse button next to the target executable edit field is pressed.
//-----------------------------------------------------------------------------
void SetupTargetApplicationView::OnTargetExeBrowseButtonPressed()
{
    // Fill the dialog with the existing target executable directory.
    const QString& lastAppPath = RDPSettings::Get().GetLastTargetExecutableDirectory();

#ifdef WIN32
    // Create file filters for the windows file dialog box
    QString fileNameFilter = "All files (*.*);;Exe files (*.exe)";
    QString defaultFilter = "Exe files (*.exe)";

    // Open a new file selection dialog so the user can choose a target executable.
    const QString& applicationFilepath = QFileDialog::getOpenFileName(this, gs_BROWSE_APPLICATION_FILEPATH_CAPTION_TEXT, lastAppPath, fileNameFilter, &defaultFilter);
#else
    // Open a new file selection dialog so the user can choose a target executable.
    const QString& applicationFilepath = QFileDialog::getOpenFileName(this, gs_BROWSE_APPLICATION_FILEPATH_CAPTION_TEXT, lastAppPath);
#endif // WIN32

    // getOpenFileName returns a "null" QString if the user cancels the dialog. Only continue if they select something.
    if (!applicationFilepath.isNull())
    {
        // Trim the full path to just the application filename to filter with.
        QFileInfo fileInfo(applicationFilepath);
        QString executableOnly(fileInfo.fileName());

        // Keep the user's last directory to start at next time.
        RDPSettings::Get().SetLastTargetExecutableDirectory(applicationFilepath);

        ui->targetExeLineEdit->setText(executableOnly);

        AddToList(true);
    }
}

//-----------------------------------------------------------------------------
/// Adjust the column widths of the target applications table
//-----------------------------------------------------------------------------
void SetupTargetApplicationView::AdjustTableColumns()
{
    int numRows = m_pSetupTargetApplicationModel->GetTableModel()->rowCount();
    QtCommon::QtUtil::AutoAdjustTableColumns(ui->TargetApplicationList, numRows, 10);
}

//-----------------------------------------------------------------------------
/// Enable the "Remove from list" button when an exe is selected from the list
/// \param index the model index of the clicked item
//-----------------------------------------------------------------------------
void SetupTargetApplicationView::OnApplicationSelected(const QModelIndex& index)
{
    Q_UNUSED(index);

    // A row was clicked on, so enable the "Remove from list" button
    ui->removeFromListButton->setEnabled(true);
}

//-----------------------------------------------------------------------------
/// Enable/disable the "Add to list" button as necessary
/// \param text The text enetered into the line edit
//-----------------------------------------------------------------------------
void SetupTargetApplicationView::OnTargetExeLineEditTextChanged(const QString& text)
{
    // Enable/Disable the add to list button
    ui->addToListButton->setEnabled(!text.isEmpty());
}

//-----------------------------------------------------------------------------
/// Slot invoked when the RGPTraceModel either starts or finishes collecting an RGP trace.
/// \param traceIsBeingCollected A flag that indicates the current state of RGP Trace collection.
//-----------------------------------------------------------------------------
void SetupTargetApplicationView::OnTraceCollectionStatusUpdated(bool traceIsBeingCollected)
{
    m_traceInProgress = traceIsBeingCollected;
}

//-----------------------------------------------------------------------------
/// Return the model object pointer.
/// \return SetupTargetApplicationModel pointer
//-----------------------------------------------------------------------------
SetupTargetApplicationModel* SetupTargetApplicationView::GetSetupTargetApplicationModel()
{
    return m_pSetupTargetApplicationModel;
}

//-----------------------------------------------------------------------------
/// Slot invoked when the user profiling checkbox click is not valid
//-----------------------------------------------------------------------------
void SetupTargetApplicationView::OnProfilingCheckboxClickError()
{
    // Put up an error message
    RDPUtil::ShowNotification(gs_UNCHECK_PROFILE_WHILE_COLLECTING_TRACE_TITLE, gs_UNCHECK_PROFILE_WHILE_COLLECTING_TRACE_MSG, NotificationWidget::Button::Ok);
}

//-----------------------------------------------------------------------------
/// Slot invoked when the user attempts to profile multiple instances of the same application
/// \param profiledProcessInfo Process information for the latest application instance started.
//-----------------------------------------------------------------------------
void SetupTargetApplicationView::OnProfilingMultipleTargetsWarning(const ProcessInfoModel& profiledProcessInfo)
{
    // Put up an warning message
    QString message = QString(gs_MULTIPLE_TARGET_APPLICATION_INSTANCES_MSG).arg(profiledProcessInfo.GetProcessName().toStdString().c_str()).arg(profiledProcessInfo.GetProcessId());
    RDPUtil::ShowNotification(gs_MULTIPLE_TARGET_APPLICATION_INSTANCES_TITLE, message, NotificationWidget::Button::Ok);
}

//-----------------------------------------------------------------------------
/// Slot invoked when the user attempts to select another targe application for profiles while
/// another application is already being profiled.
/// \param processInfo Process information for the latest application instance started.
//-----------------------------------------------------------------------------
void SetupTargetApplicationView::OnProfilerInUseWarning(const ProcessInfoModel& processInfo)
{
    // Put up an warning message
    QString message = QString(gs_PROFILER_ALREADY_IN_USE_MSG).arg(processInfo.GetProcessName().toStdString().c_str()).arg(processInfo.GetProcessId());
    RDPUtil::ShowNotification(gs_PROFILER_ALREADY_IN_USE_TITLE, message, NotificationWidget::Button::Ok);
}

//-----------------------------------------------------------------------------
/// Handle a drag enter event
/// \param pEvent drag enter event
//-----------------------------------------------------------------------------
void SetupTargetApplicationView::dragEnterEvent(QDragEnterEvent* pEvent)
{
    if (pEvent != nullptr)
    {
        if (pEvent->mimeData()->hasUrls())
        {
            pEvent->setDropAction(Qt::LinkAction);
            pEvent->accept();
        }
    }
}

//-----------------------------------------------------------------------------
/// Handle a drag-n-drop event
/// \param pEvent drop event
//-----------------------------------------------------------------------------
void SetupTargetApplicationView::dropEvent(QDropEvent* pEvent)
{
    if (pEvent != nullptr)
    {
        const uint32_t numUrls = pEvent->mimeData()->urls().size();

        if (numUrls == 1)
        {
            const QString potentialExePath = pEvent->mimeData()->urls().at(0).toLocalFile();

            QFileInfo executable(potentialExePath);
            if (executable.exists() && executable.isFile())
            {
                AddExecutableToList(potentialExePath);
            }
        }
    }
}
