//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of a general purpose commandline parser.
//=============================================================================

#include "CommandlineParser.h"
#ifndef WIN32
#include <strings.h>
#include <limits.h>
#endif // WIN32

//-----------------------------------------------------------------------------
/// CommandLineProcessor class constructor.
/// \param argc The number of arguments on the commandline.
/// \param argv An array of argument strings on the command line.
//-----------------------------------------------------------------------------
CommandlineParser::CommandlineParser(int argc, char* argv[]) :
    m_isHelpOptionFound(false)
{
    // Save the arguments on the commandline.  Skip the first item (the executable name).
    argc--;
    int index = 1;
    while (argc)
    {
        m_commandlineArguments.push_back(argv[index]);
        index++;
        argc--;
    }
}

//-----------------------------------------------------------------------------
/// Indicates that the help option (defined with SetHelpOption() method) is
/// present on the commandline.  The Parse() method must be called before calling
/// this method.
/// \return True if the help option is present after parsing the commandline,
/// otherwise returns false.
//-----------------------------------------------------------------------------
bool CommandlineParser::IsHelpRequested() const
{
    return m_isHelpOptionFound;
}

//-----------------------------------------------------------------------------
/// Defines the help option for the commandline.
/// \param pName The string for the commandline option used to display a help message
/// for all defined parameters.
/// \param pDescription Help information for using the help option.
//-----------------------------------------------------------------------------
void CommandlineParser::SetHelpOption(const char* pName, const char* pDescription)
{
    if (pName != nullptr)
    {
        m_helpOptionName = pName;
    }

    if (pDescription != nullptr)
    {
        m_helpOptionDescription = pDescription;
    }
}

//-----------------------------------------------------------------------------
/// Adds a commandline parameter to be parsed.
/// \param pParameter A pointer to a CommadlineParameter definition.
//-----------------------------------------------------------------------------
void CommandlineParser::AddParameter(CommandlineParameter* pParameter)
{
    m_definedParameters.push_back(pParameter);
}

//-----------------------------------------------------------------------------
/// Parses the defined parameters for a commandline.
/// \return True if all specified parameters are valid, otherwise returns false.
//-----------------------------------------------------------------------------
bool CommandlineParser::Parse()
{
    m_errorString.clear();
    bool parseSuccessful = true;
    std::string parameterName;
    std::string argument;
    for (auto cmdArgsIterator = m_commandlineArguments.begin(); cmdArgsIterator != m_commandlineArguments.end(); ++cmdArgsIterator)
    {
        if (*cmdArgsIterator == m_helpOptionName)
        {
            m_isHelpOptionFound = true;
        }
        else
        {
            CommandlineParameter* pParameterDef = MatchParameter(*cmdArgsIterator);
            if (pParameterDef != nullptr)
            {
                if (!pParameterDef->IsFlag())
                {
                    parameterName = *cmdArgsIterator;
                    ++cmdArgsIterator;
                    if (cmdArgsIterator != m_commandlineArguments.end())
                    {
                        argument = *cmdArgsIterator;
                    }
                    else
                    {
                        // end of argument list reached.
                        m_errorString += "Missing value for parameter '" + parameterName + "'.\n";
                        parseSuccessful = false;
                        break;
                    }
                }
                if (pParameterDef->IsParsed())
                {
                    m_errorString += "Parameter '" + parameterName + "' listed more than once.\n";
                    parseSuccessful = false;
                }
                else
                {

                    bool parseParameterResult = pParameterDef->Parse(argument);
                    if (!parseParameterResult)
                    {
                        m_errorString += "Invalid value '" + argument + "' for parameter '" + parameterName + "'.\n";
                    }
                    parseSuccessful &= parseParameterResult;
                }
            }
            else
            {
                m_errorString += "Invalid parameter - '" + *cmdArgsIterator + "'.\n";
                parseSuccessful = false;
            }
        }
    }

    if (parseSuccessful)
    {
        // Make sure all required parameters are present.
        for (auto parametersIterator = m_definedParameters.begin(); parametersIterator != m_definedParameters.end(); ++parametersIterator)
        {
            if ((*parametersIterator)->IsRequired() && !(*parametersIterator)->IsParsed())
            {
                m_errorString += "A required parameter, '" + (*parametersIterator)->Name() + "', is missing.\n";
                parseSuccessful = false;
                break;
            }
        }
    }

    return parseSuccessful;
}

//-----------------------------------------------------------------------------
/// Searches all defined commandline parameters for one matching the parameterName
/// string argument.  A case insensitive comparison is made.
/// \param parameterName A string containing the name of the parameter to match.
/// \return A pointer to the CommandlineParameter definition that matches the string with
/// the name matching the parameterName argument.  Null is returned if a match is not found.
//-----------------------------------------------------------------------------
CommandlineParameter* CommandlineParser::MatchParameter(const std::string& parameterName) const
{
    CommandlineParameter* pParam = nullptr;
    for (auto parametersIterator = m_definedParameters.begin(); parametersIterator != m_definedParameters.end(); ++parametersIterator)
    {
#ifdef WIN32
        if (_strnicmp((*parametersIterator)->Name().c_str(), parameterName.c_str(), _MAX_PATH) == 0)
#else
        if (strncasecmp((*parametersIterator)->Name().c_str(), parameterName.c_str(), PATH_MAX) == 0)
#endif // WIN32
        {
            pParam = *parametersIterator;
            break;
        }
    }
    return pParam;
}

//-----------------------------------------------------------------------------
/// Returns the error string generated by the Parse() method when invalid parameters
/// are parsed on the commandline.
/// \return The error string.
//-----------------------------------------------------------------------------
const std::string& CommandlineParser::ErrorString() const
{
    return m_errorString;
}

//-----------------------------------------------------------------------------
/// Builds and returns a help message string for all of the defined parameters
/// \return The generated help string.
//-----------------------------------------------------------------------------
const std::string& CommandlineParser::HelpString()
{
    static std::string helpString;
    const std::string indentString("          ");
    helpString = "Options:\n";

    for (auto parametersIterator = m_definedParameters.begin(); parametersIterator != m_definedParameters.end(); ++parametersIterator)
    {
        if (!(*parametersIterator)->Description().empty())
        {
            helpString += indentString + (*parametersIterator)->Description() + "\n";
        }
    }
    helpString += indentString + m_helpOptionDescription + "\n";
    return helpString;
}
