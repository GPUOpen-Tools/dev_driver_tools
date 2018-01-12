//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation for Platform-specific process handling.
//=============================================================================

#ifdef WIN32
#include <Windows.h>
#endif

#include <QProcess>

#include "Process.h"
#include "DriverToolsDefinitions.h"

/// wait 5 seconds for the process to close down after terminating it
static const int s_WAIT_FOR_FINISHED_TIMEOUT = 5000;

/// Base Process abstract class
class ProcessImpl
{
public:
    /// Constructor
    ProcessImpl() {}

    /// Destructor
    virtual ~ProcessImpl() {}

    /// Create a new process
    virtual DWORD Create(const QString& executablePath, const QString& workingDir, const QString& args, QObject* pParent) = 0;

    /// Terminate the created process
    virtual bool Terminate() = 0;
};

/// Class to use Qt's Process functionality, QProcess
class ProcessQt : public ProcessImpl
{
public:
    //-----------------------------------------------------------------------------
    /// Constructor.
    //-----------------------------------------------------------------------------
    ProcessQt()
        : m_pProcess(nullptr)
    {
    }

    //-----------------------------------------------------------------------------
    /// Destructor.
    //-----------------------------------------------------------------------------
    ~ProcessQt()
    {
        Terminate();
    }

    //-----------------------------------------------------------------------------
    /// Create a new process.
    /// \param executablePath The full path of the executable process to start
    /// \param workingDir The working directory of the exeutable to start
    /// \param args The command line arguments to start with
    /// \param pParent Pointer to the parent object
    /// \return 0 if no error, anything else for error code
    //-----------------------------------------------------------------------------
    DWORD Create(const QString& executablePath, const QString& workingDir, const QString& args, QObject* pParent)
    {
        m_pProcess = new QProcess(pParent);
        m_pProcess->setWorkingDirectory(workingDir);

        QString commandLine = (executablePath + " " + args);
        m_pProcess->start(commandLine);
        bool startedProcess = m_pProcess->waitForStarted();

        if (startedProcess == true)
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }

    //-----------------------------------------------------------------------------
    /// Terminate the created process
    //-----------------------------------------------------------------------------
    bool Terminate()
    {
        if (m_pProcess != nullptr)
        {
            m_pProcess->terminate();
            bool finished = m_pProcess->waitForFinished(s_WAIT_FOR_FINISHED_TIMEOUT);
            if (finished == false)
            {
                m_pProcess->kill();
            }
            SAFE_DELETE(m_pProcess);
        }
        return true;
    }

private:
    QProcess* m_pProcess;
};

#ifdef WIN32
/// Class to implement Win32's Process functionality
class ProcessWin32 : public ProcessImpl
{
public:
    //-----------------------------------------------------------------------------
    /// Constructor.
    //-----------------------------------------------------------------------------
    ProcessWin32()
        : m_processHandle(0)
    {
    }

    //-----------------------------------------------------------------------------
    /// Destructor.
    //-----------------------------------------------------------------------------
    ~ProcessWin32()
    {
    }

    //-----------------------------------------------------------------------------
    /// \param executablePath The full path of the executable process to start
    /// \param workingDir The working directory of the exeutable to start
    /// \param args The command line arguments to start with
    /// \param pParent Pointer to the parent object
    /// \return 0 if no error, anything else for error code
    //-----------------------------------------------------------------------------
    DWORD Create(const QString& executablePath, const QString& workingDir, const QString& args, QObject* pParent)
    {
        Q_UNUSED(workingDir);
        Q_UNUSED(pParent);

        // Combine exe and args into single command line c string
        std::string commandLineString = (executablePath + " " + args).toStdString();
        char* commandLineAsChars = const_cast<char*>(commandLineString.c_str());

        STARTUPINFO info = { sizeof(info) };
        PROCESS_INFORMATION processInfo;
        BOOL createResult = CreateProcess(NULL, commandLineAsChars, NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo);

        m_processHandle = processInfo.hProcess;

        if (createResult == TRUE)
        {
            return S_OK;
        }
        else if (createResult == FALSE)
        {
            DWORD lastError = GetLastError();
            return lastError;
        }

        return false;
    }

    //-----------------------------------------------------------------------------
    /// Terminate the created process
    //-----------------------------------------------------------------------------
    bool Terminate()
    {
        bool result = TerminateProcess(m_processHandle, 0);
        CloseHandle(m_processHandle);
        return result;
    }

private:
    HANDLE m_processHandle;     ///< Handle to created process
};
#endif // def WIN32

//-----------------------------------------------------------------------------
/// Constructor.
//-----------------------------------------------------------------------------
Process::Process()
{
#ifdef WIN32
    m_pImpl = new ProcessWin32();
#else
    m_pImpl = new ProcessQt();
#endif
}

//-----------------------------------------------------------------------------
/// Destructor.
//-----------------------------------------------------------------------------
Process::~Process()
{
    delete m_pImpl;
}

//-----------------------------------------------------------------------------
/// \param executablePath The full path of the executable process to start
/// \param workingDir The working directory of the exeutable to start
/// \param args The command line arguments to start with
/// \param pParent Pointer to the parent object
/// \return 0 if no error, anything else for error code
//-----------------------------------------------------------------------------
DWORD Process::Create(const QString& executablePath, const QString& workingDir, const QString& args, QObject* pParent)
{
    return m_pImpl->Create(executablePath, workingDir, args, pParent);
}

//-----------------------------------------------------------------------------
/// Terminate the created process
//-----------------------------------------------------------------------------
bool Process::Terminate()
{
    return m_pImpl->Terminate();
}
