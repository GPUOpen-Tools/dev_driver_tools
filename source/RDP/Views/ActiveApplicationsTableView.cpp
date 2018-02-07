//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation for the Active Applications table interface.
//=============================================================================

#include <QLineEdit>
#include <QPushButton>
#include <QStandardItemModel>

#include "ActiveApplicationsTableView.h"
#include "SetupTargetApplicationView.h"
#include "ui_ActiveApplicationsTableView.h"
#include "../RDPDefinitions.h"
#include "../Settings/RDPSettings.h"
#include "../Util/RDPUtil.h"
#include "../Models/ActiveApplicationTableModel.h"
#include "../Models/SetupTargetApplicationModel.h"
#include "../Models/DeveloperPanelModel.h"
#include <QtUtil.h>

//-----------------------------------------------------------------------------
/// Explicit constructor
/// \param pDeveloperPanelModel The main RDP model instance.
/// \param pTargetApplicationView The TargetApplicationView interface where the list of target applications lives.
/// \param pParent The parent widget for this interface.
//-----------------------------------------------------------------------------
ActiveApplicationsTableView::ActiveApplicationsTableView(DeveloperPanelModel* pDeveloperPanelModel, SetupTargetApplicationView* pTargetApplicationView, QWidget* pParent) :
    QWidget(pParent),
    ui(new Ui::ActiveApplicationsTableView),
    m_pDeveloperPanelModel(pDeveloperPanelModel),
    m_pSetupTargetApplicationView(pTargetApplicationView)
{
    ui->setupUi(this);

    // Create a model for the table view.
    m_pActiveApplicationsTableModel = new ActiveApplicationTableModel();

    Q_ASSERT(m_pActiveApplicationsTableModel != nullptr);
    QtCommon::QtUtil::ApplyStandardTableStyle(ui->ActiveApplicationsList);

    ui->ActiveApplicationsList->setModel(m_pActiveApplicationsTableModel->GetTableModel());
    ui->ActiveApplicationsList->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);

    // Set up signals/slots.
    connect(ui->addToTargets, &QPushButton::clicked, this, &ActiveApplicationsTableView::AddToList);
    connect(pDeveloperPanelModel, &DeveloperPanelModel::UpdateClientRunStatus, m_pActiveApplicationsTableModel, &ActiveApplicationTableModel::OnClientDiscovered);
    connect(ui->ActiveApplicationsList, &AppListTreeView::clicked, this, &ActiveApplicationsTableView::OnApplicationSelected);
    connect(ui->ActiveApplicationsList, &AppListTreeView::doubleClicked, this, &ActiveApplicationsTableView::OnRowDoubleClicked);
    connect(m_pSetupTargetApplicationView, &SetupTargetApplicationView::ApplicationRemovedFromList, this, &ActiveApplicationsTableView::OnApplicationRemoved);

    // Connect the signal responsible for auto-resizing the table columns when the table is updated.
    QStandardItemModel* pActiveAppsTableModel = m_pActiveApplicationsTableModel->GetTableModel();
    if (pActiveAppsTableModel != nullptr)
    {
        connect(pActiveAppsTableModel, &QStandardItemModel::dataChanged, this, &ActiveApplicationsTableView::OnTableDataChanged);
    }

    // Disable the add to list button for now
    ui->addToTargets->setEnabled(false);

    // Hide the running status column - this column exists in the model to allow sorting by running
    // status, but it should not be visible in the view
    ui->ActiveApplicationsList->setColumnHidden(ACTIVE_APPLICATION_TABLE_COLUMN_RUNNING_STATUS, true);

    // Update the model.
    AdjustTableColumns();
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
ActiveApplicationsTableView::~ActiveApplicationsTableView()
{
    SAFE_DELETE(ui);
    SAFE_DELETE(m_pActiveApplicationsTableModel);
}

//-----------------------------------------------------------------------------
/// Slot to handle what happens when the user clicks the 'Add to targets' button.
/// \param clicked Whether the button is clicked
//-----------------------------------------------------------------------------
void ActiveApplicationsTableView::AddToList(bool clicked)
{
    Q_UNUSED(clicked);

    const QModelIndex& selectedRowIndex = ui->ActiveApplicationsList->currentIndex();
    if (selectedRowIndex.isValid())
    {
        // Add the selected line to the application list.
        int selectedRow = selectedRowIndex.row();
        AddAppByRowIndex(selectedRow);
    }
}

