//=============================================================================
/// Copyright (c) 2018 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for the customized Application List Treeview with delegates.
/// Extends AppListTreeView.
//=============================================================================

#ifndef _APP_LIST_SETUP_TREEVIEW_H_
#define _APP_LIST_SETUP_TREEVIEW_H_

#include "AppListTreeView.h"
#include <QTreeView>
#include <QHeaderView>
class SetupTargetApplicationModel;

/// Class to handle an application setup list treeview. The usual 'clicked' signal
/// doesn't work in some cases as it won't detect mouse pressed and drag events
/// and this can lead to multiple profile checkboxes being selected.
class AppListSetupTreeView : public AppListTreeView
{
    Q_OBJECT

public:
    explicit AppListSetupTreeView(QWidget* pParent = nullptr);
    virtual ~AppListSetupTreeView();
    void SetTargetApplicationModel(SetupTargetApplicationModel* pModel);

};

#endif // _APP_LIST_SETUP_TREEVIEW_H_
