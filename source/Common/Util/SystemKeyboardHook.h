//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A system wide keyboard handler
//=============================================================================

#ifndef _SYSTEM_KEYBOARD_HOOK_H_
#define _SYSTEM_KEYBOARD_HOOK_H_

#include <QObject>

class KeyboardHookImpl;

// Interface for system wide keyboard hook
class SystemKeyboardHook : public QObject
{
    Q_OBJECT

public:
    static SystemKeyboardHook* GetInstance();
    bool Connect();
    void Disconnect();
    void SetHotKey(uint asciiCode, uint modifiers);
    bool Enabled();

signals:
    void HotKeyPressed(uint key);

private:
    SystemKeyboardHook() {}

    static KeyboardHookImpl* s_pImpl;    ///< Pointer to platform dependent implementation
};
#endif // _SYSTEM_KEYBOARD_HOOK_H_
