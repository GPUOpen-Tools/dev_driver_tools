//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Header for DevDriverTools' debug window.
//=============================================================================

#ifndef _DEBUG_WINDOW_H_
#define _DEBUG_WINDOW_H_

#include <QDialog>

namespace Ui
{
class DebugWindow;
}

/// Support for DevDriverTools' debug window.
class DebugWindow : public QDialog
{
    Q_OBJECT

public:
    explicit DebugWindow(QWidget* pParent = nullptr);
    ~DebugWindow();

    void SetReadOnly(bool readOnly);

signals:
    void EmitSetText(const QString& str);

private slots:
    void SetText(const QString& str);

private:
    void ScrollToBottom();

    Ui::DebugWindow* ui;
};

#endif // _DEBUG_WINDOW_H_
