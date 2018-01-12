//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A system wide keyboard handler
//=============================================================================

#include "SystemKeyboardHook.h"

KeyboardHookImpl* SystemKeyboardHook::s_pImpl = nullptr;

//-----------------------------------------------------------------------------
/// Keyboard hook base implementation class
//-----------------------------------------------------------------------------
class KeyboardHookImpl
{
public:
    virtual bool Enabled() = 0;
    virtual bool Connect() = 0;
    virtual void Disconnect() = 0;

    //-----------------------------------------------------------------------------
    /// Configures the hot key triggers the HotKeyPressed signal.
    /// \param asciiCode The keyboard ascii code for the hot key.
    /// \param keyboard modifiers (Qt::ControlModifier and/or Qt::ShiftModifier)
    //-----------------------------------------------------------------------------
    virtual void SetHotKey(uint asciiCode, uint modifiers)
    {
        m_hotkey.asciiCode = asciiCode;
        m_hotkey.modifiers = modifiers;
    }

protected:
    //-----------------------------------------------------------------------------
    /// Constructor
    //-----------------------------------------------------------------------------
    explicit KeyboardHookImpl()
    {
    }

    //-----------------------------------------------------------------------------
    /// Destructor
    //-----------------------------------------------------------------------------
    virtual ~KeyboardHookImpl()
    {
    }

    /// structure defining the hotkey.
    struct HotKeyDef
    {
        uint asciiCode;     ///< The ascii code of the hotkey
        uint modifiers;     ///< Modifiers (if any) required by the hotkey (shift, ctrl, alt, etc.)
    };

    HotKeyDef m_hotkey;     ///< Hot key definition
};

#ifdef Q_OS_WIN
#include <Windows.h>

//-----------------------------------------------------------------------------
/// global keyboard hook Windows implementation
//-----------------------------------------------------------------------------
class KeyboardHookWindowsImpl : public KeyboardHookImpl
{
public:
    //-----------------------------------------------------------------------------
    /// Gets the KeyboardHookWindowsImpl instance.
    /// \returns pointer to the KeyboardHookWindowsImpl instance.
    //-----------------------------------------------------------------------------
    static KeyboardHookWindowsImpl* GetInstance()
    {
        static KeyboardHookWindowsImpl s_impl;
        return &s_impl;
    }

    //-----------------------------------------------------------------------------
    /// Connects the keyboard hook
    /// \returns true if reconnected successfully, false otherwise
    //-----------------------------------------------------------------------------
    bool Connect()
    {
        if (m_handle == NULL)
        {
            m_handle = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandle(NULL), 0);
        }

        return m_handle;
    }

    //-----------------------------------------------------------------------------
    /// Disconnects the keyboard hook
    //-----------------------------------------------------------------------------
    void Disconnect()
    {
        if (m_handle)
        {
            UnhookWindowsHookEx(m_handle);
            m_handle = NULL;
        }
    }

    //-----------------------------------------------------------------------------
    /// Is the global keyboard hook enabled? Currently always available on Windows,
    /// only available on Linux is the Panel is run with root privileges.
    /// \return true. Always enabled on Windows.
    //-----------------------------------------------------------------------------
    bool Enabled()
    {
        return true;
    }

