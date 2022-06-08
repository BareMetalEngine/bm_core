/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"

#include "commandLine.h"
#include "bm/core/containers/include/stringParser.h"
#include "bm/core/containers/include/stringBuilder.h"

BEGIN_INFERNO_NAMESPACE()

//--

CommandLine::CommandLine() = default;
CommandLine::CommandLine(const CommandLine& other) = default;
CommandLine::CommandLine(CommandLine&& other) = default;
CommandLine& CommandLine::operator=(const CommandLine& other) = default;
CommandLine& CommandLine::operator=(CommandLine&& other) = default;

/*bool CommandLine::operator==(const CommandLine& other) const
{
    if (!(m_commands == other.m_commands))
        return false;
    if (m_params.size() != other.m_params.size())
        return false;

    for (auto& param : m_params)
    {
        bool matched = false;
        for (auto& otherParam : other.m_params)
            if (param.name == otherParam.name)
                if (param.value == otherParam.value)
                {
                    matched = true;
                    break;
                }

        if (!matched)
            return false;
    }

    return true;
}

bool CommandLine::operator!=(const CommandLine& other) const
{
    return !operator==(other);
}*/

//--

bool CommandLine::parse(StringView commandline, bool skipProgramPath)
{
    StringParser parser(commandline);

    // skip the program path
    if (skipProgramPath)
    {
        StringView programPath;
        if (!parser.parseString(programPath))
        {
            TRACE_WARNING("No program path in passed commandline");
            return false;
        }
    }

    // parse the ordered commands
    bool parseInitialChar = true;
    while (parser.parseWhitespaces())
    {
        // detect start of parameters
        if (parser.parseKeyword("-"))
        {
            parseInitialChar = false;
            break;
        }

        // get the command
        StringView commandName;
        if (!parser.parseIdentifier(commandName))
        {
            TRACE_ERROR("Commandline parsing error: expecting command name. Application may not work as expected.");
            return false;
        }

        m_commands.pushBack(StringBuf(commandName));
    }

    // parse the text
    while (parser.parseWhitespaces())
    {
        // Skip the initial '-'
        if (!parser.parseKeyword("-") && parseInitialChar)
            break;
        parseInitialChar = true;

        // Get the name of the parameter
        StringView paramName;
        if (!parser.parseIdentifier(paramName))
        {
            TRACE_ERROR("Commandline parsing error: expecting param name after '-'. Application may not work as expected.");
            return false;
        }

        // Do we have a value ?
        StringView paramValue;
        if (parser.parseKeyword("="))
        {
            // Read value
            if (!parser.parseString(paramValue))
            {
                TRACE_ERROR("Commandline parsing error: expecting param value after '=' for param '{}'. Application may not work as expected.", paramName);
                return false;
            }
        }

        // add parameter
        m_params.emplaceBack(CommandLineParam{ StringBuf(paramName), StringBuf(paramValue) });
    }

    // parsed
    return true;
}

bool CommandLine::parse(const wchar_t* commandline, bool skipProgramPath)
{
    StringBuf ansiCommandline(commandline);
    return parse(ansiCommandline.c_str(), skipProgramPath);
}

bool CommandLine::parse(int argc, wchar_t **argv)
{
    StringBuilder str;
    for (int i = 1; i < argc; ++i)
    {
        if (i > 1)
            str.append(" ");

        str.append(argv[i]);
    }

    return parse(str.c_str(), false);
}

bool CommandLine::parse(int argc, char **argv)
{
    StringBuilder str;
    for (int i = 1; i < argc; ++i)
    {
        if (i > 1)
            str.append(" ");

        str.append(argv[i]);
    }

    return parse(str.c_str(), false);
}

void CommandLine::addCommand(StringView command)
{
    if (!hasCommand(command))
        m_commands.pushBack(StringBuf(command));
}

void CommandLine::removeCommand(StringView command)
{
    for (uint32_t i=0; i<m_commands.size(); ++i)
        if (0 == m_commands[i].compare(command, StringCaseComparisonMode::NoCase))
            m_commands.erase(i);
}

bool CommandLine::hasCommand(StringView command) const
{
    for (auto& ptr : m_commands)
        if (0 == ptr.compare(command, StringCaseComparisonMode::NoCase))
            return true;

    return false;
}

void CommandLine::param(StringView name, StringView value)
{
    for (auto& param : m_params)
    {
        if (param.name == name)
        {
            param.value = StringBuf(value);
            return;
        }
    }

    m_params.emplaceBack(CommandLineParam{ StringBuf(name), StringBuf(value) });
}

void CommandLine::removeParam(StringView name)
{
    for (int i = (int)m_params.size() - 1; i >= 0; --i)
    {
        if (m_params[i].name == name)
        {
            m_params.erase(i);
        }
    }
}

bool CommandLine::hasParam(StringView paramName) const
{
    // Linear search
    for (auto& param : m_params)
        if (param.name == paramName)
            return true;

    // Param was not defined
    return false;
}

const StringBuf& CommandLine::singleValue(StringView paramName) const
{
    // Linear search
    for (auto& param : m_params)
        if (param.name == paramName)
            return param.value;

    // No value
    return StringBuf::EMPTY();
}

StringView CommandLine::singleValueStr(StringView paramName, StringView defaultValue /*= ""*/) const
{
    // Linear search
    for (auto& param : m_params)
        if (param.name == paramName)
            return param.value;

    // No value
    return defaultValue;
}

int CommandLine::singleValueInt(StringView paramName, int defaultValue /*= 0*/) const
{
    // Linear search
    for (auto& param : m_params)
    {
        if (param.name == paramName)
        {
            int ret = defaultValue;
            if (param.value.view().match(ret))
                return ret;
        }
    }

    // No value
    return defaultValue;
}

