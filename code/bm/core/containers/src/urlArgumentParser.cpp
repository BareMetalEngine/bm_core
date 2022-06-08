/***
* Inferno Engine v4
* Written by Tomasz "RexDex" Jonarski
***/

#include "build.h"
#include "urlArgumentParser.h"

BEGIN_INFERNO_NAMESPACE()

//---

URLArgumentsParser::URLArgumentsParser()
{
	memset(m_buckets, 0xFF, sizeof(m_buckets));
}

URLArgumentsParser::URLArgumentsParser(StringView txt)
{
	memset(m_buckets, 0xFF, sizeof(m_buckets));
	Parse(txt, *this);
}

//--

URLArgumentsParser::BucketHash URLArgumentsParser::MakeHash(StringView key)
{
	BucketHash ret;
	ret.hash = StringView::CalcHash(key);
	ret.bucketIndex = ret.hash % NUM_BUCKETS;
	return ret;
}

bool URLArgumentsParser::has(StringView key) const
{
	StringView ret;
	return raw(key, ret);	
}

bool URLArgumentsParser::raw(StringView key, StringView& outView) const
{
	VALIDATION_RETURN_V(key, false); // empty keys are not checked

	const auto hash = MakeHash(key);

	auto index = m_buckets[hash.bucketIndex];
	while (index)
	{
		const auto& param = m_parameters[index];
		if (param.hash == hash.hash && param.key == key)
		{
			outView = param.value;
			return true;
		}

		index = param.next;
	}

	return false;
}

StringView URLArgumentsParser::raw(StringView key) const
{
	StringView ret;
	raw(key, ret);
	return ret;
}

//--

bool URLArgumentsParser::safeString(StringView key, StringBuf& outValue) const
{
	StringView value;
	if (raw(key, value))
		return value.decode(EncodingType::URL, outValue);

	return false;
}

bool URLArgumentsParser::safeInt(StringView key, int& outValue) const
{
	StringView value;
	if (raw(key, value))
		return value.match(outValue);

	return false;

}

bool URLArgumentsParser::safeInt(StringView key, int64_t& outValue) const
{
	StringView value;
	if (raw(key, value))
		return value.match(outValue);

	return false;
}

bool URLArgumentsParser::safeFloat(StringView key, float& outValue) const
{
	StringView value;
	if (raw(key, value))
		return value.match(outValue);

	return false;
}

bool URLArgumentsParser::safeFloat(StringView key, double& outValue) const
{
	StringView value;
	if (raw(key, value))
		return value.match(outValue);

	return false;
}

bool URLArgumentsParser::safeBuffer(StringView key, Buffer& outValue) const
{
	StringView value;
	if (raw(key, value))
		return Buffer::CreateFromEncoding(MainPool(), EncodingType::Base64, value, false);

	return false;
}

bool URLArgumentsParser::safeBufferRaw(StringView key, void* outValue, uint32_t valueSize) const
{
	// TODO: optimize!

	Buffer temp;
	if (safeBuffer(key, temp))
	{
		if (temp.size() == valueSize)
		{
			memcpy(outValue, temp.data(), valueSize);
			return true;
		}
	}

	return false;
}

bool URLArgumentsParser::safeBufferHex(StringView key, Buffer& outValue) const
{
	StringView value;
	if (raw(key, value))
		return Buffer::CreateFromEncoding(MainPool(), EncodingType::Hex, value, false);

	return false;
}

bool URLArgumentsParser::safeBufferHexRaw(StringView key, void* outValue, uint32_t valueSize) const
{
	Buffer temp;
	if (safeBufferHex(key, temp))
	{
		if (temp.size() == valueSize)
		{
			memcpy(outValue, temp.data(), valueSize);
			return true;
		}
	}

	return false;
}

bool URLArgumentsParser::safeGuid(StringView key, GUID& outGUID) const
{
	return safeType<GUID>(key, outGUID);
}

//--

StringBuf URLArgumentsParser::quickString(StringView key, StringView defaultValue) const
{
	StringBuf ret;
	if (safeString(key, ret))
		return ret;

	return StringBuf(defaultValue);
}

int64_t URLArgumentsParser::quickInt(StringView key, int64_t defaultValue) const
{
	StringView value;
	if (raw(key, value))
	{
		int64_t ret = 0;
		if (value.match(ret))
			return ret;
	}

	return defaultValue;
}

double URLArgumentsParser::quickFloat(StringView key, double defaultValue) const
{
	StringView value;
	if (raw(key, value))
	{
		double ret = 0.0;
		if (value.match(ret))
			return ret;
	}

	return defaultValue;
}

Buffer URLArgumentsParser::quickBuffer(StringView key) const
{
	Buffer ret;
	if (safeBuffer(key, ret))
		return ret;

	return nullptr;
}

