//=============================================================================
/// Copyright (c) 2016-2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of the Recent Connections panel interface.
//=============================================================================

#include <QMenu>
#include <QStandardItemModel>

#include "RecentConnectionsView.h"
#include "ui_RecentConnectionsView.h"
#include "../RDPDefinitions.h"
#include "../Settings/RDPSettings.h"
#include "../Util/RDPUtil.h"
#include "../Models/RecentConnectionsModel.h"
#include "../../Common/DriverToolsDefinitions.h"
#include <QtUtil.h>

//-----------------------------------------------------------------------------
/// Explicit constructor
//-----------------------------------------------------------------------------
RecentConnectionsView::RecentConnectionsView(QWidget* pParent) :
    QWidget(pParent),
    ui(new Ui::RecentConnectionsView)
{
    ui->setupUi(this);

    m_pRecentConnectionsModel = new RecentConnectionsModel();
    Q_ASSERT(m_pRecentConnectionsModel != nullptr);

    QtCommon::QtUtil::ApplyStandardTableStyle(ui->RecentConnectionsList);

    ui->RecentConnectionsList->setModel(m_pRecentConnectionsModel->GetTableModel());
    EnableRemoveButtonsCheck();

    // Detect when data has been added or removed from the connection list
    connect(m_pRecentConnectionsModel->GetTableModel(), &QAbstractItemModel::rowsInserted, this, &RecentConnectionsView::OnRowCountChanged);
    connect(m_pRecentConnectionsModel->GetTableModel(), &QAbstractItemModel::rowsRemoved, this, &RecentConnectionsView::OnRowCountChanged);

    // Allow the user to only select a single row in the recent connections table.
    ui->RecentConnectionsList->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->RecentConnectionsList->setSelectionMode(QAbstractItemView::SingleSelection);

    // Attach the connect button handler.
    connect(ui->RecentConnectionsList->selectionModel(), &QItemSelectionModel::currentChanged, this, &RecentConnectionsView::OnConnectionSelected);
    connect(ui->RecentConnectionsList, &QTreeView::doubleClicked, this, &RecentConnectionsView::OnConnectionDoubleClicked);

    // Connect a ContextMenu handler to the recent connections table to quickly delete a row.
    connect(ui->RecentConnectionsList, &QTreeView::customContextMenuRequested, this, &RecentConnectionsView::OnShowRecentConnectionsContextMenu);
    ui->RecentConnectionsList->setContextMenuPolicy(Qt::CustomContextMenu);

    AdjustTableColumns();

    // Clear/remove button signals and slots
    connect(ui->removeButton, &QPushButton::clicked, this, &RecentConnectionsView::OnRemoveConnectionButtonClicked);
    connect(ui->clearButton, &QPushButton::clicked, this, &RecentConnectionsView::OnClearConnectionsButtonClicked);

    // Set button names and tooltips from RDP definitions
    ui->clearButton->setText(gs_RECENT_CONNECTIONS_CLEAR_BUTTON_NAME);
    ui->clearButton->setToolTip(gs_RECENT_CONNECTIONS_CLEAR_BUTTON_TOOLTIP);
    ui->removeButton->setText(gs_RECENT_CONNECTIONS_REMOVE_BUTTON_NAME);
    ui->removeButton->setToolTip(gs_RECENT_CONNECTIONS_REMOVE_BUTTON_TOOLTIP);

    // Select first row
    SelectRow(0);
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
RecentConnectionsView::~RecentConnectionsView()
{
    SAFE_DELETE(ui);
    SAFE_DELETE(m_pRecentConnectionsModel);
}

//-----------------------------------------------------------------------------
/// Adjust the column widths of the recent connections table
//-----------------------------------------------------------------------------
void RecentConnectionsView::AdjustTableColumns() const
{
    QAbstractItemModel* pDataModel = ui->RecentConnectionsList->model();
    if (pDataModel)
    {
        int numRows = pDataModel->rowCount();
        QtCommon::QtUtil::AutoAdjustTableColumns(ui->RecentConnectionsList, numRows, gs_COLUMN_PADDING);
    }
}

//-----------------------------------------------------------------------------
/// Enables or disables the "Remove All" button depending on the number of items
/// in the Recent Connections List.  If the list only contains the localhost,
/// then the "Remove All" button is disabled.
//-----------------------------------------------------------------------------
void RecentConnectionsView::EnableRemoveButtonsCheck()
{
    bool enable = true;

    // Disable "Clear" button if there's only one item in the Recent Connections List (i.e. only localhost listed).
    if (m_pRecentConnectionsModel->GetTableModel()->rowCount() < 2)
    {
        enable = false;
    }

    ui->clearButton->setEnabled(enable);

    // The "Remove" button should be enabled only if the first "localhost" row isn't selected.
    bool removeButtonEnabled = false;
    QItemSelectionModel* pRecentConnectionsTableSelectionModel = ui->RecentConnectionsList->selectionModel();
    if (pRecentConnectionsTableSelectionModel != nullptr)
    {
        const QModelIndexList& selectedRows = pRecentConnectionsTableSelectionModel->selectedRows();
        if (selectedRows.size() > 0)
        {
            int selectedRow = selectedRows[0].row();
            removeButtonEnabled = (selectedRow != 0);
        }
    }

    // Disable the remove button when the first row (always localhost) is selected
    ui->removeButton->setEnabled(removeButtonEnabled);
}

//-----------------------------------------------------------------------------
/// Get a pointer to the Recent Connections table control.
/// \returns A pointer to the Recent Connections table control.
//-----------------------------------------------------------------------------
QTreeView* RecentConnectionsView::GetRecentConnectionsTable() const
{
    return ui->RecentConnectionsList;
}

//-----------------------------------------------------------------------------
/// Select a numbered row in the recent selection list.
/// \param row The row number to select.
//-----------------------------------------------------------------------------
void RecentConnectionsView::SelectRow(int row)
{
    QItemSelectionModel* pSelectionModel = ui->RecentConnectionsList->selectionModel();
    QAbstractItemModel* pDataModel = ui->RecentConnectionsList->model();

    // Do nothing if row is out of range or models are null
    if (row < 0 || row > pDataModel->rowCount() - 1 ||
        pSelectionModel == nullptr ||
        pDataModel == nullptr)
    {
        return;
    }

    QItemSelection selection(pDataModel->index(row, 0), pDataModel->index(row, pDataModel->columnCount() - 1));
    pSelectionModel->select(selection, QItemSelectionModel::Select);

    OnConnectionSelected(pDataModel->index(row, 0));
}

//-----------------------------------------------------------------------------
/// Toggle the enabledness of all controls related to establishing a connection.
/// \param enabled A flag that dictates if the connection controls should be enabled.
//-----------------------------------------------------------------------------
void RecentConnectionsView::ToggleDisabledControlsWhileConnecting(bool enabled)
{
    // While attempting a connection, disable buttons that let the user alter the recent connections table.
    ui->removeButton->setEnabled(enabled);
    ui->clearButton->setEnabled(enabled);

    // If we're re-enabling connection controls, need to re-run the logic to determine if
    // the remove and clear buttons should be enabled.
    if (enabled)
    {
        EnableRemoveButtonsCheck();
        ui->RecentConnectionsList->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    }
    else
    {
        ui->RecentConnectionsList->setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);
        ui->RecentConnectionsList->selectionModel()->clearSelection();
    }
}

