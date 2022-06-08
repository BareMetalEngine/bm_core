/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//---

/// formatting stream
class BM_CORE_SYSTEM_API IFormatStream : public NoCopy
{
public:
    virtual ~IFormatStream();

    // append number of chars to the stream
    virtual IFormatStream& append(const char* str, uint32_t len = INDEX_MAX) = 0;

    // append wide-char stream
    virtual IFormatStream& append(const wchar_t* str, uint32_t len = INDEX_MAX);

    //---

    // append any printable type to stream, uses the printers
    template< typename T >
    INLINE IFormatStream& appendT(T& data);

    // append any printable type to stream, uses the printers
    template< typename... Args>
    INLINE IFormatStream& appendf(const char* pos, const Args&... args)
    {
        innerFormatter(pos, args...);
        return *this;
    }

    //---

    // add single character
    INLINE IFormatStream& appendch(char ch)
    {
        char str[2] = {ch,0};
        return append(str);
    }

    // add wide character (encodes it in UTF8)
    IFormatStream& appendUTF32(uint32_t ch);

    // append padding string
    IFormatStream& appendPadding(char ch, uint32_t count);

    //---

    // stream like writer
    template< typename T >
    INLINE friend IFormatStream& operator << (IFormatStream& s, const T& value) { return s.appendT(value); }

    //---

    // default NULL stream
    static IFormatStream& NullStream();

    //---

	// append numbers in various formats
	static void PrintNumber(IFormatStream& f, char sign, uint64_t val, char padding=0, int wholeDigits=-1);
	static void PrintNumberF(IFormatStream& f, char sign, double val, char padding=0, int wholeDigits= -1, int fractionDigits=-1);
    static void PrintPreciseNumber(IFormatStream& f, float val); // prints exactly the amount of digits to preserve BIT PERFECT precision
    static void PrintPreciseNumber(IFormatStream& f, double val); // prints exactly the amount of digits to preserve BIT PERFECT precision
    static void PrintPointer(IFormatStream& f, const void* ptr);
    static void PrintHexData(IFormatStream& f, const void* ptr, uint32_t size);
    static void PrintHexNumber(IFormatStream& f, uint64_t value, char paddingChar=0, int wholeDigits=-1);
    static void PrintMemorySize(IFormatStream& f, uint64_t value);
    static void PrintTimeInterval(IFormatStream& f, double value);

    //--

protected:
    // consume and print the string buffer, stop on format argument {}
    bool consumeFormatString(const char*& pos);

    //--

    INLINE void innerFormatter(const char*& pos)
    {
        while (consumeFormatString(pos))
        {
            append("<undefined>");
            innerFormatter(pos);
        }
    }

    template< typename T, typename... Args>
    INLINE void innerFormatter(const char*& pos, const T& value, const Args&... args)
    {
        while (consumeFormatString(pos))
        {
            appendT(value);
            innerFormatter(pos, args...);
        }
    }
};

//----

template<typename T, typename Enable = void>
struct PrintableConverter
{
	static void Print(IFormatStream& s, const T& val)
	{
        s << "<unknown type>";
	}
};

template<typename T>
struct has_csrt_method
{
private:
    typedef char YesType[1];
    typedef char NoType[2];

    template <typename C> static YesType& test( decltype(&C::c_str) ) ;
    template <typename C> static NoType& test(...);


public:
    enum { value = sizeof(test<T>(0)) == sizeof(YesType) };
};

template<typename T>
struct has_print_method
{
private:
    typedef char YesType[1];
    typedef char NoType[2];

    template <typename C> static YesType& test( decltype(&C::print) ) ;
    template <typename C> static NoType& test(...);


public:
    enum { value = sizeof(test<T>(0)) == sizeof(YesType) };
};


template<typename T>
struct has_print_ex_method
{
private:
	typedef char YesType[1];
	typedef char NoType[2];

	template <typename C> static YesType& test(decltype(&C::printEx));
	template <typename C> static NoType& test(...);


public:
	enum { value = sizeof(test<T>(0)) == sizeof(YesType) };
};

template<typename T>
struct has_Print_method
{
private:
    typedef char YesType[1];
    typedef char NoType[2];

    template <typename C> static YesType& test( decltype(&C::Print) ) ;
    template <typename C> static NoType& test(...);


public:
    enum { value = sizeof(test<T>(0)) == sizeof(YesType) };
};

template<typename T>
struct PrintableConverter<T, typename std::enable_if<has_print_method<T>::value>::type >
{
    static void Print(IFormatStream& s, const T& val)
    {
        val.print(s);
    }
};

template<typename T>
struct PrintableConverter<T, typename std::enable_if<has_print_ex_method<T>::value>::type >
{
	static void Print(IFormatStream& s, const T& val)
	{
		val.printEx(s, format);
	}
};

