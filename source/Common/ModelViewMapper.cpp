//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  ModelViewMapper class implementation.
//=============================================================================

// frontend headers
#include <QStandardItemModel>
#include <QDataWidgetMapper>

#include "ModelViewMapper.h"

//-----------------------------------------------------------------------------
/// Explicit constructor
/// \param modelCount the number of models required. Each model is referenced
///        by an ID
//-----------------------------------------------------------------------------
ModelViewMapper::ModelViewMapper(uint32_t modelCount)
    : m_modelCount(modelCount)
{
    m_ppControlModel = new QStandardItemModel*[modelCount];
    m_ppControlMapper = new QDataWidgetMapper*[modelCount];
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
ModelViewMapper::~ModelViewMapper()
{
    for (uint32_t loop = 0; loop < m_modelCount; loop++)
    {
        delete m_ppControlModel[loop];
        delete m_ppControlMapper[loop];
    }

    delete[] m_ppControlModel;
    delete[] m_ppControlMapper;
}

//-----------------------------------------------------------------------------
/// Initialize a model corresponding to a IO control property. Allows model/view
/// to work on any UI component
/// \param pWidget the UI widget to use.
/// \param index the ID of the model so it can be referenced internally.
/// \param propertyName the name of the widget property that the model will
///        be mapped to.
//-----------------------------------------------------------------------------
void ModelViewMapper::InitializeModel(QWidget* pWidget, uint32_t id, const QString& propertyName)
{
    m_ppControlModel[id] = new QStandardItemModel(1, 1);
    QString defaultValue = "{0}";
    if (propertyName.compare("styleSheet") == 0)
    {
        defaultValue = "";
    }
    QStandardItem* item1 = new QStandardItem(defaultValue);
    m_ppControlModel[id]->setItem(0, 0, item1);

    // Add a mapping between a QLabel and a section from the model by using QDataWidgetMapper.
    m_ppControlMapper[id] = new QDataWidgetMapper();
    m_ppControlMapper[id]->setModel(m_ppControlModel[id]);
    m_ppControlMapper[id]->addMapping(pWidget, 0, propertyName.toUtf8());
    m_ppControlMapper[id]->toFirst();
}

//-----------------------------------------------------------------------------
///
//-----------------------------------------------------------------------------
void ModelViewMapper::SetModelData(int id, const QVariant& data)
{
    m_ppControlModel[id]->setData(m_ppControlModel[id]->index(0, 0), data);
}
