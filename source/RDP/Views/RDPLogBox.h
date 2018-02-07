//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Header of a custom plain text box
//=============================================================================

#ifndef _RDP_LOG_BOX_H_
#define _RDP_LOG_BOX_H_

#include <QPlainTextEdit>

/// Class to implement custom plain text edit box for log tab that will ignore certain mouse wheel events
class RDPLogBox : public QPlainTextEdit
{
    Q_OBJECT;

public:
    explicit RDPLogBox(QWidget* pParent = nullptr);
    virtual ~RDPLogBox() {};

    // Reimplement mouse wheel event handler
    virtual void wheelEvent(QWheelEvent* pEvent);
};

#endif // _RDP_LOG_BOX_H_

