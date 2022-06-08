/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//----

// An abstract file with write semantics, not optimized
class BM_CORE_FILE_API IFileWriter : public IReferencable
{
public:
	IFileWriter(FileFlags flags, StringBuf info);
    virtual ~IFileWriter();

    //----

    //! get debug information about source if this file (usually the path)
    INLINE const StringBuf& info() const { return m_info; }

    //! file flags
    INLINE FileFlags flags() const { return m_flags; }

	//----
        
    //! get size of the file (note: will include written size)
	virtual uint64_t size() const = 0;

	//! get current position in file
	virtual uint64_t pos() const = 0;

	//! change position in file
	virtual void seek(uint64_t offset) = 0;;

	//----
	
	//! read file data at specified offset
	virtual uint64_t readSync(void* ptr, uint64_t size) = 0;

	//! write data to file at current location
	virtual uint64_t writeSync(const void* ptr, uint64_t size) = 0;

    //----

protected:
    StringBuf m_info;
	FileFlags m_flags;
};

//--

END_INFERNO_NAMESPACE()
