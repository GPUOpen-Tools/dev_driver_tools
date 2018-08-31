//=============================================================================
/// Copyright (c) 2016-2018 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Shared definitions for the Developer Panel.
//=============================================================================

#ifndef _RDP_DEFINITIONS_H_
#define _RDP_DEFINITIONS_H_

#include <QColor>
#include "../Common/DriverToolsDefinitions.h"

#define RDP_UNUSED(x) (void)(x);

#define ENABLE_PIPELINE_BINARIES (0 & RADEON_DRIVER_TOOLS_INTERNAL)

static const QString gs_PRODUCT_NAME_STRING             = "Radeon Developer Panel" " " DEV_DRIVER_TOOLS_BUILD_SUFFIX;     ///< The human-readable product name.
static const QString gs_PRODUCT_SETTINGS_FILENAME       = "RDPSettings.xml";            ///< The filename for RDP application settings.
static const QString gs_PRODUCT_LOG_FILENAME            = "RDPLogFile.txt";             ///< The filename for RDP logs.
static const QString gs_PROCESS_NAME_BLACKLIST_FILENAME = "ProcessBlacklist.txt";       ///< The filename for the process name blacklist.
static const QString gs_APPLICATION_SETTINGS_DIRECTORY  = "AppSettings";                ///< The directory to place Application Settings files in.
static const QString gs_DEFAULT_TRACE_DIRECTORY         = "profiles";                   ///< The default directory to dump profiles to.
static const QString gs_DEFAULT_PIPELINES_DIRECTORY     = "pipelines";                  ///< The default directory to dump pipelines to.
static const char gs_CAPTURE_TRACE_HOTKEY               = 'C';                          ///< Keypress + [SHIFT] + [CTRL] that triggers a trace capture.

// RDP's default dimensions.
static const int gs_PRODUCT_DEFAULT_WIDTH   = 550;  ///< The default width for the main RDP window.
static const int gs_PRODUCT_DEFAULT_HEIGHT  = 650;  ///< The default height for the main RDP window.

// RDP Color definitions.
static const QColor gs_PRODUCT_COLOR_GUNMETAL = QColor(51, 51, 51);     ///< A potentially lethal shade of gray. Use caution when handling this color.

// Define font point sizes
static const int gs_SUB_TAB_POINT_FONT_SIZE = 12;

// General purpose strings used throughout the Panel interface.
static const QString gs_EMPTY_TEXT          = "";
static const QString gs_DASH_TEXT           = "-";
static const QString gs_UNKNOWN_TEXT        = "Unknown";
static const QString gs_ON_TEXT             = "On";
static const QString gs_OFF_TEXT            = "Off";
static const QString gs_TRUE_TEXT           = "True";
static const QString gs_FALSE_TEXT          = "False";
static const QString gs_LOCAL_HOST          = "localhost";
static const QString gs_LOCAL_HOST_IP       = "127.0.0.1";

static const unsigned int gs_CONNECTION_TIMEOUT_PERIOD = 10000;  ///< The timeout duration to wait while attempting to connect to RDS.

// Connection Status Indicator strings.
static const QString gs_CONNECTION_STATUS_DISCONNECTED_TEXT         = "You are not connected to an instance of the Radeon Developer Service.";
static const QString gs_CONNECTION_STATUS_ATTEMPT_TEXT              = "Attempting to establish a connection to the Radeon Developer Service running at %1";
static const QString gs_CONNECTION_STATUS_CONNECTED_TEXT            = "You are connected to the Radeon Developer Service running at %1";

// Help file locations
static const QString gs_HELP_FILE                                   = "/docs/help/rdp/html/index.html";

// Host Connection interface strings.
static const QString gs_DEFAULT_SERVER_HOST                         = gs_LOCAL_HOST;
static const QString gs_CONNECTION_SETTINGS_CONNECT_BUTTON_TEXT     = "Connect";
static const QString gs_CONNECTION_SETTINGS_DISCONNECT_BUTTON_TEXT  = "Disconnect";
static const QString gs_CONNECTION_LOST_TITLE                       = "Server disconnect";
static const QString gs_CONNECTION_LOST_TEXT                        = "Lost connection to Radeon Developer Service. Please reconnect.";
static const QString gs_CONNECTION_ATTEMPT_FAILED_TITLE             = "Connection Failure";
static const QString gs_CONNECTION_ATTEMPT_FAILED_TEXT              = "Failed to connect. Please start the Radeon Developer Service and try again.";
static const QString gs_DISCONNECT_CONFIRMATION_TITLE               = "Confirm disconnect";
static const QString gs_DISCONNECT_CONFIRMATION_TEXT                = "Are you sure you want to disconnect from %1?";
static const QString gs_PROCESS_TABLE_HEADER_PROCESS_NAME_TEXT      = "Process";
static const QString gs_PROCESS_TABLE_HEADER_PROCESS_ID_TEXT        = "Id";
static const QString gs_PROCESS_TABLE_HEADER_PROCESS_STATUS_TEXT    = "Status";
static const QString gs_PROCESS_TABLE_PROCESS_STATUS_HALTED         = "Halted";
static const QString gs_PROCESS_TABLE_PROCESS_STATUS_RUNNING        = "Running";

