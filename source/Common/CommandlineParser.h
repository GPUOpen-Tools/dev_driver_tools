//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for the general purpose commandline parser class.
//=============================================================================

#ifndef _COMMANDLINE_PARSER_H_
#define _COMMANDLINE_PARSER_H_

#include "CommandlineParameter.h"

class CommandlineParser
{
public:
    CommandlineParser(int argc, char* argv[]);
    void AddParameter(CommandlineParameter* pParameter);
    void SetHelpOption(const char* pName, const char* pDescription);
    bool Parse();
    const std::string& ErrorString() const;
    const std::string& HelpString();
    bool IsHelpRequested() const;

protected:
    CommandlineParameter* MatchParameter(const std::string& ParameterName) const;

private:
    std::vector<CommandlineParameter*>  m_definedParameters;        ///< List of commandline parameter definitions.
    std::vector<std::string>            m_commandlineArguments;     ///< List of items specified on the commandline.
    std::string                         m_errorString;              ///< Error string generated after parsing the commandline.
    std::string                         m_helpOptionName;           ///< Commandline option used to display a help message.
    std::string                         m_helpOptionDescription;    ///< Description of the help option.
    bool                                m_isHelpOptionFound;        ///< Indicates that the help option was specified on the commandline.
};
#endif //_COMMANDLINE_PARSER_H_