private:
    //-----------------------------------------------------------------------------
    /// Constructor for KeyboardHookWindows
    //-----------------------------------------------------------------------------
    explicit KeyboardHookWindowsImpl()
        : m_handle(NULL)
    {
    }

    //-----------------------------------------------------------------------------
    /// Destructor for KeyboardHookWindows
    //-----------------------------------------------------------------------------
    virtual ~KeyboardHookWindowsImpl()
    {
    }

    //-----------------------------------------------------------------------------
    /// Determines if the keyboard hook is connected
    /// \returns true if connected, false otherwise
    //-----------------------------------------------------------------------------
    bool Connected()
    {
        return m_handle;
    }

    //-----------------------------------------------------------------------------
    /// Get Keyboard mapped modifiers for the key press.
    /// \returns Shift and Control flags
    //-----------------------------------------------------------------------------
    uint GetModifiers()
    {
        int modifiers = 0;
        if ((GetKeyState(VK_SHIFT) & 0x8000))
        {
            modifiers |= Qt::ShiftModifier;
        }

        if ((GetKeyState(VK_CONTROL) & 0x8000))
        {
            modifiers |= Qt::ControlModifier;
        }
        return modifiers;
    }

    //-----------------------------------------------------------------------------
    /// Reconnects the keyboard hook
    /// \returns true if reconnected successfully, false otherwise
    //-----------------------------------------------------------------------------
    bool Reconnect()
    {
        Disconnect();
        return Connect();
    }

    //-----------------------------------------------------------------------------
    /// Keyboard callback procedure (Windows specific)
    /// \param nCode determine how to process the message
    /// \param wParam The virtual-key code of the key that generated the keystroke message.
    /// \lParam The repeat count, scan code, extended-key flag, context code, previous key-state
    /// flag, and transition-state flag.
    /// returns The result for the next keyboard hook in the change or returns 0 if the hot key
    /// has been processed.
    //-----------------------------------------------------------------------------
    static LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
    {
        bool keypressHandled = false;

        if (nCode == HC_ACTION)
        {
            KBDLLHOOKSTRUCT* pKeyboard = (KBDLLHOOKSTRUCT*)lParam;
            if (wParam == WM_SYSKEYDOWN || wParam == WM_KEYDOWN)
            {
                if (GetInstance()->m_hotkey.asciiCode == pKeyboard->vkCode)
                {
                    if (GetInstance()->m_hotkey.modifiers == GetInstance()->GetModifiers())
                    {
                        emit SystemKeyboardHook::GetInstance()->HotKeyPressed((uint)pKeyboard->vkCode);
                        keypressHandled = true;
                    }
                }
            }

            GetInstance()->Reconnect();
        }
        else if (nCode < 0)
        {
            return CallNextHookEx(0, nCode, wParam, lParam);
        }

        return keypressHandled;
    }

    HHOOK m_handle;         ///< Handle used by the keyboard hook
};

#else
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <dirent.h>

#include <fcntl.h>
#include <linux/input.h>
#include <iostream>
#include <fstream>

static const uint64_t s_KEYBOARD_TIMER_RATE = 20;  // keyboard poll rate, in ms

static const char* s_pKeyboardPath = "/dev/input/by-path/";
static const char* s_pKeyboardFileMask = "event-kbd";
static const int   s_bufferSize = 1024;

//-----------------------------------------------------------------------------
/// global keyboard hook Linux implementation
//-----------------------------------------------------------------------------
class KeyboardHookLinuxImpl : public KeyboardHookImpl
{
public:
    //-----------------------------------------------------------------------------
    /// Gets the KeyboardHookLinuxImpl instance.
    /// \returns pointer to the KeyboardHookLinuxImpl instance.
    //-----------------------------------------------------------------------------
    static KeyboardHookLinuxImpl* GetInstance()
    {
        static KeyboardHookLinuxImpl s_impl;
        return &s_impl;
    }

    //-----------------------------------------------------------------------------
    /// Connects the keyboard hook
    /// \returns true if reconnected successfully, false otherwise
    //-----------------------------------------------------------------------------
    bool Connect()
    {
        m_enabled = false;

        // look for KeyboardDevice.txt file and use the driver in here if it exists
        bool found = false;

        // get pid and look in /proc/<pid>/exe
        static char procPath[s_bufferSize];
        static char exePath[s_bufferSize];
        memset(exePath, 0, s_bufferSize);
        struct stat info;
        pid_t pid = getpid();
        sprintf(procPath, "/proc/%d/exe", pid);
        if (readlink(procPath, exePath, s_bufferSize) != -1)
        {
            char* pos = strrchr(exePath, '/');
            if (pos != nullptr)
            {
                *pos = '\0';
                strcat(exePath,"/KeyboardDevice.txt");
                std::ifstream kbdDeviceFile;
                kbdDeviceFile.open(exePath, std::ifstream::in);
                if (kbdDeviceFile.good())
                {
                    // open all lines in the file and try to read the keyboard device
                    for (std::string line; getline(kbdDeviceFile, line);)
                    {
                        m_keyboardFileHandle = open(line.c_str(), O_RDONLY);
                        if (m_keyboardFileHandle != -1)
                        {
                            found = true;
                            break;
                        }
                    }

                    kbdDeviceFile.close();
                }
            }
        }

        // If KeyboardDevice.txt file not found or no valid keyboard devices in the file,
        // traverse all files in the keyboard folder.
        if (found == false)
        {
            DIR* dir;
            struct dirent* ent;
            if ((dir = opendir(s_pKeyboardPath)) != nullptr)
            {
                while ((ent = readdir(dir)) != nullptr && found == false)
                {
                    if (strstr(ent->d_name, s_pKeyboardFileMask) != nullptr)
                    {
                        static char fileBuffer[s_bufferSize];
                        sprintf(fileBuffer, "%s%s", s_pKeyboardPath, ent->d_name);
                        m_keyboardFileHandle = open(fileBuffer, O_RDONLY);
                        if (m_keyboardFileHandle != -1)
                        {
                            found = true;
                        }
                    }
                }
                closedir(dir);
            }
        }

        if (m_keyboardFileHandle != -1)
        {
            // set file access to non-blocking read (in case the keyboard buffer is empty)
            int flags = fcntl(m_keyboardFileHandle, F_GETFL, 0);
            fcntl(m_keyboardFileHandle, F_SETFL, flags | O_NONBLOCK);

            m_enabled = StartTimer();
        }

        return m_enabled;
    }

