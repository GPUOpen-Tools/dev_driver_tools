//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  An interface used to view and filter driver log messages.
//=============================================================================

#include <QScrollBar>
#include "DriverLoggerView.h"
#include "ui_DriverLoggerView.h"
#include "../Models/DeveloperPanelModel.h"
#include "../Models/DriverLoggingModel.h"
#include "../Models/DriverLogfileModel.h"
#include "../RDPDefinitions.h"
#include "../../Common/ToolUtil.h"

//-----------------------------------------------------------------------------
/// Constructor for the DriverLoggerView interface.
/// \param pPanelModel The main Panel model used to maintain a connection to RDS.
/// \param pParent The parent widget for the Driver Logger View.
//-----------------------------------------------------------------------------
DriverLoggerView::DriverLoggerView(DeveloperPanelModel* pPanelModel, QWidget* pParent) :
    QWidget(pParent),
    ui(new Ui::DriverLoggerView)
{
    ui->setupUi(this);

    // Set white background for this pane
    ToolUtil::SetWidgetBackgroundColor(this, Qt::white);

    m_pDriverLoggingModel = new DriverLoggingModel(pPanelModel, DRIVER_LOGGER_CONTROLS_COUNT);
    if (m_pDriverLoggingModel != nullptr)
    {
        // @TODO: Need to consider how to enable/disable log collection.
        /*
        pPanelModel->RegisterProtocolModel(MAIN_PANEL_MODEL_DRIVER_LOGGING, m_pDriverLoggingModel);
        m_pDriverLoggingModel->InitializeModel(ui->debugLevelCombo, DRIVER_LOGGER_CONTROLS_DEBUG_LEVEL, "checked");
        connect(ui->enableLoggingButton, SIGNAL(clicked(bool)), this, SLOT(OnEnableLoggingClicked(bool)));
        connect(ui->disableLoggingButton, SIGNAL(clicked(bool)), this, SLOT(OnDisableLoggingClicked(bool)));

        */

        connect(ui->logSourceComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnLogSourceIndexChanged(int)));
    }
}

//-----------------------------------------------------------------------------
/// Destructor for the DriverLoggerView interface.
//-----------------------------------------------------------------------------
DriverLoggerView::~DriverLoggerView()
{
    SAFE_DELETE(m_pDriverLoggingModel);

    delete ui;
}

//-----------------------------------------------------------------------------
/// Update the connected Client Id for any models that are connected to clients.
/// \param clientId The client Id for the client.
//-----------------------------------------------------------------------------
void DriverLoggerView::OnClientIdUpdated(DevDriver::ClientId clientId)
{
    m_pDriverLoggingModel->SetConnectedClientId(clientId);
}

//-----------------------------------------------------------------------------
/// Handler invoked when the "Save log file" button is clicked.
/// \param checked A bool indicating if the button is checked.
//-----------------------------------------------------------------------------
void DriverLoggerView::OnSaveLogFileClicked(bool checked)
{
    Q_UNUSED(checked);

    // @TODO: Open a "Save As..." dialog, and validate the path.
    // Instruct the DriverLoggingModel to write out the current log to a file.
}

//-----------------------------------------------------------------------------
/// Enable collecting driver log messages from the connected RDS instance.
/// \param checked True if the button has been checked, and false if it hasn't.
//-----------------------------------------------------------------------------
void DriverLoggerView::OnEnableLoggingClicked(bool checked)
{
    Q_UNUSED(checked);

    bool logInitialized = m_pDriverLoggingModel->InitializeLogging();
    if (logInitialized)
    {
        DriverLogfileModel* pDriverLogfileModel = m_pDriverLoggingModel->GetLogfileModel();
        ui->logTextbox->setModel(pDriverLogfileModel);

        m_pDriverLoggingModel->StartLogReaderWorker();
    }
}

//-----------------------------------------------------------------------------
/// Disable collecting driver log messages from the connected RDS instance.
/// \param checked True if the button has been checked, and false if it hasn't.
//-----------------------------------------------------------------------------
void DriverLoggerView::OnDisableLoggingClicked(bool checked)
{
    Q_UNUSED(checked);

    m_pDriverLoggingModel->StopLogReaderWorker();
}
