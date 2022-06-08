/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "buffer.h"
#include "bufferView.h"
#include "poolUnmanaged.h"

BEGIN_INFERNO_NAMESPACE()

//--

TYPE_ALIGN(16, class) BM_CORE_MEMORY_API BufferStorage : public MainPoolData<NoCopy>
{
public:
    // get pointer to data, read only access, should NOT be freed
    INLINE uint8_t * data() const { return m_offsetToPayload ? ((uint8_t*)this + m_offsetToPayload) : (uint8_t*)m_externalPayload; }

    // get size of the data in the buffer
    INLINE uint64_t size() const { return m_size; }

    // change size
    INLINE void adjustSize(uint64_t size) { m_size = size; }

    //--

    // add a reference count
    void addRef();

    // remove reference, releasing last reference will free the memory
    void releaseRef();

    //--

    // create a buffer storage with internal storage
    static BufferStorage* CreateInternal(IPoolUnmanaged& pool, uint64_t size, uint32_t alignment);

    // create a buffer with external storage
    static BufferStorage* CreateExternal(uint64_t size, void* externalData, TBufferFreeFunc freeFunc);

	//--

	// detach memory from this buffer storage
	void* detach();

	//--

private:
    ~BufferStorage() = delete;

    uint64_t m_size = 0;
    uint16_t m_offsetToPayload = 0;
    void* m_externalPayload = nullptr;
    TBufferFreeFunc m_freeFunc;
    std::atomic<uint32_t> m_refCount = 1;
    IPoolUnmanaged* m_pool = nullptr;

    void freeData();
};

//--

void BufferStorage::addRef()
{
    ++m_refCount;
}

void BufferStorage::releaseRef()
{
    if (0 == --m_refCount)
        freeData();
}

void BufferStorage::freeData()
{
	m_size = 0;
	m_offsetToPayload = 0;

    if (m_externalPayload)
    {
        auto freeFunc = m_freeFunc;
        auto payload = m_externalPayload;
        m_freeFunc = nullptr;
        m_externalPayload = nullptr;
        m_pool = nullptr;
        freeFunc(payload);
    }
    else if (m_pool)
    {
        auto pool = m_pool;
        m_freeFunc = nullptr;
        m_externalPayload = nullptr;
        m_pool = nullptr;
        pool->freeMemory(this);
    }
}

BufferStorage* BufferStorage::CreateInternal(IPoolUnmanaged& pool, uint64_t size, uint32_t alignment)
{
    DEBUG_CHECK_EX(alignment <= 4096, "Alignment requirement on buffer if really big");

    auto totalSize = size + sizeof(BufferStorage);
    if (alignment > alignof(BufferStorage))
        totalSize += (alignment - alignof(BufferStorage));

    auto* mem = (uint8_t*)pool.allocateMemory(totalSize, alignment);
    auto* payloadPtr = AlignPtr(mem + sizeof(BufferStorage), alignment);
    auto offset = payloadPtr - mem;
    DEBUG_CHECK_EX(offset <= 65535, "Offset to payload is to big");

    auto* ret = new (mem) BufferStorage();
    ret->m_pool = &pool;
    ret->m_refCount = 1;
    ret->m_offsetToPayload = (uint16_t)offset;
    ret->m_size = size;
    return ret;
}

BufferStorage* BufferStorage::CreateExternal(uint64_t size, void *externalPayload, TBufferFreeFunc freeFunc)
{
    DEBUG_CHECK_EX(freeFunc && externalPayload, "Invalid setup for external buffer");

    auto* ret = new BufferStorage();
    ret->m_refCount = 1;
    ret->m_pool = &MainPool();
    ret->m_offsetToPayload = 0;
    ret->m_externalPayload = std::move(externalPayload);
    ret->m_freeFunc = freeFunc;
    ret->m_size = size;
    return ret;
}

//--

Buffer::Buffer(const Buffer& other)
    : m_storage(other.m_storage)
{
    if (m_storage)
        m_storage->addRef();
}

Buffer::Buffer(Buffer&& other)
{
    m_storage = other.m_storage;
    other.m_storage = nullptr;
}

Buffer::~Buffer()
{
    reset();
}

Buffer& Buffer::operator=(const Buffer& other)
{
    if (this != &other)
    {
        auto old = m_storage;
        m_storage = other.m_storage;
        if (m_storage)
            m_storage->addRef();
        if (old)
            old->releaseRef();
    }
    return *this;
}

