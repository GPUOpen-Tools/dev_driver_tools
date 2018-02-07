//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of the Log View.
//=============================================================================

#include <QScrollBar>
#include <QFileDialog>
#include <QTextStream>

#include "LogView.h"
#include "ui_LogView.h"
#include "../RDPDefinitions.h"
#include "../Util/RDPUtil.h"

//-----------------------------------------------------------------------------
/// Explicit Constructor for the Log View
/// \param pParent The parent widget.
//-----------------------------------------------------------------------------
LogView::LogView(QWidget* pParent) :
    QWidget(pParent),
    ui(new Ui::LogView)
{
    ui->setupUi(this);

    ToolUtil::SetWidgetBackgroundColor(this, Qt::white);

    connect(ui->clearButton, SIGNAL(clicked(bool)), this, SLOT(Clear(bool)));
    connect(ui->saveAsButton, SIGNAL(clicked(bool)), this, SLOT(SaveLog(bool)));

    // Disable the "Save as..." and "Clear" buttons for now since the log is empty
    ui->saveAsButton->setEnabled(false);
    ui->clearButton->setEnabled(false);
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
LogView::~LogView()
{
    delete ui;
}

//-----------------------------------------------------------------------------
/// Add a message to the log window
/// \param logString The string to be added
//-----------------------------------------------------------------------------
void LogView::AddLogMessage(const QString& logString)
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
void LogView::Clear(bool checked)
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
void LogView::SaveLog(bool checked)
{
    Q_UNUSED(checked);

    // Get the user to select a directory to export to
    QString filename = QFileDialog::getSaveFileName(this, "Save log file", gs_LOG_FILE_NAME_TEXT, "RDS Log files (*.txt)");

    // If the user cancels, do nothing. Otherwise save the file
    if (!filename.isEmpty())
    {
        QFileInfo fileInfo(filename);

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