//-----------------------------------------------------------------------------
/// Handler invoked when a recent connection item is selected.
/// \param index The index of the selected item in the table.
//-----------------------------------------------------------------------------
void RecentConnectionsView::OnConnectionSelected(const QModelIndex& index)
{
    // If the model or selection model is nullptr, early out.
    if (m_pRecentConnectionsModel == nullptr ||
        ui->RecentConnectionsList->selectionModel() == nullptr)
    {
        return;
    }

    // Get connection info
    int selectedRowIndex = index.row();
    RDSConnectionInfo selectedConnectionInfo = {};
    m_pRecentConnectionsModel->GetConnectionInfoAtRow(selectedRowIndex, selectedConnectionInfo);

    // Disable the remove button when the first row is selected
    if (selectedRowIndex == 0)
    {
        ui->removeButton->setEnabled(false);
    }
    else
    {
        ui->removeButton->setEnabled(true);
    }

    // ConnectionSettingsView will handle this in a slot.
    emit ConnectionSelected(selectedConnectionInfo);
}

//-----------------------------------------------------------------------------
/// Handler invoked when a recent connection item is double clicked.
/// \param index The index of the clicked item in the table.
//-----------------------------------------------------------------------------
void RecentConnectionsView::OnConnectionDoubleClicked(const QModelIndex& index)
{
    // If the model or selection model is nullptr, early out.
    if (m_pRecentConnectionsModel == nullptr ||
        ui->RecentConnectionsList->selectionModel() == nullptr)
    {
        return;
    }

    // Get connection info
    int selectedRowIndex = index.row();
    RDSConnectionInfo selectedConnectionInfo = {};
    m_pRecentConnectionsModel->GetConnectionInfoAtRow(selectedRowIndex, selectedConnectionInfo);

    // ConnectionSettingsView will handle this in a slot.
    emit ConnectionRequested(selectedConnectionInfo);
}

