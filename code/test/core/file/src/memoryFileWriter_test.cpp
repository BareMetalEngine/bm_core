/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"

#include "bm/core/file/include/fileMemoryWriter.h"

BEGIN_INFERNO_NAMESPACE()

//--

TEST(MemoryFileWriter, CreatedZeroSize)
{
	auto writer = RefNew<FileMemoryWriter>();
	EXPECT_EQ(0, writer->size());
}

TEST(MemoryFileWriter, CreatedZeroPosition)
{
	auto writer = RefNew<FileMemoryWriter>();
	EXPECT_EQ(0, writer->pos());
}

TEST(MemoryFileWriter, CreatedNoPages)
{
	auto writer = RefNew<FileMemoryWriter>();
	EXPECT_EQ(0, writer->pageCount());
}

TEST(MemoryFileWriter, WriteAdvancesPosition)
{
	auto writer = RefNew<FileMemoryWriter>();

	uint32_t data = 0;
	writer->writeSync(&data, sizeof(data));
	EXPECT_EQ(4, writer->pos());
}

TEST(MemoryFileWriter, WriteAdvancesSize)
{
	auto writer = RefNew<FileMemoryWriter>();

	uint32_t data = 0;
	writer->writeSync(&data, sizeof(data));
	EXPECT_EQ(4, writer->size());
}

TEST(MemoryFileWriter, WriteAllocatesSinglePage)
{
	auto writer = RefNew<FileMemoryWriter>();

	uint32_t data = 0;
	writer->writeSync(&data, sizeof(data));
	EXPECT_EQ(1, writer->pageCount());
}

TEST(MemoryFileWriter, PositionChangeWorks)
{
	auto writer = RefNew<FileMemoryWriter>();

	uint32_t data = 0;
	writer->writeSync(&data, sizeof(data));
	writer->seek(0);
	EXPECT_EQ(0, writer->pos());
}

TEST(MemoryFileWriter, AnyPositionWriteUpdatesSize)
{
	auto writer = RefNew<FileMemoryWriter>();

	uint32_t data = 0;
	writer->seek(100);
	writer->writeSync(&data, sizeof(data));
	EXPECT_EQ(104, writer->size());
}

TEST(MemoryFileWriter, OverrideDoesNotUpdateSize)
{
	auto writer = RefNew<FileMemoryWriter>();

	uint32_t data = 0;
	writer->writeSync(&data, sizeof(data));
	writer->seek(0);
	writer->writeSync(&data, sizeof(data));
	EXPECT_EQ(4, writer->size());
}

TEST(MemoryFileWriter, SequentialWritesUpdatePosition)
{
	auto writer = RefNew<FileMemoryWriter>();

	for (uint8_t i = 0; i < 16; ++i)
		writer->writeSync(&i, sizeof(i));
	
	EXPECT_EQ(16, writer->pos());
}

TEST(MemoryFileWriter, WriteAtPageEndAllocatesNewPage)
{
	auto writer = RefNew<FileMemoryWriter>();

	writer->seek(writer->pageSize() - 2);
	EXPECT_EQ(1, writer->pageCount());

	uint32_t data = 0;
	writer->writeSync(&data, sizeof(data));

	EXPECT_EQ(2, writer->pageCount());
}

TEST(MemoryFileWriter, SeekAllocatesAllNecessaryPages)
{
	auto writer = RefNew<FileMemoryWriter>();

	writer->seek(writer->pageSize());
	EXPECT_EQ(2, writer->pageCount());
}

TEST(MemoryFileWriter, ReadbackOfSingleWrittenData)
{
	auto writer = RefNew<FileMemoryWriter>();

	uint32_t data = 0xDEAFF00D;
	writer->writeSync(&data, sizeof(data));
	writer->seek(0);

	uint32_t readData = 0;
	writer->readSync(&readData, sizeof(readData));
	EXPECT_EQ(data, readData);
}

TEST(MemoryFileWriter, ReadbackOfSequentialWrite)
{
	auto writer = RefNew<FileMemoryWriter>();

	{
		uint8_t data = 0x0D;
		writer->writeSync(&data, sizeof(data));
		data = 0xF0;
		writer->writeSync(&data, sizeof(data));
		data = 0xAF;
		writer->writeSync(&data, sizeof(data));
		data = 0xDE;
		writer->writeSync(&data, sizeof(data));
	}

	uint32_t readData = 0;
	writer->seek(0);
	writer->readSync(&readData, sizeof(readData));

	EXPECT_EQ(0xDEAFF00D, readData);
}

TEST(MemoryFileWriter, ReadbackOfSingleWrittenDataAcrossPageBoundary)
{
	auto writer = RefNew<FileMemoryWriter>();

	uint32_t data = 0xDEAFF00D;
	writer->seek(writer->pageSize() - 2);
	writer->writeSync(&data, sizeof(data));

	uint32_t readData = 0;
	writer->seek(writer->pageSize() - 2);
	writer->readSync(&readData, sizeof(readData));
	EXPECT_EQ(data, readData);
}