Buffer& Buffer::operator=(Buffer&& other)
{
    if (this != &other)
    {
        auto old = m_storage;
        m_storage = other.m_storage;
        other.m_storage = nullptr;
        if (old)
            old->releaseRef();
    }
    return *this;
}

bool Buffer::operator==(const Buffer& other) const
{
    if (m_storage == other.m_storage)
        return true;
    if (size() != other.size())
        return false;
    return 0 == memcmp(data(), other.data(), size());
}

bool Buffer::operator!=(const Buffer& other) const
{
    return !operator==(other);
}

uint8_t* Buffer::data() const 
{
    return m_storage ? m_storage->data() : nullptr;
}

uint64_t Buffer::size() const
{
    return m_storage ? m_storage->size() : 0;
}

Buffer::operator BufferView() const
{
    return view();
}

BufferView Buffer::view() const
{
    return m_storage ? BufferView(m_storage->data(), m_storage->size()) : nullptr;
}

void Buffer::reset()
{
    auto storage = m_storage;
    m_storage = nullptr;

    if (storage)
        storage->releaseRef();

    // NO LINES HERE - this is already freed
}

void Buffer::adjustSize(uint64_t newBufferSize)
{
    DEBUG_CHECK_EX(newBufferSize <= m_storage->size(), "Can't adjust buffer to be bigger, sorry");
    if (newBufferSize <= m_storage->size())
        m_storage->adjustSize(newBufferSize);
}

Buffer Buffer::createSubBuffer(uint64_t offset, uint64_t partSize) const
{
    DEBUG_CHECK_RETURN_EX_V(offset <= size(), "Invalid offset", nullptr);
    DEBUG_CHECK_RETURN_EX_V(partSize <= size(), "Invalid size", nullptr);
    DEBUG_CHECK_RETURN_EX_V(partSize + offset <= size(), "Invalid buffer range", nullptr);

    if (auto storage = m_storage)
    {
        storage->addRef();

        auto freeFunc = [storage](void* ptr) { storage->releaseRef(); };
        return BufferStorage::CreateExternal(partSize, m_storage->data() + offset, freeFunc);
    }
    else
    {
        return nullptr;
    }
}


void Buffer::print(IFormatStream& f) const
{
    if (size())
        f.appendf("<NullBuffer>");
    else
        f.appendf("<Buffer {}>", size());
}

//--

Buffer Buffer::CreateEmpty(IPoolUnmanaged& pool, uint64_t size, uint32_t alignment /*= BUFFER_DEFAULT_ALIGNMNET*/, BufferInitState clearState /*= BufferInitState::NoClear*/)
{
    if (size)
    {
        auto storage = BufferStorage::CreateInternal(pool, size, alignment);
        DEBUG_CHECK_RETURN_EX_V(storage, "OOM", nullptr);

        if (clearState == BufferInitState::ClearToZero)
            memzero(storage->data(), storage->size());

        return Buffer(storage);
    }

    return nullptr;
}

Buffer Buffer::CreateFromCopy(IPoolUnmanaged& pool, const void* data, uint64_t size, uint32_t alignment /*= BUFFER_DEFAULT_ALIGNMNET*/)
{
    if (!data || !size)
        return nullptr;

    return CreateFromCopy(pool, BufferView(data, size), alignment);
}

Buffer Buffer::CreateFromCopy(IPoolUnmanaged& pool, const BufferView& view, uint32_t alignment /*= BUFFER_DEFAULT_ALIGNMNET*/)
{
    if (view.empty())
        return nullptr;

	auto storage = BufferStorage::CreateInternal(pool, view.size(), alignment);
	DEBUG_CHECK_RETURN_EX_V(storage, "OOM", nullptr);

    memcpy(storage->data(), view.data(), view.size());
	return Buffer(storage);
}

Buffer Buffer::CreateExternal(const BufferView& view, TBufferFreeFunc freeFunc)
{
    DEBUG_CHECK_RETURN_EX_V(freeFunc, "Deallocation function must be specified", nullptr);
    return Buffer(BufferStorage::CreateExternal(view.size(), (void*)view.data(), freeFunc));
}

Buffer Buffer::CreateFromAlreadyAllocatedMemory(IPoolUnmanaged& pool, const BufferView& view)
{
    VALIDATION_RETURN_V(view, nullptr);
    auto freeFunc = [&pool](void* ptr) { pool.freeMemory(ptr); };
    return Buffer(BufferStorage::CreateExternal(view.size(), (void*)view.data(), freeFunc));
}

//--

END_INFERNO_NAMESPACE()
