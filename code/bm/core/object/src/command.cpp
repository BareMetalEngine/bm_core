/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "command.h"

BEGIN_INFERNO_NAMESPACE()

//--

RTTI_BEGIN_TYPE_CLASS(CommandNameMetadata);
RTTI_END_TYPE();

CommandNameMetadata::CommandNameMetadata()
{}

//--

RTTI_BEGIN_TYPE_ABSTRACT_CLASS(ICommand);
RTTI_END_TYPE();

//--

ICommand::ICommand()
{}

ICommand::~ICommand()
{}

//--

END_INFERNO_NAMESPACE()
