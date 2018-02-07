//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A model responsible for maintaining a list of recently collected RGP traces.
//=============================================================================

#include <QDateTime>
#include <QFileInfo>
#include <QTextStream>

#include "RGPRecentTraceListModel.h"
#include "../RDPDefinitions.h"

//-----------------------------------------------------------------------------
/// Return the number of recent trace files available to open.
/// \param parent The parent index for the row.
/// \returns The number of recent files available to open.
//-----------------------------------------------------------------------------
int RGPRecentTraceListModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);

    return m_recentTraceFiles.size();
}

//-----------------------------------------------------------------------------
/// Retrieve the data for the given index and role.
/// \param index The parent index for the row.
/// \param role The role to update data for.
/// \returns The data to display in the cell at the given index.
//-----------------------------------------------------------------------------
QVariant RGPRecentTraceListModel::data(const QModelIndex& index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        int currentRow = index.row();
        int currentColumn = index.column();

        if (currentRow >= 0 && currentRow < rowCount())
        {
            const RgpTraceFileInfo& recentTrace = m_recentTraceFiles.at(currentRow);

            if (currentColumn == RECENT_TRACE_COLUMN_FILEPATH)
            {
                return recentTrace.fileToDisplay;
            }
            else if (currentColumn == RECENT_TRACE_COLUMN_SIZE)
            {
                // Build a string containing the file size in MB, but use only 3 numbers after the decimal.
                QString str = "";
                QTextStream out(&str);
                out.setRealNumberNotation(QTextStream::FixedNotation);
                out.setLocale(QLocale::English);
                out.setRealNumberPrecision(3);

                // Default to displaying the trace file size in MB.
                QString filesizeLabel;

                // If the trace is under 1MB, display the file size in KB.
                float totalMegs = recentTrace.totalBytes / (1024.0f * 1024.0f);
                if (totalMegs < 1.0f)
                {
                    float totalKB = (recentTrace.totalBytes / 1024.0f);
                    out << totalKB;
                    filesizeLabel = QString("%1 KB").arg(str);
                }
                else
                {
                    out << totalMegs;
                    filesizeLabel = QString("%1 MB").arg(str);
                }

                return filesizeLabel;
            }
            else if (currentColumn == RECENT_TRACE_COLUMN_CREATED_TIMESTAMP)
            {
                const QDateTime creationTime = QDateTime::fromTime_t(recentTrace.traceCreationTimestamp);
                const QString creationDate = creationTime.toString(Qt::TextDate);
                return creationDate;
            }
        }
    }

    return QVariant();
}

//-----------------------------------------------------------------------------
/// Retrieve the header data for the recent traces list view.
/// \param section The header index to get data for.
/// \param orientation The orientation of the given header index.
/// \param role The role to update the header data for.
/// \returns The value for the given role.
//-----------------------------------------------------------------------------
QVariant RGPRecentTraceListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Orientation::Horizontal)
    {
        if (section == RECENT_TRACE_COLUMN_FILEPATH)
        {
            return gs_RECENT_TRACE_FILEPATH_HEADER;
        }
        else if (section == RECENT_TRACE_COLUMN_SIZE)
        {
            return gs_RECENT_TRACE_FILE_SIZE;
        }
        else if (section == RECENT_TRACE_COLUMN_CREATED_TIMESTAMP)
        {
            return gs_RECENT_TRACE_CREATION_TIMESTAMP;
        }
    }

    return QVariant();
}

//-----------------------------------------------------------------------------
/// Retrieve the index of the given row and column.
/// \param row The row to get the index for.
/// \param column The column to get the index for.
/// \param parent The parent of the given index.
/// \returns The index for the given arguments.
//-----------------------------------------------------------------------------
QModelIndex RGPRecentTraceListModel::index(int row, int column, const QModelIndex& parent) const
{
    QModelIndex resultIndex;

    if (hasIndex(row, column, parent))
    {
        // Create a new index for the row.
        resultIndex = createIndex(row, column);
    }

    return resultIndex;
}

