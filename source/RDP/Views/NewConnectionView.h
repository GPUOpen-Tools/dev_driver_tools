//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for the New Connection view interface.
//=============================================================================

#ifndef _NEW_CONNECTION_VIEW_H_
#define _NEW_CONNECTION_VIEW_H_

#include <QWidget>

class ConnectionSettingsModel;
class QPushButton;

namespace Ui {
class NewConnectionView;
}

/// class to handle the new connection view
class NewConnectionView : public QWidget
{
    Q_OBJECT

public:
    explicit NewConnectionView(ConnectionSettingsModel* pConnectionSettingsModel, QWidget* pParent = nullptr);
    virtual ~NewConnectionView();

    QObject* GetConnectButton() const;
    QObject* GetIPAddressLineEdit() const;
    QObject* GetPortNumberLineEdit() const;
    bool     IsIPAddressValid() const;

    void ToggleDisabledControlsWhileConnecting(bool enabled);

private slots :
    void OnPortChanged(const QString &text);
    void OnServerHostChanged(const QString &text);

private:
    void UpdateConnectButton() const;
    void UpdatePortField();

    Ui::NewConnectionView*      ui;

    ConnectionSettingsModel*    m_pConnectionSettingsModel;     ///< reference to the connection settings model
    bool                        m_validIPAddress;               ///< Bool to indicate valid IP Address
};

#endif // _NEW_CONNECTION_VIEW_H_