// Recent connections table strings.
static const QString gs_RECENT_CONNECTIONS_TABLE_NAME               = "Name";
static const QString gs_RECENT_CONNECTIONS_TABLE_IP_ADDRESS         = "IP Address";
static const QString gs_RECENT_CONNECTIONS_TABLE_PORT               = "Port";
static const QString gs_RECENT_CONNECTIONS_TABLE_AVAILABLE          = "Available";
static const QString gs_RECENT_CONNECTIONS_TABLE_AUTOCONNECT        = "Autoconnect";
static const QString gs_RECENT_CONNECTIONS_CONTEXT_MENU_CONNECT     = "Connect";
static const QString gs_RECENT_CONNECTIONS_CONTEXT_MENU_REMOVE      = "Remove";
static const QString gs_RECENT_CONNECTIONS_REMOVE_BUTTON_NAME       = "Remove";
static const QString gs_RECENT_CONNECTIONS_REMOVE_BUTTON_TOOLTIP    = "Remove the selected recent connection";
static const QString gs_RECENT_CONNECTIONS_CLEAR_BUTTON_NAME        = "Remove all";
static const QString gs_RECENT_CONNECTIONS_CLEAR_BUTTON_TOOLTIP     = "Remove all recent connections";
static const QString gs_RECENT_CONNECTIONS_CLEAR_POPUP_TITLE        = "Remove all recent connections";
static const QString gs_RECENT_CONNECTIONS_CLEAR_POPUP_TEXT         = "Are you sure you want to remove all recent connections? (This will not remove the 127.0.0.1 local connection)";
static const QString gs_RECENT_CONNECTIONS_PROFILE_DOESNT_EXIST_TITLE       = "File not found";
static const QString gs_RECENT_CONNECTIONS_PROFILE_DOESNT_EXIST_RENAME_TEXT = "This profile cannot be renamed because it no longer exists on disk.";
static const QString gs_RECENT_CONNECTIONS_PROFILE_DOESNT_EXIST_DELETE_TEXT = "This profile cannot be deleted because it no longer exists on disk.";

// Target application table strings.
static const QString gs_TARGET_APPLICATION_TABLE_EXECUTABLE_NAME    = "Executable name";
static const QString gs_TARGET_APPLICATION_TABLE_ID                 = "ID";
static const QString gs_TARGET_APPLICATION_TABLE_TITLE              = "Title";
static const QString gs_TARGET_APPLICATION_TABLE_API                = "API";
static const QString gs_TARGET_APPLICATION_TABLE_APPLY_SETTINGS     = "Apply settings";
static const QString gs_TARGET_APPLICATION_TABLE_ENABLE_PROFILING   = "Enable profiling";

// Delete application pop up dialog
static const QString gs_DELETE_APPLICATION_TITLE                    = "Remove confirmation";
static const QString gs_DELETE_APPLICATION                          = "Do you want to remove the selected executable from the list?";
static const QString gs_DELETE_WHILE_PROFILING_TITLE                = "Remove error";
static const QString gs_DELETE_WHILE_PROFILING_MSG                  = "Cannot remove an application from list while it is being profiled.";

// Multiple target application instances
static const QString gs_MULTIPLE_TARGET_APPLICATION_INSTANCES_TITLE = "Multiple target applications detected";
static const QString gs_MULTIPLE_TARGET_APPLICATION_INSTANCES_MSG   = "Target application '%1' is already being profiled (ProcessId %2).\n\nIn order to profile another application, the currently profiled application must be shut down.";

static const QString gs_PROFILER_ALREADY_IN_USE_TITLE               = "Profiler already in use";
static const QString gs_PROFILER_ALREADY_IN_USE_MSG                 = "Target application '%1' is currently being profiled (ProcessId %2).\n\nPlease shut down this application first before attempting to change the profiling target.";

// Profiling selection pop up dialog
static const QString gs_UNCHECK_PROFILE_WHILE_COLLECTING_TRACE_TITLE = "Selection error";
static const QString gs_UNCHECK_PROFILE_WHILE_COLLECTING_TRACE_MSG   = "Cannot select/deselect while collecting a profile.";

