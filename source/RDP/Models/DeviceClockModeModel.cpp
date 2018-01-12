//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief The model containing clock mode frequencies retrieved from a device.
//=============================================================================

#include "DeviceClockModeModel.h"
#include "../RDPDefinitions.h"
#include <QVariant>

//-----------------------------------------------------------------------------
/// Constructor for DeviceClockModeModel.
/// \param properties The properties to use when creating an individual clock mode setting.
/// \param modelCount the number of models required.
//-----------------------------------------------------------------------------
DeviceClockModeModel::DeviceClockModeModel(const ModeProperties properties, uint32_t modelCount)
    : ModelViewMapper(modelCount)
    , m_clockFrequencies({})
{
    m_modeProperties.m_modeName = properties.m_modeName;
    m_modeProperties.m_description = properties.m_description;
    m_modeProperties.m_clockMode = properties.m_clockMode;
}

//-----------------------------------------------------------------------------
/// Destructor for DeviceClockModeModel.
//-----------------------------------------------------------------------------
DeviceClockModeModel::~DeviceClockModeModel()
{
}

//-----------------------------------------------------------------------------
/// Initialize default values within the interface.
//-----------------------------------------------------------------------------
void DeviceClockModeModel::InitializeDefaults()
{
    Update(CLOCK_MODE_CONTROLS_MODE_NAME, m_modeProperties.m_modeName);
    Update(CLOCK_MODE_CONTROLS_MODE_DESCRIPTION, m_modeProperties.m_description);

    Update(CLOCK_MODE_CONTROLS_BASE_SHADER_CLOCK, gs_DASH_TEXT);
    Update(CLOCK_MODE_CONTROLS_BASE_MEMORY_CLOCK, gs_DASH_TEXT);
    Update(CLOCK_MODE_CONTROLS_MAX_SHADER_CLOCK, gs_DASH_TEXT);
    Update(CLOCK_MODE_CONTROLS_MAX_MEMORY_CLOCK, gs_DASH_TEXT);
}

//-----------------------------------------------------------------------------
/// Update specific model data based on the incoming model index and QVariant value.
/// \param modelIndex The index of the model to update.
/// \param value The value of the model data.
//-----------------------------------------------------------------------------
void DeviceClockModeModel::Update(ClockModeControls modelIndex, const QVariant& value)
{
    SetModelData(modelIndex, value);

    float floatValue = value.toString().toFloat();

    switch (modelIndex)
    {
    case CLOCK_MODE_CONTROLS_MODE_NAME:
        m_modeProperties.m_modeName = value.toString();
        break;
    case CLOCK_MODE_CONTROLS_MODE_DESCRIPTION:
        m_modeProperties.m_description = value.toString();
        break;
    case CLOCK_MODE_CONTROLS_BASE_SHADER_CLOCK:
        m_clockFrequencies.current.shaderClock = floatValue;
        break;
    case CLOCK_MODE_CONTROLS_BASE_MEMORY_CLOCK:
        m_clockFrequencies.current.memoryClock = floatValue;
        break;
    case CLOCK_MODE_CONTROLS_MAX_SHADER_CLOCK:
        m_clockFrequencies.max.shaderClock = floatValue;
        break;
    case CLOCK_MODE_CONTROLS_MAX_MEMORY_CLOCK:
        m_clockFrequencies.max.memoryClock = floatValue;
        break;
    default:
        Q_ASSERT(false);
        break;
    }
}
