//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  An interface class with the service class to initialize dev driver
///         protocols to capture rgp trace from within the profiled
///         application.
//=============================================================================
#ifndef _RGP_API_H_
#define _RGP_API_H_

#ifdef BUILD_DLL
 #include <RGP_API_export.h>
#else
 #define RGP_API_EXPORT
#endif

class RGPClientInProcessModel;

class RGP_API_EXPORT RGP_API
{
public:
    RGP_API();
    ~RGP_API();

    /// Initialization function. To be called before initializing the device
    /// \return true if successful, false otherwise
    bool Init();

    /// Cleanup function. To be called at application shutdown
    void Finish();

    /// Has a profile been taken? Don't need to do the rendering if so.
    /// \return true if so, false otherwise.
    bool IsProfileCaptured();

private:
    RGPClientInProcessModel*   m_pImpl;         ///< Pointer to the RGP capture implementation
};

#endif // _RGP_API_H_
