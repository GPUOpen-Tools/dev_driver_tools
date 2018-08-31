//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of the New Connection view interface.
//=============================================================================

#include "NewConnectionView.h"
#include "ui_NewConnectionView.h"

#include "ConnectionSettingsView.h"
#include "../Models/ConnectionSettingsModel.h"
#include "../Settings/RDPSettings.h"
#include "../../Common/RestoreCursorPosition.h"

static const QString s_IPAddressValidator = "^([01]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])\\.([01]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])\\.([01]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])\\.([01]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])$";

//-----------------------------------------------------------------------------
/// Explicit constructor
//-----------------------------------------------------------------------------
NewConnectionView::NewConnectionView(ConnectionSettingsModel* pConnectionSettingsModel, QWidget* pParent) :
    QWidget(pParent),
    ui(new Ui::NewConnectionView),
    m_validIPAddress(false)
{
    ui->setupUi(this);

    m_pConnectionSettingsModel = pConnectionSettingsModel;

    m_pConnectionSettingsModel->InitializeModel(ui->serverHostTextbox,  CONNECTION_SETTINGS_SERVER_HOST_STRING, "text");
    m_pConnectionSettingsModel->InitializeModel(ui->portTextbox,        CONNECTION_SETTINGS_SERVER_PORT_STRING, "text");

    // Apply a port range validator and signal when the text changes.
    ui->portTextbox->setValidator(new QIntValidator(0, gs_MAX_LISTEN_PORT, this));

    // Setup connections
    connect(ui->serverHostTextbox,  &QLineEdit::textChanged,    this, &NewConnectionView::OnServerHostChanged);
    connect(ui->portTextbox,        &QLineEdit::textChanged,    this, &NewConnectionView::OnPortChanged);
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
NewConnectionView::~NewConnectionView()
{
    delete ui;
}

//-----------------------------------------------------------------------------
/// Get the 'connect' button from this view
/// \return Pointer to the connect button
//-----------------------------------------------------------------------------
QObject* NewConnectionView::GetConnectButton() const
{
    return ui->connectButton;
}

//-----------------------------------------------------------------------------
/// Get the 'ip address' line edit from this view
/// \return Pointer to the ip address line edit
//-----------------------------------------------------------------------------
QObject* NewConnectionView::GetIPAddressLineEdit() const
{
    return ui->serverHostTextbox;
}

//-----------------------------------------------------------------------------
/// Get the 'port number' line edit from this view
/// \return Pointer to the port number line edit
//-----------------------------------------------------------------------------
QObject* NewConnectionView::GetPortNumberLineEdit() const
{
    return ui->portTextbox;
}

//-----------------------------------------------------------------------------
/// Toggle the user's ability to click controls responsible for establishing a connection.
/// \param enabled A flag used to determine whether to enable or disable connection-attempt controls.
//-----------------------------------------------------------------------------
void NewConnectionView::ToggleDisabledControlsWhileConnecting(bool enabled)
{
    ui->serverHostTextbox->setEnabled(enabled);
    ui->portTextbox->setEnabled(enabled);
}

//-----------------------------------------------------------------------------
/// Handle the port text changing.
/// \param text The new text in the port textbox.
//-----------------------------------------------------------------------------
void NewConnectionView::OnPortChanged(const QString &text)
{
    RestoreCursorPosition cacheCursorPosition(ui->portTextbox);
    m_pConnectionSettingsModel->Update(CONNECTION_SETTINGS_SERVER_PORT_STRING, text);

    // Enable/Disable the connect button
    UpdateConnectButton();
}

//-----------------------------------------------------------------------------
/// Handle the server host text changing.
/// \param text The new text in the server host textbox.
//-----------------------------------------------------------------------------
void NewConnectionView::OnServerHostChanged(const QString &text)
{
    RestoreCursorPosition cacheCursorPosition(ui->serverHostTextbox);

#ifdef _DEBUG
    // Make sure we have a valid IP address before processing it
    QRegExp IPRegex(s_IPAddressValidator);
    if ((text.compare(gs_LOCAL_HOST_IP) == 0) || (text.compare(gs_LOCAL_HOST) == 0) || IPRegex.exactMatch(text))
    {
        // Enable the connect button
        ui->connectButton->setEnabled(true);

        // Update the IP address validity boolean
        m_validIPAddress = true;
    }
    else
    {
        // Disable the connect button
        ui->connectButton->setEnabled(false);

        // Update the IP address validity boolean
        m_validIPAddress = false;

        return;
    }
#else
    m_validIPAddress = true;
#endif

    // If using "127.0.0.1", set the data to "localhost"
    if (text.compare(gs_LOCAL_HOST_IP) == 0)
    {
        m_pConnectionSettingsModel->Update(CONNECTION_SETTINGS_SERVER_HOST_STRING, gs_LOCAL_HOST);
        ui->serverHostTextbox->setText(gs_LOCAL_HOST);
    }
    else
    {
        m_pConnectionSettingsModel->Update(CONNECTION_SETTINGS_SERVER_HOST_STRING, text);
    }

    // Enable/Disable the connect button
    UpdateConnectButton();

#ifdef Q_OS_WIN
    // Show or hide the port field
    UpdatePortField();
#endif
}

//-----------------------------------------------------------------------------
/// Hide the port field when running on Windows OS if connecting to local host.
/// Otherwise, make the port field visible.
//-----------------------------------------------------------------------------
void NewConnectionView::UpdatePortField()
{
#ifdef Q_OS_WIN
    bool portVisible = true;
    if (ui->serverHostTextbox->text() == gs_LOCAL_HOST)
    {
        portVisible = false;
    }
    else if (ui->portTextbox->text() == "0")
    {
        m_pConnectionSettingsModel->Update(CONNECTION_SETTINGS_SERVER_PORT_STRING, QString::number(gs_DEFAULT_CONNECTION_PORT));
    }
    ui->portLabel->setVisible(portVisible);
    ui->portTextbox->setVisible(portVisible);
#endif
}

//-----------------------------------------------------------------------------
/// Enable the connect button only if the IP address and the port number
/// edit boxes are not empty.
//-----------------------------------------------------------------------------
void NewConnectionView::UpdateConnectButton() const
{
    // Get the connection information from the model
    const RDSConnectionInfo& connectionInfo = m_pConnectionSettingsModel->GetConnectionCreateInfo();

    // Update the connect button depending on values
    const QString& hostName = QString(connectionInfo.rdsInfo.connectionInfo.hostname);
    uint16_t port = connectionInfo.rdsInfo.connectionInfo.port;

#ifdef Q_OS_WIN
    if ((hostName.isEmpty()) || ((port == 0) && (hostName != gs_LOCAL_HOST_IP)))
#else
    if ((hostName.isEmpty()) || (port == 0))
#endif
    {
        ui->connectButton->setEnabled(false);
    }
    else
    {
        ui->connectButton->setEnabled(true);
    }
}

//-----------------------------------------------------------------------------
/// Return the boolean to indicate the validity of the IP address
//-----------------------------------------------------------------------------
bool NewConnectionView::IsIPAddressValid() const
{
    return m_validIPAddress;
}
