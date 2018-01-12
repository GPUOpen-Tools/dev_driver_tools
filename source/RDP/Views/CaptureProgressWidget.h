//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A widget that shows the progress of a trace being captured.
//=============================================================================

#ifndef _CAPTURE_PROGRESS_WIDGET_H_
#define _CAPTURE_PROGRESS_WIDGET_H_

#include <QWidget>
#include "../../DevDriverComponents/inc/gpuopen.h"

namespace Ui {
class CaptureProgressWidget;
}

/// A panel that shows the progress of a trace being captured.
class CaptureProgressWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CaptureProgressWidget(QWidget* pParent = 0);
    ~CaptureProgressWidget();

signals:
    void TraceCancelled();

public slots:
    void OnTraceProgressUpdated(quint64 recievedBytes, quint64 totalBytes, quint64 receiveRateInMb);

private slots:
    void OnCancelTraceClicked(bool checked);

private:
    Ui::CaptureProgressWidget *ui;
};

#endif // _CAPTURE_PROGRESS_WIDGET_H_
