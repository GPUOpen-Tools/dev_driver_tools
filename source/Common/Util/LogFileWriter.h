//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Declaration for the log file writer
//=============================================================================

#ifndef _LOG_FILE_WRITER_H_
#define _LOG_FILE_WRITER_H_

#include <QString>
#include <QMutex>

/// Log file writer implementation
class LogFileWriter
{

public:
    static LogFileWriter& Get();

    void WriteLog(const QString& logMessage);
    QString GetLogFileLocation();

private:
    // Constructor/destructor is private for singleton
    explicit LogFileWriter();
    ~LogFileWriter() {}

    QMutex                m_mutex;         ///< The mutex to write the log
    static LogFileWriter* s_pInstance;   ///< LogFileWriter instance pointer
};

#endif // _LOG_FILE_WRITER_H_
