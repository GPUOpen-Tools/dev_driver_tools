//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A widget that shows the status of a trace being captured.
//=============================================================================

#include "CaptureProgressWidget.h"
#include "ui_CaptureProgressWidget.h"
#include "../RDPDefinitions.h"
#include "../Util/RDPUtil.h"
#include "../../Common/DriverToolsDefinitions.h"
#include "../../Common/ToolUtil.h"
#include <QtUtil.h>

//-----------------------------------------------------------------------------
/// Constructor for the progress panel.
/// \param pParent The parent widget for the progress panel.
//-----------------------------------------------------------------------------
CaptureProgressWidget::CaptureProgressWidget(QWidget* pParent) :
    QWidget(pParent),
    ui(new Ui::CaptureProgressWidget)
{
    // Initialize the UI and set white background color
    ui->setupUi(this);

    ToolUtil::SetWidgetBackgroundColor(this, Qt::white);

    // Update to use the Busy cursor when a trace is being collected.
    setCursor(Qt::BusyCursor);

    // Connect the cancel button to the OnClicked handler.
    connect(ui->cancelTraceButton, &QPushButton::clicked, this, &CaptureProgressWidget::OnCancelTraceClicked);
}

//-----------------------------------------------------------------------------
/// Destructor for the progress panel.
//-----------------------------------------------------------------------------
CaptureProgressWidget::~CaptureProgressWidget()
{
    SAFE_DELETE(ui);

    // Revert back to a normal Arrow cursor when the trace is finished.
    setCursor(Qt::ArrowCursor);
}

//-----------------------------------------------------------------------------
/// Handler invoked when the Cancel trace button is clicked.
/// \param Checked a flag indicating if the button is checked or not.
//-----------------------------------------------------------------------------
void CaptureProgressWidget::OnCancelTraceClicked(bool checked)
{
    Q_UNUSED(checked);

    // Signal canceling a trace.
    emit TraceCancelled();
}

//-----------------------------------------------------------------------------
/// Handler invoked during trace collection, upon reading each new chunk.
/// \param recievedBytes The current transferred byte count in the trace file being collected.
/// \param totalBytes The total number of bytes in the trace file being collected.
/// \param bytesPerSec The current transfer rate in bytes/second.
//-----------------------------------------------------------------------------
void CaptureProgressWidget::OnTraceProgressUpdated(quint64 recievedBytes, quint64 totalBytes, quint64 bytesPerSec)
{
    // Convert the transferred byte count and total trace size to sensible units.
    QString sizeString;
    QtCommon::QtUtil::GetFilesizeAcronymFromByteCount(recievedBytes, sizeString);

    QString totalSizeString;
    QtCommon::QtUtil::GetFilesizeAcronymFromByteCount(totalBytes, totalSizeString);

    // Set the progress meter's text to the last transfer size update.
    QString transferProgress = gs_RGP_TRACE_PROGRESS_RECEIVED.arg(sizeString, totalSizeString);
    ui->transferProgressLabel->setText(transferProgress);

    // @TODO: Work in progress. Add a transfer rate label when it's computed correctly.
    Q_UNUSED(bytesPerSec);

    if (totalBytes != 0)
    {
        // Compute the transfer percentage and set the progress bar value.
        float transferCoefficient = recievedBytes / static_cast<float>(totalBytes);
        int progressPercentage = transferCoefficient * ui->progressBar->maximum();
        ui->progressBar->setValue(progressPercentage);
    }
    else
    {
        // Backwards compatibility workaround for RGPClient protocol < 4:
        // The total trace size isn't known upfront, so we can't display a legitimate progress bar.
        // Fall back to displaying the "indeterminate progress" state in the progress bar.
        ui->progressBar->setMinimum(0);
        ui->progressBar->setMaximum(0);
    }
}
