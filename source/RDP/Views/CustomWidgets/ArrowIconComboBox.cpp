//=============================================================================
/// Copyright (c) 2018 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of our down arrow combobox
//=============================================================================

#include "ArrowIconComboBox.h"

#include <QApplication>
#include <ListWidget.h>

//-----------------------------------------------------------------------------
/// Constructor for ArrowIconComboBox.
/// \param pParent The parent widget
//-----------------------------------------------------------------------------
ArrowIconComboBox::ArrowIconComboBox(QObject* pParent)
{
    Q_UNUSED(pParent);
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
ArrowIconComboBox::~ArrowIconComboBox()
{
    delete m_pItemList;
}

//-----------------------------------------------------------------------------
/// Init the widget
/// \param pParent The parent widget
/// \param defaultText The starting string
/// \param whether want this guy to remain open as multiple things are selected
//-----------------------------------------------------------------------------
void ArrowIconComboBox::Init(QWidget* pParent, const QString& defaultText, bool multiSelect)
{
    // create list widget for the combo box
    m_pItemList = new ListWidget(pParent, this, false);

    // also disable scrollbars on this list widget
    m_pItemList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pItemList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // start out the combo box with the first entry
    m_pItemList->setCurrentRow(0);
    m_pItemList->hide();

    m_multiSelect = multiSelect;
    m_defaultText = defaultText;

    ResetSelection();

    if (multiSelect == true)
    {
        m_pItemList->setSelectionMode(QAbstractItemView::SelectionMode::MultiSelection);
    }

    connect(m_pItemList, &ListWidget::itemClicked, this, &ArrowIconComboBox::ListItemClicked);

    qApp->installEventFilter(this);
}

//-----------------------------------------------------------------------------
/// Update widget state
//-----------------------------------------------------------------------------
void ArrowIconComboBox::Update()
{
    if (m_pItemList->count() == 0)
    {
        CloseMenu();

        ResetSelection();
    }
}

//-----------------------------------------------------------------------------
/// Reset to starting state
//-----------------------------------------------------------------------------
void ArrowIconComboBox::ResetSelection()
{
    SetSelectedText(m_defaultText);
}

//-----------------------------------------------------------------------------
/// Throw it all out
//-----------------------------------------------------------------------------
void ArrowIconComboBox::ClearItems()
{
    m_pItemList->clear();
}

//-----------------------------------------------------------------------------
/// Add a new list item.
/// \param newItem The string to be added to the list.
//-----------------------------------------------------------------------------
void ArrowIconComboBox::AddItem(const QString& newItem)
{
    m_pItemList->addItem(newItem);
}

//-----------------------------------------------------------------------------
/// Remove a list item.
/// \param itemString The string to be removed from the list.
//-----------------------------------------------------------------------------
void ArrowIconComboBox::RemoveItem(const QString& itemString)
{
    const QList<QListWidgetItem*> itemList = m_pItemList->findItems(itemString, Qt::MatchExactly);
    foreach(QListWidgetItem* pItem, itemList)
    {
        m_pItemList->removeItemWidget(pItem);
        delete pItem;
    }
}

//-----------------------------------------------------------------------------
/// Open the list
//-----------------------------------------------------------------------------
void ArrowIconComboBox::OpenMenu()
{
    if (m_pItemList->count() > 0)
    {
        SetDirection(ArrowIconWidget::UpArrow);

        m_pItemList->show();
    }
}

//-----------------------------------------------------------------------------
/// Close the list
//-----------------------------------------------------------------------------
void ArrowIconComboBox::CloseMenu()
{
    SetDirection(ArrowIconWidget::DownArrow);
    m_pItemList->hide();
}

//-----------------------------------------------------------------------------
/// Event filter to make the list close when clicking outside of widget
/// \param pObject the test object
/// \param pEvent the actual event
/// \return pass or fail
//-----------------------------------------------------------------------------
bool ArrowIconComboBox::eventFilter(QObject* pObject, QEvent* pEvent)
{
    Q_UNUSED(pObject);

    if (pEvent->type() == QEvent::MouseButtonPress)
    {
        if ((this->underMouse() == false) && (m_pItemList->underMouse() == false))
        {
            CloseMenu();
        }
    }

    return false;
}

//-----------------------------------------------------------------------------
/// Handle what happens when a list item is clicked
/// \param pItem the clicked item
//-----------------------------------------------------------------------------
void ArrowIconComboBox::ListItemClicked(QListWidgetItem* pItem)
{
    if (pItem != nullptr)
    {
        SetSelectedText(pItem->text());

        if (m_multiSelect == false)
        {
            CloseMenu();
        }
    }
}

//-----------------------------------------------------------------------------
/// Toggle the list when the master button is clicked
/// \param pItem the clicked item
//-----------------------------------------------------------------------------
void ArrowIconComboBox::mousePressEvent(QMouseEvent* pEvent)
{
    Q_UNUSED(pEvent);

    if (m_pItemList->isVisible())
    {
        CloseMenu();
    }
    else
    {
        OpenMenu();
    }
}

//-----------------------------------------------------------------------------
/// Cache the last user selection
/// \param selection the selected text
//-----------------------------------------------------------------------------
void ArrowIconComboBox::SetSelectedText(const QString& selection)
{
    setText(selection);
    m_selectedText = selection;

    emit SelectionChanged();
}
