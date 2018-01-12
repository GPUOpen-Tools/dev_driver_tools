//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for ToolUtil which holds useful utility functions.
//=============================================================================

#ifndef _DRIVER_TOOL_UTILS_H_
#define _DRIVER_TOOL_UTILS_H_

#include <stddef.h>
#include "../DevDriverComponents/inc/gpuopen.h"

class QColor;
class QString;
class QWidget;

class DebugWindow;

namespace ToolUtil
{
    bool CheckFilepathExists(const QString& filepath);
    void DbgMsg(const char* pFormat, ...);
    QString GetDriverToolsXmlFileLocation();
    QString GetFormattedVersionString();
    QString GetResultString(DevDriver::Result result);
    QString GetProtocolTypeString(DevDriver::Protocol protocolType);
    void RegisterDbgWindow(DebugWindow* pDebugWindow);
    void SetWidgetBackgroundColor(QWidget* pWidget, const QColor& color);
}

#endif // _DRIVER_TOOL_UTILS_H_
