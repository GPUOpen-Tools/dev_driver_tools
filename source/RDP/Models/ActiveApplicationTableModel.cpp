//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A model used to maintain a list of active developer mode applications.
//=============================================================================

#include <QStandardItemModel>
#include "ActiveApplicationTableModel.h"
#include "../RDPDefinitions.h"

//-----------------------------------------------------------------------------
/// Constructor for ActiveApplicationTableModel.
//-----------------------------------------------------------------------------
ActiveApplicationTableModel::ActiveApplicationTableModel()
{
    m_pActiveApplicationsTableModel = new QStandardItemModel(0, ACTIVE_APPLICATION_TABLE_COLUMN_COUNT);

    m_pActiveApplicationsTableModel->setHorizontalHeaderItem(ACTIVE_APPLICATION_TABLE_COLUMN_EXECUTABLE_NAME, new QStandardItem(gs_TARGET_APPLICATION_TABLE_EXECUTABLE_NAME));
    m_pActiveApplicationsTableModel->setHorizontalHeaderItem(ACTIVE_APPLICATION_TABLE_COLUMN_API, new QStandardItem(gs_TARGET_APPLICATION_TABLE_API));
}

//-----------------------------------------------------------------------------
/// Destructor for ActiveApplicationTableModel.
//-----------------------------------------------------------------------------
ActiveApplicationTableModel::~ActiveApplicationTableModel()
{
    SAFE_DELETE(m_pActiveApplicationsTableModel);
}

//-----------------------------------------------------------------------------
/// Retrieve the executable filename string for the given row.
/// \param rowIndex The row to retrieve the executable for.
/// \param executableName An out-param containing the executable name in the given row.
/// \returns True if the executable name was retrieved successfully, and false if it wasn't.
//-----------------------------------------------------------------------------
bool ActiveApplicationTableModel::GetExecutableNameAtRow(int rowIndex, QString& executableName) const
{
    bool gotExecutable = false;
    const QModelIndex& rowExecutableNameIndex = m_pActiveApplicationsTableModel->index(rowIndex, ACTIVE_APPLICATION_TABLE_COLUMN_EXECUTABLE_NAME);

    if (rowExecutableNameIndex.isValid())
    {
        executableName = m_pActiveApplicationsTableModel->data(rowExecutableNameIndex, Qt::DisplayRole).toString();
        gotExecutable = true;
    }

    return gotExecutable;
}

//-----------------------------------------------------------------------------
/// Handler invoked when a new developer mode process is discovered.
/// \param processInfo The info for the new process.
/// \param isActive Running status of the process.
//-----------------------------------------------------------------------------
void ActiveApplicationTableModel::OnClientDiscovered(const ProcessInfoModel& processInfo, bool isActive)
{
    // Check to make sure the process isn't already in the list. This may be called multiple
    // times due applications destroying/recreating their device instance repeatedly.
    int rowIndex = IsAlreadyInList(processInfo);
    if (rowIndex == -1)
    {
        // New process - add to the list and always set active state
        rowIndex = AddActiveApplication(processInfo);
        isActive = true;
    }

    // Update the running status
    SetRowEnabled(rowIndex, isActive);

    // Set running status column data (for row sorting)
    QModelIndex runningStatusIndex = m_pActiveApplicationsTableModel->index(rowIndex, ACTIVE_APPLICATION_TABLE_COLUMN_RUNNING_STATUS);
    m_pActiveApplicationsTableModel->setData(runningStatusIndex, QVariant(isActive));

    // Newly enabled clients are moved to the top of the list
    if (isActive)
    {
        QList<QStandardItem*> rowItems = m_pActiveApplicationsTableModel->takeRow(rowIndex);
        m_pActiveApplicationsTableModel->insertRow(0, rowItems);
    }

    // Sort processes by running status (running processes at top, dead ones at bottom)
    m_pActiveApplicationsTableModel->sort(ACTIVE_APPLICATION_TABLE_COLUMN_RUNNING_STATUS, Qt::DescendingOrder);
}

//-----------------------------------------------------------------------------
/// Add a new application to the active applications table.
/// \param processInfo The process info for the actively running process.
/// \returns The index of the newly added row.
//-----------------------------------------------------------------------------
int ActiveApplicationTableModel::AddActiveApplication(const ProcessInfoModel& processInfo)
{
    int currentRowCount = m_pActiveApplicationsTableModel->rowCount();

    int newRowCount = currentRowCount + 1;
    m_pActiveApplicationsTableModel->setRowCount(newRowCount);

    SetTableModelData(processInfo.GetProcessName(), currentRowCount, ACTIVE_APPLICATION_TABLE_COLUMN_EXECUTABLE_NAME);
    SetTableModelData(processInfo.GetAPI(),         currentRowCount, ACTIVE_APPLICATION_TABLE_COLUMN_API);

    return currentRowCount;
}

//-----------------------------------------------------------------------------
/// Check if the given process is already in the Active Applications table.
/// \param processInfo The info for the process to check against.
/// \returns The row index if it's in the table, or -1 if it's not.
//-----------------------------------------------------------------------------
int ActiveApplicationTableModel::IsAlreadyInList(const ProcessInfoModel& processInfo) const
{
    int numRows = m_pActiveApplicationsTableModel->rowCount();
    if (numRows > 0)
    {
        for (int rowIndex = 0; rowIndex < numRows; ++rowIndex)
        {
            const QString& processExecutableName = processInfo.GetProcessName();
            QString executableAtRow;
            if (GetExecutableNameAtRow(rowIndex, executableAtRow))
            {
                if (executableAtRow.compare(processExecutableName) == 0)
                {
                    return rowIndex;
                }
            }
        }
    }

    return -1;
}

//-----------------------------------------------------------------------------
/// Set the data for a cell in the table.
/// \param modelData The data to display in the given cell.
/// \param row The row index of the given cell.
/// \param column The column index of the given cell.
/// \param alignment The alignment to use for the data in the given cell.
//-----------------------------------------------------------------------------
void ActiveApplicationTableModel::SetTableModelData(const QString& modelData, uint row, uint column, enum Qt::AlignmentFlag alignment)
{
    m_pActiveApplicationsTableModel->setData(m_pActiveApplicationsTableModel->index(row, column), QString(modelData));
    m_pActiveApplicationsTableModel->setData(m_pActiveApplicationsTableModel->index(row, column), (int)alignment | Qt::AlignVCenter, Qt::TextAlignmentRole);
}

//-----------------------------------------------------------------------------
/// Set the visual state of the given row to be enabled or disabled by setting text to gray or black.
/// \param rowIndex The index of the row to alter the appearance of.
/// \param isEnabled A flag that determines if the row is enabled or disabled.
//-----------------------------------------------------------------------------
void ActiveApplicationTableModel::SetRowEnabled(int rowIndex, bool isEnabled)
{
    // The text will use black when enabled. If the row is disabled, switch it to gray.
    QColor enabledColor(Qt::black);
    if (!isEnabled)
    {
        enabledColor = QColor(Qt::gray);
    }

    // Step through each column in the row and apply the style.
    for (int columnIndex = 0; columnIndex < ACTIVE_APPLICATION_TABLE_COLUMN_COUNT; ++columnIndex)
    {
        m_pActiveApplicationsTableModel->setData(m_pActiveApplicationsTableModel->index(rowIndex, columnIndex), enabledColor, Qt::ForegroundRole);
    }
}
