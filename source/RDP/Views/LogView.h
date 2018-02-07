//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for the Log View.
//=============================================================================

#ifndef _LOGVIEW_H_
#define _LOGVIEW_H_

#include <QWidget>

namespace Ui {
class LogView;
}

/// class to handle RDP's log output
class LogView : public QWidget
{
    Q_OBJECT

public:
    explicit LogView(QWidget* pParent = nullptr);
    ~LogView();

    void AddLogMessage(const QString& logString);

private slots:
    void Clear(bool checked);
    void SaveLog(bool checked);

private:
    Ui::LogView *ui;
};

#endif // _LOGVIEW_H_