// Driver settings default all confirmation dialog strings.
static const QString gs_SETTING_DEFAULT_ALL_CONFIRMATION_DIALOG_TITLE = "Default all confirmation";
static const QString gs_SETTING_DEFAULT_ALL_CONFIRMATION_DIALOG_TEXT = "Do you want to reset all driver settings to default values?";

// Clocks panel descriptions
static const QString gs_CLOCKS_FIXED_FREQUENCY_HEADER   = "Fixed";
static const QString gs_CLOCKS_MODE_NAME_TEXT_NORMAL    = "Normal";
static const QString gs_CLOCKS_MODE_NAME_TEXT_STABLE    = "Stable";
static const QString gs_CLOCKS_MODE_NAME_TEXT_PEAK      = "Peak";
static const QString gs_CLOCKS_MODE_DESCRIPTION_TEXT_NORMAL = "Default clock mode used by consumers, clock speed may vary significantly according to power constraints and application load.";
static const QString gs_CLOCKS_MODE_DESCRIPTION_TEXT_STABLE = "Developer-only clock mode. Sets a stable clock frequency that may be lower than the normal operating frequency. Using this mode is advisable to ensure consistency between different runs of your application.";
static const QString gs_CLOCKS_MODE_DESCRIPTION_TEXT_PEAK = "Developer-only clock mode. Sets peak clock frequency.";

// Target Executable interface strings.
static const QString gs_BROWSE_APPLICATION_FILEPATH_CAPTION_TEXT    = "Select executable to profile";
static const QString gs_DUPLICATE_FILE_TEXT                         = "This file is already in the list of target executables.";
static const QString gs_NO_FILE_SPECIFIED_TEXT                      = "No file name specified. Type in an executable name or browse to an executable on disk.";

// RDP logging constants
static const QString gs_LOG_FILE_NAME_TEXT                          = "./rdplog.txt";

// Table column padding
static const int gs_COLUMN_PADDING                                  = 10;

// RGP Tracing interface strings.
static const DevDriver::uint32 gs_NUM_PREPARATION_FRAMES            = 4;
static const QString gs_BROWSE_TRACE_DIRECTORY_CAPTION_TEXT         = "Choose profile output directory...";
static const QString gs_BROWSE_RGP_INSTALL_PATH                     = "Choose install path to Radeon GPU Profile...";
static const QString gs_RECENT_TRACE_FILEPATH_HEADER                = "Profile";
static const QString gs_RECENT_TRACE_FILE_SIZE                      = "Size";
static const QString gs_RECENT_TRACE_CREATION_TIMESTAMP             = "Created";
static const QString gs_RECENT_TRACE_CONTEXT_MENU_OPEN_TEXT         = "Open profile";
static const QString gs_RECENT_TRACE_CONTEXT_MENU_RENAME_TEXT       = "Rename";
static const QString gs_RECENT_TRACE_CONTEXT_MENU_RENAME_TITLE      = "Rename profile...";
static const QString gs_RECENT_TRACE_CONTEXT_MENU_RENAME_MESSAGE    = "Choose new profile filename...";
static const QString gs_RGP_TRACE_DUPLICATE_RENAME_DIALOG_TITLE     = "Duplicate file name";
static const QString gs_RGP_TRACE_DUPLICATE_RENAME_DIALOG_MESSAGE   = "This file already exists.";
static const QString gs_RECENT_TRACE_CONTEXT_MENU_DELETE_TEXT       = "Delete profile";
static const QString gs_RGP_EXE_DIALOG_TITLE                        = "Radeon GPU Profiler missing";
static const QString gs_RGP_PROFILE_FAILED_TITLE                    = "Failed to write profile";
static const QString gs_RGP_PROFILE_FAILED_NO_SPACE_TEXT            = "Not enough disk space in target directory to write profile.";
static const QString gs_RGP_PROFILE_FAILED_ERROR                    = "Failed to obtain profile with code '%1'.";
static const QString gs_RGP_TRACE_FILE_MISSING_TEXT                 = "Failed to open profile in Radeon GPU Profiler, as the file cannot be found.";
static const QString gs_RGP_TRACE_FILE_MISSING_FILE_BROWSER_FAILED  = "Failed to open profile in file system, as the file cannot be found.";
static const QString gs_RGP_EXE_MISSING_MESSAGE                     = "The Radeon GPU Profiler executable cannot be found at \"%1\". Do you want to revert to the default RGP path?";
static const QString gs_DEFAULT_RGP_EXE_MISSING_MESSAGE             = "The Radeon GPU Profiler executable could not be found at: \"%1\" (default path)";
static const QString gs_RGP_EXE_MISSING_BROWSE_BUTTON_TEXT          = "Browse...";
static const QString gs_RGP_EXE_MISSING_REVERT_BUTTON_TEXT          = "Revert";
static const QString gs_RGP_EXE_MISSING_CANCEL_BUTTON_TEXT          = "Cancel";
static const QString gs_RGP_EXE_NAME_MISSING_DIALOG_TITLE           = "Radeon GPU Profiler name missing";
static const QString gs_RGP_EXE_NAME_MISSING_MESSAGE_1              = "The Radeon GPU Profiler executable name is missing. ";
static const QString gs_RGP_EXE_NAME_MISSING_MESSAGE_2              = "Please type in a valid file name for the Radeon GPU Profiler.";
static const QString gs_RGP_TRACE_PROGRESS_RECEIVED                 = "%1/%2";
static const QString gs_RGP_TRACE_PROGRESS_RATE                     = "Rate: %1/s";
static const QString gs_RGP_DIR_NOT_WRITABLE_TITLE                  = "Target directory not writable";
static const QString gs_RGP_DIR_NOT_WRITABLE_MESSAGE                = "Directory \"%1\" is not writable.";
static const QString gs_PROFILING_NOT_SUPPORTED_TITLE               = "GPU hardware not supported.";
static const QString gs_PROFILING_NOT_SUPPORTED_TEXT                = "Unsupported hardware, refer to the documentation for the list of supported GPUs.";
static const QString gs_DRIVER_WARNING_TITLE                        = "Driver incompatibility warning";
static const QString gs_DRIVER_WARNING_TEXT                         = "To use the latest features of RGP, it is strongly recommended that users update to the latest driver.\n\n" \
                                                                      "Currently installed driver: %1\nRecommended driver: 18.10.16-180516a-328911C-RadeonSoftwareAdrenalin " \
                                                                      "(Radeon Software Version 18.5.1)\n\nhttps://support.amd.com/en-us/download";
