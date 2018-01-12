//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  The model used to hold together all application-specific setting data.
//=============================================================================

#ifndef _APPLICATION_SETTINGS_MODEL_H_
#define _APPLICATION_SETTINGS_MODEL_H_

#include "../../Common/ModelViewMapper.h"
#include "../../DevDriverComponents/inc/gpuopen.h"

struct RDPApplicationSettingsFile;
class ApplicationSettingsFile;
class DeveloperPanelModel;
class DriverSettingsModel;
class RGPTraceModel;

/// An enum used to represent the controls in the driver settings interface.
enum ApplicationSettingsControls
{
    APPLICATION_SETTINGS_CONTROLS_TARGET_EXECUTABLE_TEXTBOX,    ///< The target executable textbox.
    APPLICATION_SETTINGS_CONTROLS_COUNT                         ///< The number of controls in the interface.
};

/// The model used to hold together all application-specific settings.
class ApplicationSettingsModel : public ModelViewMapper
{
public:
    ApplicationSettingsModel(DeveloperPanelModel* pParentModel, uint32_t modelCount);
    virtual ~ApplicationSettingsModel();

    void InitializeDefaults();
    void InitializeFromFile(RDPApplicationSettingsFile* pAppSettingsFile);
    void Update(ApplicationSettingsControls modelIndex, const QVariant& value);
    const QString GetTargetExecutableMatchString() const { return m_targetExecutableMatchString; }
    const QString GetFullTargetExecutableProcessName() const { return m_fullTargetExecutableName; }
    void SetTargetExecutableName(const QString& targetExecutableName) { m_targetExecutableMatchString = targetExecutableName; }
    void SetConnectedClientId(DevDriver::ClientId clientId);
    void SetConnectedProcessName(const QString& processName);
    DevDriver::ClientId GetConnectedClientId() const { return m_clientId; }

    ApplicationSettingsFile* GetSettingsFile() const { return m_pApplicationSettingsFile; }
    DriverSettingsModel* GetDriverSettingsModel() const { return m_pDriverSettingsModel; }
    RGPTraceModel* GetRgpTraceModel() const { return m_pRgpTraceModel; }

private:
    QString m_targetExecutableMatchString;                  ///< The target executable regex match string provided by the user.
    QString m_fullTargetExecutableName;                     ///< The full target executable name string.
    DeveloperPanelModel* m_pParentModel;                    ///< The parent model managing this settings model.
    DriverSettingsModel* m_pDriverSettingsModel;            ///< The driver settings model.
    RGPTraceModel* m_pRgpTraceModel;                        ///< The RGP trace collection model.
    ApplicationSettingsFile* m_pApplicationSettingsFile;    ///< The application settings file to read and write to disk.
    DevDriver::ClientId m_clientId;                         ///< The ClientId of the connected Developer Mode process.
    bool m_isGlobal;                                        ///< A flag that indicates if this is the "Global" settings tab.
};

#endif // _APPLICATION_SETTINGS_MODEL_H_
