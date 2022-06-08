/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
*
* [#filter: reflection #]
***/

#pragma once

#include "reflectionTypeName.h"
#include "reflectionPropertyBuilder.h"

#include "rttiFunction.h"
#include "rttiFunctionPointer.h"

BEGIN_INFERNO_NAMESPACE()

//---

// helper class that can build a function signature
class BM_CORE_OBJECT_API FunctionSignatureBuilder : public MainPoolData<NoCopy>
{
public:
    template< typename T >
    INLINE static FunctionParamType MakeType()
    {
        typedef typename std::remove_reference< typename std::remove_pointer<T>::type >::type InnerT;
        typedef typename std::remove_cv<InnerT>::type SafeT;

        auto retType  = GetTypeObject<SafeT>();
        ASSERT_EX(retType != nullptr, "Return type not found in RTTI, only registered types can be used");

        FunctionParamFlags flags = FunctionParamFlag::Normal;
        if (std::is_const<InnerT>::value)
            flags |= FunctionParamFlag::Const;
        if (std::is_reference<T>::value)
            flags |= FunctionParamFlag::Ref;
        if (std::is_pointer<T>::value)
            flags |= FunctionParamFlag::Ptr;

        return FunctionParamType(retType, flags);
    }

    template< typename T >
    INLINE static void returnType(FunctionSignature& outSig)
    {
        outSig.m_returnType = MakeType<T>();
    }

    template< typename T >
    INLINE static void addParamType(FunctionSignature& outSig)
    {
        outSig.m_paramTypes.emplaceBack(MakeType<T>());
    }

    INLINE static void constFlag(FunctionSignature& outSig)
    {
        outSig.m_const = true;
    }

    INLINE static void staticFlag(FunctionSignature& outSig)
    {
        outSig.m_static = true;
    }

    INLINE static void finish(FunctionSignature& outSig)
    {
        outSig.updateParamBlockData();
    }

private:
    inline FunctionSignatureBuilder() {};
    inline ~FunctionSignatureBuilder() {};
};

//---

// helper class that can build a function call proxy for given class
template< typename _class >
class FunctionSignatureBuilderClass : public FunctionSignatureBuilder
{
public:
    //-- Set/Get

    template< typename T >
    INLINE static FunctionSignature captureSetter()
    {
        FunctionSignature sig;
        addParamType<T>(sig);
        finish(sig);
        return sig;
    }

    template< typename T >
    INLINE static FunctionSignature captureGetter()
    {
        FunctionSignature sig;
        returnType<T>(sig);
        constFlag(sig);
        finish(sig);
        return sig;
    }

    //-- Param Count: 0

    INLINE static FunctionSignature capture(void(_class::* func)())
    {
        return FunctionSignature();
    }

    INLINE static FunctionSignature capture(void(_class::* func)() const)
    {
        FunctionSignature sig;
        constFlag(sig);
        finish(sig);
        return sig;
    }

    template< typename Ret >
    INLINE static FunctionSignature capture(Ret(_class::*func)())
    {
        FunctionSignature sig;
        returnType<Ret>(sig);
        finish(sig);
        return sig;
    }

    template< typename Ret >
    INLINE static FunctionSignature capture(Ret(_class::* func)() const)
    {
        FunctionSignature sig;
        returnType<Ret>(sig);
        constFlag(sig);
        finish(sig);
        return sig;
    }

    //-- Param Count: 1

    template< typename F1 >
    INLINE static FunctionSignature capture(void(_class::*func)(F1))
    {
        FunctionSignature sig;
        addParamType<F1>(sig);
        finish(sig);
        return sig;
    }

    template< typename Ret, typename F1 >
    INLINE static FunctionSignature capture(Ret(_class::*func)(F1))
    {
        FunctionSignature sig;
        returnType<Ret>(sig);
        addParamType<F1>(sig);
        finish(sig);
        return sig;
    }

    template< typename F1 >
    INLINE static FunctionSignature capture(void(_class::* func)(F1) const)
    {
        FunctionSignature sig;
        addParamType<F1>(sig);
        constFlag(sig);
        finish(sig);
        return sig;
    }

    template< typename Ret, typename F1 >
    INLINE static FunctionSignature capture(Ret(_class::* func)(F1) const)
    {
        FunctionSignature sig;
        returnType<Ret>(sig);
        addParamType<F1>(sig);
        constFlag(sig);
        finish(sig);
        return sig;
    }

    //-- Param Count: 2

    template< typename F1, typename F2 >
    INLINE static FunctionSignature capture(void(_class::*func)(F1, F2))
    {
        FunctionSignature sig;
        addParamType<F1>(sig);
        addParamType<F2>(sig);
        finish(sig);
        return sig;
    }

    template< typename Ret, typename F1, typename F2 >
    INLINE static FunctionSignature capture(Ret(_class::*func)(F1, F2))
    {
        FunctionSignature sig;
        returnType<Ret>(sig);
        addParamType<F1>(sig);
        addParamType<F2>(sig);
        finish(sig);
        return sig;
    }

    template< typename F1, typename F2 >
    INLINE static FunctionSignature capture(void(_class::* func)(F1, F2) const)
    {
        FunctionSignature sig;
        addParamType<F1>(sig);
        addParamType<F2>(sig);
        constFlag(sig);
        finish(sig);
        return sig;
    }

    template< typename Ret, typename F1, typename F2 >
    INLINE static FunctionSignature capture(Ret(_class::* func)(F1, F2) const)
    {
        FunctionSignature sig;
        returnType<Ret>(sig);
        addParamType<F1>(sig);
        addParamType<F2>(sig);
        constFlag(sig);
        finish(sig);
        return sig;
    }

    //-- Param Count: 3

    template< typename F1, typename F2, typename F3 >
    INLINE static FunctionSignature capture(void(_class::*func)(F1, F2, F3))
    {
        FunctionSignature sig;
        addParamType<F1>(sig);
        addParamType<F2>(sig);
        addParamType<F3>(sig);
        finish(sig);
        return sig;
    }

    template< typename Ret, typename F1, typename F2, typename F3 >
    INLINE static FunctionSignature capture(Ret(_class::*func)(F1, F2, F3))
    {
        FunctionSignature sig;
        returnType<Ret>(sig);
        addParamType<F1>(sig);
        addParamType<F2>(sig);
        addParamType<F3>(sig);
        finish(sig);
        return sig;
    }

    template< typename F1, typename F2, typename F3 >
    INLINE static FunctionSignature capture(void(_class::* func)(F1, F2, F3) const)
    {
        FunctionSignature sig;
        addParamType<F1>(sig);
        addParamType<F2>(sig);
        addParamType<F3>(sig);
        constFlag(sig);
        finish(sig);
        return sig;

    }

    template< typename Ret, typename F1, typename F2, typename F3 >
    INLINE static FunctionSignature capture(Ret(_class::* func)(F1, F2, F3) const)
    {
        FunctionSignature sig;
        returnType<Ret>(sig);
        addParamType<F1>(sig);
        addParamType<F2>(sig);
        addParamType<F3>(sig);
        constFlag(sig);
        finish(sig);
        return sig;
    }

    //-- Param Count: 4

    template< typename F1, typename F2, typename F3, typename F4 >
    INLINE static FunctionSignature capture(void(_class::*func)(F1, F2, F3, F4))
    {
        FunctionSignature sig;
        addParamType<F1>(sig);
        addParamType<F2>(sig);
        addParamType<F3>(sig);
        addParamType<F4>(sig);
        finish(sig);
        return sig;
    }

    template< typename Ret, typename F1, typename F2, typename F3, typename F4 >
    INLINE static FunctionSignature capture(Ret(_class::*func)(F1, F2, F3, F4))
    {
        FunctionSignature sig;
        returnType<Ret>(sig);
        addParamType<F1>(sig);
        addParamType<F2>(sig);
        addParamType<F3>(sig);
        addParamType<F4>(sig);
        finish(sig);
        return sig;
    }

    template< typename F1, typename F2, typename F3, typename F4 >
    INLINE static FunctionSignature capture(void(_class::* func)(F1, F2, F3, F4) const)
    {
        FunctionSignature sig;
        addParamType<F1>(sig);
        addParamType<F2>(sig);
        addParamType<F3>(sig);
        addParamType<F4>(sig);
        constFlag(sig);
        finish(sig);
        return sig;
    }

    template< typename Ret, typename F1, typename F2, typename F3, typename F4 >
    INLINE static FunctionSignature capture(Ret(_class::* func)(F1, F2, F3, F4) const)
    {
        FunctionSignature sig;
        returnType<Ret>(sig);
        addParamType<F1>(sig);
        addParamType<F2>(sig);
        addParamType<F3>(sig);
        addParamType<F4>(sig);
        constFlag(sig);
        finish(sig);
        return sig;
    }

    //-- Param Count: 5

    template< typename F1, typename F2, typename F3, typename F4, typename F5 >
    INLINE static FunctionSignature capture(void(_class::*func)(F1, F2, F3, F4, F5))
    {
        FunctionSignature sig;
        addParamType<F1>(sig);
        addParamType<F2>(sig);
        addParamType<F3>(sig);
        addParamType<F4>(sig);
        addParamType<F5>(sig);
        finish(sig);
        return sig;
    }

    template< typename Ret, typename F1, typename F2, typename F3, typename F4, typename F5 >
    INLINE static FunctionSignature capture(Ret(_class::*func)(F1, F2, F3, F4, F5))
    {
        FunctionSignature sig;
        returnType<Ret>(sig);
        addParamType<F1>(sig);
        addParamType<F2>(sig);
        addParamType<F3>(sig);
        addParamType<F4>(sig);
        addParamType<F5>(sig);
        finish(sig);
        return sig;
    }

    template< typename F1, typename F2, typename F3, typename F4, typename F5 >
    INLINE static FunctionSignature capture(void(_class::* func)(F1, F2, F3, F4, F5) const)
    {
        FunctionSignature sig;
        addParamType<F1>(sig);
        addParamType<F2>(sig);
        addParamType<F3>(sig);
        addParamType<F4>(sig);
        addParamType<F5>(sig);
        constFlag(sig);
        finish(sig);
        return sig;
    }

    template< typename Ret, typename F1, typename F2, typename F3, typename F4, typename F5 >
    INLINE static FunctionSignature capture(Ret(_class::* func)(F1, F2, F3, F4, F5) const)
    {
        FunctionSignature sig;
        returnType<Ret>(sig);
        addParamType<F1>(sig);
        addParamType<F2>(sig);
        addParamType<F3>(sig);
        addParamType<F4>(sig);
        addParamType<F5>(sig);
        constFlag(sig);
        finish(sig);
        return sig;
    }

    //-- Param Count: 6

    template< typename F1, typename F2, typename F3, typename F4, typename F5, typename F6 >
    INLINE static FunctionSignature capture(void(_class::* func)(F1, F2, F3, F4, F5, F6))
    {
        FunctionSignature sig;
        addParamType<F1>(sig);
        addParamType<F2>(sig);
        addParamType<F3>(sig);
        addParamType<F4>(sig);
        addParamType<F5>(sig);
        addParamType<F6>(sig);
        finish(sig);
        return sig;
    }

    template< typename Ret, typename F1, typename F2, typename F3, typename F4, typename F5, typename F6 >
    INLINE static FunctionSignature capture(Ret(_class::* func)(F1, F2, F3, F4, F5, F6))
    {
        FunctionSignature sig;
        returnType<Ret>(sig);
        addParamType<F1>(sig);
        addParamType<F2>(sig);
        addParamType<F3>(sig);
        addParamType<F4>(sig);
        addParamType<F5>(sig);
        addParamType<F6>(sig);
        finish(sig);
        return sig;
    }

    template< typename F1, typename F2, typename F3, typename F4, typename F5, typename F6 >
    INLINE static FunctionSignature capture(void(_class::* func)(F1, F2, F3, F4, F5, F6) const)
    {
        FunctionSignature sig;
        addParamType<F1>(sig);
        addParamType<F2>(sig);
        addParamType<F3>(sig);
        addParamType<F4>(sig);
        addParamType<F5>(sig);
        addParamType<F6>(sig);
        constFlag(sig);
        finish(sig);
        return sig;
    }

    template< typename Ret, typename F1, typename F2, typename F3, typename F4, typename F5, typename F6 >
    INLINE static FunctionSignature capture(Ret(_class::* func)(F1, F2, F3, F4, F5, F6) const)
    {
        FunctionSignature sig;
        returnType<Ret>(sig);
        addParamType<F1>(sig);
        addParamType<F2>(sig);
        addParamType<F3>(sig);
        addParamType<F4>(sig);
        addParamType<F5>(sig);
        addParamType<F6>(sig);
        constFlag(sig);
        finish(sig);
        return sig;
    }

    //-- Param Count: 7

    template< typename F1, typename F2, typename F3, typename F4, typename F5, typename F6, typename F7 >
    INLINE static FunctionSignature capture(void(_class::* func)(F1, F2, F3, F4, F5, F6, F7))
    {
        FunctionSignature sig;
        addParamType<F1>(sig);
        addParamType<F2>(sig);
        addParamType<F3>(sig);
        addParamType<F4>(sig);
        addParamType<F5>(sig);
        addParamType<F6>(sig);
        addParamType<F7>(sig);
        finish(sig);
        return sig;
    }

    template< typename Ret, typename F1, typename F2, typename F3, typename F4, typename F5, typename F6, typename F7 >
    INLINE static FunctionSignature capture(Ret(_class::* func)(F1, F2, F3, F4, F5, F6, F7))
    {
        FunctionSignature sig;
        returnType<Ret>(sig);
        addParamType<F1>(sig);
        addParamType<F2>(sig);
        addParamType<F3>(sig);
        addParamType<F4>(sig);
        addParamType<F5>(sig);
        addParamType<F6>(sig);
        addParamType<F7>(sig);
        finish(sig);
        return sig;
    }

    template< typename F1, typename F2, typename F3, typename F4, typename F5, typename F6, typename F7 >
    INLINE static FunctionSignature capture(void(_class::* func)(F1, F2, F3, F4, F5, F6, F7) const)
    {
        FunctionSignature sig;
        addParamType<F1>(sig);
        addParamType<F2>(sig);
        addParamType<F3>(sig);
        addParamType<F4>(sig);
        addParamType<F5>(sig);
        addParamType<F6>(sig);
        addParamType<F7>(sig);
        constFlag(sig);
        finish(sig);
        return sig;
    }

    template< typename Ret, typename F1, typename F2, typename F3, typename F4, typename F5, typename F6, typename F7 >
    INLINE static FunctionSignature capture(Ret(_class::* func)(F1, F2, F3, F4, F5, F6, F7) const)
    {
        FunctionSignature sig;
        returnType<Ret>(sig);
        addParamType<F1>(sig);
        addParamType<F2>(sig);
        addParamType<F3>(sig);
        addParamType<F4>(sig);
        addParamType<F5>(sig);
        addParamType<F6>(sig);
        addParamType<F7>(sig);
        constFlag(sig);
        finish(sig);
        return sig;
    }

private:
    FunctionSignatureBuilder* m_builder;
};

//---

// helper class that can build a function call proxy for static function
class FunctionSignatureBuilderStatic : public FunctionSignatureBuilder
{
public:
    //-- Param Count: 0

    INLINE static FunctionSignature capture(void(*func)())
    {
        FunctionSignature sig;
        staticFlag(sig);
        finish(sig);
        return sig;
    }

    template< typename Ret >
    INLINE static FunctionSignature capture(Ret(*func)())
    {
        FunctionSignature sig;
        returnType<Ret>(sig);
        staticFlag(sig);
        finish(sig);
        return sig;
    }

    //-- Param Count: 1

    template< typename F1 >
    INLINE static FunctionSignature capture(void(*func)(F1))
    {
        FunctionSignature sig;
        addParamType<F1>(sig);
        staticFlag(sig);
        finish(sig);
        return sig;
    }

    template< typename Ret, typename F1 >
    INLINE static FunctionSignature capture(Ret(*func)(F1))
    {
        FunctionSignature sig;
        returnType<Ret>(sig);
        addParamType<F1>(sig);
        staticFlag(sig);
        finish(sig);
        return sig;
    }

    //-- Param Count: 2

    template< typename F1, typename F2 >
    INLINE static FunctionSignature capture(void(*func)(F1, F2))
    {
        FunctionSignature sig;
        addParamType<F1>(sig);
        addParamType<F2>(sig);
        staticFlag(sig);
        finish(sig);
        return sig;
    }

    template< typename Ret, typename F1, typename F2 >
    INLINE static FunctionSignature capture(Ret(*func)(F1, F2))
    {
        FunctionSignature sig;
        returnType<Ret>(sig);
        addParamType<F1>(sig);
        addParamType<F2>(sig);
        staticFlag(sig);
        finish(sig);
        return sig;
    }

    //-- Param Count: 3

    template< typename F1, typename F2, typename F3 >
    INLINE static FunctionSignature capture(void(*func)(F1, F2, F3))
    {
        FunctionSignature sig;
        addParamType<F1>(sig);
        addParamType<F2>(sig);
        addParamType<F3>(sig);
        staticFlag(sig);
        finish(sig);
        return sig;
    }

    template< typename Ret, typename F1, typename F2, typename F3 >
    INLINE static FunctionSignature capture(Ret(*func)(F1, F2, F3))
    {
        FunctionSignature sig;
        returnType<Ret>(sig);
        addParamType<F1>(sig);
        addParamType<F2>(sig);
        addParamType<F3>(sig);
        staticFlag(sig);
        finish(sig);
        return sig;
    }

    //-- Param Count: 4

    template< typename F1, typename F2, typename F3, typename F4 >
    INLINE static FunctionSignature capture(void(*func)(F1, F2, F3, F4))
    {
        FunctionSignature sig;
        addParamType<F1>(sig);
        addParamType<F2>(sig);
        addParamType<F3>(sig);
        addParamType<F4>(sig);
        staticFlag(sig);
        finish(sig);
        return sig;
    }

    template< typename Ret, typename F1, typename F2, typename F3, typename F4 >
    INLINE static FunctionSignature capture(Ret(*func)(F1, F2, F3, F4))
    {
        FunctionSignature sig;
        returnType<Ret>(sig);
        addParamType<F1>(sig);
        addParamType<F2>(sig);
        addParamType<F3>(sig);
        addParamType<F4>(sig);
        staticFlag(sig);
        finish(sig);
        return sig;
    }

    //-- Param Count: 5

    template< typename F1, typename F2, typename F3, typename F4, typename F5 >
    INLINE static FunctionSignature capture(void(*func)(F1, F2, F3, F4, F5))
    {
        FunctionSignature sig;
        addParamType<F1>(sig);
        addParamType<F2>(sig);
        addParamType<F3>(sig);
        addParamType<F4>(sig);
        addParamType<F5>(sig);
        staticFlag(sig);
        finish(sig);
        return sig;
    }

    template< typename Ret, typename F1, typename F2, typename F3, typename F4, typename F5 >
    INLINE static FunctionSignature capture(Ret(*func)(F1, F2, F3, F4, F5))
    {
        FunctionSignature sig;
        returnType<Ret>(sig);
        addParamType<F1>(sig);
        addParamType<F2>(sig);
        addParamType<F3>(sig);
        addParamType<F4>(sig);
        addParamType<F5>(sig);
        staticFlag(sig);
        finish(sig);
        return sig;
    }

    //-- Param Count: 6

    template< typename F1, typename F2, typename F3, typename F4, typename F5, typename F6 >
    INLINE static FunctionSignature capture(void(*func)(F1, F2, F3, F4, F5, F6))
    {
        FunctionSignature sig;
        addParamType<F1>(sig);
        addParamType<F2>(sig);
        addParamType<F3>(sig);
        addParamType<F4>(sig);
        addParamType<F5>(sig);
        addParamType<F6>(sig);
        staticFlag(sig);
        finish(sig);
        return sig;
    }

    template< typename Ret, typename F1, typename F2, typename F3, typename F4, typename F5, typename F6 >
    INLINE static FunctionSignature capture(Ret(*func)(F1, F2, F3, F4, F5, F6))
    {
        FunctionSignature sig;
        returnType<Ret>(sig);
        addParamType<F1>(sig);
        addParamType<F2>(sig);
        addParamType<F3>(sig);
        addParamType<F4>(sig);
        addParamType<F5>(sig);
        addParamType<F6>(sig);
        staticFlag(sig);
        finish(sig);
        return sig;
    }
};

//--

template <typename... Args>
struct NativeFunctionShimParamOffsetCalculator;

template <typename First, typename... Args>
struct NativeFunctionShimParamOffsetCalculator<First, Args...>
{
    constexpr static uint32_t setup(uint32_t offset)
    {
        if (std::is_reference<First>::value || std::is_pointer<First>::value)
        {
            //offset = offset & ~(alignof(void*) - 1);
            offset += sizeof(void*);
        }
        else
        {
            offset = offset & ~(alignof(First) - 1);
            offset += sizeof(First);
        }

        return NativeFunctionShimParamOffsetCalculator<Args...>::setup(offset);
    }
};

template <>
struct NativeFunctionShimParamOffsetCalculator<>
{
    constexpr static uint32_t setup(uint32_t prevOffset)
    {
        return prevOffset;
    }
};

template <typename T>
struct NativeFunctionShimParamFetch
{
    INLINE static T Fetch(const void* ptr)
    {
        return *(const T*)ptr;
    }
};

template <typename T>
struct NativeFunctionShimParamFetch<T&>
{
    INLINE static T& Fetch(const void* ptr)
    {
        return *(T*)ptr;
    }
};

template <typename T>
struct NativeFunctionShimParamFetch<const T&>
{
    INLINE static const T& Fetch(const void* ptr)
    {
        const void* refPtr = *(const void**)ptr;
        return *(T*)refPtr;
    }
};

template <typename T, typename... Args>
INLINE T NativeFunctionShimParamPtr(const void* ptr)
{
    constexpr auto offset = NativeFunctionShimParamOffsetCalculator<Args...>::setup(0) & ~(alignof(T) - 1);
    return NativeFunctionShimParamFetch<T>::Fetch((const char*)ptr + offset);
}

//--

template< typename _class, typename FuncType, FuncType funcPtr >
class NativeFunctionShimBuilderClass : public MainPoolData<NoCopy>
{
public:
    //-- Param Count: 0

    static INLINE TFunctionWrapperPtr buildShim(void(_class::* func)())
    {
        return [](void* contextPtr, const void* params, void* result)
        {
            auto classPtr = (_class*)contextPtr;
            (classPtr->*funcPtr)();
        };
    }

    static INLINE TFunctionWrapperPtr buildShim(void(_class::* func)() const)
    {
        return [](void* contextPtr, const void* params, void* result)
        {
            auto classPtr = (const _class*)contextPtr;
            (classPtr->*funcPtr)();
        };
    }

    template <typename Ret>
    static INLINE TFunctionWrapperPtr buildShim(Ret(_class::* func)())
    {
        return [](void* contextPtr, const void* params, void* result)
        {
            auto classPtr = (_class*)contextPtr;
            *(Ret*)result = ((classPtr->*funcPtr)());
        };
    }

    template <typename Ret>
    static INLINE TFunctionWrapperPtr buildShim(Ret(_class::* func)() const)
    {
        return [](void* contextPtr, const void* params, void* result)
        {
            auto classPtr = (const _class*)contextPtr;
            *(Ret*)result = ((classPtr->*funcPtr)());
        };
    }

    //-- Param Count: 1

    template< typename F1 >
    static INLINE TFunctionWrapperPtr buildShim(void(_class::* func)(F1))
    {
        return [](void* contextPtr, const void* params, void* result)
        {
            auto classPtr = (_class*)contextPtr;
            (classPtr->*funcPtr)(NativeFunctionShimParamPtr<F1>(params));
        };
    }

    template< typename Ret, typename F1 >
    static INLINE TFunctionWrapperPtr buildShim(Ret(_class::* func)(F1))
    {
        return [](void* contextPtr, const void* params, void* result)
        {
            auto classPtr = (_class*)contextPtr;
            *(Ret*)result = ((classPtr->*funcPtr)(NativeFunctionShimParamPtr<F1>(params)));
        };
    }

    template< typename F1 >
    static INLINE TFunctionWrapperPtr buildShim(void(_class::* func)(F1) const)
    {
        return [](void* contextPtr, const void* params, void* result)
        {
            auto classPtr = (const _class*)contextPtr;
            (classPtr->*funcPtr)(NativeFunctionShimParamPtr<F1>(params));
        };
    }

    template< typename Ret, typename F1 >
    static INLINE TFunctionWrapperPtr buildShim(Ret(_class::* func)(F1) const)
    {
        return [](void* contextPtr, const void* params, void* result)
        {
            auto classPtr = (const _class*)contextPtr;
            *(Ret*)result = ((classPtr->*funcPtr)(NativeFunctionShimParamPtr<F1>(params)));
        };
    }

    //-- Param Count: 2

    template< typename F1, typename F2 >
    static INLINE TFunctionWrapperPtr buildShim(void(_class::* func)(F1, F2))
    {
        return [](void* contextPtr, const void* params, void* result)
        {            
            auto classPtr = (_class*)contextPtr;
            (classPtr->*funcPtr)(NativeFunctionShimParamPtr<F1>(params), NativeFunctionShimParamPtr<F2, F1>(params));
        };
    }

    template< typename Ret, typename F1, typename F2 >
    static INLINE TFunctionWrapperPtr buildShim(Ret(_class::* func)(F1, F2))
    {
        return [](void* contextPtr, const void* params, void* result)
        {
            auto classPtr = (_class*)contextPtr;
            *(Ret*)result = (classPtr->*funcPtr)(NativeFunctionShimParamPtr<F1>(params), NativeFunctionShimParamPtr<F2, F1>(params));
        };
    }

    template< typename F1, typename F2 >
    static INLINE TFunctionWrapperPtr buildShim(void(_class::* func)(F1, F2) const)
    {
        return [](void* contextPtr, const void* params, void* result)
        {
            auto classPtr = (const _class*)contextPtr;
            (classPtr->*funcPtr)(NativeFunctionShimParamPtr<F1>(params), NativeFunctionShimParamPtr<F2, F1>(params));
        };
    }

    template< typename Ret, typename F1, typename F2 >
    static INLINE TFunctionWrapperPtr buildShim(Ret(_class::* func)(F1, F2) const)
    {
        return [](void* contextPtr, const void* params, void* result)
        {
            auto classPtr = (const _class*)contextPtr;
            *(Ret*)result = (classPtr->*funcPtr)(NativeFunctionShimParamPtr<F1>(params), NativeFunctionShimParamPtr<F2, F1>(params));
        };
    }

    //-- Param Count: 3

    template< typename F1, typename F2, typename F3 >
    static INLINE TFunctionWrapperPtr buildShim(void(_class::* func)(F1, F2, F3))
    {
        return [](void* contextPtr, const void* params, void* result)
        {
            auto classPtr = (_class*)contextPtr;
            (classPtr->*funcPtr)(
                NativeFunctionShimParamPtr<F1>(params),
                NativeFunctionShimParamPtr<F2, F1>(params),
                NativeFunctionShimParamPtr<F3, F2, F1>(params));
        };
    }

    template< typename Ret, typename F1, typename F2, typename F3 >
    static INLINE TFunctionWrapperPtr buildShim(Ret(_class::* func)(F1, F2, F3))
    {
        return [](void* contextPtr, const void* params, void* result)
        {
            auto classPtr = (_class*)contextPtr;
            *(Ret*)result = (classPtr->*funcPtr)(
                NativeFunctionShimParamPtr<F1>(params),
                NativeFunctionShimParamPtr<F2, F1>(params),
                NativeFunctionShimParamPtr<F3, F2, F1>(params));
        };
    }

    template< typename F1, typename F2, typename F3 >
    static INLINE TFunctionWrapperPtr buildShim(void(_class::* func)(F1, F2, F3) const)
    {
        return [](void* contextPtr, const void* params, void* result)
        {
            auto classPtr = (const _class*)contextPtr;
            (classPtr->*funcPtr)(
                NativeFunctionShimParamPtr<F1>(params),
                NativeFunctionShimParamPtr<F2, F1>(params),
                NativeFunctionShimParamPtr<F3, F2, F1>(params));
        };
    }

    template< typename Ret, typename F1, typename F2, typename F3 >
    static INLINE TFunctionWrapperPtr buildShim(Ret(_class::* func)(F1, F2, F3) const)
    {
        return [](void* contextPtr, const void* params, void* result)
        {
            auto classPtr = (const _class*)contextPtr;
            *(Ret*)result = (classPtr->*funcPtr)(
                NativeFunctionShimParamPtr<F1>(params),
                NativeFunctionShimParamPtr<F2, F1>(params),
                NativeFunctionShimParamPtr<F3, F2, F1>(params));
        };
    }

    //-- Param Count: 4

    template< typename F1, typename F2, typename F3, typename F4 >
    static INLINE TFunctionWrapperPtr buildShim(void(_class::* func)(F1, F2, F3, F4))
    {
        return [](void* contextPtr, const void* params, void* result)
        {
            auto classPtr = (_class*)contextPtr;
            (classPtr->*funcPtr)(
                NativeFunctionShimParamPtr<F1>(params),
                NativeFunctionShimParamPtr<F2, F1>(params),
                NativeFunctionShimParamPtr<F3, F2, F1>(params),
                NativeFunctionShimParamPtr<F4, F3, F2, F1>(params));
        };
    }

    template< typename Ret, typename F1, typename F2, typename F3, typename F4 >
    static INLINE TFunctionWrapperPtr buildShim(Ret(_class::* func)(F1, F2, F3, F4))
    {
        return [](void* contextPtr, const void* params, void* result)
        {
            auto classPtr = (_class*)contextPtr;
            *(Ret*)result = (classPtr->*funcPtr)(
                NativeFunctionShimParamPtr<F1>(params),
                NativeFunctionShimParamPtr<F2, F1>(params),
                NativeFunctionShimParamPtr<F3, F2, F1>(params),
                NativeFunctionShimParamPtr<F4, F3, F2, F1>(params));
        };
    }

    template< typename F1, typename F2, typename F3, typename F4 >
    static INLINE TFunctionWrapperPtr buildShim(void(_class::* func)(F1, F2, F3, F4) const)
    {
        return [](void* contextPtr, const void* params, void* result)
        {
            auto classPtr = (const _class*)contextPtr;
            (classPtr->*funcPtr)(
                NativeFunctionShimParamPtr<F1>(params),
                NativeFunctionShimParamPtr<F2, F1>(params),
                NativeFunctionShimParamPtr<F3, F2, F1>(params),
                NativeFunctionShimParamPtr<F4, F3, F2, F1>(params));
        };
    }

    template< typename Ret, typename F1, typename F2, typename F3, typename F4 >
    static INLINE TFunctionWrapperPtr buildShim(Ret(_class::* func)(F1, F2, F3, F4) const)
    {
        return [](void* contextPtr, const void* params, void* result)
        {
            auto classPtr = (const _class*)contextPtr;
            *(Ret*)result = (classPtr->*funcPtr)(
                NativeFunctionShimParamPtr<F1>(params),
                NativeFunctionShimParamPtr<F2, F1>(params),
                NativeFunctionShimParamPtr<F3, F2, F1>(params),
                NativeFunctionShimParamPtr<F4, F3, F2, F1>(params));
        };
    }

    //-- Param Count: 5

    template< typename F1, typename F2, typename F3, typename F4, typename F5 >
    static INLINE TFunctionWrapperPtr buildShim(void(_class::* func)(F1, F2, F3, F4, F5))
    {
        return [](void* contextPtr, const void* params, void* result)
        {
            auto classPtr = (_class*)contextPtr;
            (classPtr->*funcPtr)(
                NativeFunctionShimParamPtr<F1>(params),
                NativeFunctionShimParamPtr<F2, F1>(params),
                NativeFunctionShimParamPtr<F3, F2, F1>(params),
                NativeFunctionShimParamPtr<F4, F3, F2, F1>(params),
                NativeFunctionShimParamPtr<F5, F4, F3, F2, F1>(params));
        };
    }

    template< typename Ret, typename F1, typename F2, typename F3, typename F4, typename F5 >
    static INLINE TFunctionWrapperPtr buildShim(Ret(_class::* func)(F1, F2, F3, F4, F5))
    {
        return [](void* contextPtr, const void* params, void* result)
        {
            auto classPtr = (_class*)contextPtr;
            *(Ret*)result = (classPtr->*funcPtr)(
                NativeFunctionShimParamPtr<F1>(params),
                NativeFunctionShimParamPtr<F2, F1>(params),
                NativeFunctionShimParamPtr<F3, F2, F1>(params),
                NativeFunctionShimParamPtr<F4, F3, F2, F1>(params),
                NativeFunctionShimParamPtr<F5, F4, F3, F2, F1>(params));
        };
    }

    template< typename F1, typename F2, typename F3, typename F4, typename F5 >
    static INLINE TFunctionWrapperPtr buildShim(void(_class::* func)(F1, F2, F3, F4, F5) const)
    {
        return [](void* contextPtr, const void* params, void* result)
        {
            auto classPtr = (const _class*)contextPtr;
            (classPtr->*funcPtr)(
                NativeFunctionShimParamPtr<F1>(params),
                NativeFunctionShimParamPtr<F2, F1>(params),
                NativeFunctionShimParamPtr<F3, F2, F1>(params),
                NativeFunctionShimParamPtr<F4, F3, F2, F1>(params),
                NativeFunctionShimParamPtr<F5, F4, F3, F2, F1>(params));
        };
    }

    template< typename Ret, typename F1, typename F2, typename F3, typename F4, typename F5 >
    static INLINE TFunctionWrapperPtr buildShim(Ret(_class::* func)(F1, F2, F3, F4, F5) const)
    {
        return [](void* contextPtr, const void* params, void* result)
        {
            auto classPtr = (const _class*)contextPtr;
            *(Ret*)result = (classPtr->*funcPtr)(
                NativeFunctionShimParamPtr<F1>(params),
                NativeFunctionShimParamPtr<F2, F1>(params),
                NativeFunctionShimParamPtr<F3, F2, F1>(params),
                NativeFunctionShimParamPtr<F4, F3, F2, F1>(params),
                NativeFunctionShimParamPtr<F5, F4, F3, F2, F1>(params));
        };
    }

    //--
};

//--

template< typename FuncType, FuncType funcPtr >
class NativeFunctionShimBuilderStatic : public MainPoolData<NoCopy>
{
public:
    static INLINE TFunctionWrapperPtr buildShim(void(*func)())
    {
        return [](void* contextPtr, const void* params, void* result)
        {
            (*funcPtr)();
        };
    }

    template< typename Ret >
    static INLINE TFunctionWrapperPtr buildShim(Ret(*func)())
    {
        return [](void* contextPtr, const void* params, void* result)
        {
            *(Ret*)result = (*funcPtr)();
        };
    }

    //-- Param Count: 1

    template< typename F1 >
    static INLINE TFunctionWrapperPtr buildShim(void(*func)(F1))
    {
        return [](void* contextPtr, const void* params, void* result)
        {
            (*funcPtr)(NativeFunctionShimParamPtr<F1>(params));
        };
    }

    template< typename Ret, typename F1 >
    static INLINE TFunctionWrapperPtr buildShim(Ret(*func)(F1))
    {
        return [](void* contextPtr, const void* params, void* result)
        {
            *(Ret*)result = (*funcPtr)(NativeFunctionShimParamPtr<F1>(params));
        };
    }

    //-- Param Count: 2

    template< typename F1, typename F2 >
    static INLINE TFunctionWrapperPtr buildShim(void(*func)(F1, F2))
    {
        return [](void* contextPtr, const void* params, void* result)
        {
            (*funcPtr)(
                NativeFunctionShimParamPtr<F1>(params),
                NativeFunctionShimParamPtr<F2, F1>(params));
        };
    }

    template< typename Ret, typename F1, typename F2 >
    static INLINE TFunctionWrapperPtr buildShim(Ret(*func)(F1, F2))
    {
        return [](void* contextPtr, const void* params, void* result)
        {
            *(Ret*)result = (*funcPtr)(
                NativeFunctionShimParamPtr<F1>(params),
                NativeFunctionShimParamPtr<F2, F1>(params));
        };
    }

    //-- Param Count: 3

    template< typename F1, typename F2, typename F3 >
    static INLINE TFunctionWrapperPtr buildShim(void(*func)(F1, F2, F3))
    {
        return [](void* contextPtr, const void* params, void* result)
        {
            (*funcPtr)(
                NativeFunctionShimParamPtr<F1>(params),
                NativeFunctionShimParamPtr<F2, F1>(params),
                NativeFunctionShimParamPtr<F3, F2, F1>(params));
        };
    }

    template< typename Ret, typename F1, typename F2, typename F3 >
    static INLINE TFunctionWrapperPtr buildShim(Ret(*func)(F1, F2, F3))
    {
        return [](void* contextPtr, const void* params, void* result)
        {
            *(Ret*)result = (*funcPtr)(
                NativeFunctionShimParamPtr<F1>(params),
                NativeFunctionShimParamPtr<F2, F1>(params),
                NativeFunctionShimParamPtr<F3, F2, F1>(params));
        };
    }

    //-- Param Count: 4

    template< typename F1, typename F2, typename F3, typename F4 >
    static INLINE TFunctionWrapperPtr buildShim(void(*func)(F1, F2, F3, F4))
    {
        return [](void* contextPtr, const void* params, void* result)
        {
            (*funcPtr)(
                NativeFunctionShimParamPtr<F1>(params),
                NativeFunctionShimParamPtr<F2, F1>(params),
                NativeFunctionShimParamPtr<F3, F2, F1>(params),
                NativeFunctionShimParamPtr<F4, F3, F2, F1>(params));
        };
    }

    template< typename Ret, typename F1, typename F2, typename F3, typename F4 >
    static INLINE TFunctionWrapperPtr buildShim(Ret(*func)(F1, F2, F3, F4))
    {
        return [](void* contextPtr, const void* params, void* result)
        {   
            *(Ret*)result = (*funcPtr)(
                NativeFunctionShimParamPtr<F1>(params),
                NativeFunctionShimParamPtr<F2, F1>(params),
                NativeFunctionShimParamPtr<F3, F2, F1>(params),
                NativeFunctionShimParamPtr<F4, F3, F2, F1>(params));
        };
    }

    //-- Param Count: 5

    template< typename F1, typename F2, typename F3, typename F4, typename F5 >
    static INLINE TFunctionWrapperPtr buildShim(void(*func)(F1, F2, F3, F4, F5))
    {
        return [](void* contextPtr, const void* params, void* result)
        {
            (*funcPtr)(
                NativeFunctionShimParamPtr<F1>(params),
                NativeFunctionShimParamPtr<F2, F1>(params),
                NativeFunctionShimParamPtr<F3, F2, F1>(params),
                NativeFunctionShimParamPtr<F4, F3, F2, F1>(params),
                NativeFunctionShimParamPtr<F5, F4, F3, F2, F1>(params));
        };
    }

    template< typename Ret, typename F1, typename F2, typename F3, typename F4, typename F5 >
    static INLINE TFunctionWrapperPtr buildShim(Ret(*func)(F1, F2, F3, F4, F5))
    {
        return [](void* contextPtr, const void* params, void* result)
        {
            *(Ret*)result = (*funcPtr)(
                NativeFunctionShimParamPtr<F1>(params),
                NativeFunctionShimParamPtr<F2, F1>(params),
                NativeFunctionShimParamPtr<F3, F2, F1>(params),
                NativeFunctionShimParamPtr<F4, F3, F2, F1>(params),
                NativeFunctionShimParamPtr<F5, F4, F3, F2, F1>(params));
        };
    }

    //-- Param Count: 6

    template< typename F1, typename F2, typename F3, typename F4, typename F5, typename F6 >
    static INLINE TFunctionWrapperPtr buildShim(void(*func)(F1, F2, F3, F4, F5, F6))
    {
        return [](void* contextPtr, const void* params, void* result)
        {
            (*funcPtr)(
                NativeFunctionShimParamPtr<F1>(params),
                NativeFunctionShimParamPtr<F2, F1>(params),
                NativeFunctionShimParamPtr<F3, F2, F1>(params),
                NativeFunctionShimParamPtr<F4, F3, F2, F1>(params),
                NativeFunctionShimParamPtr<F5, F4, F3, F2, F1>(params),
                NativeFunctionShimParamPtr<F6, F5, F4, F3, F2, F1>(params));
        };
    }

    template< typename Ret, typename F1, typename F2, typename F3, typename F4, typename F5, typename F6 >
    static INLINE TFunctionWrapperPtr buildShim(Ret(*func)(F1, F2, F3, F4, F5, F6))
    {
        return [](void* contextPtr, const void* params, void* result)
        {   
            *(Ret*)result = (*funcPtr)(
                NativeFunctionShimParamPtr<F1>(params),
                NativeFunctionShimParamPtr<F2, F1>(params),
                NativeFunctionShimParamPtr<F3, F2, F1>(params),
                NativeFunctionShimParamPtr<F4, F3, F2, F1>(params),
                NativeFunctionShimParamPtr<F5, F4, F3, F2, F1>(params),
                NativeFunctionShimParamPtr<F6, F5, F4, F3, F2, F1>(params));
        };
    }

};

//--

END_INFERNO_NAMESPACE()
