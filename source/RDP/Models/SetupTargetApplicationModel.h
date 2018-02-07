//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for the Setup Target Application model.
//=============================================================================

#ifndef _SETUP_TARGET_APPLICATION_MODEL_H_
#define _SETUP_TARGET_APPLICATION_MODEL_H_

#include "../Models/ProcessInfoModel.h"
#include <QWidget>
#include <QStandardItem>

class QStandardItemModel;
class QSortFilterProxyModel;

/// An enumeration that declares the purpose of columns in the Target Applications table.
enum TargetApplicationTableColumns
{
    TARGET_APPLICATION_TABLE_COLUMN_EXECUTABLE_NAME,
    // @HACK: Removing these for BETA 1. These columns need to be populated by pinging RDS to ask which applications are available.
    // This system isn't built yet, so we are temporarily removing these 3 columns.
    /*
    TARGET_APPLICATION_TABLE_COLUMN_PROCESS_ID,
    TARGET_APPLICATION_TABLE_COLUMN_TITLE,
    TARGET_APPLICATION_TABLE_COLUMN_API,
    */
    TARGET_APPLICATION_TABLE_COLUMN_APPLY_SETTINGS,
    TARGET_APPLICATION_TABLE_COLUMN_ENABLE_PROFILING,

    TARGET_APPLICATION_TABLE_COLUMN_COUNT,
};

/// Class for the Recent Connections model
class SetupTargetApplicationModel : public QWidget
{
    Q_OBJECT

public:
    SetupTargetApplicationModel();
    ~SetupTargetApplicationModel();

    QAbstractItemModel* GetTableModel();
    bool AddApplication(const QString& application);
    bool IsApplicationInTargetList(const QString& executableFilename) const;
    void RemoveApplication(int proxyRowIndex);
    void Update();
    void TargetApplicationTableClicked(const QModelIndex& proxyIndex);
    QString ActivelyProfiledApplication();
    int MapToSourceModelRow(const QModelIndex& index) const;
    bool GetExecutableNameAtRow(int index, QString& executableName) const;
    void ToggleProfilingForRow(int rowIndex);

public slots:
    void OnTraceCollectionStatusUpdated(bool traceIsBeingCollected);

signals:
    void ProfilingCheckboxUnchecked();
    void ProfilingCheckboxClickError();
    void ProfilerInUseWarning(ProcessInfoModel& processInfo);
    void QueryProfiledTargetInfo(ProcessInfoModel& processInfo);

private:
    void EnableProfilingForRow(int rowIndex);
    void SetTableModelData(const QString& modelData, uint row, uint column, enum Qt::AlignmentFlag alignment = Qt::AlignLeft);
    bool isCheckBoxClickValid(QStandardItem* pItem);

    QStandardItemModel*     m_pApplicationsTableModel;      ///< model associated with the target applications table
    QSortFilterProxyModel*  m_pProxyModel;                  ///< Proxy model used to keep view row order seperate from underlying data row order
    bool m_traceInProgress;                                 ///< trace in progress indicator
};

#endif // _SETUP_TARGET_APPLICATION_MODEL_H_

