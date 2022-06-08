/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

/// data free function
typedef std::function<void(void*)> TBufferFreeFunc;

/// storage for buffer, can be shared
/// NOTE: storage is never empty
class BufferStorage;
class BufferView;

//--

/// how should the buffer's memory be initialized
enum class BufferInitState : uint8_t
{
    NoClear,
    ClearToZero,
};

//--

/// a buffer of memory with custom free function
class BM_CORE_MEMORY_API Buffer : public MainPoolData<NoCopy>
{
public:
    //--

	static const uint64_t BUFFER_MAX_SIZE = 16ULL * 1024 * 1024 * 1024; // asking for more than 16GB is very sus
    static const uint64_t BUFFER_SYSTEM_MEMORY_MIN_SIZE = 4 * 1024 * 1024; // buffer size that is allocated directly from system, bypassing any allocator we have
    static const uint64_t BUFFER_SYSTEM_MEMORY_LARGE_PAGES_SIZE = 128 * 1024 * 1024; // buffer size that is allocated directly from system, bypassing any allocator we have
    static const uint32_t BUFFER_DEFAULT_ALIGNMNET = 16; // buffers are usually big, this is useful alignment

    //--

    INLINE Buffer() {};
    INLINE Buffer(std::nullptr_t) {};
    INLINE Buffer(BufferStorage* storage) : m_storage(storage) {};
    Buffer(const Buffer& other);
    Buffer(Buffer&& other);
    ~Buffer(); // frees the data

    Buffer& operator=(const Buffer& other);
    Buffer& operator=(Buffer&& other);

    // memcmp
    bool operator==(const Buffer& other) const;
    bool operator!=(const Buffer& other) const;

    //--

    // empty ?
    INLINE bool empty() const { return m_storage == nullptr; }

    // cast to bool check
    INLINE operator bool() const { return m_storage != nullptr; }

	// auto cast to view
    operator BufferView() const;

    //--

    // get pointer to data, read only access, should NOT be freed
    uint8_t* data() const;

    // get size of the data in the buffer
    uint64_t size() const;

    // get view of the buffer, most nice operations are in the view
    BufferView view() const;

    //--

    // reset reference
    void reset();

    // create a sub-buffer WITHOUT copying the storage
    Buffer createSubBuffer(uint64_t offset, uint64_t size) const;

    //--

    // print to stream (prints as BASE64)
    void print(IFormatStream& f) const;

    //--
    
	// create empty buffer of given size and alignment, optionally the buffer memory can be cleared
	static Buffer CreateEmpty(IPoolUnmanaged& pool, uint64_t size, uint32_t alignment = BUFFER_DEFAULT_ALIGNMNET, BufferInitState clearState = BufferInitState::NoClear);

    // create buffer from external memory that is not managed by us and will be freed by provided function
    static Buffer CreateExternal(const BufferView& view, TBufferFreeFunc freeFunc);

	// create buffer for given size and copy content for provided pointer
	static Buffer CreateFromCopy(IPoolUnmanaged& pool, const BufferView& view, uint32_t alignment = BUFFER_DEFAULT_ALIGNMNET);

	// create buffer for given size and copy content for provided pointer
	static Buffer CreateFromCopy(IPoolUnmanaged& pool, const void* data, uint64_t size, uint32_t alignment = BUFFER_DEFAULT_ALIGNMNET);

    // create buffer from already allocated memory (shorthand for CreateExternal with default free function)
    static Buffer CreateFromAlreadyAllocatedMemory(IPoolUnmanaged& pool, const BufferView& view);

    // create buffer from text in Base64 encoding, if decoding fails empty buffer is returned
    static Buffer CreateFromEncoding(IPoolUnmanaged& pool, EncodingType et, const BufferView& view, bool allowWhitespces = true, uint32_t alignment = BUFFER_DEFAULT_ALIGNMNET);

	// create buffer from text in hexadecimal encoding, if decoding fails empty buffer is returned
	static Buffer CreateEncoded(IPoolUnmanaged& pool, EncodingType et, const BufferView& view, uint32_t alignment = BUFFER_DEFAULT_ALIGNMNET);

    // create buffer by compressing memory
    static Buffer CreateCompressed(IPoolUnmanaged& pool, CompressionType ct, const BufferView& data);

	// create buffer by decompressing memory
    // NOTE: decompressed size must be known
	static Buffer CreateDecompressed(IPoolUnmanaged& pool, CompressionType ct, const BufferView& data, uint64_t decompressedSize);
	
    //--

private:
    BufferStorage* m_storage = nullptr; // refcounted

protected:
	void adjustSize(uint64_t newBufferSize);

    friend class BufferView;
};

//--

END_INFERNO_NAMESPACE()