    //-----------------------------------------------------------------------------
    /// Disconnects the keyboard hook
    //-----------------------------------------------------------------------------
    void Disconnect()
    {
        if (m_enabled)
        {
            timer_delete(m_timerID);
            m_enabled = false;
            close(m_keyboardFileHandle);
        }
    }

    //-----------------------------------------------------------------------------
    /// Is the global keyboard hook enabled? Currently always available on Windows,
    /// only available on Linux is the Panel is run with root privileges.
    /// \return true if enabled, false if not.
    //-----------------------------------------------------------------------------
    bool Enabled()
    {
        return m_enabled;
    }

    //-----------------------------------------------------------------------------
    /// Configures the hot key.
    /// \param asciiCode The keyboard ascii code for the hot key.
    /// \param keyboard modifiers (Qt::ControlModifier and/or Qt::ShiftModifier)
    //-----------------------------------------------------------------------------
    virtual void SetHotKey(const uint asciiCode, const uint modifiers)
    {
        KeyboardHookImpl::SetHotKey(asciiCode, modifiers);

        // convert ascii code to a key code
        // TODO: build this out later
        m_captureKeyCode = KEY_C;
        m_captureAsciiCode = asciiCode;
    }

private:
    //-----------------------------------------------------------------------------
    /// Constructor for KeyboardHookLinuxImpl
    //-----------------------------------------------------------------------------
    explicit KeyboardHookLinuxImpl()
        : m_enabled(false)
        , m_keyboardFileHandle(-1)
    {
    }

    //-----------------------------------------------------------------------------
    /// Destructor for KeyboardHookLinuxImpl
    //-----------------------------------------------------------------------------
    virtual ~KeyboardHookLinuxImpl()
    {
    }

    //-----------------------------------------------------------------------------
    /// Timer signal handler. This is the callback that checks the keyboard for
    /// the capture hotkey combination. This function isn't called directly but
    /// is called as an event handler, with the parameters passed in by the OS.
    /// See the Linux manual entry for sigaction for more information.
    //-----------------------------------------------------------------------------
    static void KeyboardProc(int sig, siginfo_t *si, void *uc)
    {
        Q_UNUSED(sig);
        Q_UNUSED(si);
        Q_UNUSED(uc);

        input_event ie;
        if (read(GetInstance()->m_keyboardFileHandle, &ie, sizeof(ie)) != 0)
        {
            // is it a key press or release
            if (ie.type == EV_KEY)
            {
                bool pressed = false;
                if (ie.value != 0)
                {
                    pressed = true;
                }

                if (GetInstance()->m_hotkey.modifiers == GetInstance()->GetModifiers(ie.code, pressed))
                {
                    if (GetInstance()->m_hotkey.asciiCode == GetInstance()->GetAsciiCode(ie.code, pressed))
                    {
                        emit SystemKeyboardHook::GetInstance()->HotKeyPressed(GetInstance()->m_hotkey.asciiCode);
                    }
                }
            }
        }
    }

