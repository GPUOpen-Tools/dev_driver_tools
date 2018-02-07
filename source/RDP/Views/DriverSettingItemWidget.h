//=============================================================================
/// Copyright (c) 2016-2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Widget to represent an editable settings item in the DriverSettingsView
//=============================================================================

#ifndef _DRIVER_SETTING_ITEM_WIDGET_H_
#define _DRIVER_SETTING_ITEM_WIDGET_H_

#include <QObject>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>
#include <QWidget>
#include "../../DevDriverComponents/inc/protocols/settingsClient.h"

/// Driver setting item
class DriverSettingItemWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DriverSettingItemWidget(const QString& categoryName, const DevDriver::SettingsProtocol::Setting& setting, QWidget* pParent = nullptr);
    virtual ~DriverSettingItemWidget();

private:
    QWidget* CreateEditWidget();
    void SetViewFromModel();
    void SetModelFromView();
    void FixNewlineCharacters(const QString& input, QString& output);

    // Reimplemented qt event handlers
    virtual void enterEvent(QEvent* pEvent);
    virtual void leaveEvent(QEvent* pEvent);

    DevDriver::SettingsProtocol::Setting m_setting;     ///< Driver setting this widget displays
    QLabel* m_pTitleLabel;                              ///< Setting title
    QLabel* m_pDescriptionLabel;                        ///< Setting description
    QWidget* m_pEditWidget;                             ///< Widget used for editing the setting
    QCheckBox* m_pShowFullDescriptionButton;            ///< Button used to show the full description text
    QPushButton* m_pRestoreDefaultSettingButton;        ///< Button used to restore the default driver setting

    QString m_categoryName;                             ///< Driver setting category name
    QString m_descriptionTextFirstLine;                 ///< First line of description text
    QString m_descriptionTextFull;                      ///< Full description text (with newline characters fixed)

    /// Description button state values
    enum DescriptionButtonState
    {
        STATE_HIDE_DESCRIPTION,
        STATE_SHOW_DESCRIPTION
    };

signals:
    void SettingChanged(const QString& categoryName, const DevDriver::SettingsProtocol::Setting& newSetting);

private slots:
    void OnSettingEdited();
    void OnFullDescriptionButtonPressed(int state);
    void OnRestoreDefaultButtonPressed();
};

#endif // _DRIVER_SETTING_ITEM_WIDGET_H_
