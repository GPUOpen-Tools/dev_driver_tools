//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  The model responsible for updating the driver log messages interface.
//=============================================================================

#ifndef _DRIVER_LOGGING_MODEL_H_
#define _DRIVER_LOGGING_MODEL_H_

#include "DriverProtocolModel.h"

class DriverLogfileModel;
class DriverLogBackgroundWorker;

/// An enumeration representing the controls displayed in the driver logging interface.
enum DriverLoggerControls
{
    DRIVER_LOGGER_CONTROLS_DEBUG_LEVEL,
    DRIVER_LOGGER_CONTROLS_COUNT,
};

/// The model responsible for updating the driver log messages interface.
class DriverLoggingModel : public DriverProtocolModel
{
public:
    DriverLoggingModel(DeveloperPanelModel* pPanelModel, uint32_t modelCount);
    ~DriverLoggingModel();

    bool InitializeLogging();
    void StartLogReaderWorker();
    void StopLogReaderWorker();
    void Update(DriverLoggerControls modelIndex, const QVariant& value);
    DriverLogfileModel* GetLogfileModel() const { return m_pLogfileModel; }

private:
    void ResetLogfileModel();

    QThread* m_pDriverLogWorkerThread;              ///< An background worker thread used to read incoming log messages.
    DriverLogBackgroundWorker* m_pLogReaderWorker;  ///< The worker responsible for reading driver log lines.
    DriverLogfileModel* m_pLogfileModel;            ///< The model where driver log data is inserted.
    bool m_logWorkerInitialized;                   ///< A bool indicating if the log reader has been initialized.
};

#endif // _DRIVER_LOGGING_MODEL_H_
