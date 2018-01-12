//=============================================================================
/// Copyright (c) 2016-2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Implementation of a tab widget with toolbar space in the unused tab
/// bar area. This allows other widgets to be added into the empty tab space.
//=============================================================================

#include "ToolbarTabWidget.h"

#include <QEvent>
#include <QResizeEvent>
#include <QTabBar>
#include <QLayout>
#include <ScalingManager.h>
#include "DriverToolsDefinitions.h"

const static QString s_TOOLBAR_TAB_STYLESHEET(
"QTabBar::tab:last"
"{"
"   width: %1px;"
"   height: %2px;"
"   padding: 0px;"
"   margin: 0px;"
"   border: 0px;"
"}"
);

//-----------------------------------------------------------------------------
/// Constructor.
//-----------------------------------------------------------------------------
ToolbarTabWidget::ToolbarTabWidget(QWidget* pParent) : QTabWidget(pParent)
{
    m_pToolbar = new TabToolbar;
    m_pLayout = new QHBoxLayout(m_pToolbar);

    // Setup toolbar
    m_pLayout->setContentsMargins(0, 0, 0, 0);
    m_pLayout->setDirection(QBoxLayout::LeftToRight);
    m_pLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum));

    // Add end tab
    tabBar()->addTab("");
    tabBar()->setTabButton(0, QTabBar::LeftSide, m_pToolbar);
    setTabEnabled(0, false);

    // Set initial index of spacer
    m_spacerIndex = 0;

    // Set initial tab parameters
    m_tabHeight = 0;
    m_lastTabWidth = 0;
}

//-----------------------------------------------------------------------------
/// Destructor.
//-----------------------------------------------------------------------------
ToolbarTabWidget::~ToolbarTabWidget()
{
    SAFE_DELETE(m_pLayout);
}

//-----------------------------------------------------------------------------
/// Event handler to intercept layout request and resize events, where the toolbar
/// tab size needs to be calculated and applied.
/// \param pEvent Pointer to the qt event.
/// \return Event accept/reject state (passed from QTabWidget::event).
//-----------------------------------------------------------------------------
bool ToolbarTabWidget::event(QEvent* pEvent)
{
    // Process event normally
    bool result = QTabWidget::event(pEvent);

    // Size adjust on layout request and resize events
    if (pEvent->type() == QEvent::LayoutRequest || pEvent->type() == QEvent::Resize)
    {
        // Calculate width of last tab such that it fills the entire tab bar width
        int tabWidth = tabBar()->tabRect(0).width();
        int tabHeight = tabBar()->tabRect(0).height();
        int lastTabWidth = size().width() - (tabWidth * (count() - 1));

        // Only update stylesheet and toolbar when tab values have changed
        if (tabHeight != m_tabHeight || lastTabWidth != m_lastTabWidth)
        {
            m_tabHeight = tabHeight;
            m_lastTabWidth = lastTabWidth;

            // Set the width of the last tab
            tabBar()->setStyleSheet(s_TOOLBAR_TAB_STYLESHEET.arg(lastTabWidth).arg(tabHeight));

            // Set toolbar size
            m_pToolbar->setFixedWidth(lastTabWidth - 4); // Make the toolbar slightly smaller than the tab to account for the left hand padding (can't remove)
            m_pToolbar->setFixedHeight(tabHeight);
        }
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Tab insert handling. This is called just after a new tab is inserted, and
/// makes sure that the toolbar tab gets moved to the end.
/// \param index The index of the newly inserted tab.
//-----------------------------------------------------------------------------
void ToolbarTabWidget::tabInserted(int index)
{
    // Guarantee the toolbar tab stays at the end
    if (index == count() - 1)
    {
        tabBar()->moveTab(index - 1, index);
    }
}

//-----------------------------------------------------------------------------
/// Add a widget to the toolbar at the given position. Widgets added to the left
/// side will be displayed in left to right order and will be left aligned in
/// the toolbar space. Widgets added to the right side will be displayed in
/// right to left order and will be right aligned in the toolbar space.
/// \param position The position and alignment of the widget in the toolbar.
//-----------------------------------------------------------------------------
void ToolbarTabWidget::addWidgetToToolbar(QWidget* pWidget, QTabBar::ButtonPosition position)
{
    // Left side items are added just before the spacer
    if (position == QTabBar::LeftSide)
    {
        m_pLayout->insertWidget(m_spacerIndex, pWidget);
        m_spacerIndex++;
    }

    // Right side items are added just after the spacer
    if (position == QTabBar::RightSide)
    {
        m_pLayout->insertWidget(m_spacerIndex + 1, pWidget);
    }
}

//-----------------------------------------------------------------------------
/// Set the contents margins of the layout used for the toolbar.
/// \param left Left side margin.
/// \param top Top side margin.
/// \param right Right side margin.
/// \param bottom Bottom side margin.
//-----------------------------------------------------------------------------
void ToolbarTabWidget::setToolbarContentsMargins(int left, int top, int right, int bottom)
{
    m_pLayout->setContentsMargins(left, top, right, bottom);
}
