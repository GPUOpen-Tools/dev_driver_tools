//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header file for Platform-specific process handling.
//=============================================================================

#ifndef _PROCESS_H_
#define _PROCESS_H_

class QObject;
class QString;
class ProcessImpl;

#ifndef WIN32
typedef unsigned long DWORD;
#endif

/// class to handle platform-specific process creation and termination
class Process
{
public:
    Process();
    ~Process();

    DWORD Create(const QString& executablePath, const QString& workingDir, const QString& args, QObject* pParent);
    bool Terminate();

public:
    ProcessImpl*    m_pImpl;    ///< Platform-specific implementation
};

#endif // def _PROCESS_H_
