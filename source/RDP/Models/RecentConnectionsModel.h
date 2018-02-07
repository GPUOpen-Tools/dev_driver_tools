//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for the Recent Connections model.
//=============================================================================

#ifndef _RECENT_CONNECTIONS_MODEL_H_
#define _RECENT_CONNECTIONS_MODEL_H_

#include "../Settings/RDPSettings.h"

class QStandardItemModel;

/// Class for the Recent Connections model
class RecentConnectionsModel
{
public:
    RecentConnectionsModel();
    ~RecentConnectionsModel();

    QStandardItemModel* GetTableModel();
    void AddConnectionInfo(const RDSConnectionInfo& connectionInfo);
    void RemoveConnectionInfoRow(int rowIndex);
    void ClearConnectionInfoRows();
    int GetNumConnectionRows() const;
    void GetConnectionInfoAtRow(int row, RDSConnectionInfo& connectionInfo);

private:
    void SetTableModelData(const QString &data, uint row, uint column, enum Qt::AlignmentFlag alignment = Qt::AlignLeft);

    QStandardItemModel*     m_pRecentConnectionsTableModel;        ///< model associated with the recent connections table
};

#endif // _RECENT_CONNECTIONS_MODEL_H_