float CommandLine::singleValueFloat(StringView paramName, float defaultValue /*= 0*/) const
{
    // Linear search
    for (auto& param : m_params)
    {
        if (param.name == paramName)
        {
            float ret = defaultValue;
            if (param.value.view().match(ret))
                return ret;
        }
    }

    // No value
    return defaultValue;
}

bool CommandLine::singleValueBool(StringView paramName, bool defaultValue /*= 0*/) const
{
    // Linear search
    for (auto& param : m_params)
    {
        if (param.name == paramName)
        {
            if (param.value == "true")
                return true;
            else if (param.value == "false")
                return false;

            int ret = defaultValue;
            if (param.value.view().match(ret))
                return ret != 0;
        }
    }

    // No value
    return defaultValue;
}

Array< StringBuf > CommandLine::allValues(StringView paramName) const
{
    Array< StringBuf > ret;

    // Extract all
    for (auto& param : m_params)
        if (param.name == paramName)
            if (!param.value.empty())
                ret.pushBack(param.value);

    return ret;
}

//--

bool CommandLine::Parse(const StringBuf& buf, CommandLine& outRet)
{
    CommandLine ret;
    if (ret.parse(buf.view(), false))
    {
        outRet = std::move(ret);
        return true;
    }
    return false;
}

//--

static bool ShouldWrapPram(StringView str)
{
    for (auto ch : str)
    {
        if (ch == '\"' || ch == '\'' || ch <= 32 || ch == '=' || ch == '/' || ch == '\\')
            return true;
    }
    return false;
}

StringBuf CommandLine::toString() const
{
    StringBuilder ret;

    // commands
    for (auto& cmd : m_commands)
    {
        if (!ret.empty())
            ret << " ";
        ret << cmd;
    }

    // parameters
    for (auto& param : m_params)
    {
        if (!ret.empty())
            ret << " ";

        ret << "-" << param.name;

        if (!param.value.empty())
        {
            ret << "=";

            if (ShouldWrapPram(param.value))
                ret << "\"" << param.value << "\"";
            else
                ret << param.value;
        }
    }

    return StringBuf(ret);
}

Array<wchar_t> CommandLine::toUTF16String() const
{
    return toString().view().exportUTF16();
}

Array<Array<wchar_t>> CommandLine::toUTF16StringArray() const
{
    Array<Array<wchar_t>> ret;

    for (auto& cmd : m_commands)
        ret.emplaceBack(cmd.view().exportUTF16());

    for (auto& param : m_params)
    {
        StringBuilder txt;
        txt << "-";
        txt << param.name;

        if (!param.value.empty())
        {
            txt << "=";

            if (ShouldWrapPram(param.value))
            {
                txt << "\"";
                txt << param.value;
                txt << "\"";
            }
            else
            {
                txt << param.value;
            }
        }

        ret.emplaceBack(txt.view().exportUTF16());
    }

    return ret;
}

//--

void CommandLine::paramStr(const StringBuf& name, const StringBuf& value)
{
    param(name, value);
}

void CommandLine::removeParamStr(const StringBuf& name)
{
    removeParam(name);
}

bool CommandLine::hasParamStr(const StringBuf& name) const
{
    return hasParam(name);
}

StringBuf CommandLine::singleValueStr(const StringBuf& name) const
{
    return singleValue(name);
}

int CommandLine::singleValueIntStr(const StringBuf& name, int defaultValue) const
{
    return singleValueInt(name, defaultValue);
}

bool CommandLine::singleValueBoolStr(const StringBuf& name, bool defaultValue) const
{
    return singleValueBool(name, defaultValue);
}

float CommandLine::singleValueFloatStr(const StringBuf& name, float defaultValue) const
{
    return singleValueFloat(name, defaultValue);
}

Array<StringBuf> CommandLine::allValuesStr(const StringBuf& param) const
{
    return allValues(param);
}

//--

void CommandLine::print(IFormatStream& f) const
{
    bool needsSeparator = false;

    for (const auto& command : m_commands)
    {
        if (needsSeparator)
            f << " ";

        f << command;
        needsSeparator = true;
    }

    for (const auto& param : m_params)
    {
        if (needsSeparator)
            f << " ";

        f << "-";
        f << param.name;

        if (!param.value.empty())
        {
            f << "=";
            f << param.value;
        }
    }
}

//--

/*CommandLineUnpackedAnsi::CommandLineUnpackedAnsi(const CommandLine& cmdLine)
    : m_mem(POOL_TEMP)
{
    // count arguments
    uint32_t numArgs = cmdLine.params().size() + cmdLine.commands().size() + 1;

    // get path to executable
    auto executablePath = SystemPath(PathCategory::ExecutableFile);
    if (!executablePath.empty())
        numArgs += 1;

    // allocate structures
    m_argc = numArgs - 1;
    m_argv = (char**) m_mem.alloc(sizeof(const char*) * numArgs, 1);
    memset(m_argv, 0, sizeof(const char*) * numArgs);

    // copy program path
    uint32_t paramIndex = 0;
    if (!executablePath.empty())
        m_argv[paramIndex++] = m_mem.strcpy(executablePath.c_str());

    // copy commands
    for (auto& command : cmdLine.commands())
        m_argv[paramIndex++] = m_mem.strcpy(command.c_str());

    // params
    for (auto& param : cmdLine.params())
    {
        if (param.value.empty())
            m_argv[paramIndex++] = m_mem.strcpy(TempString("-{}", param.name).c_str());
        else
            m_argv[paramIndex++] = m_mem.strcpy(TempString("-{}={}", param.name, param.value).c_str());
    }
}

CommandLineUnpackedAnsi::~CommandLineUnpackedAnsi()
{}*/

//--

END_INFERNO_NAMESPACE()
