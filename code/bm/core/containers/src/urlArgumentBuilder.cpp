/***
* Inferno Engine v4
* Written by Tomasz "RexDex" Jonarski
***/

#include "build.h"
#include "urlArgumentBuilder.h"

BEGIN_INFERNO_NAMESPACE()

//---

URLArgumentBuilder::URLArgumentBuilder()
{
	m_textBuffer.pushBack(0);
}

//--

void URLArgumentBuilder::clear()
{
	m_params.reset();
	m_textBuffer.reset();
	m_textBuffer.pushBack(0);
	m_textLength = 0;
}

void URLArgumentBuilder::print(IFormatStream& f) const
{
	const char* separator = "?";

	for (const auto& param : m_params)
	{
		const char* keyStr = m_textBuffer.typedData() + param.key;
		const char* valueStr = m_textBuffer.typedData() + param.value;

		f.append(separator);
		f.append(keyStr);
		f.append("=");

		StringView(valueStr).bufferView().encode(EncodingType::URL, f);

		separator = "&";
	}
}

URLArgumentBuilder& URLArgumentBuilder::param(StringView key, StringView value)
{
	DEBUG_CHECK_RETURN_EX_V(key, "Invalid key", *this);

	auto& entry = m_params.emplaceBack();
	entry.key = makeString(key);
	entry.value = makeSafeString(value);
	return *this;
}

URLArgumentBuilder& URLArgumentBuilder::paramInt(StringView key, int64_t value)
{
	return param(key, TempString("{}", value));
}

URLArgumentBuilder& URLArgumentBuilder::paramFloat(StringView key, double value)
{
	return param(key, TempString("{}", value));
}

URLArgumentBuilder& URLArgumentBuilder::paramBool(StringView key, bool value)
{
	return param(key, value ? "1" : "0");
}

URLArgumentBuilder& URLArgumentBuilder::paramBase64(StringView key, BufferView buf)
{
	DEBUG_CHECK_RETURN_EX_V(key, "Invalid key", *this);

	auto& entry = m_params.emplaceBack();
	entry.key = makeString(key);
	entry.value = makeBase64(buf);
	return *this;
}

URLArgumentBuilder& URLArgumentBuilder::paramHex(StringView key, BufferView buf)
{
	DEBUG_CHECK_RETURN_EX_V(key, "Invalid key", *this);

	auto& entry = m_params.emplaceBack();
	entry.key = makeString(key);
	entry.value = makeHex(buf);
	return *this;
}

//--

uint32_t URLArgumentBuilder::makeString(StringView txt)
{
	const auto length = txt.length();
	const auto offset = m_textBuffer.size();

	auto view = m_textBuffer.allocateUninitializedView(length);
	DEBUG_CHECK_RETURN_EX_V(view, "Out of memory", 0);

	memcpy(view.data(), txt.data(), txt.length());
	view[txt.length()] = 0;

	m_textLength += txt.length();

	return offset;
}

uint32_t URLArgumentBuilder::makeSafeString(StringView txt)
{
	const auto length = txt.bufferView().estimateEncodedSize(EncodingType::URL);

	const auto offset = m_textBuffer.size();

	auto view = m_textBuffer.allocateUninitializedView(length + 1);
	DEBUG_CHECK_RETURN_EX_V(view, "Out of memory", 0);

	BufferOutputStream<char> output(view);
	DEBUG_CHECK_RETURN_EX_V(txt.encode(EncodingType::URL, output), "Encoding error", 0);

	m_textLength += output.size();
	return offset;
}

uint32_t URLArgumentBuilder::makeBase64(BufferView data)
{
	const auto length = data.estimateEncodedSize(EncodingType::Base64);

	const auto offset = m_textBuffer.size();
	auto view = m_textBuffer.allocateUninitializedView(length + 1);
	DEBUG_CHECK_RETURN_EX_V(view, "Out of memory", 0);

	BufferOutputStream<char> output(view);
	DEBUG_CHECK_RETURN_EX_V(data.encode(EncodingType::Base64, output), "Encoding error", 0);

	m_textLength += output.size();
	return offset;
}

uint32_t URLArgumentBuilder::makeHex(BufferView data)
{
	const auto length = data.estimateEncodedSize(EncodingType::Hex);

	const auto offset = m_textBuffer.size();
	auto view = m_textBuffer.allocateUninitializedView(length + 1);
	DEBUG_CHECK_RETURN_EX_V(view, "Out of memory", 0);

	BufferOutputStream<char> output(view);
	DEBUG_CHECK_RETURN_EX_V(data.encode(EncodingType::Hex, output), "Encoding error", 0);

	m_textLength += output.size();
	return offset;
}

//---

uint32_t URLArgumentBuilder::measureNeededSize() const
{
	uint32_t length = m_textLength;
	length += m_params.size() * 2;
	return length;
}

void URLArgumentBuilder::write(char* str, uint32_t length) const
{
	const char* strEnd = str + length;

	char separator = '?';
	for (const auto& param : m_params)
	{
		const auto* keyStr = m_textBuffer.typedData() + param.key;
		const auto* valueStr = m_textBuffer.typedData() + param.value;

		const auto keyLength = strlen(keyStr);
		const auto valueLength = strlen(valueStr);
		ASSERT_EX(str + 2 + keyLength + valueLength < strEnd, "Trying to write outside the allocated range");

		*str++ = separator;
		memcpy(str, keyStr, keyLength); str += keyLength;
		*str++ = '=';
		memcpy(str, valueStr, valueLength); str += valueLength;
	}

	ASSERT_EX(str == strEnd, "Not all chars written");
}

StringBuf URLArgumentBuilder::toString() const
{
	if (const auto length = measureNeededSize())
	{
		auto ret = StringBuf(length);
		DEBUG_CHECK_RETURN_EX_V(ret, "Out of memory", nullptr);

		char* str = (char*)ret.c_str();
		write(str, length);
		str[length] = 0;

		return ret;
	}

	return StringBuf::EMPTY();
}

Buffer URLArgumentBuilder::toBuffer() const
{
	if (const auto length = measureNeededSize())
	{
		auto ret = Buffer::CreateEmpty(StringBuf::StringPool(), length);
		DEBUG_CHECK_RETURN_EX_V(ret, "Out of memory", nullptr);

		char* str = (char*)ret.data();
		write(str, length);

		return ret;
	}

	return nullptr;
}

//---

END_INFERNO_NAMESPACE()