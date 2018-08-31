//=============================================================================
/// Copyright (c) 2018 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for our down arrow combobox
//=============================================================================

#ifndef _ARROW_ICON_COMBO_BOX_H_
#define _ARROW_ICON_COMBO_BOX_H_

#include <ArrowIconWidget.h>

class ListWidget;
class QListWidgetItem;

/// Class that implements the down arrow combobox widgets seen in RGP
class ArrowIconComboBox : public ArrowIconWidget
{
    Q_OBJECT

public:
    explicit ArrowIconComboBox(QObject* pParent = nullptr);
    virtual ~ArrowIconComboBox();

    void mousePressEvent(QMouseEvent* pEvent);
    void SetSelectedText(const QString& selection);
    void Init(QWidget* pParent, const QString& defaultText, bool multiSelect);
    void ClearItems();
    void AddItem(const QString& newItem);
    void RemoveItem(const QString& itemString);
    void OpenMenu();
    void CloseMenu();
    bool eventFilter(QObject* pObject, QEvent* pEvent);
    void Update();
    void ResetSelection();

    QString SelectedText() { return m_selectedText; }
    QString DefaultText() { return m_defaultText; }

signals:
    void SelectionChanged();

private slots:
    void ListItemClicked(QListWidgetItem* pItem);

private:
    ListWidget * m_pItemList;   ///< The list of items
    QString m_defaultText;      ///< The default starting string
    QString m_selectedText;     ///< Track selected text
    bool m_multiSelect;         ///< Whether this guy supports multi-selection
};

#endif // _ARROW_ICON_COMBO_BOX_H_
