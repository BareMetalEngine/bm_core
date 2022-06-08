/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "stringBuf.h"
#include "bm/core/memory/include/inplaceBuffer.h"

BEGIN_INFERNO_NAMESPACE()

//---

/// a helper class that can build long strings in a safe way
class BM_CORE_CONTAINERS_API StringBuilder : public IFormatStream
{
public:
    StringBuilder(IPoolUnmanaged& pool = StringBuf::StringPool());
    virtual ~StringBuilder();

    //---

    // is the builder empty ?
    INLINE bool empty() const { return m_buffer.empty(); }

	// returns true if we still use local inplace buffer
	INLINE bool local() const { return m_buffer.local(); }

    // get size of the data
    INLINE uint32_t length() const { return m_buffer.size(); }

	// string view of the printed content
    INLINE StringView view() const { return StringView(m_buffer.view()); }

    // raw C-string pointer (builder ensures text is zero terminated for convenience)
    INLINE const char* c_str() const { checkNullTerminator(); return view().data(); }

    // auto check if empty
    INLINE operator bool() const { return !m_buffer.empty(); }

	// auto cast to string view
    INLINE operator StringView() const { return view(); }

    //--

	// clear builder
	void clear();

	// reset builder
	void reset();

    //---
    // IFormatStream

	virtual IFormatStream& append(const char* str, uint32_t len = INDEX_MAX) override final;
	virtual IFormatStream& append(const wchar_t* str, uint32_t len = INDEX_MAX) override final;

    IFormatStream& append(StringView view);
    IFormatStream& append(const StringBuf& str);
    IFormatStream& append(StringID str);

    //---

private:
    static const uint32_t INLINE_BUFFER_SIZE = 256;

    InplaceBuffer<INLINE_BUFFER_SIZE, 1> m_buffer;

    INLINE void checkNullTerminator() const
    {
        DEBUG_CHECK_EX(m_buffer.data()[m_buffer.size()] == 0, "");
    }

    INLINE void writeNullTerminator()
    {
        DEBUG_CHECK_EX(m_buffer.size() < m_buffer.capacity(), "run out of space");
        m_buffer.data()[m_buffer.size()] = 0;
    }
};

//---

END_INFERNO_NAMESPACE()
