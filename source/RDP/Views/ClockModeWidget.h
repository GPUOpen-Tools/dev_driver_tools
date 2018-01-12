//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A widget used to display the frequencies for a given clock mode.
//=============================================================================

#ifndef _CLOCK_MODE_WIDGET_H_
#define _CLOCK_MODE_WIDGET_H_

#include <QCheckBox>
#include <QWidget>

class DeviceClockModeModel;
namespace Ui {
class ClockModeWidget;
}

/// A widget used to display clock frequencies.
class ClockModeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ClockModeWidget(DeviceClockModeModel* pClockModeModel, bool usesStableClocks, QWidget* pParent = nullptr);
    virtual ~ClockModeWidget();

    QCheckBox* GetClockButton() const;
    void SetModeImage(const QString& imagePrefix);

private:
    Ui::ClockModeWidget* ui;                    ///< The widget interface.
    DeviceClockModeModel* m_pClockModeModel;    ///< The model containing description and clock frequencies.
};

#endif // _CLOCK_MODE_WIDGET_H_
