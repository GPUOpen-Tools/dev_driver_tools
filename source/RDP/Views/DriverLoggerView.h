//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  An interface used to view and filter driver log messages.
//=============================================================================

#ifndef _DRIVER_LOGGER_VIEW_H_
#define _DRIVER_LOGGER_VIEW_H_

#include <QWidget>
#include "../../DevDriverComponents/inc/gpuopen.h"

class DriverLoggingModel;
class DeveloperPanelModel;

namespace Ui {
class DriverLoggerView;
}

/// An interface used to view and filter driver log messages.
class DriverLoggerView : public QWidget
{
    Q_OBJECT
public:
    explicit DriverLoggerView(DeveloperPanelModel* pPanelModel, QWidget* pParent = nullptr);
   virtual ~DriverLoggerView();

protected:
    virtual void OnClientIdUpdated(DevDriver::ClientId clientId);

public slots:
    void OnEnableLoggingClicked(bool checked);
    void OnDisableLoggingClicked(bool checked);
    void OnSaveLogFileClicked(bool checked);

private:
    Ui::DriverLoggerView *ui;
    DriverLoggingModel* m_pDriverLoggingModel;  ///< The model responsible for processing incoming log messages.
};

#endif // _DRIVER_LOGGER_VIEW_H_