template<typename T>
struct PrintableConverter<T, typename std::enable_if<has_csrt_method<T>::value && !has_print_method<T>::value>::type >
{
    static void Print(IFormatStream& s, const T& val)
    {
        s.append(val.c_str());
    }
};

template<> struct PrintableConverter<bool>
{
    static void Print(IFormatStream& s, bool val)
    {
        s << (val ? "true" : "false");
    }
};

template<> struct PrintableConverter<int8_t>
{
    static void Print(IFormatStream& s, int8_t val)
    {
        IFormatStream::PrintNumber(s, '-', -val);
    }
};

template<> struct PrintableConverter<char>
{
	static void Print(IFormatStream& s, int8_t val)
	{
        if (val < 0)
			IFormatStream::PrintNumber(s, '-', -val);
		else
			IFormatStream::PrintNumber(s, 0, val);
	}
};


template<> struct PrintableConverter<int16_t>
{
    static void Print(IFormatStream& s, int16_t val)
    {
		if (val < 0)
			IFormatStream::PrintNumber(s, '-', -val);
		else
			IFormatStream::PrintNumber(s, 0, val);
    }
};

/*template<> struct PrintableConverter<short>
{
	static void Print(IFormatStream& s, int16_t val, const FormatSpecifier& format)
	{
		if (format.matchKeyword("x"))
			IFormatStream::PrintHexNumber(s, (uint16_t)val, format);
		else if (val < 0)
			IFormatStream::PrintNumber(s, '-', -val, format);
		else
			IFormatStream::PrintNumber(s, 0, val, format);
	}
};*/


template<> struct PrintableConverter<int32_t>
{
    static void Print(IFormatStream& s, int32_t val)
    {
        if (val < 0)
			IFormatStream::PrintNumber(s, '-', -val);
		else
			IFormatStream::PrintNumber(s, 0, val);
    }
};

/*template<> struct PrintableConverter<int>
{
	static void Print(IFormatStream& s, int32_t val, const FormatSpecifier& format)
	{
		if (format.matchKeyword("x"))
			IFormatStream::PrintHexNumber(s, (uint32_t)val, format);
		else if (val < 0)
			IFormatStream::PrintNumber(s, '-', -val, format);
		else
			IFormatStream::PrintNumber(s, 0, val, format);
	}
};*/

template<> struct PrintableConverter<int64_t>
{
    static void Print(IFormatStream& s, int64_t val)
    {
        if (val < 0)
			IFormatStream::PrintNumber(s, '-', -val);
		else
			IFormatStream::PrintNumber(s, 0, val);
    }
};

template<> struct PrintableConverter<uint8_t>
{
    static void Print(IFormatStream& s, uint8_t val)
    {
        IFormatStream::PrintNumber(s, 0, val);
    }
};

template<> struct PrintableConverter<uint16_t>
{
    static void Print(IFormatStream& s, uint16_t val)
    {
        IFormatStream::PrintNumber(s, 0, val);
    }
};

template<> struct PrintableConverter<uint32_t>
{
    static void Print(IFormatStream& s, uint32_t val)
    {
        IFormatStream::PrintNumber(s, 0, val);
    }
};

template<> struct PrintableConverter<uint64_t>
{
    static void Print(IFormatStream& s, uint64_t val)
    {
        IFormatStream::PrintNumber(s, 0, val);
    }
};

#ifdef PLATFORM_MSVC
template<> struct PrintableConverter<long unsigned int>
{
    static void Print(IFormatStream& s, long unsigned val)
    {
        IFormatStream::PrintNumber(s, 0, val);
    }
};
#endif

#ifdef PLATFORM_MSVC
template<> struct PrintableConverter<long int>
{
    static void Print(IFormatStream& s, long int val)
    {
        if (val < 0)
			IFormatStream::PrintNumber(s, '-', -val);
		else
			IFormatStream::PrintNumber(s, 0, val);
    }
};
#endif

template<> struct PrintableConverter<float>
{
    static void Print(IFormatStream& s, float val)
    {
		if (val < 0.0f)
			IFormatStream::PrintNumberF(s, '-', -val);
		else
			IFormatStream::PrintNumberF(s, 0, val);
    }
};

template<> struct PrintableConverter<double>
{
    static void Print(IFormatStream& s, double val)
    {
		if (val < 0.0f)
			IFormatStream::PrintNumberF(s, '-', -val);
		else
			IFormatStream::PrintNumberF(s, 0, val);
    }
};

template<> struct PrintableConverter<const char* const>
{
    static void Print(IFormatStream& s, const char* val)
    {
        s.append(val);
    }
};

template<> struct PrintableConverter<const wchar_t* const>
{
    static void Print(IFormatStream& s, const wchar_t* val)
    {
        s.append(val);
    }
};

template<> struct PrintableConverter<const char*>
{
    static void Print(IFormatStream& s, const char* val)
    {
        s.append(val);
    }
};

template<> struct PrintableConverter<const wchar_t*>
{
    static void Print(IFormatStream& s, const wchar_t* val)
    {
        s.append(val);
    }
};

