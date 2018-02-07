//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A notification overlay that can be used to display messages to
///         the user on top of the RDP window contents.
//=============================================================================

#ifndef _NOTIFICATION_WIDGET_H_
#define _NOTIFICATION_WIDGET_H_

#include <QGraphicsBlurEffect>
#include <QPainter>
#include <QResizeEvent>
#include <QVector>
#include <QWidget>

class QButtonGroup;
class QPushButton;

namespace Ui {
class NotificationWidget;
}

/// A container that handles overlaying one widget on top of another.
class ContainerWidget : public QWidget
{
    friend class NotificationWidget;
public:
    explicit ContainerWidget(QWidget* pParent = nullptr);

    void SetSize(QObject* pObject);
    void SetBackgroundWidget(QWidget* pBackgroundWidget);
    void SetOverlayWidget(QWidget* pOverlay);
    void ShowOverlay();
    void HideOverlay();

protected:
    virtual void resizeEvent(QResizeEvent* pEvent);

private:
    void SetEnableBlur(bool enable);

    QWidget* m_pBackgroundWidget;   ///< The widget that appears obscured behind an overlay.
    QWidget* m_pOverlayWidget;      ///< The widget that is overlaid on top of the background widget.
};

/// The notification widget overlay class declaration.
class NotificationWidget : public QWidget
{
    Q_OBJECT
public:
    /// An enumeration containing all of the button choices to display to the user.
    enum Button
    {
        Unset   = 1 << 0,
        Ok      = 1 << 1,
        Yes     = 1 << 2,
        No      = 1 << 3,
        Browse  = 1 << 4,
        Revert  = 1 << 5,
        Cancel  = 1 << 6,
        Button_Max
    };

public:
    explicit NotificationWidget(ContainerWidget* pParent = nullptr);
    virtual ~NotificationWidget();

    void SetTitle(const QString& titleText);
    void SetText(const QString& text);
    void SetButtons(uint buttons, uint defaultButtons);
    Button GetResult() const { return m_result; }
    void ShowDoNotAsk(bool showCheckbox);
    bool GetShowDoNotAsk() const { return m_showDoNotAskAgain; }
    bool GetIsDoNotAskChecked() const;

protected:
    virtual void showEvent(QShowEvent* pEvent);
    void NewParentSet();

private slots:
    void OnButtonClicked(int buttonId);

private:
    void InitializeButtons();
    void DestroyButtons();

    Ui::NotificationWidget* ui;             ///< The notification overlay's interface design.
    ContainerWidget* m_pParentContainer;    ///< The parent widget responsible for rendering the overlay.
    QVector<QPushButton*> m_buttonOptions;  ///< The set of buttons added as options.
    QButtonGroup* m_pOptionsGroup;          ///< The group of buttons presented to the user.
    Button m_result;                        ///< The result choice for the last notification.
    uint m_defaultButtonBitIndex;           ///< A bit index for the default button.
    uint m_buttons;                         ///< A bitfield of buttons to display.
    bool m_showDoNotAskAgain;               ///< A flag that determines if the "Don't show me again" checkbox will appear.
};

#endif // _NOTIFICATION_WIDGET_H_
