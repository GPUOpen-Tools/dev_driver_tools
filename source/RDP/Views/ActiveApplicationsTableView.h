//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for the Active Applications interface.
//=============================================================================

#ifndef _ACTIVE_APPLICATIONS_TABLE_VIEW_H_
#define _ACTIVE_APPLICATIONS_TABLE_VIEW_H_

#include <QWidget>

class ActiveApplicationTableModel;
class DeveloperPanelModel;
class SetupTargetApplicationView;

namespace Ui {
class ActiveApplicationsTableView;
}

/// Class declaration for the Active Applications view
class ActiveApplicationsTableView : public QWidget
{
    Q_OBJECT

public:
    explicit ActiveApplicationsTableView(DeveloperPanelModel* pDeveloperPanelModel, SetupTargetApplicationView* pTargetApplicationView, QWidget* pParent = nullptr);
    ~ActiveApplicationsTableView();

    ActiveApplicationTableModel* GetActiveApplicationsTableModel() const { return m_pActiveApplicationsTableModel; }

private slots:
    void AddToList(bool clicked);
    void OnApplicationSelected(const QModelIndex& index);
    void OnRowDoubleClicked(const QModelIndex& index);
    void OnTableDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles = QVector<int>());
    void OnApplicationRemoved(const QString& applicationName);

private:
    void AddAppByRowIndex(int rowIndex);
    void AdjustTableColumns();

    Ui::ActiveApplicationsTableView* ui;                            ///< The active applications table interface.
    ActiveApplicationTableModel* m_pActiveApplicationsTableModel;   ///< The model associated with this view.
    DeveloperPanelModel* m_pDeveloperPanelModel;                    ///< The main RDP model.
    SetupTargetApplicationView* m_pSetupTargetApplicationView;      ///< The Target Application interface.
    bool m_traceInProgress;                                         ///< Trace in progress flag.
};

#endif // _ACTIVE_APPLICATIONS_TABLE_VIEW_H_
