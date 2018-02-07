//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  ModelViewMapper class declaration.
//=============================================================================

#ifndef _MODEL_VIEW_MAPPER_H_
#define _MODEL_VIEW_MAPPER_H_

#include <memory>
#include <QObject>

class QStandardItemModel;
class QDataWidgetMapper;

class ModelViewMapper : public QObject
{
    Q_OBJECT
public:
    explicit ModelViewMapper(uint32_t modelCount);
    virtual ~ModelViewMapper();
    void InitializeModel(QWidget* pWidget, uint32_t id, const QString& propertyName);

protected:
    void SetModelData(int id, const QVariant& data);

    QStandardItemModel**                m_ppControlModel;        ///< model associated with Index count control
    QDataWidgetMapper**                 m_ppControlMapper;       ///< the mapper to map a control to a model
    uint32_t                            m_modelCount;           ///< The number of models needed, 1 per widget property
};

#endif // _MODEL_VIEW_MAPPER_H_
