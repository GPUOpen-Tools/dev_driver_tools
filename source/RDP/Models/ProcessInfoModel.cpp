//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A helper structure used to track info for each halted processes the panel has seen.
//=============================================================================

#include "ProcessInfoModel.h"
#include "../RDPDefinitions.h"

//-----------------------------------------------------------------------------
/// Constructor for ProcessInfoModel.
/// \param name The executable filename of the new process.
/// \param description The description for the new process.
/// \param processId The process Id for the new process.
//-----------------------------------------------------------------------------
ProcessInfoModel::ProcessInfoModel(const QString& name, const QString& description, DevDriver::ProcessId processId)
    : m_processName(name)
    , m_processDescription(description)
    , m_processId(processId)
    , m_isConnected(false)
    , m_isProfilingEnabled(false)
    , m_isDriverInitialized(false)
{
}

//-----------------------------------------------------------------------------
/// Destructor for ProcessInfoModel.
//-----------------------------------------------------------------------------
ProcessInfoModel::~ProcessInfoModel()
{
}

//-----------------------------------------------------------------------------
/// Retrieve the most recent ClientId that an application used to communicate with RDP.
/// \returns The most recent ClientId that an application used to communicate with RDP.
//-----------------------------------------------------------------------------
DevDriver::ClientId ProcessInfoModel::GetMostRecentClientId() const
{
    // The ProcessInfo must have at least one ClientId.
    bool noClientIds = m_clientIds.empty();
    Q_ASSERT(!noClientIds);

    if (!noClientIds)
    {
        const DevDriver::ClientId lastClientId = m_clientIds.last();
        return lastClientId;
    }

    return 0;
}

//-----------------------------------------------------------------------------
/// Has RDP already seen the given ClientId in a previous halted message?
/// \param clientId The clientId given to RDP in a halted message.
/// \returns True if RDP has already seen the given ClientId in a halted message, and false if it hasn't.
//-----------------------------------------------------------------------------
bool ProcessInfoModel::HasSeenClientId(DevDriver::ClientId clientId) const
{
    const ClientIdVector& clientIdVector = m_clientIds;
    Q_ASSERT(clientIdVector.empty() != true);

    return clientIdVector.contains(clientId);
}

//-----------------------------------------------------------------------------
/// Update the ClientId that the process is using to communicate with RDP.
/// \param newClientId The new clientId given to RDP in a halted message.
//-----------------------------------------------------------------------------
void ProcessInfoModel::UpdateClientId(DevDriver::ClientId newClientId)
{
    m_clientIds.push_back(newClientId);
    m_isConnected = true;
}

//-----------------------------------------------------------------------------
/// Get the API string for the process by parsing the process description string.
/// \returns A string containing the rendering API being used by the process.
//-----------------------------------------------------------------------------
const QString& ProcessInfoModel::GetAPI() const
{
    const QString& processDescription = GetProcessDescription();

    // The description string can be parsed to extract the API being used by the process.
    if (processDescription.compare(gs_AMDXC64_DESCRIPTION_STRING) == 0)
    {
        return gs_AMDXC64_API_STRING;
    }
    else if (processDescription.compare(gs_AMDVLK64_DESCRIPTION_STRING) == 0)
    {
        return gs_AMDVLK64_API_STRING;
    }

    return gs_UNKNOWN_API;
}
