/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//--

// the application "command" that can be run by the BCC (Content Compiler)
// the typical role of a Command is to do some processing and exit, there's no notion of a loop
// NOTE: command is run on a separate thread not to block main thread
class BM_CORE_OBJECT_API ICommand : public IObject
{
    RTTI_DECLARE_OBJECT_CLASS(ICommand, IObject);

public:
    ICommand();
    virtual ~ICommand();

    //--

    /// do actual work of the command, called after all local services were initialized
    /// returned value will be the return code of the whole app
    virtual int run(const CommandLine& commandline) = 0;

    //--
};

//--

// metadata with name of the command 
class BM_CORE_OBJECT_API CommandNameMetadata : public IMetadata
{
    RTTI_DECLARE_VIRTUAL_CLASS(CommandNameMetadata, IMetadata);

public:
    CommandNameMetadata();

    INLINE StringView name() const { return m_name; }

    INLINE void name(StringView name) { m_name = name; }

private:
    StringView m_name;
};

//--

END_INFERNO_NAMESPACE()