Buffer URLArgumentsParser::quickBufferHex(StringView key) const
{
	Buffer ret;
	if (safeBufferHex(key, ret))
		return ret;

	return nullptr;
}

//--

bool URLArgumentsParser::collectStrings(StringView key, Array<StringBuf>& outValues) const
{
	VALIDATION_RETURN_V(key, false); // empty keys are not checked

	const auto hash = MakeHash(key);

	auto index = m_buckets[hash.bucketIndex];
	while (index)
	{
		const auto& param = m_parameters[index];
		if (param.hash == hash.hash && param.key == key)
		{
			StringBuf value;
			if (!param.value.decode(EncodingType::URL, value, false))
				return false;

			outValues.emplaceBack(value);
		}

		index = param.next;
	}

	return true;
}

bool URLArgumentsParser::collectInts(StringView key, Array<int>& outValues) const
{
	VALIDATION_RETURN_V(key, false); // empty keys are not checked

	const auto hash = MakeHash(key);

	auto index = m_buckets[hash.bucketIndex];
	while (index)
	{
		const auto& param = m_parameters[index];
		if (param.hash == hash.hash && param.key == key)
		{
			int value = 0;

			if (!param.value.match(value))
				return false;

			outValues.emplaceBack(value);
		}

		index = param.next;
	}

	return true;
}

bool URLArgumentsParser::collectInts(StringView key, Array<int64_t>& outValues) const
{
	VALIDATION_RETURN_V(key, false); // empty keys are not checked

	const auto hash = MakeHash(key);

	auto index = m_buckets[hash.bucketIndex];
	while (index)
	{
		const auto& param = m_parameters[index];
		if (param.hash == hash.hash && param.key == key)
		{
			int64_t value = 0;

			if (!param.value.match(value))
				return false;

			outValues.emplaceBack(value);
		}

		index = param.next;
	}

	return true;
}

bool URLArgumentsParser::collectFloats(StringView key, Array<float>& outValues) const
{
	VALIDATION_RETURN_V(key, false); // empty keys are not checked

	const auto hash = MakeHash(key);

	auto index = m_buckets[hash.bucketIndex];
	while (index)
	{
		const auto& param = m_parameters[index];
		if (param.hash == hash.hash && param.key == key)
		{
			float value = 0.0f;

			if (!param.value.match(value))
				return false;

			outValues.emplaceBack(value);
		}

		index = param.next;
	}

	return true;
}

bool URLArgumentsParser::collectFloats(StringView key, Array<double>& outValues) const
{
	VALIDATION_RETURN_V(key, false); // empty keys are not checked

	const auto hash = MakeHash(key);

	auto index = m_buckets[hash.bucketIndex];
	while (index)
	{
		const auto& param = m_parameters[index];
		if (param.hash == hash.hash && param.key == key)
		{
			double value = 0.0;

			if (!param.value.match(value))
				return false;

			outValues.emplaceBack(value);
		}

		index = param.next;
	}

	return true;
}

//---

static INLINE bool IsAlphaNumeric(char ch)
{
	return
		((ch >= '0') && (ch <= '9')) ||
		((ch >= 'a') && (ch <= 'z')) ||
		((ch >= 'A') && (ch <= 'Z'));
}

bool URLArgumentsParser::Parse(StringView txt, URLArgumentsParser& outParser)
{
	auto str = txt.data();
	auto end = str + txt.length();

	// skip the ? if provided
	if (str < end && *str == '?')
		str++;

	// parse whole string
	while (str < end)
	{
		Parameter param;

		// read the ident name
		auto identStart = str;
		while (str < end)
		{
			if (*str == '=')
				break;
			if (!IsAlphaNumeric(*str))
				return false;
			str++;
		}

		// empty ident
		if (str == identStart || str == end)
			return false;

		// we have the ident
		param.key = StringView(identStart, str);
		param.hash = StringView::CalcHash(param.key);
		
		// skip the '=' that we found
		str += 1;

		// now parse the value
		auto valueStart = str;
		while (str < end)
		{
			if (*str == '&')
				break;

			const char ch = *str++;

			if (IsAlphaNumeric(ch) || ch == '.' || ch == '~' || ch == '%')
				continue;

			return false;
		}

		// define value
		param.value = StringView(valueStart, str);

		// link
		{
			auto bucketIndex = param.hash % NUM_BUCKETS;
			param.next = outParser.m_buckets[bucketIndex];

			outParser.m_parameters.pushBack(param);
			outParser.m_buckets[bucketIndex] = outParser.m_parameters.lastValidIndex();			
		}
		
		// skip the separator
		if (str < end)
		{
			if (*str != '&')
				return false;
			str += 1;
		}
	}

	// parsed
	return true;
}

//---

END_INFERNO_NAMESPACE()