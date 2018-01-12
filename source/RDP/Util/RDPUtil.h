//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for RGPUtil which holds useful utility functions.
//=============================================================================

#ifndef _RDP_UTIL_H_
#define _RDP_UTIL_H_

#include "../../Common/ToolUtil.h"
#include "../../DevDriverComponents/inc/protocols/driverControlProtocol.h"
#include "../Views/NotificationWidget.h"
#include <QtGlobal>

class QFont;
class QTreeView;

class MainWindow;

namespace RDPUtil
{
    void DbgMsg(const char* pFormat, ...);
    QString GetClockModeAsString(DevDriver::DriverControlProtocol::DeviceClockMode clockMode);
    QString GetDefaultRGPPath();
    void OpenProfilingTab();
	void SetDisconnectButtonEnabled(bool enabled);
    void RegisterLogWindow(MainWindow* pOutputWindow);
    NotificationWidget::Button ShowNotification(const QString& title, const QString& message, uint buttons, uint defaultButton = 0);
    NotificationWidget::Button ShowNotification(const QString& title, const QString& message, uint buttons, bool& showDoNotAsk, uint defaultButton = 0);
    void UnregisterLogWindow();
}

#endif // _RDP_UTIL_H_
