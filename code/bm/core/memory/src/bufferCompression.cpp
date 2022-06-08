/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "buffer.h"
#include "bufferView.h"

#include <lz4.h>
#include <lz4hc.h>

#define ZLIB_CONST
#include <zlib.h>

BEGIN_INFERNO_NAMESPACE()

//--


// allocator for the ZLib-internals
namespace prv
{
    static voidpf ZlibAlloc(voidpf, uInt items, uInt size)
    {
        return PoolAllocate(MainPool(), size*items, 1);
    }

    static void ZlibFree(voidpf, voidpf address)
    {
        PoolFree(MainPool(), address);
    }

    //--

    static bool CompressZlib(BufferView input, BufferOutputStream<uint8_t>& output)
    {
        void* ret = nullptr;

        z_stream zstr;
        memzero(&zstr, sizeof(zstr));

        zstr.next_in = (Bytef*)input.data();
        zstr.avail_in = input.size();
        zstr.next_out = static_cast<Bytef*>(output.pos());
        zstr.avail_out = output.capacityLeft();
        zstr.zalloc = &prv::ZlibAlloc;
        zstr.zfree = &prv::ZlibFree;

        // start compression
        auto initRet = deflateInit(&zstr, Z_BEST_COMPRESSION);
        DEBUG_CHECK_RETURN_EX_V(initRet == Z_OK, "Failed to initialize z-lib deflate", false);

        // single step DEFLATE
        auto result = deflate(&zstr, Z_FINISH);
        deflateEnd(&zstr);

        // check results
        if (result != Z_STREAM_END)
            return false; // possible buffer was to small

        output.alloc(zstr.total_out);
        return true;
    }

    static bool DecompressZlib(BufferView input, BufferOutputStream<uint8_t>& output)
    {
        z_stream zstr;
        memzero(&zstr, sizeof(zstr));

        zstr.next_in = (Bytef*)input.data();
        zstr.avail_in = input.size();
        zstr.zalloc = &prv::ZlibAlloc;
        zstr.zfree = &prv::ZlibFree;

        // initialize
        auto initRet = inflateInit(&zstr);
        DEBUG_CHECK_EX(initRet == Z_OK, "Failed to initialize z-lib inflate");
        if (initRet != Z_OK)
            return false;

        zstr.next_out = (Bytef*)output.pos();
        zstr.avail_out = output.capacityLeft();

        auto result = inflate(&zstr, Z_NO_FLUSH);
        inflateEnd(&zstr);

        if (result != Z_STREAM_END)
            return false;

        output.alloc(zstr.total_out);
        return true;
    }

    //--

    static bool CompressLZ4(BufferView input, BufferOutputStream<uint8_t>& output)
    {
        auto compressedSize = LZ4_compress_default((const char*)input.data(), (char*)output.pos(), input.size(), output.capacityLeft());
        if (compressedSize == 0)
            return false;

        output.alloc(compressedSize);

        return true;
    }

	static bool CompressLZ4HC(BufferView input, BufferOutputStream<uint8_t>& output)
	{
		void* lzState = alloca(LZ4_sizeofStateHC());

		// compress the data
		auto compressedSize = LZ4_compress_HC_extStateHC(lzState, (const char*)input.data(), (char*)output.pos(), input.size(), output.capacityLeft(), LZ4HC_CLEVEL_OPT_MIN);
		if (compressedSize == 0)
			return false;

		output.alloc(compressedSize);

		return true;
	}

	static bool DecompressLZ4(BufferView input, BufferOutputStream<uint8_t>& output)
	{
		auto compressedBytesProduced = LZ4_decompress_safe((const char*)input.data(), (char*)output.pos(), input.size(), output.capacityLeft());
        if (compressedBytesProduced < 0)
            return false;

        output.alloc(compressedBytesProduced);
        return true;
	}

    //
}

//--

uint64_t BufferView::esitimateCompressedSize(CompressionType ct) const
{
	switch (ct)
	{
	case CompressionType::Uncompressed:
		return size();

	case CompressionType::LZ4HC:
	case CompressionType::LZ4:
		return LZ4_compressBound(size());

	case CompressionType::Zlib:
		return compressBound(size());

	default:
		ASSERT(!"Unsupported compression");
		return 0;
	}
}

bool BufferView::compress(CompressionType ct, BufferOutputStream<uint8_t>& output) const
{
    VALIDATION_RETURN_V(!empty(), true); // empty input generates no output

    //const auto minSize = esitimateCompressedSize(ct);
	//DEBUG_CHECK_RETURN_EX_V(output.capacityLeft() >= minSize, "Target buffer is to small", false);

	switch (ct)
	{
	case CompressionType::Uncompressed:
    {
        auto* writePtr = output.alloc(size());
        memcpy(writePtr, data(), size());
        return true;
    }

	case CompressionType::LZ4:
        return prv::CompressLZ4(*this, output);

	case CompressionType::LZ4HC:
        return prv::CompressLZ4HC(*this, output);

	case CompressionType::Zlib:
		return prv::CompressZlib(*this, output);

	default:
		ASSERT(!"Unsupported compression");
        return false;
	}
}

bool BufferView::decompress(CompressionType ct, BufferOutputStream<uint8_t>& output) const
{
    VALIDATION_RETURN_V(!empty(), true); // empty input generates no output

	switch (ct)
	{
	case CompressionType::Uncompressed:
	{
		auto* writePtr = output.alloc(size());
        if (!writePtr)
            return false;

		memcpy(writePtr, data(), size());
		return true;
	}

	case CompressionType::LZ4HC:
	case CompressionType::LZ4:
        return prv::DecompressLZ4(*this, output);

	case CompressionType::Zlib:
        return prv::DecompressZlib(*this, output);

	default:
		ASSERT(!"Unsupported compression");
		return false;
	}
}

//--

Buffer Buffer::CreateCompressed(IPoolUnmanaged& pool, CompressionType ct, const BufferView& data)
{
    const auto estimatedSize = data.esitimateCompressedSize(ct);

    auto ret = Buffer::CreateEmpty(pool, estimatedSize);
    DEBUG_CHECK_RETURN_EX_V(ret, "Out of memory", nullptr);

    BufferOutputStream<uint8_t> writer(ret);
    if (!data.compress(ct, writer))
        return nullptr;

    ret.adjustSize(writer.size());
    return ret;
}

Buffer Buffer::CreateDecompressed(IPoolUnmanaged& pool, CompressionType ct, const BufferView& data, uint64_t decompressedSize)
{
    if (!decompressedSize)
        return nullptr;

	auto ret = Buffer::CreateEmpty(pool, decompressedSize);
    DEBUG_CHECK_RETURN_EX_V(ret, "Out of memory", nullptr);

    BufferOutputStream<uint8_t> writer(ret);
    DEBUG_CHECK_RETURN_EX_V(data.decompress(ct, writer), "Decompression error", nullptr);

	ret.adjustSize(writer.size());
	return ret;
}

//---

END_INFERNO_NAMESPACE()
