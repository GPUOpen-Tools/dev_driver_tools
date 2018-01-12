//=============================================================================
/// Copyright (c) 2016-2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  The interface used to collect RGP traces and view recent traces.
//=============================================================================

#ifndef _RGP_TRACE_VIEW_H_
#define _RGP_TRACE_VIEW_H_

#include "CaptureProgressWidget.h"
#include "../Models/ProcessInfoModel.h"
#include "../Models/RGPTraceModel.h"

class ApplicationSettingsModel;
class DeveloperPanelModel;
class RGPRecentTraceListModel;

namespace Ui {
class RGPTraceView;
}

/// The interface used to collect RGP traces and view recent traces.
class RGPTraceView : public QWidget
{
    Q_OBJECT
    enum PathValidationMode
    {
        ValidateDirectory,
        ValidateFile,
        ValidateExecutable
    };

public:
    explicit RGPTraceView(DeveloperPanelModel* pPanelModel, ApplicationSettingsModel* pApplicationSettingsModel, QWidget* pParent = nullptr);
    virtual ~RGPTraceView();

    virtual void OnClientIdUpdated(DevDriver::ClientId clientId);
    bool ValidateFilenamePath(const QString& filePathString, PathValidationMode mode);
    void UpdateRgpExecutablePathStatus(const QString& filenamePathString);
    void UpdateTraceDirectoryStatus(const QString& pathString);

public slots:
    void OnProfilingTargetUpdated(const ProcessInfoModel& processInfoModel);

private slots:
    void OnBrowseTraceDirectoryClicked(bool checked);
    void OnBrowseToRGPButtonClicked(bool checked);
    void OnCollectTraceClicked(bool checked);
    void OnOpenInRGPClicked(bool checked);
    void OnRecentTraceDoubleClicked(const QModelIndex& index);
    void OnTraceDirectoryTextboxChanged(const QString& text);
    void OnRGPFilepathTextboxChanged(const QString& text);
    void OnShowRecentTracesContextMenu(const QPoint& pos);
    void OnCollectDetailedTraceDataChanged(int checkState);
    void OnAllowComputePresentsChanged(int checkState);
    void OnTraceAdded(const QModelIndex &parent, int start, int end);
    void OnTraceCollectionStatusUpdated(bool traceIsBeingCollected);
    void OnUpdateCollectRGPTraceButton(bool enable);
    void OnRDSDisconnect();
    void OnHotKeyPressed(uint key);

signals:
    void ProgressUpdated(DevDriver::uint64 recievedBytes, DevDriver::uint64 totalBytes);

private:
    bool DoesRecentTraceExistOnDisk(int recentTraceRow, RgpTraceFileInfo& traceFileInfo);
    void ShowProgressWidget();
    void HideProgressWidget();
    bool OpenRecentTraceAtModelIndex(const QModelIndex& recentTraceIndex);
    void UpdateTraceCollectionControls();
    void AdjustTableColumns();
    void RemoveRecentTraceRow(int rowIndex);
    void SelectRecentTraceRow(int row);
    bool IsDirectoryWritable(const QString& updatedTraceDirectory) const;
    void ShowDirectoryNotWritableNotification(const QString& traceDirectory) const;

    Ui::RGPTraceView *ui;
    RGPTraceModel* m_pRGPTraceModel;                        ///< The model used to track RGP trace capture settings.
    ApplicationSettingsModel* m_pApplicationSettingsModel;  ///< The settings model being altered by this interface.
    CaptureProgressWidget* m_pProgressWidget;               ///< A widget showing the progress of the trace being captured.
    bool m_targetApplicationIsProfilable;                   ///< A flag that determines if the connected app is enabled for profiling.
    bool m_traceInProgress;                                 ///< A flag that determines if an RGP trace is currently being collected.
};

#endif // _RGP_TRACE_VIEW_H_
