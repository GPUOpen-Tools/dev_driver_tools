//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for the Connection Log View.
//=============================================================================

#ifndef _CONNECTION_LOG_VIEW_H_
#define _CONNECTION_LOG_VIEW_H_

#include <QWidget>

namespace Ui {
class ConnectionLogView;
}

/// class to handle RDP's connection log output
class ConnectionLogView : public QWidget
{
    Q_OBJECT

public:
    explicit ConnectionLogView(QWidget* pParent = nullptr);
    ~ConnectionLogView();

    void AddLogMessage(const QString& logString);

private slots:
    void Clear(bool checked);
    void SaveLog(bool checked);
    void CopyToClipboard(bool checked);

private:
    Ui::ConnectionLogView *ui;      ///< Qt ui component
};

#endif // _CONNECTION_LOG_VIEW_H_