template<> struct PrintableConverter<char*>
{
    static void Print(IFormatStream& s, const char* val)
    {
        s.append(val);
    }
};

template<> struct PrintableConverter<wchar_t*>
{
    static void Print(IFormatStream& s, const wchar_t* val)
    {
        s.append(val);
    }
};

template<int N>
struct PrintableConverter<char[N]>
{
    static void Print(IFormatStream& s, const char* val)
    {
        if (N > 1)
            s.append(val, N-1);
    }
};

template<int N>
struct PrintableConverter<wchar_t[N]>
{
    static void Print(IFormatStream& s, const wchar_t* val)
    {
        if (N > 1)
            s.append(val, N-1);
    }
};

template<typename T>
struct PrintableConverter<T*>
{
    static void Print(IFormatStream& s, T* ptr)
    {
        IFormatStream::PrintPointer(s, ptr);
    }
};


//----

template< typename T >
INLINE IFormatStream& IFormatStream::appendT(T& data)
{
    //static_assert(has_Print_method<PrintableConverter<T>>::value, "There is no printer for this type");
    PrintableConverter<typename std::remove_cv<T>::type>::Print(*this, data);
    return *this;
}

//----

template< char digits, char ch='0' >
struct PaddedInteger
{
	int64_t value = 0;

	INLINE PaddedInteger() {};

	INLINE PaddedInteger(int64_t val)
	{
        value = val;
	}

	void print(IFormatStream& f) const
	{
        if (value < 0)
            IFormatStream::PrintNumber(f, '-', -value, ch, digits);
        else
            IFormatStream::PrintNumber(f, 0, value, ch, digits);
	}
};

template< char digits, char fractionDigits, char ch = '0' >
struct PaddedFloat
{
	double value = 0;

	INLINE PaddedFloat() {};

	INLINE PaddedFloat(double val)
	{
		value = val;
	}

	void print(IFormatStream& f) const
	{
		if (value < 0)
			IFormatStream::PrintNumberF(f, '-', -value, ch, digits, fractionDigits);
		else
			IFormatStream::PrintNumberF(f, 0, value, ch, digits, fractionDigits);
	}
};

struct PreciseFloat
{
	float value = 0;

	INLINE PreciseFloat() {};

	INLINE PreciseFloat(float val)
	{
		value = val;
	}

	void print(IFormatStream& f) const
	{
        IFormatStream::PrintPreciseNumber(f, value);
	}
};

struct PreciseDouble
{
	double value = 0;

	INLINE PreciseDouble() {};

	INLINE PreciseDouble(double val)
	{
		value = val;
	}

	void print(IFormatStream& f) const
	{
		IFormatStream::PrintPreciseNumber(f, value);
	}
};


template< typename T >
struct Percent
{
    double value = 100.0;

    INLINE Percent() {};

    INLINE Percent(T count, T total)
    {
        if (total)
            value = 100.0 * (count / (double)total);
        else
            value = 0.0;
    }

    void print(IFormatStream& f) const
    {
        f.appendf("{}%", PaddedFloat<1, 2>(value));
    }
};

//----

struct TimeInterval
{
	double value = 0.0;

	INLINE TimeInterval() {};

	INLINE TimeInterval(float value_)
        : value(value_)
	{
	}

	void print(IFormatStream& f) const
	{
        IFormatStream::PrintTimeInterval(f, value);
	}
};

struct MemSize
{
	uint64_t value = 0;

	INLINE MemSize() {};

	INLINE MemSize(uint64_t value_)
		: value(value_)
	{
	}

	void print(IFormatStream& f) const
	{
		IFormatStream::PrintMemorySize(f, value);
	}
};

struct Hex
{
	uint64_t value = 0;

	INLINE Hex() {};
    
    template< typename T >
	INLINE Hex(T value_)
		: value((uint64_t)value_)
	{
	}

	void print(IFormatStream& f) const
	{
		IFormatStream::PrintHexNumber(f, value);
	}
};

template< int digits, char ch = '0' >
struct PaddedHex
{
	uint64_t value = 0;

	INLINE PaddedHex() {};

	template< typename T >
	INLINE PaddedHex(T value_)
		: value((uint64_t)value_)
	{
	}

	void print(IFormatStream& f) const
	{
		IFormatStream::PrintHexNumber(f, value, ch, digits);
	}
};


struct HexBytes
{
    const void* data = nullptr;
    uint32_t length = 0;

	INLINE HexBytes() {};

	template< typename T >
	INLINE HexBytes(const T& value_)
		: data(&value_)
        , length(sizeof(T))
	{
	}

    INLINE HexBytes(const void* data_, uint32_t length_)
        : data(data_), length(length_)
    {}

	void print(IFormatStream& f) const
	{
        IFormatStream::PrintHexData(f, data, length);
	}
};

//----


END_INFERNO_NAMESPACE()

