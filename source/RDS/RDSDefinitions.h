//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Shared definitions for the Radeon Developer Service.
//=============================================================================

#ifndef _RDS_DEFINITIONS_H_
#define _RDS_DEFINITIONS_H_

#include "../Common/DriverToolsDefinitions.h"

#ifndef HEADLESS
#include <QString>
static const QString gs_PRODUCT_NAME_STRING             = "Radeon Developer Service" " " DEV_DRIVER_TOOLS_BUILD_SUFFIX;           ///< The human-readable product name.
static const QString gs_RDS_PRODUCT_SETTINGS_FILENAME   = "RDSSettings.xml";    ///< The filename for RDS application settings.

// General purpose strings
static const QString gs_STRINGS_ENABLE                  = "enable";             ///< Enable string used to build confirmation text.
static const QString gs_STRINGS_DISABLE                 = "disable";            ///< Disable string used to build confirmation text.

// Toggling UWP mode confirmation dialogs.
static const QString gs_TOGGLE_UWP_CONFIRMATION_TITLE   = "Toggle UWP mode";    ///< UWP confirmation messagebox title.
static const QString gs_TOGGLE_UWP_CONFIRMATION_TEXT    = "Are you sure you want to %1 UWP mode?\nRDS will restart, and the Radeon Developer Panel must be manually reconnected to RDS after toggling.";         ///< Text to display to the user when enabling UWP mode.

// RDS tray icon context menu strings.
static const QString gs_ENABLE_UWP_CONTEXT_MENU         = "Enable UWP";         ///< Toggle support for UWP in system tray menu.
static const QString gs_CONFIGURE_CONTEXT_MENU          = "Configure";          ///< Configure action in the system tray menu.
static const QString gs_QUIT_CONTEXT_MENU               = "Quit";               ///< Toggle displayed in system tray menu.
#else
static const char* gs_PRODUCT_NAME_STRING = "Radeon Developer Service" " " DEV_DRIVER_TOOLS_BUILD_SUFFIX;           ///< The human-readable product name.
#endif // HEADLESS
#endif // _RDS_DEFINITIONS_H_