//-----------------------------------------------------------------------------
/// Helper function that adds the info at the given row to the target applications table.
/// \param rowIndex The index of the row to add to the target applications table.
//-----------------------------------------------------------------------------
void ActiveApplicationsTableView::AddAppByRowIndex(int rowIndex)
{
    QString executableFilename;
    if (m_pActiveApplicationsTableModel->GetExecutableNameAtRow(rowIndex, executableFilename))
    {
        m_pSetupTargetApplicationView->AddExecutableToList(executableFilename);

        // The executable was just added to the target list. Disable the add button so it can't be added again.
        ui->addToTargets->setEnabled(false);
    }
}

//-----------------------------------------------------------------------------
/// Adjust the column widths of the target applications table
//-----------------------------------------------------------------------------
void ActiveApplicationsTableView::AdjustTableColumns()
{
    int numRows = m_pActiveApplicationsTableModel->GetTableModel()->rowCount();
    QtCommon::QtUtil::AutoAdjustTableColumns(ui->ActiveApplicationsList, numRows, 10);
}

//-----------------------------------------------------------------------------
/// Enable the "Remove from list" button when an exe is selected from the list
/// \param index the model index of the clicked item
//-----------------------------------------------------------------------------
void ActiveApplicationsTableView::OnApplicationSelected(const QModelIndex& index)
{
    if (index.isValid())
    {
        // Add the selected line to the application list.
        int selectedRow = index.row();

        QString executableFilename;
        bool gotExecutableFilename = m_pActiveApplicationsTableModel->GetExecutableNameAtRow(selectedRow, executableFilename);
        if (gotExecutableFilename)
        {
            // Only enable the "Add to targets" button if the selected item isn't already a target.
            bool alreadyInTargetList = m_pSetupTargetApplicationView->GetSetupTargetApplicationModel()->IsApplicationInTargetList(executableFilename);

            if (!alreadyInTargetList)
            {
                ui->addToTargets->setEnabled(true);
            }
        }
    }
}

//-----------------------------------------------------------------------------
/// Handler invoked when the active applications table is updated.
/// \param topLeft The index of the top-left cell that was updated.
/// \param bottomRight The index of the bottom-right cell that was updated.
/// \param roles The vector of roles that was updated.
//-----------------------------------------------------------------------------
void ActiveApplicationsTableView::OnTableDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles)
{
    // Ignore all of the incoming arguments- don't need them.
    Q_UNUSED(topLeft);
    Q_UNUSED(bottomRight);
    Q_UNUSED(roles);

    // Just resize the columns correctly after the table was updated.
    AdjustTableColumns();
}

//-----------------------------------------------------------------------------
/// Handler invoked when a row in the Active Applications table is double-clicked.
/// \param index The index of the row tree row was clicked.
//-----------------------------------------------------------------------------
void ActiveApplicationsTableView::OnRowDoubleClicked(const QModelIndex& index)
{
    if (index.isValid())
    {
        // Determine which row was selected and add it to the list.
        int selectedRow = index.row();
        AddAppByRowIndex(selectedRow);
    }
}

//-----------------------------------------------------------------------------
/// Handler invoked when an application in the Setup target application
/// table is removed.
/// \param applicationName The name of the application that was removed
//-----------------------------------------------------------------------------
void ActiveApplicationsTableView::OnApplicationRemoved(const QString& applicationName)
{
    // Find out if this application is in the model
    // if so, go ahead and enable the "Add to targets" button
    int numRows = m_pActiveApplicationsTableModel->GetTableModel()->rowCount();
    if (numRows > 0)
    {
        for (int rowIndex = 0; rowIndex < numRows; ++rowIndex)
        {
            QString executableAtRow;
            if (m_pActiveApplicationsTableModel->GetExecutableNameAtRow(rowIndex, executableAtRow))
            {
                if (executableAtRow.compare(applicationName) == 0)
                {
                    ui->addToTargets->setEnabled(true);
                    // also make the row for this app selected
                    ui->ActiveApplicationsList->setCurrentIndex(m_pActiveApplicationsTableModel->GetTableModel()->index(rowIndex, 0));
                }
            }
        }
    }
}
