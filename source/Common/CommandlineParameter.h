//=============================================================================
/// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Header for the commandline parameter definition and parser class.
///  Used by the CommandlineParser class.
//=============================================================================
#ifndef _COMMANDLINE_PARAMETER_H_
#define _COMMANDLINE_PARAMETER_H_
#include <string>
#include <vector>

class CommandlineParameter
{
public:
    CommandlineParameter(const char* pName, const char* pDescription, bool required = false, bool flagParameter = false, const char* pDefaultValue = "");

    virtual bool Parse(const std::string& value);
    const std::string& Value() const;
    bool IsParameterPresent() const;
    bool IsValuePresent() const;
    bool IsValid() const;
    bool IsParsed() const;
    bool IsFlag() const;
    bool IsRequired() const;
    const std::string& Description() const;
    const std::string& Name() const;

protected:
    std::string m_name;         ///< The name of the parameter.
    std::string m_description;  ///< The description of the parameter.
    std::string m_value;        ///< The parsed value of the parameter (or default value).
    bool m_isParsed;            ///< Indicates that the parameter has been parsed.
    bool m_isParameterPresent;  ///< Indicates that the parameter is present on the parsed commandline.
    bool m_isValuePresent;      ///< Indicates that the parameter's value is present on the parsed commandline.
    bool m_isValid;             ///< Indicates that the parameter's parsed value is valid.
    bool m_isFlag;              ///< Indicates that the parameter is defined as a flag.
    bool m_isRequired;          ///< Indicates that the parameter is required to be on the commandline.
};

class Int16CommandlineParameter : public CommandlineParameter
{
public:
    Int16CommandlineParameter(const char* pName, const char* pDescription, bool required = false, int defaultValue = 0);
    virtual bool Parse(const std::string& value);
    int ValueAsInt() const;

private:
    int m_intValue;             ///< The 16 bit value parsed on the commandline associated with the parameter.
};
#endif // _COMMANDLINE_PARAMETER_H_
