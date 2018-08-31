//=============================================================================
/// Copyright (c) 2017-2018 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A model used to store driver log file lines.
//=============================================================================

#include "DriverLogfileModel.h"
#include "../RDPDefinitions.h"

static const char s_TimestampFormat[] = "yyyy-MM-dd hh:mm:ss.zzz";

//-----------------------------------------------------------------------------
/// Retrieves column data for the model
/// \param section The column ID number
/// \param orientation Horizontal or Vertical
/// \param role The attribute for the column to retrieve
/// \return The name or attribute for the column.
//-----------------------------------------------------------------------------
QVariant DriverLogfileModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant result(QAbstractTableModel::headerData(section, orientation, role));

    if (orientation == Qt::Horizontal)
    {
        if (role == Qt::DisplayRole)
        {
            if ((section >= 0) && (section < COLUMN_COUNT))
            {
                result = ColumnNameLookup((ColumnIDs)section);
            }
        }
    }
    return result;
}

//-----------------------------------------------------------------------------
/// The number of rows in the model.
/// \param parent The parent index for tree models (not used).
/// \return The row count.
//-----------------------------------------------------------------------------
int DriverLogfileModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_modelData.size();
}

//-----------------------------------------------------------------------------
/// The number of columns in the model.
/// \param parent The parent index for tree models (not used).
/// \return The row count.
//-----------------------------------------------------------------------------
int DriverLogfileModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return COLUMN_COUNT;
}

//-----------------------------------------------------------------------------
/// Retrieves data for a cell in the model
/// \param index Identifies the cell to retrieve
/// \param role the Attribute for the cell to retrieve
/// \return The value or attribute for the cell.
//-----------------------------------------------------------------------------
QVariant DriverLogfileModel::data(const QModelIndex& index, int role) const
{
    QVariant result;

    if (index.isValid())
    {
        const int row = index.row();
        if ((row >= 0) && (row <= m_modelData.size()))
        {
            if ((role == Qt::DisplayRole) || (role == Qt::EditRole) )
            {
                const Schema& rowData = m_modelData.at(row);
                switch (index.column())
                {
                case COLUMN_PROCESS_ID:
                {
                    if (role == Qt::DisplayRole)
                    {
                        result = QString("0x%1").arg(rowData.processId, 8, 16, (QLatin1Char)'0');
                    }
                    else
                    {
                        result = rowData.processId;
                    }
                    break;
                }

                case COLUMN_PROCESS_NAME:
                    result = rowData.processName;
                    break;

                case COLUMN_TIMESTAMP:
                {
                    if (rowData.timestamp.isValid() == true)
                    {
                        result = rowData.timestamp.toString(s_TimestampFormat);
                    }
                    else
                    {
                        result = QString("");
                    }
                    break;
                }
                case COLUMN_LOG_LINE:
                    result = rowData.logLine;
                    break;

                default:
                    break;
                }
            }
        }
    }
    return result;
}

//-----------------------------------------------------------------------------
/// Set data for a cell in the model.
/// \param index Identifies the cell to be set.
/// \param value The data to store in the model's cell.
/// \param role The attribute type of the cell data being returned.
/// \return true if the index is valid and role supported, otherwise returns false.
//-----------------------------------------------------------------------------
bool DriverLogfileModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    bool result = false;
    if (index.isValid())
    {
        const int row = index.row();
        Schema rowData = m_modelData.at(index.row());

        if (role == Qt::EditRole)
        {
            result = true;
            switch (index.column())
            {
            case COLUMN_PROCESS_ID:
                rowData.processId = value.toUInt();
                break;

            case COLUMN_PROCESS_NAME:
                rowData.processName = value.toString();
                break;

            case COLUMN_TIMESTAMP:
                rowData.timestamp = value.toDateTime();
                break;

            case COLUMN_LOG_LINE:
                rowData.logLine = value.toBool();
                break;

            default:
                result = false;
            }
        }

        if (result == true)
        {
            m_modelData.replace(row, rowData);
            emit dataChanged(index, index);
        }
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Add a new log line string to the log file.
/// \param logLine The new log line to append to the log.
//-----------------------------------------------------------------------------
void DriverLogfileModel::AddLogLine(const QString& logLine)
{
    const QDateTime timestamp(QDateTime::currentDateTime());
    Schema newRowData{ timestamp, "", 0, logLine };
    int row = m_modelData.count();
    beginInsertRows(QModelIndex(), row, row);
    m_modelData.insert(row, newRowData);
    endInsertRows();
    emit(dataChanged(index(row, 0), index(row, 0)));
}
//-----------------------------------------------------------------------------
/// Clear all the log data for this log file.
//-----------------------------------------------------------------------------
void DriverLogfileModel::ClearLogfile()
{
    if (m_modelData.size() > 0)
    {
        beginResetModel();
        m_modelData.clear();
        endResetModel();
        emit(dataChanged(QModelIndex(), QModelIndex()));
    }
}

//-----------------------------------------------------------------------------
/// Output a text string containing all of the rows in the model.
/// \param logTextOut The string to contain the log messages.
//-----------------------------------------------------------------------------
void DriverLogfileModel::GetModelText(QString& logTextOut)
{
    QString logLineFormat("%1 %2(pid=0x%3) %4\n");
    foreach(const Schema& rowData, m_modelData)
    {
        logTextOut.append(logLineFormat.arg(rowData.timestamp.toString(s_TimestampFormat)).arg(rowData.processName).arg(rowData.processId, 8, 16, (QLatin1Char)'0').arg(rowData.logLine));
    }
}
