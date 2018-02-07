//=============================================================================
/// Copyright (c) 2016-2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  An interface used to view and alter driver settings.
//=============================================================================

#ifndef _DRIVER_SETTINGS_VIEW_H_
#define _DRIVER_SETTINGS_VIEW_H_

#include <QItemSelection>
#include <QLabel>
#include "EmptyDriverSettingsView.h"
#include "../AppSettings/ApplicationSettingsFile.h"
#include "../../DevDriverComponents/inc/protocols/settingsClient.h"

class ApplicationSettingsModel;
class DriverSettingsModel;
class DeveloperPanelModel;
class SettingTableComboBoxDelegate;

namespace Ui {
class DriverSettingsView;
}

/// The driver settings tab interface.
class DriverSettingsView : public QWidget
{
    Q_OBJECT
public:
    explicit DriverSettingsView(DeveloperPanelModel* pPanelModel, ApplicationSettingsModel* pApplicationSettingsModel, QWidget* pParent = nullptr);
    virtual ~DriverSettingsView();

    virtual void OnClientIdUpdated(DevDriver::ClientId clientId);

public:
    void PopulateSettingsInterface();
    void SelectCategoryAtIndex(int index);

private slots:
    void OnSettingItemChanged(const QString& categoryName, const DevDriver::SettingsProtocol::Setting& setting);
    void OnDefaultAllButtonPressed();
    void OnImportButtonPressed();
    void OnExportButtonPressed();
    void OnSearchTextChanged(const QString &text);
    void OnCategorySelected();

private:
    void AddSettingItemWidgets();
    void AddSettingItemsFromCategory(const QString& categoryName, const DriverSettingsMap& settingsMap);
    void AddCategoryListItems();
    void ClearSettingItemWidgets();
    void ClearCategoryList();

    Ui::DriverSettingsView *ui;                             ///< Qt ui component

    EmptyDriverSettingsView* m_pEmptyDriverSettingsView;    ///< Empty settings view shown when there are no settings

    DriverSettingsModel* m_pDriverSettingsModel;            ///< The DriverSettingsModel backend instance.
    SettingTableComboBoxDelegate* m_pSettingTableEditor;    ///< The delegate used for the settings editor dropdown.
    ApplicationSettingsModel* m_pApplicationSettingsModel;  ///< The settings model being altered by this interface.

    bool m_searchActive;                                    ///< Search active state, true if searching false otherwise
    int m_preSearchCategoryIndex;                           ///< Index of selected category prior to search
};

#endif // _DRIVER_SETTINGS_VIEW_H_
