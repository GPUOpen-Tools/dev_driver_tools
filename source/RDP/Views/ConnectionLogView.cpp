//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of the Connection Log View.
//=============================================================================

#include <QScrollBar>
#include <QFileDialog>
#include <QTextStream>
#include <QClipboard>

#include "ConnectionLogView.h"
#include "ui_ConnectionLogView.h"
#include "../RDPDefinitions.h"
#include "../Util/RDPUtil.h"

//-----------------------------------------------------------------------------
/// Explicit Constructor for the Log View
/// \param pParent The parent widget.
//-----------------------------------------------------------------------------
ConnectionLogView::ConnectionLogView(QWidget* pParent) :
    QWidget(pParent),
    ui(new Ui::ConnectionLogView)
{
    ui->setupUi(this);

    ToolUtil::SetWidgetBackgroundColor(this, Qt::white);

    connect(ui->clearButton, &QPushButton::clicked, this, &ConnectionLogView::Clear);
    connect(ui->saveAsButton, &QPushButton::clicked, this, &ConnectionLogView::SaveLog);
    connect(ui->copyButton, &QPushButton::clicked, this, &ConnectionLogView::CopyToClipboard);

    // Disable the "Save as..." and "Clear" buttons for now since the log is empty
    ui->saveAsButton->setEnabled(false);
    ui->clearButton->setEnabled(false);
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
ConnectionLogView::~ConnectionLogView()
{
    delete ui;
}

//-----------------------------------------------------------------------------
/// Add a message to the log window
/// \param logString The string to be added
//-----------------------------------------------------------------------------
void ConnectionLogView::AddLogMessage(const QString& logString)
{
    ui->plainTextEdit->appendPlainText(logString);
    QScrollBar* pScrollBar = ui->plainTextEdit->verticalScrollBar();
    pScrollBar->setValue(pScrollBar->maximum());

    // Enable the "Save as..." and "Clear" buttons
    ui->saveAsButton->setEnabled(true);
    ui->clearButton->setEnabled(true);
}

//-----------------------------------------------------------------------------
/// Clear the log pane
/// \param checked The button state for toggle buttons (unused)
//-----------------------------------------------------------------------------
void ConnectionLogView::Clear(bool checked)
{
    Q_UNUSED(checked);
    ui->plainTextEdit->setPlainText("");

    // Disable the "Save as..." and "Clear" buttons since the log is empty now
    ui->saveAsButton->setEnabled(false);
    ui->clearButton->setEnabled(false);
}

//-----------------------------------------------------------------------------
/// Save the log file data to disk
/// \param checked The button state for toggle buttons (unused)
//-----------------------------------------------------------------------------
void ConnectionLogView::SaveLog(bool checked)
{
    Q_UNUSED(checked);

    // Get the user to select a directory to export to
    const QString filename = QFileDialog::getSaveFileName(this, "Save log file", gs_LOG_FILE_NAME_TEXT, "RDS log files (*.txt)");

    // If the user cancels, do nothing. Otherwise save the file
    if (!filename.isEmpty())
    {
        const QFileInfo fileInfo(filename);

        // Make sure that the directory for the file is created before writing.
        QDir filepathDir = fileInfo.absoluteDir();

        bool pathCreated = true;
        if (!filepathDir.exists())
        {
            pathCreated = filepathDir.mkdir(filepathDir.absolutePath());
        }

        if (pathCreated)
        {
            QFile file(fileInfo.absoluteFilePath());
            bool success = file.open(QFile::WriteOnly | QFile::Text);
            if (success == true)
            {
                QTextStream outStream(&file);
                outStream << ui->plainTextEdit->toPlainText();
            }
            else
            {
                RDPUtil::DbgMsg("[RDP] Can't create log file %s", file.fileName().toLatin1().data());
            }
        }
    }
}

//-----------------------------------------------------------------------------
/// Copy the contents of the log window to the clipboard
/// \param checked The button state for toggle buttons (unused)
//-----------------------------------------------------------------------------
void ConnectionLogView::CopyToClipboard(bool checked)
{
    Q_UNUSED(checked);

    QClipboard* pClipboard = QGuiApplication::clipboard();
    Q_ASSERT(pClipboard != nullptr);

    if (pClipboard != nullptr)
    {
        pClipboard->setText(ui->plainTextEdit->toPlainText());
    }
}
