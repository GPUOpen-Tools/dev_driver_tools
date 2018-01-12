//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Empty driver settings placeholder ui
//=============================================================================

#include "EmptyDriverSettingsView.h"
#include "ui_EmptyDriverSettingsView.h"
#include "../../Common/ToolUtil.h"

//-----------------------------------------------------------------------------
/// Constructor.
/// \param pParent The parent widget.
//-----------------------------------------------------------------------------
EmptyDriverSettingsView::EmptyDriverSettingsView(QWidget* pParent) :
    QWidget(pParent),
    ui(new Ui::EmptyDriverSettingsView)
{
    ui->setupUi(this);

    // Set white background color
    ToolUtil::SetWidgetBackgroundColor(this, Qt::white);

    // Send out signal when the import button is pressed
    connect(ui->importButton, SIGNAL(pressed()), this, SIGNAL(importButtonPressed()));
}

//-----------------------------------------------------------------------------
/// Destructor.
//-----------------------------------------------------------------------------
EmptyDriverSettingsView::~EmptyDriverSettingsView()
{
    delete ui;
}
