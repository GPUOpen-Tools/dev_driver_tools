//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A small control responsible for showing the connection status to RDS.
//=============================================================================

#include "ConnectionStatusView.h"
#include "ui_ConnectionStatusView.h"
#include "../RDPDefinitions.h"
#include "../../Common/ToolUtil.h"
#include "../../Common/DriverToolsDefinitions.h"

const static QString s_CONNECTED_STATUS_IMAGE_PATH(":/images/check.png");
const static QString s_DISCONNECTED_STATUS_IMAGE_PATH(":/images/X.png");
const static QString s_CONNECTED_RDS_IMAGE_PATH(":/images/RDS_Icon.png");
const static QString s_DISCONNECTED_RDS_IMAGE_PATH(":/images/RDS_Icon_Gray.png");

//-----------------------------------------------------------------------------
/// Constructor for the ConnectionStatusView control.
/// \param pParent The parent widget for the control.
//-----------------------------------------------------------------------------
ConnectionStatusView::ConnectionStatusView(QWidget* pParent) :
    QWidget(pParent),
    ui(new Ui::ConnectionStatusView)
{
    ui->setupUi(this);

    ui->rdsHostImage->setScaledContents(true);

    // Set the background to white for this control.
    ToolUtil::SetWidgetBackgroundColor(this, Qt::white);

    // Connect buttons to signals
    connect(ui->disconnectButton, &QAbstractButton::pressed, this, &ConnectionStatusView::DisconnectPressed);
    connect(ui->stopAttemptButton, &QAbstractButton::pressed, this, &ConnectionStatusView::StopPressed);

    // Default to disconnected status
    SetConnectionStatus(DISCONNECTED);
}

//-----------------------------------------------------------------------------
/// Destructor for the ConnectionStatusView widget.
//-----------------------------------------------------------------------------
ConnectionStatusView::~ConnectionStatusView()
{
    SAFE_DELETE(ui);
}

//-----------------------------------------------------------------------------
/// Set the connection status of this view.
/// \param status The connection status constant (DISCONNECTED, ATTEMPT, CONNECTED)
//-----------------------------------------------------------------------------
void ConnectionStatusView::SetConnectionStatus(ConnectionStatus status)
{
    m_connectionStatus = status;
    Update();
}

//-----------------------------------------------------------------------------
/// Get the connection status of this view.
/// \return The connection status constant (DISCONNECTED, ATTEMPT, CONNECTED)
//-----------------------------------------------------------------------------
ConnectionStatus ConnectionStatusView::GetConnectionStatus()
{
    return m_connectionStatus;
}

//-----------------------------------------------------------------------------
/// Set the host connection string displayed in the connected and connection
/// attempt states.
/// \param hostConnectionString The hostname string.
//-----------------------------------------------------------------------------
void ConnectionStatusView::SetHostConnectionString(const QString& hostConnectionString)
{
    m_hostConnectionString = hostConnectionString;
    Update();
}

//-----------------------------------------------------------------------------
/// Set the remaining attempt time displayed during the connection attempt state.
/// \param remainingTimeMsecs The attempt time remaining in milliseconds.
//-----------------------------------------------------------------------------
void ConnectionStatusView::SetRemainingTime(int remainingTimeMsecs)
{
    // Ceiling the remaining time number (gives countdown from 10s to 1s rather than 9s to 0s)
    int remainingTimeSecs = ((remainingTimeMsecs - 1) / 1000) + 1;
    m_remainingTimeString = QString::number(remainingTimeSecs) + "s";

    // Set text for attempt timeout label
    ui->attemptTimeoutText->setText(m_remainingTimeString);
}

//-----------------------------------------------------------------------------
/// Set the enabled state of the disconnect button.
/// \param enabled Whether the button should be enabled.
//-----------------------------------------------------------------------------
void ConnectionStatusView::SetDisconnectButtonEnabled(bool enabled)
{
    ui->disconnectButton->setEnabled(enabled);
}

//-----------------------------------------------------------------------------
/// Updates the view to reflect it's current connection status.
//-----------------------------------------------------------------------------
void ConnectionStatusView::Update()
{
    switch (m_connectionStatus)
    {
    case DISCONNECTED:
        // Set disconnected state widget parameters
        ui->connectionStatusImage->setPixmap(s_DISCONNECTED_STATUS_IMAGE_PATH);
        ui->rdsHostImage->setPixmap(s_DISCONNECTED_RDS_IMAGE_PATH);
        ui->connectionStatusText->setText(gs_CONNECTION_STATUS_DISCONNECTED_TEXT);

        // Hide/show widgets for disconnected state
        ui->statusWidget->show();
        ui->progressWidget->hide();
        ui->disconnectButton->hide();
        ui->stopAttemptButton->hide();
        ui->attemptTimeoutText->hide();
        break;

    case ATTEMPT:
        // Set connection attempt state widget parameters
        ui->rdsHostImage->setPixmap(s_DISCONNECTED_RDS_IMAGE_PATH);
        ui->connectionStatusText->setText(gs_CONNECTION_STATUS_ATTEMPT_TEXT.arg(m_hostConnectionString));
        ui->attemptTimeoutText->setText(m_remainingTimeString);

        // Hide/show widgets for connection attempt state
        ui->statusWidget->hide();
        ui->progressWidget->show();
        ui->disconnectButton->hide();
        ui->stopAttemptButton->show();
        ui->attemptTimeoutText->show();
        break;

    case CONNECTED:
        // Set connected state widget parameters
        ui->connectionStatusImage->setPixmap(s_CONNECTED_STATUS_IMAGE_PATH);
        ui->rdsHostImage->setPixmap(s_CONNECTED_RDS_IMAGE_PATH);
        ui->connectionStatusText->setText(gs_CONNECTION_STATUS_CONNECTED_TEXT.arg(m_hostConnectionString));

        // Hide/show widgets for connected state
        ui->statusWidget->show();
        ui->progressWidget->hide();
        ui->disconnectButton->show();
        ui->stopAttemptButton->hide();
        ui->attemptTimeoutText->hide();
        break;
    }
}