static const QString gs_VERSION_CHANGED_TITLE                       = "New Radeon Developer Panel detected";
static const QString gs_VERSION_CHANGED_TEXT                        = "The Radeon Developer Panel has been updated. The version of the Radeon GPU Profiler used to open profiles from here may also have been updated. Please ensure you are using the latest version of RGP by clicking on the 'Profiling' tab and browsing to the newest copy of RGP";

// Description strings that can be used to identify the AMD UMD running within a target process.
static const QString gs_UNKNOWN_API                 = "Unknown";
static const QString gs_AMDXC64_DESCRIPTION_STRING  = "AMD DirectX12 Driver";
static const QString gs_AMDXC64_API_STRING          = "DirectX 12";
static const QString gs_AMDVLK64_DESCRIPTION_STRING = "AMD Vulkan Driver";
static const QString gs_AMDVLK64_API_STRING         = "Vulkan";

// The following declarations are used interchangeably, but contain different contents depending on the OS.
#ifdef WIN32
static const QString gs_RECENT_TRACE_CONTEXT_MENU_SHOW_IN_FILE_BROWSER  = "Show in Explorer";
#else
static const QString gs_RECENT_TRACE_CONTEXT_MENU_SHOW_IN_FILE_BROWSER  = "Show in File Browser";
#endif

// XML settings file reading and writing element strings.
static const QString gs_APPLICATION_SETTINGS_ROOT_ELEMENT       = "ApplicationSettings";
static const QString gs_APPLICATION_SETTINGS_IS_GLOBAL          = "IsGlobal";
static const QString gs_APPLICATION_SETTINGS_TARGET_EXECUTABLE  = "TargetExecutable";
static const QString gs_APPLICATION_SETTINGS_DRIVER_SETTINGS    = "DriverSettings";
static const QString gs_APPLICATION_SETTINGS_SETTING            = "Setting";
static const QString gs_APPLICATION_SETTINGS_CATEGORY           = "Category";
static const QString gs_APPLICATION_SETTINGS_SETTING_NAME       = "Name";
static const QString gs_APPLICATION_SETTINGS_CATEGORY_NAME      = "CategoryName";
static const QString gs_APPLICATION_SETTINGS_CATEGORY_INDEX     = "CategoryIndex";
static const QString gs_APPLICATION_SETTINGS_DESCRIPTION        = "Description";
static const QString gs_APPLICATION_SETTINGS_TYPE               = "Type";
static const QString gs_APPLICATION_SETTINGS_VALUE              = "Value";
static const QString gs_APPLICATION_SETTINGS_DEFAULT_VALUE      = "DefaultValue";

#endif // _RDP_DEFINITIONS_H_
