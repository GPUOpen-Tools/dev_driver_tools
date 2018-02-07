//=============================================================================
/// Copyright (c) 2016-2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  An interface used to view and alter driver settings.
//=============================================================================

#include <QTextStream>
#include <QFileDialog>
#include "ui_DriverSettingsView.h"
#include "DriverSettingItemWidget.h"
#include "DriverSettingsView.h"
#include "../RDPDefinitions.h"
#include "../Models/ApplicationSettingsModel.h"
#include "../Models/DeveloperPanelModel.h"
#include "../Models/DriverSettingsModel.h"
#include "../Util/RDPUtil.h"
#include "../Settings/RDPSettings.h"
#include <ScalingManager.h>

const static QString s_SEARCH_CATEGORY_NAME("Search Results");

//-----------------------------------------------------------------------------
/// DriverSettingsView constructor.
/// \param pPanelModel The main Panel model used to maintain a connection to RDS.
/// \param pApplicationSettingsModel The model used to store settings for a single application.
/// \param pParent The parent widget for the Driver Settings View.
//-----------------------------------------------------------------------------
DriverSettingsView::DriverSettingsView(DeveloperPanelModel* pPanelModel, ApplicationSettingsModel* pApplicationSettingsModel, QWidget* pParent)
    : QWidget(pParent)
    , ui(new Ui::DriverSettingsView)
    , m_pApplicationSettingsModel(pApplicationSettingsModel)
    , m_searchActive(false)
{
    ui->setupUi(this);

    // Set white background for this pane
    ToolUtil::SetWidgetBackgroundColor(this, Qt::white);

    m_pDriverSettingsModel = pApplicationSettingsModel->GetDriverSettingsModel();
    pPanelModel->RegisterProtocolModel(MAIN_PANEL_MODEL_DRIVER_SETTINGS, m_pDriverSettingsModel);

    m_pEmptyDriverSettingsView = new EmptyDriverSettingsView();
    layout()->addWidget(m_pEmptyDriverSettingsView);

    if (m_pDriverSettingsModel != nullptr)
    {
        m_pDriverSettingsModel->InitializeDefaults();

        connect(ui->defaultAllButton, &QPushButton::pressed, this, &DriverSettingsView::OnDefaultAllButtonPressed);
        connect(ui->exportButton, &QPushButton::pressed, this, &DriverSettingsView::OnExportButtonPressed);
        connect(ui->importButton, &QPushButton::pressed, this, &DriverSettingsView::OnImportButtonPressed);
        connect(m_pEmptyDriverSettingsView, &EmptyDriverSettingsView::importButtonPressed, this, &DriverSettingsView::OnImportButtonPressed);
        connect(ui->searchTextbox, &QLineEdit::textChanged, this, &DriverSettingsView::OnSearchTextChanged);
        connect(ui->listWidget, &QListWidget::itemSelectionChanged, this, &DriverSettingsView::OnCategorySelected);

        // The settings file may already be populated at this point. Attempt to populate the interface.
        PopulateSettingsInterface();
    }
    else
    {
        RDPUtil::DbgMsg("[RDP] Failed to initialize the Driver Settings model.");
    }
}

//-----------------------------------------------------------------------------
/// Destructor for the DriverSettingsView interface.
//-----------------------------------------------------------------------------
DriverSettingsView::~DriverSettingsView()
{
    SAFE_DELETE(m_pDriverSettingsModel);

    delete ui;
}

//-----------------------------------------------------------------------------
/// Handler invoked when one of the setting item widgets indicates the setting
/// has changed.
/// \param setting The edited driver setting.
//-----------------------------------------------------------------------------
void DriverSettingsView::OnSettingItemChanged(const QString& categoryName, const DevDriver::SettingsProtocol::Setting& setting)
{
    // Update driver setting
    m_pDriverSettingsModel->UpdateDriverSetting(categoryName, setting);
}

//-----------------------------------------------------------------------------
/// Update the connected Client Id for any models that are connected to clients.
/// \param clientId The client Id for the client.
//-----------------------------------------------------------------------------
void DriverSettingsView::OnClientIdUpdated(DevDriver::ClientId clientId)
{
    m_pDriverSettingsModel->SetConnectedClientId(clientId);
}

//-----------------------------------------------------------------------------
/// Hanlder invoked when the deault all button is pressed.
//-----------------------------------------------------------------------------
void DriverSettingsView::OnDefaultAllButtonPressed()
{
    // Put up a message box to verify action

    NotificationWidget::Button resultButton = RDPUtil::ShowNotification(
        gs_SETTING_DEFAULT_ALL_CONFIRMATION_DIALOG_TITLE,
        gs_SETTING_DEFAULT_ALL_CONFIRMATION_DIALOG_TEXT,
        NotificationWidget::Button::Yes | NotificationWidget::Button::No);

    if (resultButton == NotificationWidget::Button::Yes)
    {
        // Restore defaults
        m_pApplicationSettingsModel->GetSettingsFile()->RestoreToDefaultSettings();

        // Update model data
        DriverSettingsMap settingsMap = m_pApplicationSettingsModel->GetSettingsFile()->GetDriverSettings();
        m_pDriverSettingsModel->UpdateDriverSettings(settingsMap);

        // Redraw view widgets
        AddSettingItemWidgets();
    }
}

