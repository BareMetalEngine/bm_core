/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//----

// global configuration property
class BM_CORE_OBJECT_API IConfigProperty : public MainPoolData<NoCopy>
{
public:
    // supported flags: 
    //  D - dev (visible only in dev mode)
    //  R - read only (property can't be changed at runtime)
    IConfigProperty(StringView groupName, StringView name, StringView flags="");
    virtual ~IConfigProperty();

    /// get the group name (config group)
    INLINE StringView group() const { return m_group; }

    /// get the property name
    INLINE StringID name() const { return m_name; }

    /// is this config value modifiable only from console?
    INLINE bool readOnly() const { return m_readOnly; }

    /// is this config value hidden from normal listings
    INLINE bool developer() const { return m_developer; }

    ///--

    /// parse and store local data from string, called during config load
    /// NOTE: we may have more than one element (in case of array)
    virtual bool applyFromString(const Array<StringBuf>& values) = 0;

    /// capture current data to string
    virtual void captureToString(Array<StringBuf>& outValues) const = 0;

    ///--

    /// send current value of this property to config storage
    void sendToStorage();

    /// read value of this property from config storage (if found)
    bool readFromStorage();

    ///--

protected:
    StringView m_group;
    StringID m_name;

    bool m_readOnly : 1;
    bool m_developer : 1;
};

//----

// typed configuration property
template< typename T >
class ConfigProperty : public IConfigProperty
{
public:
    INLINE ConfigProperty(StringView groupName, StringView name, const T& value, StringView flags="")
        : IConfigProperty(groupName, name, flags)
        , m_value(value)
    {}

    INLINE const T& get() const
    {
        return m_value;
    }

    INLINE T& get()
    {
        return m_value;
    }

    INLINE void set(const T& newValue)
    {
        if (m_value != newValue)
        {
            m_value = newValue;
            sendToStorage();
        }
    }

protected:
    T m_value;

    virtual bool applyFromString(const Array<StringBuf>& values) override final
    {
        return IType::ApplyFromString(GetTypeObject<T>().ptr(), &m_value, values);
    }

    virtual void captureToString(Array<StringBuf>& outValues) const override final
    {
        IType::CaptureToString(GetTypeObject<T>().ptr(), &m_value, outValues);
    }
};

//----
    
END_INFERNO_NAMESPACE()
