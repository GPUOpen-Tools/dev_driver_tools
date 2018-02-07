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

    /// Initialization function. To be called before initializing the device.
    /// \return true if successful, false otherwise.
    bool Init();

    /// Cleanup function. To be called at application shutdown.
    void Finish();

    /// Start triggering a capture. The actual capture is done in a separate thread. The
    /// calling function will need to call IsProfileCaptured() to determine if the capture
    /// has finished.
    /// \param captureFileName The file (and path) used to save the captured profile to. If the
    ///  path is omitted, the file will be saved to the default folder. If nullptr is
    ///  specified, then a filename will be generated based on the process name and a date/time
    ///  timestamp
    /// \return true if capture started, false if not (either the driver hasn't completed its
    ///  initialization or a capture is currently in progress).
    bool TriggerCapture(const char* captureFileName);

    /// Has a profile been taken?
    /// \return true if so, false otherwise.
    bool IsProfileCaptured();

    /// Get the name of the last captured profile. Call this after taking a capture.
    /// \return The name of the file captured.
    const char* GetProfileName();

private:
    RGPClientInProcessModel*   m_pImpl;         ///< Pointer to the RGP capture implementation
};

#endif // _RGP_API_H_
