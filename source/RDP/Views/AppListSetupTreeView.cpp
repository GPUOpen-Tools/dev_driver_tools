//=============================================================================
/// Copyright (c) 2018 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of the customized Application List Treeview with delegates.
/// Extends AppListTreeView.
//=============================================================================

#include <QApplication>
#include <QTreeView>
#include <QHeaderView>
#include <QMouseEvent>
#include <QStandardItemModel>
#include <QFileDialog>
#include <QStringListModel>

#include "AppListSetupTreeView.h"
#include "../Settings/RDPSettings.h"
#include "../Models/SetupTargetApplicationModel.h"

//-----------------------------------------------------------------------------
/// Explicit Constructor
/// \param pParent The parent widget.
//-----------------------------------------------------------------------------
AppListSetupTreeView::AppListSetupTreeView(QWidget* pParent) :
    AppListTreeView(pParent)
{
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
AppListSetupTreeView::~AppListSetupTreeView()
{
}

//-----------------------------------------------------------------------------
/// Set the target application model. This is the model for the whole target
/// application pane and not the model associated with this treeview
/// \param pModel The target application model
//-----------------------------------------------------------------------------
void AppListSetupTreeView::SetTargetApplicationModel(SetupTargetApplicationModel* pModel)
{
    AppListTreeView::SetTargetApplicationModel(pModel);
}

