/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [#filter: rtti\types\function #]
***/

#pragma once

#include "rttiFunctionPointer.h"
#include "rttiTypeRef.h"

BEGIN_INFERNO_NAMESPACE()

//---

class FunctionBuilder;

// abstract function call stack frame
class BM_CORE_OBJECT_API IFunctionStackFrame : public MainPoolData<NoCopy>
{
public:
    IFunctionStackFrame(const IFunctionStackFrame* parent);
    virtual ~IFunctionStackFrame();

    INLINE const IFunctionStackFrame* parent() const { return m_parent; }

    virtual StringID functionName() const = 0;
    virtual StringID className() const = 0;

    virtual StringBuf sourceFile() const = 0;
    virtual uint32_t sourceLine() const = 0;

    virtual uint32_t findCurrentSourceLine() const = 0;

    virtual void throwException(const char* txt) const;

    //--

    // print callstack
    void print(IFormatStream& f) const;

    // dump callstack to log (debug)
    void dump();

    //--

    // Get current script stack frame (if we are being called from scripts)
    // Main use: to get the file/line or to throw an "exception"
    // NOTE: can return NULL if we are NOT called from scripts
    static const IFunctionStackFrame* CurrentStackFrame();

    //--

protected:
    const IFunctionStackFrame* m_parent = nullptr;

    const IFunctionStackFrame* ChangeStackFrame(const IFunctionStackFrame* next);
};

//---

// modifier
enum class FunctionParamFlag : uint8_t
{
    Normal = 0,
    Ref = 1,
    Const = 2,
    ConstRef = 3,
    Ptr = 4,
    ConstPtr = 6,
    HasDestructor = 8,
    HasConstructor = 16,
};

typedef DirectFlags<FunctionParamFlag> FunctionParamFlags;

// function parameter
#pragma pack(push, 1)
struct BM_CORE_OBJECT_API FunctionParamType
{
    Type type;
    FunctionParamFlags flags = FunctionParamFlag::Normal; // no special refs/other modes
    uint8_t typeAlignment = 0;
    uint16_t typeSize = 0;
    uint16_t offset = 0; // in param block

    //---

    INLINE FunctionParamType()  {};
    INLINE FunctionParamType(const FunctionParamType& other) = default;
    INLINE FunctionParamType& operator=(const FunctionParamType& other) = default;
    FunctionParamType(Type, FunctionParamFlags flags = FunctionParamFlag::Normal);

    INLINE bool valid() const { return !type.empty(); }

    void print(IFormatStream& f) const;
};
#pragma pack(pop)

//---

// flags
enum class FunctionFlag : uint8_t
{
    Native = FLAG(0), // function has native (C++) implementation
    Const = FLAG(1), // function is guaranteed not to modify object
    Static = FLAG(2), // function is static (no context object required)
    Scripted = FLAG(3), // function is scripted (has script code blob)
};

typedef DirectFlags<FunctionFlag> FunctionFlags;

//---

class FunctionSignatureBuilder;

// function signature - return value and parameters
class BM_CORE_OBJECT_API FunctionSignature
{
public:
    FunctionSignature();
    FunctionSignature(const FunctionSignature& other) = default;
    FunctionSignature& operator=(const FunctionSignature& other) = default;
    FunctionSignature(const FunctionParamType& retType, const Array<FunctionParamType>& argTypes, bool isConst, bool isStatic);

    //--

    /// get type returned by the function
    INLINE FunctionParamType returnType() const { return m_returnType; }

    /// get number of function parameters
    INLINE uint32_t numParams() const { return m_paramTypes.size(); }

    /// get function parameter type
    INLINE const FunctionParamType* params() const { return m_paramTypes.typedData(); }

    /// NOTE: non static functions will require object pointer
    INLINE bool isStatic() const { return m_static; }

    /// is this a const function ?
    INLINE bool isConst() const { return m_const; }

    //--

    /// size of data required for call parameters block, may be zero
    INLINE uint32_t paramBlockSize() const { return m_paramBlockSize; }

    /// alignment of the param block size
    INLINE uint32_t paramBlockAlignment() const { return m_paramBlockAlignment; }

    /// mask of parameters that need to be destroyed
    INLINE uint32_t paramBlockDestructorMask() const { return m_paramBlockDestructorMask; }

    /// size of data required for return value, may be zero for void functions
    INLINE uint32_t returnBlockSize() const { return m_returnBlockSize; }

    /// alignment required for the return value
    INLINE uint32_t returnBlockAlignment() const { return m_returnBlockAlignment; }

    // recalculate param block size and other type-dependant properties
    void updateParamBlockData();

    //--

    /// get function signature as string
    void print(IFormatStream& f) const;

    //--

private:
    uint16_t m_paramBlockSize = 0; // size of the param block
    uint16_t m_paramBlockAlignment = 1;
    uint16_t m_returnBlockSize = 0; // size of return block
    uint16_t m_returnBlockAlignment = 1;
    uint32_t m_paramBlockDestructorMask = 0;

    FunctionParamType m_returnType; // type returned by the function

    typedef Array<FunctionParamType> TParamTypes;
    TParamTypes m_paramTypes; // type of parameters accepted by function

    bool m_static = false;
    bool m_const = false;

    friend class FunctionSignatureBuilder;
};

//---

// basic function
class BM_CORE_OBJECT_API IFunction : public MainPoolData<NoCopy>
{
public:
    IFunction(const IType* parent, StringID name, const FunctionSignature& signature, bool scripted);

    /// get the name of the property
    INLINE StringID name() const { return m_name; }

    /// get the owner of this property
    INLINE const IType* parent() const { return m_parent; }

    /// get function calling signature
    INLINE const FunctionSignature& signature() const { return m_signature; }

    /// is this a scripted function ?
    INLINE bool scripted() const { return m_scripted; }

    //--

    /// run this function in context of given object with given params (passes as a block compatible with signature)
    virtual void run(const IFunctionStackFrame* parentFrame, void* context, const void* params, void* result) const = 0;

    //--

private:
    const IType* m_parent; // class
    StringID m_name; // name of the function

    FunctionSignature m_signature;

    bool m_scripted = false;

protected:
    void updateSignature(const FunctionSignature& sig);
};

//---

// callable object's native function, usable in the visual scripting system
class BM_CORE_OBJECT_API NativeFunction : public IFunction
{
public:
    NativeFunction(const IType* parent, StringID name, const FunctionSignature& signature, FunctionPointer nativePtr, TFunctionWrapperPtr wrappedPtr);

    ///--

    /// get the native function pointer (for direct calls)
    INLINE const FunctionPointer& nativeFunctionPointer() const { return m_nativePtr; }

    /// get universal function wrapper
    INLINE TFunctionWrapperPtr nativeFunctionWrapper() const { return m_wrappedPtr; }

    //--

    /// run function, works for both native and scripted functions, totally transparent, can be used by scripting languages when running the function directly is out of question
    virtual void run(const IFunctionStackFrame* parentFrame, void* context, const void* params, void* result) const override final;

    //--

private:
    FunctionPointer m_nativePtr = nullptr;
    TFunctionWrapperPtr m_wrappedPtr = nullptr;
};

//---

// script callable function, contains specialized Mono shim that calls the native function pointer
class BM_CORE_OBJECT_API IScriptFunction : public IFunction
{
public:
    IScriptFunction(const IType* parent, StringID name, const FunctionSignature& signature);
};

//---

END_INFERNO_NAMESPACE()

//--