//-----------------------------------------------------------------------------
/// Handler invoked when a new connection target is added.
/// \param connectionInfo The connection info to add to the list.
//-----------------------------------------------------------------------------
void RecentConnectionsView::OnNewConnectionAdded(const RDSConnectionInfo& connectionInfo)
{
    // if the IP string or port is empty, do not add the connection
    if (QString(connectionInfo.ipString).isEmpty() || (connectionInfo.port == 0))
    {
        return;
    }

    // Add the new connection to the settings file.
    if (RDPSettings::Get().AddRecentConnection(connectionInfo))
    {
        m_pRecentConnectionsModel->AddConnectionInfo(connectionInfo);
        AdjustTableColumns();
    }
}

//-----------------------------------------------------------------------------
/// Open a context menu when the user right-clicks on the Recent Connections table.
/// \param pos The location of the user's mouse click.
//-----------------------------------------------------------------------------
void RecentConnectionsView::OnShowRecentConnectionsContextMenu(const QPoint& pos)
{
    // Get the index of the clicked cell.
    QModelIndex selectedCellIndex = ui->RecentConnectionsList->indexAt(pos);

    // Did the user select a valid row in the recent traces list?
    bool validRow = selectedCellIndex.isValid();

    // Create Context Menu. The menu options are only enabled if the user right-clicked a valid row.
    QMenu menu;

    // Add the "Connect" action.
    QAction* pConnectToConnectionRow = menu.addAction(gs_RECENT_CONNECTIONS_CONTEXT_MENU_CONNECT);
    pConnectToConnectionRow->setEnabled(validRow);

    bool canDelete = validRow;
    if (validRow)
    {
        // Don't let the user remove the automatically-added localhost row.
        int selectedRow = selectedCellIndex.row();
        if (selectedRow == 0)
        {
            canDelete = false;
        }
    }

    // Add the "Delete this row" action.
    QAction* pDeleteConnectionRowAction = menu.addAction(gs_RECENT_CONNECTIONS_CONTEXT_MENU_REMOVE);
    pDeleteConnectionRowAction->setEnabled(canDelete);

    // Get global position
    QPoint globalPos = QCursor::pos();

    // Execute menu actions
    QAction* pAction = menu.exec(globalPos);

    // Only open the ContextMenu if the user selected a row that has something in it.
    if (validRow && (pAction != nullptr))
    {
        int selectedRowIndex = selectedCellIndex.row();

        if (pAction == pConnectToConnectionRow)
        {
            RDSConnectionInfo selectedConnectionInfo = {};
            m_pRecentConnectionsModel->GetConnectionInfoAtRow(selectedRowIndex, selectedConnectionInfo);

            emit ConnectionRequested(selectedConnectionInfo);

        }
        else if (pAction == pDeleteConnectionRowAction)
        {
            m_pRecentConnectionsModel->RemoveConnectionInfoRow(selectedRowIndex);
        }
    }
}

//-----------------------------------------------------------------------------
/// Handler invoked when the remove connection button is clicked.
//-----------------------------------------------------------------------------
void RecentConnectionsView::OnRemoveConnectionButtonClicked()
{
    // Get the index of the selected row
    QModelIndex selectionIndex = ui->RecentConnectionsList->selectionModel()->currentIndex();
    int selectedRowIndex = selectionIndex.row();

    // Remove connection info for the selected row
    m_pRecentConnectionsModel->RemoveConnectionInfoRow(selectedRowIndex);
}

//-----------------------------------------------------------------------------
/// Handler invoked when the clear all recent connections button is clicked.
//-----------------------------------------------------------------------------
void RecentConnectionsView::OnClearConnectionsButtonClicked()
{
    // Show message box for user confirmation of the clear
    NotificationWidget::Button selectedButton = RDPUtil::ShowNotification(
            gs_RECENT_CONNECTIONS_CLEAR_POPUP_TITLE,
            gs_RECENT_CONNECTIONS_CLEAR_POPUP_TEXT,
            NotificationWidget::Button::Yes | NotificationWidget::Button::No);

    // Clear all connection rows when user confirms
    if (selectedButton == NotificationWidget::Button::Yes)
    {
        m_pRecentConnectionsModel->ClearConnectionInfoRows();
    }
}

//-----------------------------------------------------------------------------
/// Slot to handle when the number of rows in the Recent Connections list changes.
/// \param parent The parent Model Index.
/// \param first The index number of the first row added or removed.
/// \param last The index number of the last row added or removed.
//-----------------------------------------------------------------------------
void RecentConnectionsView::OnRowCountChanged(const QModelIndex& parent, int first, int last)
{
    Q_UNUSED(parent);
    Q_UNUSED(first);
    Q_UNUSED(last);

    EnableRemoveButtonsCheck();
}
