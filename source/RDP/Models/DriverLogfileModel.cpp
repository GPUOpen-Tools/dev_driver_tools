//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A model used to store driver log file lines.
//=============================================================================

#include "DriverLogfileModel.h"
#include "../RDPDefinitions.h"

//-----------------------------------------------------------------------------
/// Retrieve the number of rows stored within the log file.
/// \param parent The parent ModelIndex for the ListView.
/// \returns The count of rows to be displayed using this model.
//-----------------------------------------------------------------------------
int DriverLogfileModel::rowCount(const QModelIndex& parent) const
{
    RDP_UNUSED(parent);
    return m_logMessageLines.size();
}

//-----------------------------------------------------------------------------
/// Handler used to supply data to the logfile viewer.
/// \param index The index of the model data to be supplied.
/// \param role The role to update in the viewer.
/// \returns A QVariant value to be placed in the viewer at the index.
//-----------------------------------------------------------------------------
QVariant DriverLogfileModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole && index.isValid())
    {
        int lineIndex = index.row();
        if (lineIndex >= 0 && lineIndex < m_logMessageLines.size())
        {
            const QString& logLine = m_logMessageLines.at(lineIndex);
            return logLine;
        }
    }

    return QVariant();
}

//-----------------------------------------------------------------------------
/// Handler used to supply the correct header text for the Process ListView.
/// \param section The index of the section whose header text will be updated.
/// \param orientation The table orientation of the header text.
/// \param role The role to update in the ListView.
/// \returns A QVariant value to be placed in the ListView header.
//-----------------------------------------------------------------------------
QVariant DriverLogfileModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section);
    Q_UNUSED(orientation);
    Q_UNUSED(role);

    return QVariant();
}

//-----------------------------------------------------------------------------
/// Add a new log line string to the log file.
/// \param logLine The new log line to append to the log.
//-----------------------------------------------------------------------------
void DriverLogfileModel::AddLogLine(const QString& logLine)
{
    int rowIndex = m_logMessageLines.size();
    beginInsertRows(QModelIndex(), rowIndex, rowIndex);
    m_logMessageLines.push_back(logLine);
    endInsertRows();
}

//-----------------------------------------------------------------------------
/// Clear all the log data for this log file.
//-----------------------------------------------------------------------------
void DriverLogfileModel::ClearLogfile()
{
    if (m_logMessageLines.size() > 0)
    {
        beginResetModel();
        m_logMessageLines.clear();
        endResetModel();
    }
}
