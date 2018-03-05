//=============================================================================
/// Copyright (c) 2018 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  An API for the developer mode driver to initialize driver protocols.
///  Can be used by applications to take RGP profiles of themselves
//=============================================================================

#include <algorithm>
#include "DevDriverAPI.h"
#include "RGPClientInProcessModel.h"

// C wrapper functions

//-----------------------------------------------------------------------------
/// Initialization function. To be called before initializing the device.
/// \param initOptions A structure of type DevDriverInitOptions containing
///  the initialization parameters
/// \param pOutHandle A returned handle to the DevDriverAPI context.
/// \return DEV_DRIVER_STATUS_SUCCESS if successful, or a DevDriverStatus error
///  code if not. If this function fails, pOutHandle will be unchanged.
//-----------------------------------------------------------------------------
static DevDriverStatus DEV_DRIVER_API_CALL
Init(const DevDriverOptions initOptions[], int32_t optionsCount, DevDriverAPIContext* pOutHandle)
{
    for (int32_t loop = 0; loop < optionsCount; loop++)
    {
        DevDriverOption option = initOptions[loop].shared.m_optionBase.m_option;
        switch (option)
        {
        case DEV_DRIVER_OPTION_ENABLE_RGP_PROTOCOL:
            break;
        }
    }

    RGPClientInProcessModel* pHandle = new(std::nothrow) RGPClientInProcessModel();

    if (pHandle != nullptr)
    {
        if (pHandle->Init() == true)
        {
            *pOutHandle = reinterpret_cast<DevDriverAPIContext>(pHandle);
            return DEV_DRIVER_STATUS_SUCCESS;
        }
        else
        {
            delete pHandle;
            return DEV_DRIVER_STATUS_FAILED;
        }
    }
    else
    {
        return DEV_DRIVER_STATUS_BAD_ALLOC;
    }
}

//-----------------------------------------------------------------------------
/// Cleanup function. To be called at application shutdown.
/// \param context The DevDriverAPI context
/// \return DEV_DRIVER_STATUS_SUCCESS if successful, or a DevDriverStatus error
///  code if not.
//-----------------------------------------------------------------------------
static DevDriverStatus DEV_DRIVER_API_CALL
Finish(DevDriverAPIContext handle)
{
    if (handle != nullptr)
    {
        RGPClientInProcessModel* pObj = reinterpret_cast<RGPClientInProcessModel*>(handle);
        pObj->Finish();
        delete pObj;
        return DEV_DRIVER_STATUS_SUCCESS;
    }
    else
    {
        return DEV_DRIVER_STATUS_NULL_POINTER;
    }
}

//-----------------------------------------------------------------------------
/// Start triggering a profile. The actual profiling is done in a separate thread.
/// The calling function will need to call IsRGPProfileCaptured() to determine
/// if the profile has finished.
/// \param context The DevDriverAPI context
/// \param profileOptions A structure of type RGPProfileOptions containing the
///  profile options
/// \return DEV_DRIVER_STATUS_SUCCESS if a capture was successfully started, or
///  a DevDriverStatus error code if not.
//-----------------------------------------------------------------------------
static DevDriverStatus DEV_DRIVER_API_CALL
TriggerCapture(DevDriverAPIContext handle, const RGPProfileOptions* const profileOptions)
{
    if (handle != nullptr)
    {
        RGPClientInProcessModel* pObj = reinterpret_cast<RGPClientInProcessModel*>(handle);
        pObj->SetTriggerMarkerParams(profileOptions->m_beginFrameTerminatorTag, profileOptions->m_endFrameTerminatorTag,
                                     profileOptions->m_pBeginFrameTerminatorString, profileOptions->m_pEndFrameTerminatorString);
        bool triggered = pObj->TriggerCapture(profileOptions->m_pProfileFilePath);
        if (triggered == true)
        {
            return DEV_DRIVER_STATUS_SUCCESS;
        }
        else
        {
            return DEV_DRIVER_STATUS_CAPTURE_FAILED;
        }
    }
    else
    {
        return DEV_DRIVER_STATUS_NULL_POINTER;
    }
}

//-----------------------------------------------------------------------------
/// Has an RGP profile been taken?
/// \param context The DevDriverAPI context
/// \return DEV_DRIVER_STATUS_SUCCESS if successful, or a DevDriverStatus error
///  code if not.
//-----------------------------------------------------------------------------
static DevDriverStatus DEV_DRIVER_API_CALL
IsProfileCaptured(DevDriverAPIContext handle)
{
    if (handle != nullptr)
    {
        bool captured = reinterpret_cast<RGPClientInProcessModel*>(handle)->IsProfileCaptured();
        if (captured == true)
        {
            return DEV_DRIVER_STATUS_SUCCESS;
        }
        else
        {
            return DEV_DRIVER_STATUS_NOT_CAPTURED;
        }
    }
    else
    {
        return DEV_DRIVER_STATUS_NULL_POINTER;
    }
}

//-----------------------------------------------------------------------------
/// Get the name of the last captured RGP profile. Call this after taking a
/// profile.
/// \param context The DevDriverAPI context
/// \return DEV_DRIVER_STATUS_SUCCESS if successful, or a DevDriverStatus error
///  code if not.
//-----------------------------------------------------------------------------
static DevDriverStatus DEV_DRIVER_API_CALL
GetProfileName(DevDriverAPIContext handle, const char** ppOutProfileName)
{
    if (handle != nullptr)
    {
        const char* pProfileName = reinterpret_cast<RGPClientInProcessModel*>(handle)->GetProfileName();
        *ppOutProfileName = pProfileName;
        return DEV_DRIVER_STATUS_SUCCESS;
    }
    else
    {
        return DEV_DRIVER_STATUS_NULL_POINTER;
    }
}

//-----------------------------------------------------------------------------
/// Get the function table
/// \param pAprTableOut Pointer to an array to receive the list of function
///  pointers. Should be initialized to be at least the size of a DevDriverAPI
///  structure.
/// \return DEV_DRIVER_STATUS_SUCCESS if successful, or a DevDriverStatus error
///  code if not.
//-----------------------------------------------------------------------------
DEV_DRIVER_API_EXPORT DevDriverStatus DEV_DRIVER_API_CALL
DevDriverGetFuncTable(void* pApiTableOut)
{
    if (pApiTableOut == nullptr)
    {
        return DEV_DRIVER_STATUS_NULL_POINTER;
    }

    DevDriverAPI* pApiTable = reinterpret_cast<DevDriverAPI*>(pApiTableOut);

    if (pApiTable->majorVersion != DEV_DRIVER_API_MAJOR_VERSION)
    {
        // only support the exact major version match for now
        return DEV_DRIVER_STATUS_INVALID_MAJOR_VERSION;
    }

    uint32_t suppliedMinorVersion = pApiTable->minorVersion;

    // build the dispatch table containing all supported functions in this library
    DevDriverAPI   apiTable;

    apiTable.majorVersion = DEV_DRIVER_API_MAJOR_VERSION;
    apiTable.minorVersion = std::min<uint32_t>(suppliedMinorVersion, DEV_DRIVER_API_MINOR_VERSION);

    apiTable.DevDriverInit         = Init;
    apiTable.DevDriverFinish       = Finish;

    apiTable.TriggerRgpProfile     = TriggerCapture;
    apiTable.IsRgpProfileCaptured  = IsProfileCaptured;
    apiTable.GetRgpProfileName     = GetProfileName;

    // only copy the functions supported by the incoming requested library
    memcpy(pApiTable, &apiTable, apiTable.minorVersion);
    return DEV_DRIVER_STATUS_SUCCESS;
}
