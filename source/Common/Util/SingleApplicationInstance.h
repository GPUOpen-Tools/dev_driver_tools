//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A helper class used to check if a named process is already running on the system.
//=============================================================================

#ifndef _SINGLE_APPLICATION_INSTANCE_H_
#define _SINGLE_APPLICATION_INSTANCE_H_

#include <QApplication>
#include <QSharedMemory>
#include <QStringList>

class SingleInstance;

/// A helper class used to check if a named process is already running on the system.
class SingleApplicationInstance : public QApplication
{
    Q_OBJECT
public:
    SingleApplicationInstance(int &argc, char *argv[], const QString uniqueId, bool checkHeadlessInstances = false);
    virtual ~SingleApplicationInstance();
    bool IsAnotherInstanceRunning();
    bool IsInstanceRunning(const QString& uniqueKey) const;
    bool IsPrimaryInstance();
    bool NotifyAppInstanceStarted();

public slots:
    void OnCheckForNewInstance();

    // Reimplemented from QApplication so we can throw exceptions in slots
    virtual bool notify(QObject* pReceiver, QEvent* pEvent);

signals:
    void AppInstanceStarted();

private:
    bool CreateSharedMemory();
    bool IsNumericString(const QString& strValue) const;
    int FindProcessId(const QString& strMatchName, int excludedProcessId) const;
    void DisplayCharacters(const QString& strValue) const;
private:
    QString m_uniqueKey;                ///< GUID Key for this application
    bool m_anotherInstanceRunning;      ///< A bool that tracks if another instance is already running.
    QSharedMemory m_sharedMem;          ///< Shared memory that secondary instance uses to notify primary instance.
    SingleInstance* m_pSingleInstance;  ///< Instance detection mutex for non-gui applications
};

#endif // _SINGLE_APPLICATION_INSTANCE_H_
