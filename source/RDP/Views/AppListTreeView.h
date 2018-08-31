//=============================================================================
/// Copyright (c) 2017 - 2018 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for the Application List Treeview.
//=============================================================================

#ifndef _APP_LIST_TREEVIEW_H_
#define _APP_LIST_TREEVIEW_H_

#include <QTreeView>
#include <QHeaderView>

class SetupTargetApplicationModel;

/// Class to handle an application list treeview. The usual 'clicked' signal
/// doesn't work in some cases as it won't detect mouse pressed and drag events
/// and this can lead to multiple profile checkboxes being selected.
class AppListTreeView : public QTreeView
{
    Q_OBJECT

public:
    explicit AppListTreeView(QWidget* pParent = nullptr);
    ~AppListTreeView();

    void    SetTargetApplicationModel(SetupTargetApplicationModel* pModel);
    bool    eventFilter(QObject* pObj, QEvent* pEvent);

protected:
    virtual void mouseReleaseEvent(QMouseEvent* pEvent);

protected:
    SetupTargetApplicationModel*      m_pTargetApplicationModel;     ///< Model for target application pane
};

/// Header view for the AppListTreeView. This allows interception of mouse press
/// events such that only the first column can be clicked to sort the tree view.
class AppListHeaderView : public QHeaderView
{
    Q_OBJECT
public:
    explicit AppListHeaderView(Qt::Orientation orientation, QWidget* pParent = nullptr) : QHeaderView(orientation, pParent) {}
    virtual ~AppListHeaderView() {}

protected:
    virtual void mousePressEvent(QMouseEvent* pEvent);
};

#endif // _APP_LIST_TREEVIEW_H_
