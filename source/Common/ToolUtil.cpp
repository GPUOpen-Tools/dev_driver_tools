//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of ToolUtil which holds useful utility functions.
//=============================================================================

#include <QDir>
#include <QFile>
#include <QPalette>
#include <QWidget>
#include "ToolUtil.h"
#include "DriverToolsDefinitions.h"
#include "../../DevDriverComponents/inc/ddPlatform.h"
#include "../../Common/Views/DebugWindow.h"
#include "../../Common/Version.h"

#ifdef WIN32
    #include <Shlobj.h>
#else
    #include <unistd.h>
    #include <sys/stat.h>
    #include <pwd.h>
#endif

//-----------------------------------------------------------------------------
/// The global static instance of the Debug Window that all messages will flow into.
//-----------------------------------------------------------------------------
static DebugWindow* s_pDebugWindow = nullptr;

//-----------------------------------------------------------------------------
/// A debug message handler used for Qt's message callback.
/// \param type The type of debug message.
/// \param context The context of the debug message.
/// \param msg The message string to display.
//-----------------------------------------------------------------------------
void DebugMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
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

    s_pDebugWindow->EmitSetText(txt);
}

//-----------------------------------------------------------------------------
/// Check if the incoming filepath is valid.
/// \param filepath The filepath to check.
/// \returns True if the filepath is valid and the file exists, and false if it's invalid.
//-----------------------------------------------------------------------------
bool ToolUtil::CheckFilepathExists(const QString& filepath)
{
    if (filepath.isEmpty())
    {
        return false;
    }

    QFileInfo rgpPathInfo(filepath);
    QDir rgpFilepath = rgpPathInfo.absoluteDir();

    return rgpFilepath.exists();
}

//-----------------------------------------------------------------------------
/// Send a message to the debug window.
/// \param pFormat The format string for the message.
//-----------------------------------------------------------------------------
void ToolUtil::DbgMsg(const char* pFormat, ...)
{
    if (s_pDebugWindow)
    {
        char buffer[2048] = {};
        va_list args;
        va_start(args, pFormat);
        vsnprintf(buffer, 2048, pFormat, args);
        va_end(args);
        s_pDebugWindow->EmitSetText(QString(buffer));
    }
}

//-----------------------------------------------------------------------------
/// Get the location on disk for the DriverTools settings. Find the 'Temp' folder on the local OS
/// and create a RadeonDeveloperDriver subfolder (on linux, create .RadeonDeveloperDriver folder)
/// \returns The path to the common RadeonDeveloperDriver settings file location.
//-----------------------------------------------------------------------------
QString ToolUtil::GetDriverToolsXmlFileLocation()
{
    QString xmlFile = "";

#ifdef _WIN32
    LPWSTR wszPath = nullptr;
    HRESULT hr = SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &wszPath);
    Q_UNUSED(hr);
    Q_ASSERT(hr == S_OK);

    xmlFile = QString::fromUtf16((const ushort*)wszPath);
    xmlFile.append(QDir::separator());
    xmlFile.append(gs_RDP_SETTINGS_DIRECTORY);

#else

    struct passwd* pw = getpwuid(getuid());
    if (pw != nullptr)
    {
        const char* homedir = pw->pw_dir;
        xmlFile = homedir;
    }

    xmlFile.append(QDir::separator());
    xmlFile.append(".");
    xmlFile.append(gs_RDP_SETTINGS_DIRECTORY);
#endif

    // Make sure the folder exists. If not, create it.
    std::string dir = xmlFile.toStdString();
    if (QDir(dir.c_str()).exists() == false)
    {
        QDir qdir;
        bool pathCreated = qdir.mkpath(dir.c_str());
        if (pathCreated == false)
        {
            ToolUtil::DbgMsg("[RDP] Failed to create settings file directory at %s", dir.c_str());
        }
    }

    return xmlFile;
}

//-----------------------------------------------------------------------------
/// Generate a formatted version number string to display to the user.
/// \returns A version string with the build version information.
//-----------------------------------------------------------------------------
QString ToolUtil::GetFormattedVersionString()
{
    QString versionString = "";

    // Print the version number info into the output string.
    versionString.sprintf("V%s", DEV_DRIVER_TOOLS_VERSION_STRING);

    return versionString;
}

//-----------------------------------------------------------------------------
/// Convert a DevDriver::Result to a printable QString.
/// \param result The result code to stringify.
/// \returns A stringified version of the given result code.
//-----------------------------------------------------------------------------
QString ToolUtil::GetResultString(DevDriver::Result result)
{
    using namespace DevDriver;

    switch (result)
    {
    case Result::Success:
        return "Success";
    case Result::Error:
        return "Error";
    case Result::NotReady:
        return "NotReady";
    case Result::VersionMismatch:
        return "VersionMismatch";
    case Result::Unavailable:
        return "Unavailable";
    case Result::Rejected:
        return "Rejected";
    case Result::EndOfStream:
        return "EndOfStream";
    case Result::Aborted:
        return "Aborted";
    default:
        // If this happens, the Result enum was updated, and
        // this switch should be updated as well.
        Q_ASSERT(false);
    }

    return "Unhandled";
}

//-----------------------------------------------------------------------------
/// Convert a DevDriver::Protocol enumeration into a printable QString.
/// \param protocolType The protocol type to print.
/// \returns A stringified version of the given protocol enum.
//-----------------------------------------------------------------------------
QString ToolUtil::GetProtocolTypeString(DevDriver::Protocol protocolType)
{
    using namespace DevDriver;

    switch (protocolType)
    {
    case Protocol::DriverControl:
        return "DriverControl";
    case Protocol::Logging:
        return "Logging";
    case Protocol::Settings:
        return "Settings";
    case Protocol::RGP:
        return "RGP";
    case Protocol::ETW:
        return "ETW";
    default:
        // If this happens, the Protocol enum was updated, and
        // this switch should be updated as well.
        Q_ASSERT(false);
    }

    return "Unknown";
}

//-----------------------------------------------------------------------------
/// Register the DebugWindow with ToolUtil such that it is accessible.
/// This is only to be called once, when initializing MainWindow.
/// \param pDebugWindow The debug window being registered.
//-----------------------------------------------------------------------------
void ToolUtil::RegisterDbgWindow(DebugWindow* pDebugWindow)
{
    if (pDebugWindow != nullptr)
    {
        s_pDebugWindow = pDebugWindow;
        qInstallMessageHandler(DebugMessageHandler);
    }
}

//-----------------------------------------------------------------------------
/// Set a widget's background color.
/// \param pWidget The widget to change background color
/// \param color The new color
//-----------------------------------------------------------------------------
void ToolUtil::SetWidgetBackgroundColor(QWidget* pWidget, const QColor& color)
{
    if (pWidget != nullptr)
    {
        QPalette palette(pWidget->palette());
        palette.setColor(QPalette::Background, color);
        pWidget->setPalette(palette);
        pWidget->setAutoFillBackground(true);
    }
}
