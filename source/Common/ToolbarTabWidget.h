//=============================================================================
/// Copyright (c) 2016-2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Implementation of a tab widget with toolbar space in the unused tab
/// bar area. This allows other widgets to be added into the empty tab space.
//=============================================================================

#ifndef _TOOLBAR_TAB_WIDGET_H_
#define _TOOLBAR_TAB_WIDGET_H_

#include <QTabWidget>
#include <QAbstractButton>
#include <QTabBar>
#include <QHBoxLayout>

/// TabToolbar widget. This is a QAbstractButton that acts as a basic QWidget
/// container by passing its' paint event to the QWidget paint event function.
class TabToolbar : public QAbstractButton
{
    virtual void paintEvent(QPaintEvent* e) { QWidget::paintEvent(e); }
};

/// Tab widget that allows the empty tab bar space to be used as a toolbar.
class ToolbarTabWidget : public QTabWidget
{
public:
    explicit ToolbarTabWidget(QWidget* pParent = nullptr);
    virtual ~ToolbarTabWidget();

    void addWidgetToToolbar(QWidget* pWidget, QTabBar::ButtonPosition position);
    void setToolbarContentsMargins(int left, int top, int right, int bottom);

private:
    TabToolbar* m_pToolbar;     ///< Tab toolbar instance.
    QHBoxLayout* m_pLayout;     ///< Toolbar layout used to layout all toolbar widgets.
    int m_spacerIndex;          ///< Index of the spacer item in the toolbar layout.
    int m_tabHeight;            ///< Current tab height used for toolbar calculations.
    int m_lastTabWidth;         ///< Current last tab width used for toolbar calculations.

protected:
    virtual void tabInserted(int index);
    virtual bool event(QEvent* pEvent);
};

#endif // _TOOLBAR_TAB_WIDGET_H_
