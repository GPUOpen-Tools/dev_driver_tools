//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A background worker responsible for reading driver log messages.
//=============================================================================

#ifndef _DRIVER_MESSAGE_PROCESSOR_THREAD_H_
#define _DRIVER_MESSAGE_PROCESSOR_THREAD_H_

#include <QObject>

struct ChannelContext;
class DeveloperPanelModel;

/// A background processor thread responsible for processing incoming messages.
class DriverMessageProcessorThread : public QObject
{
    Q_OBJECT
public:
    explicit DriverMessageProcessorThread(ChannelContext* pChannelContext, DeveloperPanelModel* pPanelModel);
    virtual ~DriverMessageProcessorThread();

public slots:
    void StartMessageProcessingLoop();
    void ThreadFinished();

private:
    ChannelContext* m_pContext;                     ///< The ChannelContext used to send messages.
    DeveloperPanelModel* m_pDeveloperPanelModel;    ///< The main Developer Panel Model.
};

#endif // _DRIVER_MESSAGE_PROCESSOR_THREAD_H_
