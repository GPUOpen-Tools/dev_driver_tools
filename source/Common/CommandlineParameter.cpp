//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of a commandline parameter definition and parser.
///  Used by the CommandlineParser class.
//=============================================================================

#include "CommandlineParameter.h"
#include <assert.h>

#ifndef WIN32
#include <errno.h>
#endif // WIN32

static const unsigned short s_INT_16_MAX_VALUE = 65535;         ///< The highest value of a 2 byte integer.

//-----------------------------------------------------------------------------
/// CommandlineParameter class constructor
/// \param pName The name of the parameter, including any prefix characters (e.g. "--" or "/").
/// \param pDescription Help information for the parameter.
/// \param required True indicates that the parameter must be specified on the command line
/// otherwise an error is reported.  False indicates that the parameter is optional.  The default
/// is false.
/// \param flagParameter True indicates that the parameter has no argument associated with it.
/// False indicates that an argument must follow this parameter on the commandline.  The default
/// is false.
/// \param pDefaultValue A string value that specifies the default value for optional parameters
/// that require an argument.  The default is a null string.
//-----------------------------------------------------------------------------
CommandlineParameter::CommandlineParameter(const char* pName, const char* pDescription, bool required, bool flagParameter, const char* pDefaultValue) :
    m_isParsed(false),
    m_isParameterPresent(false),
    m_isValuePresent(false),
    m_isValid(false),
    m_isFlag(flagParameter),
    m_isRequired(required)
{
    assert(pName != nullptr);
    m_name = pName;

    if (pDescription != nullptr)
    {
        m_description = pDescription;
    }

    if (pDefaultValue != nullptr)
    {
        m_value = pDefaultValue;
    }
}

//-----------------------------------------------------------------------------
/// Parses a command line parameter and string value.
/// \param value The argument following the parameter on the commandline.
/// \return True if the value string is not blank or if the parameter is a flag.
/// Otherwise returns false.  Override this method for parsing specific argument types.
//-----------------------------------------------------------------------------
bool CommandlineParameter::Parse(const std::string& value)
{
    m_isParameterPresent = true;
    m_isParsed = true;

    if (IsFlag())
    {
        m_isValid = true;
    }
    else if (!value.empty())
    {
        m_isValuePresent = true;
        m_value = value;
        m_isValid = true;
    }
    return m_isValid;
}

//-----------------------------------------------------------------------------
/// Returns the value associated with the parameter (either the default value or
/// the parsed value).
/// \return The value string for the parameter.
//-----------------------------------------------------------------------------
const std::string& CommandlineParameter::Value() const
{
    return m_value;
}

//-----------------------------------------------------------------------------
/// Indicates that the parameter is present.  The Parse() method must be
/// called before retrieving this state.
/// \return The value of the m_isParameterPresent member variable.
//-----------------------------------------------------------------------------
bool CommandlineParameter::IsParameterPresent() const
{
    return m_isParameterPresent;
}

//-----------------------------------------------------------------------------
/// Indicates that the value is present.  The Parse() method must be
/// called before retrieving this state.
/// \return The value of the m_isValuePresent member variable.
//-----------------------------------------------------------------------------
bool CommandlineParameter::IsValuePresent() const
{
    return m_isValuePresent;
}

//-----------------------------------------------------------------------------
/// Indicates that the parameter value is valid.  The Parse() method must be
/// called before retrieving this state.
/// \return The value of the m_isValid member variable.
//-----------------------------------------------------------------------------
bool CommandlineParameter::IsValid() const
{
    return m_isValid;
}

//-----------------------------------------------------------------------------
/// Indicates that the parameter and value have been parsed.
/// \return The value of the m_isParsed member variable.
//-----------------------------------------------------------------------------
bool CommandlineParameter::IsParsed() const
{
    return m_isParsed;
}

//-----------------------------------------------------------------------------
/// Indicates that the parameter doesn't have an argument associated with it.
/// \return The value of the m_isFlag member variable.
//-----------------------------------------------------------------------------
bool CommandlineParameter::IsFlag() const
{
    return m_isFlag;
}

//-----------------------------------------------------------------------------
/// Indicates that the parameter is required to be specified on the commandline.
/// \return The value of the m_isRequired member variable.
//-----------------------------------------------------------------------------
bool CommandlineParameter::IsRequired() const
{
    return m_isRequired;
}

//-----------------------------------------------------------------------------
/// Retrieves the description of the parameter for display in a help message.
/// \return The value of the m_description member variable.
//-----------------------------------------------------------------------------
const std::string& CommandlineParameter::Description() const
{
    return m_description;
}

//-----------------------------------------------------------------------------
/// Retrieves the name of the commaline parameter.
/// \return The value of the m_name member variable.
//-----------------------------------------------------------------------------
const std::string& CommandlineParameter::Name() const
{
    return m_name;
}

//-----------------------------------------------------------------------------
/// Int16CommandlineParameter class constructor -- a 16 bit integer, derived from
/// the base CommandlineParameter class.
/// \param pName The name of the parameter, including any prefix characters (e.g. "--" or "/").
/// \param pDescription Help information for the parameter.
/// \param required True indicates that the parameter must be specified on the command line
/// otherwise an error is reported.  False indicates that the parameter is optional.  The default
/// is false.
/// \param defaultValue A 16 bit integer value that specifies the default value for optional parameters
/// The default value is 0.
//-----------------------------------------------------------------------------
Int16CommandlineParameter::Int16CommandlineParameter(const char* pName, const char* pDescription, bool required, int defaultValue) :
    CommandlineParameter(pName, pDescription, false, required),
    m_intValue(defaultValue)
{
}

//-----------------------------------------------------------------------------
/// Parses a command line parameter and string value.  The string value is converted
/// to a 16 bit integer.
/// \param value The 16 bit integer argument following the parameter on the commandline.
/// \return True if the value string is a valid 16 bit integer.  Otherwise returns false.
//-----------------------------------------------------------------------------
bool Int16CommandlineParameter::Parse(const std::string& value)
{
    m_isParameterPresent = true;
    m_isParsed = true;
    if (value.empty())
    {
        m_isValuePresent = false;
        m_isValid = false;
    }
    else
    {
        m_isValuePresent = true;
        const char* pValueEnd = value.c_str() + value.length();
        char* pStringEnd = nullptr;

        // Convert the string to a base 10 integer value.
        long commandlinePort = strtol(value.c_str(), &pStringEnd, 10);
        if ((errno != ERANGE) && (pStringEnd == pValueEnd) && (commandlinePort >= 1) && (commandlinePort <= s_INT_16_MAX_VALUE))
        {
            m_intValue = (int)commandlinePort;
            m_isValid = true;
        }
    }
    return m_isValid;
}

//-----------------------------------------------------------------------------
/// The parsed 16 bit integer value associated with the parameter.
/// \return The value of the m_intValue member variable.
//-----------------------------------------------------------------------------
int Int16CommandlineParameter::ValueAsInt() const
{
    return m_intValue;
}