TEST(MemoryFileWriter, ReadbackOfSequentialWriteAcrossPageBoundary)
{
	auto writer = RefNew<FileMemoryWriter>();

	writer->seek(writer->pageSize() - 2);

	{
		uint8_t data = 0x0D;
		writer->writeSync(&data, sizeof(data));
		data = 0xF0;
		writer->writeSync(&data, sizeof(data));
		data = 0xAF;
		writer->writeSync(&data, sizeof(data));
		data = 0xDE;
		writer->writeSync(&data, sizeof(data));
	}

	uint32_t readData = 0;
	writer->seek(writer->pageSize() - 2);
	writer->readSync(&readData, sizeof(readData));

	EXPECT_EQ(0xDEAFF00D, readData);
}

TEST(MemoryFileWriter, ExportToBufferPreservesSize)
{
	auto writer = RefNew<FileMemoryWriter>();

	uint32_t data = 0xDEAFF00D;
	writer->writeSync(&data, sizeof(data));

	auto exported = writer->exportDataToBuffer();
	EXPECT_EQ(4, exported.size());
}

TEST(MemoryFileWriter, ExportToBufferPreservesData)
{
	auto writer = RefNew<FileMemoryWriter>();

	uint32_t data = 0xDEAFF00D;
	writer->writeSync(&data, sizeof(data));

	auto exported = writer->exportDataToBuffer();
	const auto otherData = *(const uint32_t*)exported.data();
	EXPECT_EQ(data, otherData);
}

TEST(MemoryFileWriter, ExportToStreamWritesAllBytes)
{
	auto writer = RefNew<FileMemoryWriter>();

	uint32_t data = 0xDEAFF00D;
	writer->writeSync(&data, sizeof(data));

	uint8_t buffer[256];
	BufferOutputStream<uint8_t> stream(buffer, sizeof(buffer));

	const auto numWritten = writer->exportDataToMemory(0, 4, stream);
	EXPECT_EQ(4, numWritten);
}

TEST(MemoryFileWriter, ExportToStreamWritesAdvancesStream)
{
	auto writer = RefNew<FileMemoryWriter>();

	uint32_t data = 0xDEAFF00D;
	writer->writeSync(&data, sizeof(data));

	uint8_t buffer[256];
	BufferOutputStream<uint8_t> stream(buffer, sizeof(buffer));

	const auto numWritten = writer->exportDataToMemory(0, 4, stream);
	EXPECT_EQ(4, stream.size());
}

TEST(MemoryFileWriter, ExportToStreamDoesNotWriteMoreThanWeHave)
{
	auto writer = RefNew<FileMemoryWriter>();

	uint32_t data = 0xDEAFF00D;
	writer->writeSync(&data, sizeof(data));

	uint8_t buffer[256];
	BufferOutputStream<uint8_t> stream(buffer, sizeof(buffer));

	const auto numWritten = writer->exportDataToMemory(0, 8, stream);
	EXPECT_EQ(4, numWritten);
	EXPECT_EQ(4, stream.size());
}

TEST(MemoryFileWriter, ExportToStreamDoesNotWriteMoreThanIsInTheBuffer)
{
	auto writer = RefNew<FileMemoryWriter>();

	uint32_t data = 0xDEAFF00D;
	writer->writeSync(&data, sizeof(data));

	uint8_t buffer[256];
	BufferOutputStream<uint8_t> stream(buffer, sizeof(buffer));

	const auto numWritten = writer->exportDataToMemory(2, 4, stream);
	EXPECT_EQ(2, numWritten);
	EXPECT_EQ(2, stream.size());
}

TEST(MemoryFileWriter, ExportToStreamWritesCorrectContent)
{
	auto writer = RefNew<FileMemoryWriter>();

	uint32_t data = 0xDEAFF00D;
	writer->writeSync(&data, sizeof(data));

	uint8_t buffer[256];
	BufferOutputStream<uint8_t> stream(buffer, sizeof(buffer));

	writer->exportDataToMemory(0, 4, stream);

	const auto otherData = *(const uint32_t*)&buffer;
	EXPECT_EQ(data, otherData);
}

TEST(MemoryFileWriter, ExportToStreamRespectsOffset)
{
	auto writer = RefNew<FileMemoryWriter>();

	uint32_t data = 0xDEAFF00D;
	writer->writeSync(&data, sizeof(data));

	uint8_t buffer[256];
	BufferOutputStream<uint8_t> stream(buffer, sizeof(buffer));

	writer->exportDataToMemory(2, 4, stream);
	const auto otherData = *(const uint16_t*)&buffer;
	EXPECT_EQ(0xDEAF, otherData);
}

TEST(MemoryFileWriter, ExportToStreamRespectsPageBoundaries)
{
	auto writer = RefNew<FileMemoryWriter>();

	uint32_t data = 0xDEAFF00D;
	writer->seek(writer->pageSize() - 2);
	writer->writeSync(&data, sizeof(data));

	uint8_t buffer[256];
	BufferOutputStream<uint8_t> stream(buffer, sizeof(buffer));

	const auto numWritten = writer->exportDataToMemory(writer->pageSize() - 2, 4, stream);
	EXPECT_EQ(4, numWritten);

	const auto otherData = *(const uint32_t*)&buffer;
	EXPECT_EQ(data, otherData);
}

//--

END_INFERNO_NAMESPACE()