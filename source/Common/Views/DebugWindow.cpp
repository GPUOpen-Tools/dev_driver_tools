//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of DevDriverTools' debug window.
//=============================================================================

#include "DebugWindow.h"
#include "ui_DebugWindow.h"

#include <QApplication>
#include <QtDebug>
#include <QtGlobal>
#include <QScrollBar>

//-----------------------------------------------------------------------------
/// Explicit constructor.
/// \param pParent The dialog's parent.
//-----------------------------------------------------------------------------
DebugWindow::DebugWindow(QWidget* pParent) :
    QDialog(pParent),
    ui(new Ui::DebugWindow)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // Use monospace font style so that things align
    QFont font("unexistent");
    font.setStyleHint(QFont::Monospace);
    ui->plainTextEdit->setFont(font);

    connect(this, SIGNAL(EmitSetText(QString)), this, SLOT(SetText(QString)));
}

//-----------------------------------------------------------------------------
/// Destructor.
//-----------------------------------------------------------------------------
DebugWindow::~DebugWindow()
{
    delete ui;
}

//-----------------------------------------------------------------------------
/// Make ths text area read only
/// \param readOnly mark it as read only
//-----------------------------------------------------------------------------
void DebugWindow::SetReadOnly(bool readOnly)
{
    ui->plainTextEdit->setReadOnly(readOnly);
}

//-----------------------------------------------------------------------------
/// Helper function which to automatically scroll to the bottom on new line.
//-----------------------------------------------------------------------------
void DebugWindow::ScrollToBottom()
{
    QScrollBar* pScrollBar = ui->plainTextEdit->verticalScrollBar();
    pScrollBar->setValue(pScrollBar->maximum());
}

//-----------------------------------------------------------------------------
/// Add a new line of text to the debug window.
/// \param str The new line of text.
//-----------------------------------------------------------------------------
void DebugWindow::SetText(const QString& str)
{
    ui->plainTextEdit->appendPlainText(str);
    ScrollToBottom();
}
