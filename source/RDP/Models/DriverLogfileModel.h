//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A model used to store driver log file lines.
//=============================================================================

#ifndef _DRIVER_LOGFILE_MODEL_H_
#define _DRIVER_LOGFILE_MODEL_H_

#include <QAbstractTableModel>
#include <QString>
#include <QVector>

/// A model used to store driver log file lines.
class DriverLogfileModel : public QAbstractTableModel
{
public:
    DriverLogfileModel() {}
    virtual ~DriverLogfileModel() {}

    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
    virtual int columnCount(const QModelIndex& = QModelIndex()) const Q_DECL_OVERRIDE { return 1; }
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    void AddLogLine(const QString& logLine);
    void ClearLogfile();

private:
    QVector<QString> m_logMessageLines;
};

#endif // _DRIVER_LOGFILE_MODEL_H_