    //-----------------------------------------------------------------------------
    /// Start a timer which is used to poll the keyboard.
    /// \return true if the timer is started successfully, false otherwise
    //-----------------------------------------------------------------------------
    bool StartTimer()
    {
        struct sigevent sev;
        struct itimerspec its;
        sigset_t mask;
        struct sigaction sa;

        // Establish handler for timer signal
        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = KeyboardProc;
        sigemptyset(&sa.sa_mask);
        if (sigaction(SIGRTMIN, &sa, nullptr) != -1)
        {
            // Block timer signal temporarily
            sigemptyset(&mask);
            sigaddset(&mask, SIGRTMIN);
            if (sigprocmask(SIG_SETMASK, &mask, nullptr) != -1)
            {
                // Create the timer
                sev.sigev_notify = SIGEV_SIGNAL;
                sev.sigev_signo = SIGRTMIN;
                sev.sigev_value.sival_ptr = &m_timerID;
                if (timer_create(CLOCK_REALTIME, &sev, &m_timerID) != -1)
                {
                    // Start the timer
                    uint64_t freq_nanosecs = s_KEYBOARD_TIMER_RATE * 1000000;
                    its.it_value.tv_sec = freq_nanosecs / 1000000000;
                    its.it_value.tv_nsec = freq_nanosecs % 1000000000;
                    its.it_interval.tv_sec = its.it_value.tv_sec;
                    its.it_interval.tv_nsec = its.it_value.tv_nsec;

                    if (timer_settime(m_timerID, 0, &its, nullptr) != -1)
                    {
                        if (sigprocmask(SIG_UNBLOCK, &mask, nullptr) != -1)
                        {
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }

    //-----------------------------------------------------------------------------
    /// Get Keyboard mapped modifiers for the key press.
    /// \param keyCode The keycode of the key pressed
    /// \param pressed true if the key is pressed, false if released
    /// \returns Shift and Control flags
    //-----------------------------------------------------------------------------
    uint GetModifiers(const int keyCode, const bool pressed)
    {
        static int modifiers = 0;

        if (keyCode == KEY_LEFTCTRL || keyCode == KEY_RIGHTCTRL)
        {
            if (pressed)
            {
                modifiers |= Qt::ControlModifier;
            }
            else
            {
                modifiers &= ~Qt::ControlModifier;
            }
        }
        else if (keyCode == KEY_LEFTSHIFT || keyCode == KEY_RIGHTSHIFT)
        {
            if (pressed)
            {
                modifiers |= Qt::ShiftModifier;
            }
            else
            {
                modifiers &= ~Qt::ShiftModifier;
            }
        }

        return modifiers;
    }

    //-----------------------------------------------------------------------------
    /// Get the ascii code for the provided keycode
    //-----------------------------------------------------------------------------
    uint GetAsciiCode(const int keyCode, const bool pressed)
    {
        if (keyCode == m_captureKeyCode && pressed == true)
        {
            return m_captureAsciiCode;
        }
        return 0;
    }

    bool        m_enabled;              ///< hetkey functionality enabled
    int         m_keyboardFileHandle;   ///< handle to the keyboard device file
    int         m_captureKeyCode;       ///< key used for capture
    uint        m_captureAsciiCode;     ///< ASCII code corresponding to capture key code
    timer_t     m_timerID;              ///< Timer to periodically poll the keyboard
};

#endif //Q_OS_WIN

//-----------------------------------------------------------------------------
/// Gets the SystemKeyboardHook instance. Uses a static variable to eliminate
/// memory leaks.
/// \returns pointer to the SystemKeyboardHook instance.
//-----------------------------------------------------------------------------
SystemKeyboardHook* SystemKeyboardHook::GetInstance()
{
    static SystemKeyboardHook s_systemKeyboardHookInstance;
#ifdef Q_OS_WIN
    s_pImpl = KeyboardHookWindowsImpl::GetInstance();
#else
    s_pImpl = KeyboardHookLinuxImpl::GetInstance();
#endif
    return &s_systemKeyboardHookInstance;
}

//-----------------------------------------------------------------------------
/// Is the global keyboard hook enabled? Currently always available on Windows,
/// only available on Linux is the Panel is run with root privileges.
/// \return true if enabled, false if not.
//-----------------------------------------------------------------------------
bool SystemKeyboardHook::Enabled()
{
    return s_pImpl->Enabled();
}

//-----------------------------------------------------------------------------
/// Configures the hot key.
/// \param asciiCode The keyboard ascii code for the hot key.
/// \param keyboard modifiers (Qt::ControlModifier and/or Qt::ShiftModifier)
//-----------------------------------------------------------------------------
void SystemKeyboardHook::SetHotKey(uint asciiCode, uint modifiers)
{
    s_pImpl->SetHotKey(asciiCode, modifiers);
}

//-----------------------------------------------------------------------------
/// Connects the keyboard hook
/// \returns true if reconnected successfully, false otherwise
//-----------------------------------------------------------------------------
bool SystemKeyboardHook::Connect()
{
    return s_pImpl->Connect();
}

//-----------------------------------------------------------------------------
/// Disconnects the keyboard hook
//-----------------------------------------------------------------------------
void SystemKeyboardHook::Disconnect()
{
    return s_pImpl->Disconnect();
}