//-----------------------------------------------------------------------------
/// Handler invoked when export button is pressed.
//-----------------------------------------------------------------------------
void DriverSettingsView::OnExportButtonPressed()
{
    // Get the user to select a directory to export to
    QString filename = QFileDialog::getSaveFileName(this, "Export Settings File", "./untitledsettings.rds", "RDS Settings Files (*.rds)");

    // If the user cancels, do nothing
    if (filename.isEmpty() == false)
    {
        // Create a new driver setting file
        RDPApplicationSettingsFile* pFileInfo = new RDPApplicationSettingsFile();
        pFileInfo->filepath = filename;
        ApplicationSettingsFile* pExportFile = new ApplicationSettingsFile();
        pExportFile->SetFileInfo(pFileInfo);

        // Copy active settings into the new file
        pExportFile->CopyFrom(m_pApplicationSettingsModel->GetSettingsFile());

        // Save the new file
        RDPSettings::Get().WriteApplicationSettingsFile(pExportFile);
    }
}

//-----------------------------------------------------------------------------
/// Handler invoked when import button is pressed.
//-----------------------------------------------------------------------------
void DriverSettingsView::OnImportButtonPressed()
{
    // Get the user to select a file to import
    QString filename = QFileDialog::getOpenFileName(this, "Choose Application Settings File", "./", "RDS Settings Files (*.rds)");
    bool fileExists = ToolUtil::CheckFilepathExists(filename);

    // Only use valid file paths
    if (fileExists)
    {
        RDPApplicationSettingsFile fileInfo;
        fileInfo.filepath = filename;
        ApplicationSettingsFile* pImportFile = RDPSettings::Get().ReadApplicationSettingsFile(&fileInfo);

        // Do nothing if the read failed - invalid file data
        if (pImportFile != nullptr)
        {
            m_pApplicationSettingsModel->GetSettingsFile()->CopyFrom(pImportFile);
            PopulateSettingsInterface();
        }

        // Save imported changes
        RDPSettings::Get().WriteApplicationSettingsFile(m_pApplicationSettingsModel->GetSettingsFile());
    }
}

//-----------------------------------------------------------------------------
/// Handler invoked when the search text is changed.
/// \param text The new string to search for.
//-----------------------------------------------------------------------------
void DriverSettingsView::OnSearchTextChanged(const QString &text)
{
    if (m_pDriverSettingsModel != nullptr)
    {
        // Check if there is any text in the searchbox
        bool emptySearchText = text.isEmpty();

        // Search text found when not in search state, switch to search state
        if (emptySearchText == false && m_searchActive == false)
        {
            // Remember selected category prior to search
            m_preSearchCategoryIndex = ui->listWidget->selectionModel()->currentIndex().row();

            // Hide all categories
            for (int i = 0; i < ui->listWidget->count(); i++)
            {
                ui->listWidget->setRowHidden(i, true);
            }

            // Show and select the search category
            ui->listWidget->setRowHidden(ui->listWidget->count() - 1, false);
            SelectCategoryAtIndex(ui->listWidget->count() - 1);

            m_searchActive = true;
        }

        // No search text found while in search state, switch to no search state
        if (emptySearchText == true && m_searchActive == true)
        {
            // Show all other categories
            for (int i = 0; i < ui->listWidget->count(); i++)
            {
                ui->listWidget->setRowHidden(i, false);
            }

            // Hide the search category
            ui->listWidget->setRowHidden(ui->listWidget->count() - 1, true);

            // Restore category selection from before search
            SelectCategoryAtIndex(m_preSearchCategoryIndex);

            m_searchActive = false;
        }

        // Repopulate setting widgets
        AddSettingItemWidgets();
    }
}

//-----------------------------------------------------------------------------
/// Hanlder invoked when a category is selected from the list.
//-----------------------------------------------------------------------------
void DriverSettingsView::OnCategorySelected()
{
    AddSettingItemWidgets();
}

//-----------------------------------------------------------------------------
/// Handler invoked when a client has been connected and requests a settings
/// interface update.
//-----------------------------------------------------------------------------
void DriverSettingsView::PopulateSettingsInterface()
{
    if (m_pDriverSettingsModel != nullptr)
    {
        const DriverSettingsMap& settingsMap = m_pDriverSettingsModel->GetSettingsMap();

        // If there are no settings, show the empty setting pane instead
        if (settingsMap.empty())
        {
            m_pEmptyDriverSettingsView->show();
            ui->populatedSettingsView->hide();
        }
        else
        {
            ui->populatedSettingsView->show();
            m_pEmptyDriverSettingsView->hide();
        }

        // Add category items to the list
        AddCategoryListItems();

        // Add widgets for the setting items
        AddSettingItemWidgets();
    }
}

