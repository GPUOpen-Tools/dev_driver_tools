//=============================================================================
/// Copyright (c) 2017-2018 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A helper structure used to track info for each halted processes the panel has seen.
//=============================================================================

#include "ProcessInfoModel.h"
#include "../RDPDefinitions.h"

/// constructors for ClientStatus struct
ClientStatus::ClientStatus() :
    m_clientID(0),
    m_connected(false),
    m_isProfilingEnabled(false)
{
}

ClientStatus::ClientStatus(DevDriver::ClientId clientId, bool connected, bool isProfilingEnabled) :
    m_clientID(clientId),
    m_connected(connected),
    m_isProfilingEnabled(isProfilingEnabled)
{
}

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
/// Retrieve the most recent Client Id that an application used to communicate
/// with RDP.
/// \param getConnectedOnly If true, just get the most recent Client Id that is
/// still connected and ignore disconnected client Id's
/// \returns The most recent ClientId that an application used to communicate with RDP.
//-----------------------------------------------------------------------------
DevDriver::ClientId ProcessInfoModel::GetMostRecentClientId(const bool getConnectedOnly) const
{
    // The ProcessInfo must have at least one ClientId.
    bool noClientIds = m_clientIds.empty();
    Q_ASSERT(noClientIds == false);

    if (noClientIds == false)
    {
        // first try and return a connected client
        for (auto client = m_clientIds.rbegin(); client != m_clientIds.rend(); ++client)
        {
            if (client->m_connected == true)
            {
                return client->m_clientID;
            }
        }

        if (getConnectedOnly == false)
        {
            // no connected clients, so just return the last one
            const DevDriver::ClientId lastClientId = m_clientIds.last().m_clientID;
            return lastClientId;
        }
    }

    return 0;
}

//-----------------------------------------------------------------------------
/// Has RDP already seen the given ClientId in a previous halted message?
/// \param clientId The clientId given to RDP in a halted message.
/// \returns True if RDP has already seen the given ClientId in a halted message, and false if it hasn't.
//-----------------------------------------------------------------------------
bool ProcessInfoModel::HasSeenClientId(const DevDriver::ClientId clientId) const
{
    const ClientIdVector& clientIdVector = m_clientIds;
    Q_ASSERT(clientIdVector.empty() != true);

    for (auto client = m_clientIds.begin(); client != m_clientIds.end(); ++client)
    {
        if (client->m_clientID == clientId)
        {
            return true;
        }
    }
    return false;
}

//-----------------------------------------------------------------------------
/// Update the ClientId that the process is using to communicate with RDP.
/// \param newClientId The new clientId given to RDP in a halted message.
//-----------------------------------------------------------------------------
void ProcessInfoModel::UpdateClientId(DevDriver::ClientId newClientId)
{
    ClientStatus clientStatus(newClientId, true, false);
    m_clientIds.push_back(clientStatus);
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

//-----------------------------------------------------------------------------
/// Get the connected status for all clients. This is used to indicate if the
/// capture button should be active.
/// \return true if at least one client is connected, false if all clients
/// are disconnected
//-----------------------------------------------------------------------------
bool ProcessInfoModel::GetConnectedStatus() const
{
    for (auto client = m_clientIds.begin(); client != m_clientIds.end(); ++client)
    {
        if (client->m_connected == true)
        {
            return true;
        }
    }
    return false;
}

//-----------------------------------------------------------------------------
/// Set the connected status for all clients. Typically called at shutdown to
/// disconnect all clients.
/// \param isConnected The new connection status to apply to all clients
//-----------------------------------------------------------------------------
void ProcessInfoModel::SetConnectedStatus(const bool isConnected)
{
    for (auto client = m_clientIds.begin(); client != m_clientIds.end(); ++client)
    {
        client->m_connected = isConnected;
    }
}

//-----------------------------------------------------------------------------
/// Set the connected status for a particular client.
/// \param clientId The client whose status is to be modified
/// \param isConnected The new connection status to apply to the selected client
//-----------------------------------------------------------------------------
void ProcessInfoModel::SetConnectedStatus(const DevDriver::ClientId clientId, const bool isConnected)
{
    for (auto client = m_clientIds.begin(); client != m_clientIds.end(); ++client)
    {
        if (client->m_clientID == clientId)
        {
            client->m_connected = isConnected;
        }
    }
}

//-----------------------------------------------------------------------------
/// Set the profiling status for this process. Will set the same profiling
/// status for all clients in the process.
/// \param isProfilingEnabled A flag indicating if profiling is enabled or not.
//-----------------------------------------------------------------------------
void ProcessInfoModel::SetProfilingStatus(const bool isProfilingEnabled)
{
    for (auto client = m_clientIds.begin(); client != m_clientIds.end(); ++client)
    {
        client->m_isProfilingEnabled = isProfilingEnabled;
    }
}

//-----------------------------------------------------------------------------
/// Set the profiling status for this process only for the specified client Id.
/// \param clientId The client Id whose profiling status is to be modified.
/// \param isProfilingEnabled A flag indicating if profiling is enabled or not.
//-----------------------------------------------------------------------------
void ProcessInfoModel::SetProfilingStatus(const DevDriver::ClientId clientId, const bool isProfilingEnabled)
{
    for (auto client = m_clientIds.begin(); client != m_clientIds.end(); ++client)
    {
        if (client->m_clientID == clientId)
        {
            client->m_isProfilingEnabled = isProfilingEnabled;
        }
    }
}

//-----------------------------------------------------------------------------
/// Get the profiling status for this process.
/// \return If any of the client ID's have profiling enabled, return true,
/// otherwise return false.
//-----------------------------------------------------------------------------
bool ProcessInfoModel::GetProfilingStatus() const
{
    for (auto client = m_clientIds.begin(); client != m_clientIds.end(); ++client)
    {
        if (client->m_isProfilingEnabled)
        {
            return true;
        }
    }
    return false;
}
