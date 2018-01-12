//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief The model containing clock mode frequencies retrieved from a device.
//=============================================================================

#ifndef _DEVICE_CLOCK_MODE_MODEL_H_
#define _DEVICE_CLOCK_MODE_MODEL_H_

#include "../../Common/ModelViewMapper.h"
#include "../../DevDriverComponents/inc/protocols/driverControlProtocol.h"

/// A helper structure holding the device's shader and memory frequency.
struct ShaderAndMemoryClocks
{
    float shaderClock;
    float memoryClock;
};

/// A helper structure holding the current and maximum device frequencies.
struct DeviceClocks
{
    ShaderAndMemoryClocks current;
    ShaderAndMemoryClocks max;
};

/// An enumeration containing a member for each interface element bound to this model.
enum ClockModeControls
{
    CLOCK_MODE_CONTROLS_MODE_NAME,
    CLOCK_MODE_CONTROLS_MODE_DESCRIPTION,
    CLOCK_MODE_CONTROLS_BASE_SHADER_CLOCK,
    CLOCK_MODE_CONTROLS_BASE_MEMORY_CLOCK,
    CLOCK_MODE_CONTROLS_MAX_SHADER_CLOCK,
    CLOCK_MODE_CONTROLS_MAX_MEMORY_CLOCK,
    CLOCK_MODE_CONTROLS_COUNT,
};

/// A structure describing custom clock widget properties.
struct ModeProperties
{
    QString m_modeName;
    QString m_description;
    QString m_imagePrefix;
    DevDriver::DriverControlProtocol::DeviceClockMode m_clockMode;
};

/// The model that holds the data for each clock mode widget.
class DeviceClockModeModel : public ModelViewMapper
{
    Q_OBJECT
public:
    explicit DeviceClockModeModel(const ModeProperties properties, uint32_t modelCount);
    virtual ~DeviceClockModeModel();

    void InitializeDefaults();
    void Update(ClockModeControls modelIndex, const QVariant& value);
    const ModeProperties& GetModeProperties() const { return m_modeProperties; }

private:
    ModeProperties m_modeProperties;    ///< The properties for the individual clock mode.
    DeviceClocks m_clockFrequencies;    ///< The clock frequencies retrieved from the GPU.
};

#endif // _DEVICE_CLOCK_MODE_MODEL_H_
