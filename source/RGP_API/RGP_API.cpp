//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  An interface class with the service class to initialize dev driver
///         protocols to capture rgp trace from within the profiled
///         application.
//=============================================================================

#include "RGP_API.h"
#include "RGPClientInProcessModel.h"

//-----------------------------------------------------------------------------
/// Constructor
//-----------------------------------------------------------------------------
RGP_API::RGP_API()
{
    m_pImpl = new RGPClientInProcessModel();
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
RGP_API::~RGP_API()
{
    delete m_pImpl;
}

//-----------------------------------------------------------------------------
/// Initialization function. To be called before initializing the device.
/// \return true if successful, false otherwise.
//-----------------------------------------------------------------------------
bool RGP_API::Init()
{
    return m_pImpl->Init();
}

//-----------------------------------------------------------------------------
/// Cleanup function. To be called at application shutdown.
//-----------------------------------------------------------------------------
void RGP_API::Finish()
{
    m_pImpl->Finish();
}

//-----------------------------------------------------------------------------
/// Start triggering a capture. The actual capture is done in a separate thread. The
/// calling function will need to call IsProfileCaptured() to determine if the capture
/// has finished.
/// \param captureFileName The file (and path) used to save the captured profile to. If the
///  path is omitted, the file will be saved to the default folder. If nullptr is
///  specified, then a filename will be generated based on the process name and a date/time
///  timestamp
/// \return true if capture started, false if not (either the driver hasn't completed its
///  initialization or a capture is currently in progress).
//-----------------------------------------------------------------------------
bool RGP_API::TriggerCapture(const char* captureFileName)
{
    return m_pImpl->TriggerCapture(captureFileName);
}

//-----------------------------------------------------------------------------
/// Has a profile been taken?
/// \return true if so, false otherwise.
//-----------------------------------------------------------------------------
bool RGP_API::IsProfileCaptured()
{
    return m_pImpl->IsProfileCaptured();
}

//-----------------------------------------------------------------------------
/// Get the name of the last captured profile. Call this after taking a capture.
/// \return The name of the file captured.
//-----------------------------------------------------------------------------
const char* RGP_API::GetProfileName()
{
    return m_pImpl->GetProfileName();
}
