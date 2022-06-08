/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//----

// memory - mapped file of a view
class BM_CORE_FILE_API IFileMapping : public IReferencable
{
public:
	virtual ~IFileMapping();

	//--

	//! get debug information about source if this file (usually the path and offset information)
	INLINE const StringBuf& info() const { return m_info; }

	//! get size of the view
	INLINE uint64_t size() const { return m_size; }

	//! pointer to data
	INLINE const uint8_t* data() const { return m_data; }

	//! buffer view of the data
	INLINE BufferView view() const { return BufferView(m_data, m_size); }

	//--

	// create external buffer referencing this mapping
	Buffer createBuffer() const;

	//--

protected:
	IFileMapping(StringBuf info, const void* data, uint64_t size);

	StringBuf m_info;

	const uint8_t* m_data = nullptr;
	uint64_t m_size = 0;
};

//----

END_INFERNO_NAMESPACE()