//-----------------------------------------------------------------------------
/// Select the category at the given index.
/// \param row The row index of the category to select.
//-----------------------------------------------------------------------------
void DriverSettingsView::SelectCategoryAtIndex(int row)
{
    QItemSelectionModel* pSelectionModel = ui->listWidget->selectionModel();
    QAbstractItemModel* pDataModel = ui->listWidget->model();

    // Verify there is at least one column
    if (pDataModel->columnCount() > 0)
    {
        QItemSelection selection(pDataModel->index(row, 0), pDataModel->index(row, 0));
        pSelectionModel->setCurrentIndex(pDataModel->index(row, 0), QItemSelectionModel::ClearAndSelect);
    }
}

//-----------------------------------------------------------------------------
/// Add the required setting item widgets to the setting layout, such that they
/// reflect the currently selected category. If searching, all setting items
/// will be added.
//-----------------------------------------------------------------------------
void DriverSettingsView::AddSettingItemWidgets()
{
    // Clear any previous setting widgets
    ClearSettingItemWidgets();

    // Get category name from selected list item
    QList<QListWidgetItem*> selectedItems = ui->listWidget->selectedItems();
    if (selectedItems.size() < 1) return;
    QString categoryName = selectedItems.at(0)->text();

    // Get settings map
    const DriverSettingsMap& settingsMap = m_pDriverSettingsModel->GetSettingsMap();

    // Check if in searching state
    if (categoryName.compare(s_SEARCH_CATEGORY_NAME) == 0)
    {
        // Searching, add items for all categories
        DriverSettingsMap::const_iterator it;
        for (it = settingsMap.begin(); it != settingsMap.end(); ++it)
        {
            categoryName = it.key();
            AddSettingItemsFromCategory(categoryName, settingsMap);
        }
    }
    else
    {
        // Not searching, add items for selected category
        AddSettingItemsFromCategory(categoryName, settingsMap);
    }

    // Add spacer item at the end
    ui->settingsViewWidget->layout()->addItem(new QSpacerItem(40, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));
}

//-----------------------------------------------------------------------------
/// Helper function for AddSettingItemWidgets which adds the setting items
/// for a specific category.
/// \param categoryName The name of the category to add items for.
/// \param settingsMap The complete driver settings map of all settings.
//-----------------------------------------------------------------------------
void DriverSettingsView::AddSettingItemsFromCategory(const QString& categoryName, const DriverSettingsMap& settingsMap)
{
    QLayout* pLayout = ui->settingsViewWidget->layout();

    // Searchbox regex used to only add items if they match the search text
    QRegExp regex(".*" + ui->searchTextbox->text() + ".*", Qt::CaseInsensitive);

    // Get array of settings for this category
    DriverSettingVector settingsArray = settingsMap[categoryName];

    for (const DevDriver::SettingsProtocol::Setting& setting : settingsArray)
    {
        // Only add settings that match the searchbox text
        if (regex.exactMatch(setting.name) || regex.exactMatch(QString(setting.description)))
        {
            DriverSettingItemWidget* pSettingWidget = new DriverSettingItemWidget(categoryName, setting, ui->settingsViewWidget);

            pLayout->addWidget(pSettingWidget);
            ScalingManager::Get().RegisterObject(pSettingWidget);

            // Setting edits are processed in the OnSettingChanged slot
            connect(pSettingWidget, &DriverSettingItemWidget::SettingChanged, this, &DriverSettingsView::OnSettingItemChanged);
        }
    }
}

//-----------------------------------------------------------------------------
/// Add the required list items to the category list widget based on the active
/// driver settings file.
//-----------------------------------------------------------------------------
void DriverSettingsView::AddCategoryListItems()
{
    const DriverSettingsMap& settingsMap = m_pDriverSettingsModel->GetSettingsMap();

    // Clear any previous items
    ClearCategoryList();

    // Add categories to list widget
    DriverSettingsMap::const_iterator it;
    for (it = settingsMap.begin(); it != settingsMap.end(); ++it)
    {
        QString categoryName = it.key();
        ui->listWidget->addItem(categoryName);
    }

    // Add and hide the "Search Results" category
    ui->listWidget->addItem(s_SEARCH_CATEGORY_NAME);
    ui->listWidget->setRowHidden(ui->listWidget->count() - 1, true);

    // Select the first list item by default
    if (!settingsMap.empty())
    {
        SelectCategoryAtIndex(0);
    }
}

//-----------------------------------------------------------------------------
/// Clear and delete all setting item widgets from the layout.
//-----------------------------------------------------------------------------
void DriverSettingsView::ClearSettingItemWidgets()
{
    QLayout* pLayout = ui->settingsViewWidget->layout();

    // Delete all layout items and widgets
    while (QLayoutItem* pItem = pLayout->takeAt(0))
    {
        QWidget* widget = pItem->widget();
        SAFE_DELETE(widget);
        SAFE_DELETE(pItem);
    }
}

//-----------------------------------------------------------------------------
/// Clear and delete all items from the category list.
//-----------------------------------------------------------------------------
void DriverSettingsView::ClearCategoryList()
{
    // Delete all category list items
    while (QListWidgetItem* pItem = ui->listWidget->takeItem(0))
    {
        SAFE_DELETE(pItem);
    }
}
