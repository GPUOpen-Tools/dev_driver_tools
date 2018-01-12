//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A model used to maintain a list of active developer mode applications.
//=============================================================================

#ifndef _ACTIVE_APPLICATION_TABLE_MODEL_H_
#define _ACTIVE_APPLICATION_TABLE_MODEL_H_

#include <QWidget>
#include "ProcessInfoModel.h"

class QStandardItemModel;

/// An enumeration that declares the count and purpose of columns in the Active Applications table.
enum ActiveApplicationTableColumns
{
    ACTIVE_APPLICATION_TABLE_COLUMN_EXECUTABLE_NAME,
    ACTIVE_APPLICATION_TABLE_COLUMN_API,
    ACTIVE_APPLICATION_TABLE_COLUMN_RUNNING_STATUS,

    ACTIVE_APPLICATION_TABLE_COLUMN_COUNT,
};

/// A model used to maintain a list of active developer mode applications.
class ActiveApplicationTableModel : public QWidget
{
    Q_OBJECT
public:
    ActiveApplicationTableModel();
    virtual ~ActiveApplicationTableModel();

    bool GetExecutableNameAtRow(int rowIndex, QString& executableName) const;
    QStandardItemModel* GetTableModel() const { return m_pActiveApplicationsTableModel; }

public slots:
    void OnClientDiscovered(const ProcessInfoModel& processInfo, bool isActive);

private:
    int AddActiveApplication(const ProcessInfoModel& processInfo);
    int IsAlreadyInList(const ProcessInfoModel& processInfo) const;
    void SetTableModelData(const QString& modelData, uint row, uint column, enum Qt::AlignmentFlag alignment = Qt::AlignLeft);
    void SetRowEnabled(int rowIndex, bool isEnabled);

    QStandardItemModel* m_pActiveApplicationsTableModel;    ///< The model associated with the active applications table.
};

#endif // _ACTIVE_APPLICATION_TABLE_MODEL_H_
