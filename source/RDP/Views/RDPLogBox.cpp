//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Implementation of a custom plain text box to ignore wheel + control
///        key events.
//=============================================================================

#include <QWheelEvent>

#include "RDPLogBox.h"

//-----------------------------------------------------------------------------
/// Explicit constructor
//-----------------------------------------------------------------------------
RDPLogBox::RDPLogBox(QWidget* pParent) :
    QPlainTextEdit(pParent)
{
}

//-----------------------------------------------------------------------------
/// Ignore any wheel + ctrl key event
/// \param pEvent The pointer to the QWheelEvent object
//-----------------------------------------------------------------------------
void RDPLogBox::wheelEvent(QWheelEvent* pEvent)
{
    // If the event is not mouse wheel with control key, pass it on to parent
    if (!(pEvent->modifiers() & Qt::ControlModifier))
    {
        // Pass this event onto the parent
        // This is needed so the user can still scroll using the mouse wheel
        QPlainTextEdit::wheelEvent(pEvent);
    }
}
