//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for ddMemAlloc which holds device driver memory
/// management utility functions.
//=============================================================================

#ifndef _DDMEMALLOC_H_
#define _DDMEMALLOC_H_

#include <stddef.h>
#include "../DevDriverComponents/inc/gpuopen.h"
namespace ddMemAlloc
{
    void* GenericAlloc(void* pUserdata, size_t size, size_t alignment, bool zero);
    void GenericFree(void* pUserdata, void* pMemory);
}
#endif //_DDMEMALLOC_H_