//-----------------------------------------------------------------------------
/// Return the parent index for the given index.
/// \param index The index to retrieve the parent for.
/// \returns The index of the parent.
//-----------------------------------------------------------------------------
QModelIndex RGPRecentTraceListModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index);

    return QModelIndex();
}

//-----------------------------------------------------------------------------
/// Retrieve the flags for the given index.
/// \param index The index to retrieve flags for.
/// \returns The flags for the given index.
//-----------------------------------------------------------------------------
Qt::ItemFlags RGPRecentTraceListModel::flags(const QModelIndex& index) const
{
    return QAbstractItemModel::flags(index);
}

//-----------------------------------------------------------------------------
/// Add a new recent trace file to the list model.
/// \param recentTraceFilepath The new trace filepath to add to the list of recent traces.
/// \param traceSizeInBytes The total trace file size in bytes.
/// \param createdTimestamp A timestamp generated when the RGP trace was collected.
//-----------------------------------------------------------------------------
void RGPRecentTraceListModel::AddRecentTraceFile(const QString& fullFilepath, size_t traceSizeInBytes, time_t createdTimestamp)
{
    beginInsertRows(QModelIndex(), 0, 0);

    // Insert the new trace data into the list view.
    QFileInfo fileInfo(fullFilepath);
    RgpTraceFileInfo recentTracefile = { fullFilepath, fileInfo.fileName(), traceSizeInBytes, createdTimestamp };

    m_recentTraceFiles.push_front(recentTracefile);

    endInsertRows();
}

//-----------------------------------------------------------------------------
/// Retrieve the recent RGP trace info by row index.
/// \param rowIndex The row to retrieve the trace data for.
/// \param traceInfo The info for the recent RGP trace.
/// \returns True if the info was retrieved, and false if it failed.
//-----------------------------------------------------------------------------
bool RGPRecentTraceListModel::GetTraceInfoByIndex(int rowIndex, RgpTraceFileInfo& traceInfo) const
{
    bool validIndex = (rowIndex >= 0) && (rowIndex < m_recentTraceFiles.size());
    Q_ASSERT(validIndex == true);

    if (validIndex)
    {
        traceInfo = m_recentTraceFiles.at(rowIndex);
    }

    return validIndex;
}

//-----------------------------------------------------------------------------
/// Clear all recent trace files from the list model.
//-----------------------------------------------------------------------------
void RGPRecentTraceListModel::ClearRecentTraces()
{
    m_recentTraceFiles.clear();

    beginResetModel();
    endResetModel();
}

//-----------------------------------------------------------------------------
/// Remove a single row from the Recent Traces list model.
/// \param rowIndex The index of the row to be removed.
//-----------------------------------------------------------------------------
void RGPRecentTraceListModel::RemoveRecentTraceRow(int rowIndex)
{
    bool validIndex = (rowIndex >= 0) && (rowIndex < m_recentTraceFiles.size());
    Q_ASSERT(validIndex == true);

    if (validIndex)
    {
        beginRemoveRows(QModelIndex(), rowIndex, rowIndex);

        RecentTraceInfoVector::iterator rowIter = m_recentTraceFiles.begin() + rowIndex;
        m_recentTraceFiles.erase(rowIter);

        endRemoveRows();
    }
}

//-----------------------------------------------------------------------------
/// Rename a trace file's filename.
/// \param rowIndex The row index being updated.
/// \param newFilename The new filename of the file being altered.
//-----------------------------------------------------------------------------
void RGPRecentTraceListModel::RenameTraceFile(int rowIndex, const QString& newFilename)
{
    bool validIndex = (rowIndex >= 0) && (rowIndex < m_recentTraceFiles.size());
    Q_ASSERT(validIndex == true);

    if (validIndex)
    {
        beginResetModel();

        RgpTraceFileInfo& recentTraceInfo = m_recentTraceFiles[rowIndex];
        recentTraceInfo.fullPathToFile = newFilename;

        QFileInfo fileInfo(newFilename);
        recentTraceInfo.fileToDisplay = fileInfo.fileName();

        endResetModel();
    }
}
