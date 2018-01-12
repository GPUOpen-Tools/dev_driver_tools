//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Empty driver settings placeholder ui
//=============================================================================

#ifndef _EMPTY_DRIVER_SETTINGS_VIEW_H
#define _EMPTY_DRIVER_SETTINGS_VIEW_H

#include <QWidget>

namespace Ui {
class EmptyDriverSettingsView;
}

/// Empty driver settings view panel - used to indicate abscence of cached driver settings
class EmptyDriverSettingsView : public QWidget
{
    Q_OBJECT

public:
    explicit EmptyDriverSettingsView(QWidget* pParent = nullptr);
    ~EmptyDriverSettingsView();

private:
    Ui::EmptyDriverSettingsView *ui;            ///< Qt ui component

signals:
    void importButtonPressed();

};

#endif // _EMPTY_DRIVER_SETTINGS_VIEW_H
