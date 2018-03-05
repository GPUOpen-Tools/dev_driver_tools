//=============================================================================
/// Copyright (c) 2018 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  An API for the developer mode driver to initialize driver protocols.
///  Can be used by applications to take RGP profiles of themselves
//=============================================================================
#ifndef DEV_DRIVER_API_H_
#define DEV_DRIVER_API_H_

#include <stdint.h>

#ifndef __cplusplus
#include <stdbool.h>
#endif // #ifdef __cplusplus

#if defined(_WIN32) || defined(__CYGWIN__)
  /// The API calling convention on windows.
  #define DEV_DRIVER_API_CALL __cdecl
  #ifdef BUILD_DLL
    #define DEV_DRIVER_API_EXPORT __declspec(dllexport)
  #elif defined(IMPORT_DLL)
    #define DEV_DRIVER_API_EXPORT __declspec(dllimport)
  #else
    #define DEV_DRIVER_API_EXPORT
  #endif // BUILD_DLL
#else
  /// The API calling convention on linux.
  #define DEV_DRIVER_API_CALL
  #define DEV_DRIVER_API_EXPORT
#endif

#define DEV_DRIVER_API_MAJOR_VERSION 1
#define DEV_DRIVER_API_MINOR_VERSION (sizeof(DevDriverAPI))

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

typedef void* DevDriverAPIContext;

// An enum of status codes returned from the API
typedef enum
{
    DEV_DRIVER_STATUS_SUCCESS               = 0,
    DEV_DRIVER_STATUS_ERROR                 = -1,
    DEV_DRIVER_STATUS_FAILED                = -2,
    DEV_DRIVER_STATUS_NULL_POINTER          = -3,
    DEV_DRIVER_STATUS_BAD_ALLOC             = -4,
    DEV_DRIVER_STATUS_CAPTURE_FAILED        = -5,
    DEV_DRIVER_STATUS_NOT_CAPTURED          = -6,
    DEV_DRIVER_STATUS_INVALID_MAJOR_VERSION = -7,
} DevDriverStatus;

// An enum of options to pass into the DevDriverAPI
typedef enum
{
    DEV_DRIVER_OPTION_ENABLE_RGP_PROTOCOL,
    DEV_DRIVER_OPTION_ENABLE_SETTINGS_PROTOCOL,
    DEV_DRIVER_OPTION_ENABLE_LOGGING_PROTOCOL,
} DevDriverOption;

// structure containing any basic initialization options
typedef struct
{
    DevDriverOption     m_option;
    uint32_t            m_size;
} DevDriverOptionBase;

typedef struct
{
    DevDriverOptionBase m_optionBase;
    int32_t             m_data1;
    int32_t             m_data2;
} DevDriverOptionEx;

typedef struct
{
    union
    {
        // other structures here 'derived' from DevDriverOptionBase
        DevDriverOptionBase	m_optionBase;
        DevDriverOptionEx   m_optionEx;
    } shared;
} DevDriverOptions;

// structure containing any options required for taking an RGP profile
typedef struct
{
    const char* m_pProfileFilePath;             ///< The file (and path) used to save the captured profile to. If the
                                                ///< path is omitted, the file will be saved to the default folder.
                                                ///< If nullptr is specified, then a filename will be generated based
                                                ///< on the process name and a date/time timestamp

    // frame terminator values. Set the ENABLE_FRAME_TERMINATORS flag in the RGPProtocolConfigFlags to enable
    uint64_t    m_beginFrameTerminatorTag;      ///< Queue marker begin tag (vulkan). Should be non-zero if being used
    uint64_t    m_endFrameTerminatorTag;        ///< Queue marker end tag (vulkan). Should be non-zero if being used
    const char* m_pBeginFrameTerminatorString;  ///< Queue marker begin string (D3D12). Should be non-null if being used
    const char* m_pEndFrameTerminatorString;    ///< Queue marker end string (D3D12). Should be non-null if being used
} RGPProfileOptions;

// function typedefs

/// Initialization function. To be called before initializing the device.
/// \param initOptions A structure of type DevDriverInitOptions containing
///  the initialization parameters
/// \param pOutHandle A returned handle to the DevDriverAPI context.
/// \return DEV_DRIVER_STATUS_SUCCESS if successful, or a DevDriverStatus error
///  code if not. If this function fails, pOutHandle will be unchanged.
typedef DevDriverStatus(DEV_DRIVER_API_CALL*
    DevDriverFnInit)(const DevDriverOptions initOptions[], int32_t optionsCount, DevDriverAPIContext* pOutHandle);

/// Cleanup function. To be called at application shutdown.
/// \param context The DevDriverAPI context
/// \return DEV_DRIVER_STATUS_SUCCESS if successful, or a DevDriverStatus error
///  code if not.
typedef DevDriverStatus(DEV_DRIVER_API_CALL*
    DevDriverFnFinish)(DevDriverAPIContext context);

/// Start triggering a profile. The actual profiling is done in a separate thread.
/// The calling function will need to call IsRGPProfileCaptured() to determine
/// if the profile has finished.
/// \param context The DevDriverAPI context
/// \param profileOptions A structure of type RGPProfileOptions containing the
///  profile options
/// \return DEV_DRIVER_STATUS_SUCCESS if a capture was successfully started, or
///  a DevDriverStatus error code if not.
typedef DevDriverStatus(DEV_DRIVER_API_CALL*
    DevDriverFnTriggerRGPProfile)(DevDriverAPIContext context, const RGPProfileOptions* const profileOptions);

/// Has an RGP profile been taken?
/// \param context The DevDriverAPI context
/// \return DEV_DRIVER_STATUS_SUCCESS if successful, or a DevDriverStatus error
///  code if not.
typedef DevDriverStatus(DEV_DRIVER_API_CALL*
    DevDriverFnIsRGPProfileCaptured)(DevDriverAPIContext context);

/// Get the name of the last captured RGP profile. Call this after taking a
/// profile.
/// \param context The DevDriverAPI context
/// \return DEV_DRIVER_STATUS_SUCCESS if successful, or a DevDriverStatus error
///  code if not.
typedef DevDriverStatus(DEV_DRIVER_API_CALL*
    DevDriverFnGetRGPProfileName)(DevDriverAPIContext context, const char** ppOutProfileName);

// structure containing the list of functions supported by this version of the API.
// Also contains major and minor version numbers
typedef struct
{
    uint32_t                        majorVersion;
    uint32_t                        minorVersion;

    DevDriverFnInit                 DevDriverInit;
    DevDriverFnFinish               DevDriverFinish;

    DevDriverFnTriggerRGPProfile    TriggerRgpProfile;
    DevDriverFnIsRGPProfileCaptured IsRgpProfileCaptured;
    DevDriverFnGetRGPProfileName    GetRgpProfileName;
} DevDriverAPI;

/// Get the function table
DEV_DRIVER_API_EXPORT DevDriverStatus DEV_DRIVER_API_CALL DevDriverGetFuncTable(void* pApiTableOut);

#ifdef __cplusplus
}
#endif // #ifdef __cplusplus

#endif // DEV_DRIVER_API_H_
