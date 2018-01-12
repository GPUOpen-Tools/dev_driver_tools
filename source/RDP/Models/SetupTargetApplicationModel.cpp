//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of the Setup Target Application model.
//=============================================================================

#include <QFileInfo>
#include <QStandardItemModel>
#include <QApplication>
#include <QStandardItem>
#include "SetupTargetApplicationModel.h"
#include "../Settings/RDPSettings.h"
#include "../Util/RDPUtil.h"
#include "../RDPDefinitions.h"

#include <QSortFilterProxyModel>

//-----------------------------------------------------------------------------
/// Constructor
//-----------------------------------------------------------------------------
SetupTargetApplicationModel::SetupTargetApplicationModel()
{
    m_pApplicationsTableModel = new QStandardItemModel(0, TARGET_APPLICATION_TABLE_COLUMN_COUNT);

    // Set up proxy model to allow sorting
    m_pProxyModel = new QSortFilterProxyModel();
    m_pProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_pProxyModel->setSourceModel(m_pApplicationsTableModel);

    m_pApplicationsTableModel->setHorizontalHeaderItem(TARGET_APPLICATION_TABLE_COLUMN_EXECUTABLE_NAME,     new QStandardItem(gs_TARGET_APPLICATION_TABLE_EXECUTABLE_NAME));
//    m_pApplicationsTableModel->setHorizontalHeaderItem(TARGET_APPLICATION_TABLE_COLUMN_PROCESS_ID,  new QStandardItem(gs_TARGET_APPLICATION_TABLE_ID));
//    m_pApplicationsTableModel->setHorizontalHeaderItem(TARGET_APPLICATION_TABLE_COLUMN_TITLE,       new QStandardItem(gs_TARGET_APPLICATION_TABLE_TITLE));
//    m_pApplicationsTableModel->setHorizontalHeaderItem(TARGET_APPLICATION_TABLE_COLUMN_API,         new QStandardItem(gs_TARGET_APPLICATION_TABLE_API));

    QStandardItem* pCellItem;
    pCellItem = new QStandardItem(gs_TARGET_APPLICATION_TABLE_APPLY_SETTINGS);
    pCellItem->setCheckable(true);
    m_pApplicationsTableModel->setHorizontalHeaderItem(TARGET_APPLICATION_TABLE_COLUMN_APPLY_SETTINGS, pCellItem);

    pCellItem = new QStandardItem(gs_TARGET_APPLICATION_TABLE_ENABLE_PROFILING);
    pCellItem->setCheckable(true);
    m_pApplicationsTableModel->setHorizontalHeaderItem(TARGET_APPLICATION_TABLE_COLUMN_ENABLE_PROFILING, pCellItem);

    m_traceInProgress = false;
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
SetupTargetApplicationModel::~SetupTargetApplicationModel()
{
    SAFE_DELETE(m_pApplicationsTableModel);
    SAFE_DELETE(m_pProxyModel);
}

//-----------------------------------------------------------------------------
/// Get a reference to the target application table model
/// \return The SetupTargetApplication proxy model, with the table model as
/// its source
//-----------------------------------------------------------------------------
QAbstractItemModel* SetupTargetApplicationModel::GetTableModel()
{
    return m_pProxyModel;
}

//-----------------------------------------------------------------------------
/// Map a proxy index row in the list of target applications to the source model index row.
/// \param The proxy model's index row.
/// \return The source model index row.
//-----------------------------------------------------------------------------
int SetupTargetApplicationModel::MapToSourceModelRow(const QModelIndex& index) const
{
    // Map proxy row index to source row index
    QModelIndex sourceModelIndex = m_pProxyModel->mapToSource(index);
    return sourceModelIndex.row();
}

//-----------------------------------------------------------------------------
/// Check if the given executable filename already exists in the target applications list.
/// \param executableFilename The filename to search the targets list for.
/// \return True if the given executable filename is already in the targets list, and false if it isn't.
//-----------------------------------------------------------------------------
bool SetupTargetApplicationModel::IsApplicationInTargetList(const QString& executableFilename) const
{
    // Check to make sure that the user isn't trying to add a duplicate application to the list.
    const TargetApplicationVector& targetApps = RDPSettings::Get().GetTargetApplications();
    for (auto& item : targetApps)
    {
        if (item.processName.compare(executableFilename) == 0)
        {
            return true;
        }
    }

    return false;
}

//-----------------------------------------------------------------------------
/// Add an application to the target applications list.
/// \param application The application name, including full path.
/// \return true if file can be added, false if not.
//-----------------------------------------------------------------------------
bool SetupTargetApplicationModel::AddApplication(const QString& application)
{
    if (IsApplicationInTargetList(application))
    {
        return false;
    }

    // file isn't already in the list, so add it
    RDSTargetApplicationInfo appInfo;

    appInfo.processName     = application;
    appInfo.titleName       = gs_DASH_TEXT;
    appInfo.apiName         = gs_DASH_TEXT;
    appInfo.allowProfiling  = false;
    appInfo.applySettings   = true;

    // Add the target executable and save the settings file immediately.
    RDPSettings& rdpSettings = RDPSettings::Get();
    rdpSettings.AddTargetApplication(appInfo);
    rdpSettings.SaveSettings();

    Update();

    // Simulate a click on the "Enable Profiling" column on the latest application added.
    int rowCount = m_pApplicationsTableModel->rowCount();
    if ( (rowCount > 0) && (ActivelyProfiledApplication().isEmpty()) )
    {
        EnableProfilingForRow(0);
    }

    return true;
}

//-----------------------------------------------------------------------------
/// Remove an application from the target applications list.
/// \param rowIndex The index of the row to remove from the target applications list.
//-----------------------------------------------------------------------------
void SetupTargetApplicationModel::RemoveApplication(int proxyRowIndex)
{
    // Map proxy row index to source row index
    QModelIndex proxyModelIndex = m_pProxyModel->index(proxyRowIndex, 0);
    QModelIndex sourceModelIndex = m_pProxyModel->mapToSource(proxyModelIndex);
    int rowIndex = sourceModelIndex.row();

    bool rowRemoved = m_pApplicationsTableModel->removeRow(rowIndex);
    if (rowRemoved)
    {
        RDPSettings& rdpSettings = RDPSettings::Get();
        rdpSettings.RemoveTargetApplication(rowIndex);
        RDPSettings::Get().SaveSettings();
    }
    else
    {
        RDPUtil::DbgMsg("[RDP] Failed to remove application at row %d.", rowIndex);
    }
}

//-----------------------------------------------------------------------------
/// Update the model. Call this when the data for the model gets changed
//-----------------------------------------------------------------------------
void SetupTargetApplicationModel::Update()
{
    const TargetApplicationVector& targetApps = RDPSettings::Get().GetTargetApplications();
    int rowCount = targetApps.size();
    m_pApplicationsTableModel->setRowCount(rowCount);

    for (int loop = 0; loop < rowCount; loop++)
    {
        const RDSTargetApplicationInfo& targetAppInfo = targetApps[loop];
        QFileInfo fileInfo(targetAppInfo.processName);
        SetTableModelData(fileInfo.fileName(), loop, TARGET_APPLICATION_TABLE_COLUMN_EXECUTABLE_NAME);

        // TODO: see if the process is running and if so, fill in the process ID here
        //SetTableModelData("-", loop, TARGET_APPLICATION_TABLE_COLUMN_PROCESS_ID);

        //SetTableModelData(targetAppInfo.m_titleName, loop, TARGET_APPLICATION_TABLE_COLUMN_TITLE);
        //SetTableModelData(targetAppInfo.m_apiName, loop, TARGET_APPLICATION_TABLE_COLUMN_API);

        QModelIndex index;
        QStandardItem* pItem;

        index = m_pApplicationsTableModel->index(loop, TARGET_APPLICATION_TABLE_COLUMN_APPLY_SETTINGS);
        pItem = m_pApplicationsTableModel->itemFromIndex(index);
        pItem->setCheckState(targetAppInfo.applySettings ? Qt::Checked : Qt::Unchecked);
        pItem->setCheckable(true);

        index = m_pApplicationsTableModel->index(loop, TARGET_APPLICATION_TABLE_COLUMN_ENABLE_PROFILING);
        pItem = m_pApplicationsTableModel->itemFromIndex(index);
        pItem->setCheckState(targetAppInfo.allowProfiling ? Qt::Checked : Qt::Unchecked);
        pItem->setCheckable(true);
    }
}

//-----------------------------------------------------------------------------
/// A helper function that toggles the "Enable Profiling" checkbox for the given row.
/// If another row currently has the "Enable Profiling" checkbox set, it will be cleared.
/// \param rowIndex The row of the "Enable Profiling" checkbox to toggle.
//-----------------------------------------------------------------------------
void SetupTargetApplicationModel::ToggleProfilingForRow(int rowIndex)
{
    bool validRow = rowIndex >= 0 && rowIndex < m_pApplicationsTableModel->rowCount();
    if (!validRow)
    {
        return;
    }

    QModelIndex newRowEnableProfilingIndex = m_pApplicationsTableModel->index(rowIndex, TARGET_APPLICATION_TABLE_COLUMN_ENABLE_PROFILING);
    if (newRowEnableProfilingIndex.isValid())
    {
        QStandardItem* pEnableProfilingItem = m_pApplicationsTableModel->itemFromIndex(newRowEnableProfilingIndex);
        if (pEnableProfilingItem != nullptr)
        {
            // Toggle the item's checkbox state.
            pEnableProfilingItem->setCheckState(pEnableProfilingItem->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked);

            QModelIndex proxyIndex = m_pProxyModel->mapFromSource(newRowEnableProfilingIndex);
            TargetApplicationTableClicked(proxyIndex);
        }
    }
}

//-----------------------------------------------------------------------------
/// A helper function that will check the "Enable Profiling" checkbox only for the given row.
/// \param rowIndex The row that will get the "Enable Profiling" checkbox checked.
//-----------------------------------------------------------------------------
void SetupTargetApplicationModel::EnableProfilingForRow(int rowIndex)
{
    bool validRow = rowIndex >= 0 && rowIndex < m_pApplicationsTableModel->rowCount();
    if (!validRow)
    {
        return;
    }

    QModelIndex newRowEnableProfilingIndex = m_pApplicationsTableModel->index(rowIndex, TARGET_APPLICATION_TABLE_COLUMN_ENABLE_PROFILING);
    if (newRowEnableProfilingIndex.isValid())
    {
        QStandardItem* pEnableProfilingItem = m_pApplicationsTableModel->itemFromIndex(newRowEnableProfilingIndex);
        if (pEnableProfilingItem != nullptr)
        {
            pEnableProfilingItem->setCheckState(Qt::Checked);

            // Simulate click on proxy index rather than source - the TargetApplicationTableClicked
            // function expects a proxy index and maps it to the source index, so here we do the
            // reverse so that it is mapped back to the correct source index
            QModelIndex proxyIndex = m_pProxyModel->mapFromSource(newRowEnableProfilingIndex);
            TargetApplicationTableClicked(proxyIndex);
        }
    }
}

//-----------------------------------------------------------------------------
/// Set a cell in the table
/// \param data The data as a string to be added to the table.
/// \param row The destination row.
/// \param column The destination column.
/// \param alignment The alignment for the data
//-----------------------------------------------------------------------------
void SetupTargetApplicationModel::SetTableModelData(const QString& modelData, uint row, uint column, enum Qt::AlignmentFlag alignment)
{
    m_pApplicationsTableModel->setData(m_pApplicationsTableModel->index(row, column), QString(modelData));
    m_pApplicationsTableModel->setData(m_pApplicationsTableModel->index(row, column), (int)alignment | Qt::AlignVCenter, Qt::TextAlignmentRole);
}

//-----------------------------------------------------------------------------
/// Make sure that only 1 profiling checkbox is selected
/// \param proxyIndex The proxy index of the item selected (index as far as
/// the view is concerned)
//-----------------------------------------------------------------------------
void SetupTargetApplicationModel::TargetApplicationTableClicked(const QModelIndex& proxyIndex)
{
    // Get source model index from proxy index
    QModelIndex modelIndex = m_pProxyModel->mapToSource(proxyIndex);

    int column = modelIndex.column();

    RDPSettings& rdpSettings = RDPSettings::Get();

    // allow profiling column
    if (column == TARGET_APPLICATION_TABLE_COLUMN_ENABLE_PROFILING)
    {
        QStandardItem* pItem = m_pApplicationsTableModel->itemFromIndex(modelIndex);
        if (pItem->isCheckable())
        {
            // If checked, set this application as the one to be profiled.
            bool checkState = pItem->checkState() == Qt::Checked;
            // If the user unchecked the profiling checkbox, also remove the
            // process name and process id from profiling tab if the trace
            // is not currently being collected
            if (!isCheckBoxClickValid(pItem))
            {
                return;
            }
            rdpSettings.AllowTargetApplicationProfiling(modelIndex.row(), checkState);
            Update();
            rdpSettings.SaveSettings();
        }
    }
    else if (column == TARGET_APPLICATION_TABLE_COLUMN_APPLY_SETTINGS)
    {
        QStandardItem* pItem = m_pApplicationsTableModel->itemFromIndex(modelIndex);
        if (pItem->isCheckable())
        {
            bool checkState = pItem->checkState() == Qt::Checked;
            rdpSettings.ApplyDriverSettingsState(modelIndex.row(), checkState);
            Update();
            rdpSettings.SaveSettings();
        }
    }
}

//-----------------------------------------------------------------------------
/// Slot invoked when the RGPTraceModel either starts or finishes collecting an RGP trace.
/// \param traceIsBeingCollected A flag that indicates the current state of RGP Trace collection.
//-----------------------------------------------------------------------------
void SetupTargetApplicationModel::OnTraceCollectionStatusUpdated(bool traceIsBeingCollected)
{
    m_traceInProgress = traceIsBeingCollected;
}

//-----------------------------------------------------------------------------
/// Get the name of the actively profiled application
/// \return a QString object containing the name of the application currently
/// being profiiled.
//-----------------------------------------------------------------------------
QString SetupTargetApplicationModel::ActivelyProfiledApplication()
{
    ProcessInfoModel processInfo;
    emit QueryProfiledTargetInfo(processInfo);
    return processInfo.GetProcessName();
}

//-----------------------------------------------------------------------------
/// Figure out if the clicked checkbox is a valid click
/// \param pItem The item to look at
/// \return boolean indicating success or failure
//-----------------------------------------------------------------------------
bool SetupTargetApplicationModel::isCheckBoxClickValid(QStandardItem* pItem)
{
    bool status = false;

    ProcessInfoModel processInfo;
    emit QueryProfiledTargetInfo(processInfo);
    if (!processInfo.GetProcessName().isEmpty())
    {
        // undo the checking that the user just did
        if (pItem->checkState() == Qt::Checked)
        {
            pItem->setCheckState(Qt::Unchecked);
        }
        else
        {
            pItem->setCheckState(Qt::Checked);
        }
        emit ProfilerInUseWarning(processInfo);
        return false;
    }

    if (!m_traceInProgress)
    {
        emit ProfilingCheckboxUnchecked();
        status = true;
    }
    else
    {
        // undo the checking that the user just did
        if (pItem->checkState() == Qt::Checked)
        {
            pItem->setCheckState(Qt::Unchecked);
        }
        else
        {
            pItem->setCheckState(Qt::Checked);
        }

        // emit a signal to put up a message box
        emit ProfilingCheckboxClickError();
    }
    return status;
}

//-----------------------------------------------------------------------------
/// Retrieve the executable filename string for the given row.
/// \param rowIndex The row to retrieve the executable for.
/// \param executableName An out-param containing the executable name in the given row.
/// \returns True if the executable name was retrieved successfully, and false if it wasn't.
//-----------------------------------------------------------------------------
bool SetupTargetApplicationModel::GetExecutableNameAtRow(int rowIndex, QString& executableName) const
{
    bool gotExecutable = false;
    const QModelIndex& rowExecutableNameIndex = m_pApplicationsTableModel->index(rowIndex, TARGET_APPLICATION_TABLE_COLUMN_EXECUTABLE_NAME);

    if (rowExecutableNameIndex.isValid())
    {
        executableName = m_pApplicationsTableModel->data(rowExecutableNameIndex, Qt::DisplayRole).toString();
        gotExecutable = true;
    }

    return gotExecutable;
}
