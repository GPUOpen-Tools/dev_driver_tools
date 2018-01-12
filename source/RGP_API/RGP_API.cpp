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
/// Initialization function. To be called before initializing the device
/// \return true if successful, false otherwise
//-----------------------------------------------------------------------------
bool RGP_API::Init()
{
    return m_pImpl->Init();
}

//-----------------------------------------------------------------------------
/// Cleanup function. To be called at application shutdown
//-----------------------------------------------------------------------------
void RGP_API::Finish()
{
    m_pImpl->Finish();
}

//-----------------------------------------------------------------------------
/// Has a profile been taken? Don't need to do the rendering if so.
/// \return true if so, false otherwise.
//-----------------------------------------------------------------------------
bool RGP_API::IsProfileCaptured()
{
    return m_pImpl->IsProfileCaptured();
}
