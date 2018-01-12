//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  The configuration window is where the user can alter RDS configuration.
//=============================================================================

#ifndef _CONFIGURATION_WINDOW_H_
#define _CONFIGURATION_WINDOW_H_

#include <QDialog>

class ConfigurationWindowModel;

namespace Ui {
class ConfigurationWindow;
}

/// Class declaration for the Configuration Window implementation.
class ConfigurationWindow : public QDialog
{
    Q_OBJECT
public:
    explicit ConfigurationWindow(QDialog* pParent = nullptr);
    ~ConfigurationWindow();

    void EnableChangingPort(bool enabled);
    bool eventFilter(QObject* pTarget, QEvent* pEvent);

signals:
    void ListenEndpointUpdated();

public slots:
    void OnListenPortUpdated(unsigned int port);

private slots:
    void OnListenPortTextChanged(const QString& portText);
    void OnDefaultListenPortClicked();

private:
    Ui::ConfigurationWindow *ui;

    ConfigurationWindowModel* m_pConfigurationModel;
    QIcon* m_pWindowIcon;
};

#endif // _CONFIGURATION_WINDOW_H_
