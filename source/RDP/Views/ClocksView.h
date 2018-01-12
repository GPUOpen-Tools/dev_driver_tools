//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Implementation of clock mode UI
//=============================================================================

#ifndef _CLOCKS_VIEW_H_
#define _CLOCKS_VIEW_H_

#include "../Models/ProcessInfoModel.h"
#include <QWidget>
#include <QButtonGroup>

namespace Ui {
class ClocksView;
}

class ClocksTabModel;
class ClockModeWidget;
class DeveloperPanelModel;

/// Clock mode adjustment view
class ClocksView : public QWidget
{
    Q_OBJECT
public:
    explicit ClocksView(DeveloperPanelModel* pPanelModel, QWidget* pParent = nullptr);
    virtual ~ClocksView();

public slots:
    void OnDriverInitializedStatusUpdated(const ProcessInfoModel& processInfo, bool isInitialized);

private slots:
    void OnClockButtonPressed(int id);

private:
    void InitializeClockModeWidgets();
    void InitializeInterface();

    Ui::ClocksView *ui;                         ///< Qt UI component.
    QButtonGroup* m_pClockButtonGroup;          ///< Button group for clock mode selection buttons.
    ClocksTabModel* m_pClockTabModel;           ///< The model used to retrieve and store clock settings.
    QVector<ClockModeWidget*> m_modeWidgets;    ///< The list of widgets for each clock mode.
};

#endif // _CLOCKS_VIEW_H_
