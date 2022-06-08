/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "bm/core/containers/include/stringBuf.h"
#include "bm/core/containers/include/array.h"
#include "bm/core/containers/include/stringBuf.h"

BEGIN_INFERNO_NAMESPACE()

/// Command line parameter
struct BM_CORE_CONTAINERS_API CommandLineParam
{
    StringBuf name;
    StringBuf value;

    INLINE bool operator==(const CommandLineParam& other) const
    {
        return (name == other.name) && (value == other.value);
    }

    INLINE bool operator!=(const CommandLineParam& other) const
    {
        return !operator==(other);
    }
};

/// Command line arguments holder
/// NOTE: we have very specific arguments format for all of the tools:
/// First words can be without the trailing -, they are assumed to be ORDERED commands
/// We can have flags: -foo -bar
/// We can have values: -file=XXX -bar="X:\path"
/// We do not support "stray" arguments
/// Command line parameters are not enumerable - you can only ask if we have a specific one and what's the value(s)
/// Example: XXX.exe compile -file=X:\data\x.fx -outdir="X:\out dir\"
/// Example: XXX.exe help compile
/// Example: XXX.exe -dump=X:\crap.txt
/// NOTE: commandline is unicode (so we can have proper paths specified)
class BM_CORE_CONTAINERS_API CommandLine
{
public:
    CommandLine();
    CommandLine(const CommandLine& other);
    CommandLine(CommandLine&& other);
    CommandLine& operator=(const CommandLine& other);
    CommandLine& operator=(CommandLine&& other);

    //--

    //! check if the command line is empty
    INLINE bool empty() const { return m_commands.empty() && m_params.empty(); }

    //! get all of the command words
    INLINE const Array<StringBuf>& commands() const { return m_commands; }

    //! get the single command word, may be empty, returns the first word from the command list
    INLINE const StringBuf& command() const { return m_commands.empty() ? StringBuf::EMPTY() : m_commands[0]; }

    //! get all parameters
    INLINE const Array<CommandLineParam>& params() const { return m_params; }

    //--

    //! parse the commandline from specified text (UTF8) that conforms to the commandline formatting, returns false if the commandline is invalid
    bool parse(StringView txt, bool skipProgramPath);

    //! parse the commandline from specified text (UTF16) that conforms to the commandline formatting, returns false if the commandline is invalid
    bool parse(const wchar_t* txt, bool skipProgramPath);

    //! parse the command line from a "classic" C argc/argv combo
    bool parse(int argc, wchar_t **argv);

    //! parse the command line from a "classic" C argc/argv combo
    bool parse(int argc, char **argv);

    //--

    //! set value of a parameter
    void param(StringView name, StringView value);

    //! remove parameter
    void removeParam(StringView name);

    //-----

    //! check if params is defined
    bool hasParam(StringView param) const;

    //! get single value (last defined value) of a parameter
    const StringBuf& singleValue(StringView param) const;

    //! get single value (last defined value) of parameter and parse it as an integer
    StringView singleValueStr(StringView param, StringView defaultValue = "") const;

    //! get single value (last defined value) of parameter and parse it as an integer
    int singleValueInt(StringView param, int defaultValue = 0) const;

    //! get single value (last defined value) of parameter and parse it as a boolean
    bool singleValueBool(StringView param, bool defaultValue = false) const;

    //! get single value (last defined value) of parameter and parse it as a floating point value
    float singleValueFloat(StringView param, float defaultValue = 0.0f) const;

    //--

    //! add command word
    void addCommand(StringView command);

    //! remove command word
    void removeCommand(StringView command);

    //! do we have command word ?
    bool hasCommand(StringView command) const;

    //---

    //! gets all values for given parameter
    Array<StringBuf> allValues(StringView param) const;

    //---

    //! parse commandline from given string, after first error parsing stops but something is returned
    //! NOTE: this function does not guarantee safety in case of mall formated commandline
    static bool Parse(const StringBuf& buf, CommandLine& outCommandline);

    //--

    //! convert commandline to an UTF8 string that can be passed forward
    StringBuf toString() const;

    //! convert commandline to an UTF16 string that can be passed forward
    Array<wchar_t> toUTF16String() const;

    //! unpack as a list of arguments
    Array<Array<wchar_t>> toUTF16StringArray() const;

    //--

    // reprint as a command line string
    void print(IFormatStream& f) const;

private:
    Array<StringBuf> m_commands; // command words (non-params) ordered as in the original commandline
    Array<CommandLineParam> m_params; // key-value parameters

    // script interface wrappers (Scripts don't see StringView<> yet)
    void paramStr(const StringBuf& name, const StringBuf& value);
    void removeParamStr(const StringBuf& name);
    bool hasParamStr(const StringBuf& name) const;

    StringBuf singleValueStr(const StringBuf& name) const;
    int singleValueIntStr(const StringBuf& name, int defaultValue) const;
    bool singleValueBoolStr(const StringBuf& name, bool defaultValue) const;
    float singleValueFloatStr(const StringBuf& name, float defaultValue) const;

    Array<StringBuf> allValuesStr(const StringBuf& param) const;
};

//---

END_INFERNO_NAMESPACE()
