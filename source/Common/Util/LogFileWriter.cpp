//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of the LogFileWriter. The LogFileWriter facilitates
/// writing of log messages to a log file.
//=============================================================================

#include "LogFileWriter.h"

#include <QFile>
#include <QDir>

#include "../../RDP/Util/RDPUtil.h"
#include "../../RDP/RDPDefinitions.h"

/// LogFileWriter instance
LogFileWriter* LogFileWriter::s_pInstance = nullptr;

//-----------------------------------------------------------------------------
/// LogFileWriter constructor
///-----------------------------------------------------------------------------
LogFileWriter::LogFileWriter()
{
    // Delete the log file from the previous instance
    QFile::remove(GetLogFileLocation());
}

//-----------------------------------------------------------------------------
/// LogFileWriter instance get function.
/// \return a reference to the LogFileWriter instance.
//-----------------------------------------------------------------------------
LogFileWriter& LogFileWriter::Get()
{
    if (s_pInstance == nullptr)
    {
        s_pInstance = new LogFileWriter();
    }

    return *s_pInstance;
}

//-----------------------------------------------------------------------------
/// Write the log out to the log file.
/// \param logMessage The message to write to the log file.
//-----------------------------------------------------------------------------
void LogFileWriter::WriteLog(const QString& logMessage)
{
    // Lock the mutex before writing out the log to the file
    // The mutex gets released when this method returns
    QMutexLocker locker(&m_mutex);

    // Get the file name and location
    QFile file(GetLogFileLocation());

    // Open the file
    if (file.open(QIODevice::WriteOnly | QIODevice::Append))
    {
        // Convert the QString to a byte array
        QByteArray byteArray = logMessage.toLatin1();

        // Write the data to the file
        file.write(byteArray.data());

        // Add a new line to the file
        file.write("\r\n");

        // Now close the file
        file.close();
    }
}

//-----------------------------------------------------------------------------
/// Get the location of the log file.
/// \return The location of the log file
//-----------------------------------------------------------------------------
QString LogFileWriter::GetLogFileLocation()
{
    QString logFile = "";

    // Get file location
    logFile = ToolUtil::GetDriverToolsXmlFileLocation();

    // Add the file name
    logFile.append(QDir::separator());
    logFile.append(gs_PRODUCT_LOG_FILENAME);

    return logFile;
}
