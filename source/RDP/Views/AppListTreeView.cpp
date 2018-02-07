//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of the Application List Treeview.
//=============================================================================

#include <QApplication>
#include <QTreeView>
#include <QHeaderView>
#include <QMouseEvent>
#include <QStandardItemModel>

#include "AppListTreeView.h"
#include "../Settings/RDPSettings.h"
#include "../Models/SetupTargetApplicationModel.h"

//-----------------------------------------------------------------------------
/// Explicit Constructor
/// \param pParent The parent widget.
//-----------------------------------------------------------------------------
AppListTreeView::AppListTreeView(QWidget* pParent) :
    QTreeView(pParent),
    m_pTargetApplicationModel(nullptr)
{
    qApp->installEventFilter(this);

    // Use custom header with the tree view
    AppListHeaderView* pHeader = new AppListHeaderView(Qt::Horizontal);
    this->setHeader(pHeader);
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
AppListTreeView::~AppListTreeView()
{
}

//-----------------------------------------------------------------------------
/// Set the target application model. This is the model for the whole target
/// application pane and not the model associated with this treeview
/// \param pModel The target application model
//-----------------------------------------------------------------------------
void AppListTreeView::SetTargetApplicationModel(SetupTargetApplicationModel* pModel)
{
    m_pTargetApplicationModel = pModel;
}

//-----------------------------------------------------------------------------
/// EventFilter overridden to handle custom spacebar behavior.
/// \param pObj Pointer to object that the event is destined for.
/// \param pEvent Pointer to mouse released event class
//-----------------------------------------------------------------------------
bool AppListTreeView::eventFilter(QObject* pObj, QEvent* pEvent)
{
    bool eventHandled = false;
    eventHandled = QObject::eventFilter(pObj, pEvent);

    if ( hasFocus() && (pEvent->type() == QEvent::KeyPress) )
    {
        QKeyEvent* key = static_cast<QKeyEvent*>(pEvent);
        if (key->key() == Qt::Key_Space)
        {
            if (currentIndex().column() == TARGET_APPLICATION_TABLE_COLUMN_ENABLE_PROFILING)
            {
                if (m_pTargetApplicationModel != nullptr)
                {
                    m_pTargetApplicationModel->ToggleProfilingForRow(m_pTargetApplicationModel->SetupTargetApplicationModel::MapToSourceModelRow(currentIndex()));
                    eventHandled = true;
                }
            }
        }
    }
    return eventHandled;
}

//-----------------------------------------------------------------------------
/// Overridden mouse release event handler
/// \param pEvent Pointer to mouse released event class
//-----------------------------------------------------------------------------
void AppListTreeView::mouseReleaseEvent(QMouseEvent* pEvent)
{
    QModelIndex modelIndex = this->indexAt(pEvent->pos());
    if (modelIndex.isValid())
    {
        if ( (modelIndex.flags() & Qt::ItemFlag::ItemIsUserCheckable) && (modelIndex.column() == TARGET_APPLICATION_TABLE_COLUMN_ENABLE_PROFILING) )
        {
            if (pEvent->button() & Qt::MouseButton::RightButton)
            {
                emit clicked(modelIndex);
                pEvent->accept();
                return;
            }

            // Find the bounding rectangle for the checkbox on the cell.
            QStyleOptionButton opt;
            opt.QStyleOption::operator=(viewOptions());
            opt.rect = visualRect(modelIndex);
            QRect rect = style()->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, &opt);
            QHeaderView* pHeader = header();
            if (pHeader)
            {
               int margin = pHeader->style()->pixelMetric(QStyle::PM_FocusFrameHMargin, 0, pHeader);
               rect.moveLeft(rect.left() + margin);
            }

            // If only the cell of the last column, and not the checkbox item, is clicked, just select the row
            // without toggling the checkbox state.
            if (!rect.contains(pEvent->pos(), true))
            {
                selectionModel()->setCurrentIndex(modelIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
                emit clicked(modelIndex);
                pEvent->accept();
                return;
            }
        }
    }

    QTreeView::mouseReleaseEvent(pEvent);
    if (m_pTargetApplicationModel != nullptr)
    {
        m_pTargetApplicationModel->TargetApplicationTableClicked(modelIndex);
        pEvent->accept();
    }
    else
    {
        pEvent->ignore();
    }
}

//-----------------------------------------------------------------------------
/// Overridden mouse press event handler for the header view.
/// \param pEvent Pointer to mouse pressed event class.
//-----------------------------------------------------------------------------
void AppListHeaderView::mousePressEvent(QMouseEvent* pEvent)
{
    // Only pass mouse events on if the first column is clicked - ignore all others
    int index = logicalIndexAt(pEvent->pos());
    if (index == 0)
    {
        QHeaderView::mousePressEvent(pEvent);
    }
}
