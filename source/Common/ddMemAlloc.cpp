//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of ddMemAlloc which holds device driver memory
/// management utility functions.
//=============================================================================

#include "ddMemAlloc.h"
#include "../../DevDriverComponents/inc/ddPlatform.h"

//-----------------------------------------------------------------------------
/// A custom allocator used when creating new DevDriver clients,.
/// \param pUserdata A userdata pointer provided with the allocation function callbacks.
/// \param size The requested size for the allocation.
/// \param alignment The requested alignment for the allocation.
/// \param zero A switch that allows the allocated memory to be zeroed.
/// \returns A pointer to the newly allocated memory.
//-----------------------------------------------------------------------------
void* ddMemAlloc::GenericAlloc(void* pUserdata, size_t size, size_t alignment, bool zero)
{
    DD_UNUSED(pUserdata);
    return DevDriver::Platform::AllocateMemory(size, alignment, zero);
}

//-----------------------------------------------------------------------------
/// The custom allocator's corresponding free.
/// \param pUserdata A userdata pointer provided with the request to free the allocation.
/// \param pMemory The memory address corresponding to the allocation being freed.
//-----------------------------------------------------------------------------
void ddMemAlloc::GenericFree(void* pUserdata, void* pMemory)
{
    DD_UNUSED(pUserdata);
    DevDriver::Platform::FreeMemory(pMemory);
}
