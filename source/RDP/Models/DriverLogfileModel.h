//=============================================================================
/// Copyright (c) 2017-2018 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A model used to store driver log file lines.
//=============================================================================

#ifndef _DRIVER_LOGFILE_MODEL_H_
#define _DRIVER_LOGFILE_MODEL_H_

#include <QAbstractTableModel>
#include <QString>
#include <QVector>
#include <QDateTime>

/// A model used to store driver log file lines.
class DriverLogfileModel : public QAbstractTableModel
{
public:

    // IDs for column headings
    enum ColumnIDs
    {
        COLUMN_TIMESTAMP,
        COLUMN_PROCESS_NAME,
        COLUMN_PROCESS_ID,
        COLUMN_LOG_LINE,

        COLUMN_COUNT
    };

    // Return column name string for column ID
    const QString& ColumnNameLookup(ColumnIDs columnID) const
    {
        struct Hash
        {
            uint64_t id;
            QString name;
        };

        static const Hash names[]
        {
            { COLUMN_TIMESTAMP, "Timestamp" },
            { COLUMN_PROCESS_NAME, "Process Name" },
            { COLUMN_PROCESS_ID, "Process ID" },
            { COLUMN_LOG_LINE, "Message" },
        };
        return names[columnID].name;
    }

    // Schema for the model
    struct Schema
    {
        const static int    schemaVersion = 1;
        QDateTime timestamp;
        QString processName;
        uint processId;
        QString logLine;
    };
    int SchemaVersion() { return Schema::schemaVersion; }

    DriverLogfileModel() {}
    virtual ~DriverLogfileModel() {}

    // Qt Overrides
    virtual int rowCount(const QModelIndex& parent) const Q_DECL_OVERRIDE;
    virtual int columnCount(const QModelIndex& parent) const Q_DECL_OVERRIDE;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) Q_DECL_OVERRIDE;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    void AddLogLine(const QString& logLine);

    void ClearLogfile();
    void GetModelText(QString& logTextOut);

private:
    QList<Schema> m_modelData;          ///< The log messages stored in the model.
};

#endif // _DRIVER_LOGFILE_MODEL_H_
