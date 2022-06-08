/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [#filter: rtti\types\function #]
***/

#include "build.h"
#include "rttiType.h"
#include "rttiFunction.h"

#include "bm/core/containers/include/stringBuilder.h"

BEGIN_INFERNO_NAMESPACE()

//---

IFunctionStackFrame::IFunctionStackFrame(const IFunctionStackFrame* parent)
    : m_parent(parent)
{}

IFunctionStackFrame::~IFunctionStackFrame()
{}

void IFunctionStackFrame::throwException(const char* txt) const
{
#ifdef BUILD_RELEASE
#ifdef PLATFORM_GCC
    fprintf(stderr, "Script exception: %s\n", txt);
#endif
#else
    TRACE_ERROR("Script exception: {}", txt);
    print(TRACE_STREAM_ERROR());
#endif
}

void IFunctionStackFrame::print(IFormatStream& f) const
{
    auto className = this->className();
    auto funcName = functionName();

    if (!funcName)
        f.appendf("UnknownFunction()");
    else if (!className)
        f.appendf("{}()", funcName);
    else
        f.appendf("{}.{}()", className, funcName);

    auto file = sourceFile();
    if (file)
        f.appendf(" at {}({})\n", file, sourceLine());
    else

    if (auto parent  = this->parent())
    {
        f.append("\n");
        parent->print(f);
    }
}

void IFunctionStackFrame::dump()
{
    print(TRACE_STREAM_INFO());
}

//---

static TYPE_TLS const IFunctionStackFrame* GCurrentStackFrame = nullptr;

const IFunctionStackFrame* IFunctionStackFrame::CurrentStackFrame()
{
    return GCurrentStackFrame;
}

const IFunctionStackFrame* IFunctionStackFrame::ChangeStackFrame(const IFunctionStackFrame* next)
{
    ASSERT(next != GCurrentStackFrame);
    auto ret = GCurrentStackFrame;
    GCurrentStackFrame = next;
    return ret;
}

//---

FunctionParamType::FunctionParamType(Type t, FunctionParamFlags flags /*= FunctionParamFlag::Normal*/)
    : type(t)
    , flags(flags)
{}

void FunctionParamType::print(IFormatStream& f) const
{
    if (type)
    {
        if (flags.test(FunctionParamFlag::Const))
            f << "const ";
        f << type->name();
        if (flags.test(FunctionParamFlag::Ref))
            f << "&";
        if (flags.test(FunctionParamFlag::Ptr))
            f << "*";
    }
    else
    {
        f << "null";
    }
}

//---

FunctionSignature::FunctionSignature()
{}

FunctionSignature::FunctionSignature(const FunctionParamType & retType, const Array<FunctionParamType>&argTypes, bool isConst, bool isStatic)
    : m_returnType(retType)
    , m_paramTypes(argTypes)
    , m_static(isStatic)
    , m_const(isConst)
{
    updateParamBlockData();
}

void FunctionSignature::updateParamBlockData()
{
    if (m_returnType.type)
    {
        m_returnBlockAlignment = m_returnType.type->alignment();
        m_returnBlockSize = m_returnType.type->size();
    }
    else
    {
        m_returnBlockAlignment = 1;
        m_returnBlockSize = 0;
    }

    m_paramBlockAlignment = 1;
    m_paramBlockSize = 0;
    m_paramBlockDestructorMask = 0;

    for (auto i : m_paramTypes.indexRange())
    {
        auto& param = m_paramTypes[i];
        param.typeSize = param.type->size();
        param.typeAlignment = param.type->alignment();

        if (param.type->traits().initializedFromZeroMem || !param.type->traits().requiresConstructor)
            param.flags -= FunctionParamFlag::HasConstructor;
        else
            param.flags |= FunctionParamFlag::HasConstructor;

        if (param.type->traits().requiresDestructor)
            param.flags |= FunctionParamFlag::HasDestructor;
        else
            param.flags -= FunctionParamFlag::HasDestructor;

        uint32_t neededAlignment = 1;
        uint32_t neededSize = sizeof(void*);

        if (!param.flags.test(FunctionParamFlag::Ref) && !param.flags.test(FunctionParamFlag::Ptr))
        {
            neededAlignment = param.type->alignment();
            neededSize = param.type->size();

            if (param.type->traits().requiresDestructor)
                m_paramBlockDestructorMask |= (1U << i);
        }

        m_paramBlockAlignment = std::max<uint32_t>(m_paramBlockAlignment, neededAlignment);
        param.offset = Align<uint16_t>(m_paramBlockSize, neededAlignment);
        m_paramBlockSize = param.offset + neededSize;
    }
    //TRACE_SPAM("Param block size for '{}': {}", *this, m_paramBlockSize);
}

void FunctionSignature::print(IFormatStream& f) const
{
    if (m_static)
        f << "static ";

    if (m_returnType.valid())
        f << m_returnType;
    else
        f << "void";
    f << " ";
    f << "(";

    for (uint32_t i = 0; i < m_paramTypes.size(); ++i)
    {
        if (i > 0)
            f << ", ";
        f << m_paramTypes[i];
    }

    f << ")";

    if (m_const)
        f << " const";
}

//---

IFunction::IFunction(const IType* parent, StringID name, const FunctionSignature& signature, bool scripted)
    : m_parent(parent)
    , m_name(name)
    , m_signature(signature)
    , m_scripted(scripted)
{
}

void IFunction::updateSignature(const FunctionSignature& sig)
{
    ASSERT_EX(m_scripted, "Only scripted functions can get their signatures updated");
    m_signature = sig;
}

//---

NativeFunction::NativeFunction(const IType* parent, StringID name, const FunctionSignature& signature, FunctionPointer nativePtr, TFunctionWrapperPtr wrappedPtr)
    : IFunction(parent, name, signature, false)
    , m_nativePtr(nativePtr)
    , m_wrappedPtr(wrappedPtr)
{}

void NativeFunction::run(const IFunctionStackFrame* parentFrame, void* context, const void* params, void* result) const
{
    // TODO: add a stack element for native function call when not in final mode
    (*m_wrappedPtr)(context, params, result);
}

//--

IScriptFunction::IScriptFunction(const IType* parent, StringID name, const FunctionSignature& signature)
    : IFunction(parent, name, signature, true)
{
}

//--

END_INFERNO_NAMESPACE()
