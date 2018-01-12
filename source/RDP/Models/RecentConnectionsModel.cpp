//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of the Recent Connections model.
//=============================================================================

#include <QStandardItemModel>
#include "RecentConnectionsModel.h"
#include "../../Common/DriverToolsDefinitions.h"
#include "../RDPDefinitions.h"

/// An enumeration that declares the purpose of columns in the Recent Connections table.
enum RecentConnectionTableColumns
{
//    RECENT_CONNECTION_TABLE_COLUMN_NAME,
    RECENT_CONNECTION_TABLE_COLUMN_IP,
    RECENT_CONNECTION_TABLE_COLUMN_PORT,
//    RECENT_CONNECTION_TABLE_COLUMN_AVAILABLE,
//    RECENT_CONNECTION_TABLE_COLUMN_AUTOCONNECT,

    RECENT_CONNECTION_TABLE_COLUMN_COUNT,
};

//-----------------------------------------------------------------------------
/// Constructor
//-----------------------------------------------------------------------------
RecentConnectionsModel::RecentConnectionsModel()
{
    m_pRecentConnectionsTableModel = new QStandardItemModel(0, RECENT_CONNECTION_TABLE_COLUMN_COUNT);

    m_pRecentConnectionsTableModel->setHorizontalHeaderItem(RECENT_CONNECTION_TABLE_COLUMN_IP, new QStandardItem(gs_RECENT_CONNECTIONS_TABLE_IP_ADDRESS));
    m_pRecentConnectionsTableModel->setHorizontalHeaderItem(RECENT_CONNECTION_TABLE_COLUMN_PORT, new QStandardItem(gs_RECENT_CONNECTIONS_TABLE_PORT));

    // Initialize the recent connections list from whatever was loaded in the RDP settings file.
    RDPSettings& rdpSettings = RDPSettings::Get();

    // Populate the recent connections table with everything found in the RDPSettings file.
    const RecentConnectionVector& recentConnections = rdpSettings.GetRecentConnections();
    for (int connectionIndex = 0; connectionIndex < recentConnections.size(); ++connectionIndex)
    {
        const RDSConnectionInfo& connectionInfo = recentConnections.at(connectionIndex);
        AddConnectionInfo(connectionInfo);
    }
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
RecentConnectionsModel::~RecentConnectionsModel()
{
    SAFE_DELETE(m_pRecentConnectionsTableModel);
}

//-----------------------------------------------------------------------------
/// Get a reference to the recent connections table model
/// \return the RecentConnections table model
//-----------------------------------------------------------------------------
QStandardItemModel* RecentConnectionsModel::GetTableModel()
{
    return m_pRecentConnectionsTableModel;
}

//-----------------------------------------------------------------------------
/// Update the model. Call this when the data for the model gets changed
//-----------------------------------------------------------------------------
void RecentConnectionsModel::AddConnectionInfo(const RDSConnectionInfo& connectionInfo)
{
    int currentNumRows = m_pRecentConnectionsTableModel->rowCount();
    int newRowIndex = currentNumRows + 1;

    // Update the row count now that we're adding a new connection info row.
    m_pRecentConnectionsTableModel->setRowCount(newRowIndex);

    SetTableModelData(connectionInfo.ipString,                currentNumRows, RECENT_CONNECTION_TABLE_COLUMN_IP);

#ifdef Q_OS_WIN
    if (connectionInfo.ipString.isEmpty() || (connectionInfo.ipString == gs_LOCAL_HOST))
    {
        SetTableModelData(QString("N/A"), currentNumRows, RECENT_CONNECTION_TABLE_COLUMN_PORT);
    }
    else
    {
        SetTableModelData(QString::number(connectionInfo.port), currentNumRows, RECENT_CONNECTION_TABLE_COLUMN_PORT);
    }
#else
    SetTableModelData(QString::number(connectionInfo.port),   currentNumRows, RECENT_CONNECTION_TABLE_COLUMN_PORT);
#endif
}

//-----------------------------------------------------------------------------
/// Remove an existing connection info based on the given row index.
/// \param rowIndex The index of the row to remove from the table.
//-----------------------------------------------------------------------------
void RecentConnectionsModel::RemoveConnectionInfoRow(int rowIndex)
{
    if (RDPSettings::Get().RemoveRecentConnection(rowIndex))
    {
        m_pRecentConnectionsTableModel->removeRow(rowIndex);
    }
}

//-----------------------------------------------------------------------------
/// Clear all connection info rows, emptying the recent connections list.
/// Note: This will not remove the first row, as this should always exist.
//-----------------------------------------------------------------------------
void RecentConnectionsModel::ClearConnectionInfoRows()
{
    // Remove all rows, starting at index 1 (second row)
    while (GetNumConnectionRows() > 1)
    {
        RemoveConnectionInfoRow(1);
    }
}

//-----------------------------------------------------------------------------
/// Retrieve the number of rows in the table.
/// \returns The number of rows in the table.
//-----------------------------------------------------------------------------
int RecentConnectionsModel::GetNumConnectionRows() const
{
    return m_pRecentConnectionsTableModel->rowCount();
}

//-----------------------------------------------------------------------------
/// Populate the incoming Connection Info structure at the given row in the table.
/// \param row The index of the row to retrieve data for.
/// \param connectionInfo The connection info retrieved from the given row.
/// \returns True if the data was retrieved successfully, and false if it failed.
//-----------------------------------------------------------------------------
void RecentConnectionsModel::GetConnectionInfoAtRow(int row, RDSConnectionInfo& connectionInfo)
{
    // Pull the data out of the table one cell at a time, then stuff it into the outgoing connectionInfo.
    const QModelIndex& ipCell = m_pRecentConnectionsTableModel->index(row, RECENT_CONNECTION_TABLE_COLUMN_IP);
    connectionInfo.ipString = m_pRecentConnectionsTableModel->data(ipCell).toString();

    const QModelIndex& portStringCell = m_pRecentConnectionsTableModel->index(row, RECENT_CONNECTION_TABLE_COLUMN_PORT);
    connectionInfo.port = m_pRecentConnectionsTableModel->data(portStringCell).toUInt();
}

//-----------------------------------------------------------------------------
/// Set a cell in the table
/// \param data The data as a string to be added to the table.
/// \param row The destination row.
/// \param column The destination column.
/// \param alignment The alignment for the data
//-----------------------------------------------------------------------------
void RecentConnectionsModel::SetTableModelData(const QString &data, uint row, uint column, enum Qt::AlignmentFlag alignment)
{
    m_pRecentConnectionsTableModel->setData(m_pRecentConnectionsTableModel->index(row, column), QString(data));
    m_pRecentConnectionsTableModel->setData(m_pRecentConnectionsTableModel->index(row, column), (int)alignment | Qt::AlignVCenter, Qt::TextAlignmentRole);
}
