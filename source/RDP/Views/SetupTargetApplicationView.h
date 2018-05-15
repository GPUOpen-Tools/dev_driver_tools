//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for the Setup Target Application panel interface.
//=============================================================================

#ifndef _SETUP_TARGET_APPLICATION_VIEW_H_
#define _SETUP_TARGET_APPLICATION_VIEW_H_

#include <QWidget>

class SetupTargetApplicationModel;
class DeveloperPanelModel;
class ProcessInfoModel;

namespace Ui {
class SetupTargetApplicationView;
}

/// Class for the Setup Target Application view
class SetupTargetApplicationView : public QWidget
{
    Q_OBJECT

public:
    explicit SetupTargetApplicationView(DeveloperPanelModel* pPanelModel, QWidget* pParent = nullptr);
    ~SetupTargetApplicationView();
    SetupTargetApplicationModel* GetSetupTargetApplicationModel();

    void dragEnterEvent(QDragEnterEvent* pEvent);
    void dropEvent(QDropEvent* pEvent);

    bool AddExecutableToList(const QString& executableFilename);

signals:
    void ApplicationRemovedFromList(const QString& applicationName);

public slots:
    void OnTraceCollectionStatusUpdated(bool traceIsBeingCollected);
    void OnProfilingCheckboxClickError();
    void OnProfilingMultipleTargetsWarning(const ProcessInfoModel& profiledProcessInfo);
    void OnProfilerInUseWarning(const ProcessInfoModel& processInfo);

private slots:
    void AddToList(bool clicked);
    void RemoveFromList(bool clicked);
    void OnReturnPressedOnExecutableLine();
    void OnTargetExeBrowseButtonPressed();
    void OnApplicationSelected(const QModelIndex& index);
    void OnTargetExeLineEditTextChanged(const QString& text);

private:
    void AdjustTableColumns();

    Ui::SetupTargetApplicationView *ui;

    SetupTargetApplicationModel* m_pSetupTargetApplicationModel;    ///< The model associated with this view
    bool m_traceInProgress;                                         ///< trace in progress indicator
};

#endif // _SETUP_TARGET_APPLICATION_VIEW_H_
