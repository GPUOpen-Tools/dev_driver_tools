//=============================================================================
/// Copyright (c) 2016-2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for the Recent Connections panel interface.
//=============================================================================

#ifndef _RECENT_CONNECTIONS_VIEW_H_
#define _RECENT_CONNECTIONS_VIEW_H_

#include <QWidget>
#include "../Settings/RDPSettings.h"

class QTreeView;
class RecentConnectionsModel;

namespace Ui {
class RecentConnectionsView;
}

/// Class for the Recent Connections view
class RecentConnectionsView : public QWidget
{
    Q_OBJECT

public:
    explicit RecentConnectionsView(QWidget* pParent = nullptr);
    virtual ~RecentConnectionsView();

    void AdjustTableColumns() const;
    void EnableRemoveButtonsCheck();
    QTreeView* GetRecentConnectionsTable() const;
    void SelectRow(int row);
    void ToggleDisabledControlsWhileConnecting(bool enabled);

signals:
    void ConnectionRequested(const RDSConnectionInfo& selectedConnectionInfo);
    void ConnectionSelected(const RDSConnectionInfo& selectedConnectionInfo);

public slots:
    void OnNewConnectionAdded(const RDSConnectionInfo& connectionInfo);

private slots:
    void OnConnectionSelected(const QModelIndex& index);
    void OnConnectionDoubleClicked(const QModelIndex& index);
    void OnShowRecentConnectionsContextMenu(const QPoint& pos);
    void OnRemoveConnectionButtonClicked();
    void OnClearConnectionsButtonClicked();
    void OnRowCountChanged(const QModelIndex& parent, int first, int last);

private:
    Ui::RecentConnectionsView *ui;

    RecentConnectionsModel* m_pRecentConnectionsModel;  ///< The model associated with this view
};

#endif // _RECENT_CONNECTIONS_VIEW_H_
