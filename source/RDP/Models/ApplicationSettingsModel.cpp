//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  The model used to hold together all application-specific setting data.
//=============================================================================

#include <QVariant>
#include "ApplicationSettingsModel.h"
#include "DriverSettingsModel.h"
#include "RGPTraceModel.h"
#include "../Settings/RDPSettings.h"

//-----------------------------------------------------------------------------
/// Constructor for ApplicationSettingsModel.
/// \param modelCount The number of models required.
//-----------------------------------------------------------------------------
ApplicationSettingsModel::ApplicationSettingsModel(DeveloperPanelModel* pParentModel, uint32_t modelCount)
    : ModelViewMapper(modelCount)
    , m_pParentModel(pParentModel)
    , m_pDriverSettingsModel(nullptr)
    , m_pRgpTraceModel(nullptr)
    , m_pApplicationSettingsFile(nullptr)
    , m_clientId(0)
    , m_isGlobal(false)
{
    m_targetExecutableMatchString.clear();
    m_fullTargetExecutableName.clear();

    m_pDriverSettingsModel = new DriverSettingsModel(m_pParentModel, this, DRIVER_SETTINGS_CONTROLS_COUNT);
    m_pRgpTraceModel = new RGPTraceModel(m_pParentModel, this, RGP_TRACE_CONTROLS_COUNT);
}

//-----------------------------------------------------------------------------
/// Destructor for ApplicationSettingsModel.
//-----------------------------------------------------------------------------
ApplicationSettingsModel::~ApplicationSettingsModel()
{
    // The other pointers in this class are not owned by it, the higher level logic manages them

    SAFE_DELETE(m_pApplicationSettingsFile);
}

//-----------------------------------------------------------------------------
/// Initialize default values within the interface.
//-----------------------------------------------------------------------------
void ApplicationSettingsModel::InitializeDefaults()
{
    const QString executableName = GetTargetExecutableMatchString();
    Update(APPLICATION_SETTINGS_CONTROLS_TARGET_EXECUTABLE_TEXTBOX, executableName);
}

//-----------------------------------------------------------------------------
/// Initialize the Application Settings Model from a file structure, loaded from disk.
/// \param pAppSettingsFile The application settings file loaded from disk.
//-----------------------------------------------------------------------------
void ApplicationSettingsModel::InitializeFromFile(RDPApplicationSettingsFile* pAppSettingsFile)
{
    Q_ASSERT(pAppSettingsFile != nullptr);

    // Read the application settings file.
    m_pApplicationSettingsFile = RDPSettings::Get().ReadApplicationSettingsFile(pAppSettingsFile);

    if (m_pApplicationSettingsFile != nullptr)
    {
        // Copy everything from the incoming file and store it in the model.
        m_isGlobal = m_pApplicationSettingsFile->IsGlobal();

        m_targetExecutableMatchString = m_pApplicationSettingsFile->GetTargetApplicationName();
    }
    else
    {
        m_pApplicationSettingsFile = new ApplicationSettingsFile();
    }
}

//-----------------------------------------------------------------------------
/// Update the internal model members.
/// \param modelIndex The ID of the UI element that's being updated.
/// \param value The new value of the UI element.
//-----------------------------------------------------------------------------
void ApplicationSettingsModel::Update(ApplicationSettingsControls modelIndex, const QVariant& value)
{
    SetModelData(modelIndex, value);

    switch (modelIndex)
    {
    case APPLICATION_SETTINGS_CONTROLS_TARGET_EXECUTABLE_TEXTBOX:
        {
            m_targetExecutableMatchString = value.toString();

            if (m_pApplicationSettingsFile != nullptr)
            {
                m_pApplicationSettingsFile->SetTargetExecutableName(m_targetExecutableMatchString);
            }
        }
        break;
    default:
        Q_ASSERT(false);
        break;
    }

    // Save each change in the model to the backing settings file on disk.
    RDPSettings::Get().WriteApplicationSettingsFile(m_pApplicationSettingsFile);
}

//-----------------------------------------------------------------------------
/// Update the connected client Id for this model and sub models.
/// \param clientId The clientId of the connected client.
//-----------------------------------------------------------------------------
void ApplicationSettingsModel::SetConnectedClientId(DevDriver::ClientId clientId)
{
    m_clientId = clientId;

    if (m_pDriverSettingsModel != nullptr)
    {
        m_pDriverSettingsModel->SetConnectedClientId(clientId);
    }

    if (m_pRgpTraceModel != nullptr)
    {
        m_pRgpTraceModel->SetConnectedClientId(clientId);
    }
}

//-----------------------------------------------------------------------------
/// Set the given name for the connected process.
/// \param processName The executable filename string.
//-----------------------------------------------------------------------------
void ApplicationSettingsModel::SetConnectedProcessName(const QString& processName)
{
    m_fullTargetExecutableName = processName;
}
