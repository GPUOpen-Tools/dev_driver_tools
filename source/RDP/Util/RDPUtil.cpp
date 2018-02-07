//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of RDPUtil which holds useful utility functions.
//=============================================================================

#include "RDPUtil.h"
#include "../RDPDefinitions.h"
#include "../Views/MainWindow.h"
#include "../../DevDriverComponents/inc/ddPlatform.h"

#include <QFont>
#include <QTreeView>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QDir>

//-----------------------------------------------------------------------------
/// The global static instance of the output Window that all messages will flow into.
//-----------------------------------------------------------------------------
static MainWindow* s_pMainWindow = nullptr;

//-----------------------------------------------------------------------------
/// An output message handler used for Qt's message callback.
/// \param type The type of output message.
/// \param context The context of the output message.
/// \param msg The message string to display.
//-----------------------------------------------------------------------------
void OutputMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    Q_UNUSED(context);

    QString txt;

    switch (type)
    {
    case QtInfoMsg:
        txt = QString("qInfo(): %1").arg(msg);
        break;

    case QtDebugMsg:
        txt = QString("qDebug(): %1").arg(msg);
        break;

    case QtWarningMsg:
        txt = QString("qWarning(): %1").arg(msg);
        break;

    case QtCriticalMsg:
        txt = QString("qCritical(): %1").arg(msg);
        break;

    case QtFatalMsg:
        txt = QString("qFatal(): %1").arg(msg);
        break;

    default:
        txt = QString("default: %1").arg(msg);
        break;
    }

    s_pMainWindow->EmitSetText(txt);
}

//-----------------------------------------------------------------------------
/// Send a message to the output window.
/// \param pFormat The format string for the message.
//-----------------------------------------------------------------------------
void RDPUtil::DbgMsg(const char* pFormat, ...)
{
    static const int kMessageBufferLength = 2048;

    if (s_pMainWindow)
    {
        char buffer[kMessageBufferLength] = {};
        va_list args;
        va_start(args, pFormat);
        vsnprintf(buffer, kMessageBufferLength, pFormat, args);
        va_end(args);
        s_pMainWindow->EmitSetText(QString(buffer));
    }
    else
    {
        char buffer[kMessageBufferLength] = {};
        va_list args;
        va_start(args, pFormat);
        vsnprintf(buffer, kMessageBufferLength, pFormat, args);
        va_end(args);

        DD_PRINT(DevDriver::LogLevel::Always, "[RDP] %s\n", buffer);
    }
}

//-----------------------------------------------------------------------------
/// Set the enabled state of the disconnect button.
/// \param enabled Whether the button should be enabled.
//-----------------------------------------------------------------------------
void RDPUtil::SetDisconnectButtonEnabled(bool enabled)
{
    s_pMainWindow->SetDisconnectButtonEnabled(enabled);
}

//-----------------------------------------------------------------------------
/// Stringify the given device clock mode.
/// \param clockMode The GPU device clock mode enumeration to stringify.
/// \returns A stringified version of the given device clock mode.
//-----------------------------------------------------------------------------
QString RDPUtil::GetClockModeAsString(DevDriver::DriverControlProtocol::DeviceClockMode clockMode)
{
    using namespace DevDriver::DriverControlProtocol;

    QString result;

    switch (clockMode)
    {
    case DeviceClockMode::Unknown:
        {
            result = "Unknown";
        }
        break;
    case DeviceClockMode::Default:
        {
            result = gs_CLOCKS_MODE_NAME_TEXT_NORMAL;
        }
        break;
    case DeviceClockMode::Profiling:
        {
            result = gs_CLOCKS_MODE_NAME_TEXT_STABLE;
        }
        break;
    case DeviceClockMode::MinimumMemory:
        {
            result = "MinimumMemory";
        }
        break;
    case DeviceClockMode::MinimumEngine:
        {
            result = "MinimumEngine";
        }
        break;
    case DeviceClockMode::Peak:
        {
            result = gs_CLOCKS_MODE_NAME_TEXT_PEAK;
        }
        break;
    default:
        // A new DeviceClockMode has been added, so this switch needs to be updated.
        Q_ASSERT(false);
    }

    return result;
}

//-----------------------------------------------------------------------------
/// Get the default path to RGP (OS-aware)
/// \return The default path string
//-----------------------------------------------------------------------------
QString RDPUtil::GetDefaultRGPPath()
{
    QString defaultRGPPath = ".";
    defaultRGPPath.append(QDir::separator());
    defaultRGPPath.append(gs_RGP_EXECUTABLE_FILENAME);
#ifdef WIN32
    // Append an extension only in Windows.
    defaultRGPPath.append(".exe");
#endif

    return defaultRGPPath;
}

//-----------------------------------------------------------------------------
/// Open the profiling tab
//-----------------------------------------------------------------------------
void RDPUtil::OpenProfilingTab()
{
    if (s_pMainWindow != nullptr)
    {
        s_pMainWindow->OpenProfilingTab();
    }
}

//-----------------------------------------------------------------------------
/// Register the main RDP output window with RDPUtil such that it is accessible.
/// This is only to be called once, when initializing ProtocolPane.
/// \param pOutputWindow The output window being registered.
//-----------------------------------------------------------------------------
void RDPUtil::RegisterLogWindow(MainWindow* pOutputWindow)
{
    if (pOutputWindow != nullptr)
    {
        s_pMainWindow = pOutputWindow;

#ifdef _DEBUG
        qInstallMessageHandler(OutputMessageHandler);
#endif
    }
}

//-----------------------------------------------------------------------------
/// Display the notification overlay with the given strings and button options.
/// \param title The title string to display within the notification overlay.
/// \param message The message string to display within the notification overlay.
/// \param buttons The set of buttons to display within the notification overlay.
/// \param defaultButton An enum button bit for the default button (or zero to use the
/// highest bit value from the buttons parameter).
/// \returns The button that was clicked by the user within the notification.
//-----------------------------------------------------------------------------
NotificationWidget::Button RDPUtil::ShowNotification(const QString& title, const QString& message, uint buttons, uint defaultButton)
{
    return s_pMainWindow->ShowNotification(title, message, buttons, defaultButton);
}

//-----------------------------------------------------------------------------
/// Display the notification overlay with the given strings and button options.
/// \param title The title string to display within the notification overlay.
/// \param message The message string to display within the notification overlay.
/// \param buttons The set of buttons to display within the notification overlay.
/// \param showDoNotAsk A flag that determines if the "Do not ask me again" checkbox will be shown.
/// \param defaultButton An enum button bit for the default button (or zero to use the
/// highest bit value from the buttons parameter).
/// \returns The button that was clicked by the user within the notification.
//-----------------------------------------------------------------------------
NotificationWidget::Button RDPUtil::ShowNotification(const QString& title, const QString& message, uint buttons, bool& showDoNotAsk, uint defaultButton)
{
    return s_pMainWindow->ShowNotification(title, message, buttons, showDoNotAsk, defaultButton);
}

//-----------------------------------------------------------------------------
/// Unregister the current output window.
//-----------------------------------------------------------------------------
void RDPUtil::UnregisterLogWindow()
{
#ifdef _DEBUG
    qInstallMessageHandler(nullptr);
#endif
    s_pMainWindow = nullptr;
}
