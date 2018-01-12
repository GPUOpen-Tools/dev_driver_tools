//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A model responsible for maintaining a list of recently collected RGP traces.
//=============================================================================

#ifndef _RGP_RECENT_TRACE_LIST_MODEL_H_
#define _RGP_RECENT_TRACE_LIST_MODEL_H_

#include <QAbstractItemModel>
#include "RGPTraceModel.h"

/// An enumeration used only in the Recent Traces list.
enum RecentTraceColumnNames
{
    RECENT_TRACE_COLUMN_FILEPATH,
    RECENT_TRACE_COLUMN_SIZE,
    RECENT_TRACE_COLUMN_CREATED_TIMESTAMP,
};

/// A helper vector of Recent Trace info structures.
typedef QVector< RgpTraceFileInfo > RecentTraceInfoVector;

//-----------------------------------------------------------------------------
/// A model responsible for maintaining a list of recently collected RGP traces.
//-----------------------------------------------------------------------------
class RGPRecentTraceListModel : public QAbstractItemModel
{
public:
    RGPRecentTraceListModel() {}
    virtual ~RGPRecentTraceListModel() {}

    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
    virtual int columnCount(const QModelIndex& = QModelIndex()) const Q_DECL_OVERRIDE { return 3; }
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
    virtual QModelIndex parent(const QModelIndex& index) const Q_DECL_OVERRIDE;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const Q_DECL_OVERRIDE;

    void AddRecentTraceFile(const QString& fullFilepath, size_t traceSizeInBytes, time_t createdTimestamp);
    bool GetTraceInfoByIndex(int rowIndex, RgpTraceFileInfo& traceInfo) const;
    void ClearRecentTraces();
    void RemoveRecentTraceRow(int rowIndex);
    void RenameTraceFile(int rowIndex, const QString& newFilename);

private:
    RecentTraceInfoVector m_recentTraceFiles;
};

#endif // _RGP_RECENT_TRACE_LIST_MODEL_H_
