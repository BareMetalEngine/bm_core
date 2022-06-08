/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"

#include "bm/core/containers/include/inplaceArray.h"
#include "bm/core/file/include/memoryFileSystem.h"
#include "bm/core/file/include/fileReader.h"
#include "bm/core/file/include/fileWriter.h"
#include "bm/core/file/include/fileView.h"
#include "bm/core/file/include/fileMapping.h"

BEGIN_INFERNO_NAMESPACE()

//--

TEST(MemoryFileSystem, EmptyReportsRootDir)
{
	MemoryFileSystem fs;
	Array<StringBuf> ar;
	fs.testCollectFileSystemRoots(ar);
	ASSERT_EQ(1, ar.size());
	ASSERT_STREQ("/", ar[0].c_str());
}

TEST(MemoryFileSystem, EmptyReportNoFiles)
{
	MemoryFileSystem fs;
	Array<StringBuf> ar;
	fs.testCollectFiles("/", "*.*", ar);
	ASSERT_EQ(0, ar.size());
}

TEST(MemoryFileSystem, EmptyReportNoDirs)
{
	MemoryFileSystem fs;
	Array<StringBuf> ar;
	fs.testCollectSubDirs("/", ar);
	ASSERT_EQ(0, ar.size());
}

//--

TEST(MemoryFileSystem, TestFileStorageAutoCreatesDirectory)
{
	MemoryFileSystem fs;
	fs.testStoreFileContentText("/crap/test.txt", "Ala ma kota");
	EXPECT_TRUE(fs.fileInfo("/crap/test.txt"));
}

TEST(MemoryFileSystem, TestFileStorageFileVisible)
{
	MemoryFileSystem fs;
	fs.testStoreFileContentText("/crap/test.txt", "Ala ma kota");

	InplaceArray<StringBuf, 3> files;
	fs.testCollectLocalFiles("/crap/", "*.txt", files);

	ASSERT_EQ(1, files.size());
	EXPECT_STREQ("test.txt", files[0].c_str());
}

TEST(MemoryFileSystem, TestFileStorageDirectoryVisible)
{
	MemoryFileSystem fs;
	fs.testStoreFileContentText("/crap/test.txt", "Ala ma kota");

	InplaceArray<StringBuf, 3> dirs;
	fs.testCollectSubDirs("/", dirs);

	ASSERT_EQ(1, dirs.size());
	EXPECT_STREQ("crap", dirs[0].c_str());
}

//--

TEST(MemoryFileSystem, SingleFileAtRootReportsItSelf)
{
	MemoryFileSystem fs;
	fs.testStoreFileContentText("/test.txt", "Ala ma kota");

	Array<StringBuf> ar;
	fs.testCollectLocalFiles("/", "*.*", ar);
	ASSERT_EQ(1, ar.size());
	ASSERT_STREQ("test.txt", ar[0].c_str());
}

TEST(MemoryFileSystem, SingleFileAtRootReportsExistance)
{
	MemoryFileSystem fs;
	fs.testStoreFileContentText("/test.txt", "Ala ma kota");
	EXPECT_TRUE(fs.fileInfo("/test.txt"));	
}

TEST(MemoryFileSystem, SingleFileAtRootReportsCorrectTimestamp)
{
	auto ts = TimeStamp::GetNow();

	MemoryFileSystem fs;
	fs.testStoreFileContentText("/test.txt", "Ala ma kota", ts);

	TimeStamp testTs;
	EXPECT_TRUE(fs.fileInfo("/test.txt", &testTs));
	EXPECT_EQ(ts, testTs);
}

TEST(MemoryFileSystem, SingleFileAtRootReportsCorrectSize)
{
	MemoryFileSystem fs;
	fs.testStoreFileContentText("/test.txt", "Ala ma kota");

	uint64_t size = 0;
	EXPECT_TRUE(fs.fileInfo("/test.txt", nullptr, &size));
	EXPECT_EQ(11, size);
}

TEST(MemoryFileSystem, SingleFileReportsCorrectContent)
{
	MemoryFileSystem fs;
	fs.testStoreFileContentText("/test.txt", "Ala ma kota");

	StringBuf data = fs.testLoadFileContentText("/test.txt");
	EXPECT_EQ(11, data.length());
	EXPECT_STREQ("Ala ma kota", data.c_str());
}

TEST(MemoryFileSystem, SingleFileOpensForReading)
{
	MemoryFileSystem fs;
	fs.testStoreFileContentText("/test.txt", "Ala ma kota");

	auto data = fs.openForReading("/test.txt", FileReadMode::MemoryMapped);
	ASSERT_TRUE(data);
}

TEST(MemoryFileSystem, SingleFileNonExistingDoesNotOpensForReading)
{
	MemoryFileSystem fs;

	auto data = fs.openForReading("/test.txt", FileReadMode::MemoryMapped);
	ASSERT_FALSE(data);
}

TEST(MemoryFileSystem, SingleFileDeletedDoesNotOpenForReading)
{
	MemoryFileSystem fs;
	fs.testStoreFileContentText("/test.txt", "Ala ma kota");

	fs.deleteFile("/test.txt");

	auto data = fs.openForReading("/test.txt", FileReadMode::MemoryMapped);
	ASSERT_FALSE(data);
}

TEST(MemoryFileSystem, SingleFileOpensWithValidSize)
{
	MemoryFileSystem fs;
	fs.testStoreFileContentText("/test.txt", "Ala ma kota");

	auto data = fs.openForReading("/test.txt", FileReadMode::MemoryMapped);
	ASSERT_TRUE(data);
	EXPECT_EQ(11, data->size());
}

TEST(MemoryFileSystem, SingleFileOpenedFileAsyncReadWithCallback)
{
	MemoryFileSystem fs;
	fs.testStoreFileContentText("/test.txt", "Ala ma kota");

	auto data = fs.openForReading("/test.txt", FileReadMode::MemoryMapped);
	ASSERT_TRUE(data);

	char buffer[64];
	memzero(buffer, sizeof(buffer));
	volatile int numActualRead = 0;
	std::atomic<bool> readDone = false;

	data->readAsync(0, data->size(), buffer, [&readDone, &numActualRead](int actualRead)
		{
			numActualRead = actualRead;
			readDone = true;
		});

	NativeTimePoint timeout = NativeTimePoint::Now() + 1.0;
	while (!readDone.load())
	{
		ASSERT_FALSE(timeout.reached());
		Thread::Sleep(1);
	}

	ASSERT_EQ(11, numActualRead);
	ASSERT_STREQ("Ala ma kota", buffer);
}

TEST(MemoryFileSystem, SingleFileOpenedFileAsyncReadWithTask)
{
	MemoryFileSystem fs;
	fs.testStoreFileContentText("/test.txt", "Ala ma kota");

	auto data = fs.openForReading("/test.txt", FileReadMode::MemoryMapped);
	ASSERT_TRUE(data);

	uint32_t numRead = 0;
	char buffer[64];
	memzero(buffer, sizeof(buffer));
	ASSERT_TRUE(data->readAsync(NoTask(), 0, data->size(), buffer, numRead));

	ASSERT_EQ(11, numRead);
	ASSERT_STREQ("Ala ma kota", buffer);
}

TEST(MemoryFileSystem, SingleFileOpenedFileCreateView)
{
	MemoryFileSystem fs;
	fs.testStoreFileContentText("/test.txt", "Ala ma kota");

	auto data = fs.openForReading("/test.txt", FileReadMode::MemoryMapped);
	ASSERT_TRUE(data);

	auto view = data->createView(data->fullRange());
	ASSERT_TRUE(view);
}

TEST(MemoryFileSystem, SingleFileOpenedFileCreateViewCorrectSize)
{
	MemoryFileSystem fs;
	fs.testStoreFileContentText("/test.txt", "Ala ma kota");

	auto data = fs.openForReading("/test.txt", FileReadMode::MemoryMapped);
	ASSERT_TRUE(data);

	auto view = data->createView(data->fullRange());
	ASSERT_TRUE(view);
	ASSERT_EQ(11, view->size());
}

TEST(MemoryFileSystem, SingleFileOpenedFileSyncReadViaView)
{
	MemoryFileSystem fs;
	fs.testStoreFileContentText("/test.txt", "Ala ma kota");

	auto data = fs.openForReading("/test.txt", FileReadMode::MemoryMapped);
	ASSERT_TRUE(data);

	auto view = data->createView(data->fullRange());
	ASSERT_TRUE(view);

	char buffer[64];
	memzero(buffer, sizeof(buffer));
	const auto numRead = view->readSync(buffer, 64);
	ASSERT_EQ(11, numRead);
	ASSERT_STREQ("Ala ma kota", buffer);
}

TEST(MemoryFileSystem, SingleFileOpenedFileCreateMapping)
{
	MemoryFileSystem fs;
	fs.testStoreFileContentText("/test.txt", "Ala ma kota");

	auto data = fs.openForReading("/test.txt", FileReadMode::MemoryMapped);
	ASSERT_TRUE(data);

	auto view = data->createMapping(data->fullRange());
	ASSERT_TRUE(view);
}

TEST(MemoryFileSystem, SingleFileOpenedFileMappingCorrectSize)
{
	MemoryFileSystem fs;
	fs.testStoreFileContentText("/test.txt", "Ala ma kota");

	auto data = fs.openForReading("/test.txt", FileReadMode::MemoryMapped);
	ASSERT_TRUE(data);

	auto view = data->createMapping(data->fullRange());
	ASSERT_TRUE(view);
	ASSERT_EQ(11, view->size());
}

TEST(MemoryFileSystem, SingleFileOpenedFileMappingCorrectContent)
{
	MemoryFileSystem fs;
	fs.testStoreFileContentText("/test.txt", "Ala ma kota");

	auto data = fs.openForReading("/test.txt", FileReadMode::MemoryMapped);
	ASSERT_TRUE(data);

	auto view = data->createMapping(data->fullRange());
	ASSERT_TRUE(view);

	const auto txt = StringBuf(StringView((const char*)view->data(), view->size()));
	ASSERT_STREQ("Ala ma kota", txt.c_str());
}

//--

TEST(MemoryFileSystem, MultipleFilesReported)
{
	MemoryFileSystem fs;
	fs.testStoreFileContentText("/test.txt", "Ala ma kota");
	fs.testStoreFileContentText("/hello.txt", "Ala ma kota");
	fs.testStoreFileContentText("/data.dat", "Ala ma kota");

	InplaceArray<StringBuf, 3> files;
	fs.testCollectLocalFiles("/", "*.*", files);

	ASSERT_EQ(3, files.size());
	EXPECT_STREQ("test.txt", files[0].c_str());
	EXPECT_STREQ("hello.txt", files[1].c_str());
	EXPECT_STREQ("data.dat", files[2].c_str());
}

TEST(MemoryFileSystem, MultipleDirectoriesReported)
{
	MemoryFileSystem fs;
	fs.testStoreFileContentText("/crap/test.txt", "Ala ma kota");
	fs.testStoreFileContentText("/other/test.txt", "Ala ma kota");

	InplaceArray<StringBuf, 3> dirs;
	fs.testCollectSubDirs("/", dirs);

	ASSERT_EQ(2, dirs.size());
	EXPECT_STREQ("crap", dirs[0].c_str());
	EXPECT_STREQ("other", dirs[1].c_str());
}

TEST(MemoryFileSystem, MultipleFilesCreateDirectoryOnlyOnce)
{
	MemoryFileSystem fs;
	fs.testStoreFileContentText("/crap/test.txt", "Ala ma kota");
	fs.testStoreFileContentText("/crap/hello.txt", "Ala ma kota");
	fs.testStoreFileContentText("/other/test.txt", "Ala ma kota");
	fs.testStoreFileContentText("/other/hello.txt", "Ala ma kota");

	InplaceArray<StringBuf, 3> dirs;
	fs.testCollectSubDirs("/", dirs);

	ASSERT_EQ(2, dirs.size());
	EXPECT_STREQ("crap", dirs[0].c_str());
	EXPECT_STREQ("other", dirs[1].c_str());
}

TEST(MemoryFileSystem, FileFilters)
{
	MemoryFileSystem fs;
	fs.testStoreFileContentText("/test.txt", "Ala ma kota");
	fs.testStoreFileContentText("/hello.txt", "Ala ma kota");
	fs.testStoreFileContentText("/data.dat", "Ala ma kota");

	{
		InplaceArray<StringBuf, 3> files;
		fs.testCollectLocalFiles("/", "*.txt", files);

		ASSERT_EQ(2, files.size());
		EXPECT_STREQ("test.txt", files[0].c_str());
		EXPECT_STREQ("hello.txt", files[1].c_str());
	}

	{
		InplaceArray<StringBuf, 3> files;
		fs.testCollectLocalFiles("/", "*.dat", files);

		ASSERT_EQ(1, files.size());
		EXPECT_STREQ("data.dat", files[0].c_str());
	}

	{
		InplaceArray<StringBuf, 3> files;
		fs.testCollectLocalFiles("/", "*.png", files);

		ASSERT_EQ(0, files.size());
	}
}

//--

TEST(MemoryFileSystem, FileDeletionReportsNoInfo)
{
	MemoryFileSystem fs;
	fs.testStoreFileContentText("/test.txt", "Ala ma kota");

	EXPECT_TRUE(fs.fileInfo("/test.txt"));
	EXPECT_TRUE(fs.deleteFile("/test.txt"));
	EXPECT_FALSE(fs.fileInfo("/test.txt"));
}

TEST(MemoryFileSystem, FileDeletionFailsOnNonExistingFile)
{
	MemoryFileSystem fs;
	EXPECT_FALSE(fs.fileInfo("/test.txt"));
}

TEST(MemoryFileSystem, FileDeletionSecondDeletionFails)
{
	MemoryFileSystem fs;
	fs.testStoreFileContentText("/test.txt", "Ala ma kota");

	EXPECT_TRUE(fs.deleteFile("/test.txt"));
	EXPECT_FALSE(fs.deleteFile("/test.txt"));
}

TEST(MemoryFileSystem, FileDeletionPreventsFindingIt)
{
	MemoryFileSystem fs;
	fs.testStoreFileContentText("/test.txt", "Ala ma kota");

	{
		InplaceArray<StringBuf, 3> files;
		fs.testCollectLocalFiles("/", "*.txt", files);
		EXPECT_EQ(1, files.size());
	}

	EXPECT_TRUE(fs.deleteFile("/test.txt"));

	{
		InplaceArray<StringBuf, 3> files;
		fs.testCollectLocalFiles("/", "*.txt", files);
		EXPECT_EQ(0, files.size());
	}
}

TEST(MemoryFileSystem, FileDeletionPreventsOpeningIt)
{
	MemoryFileSystem fs;
	fs.testStoreFileContentText("/test.txt", "Ala ma kota");

	{
		auto file = fs.openForReading("/test.txt", FileReadMode::MemoryMapped);
		EXPECT_TRUE(file);
	}

	EXPECT_TRUE(fs.deleteFile("/test.txt"));

	{
		auto file = fs.openForReading("/test.txt", FileReadMode::MemoryMapped);
		EXPECT_FALSE(file);
	}
}

//--

TEST(MemoryFileSystem, CreatePathCreatesDirectory)
{
	MemoryFileSystem fs;
	EXPECT_TRUE(fs.createPath("/crap/"));
	EXPECT_TRUE(fs.testHasDirectory("/crap/"));
}

TEST(MemoryFileSystem, CreatePathCreatesDirectoryThatIsReported)
{
	MemoryFileSystem fs;
	EXPECT_TRUE(fs.createPath("/crap/"));
	
	InplaceArray<StringBuf, 2> dirs;
	fs.testCollectSubDirs("/", dirs);
	ASSERT_EQ(1, dirs.size());
	EXPECT_STREQ("crap", dirs[0].c_str());
}

TEST(MemoryFileSystem, CreatePathDoesNotCreateAnotherDirectoryAsTestFileContent)
{
	MemoryFileSystem fs;
	fs.testStoreFileContentText("/crap/test.txt", "Ala ma kota");
	EXPECT_TRUE(fs.createPath("/crap/"));
	EXPECT_TRUE(fs.testHasDirectory("/crap/"));
}

TEST(MemoryFileSystem, CreatePathDoesNotCreateDirectoryTwice)
{
	MemoryFileSystem fs;
	EXPECT_TRUE(fs.createPath("/crap/"));
	EXPECT_TRUE(fs.createPath("/crap/"));

	InplaceArray<StringBuf, 2> dirs;
	fs.testCollectSubDirs("/", dirs);
	ASSERT_EQ(1, dirs.size());
	EXPECT_STREQ("crap", dirs[0].c_str());
}

TEST(MemoryFileSystem, CreatePathCreatesNestedPaths)
{
	MemoryFileSystem fs;
	EXPECT_TRUE(fs.createPath("/crap/other/music/"));
	EXPECT_TRUE(fs.testHasDirectory("/crap/"));
	EXPECT_TRUE(fs.testHasDirectory("/crap/other/"));
	EXPECT_TRUE(fs.testHasDirectory("/crap/other/music/"));
}

TEST(MemoryFileSystem, CreatePathCreatesMultipleDirectories)
{
	MemoryFileSystem fs;
	EXPECT_TRUE(fs.createPath("/crap/"));
	EXPECT_TRUE(fs.createPath("/other/"));
	EXPECT_TRUE(fs.createPath("/music/"));
}

TEST(MemoryFileSystem, CreatePathDirectoriesAreReported)
{
	MemoryFileSystem fs;
	EXPECT_TRUE(fs.createPath("/crap/"));
	EXPECT_TRUE(fs.createPath("/other/"));
	EXPECT_TRUE(fs.createPath("/music/"));

	InplaceArray<StringBuf, 3> dirs;
	fs.testCollectSubDirs("/", dirs);
	ASSERT_EQ(3, dirs.size());
	EXPECT_STREQ("crap", dirs[0].c_str());
	EXPECT_STREQ("other", dirs[1].c_str());
	EXPECT_STREQ("music", dirs[2].c_str());
}

//--

TEST(MemoryFileSystem, EmptyDirectoryCanBeDeleted)
{
	MemoryFileSystem fs;
	EXPECT_TRUE(fs.createPath("/crap/"));
	EXPECT_TRUE(fs.testHasDirectory("/crap/"));
	EXPECT_TRUE(fs.deleteDir("/crap/"));
	EXPECT_FALSE(fs.testHasDirectory("/crap/"));
}

TEST(MemoryFileSystem, DeletedDirectoryIsNoLongerRaported)
{
	MemoryFileSystem fs;
	EXPECT_TRUE(fs.createPath("/crap/"));
	EXPECT_TRUE(fs.deleteDir("/crap/"));

	InplaceArray<StringBuf, 2> dirs;
	fs.testCollectSubDirs("/", dirs);
	EXPECT_EQ(0, dirs.size());
}

TEST(MemoryFileSystem, DirectoryDeletionFailsOnNonExistingDirectory)
{
	MemoryFileSystem fs;
	EXPECT_FALSE(fs.deleteDir("/crap/"));
}

TEST(MemoryFileSystem, DirectoryDeletionDoesNotDeleteWrongDirectory)
{
	MemoryFileSystem fs;
	EXPECT_TRUE(fs.createPath("/test/"));
	EXPECT_FALSE(fs.deleteDir("/crap/"));
	EXPECT_TRUE(fs.testHasDirectory("/test/"));
}

TEST(MemoryFileSystem, DirectoryWithFilesCantBeDeleted)
{
	MemoryFileSystem fs;
	fs.testStoreFileContentText("/crap/test.txt", "Ala ma kota");
	EXPECT_TRUE(fs.testHasDirectory("/crap/"));
	EXPECT_FALSE(fs.deleteDir("/crap/"));
	EXPECT_TRUE(fs.testHasDirectory("/crap/"));
}

TEST(MemoryFileSystem, DirectoryCanBeDeletedOnceFilesAreDeleted)
{
	MemoryFileSystem fs;
	fs.testStoreFileContentText("/crap/test.txt", "Ala ma kota");
	EXPECT_FALSE(fs.deleteDir("/crap/"));
	EXPECT_TRUE(fs.deleteFile("/crap/test.txt"));
	EXPECT_TRUE(fs.deleteDir("/crap/"));
	EXPECT_FALSE(fs.testHasDirectory("/crap/"));
}

TEST(MemoryFileSystem, DirectoryWithSubDirsCantBeDeleted)
{
	MemoryFileSystem fs;
	fs.testStoreFileContentText("/crap/other/test.txt", "Ala ma kota");
	EXPECT_TRUE(fs.testHasDirectory("/crap/"));
	EXPECT_FALSE(fs.deleteDir("/crap/"));
	EXPECT_TRUE(fs.testHasDirectory("/crap/"));
}

TEST(MemoryFileSystem, DirectoryCanBeDeletedOnceSubDirsAreDeleted)
{
	MemoryFileSystem fs;
	fs.testStoreFileContentText("/crap/other/test.txt", "Ala ma kota");
	EXPECT_FALSE(fs.deleteDir("/crap/"));
	EXPECT_TRUE(fs.deleteFile("/crap/other/test.txt"));
	EXPECT_TRUE(fs.deleteDir("/crap/other/"));
	EXPECT_TRUE(fs.deleteDir("/crap/"));
	EXPECT_FALSE(fs.testHasDirectory("/crap/"));
}

TEST(MemoryFileSystem, DeletedDirectoryCanBeReadded)
{
	MemoryFileSystem fs;
	fs.createPath("/crap/");
	EXPECT_TRUE(fs.testHasDirectory("/crap/"));
	EXPECT_TRUE(fs.deleteDir("/crap/"));
	EXPECT_FALSE(fs.testHasDirectory("/crap/"));
	fs.createPath("/crap/");
	EXPECT_TRUE(fs.testHasDirectory("/crap/"));
}


//--

TEST(MemoryFileSystem, SimpleFileWriterCreated)
{
	MemoryFileSystem fs;

	auto writer = fs.openForWriting("/test.txt", FileWriteMode::WriteOnly);
	EXPECT_TRUE(writer);
}

TEST(MemoryFileSystem, SimpleFileWriterFileExistsAfterWriting)
{
	MemoryFileSystem fs;

	{
		auto writer = fs.openForWriting("/test.txt", FileWriteMode::WriteOnly);
		EXPECT_TRUE(writer);
	}

	EXPECT_TRUE(fs.fileInfo("/test.txt"));
}

TEST(MemoryFileSystem, SimpleFileWriterNormalWriterHasZeroInitialSize)
{
	MemoryFileSystem fs;

	{
		auto writer = fs.openForWriting("/test.txt", FileWriteMode::WriteOnly);
		EXPECT_TRUE(writer);
		EXPECT_EQ(0, writer->size());
	}
}

TEST(MemoryFileSystem, SimpleFileWriterFileWritingWorks)
{
	MemoryFileSystem fs;

	{
		auto writer = fs.openForWriting("/test.txt", FileWriteMode::WriteOnly);
		ASSERT_TRUE(writer);

		const char* txt = "Ala ma kota";
		const auto numWritten = writer->writeSync(txt, strlen(txt));
		ASSERT_EQ(11, numWritten);
	}
}

TEST(MemoryFileSystem, SimpleFileWritenFileReportsCorrectSize)
{
	MemoryFileSystem fs;

	{
		auto writer = fs.openForWriting("/test.txt", FileWriteMode::WriteOnly);
		ASSERT_TRUE(writer);

		const char* txt = "Ala ma kota";
		const auto numWritten = writer->writeSync(txt, strlen(txt));
		ASSERT_EQ(11, numWritten);
	}

	uint64_t size = 0;
	EXPECT_TRUE(fs.fileInfo("/test.txt", nullptr, &size));
	EXPECT_EQ(11, size);
}

TEST(MemoryFileSystem, SimpleFileWritenFileHasCorrectContent)
{
	MemoryFileSystem fs;

	{
		auto writer = fs.openForWriting("/test.txt", FileWriteMode::WriteOnly);
		ASSERT_TRUE(writer);

		const char* txt = "Ala ma kota";
		const auto numWritten = writer->writeSync(txt, strlen(txt));
		ASSERT_EQ(11, numWritten);
	}

	StringBuf txt = fs.loadFileToString("/test.txt");
	EXPECT_STREQ("Ala ma kota", txt.c_str());	
}

TEST(MemoryFileSystem, SimpleFileWritenReplacesContent)
{
	MemoryFileSystem fs;

	{
		auto writer = fs.openForWriting("/test.txt", FileWriteMode::WriteOnly);
		ASSERT_TRUE(writer);

		const char* txt = "Ala ma kota";
		writer->writeSync(txt, strlen(txt));
	}

	{
		auto writer = fs.openForWriting("/test.txt", FileWriteMode::WriteOnly);
		ASSERT_TRUE(writer);

		const char* txt = "Kot ma ale";
		const auto numWritten = writer->writeSync(txt, strlen(txt));
		ASSERT_EQ(10, numWritten);
	}

	{
		StringBuf txt = fs.loadFileToString("/test.txt");
		EXPECT_STREQ("Kot ma ale", txt.c_str());
	}
}

TEST(MemoryFileSystem, SimpleFileWriterAppendWriterHasValidInitialSize)
{
	MemoryFileSystem fs;

	{
		auto writer = fs.openForWriting("/test.txt", FileWriteMode::WriteOnly);
		EXPECT_TRUE(writer);

		const char* txt = "Ala ma kota";
		writer->writeSync(txt, strlen(txt));		
	}

	{
		auto writer = fs.openForWriting("/test.txt", FileWriteMode::ReadWrite);
		ASSERT_TRUE(writer);
		EXPECT_EQ(11, writer->size());
	}
}

TEST(MemoryFileSystem, SimpleFileWriterAppendWriterHasValidInitialPosition)
{
	MemoryFileSystem fs;

	{
		auto writer = fs.openForWriting("/test.txt", FileWriteMode::WriteOnly);
		EXPECT_TRUE(writer);

		const char* txt = "Ala ma kota";
		writer->writeSync(txt, strlen(txt));
	}

	{
		auto writer = fs.openForWriting("/test.txt", FileWriteMode::ReadWrite);
		ASSERT_TRUE(writer);
		EXPECT_EQ(11, writer->pos());
	}
}

TEST(MemoryFileSystem, SimpleFileWriterAppendWriterHasValidInitialContent)
{
	MemoryFileSystem fs;

	{
		auto writer = fs.openForWriting("/test.txt", FileWriteMode::WriteOnly);
		EXPECT_TRUE(writer);

		const char* txt = "Ala ma kota";
		writer->writeSync(txt, strlen(txt));
	}

	{
		auto writer = fs.openForWriting("/test.txt", FileWriteMode::ReadWrite);
		ASSERT_TRUE(writer);

		char text[256];
		memzero(text, sizeof(text));
		writer->seek(0);
		const auto numRead = writer->readSync(text, writer->size());
		EXPECT_EQ(11, numRead);
		EXPECT_STREQ("Ala ma kota", text);
	}
}

TEST(MemoryFileSystem, SimpleFileWriterAppendModeOverrideContent)
{
	MemoryFileSystem fs;

	{
		auto writer = fs.openForWriting("/test.txt", FileWriteMode::WriteOnly);
		ASSERT_TRUE(writer);

		const char* txt = "Ala ma kota";
		writer->writeSync(txt, strlen(txt));
	}

	{
		auto writer = fs.openForWriting("/test.txt", FileWriteMode::ReadWrite);
		ASSERT_TRUE(writer);

		const char* txt = "Bob";
		writer->seek(0);
		writer->writeSync(txt, strlen(txt));
	}

	{
		StringBuf txt = fs.loadFileToString("/test.txt");
		EXPECT_STREQ("Bob ma kota", txt.c_str());
	}
}

TEST(MemoryFileSystem, SimpleFileWritenAppendContent)
{
	MemoryFileSystem fs;

	{
		auto writer = fs.openForWriting("/test.txt", FileWriteMode::WriteOnly);
		ASSERT_TRUE(writer);

		const char* txt = "Ala ";
		writer->writeSync(txt, strlen(txt));
	}

	{
		auto writer = fs.openForWriting("/test.txt", FileWriteMode::ReadWrite);
		ASSERT_TRUE(writer);

		const char* txt = "ma ";
		writer->writeSync(txt, strlen(txt));
	}

	{
		auto writer = fs.openForWriting("/test.txt", FileWriteMode::ReadWrite);
		ASSERT_TRUE(writer);

		const char* txt = "kota";
		writer->writeSync(txt, strlen(txt));
	}
		

	{
		StringBuf txt = fs.loadFileToString("/test.txt");
		EXPECT_STREQ("Ala ma kota", txt.c_str());
	}
}

TEST(MemoryFileSystem, EnumerateFileStructureAtRoot)
{
	MemoryFileSystem fs;
	fs.testStoreFileContentText("/root.dat", "Ala ma kota");
	fs.testStoreFileContentText("/test/test.txt", "Ala ma kota");
	fs.testStoreFileContentText("/test/other.txt", "Ala ma kota");
	fs.testStoreFileContentText("/test/crap.txt", "Ala ma kota");
	fs.testStoreFileContentText("/test/deep/crap.dat", "Ala ma kota");
	fs.testStoreFileContentText("/crap/test.txt", "Ala ma kota");
	fs.testStoreFileContentText("/crap/other.txt", "Ala ma kota");
	fs.testStoreFileContentText("/crap/crap.dat", "Ala ma kota");

	InplaceArray<StringBuf, 10> paths;
	fs.testCollectFiles("/", "*.*", paths);

	ASSERT_EQ(8, paths.size());
	ASSERT_STREQ("/root.dat", paths[0].c_str());
	ASSERT_STREQ("/test/test.txt", paths[1].c_str());
	ASSERT_STREQ("/test/other.txt", paths[2].c_str());
	ASSERT_STREQ("/test/crap.txt", paths[3].c_str());
	ASSERT_STREQ("/test/deep/crap.dat", paths[4].c_str());
	ASSERT_STREQ("/crap/test.txt", paths[5].c_str());
	ASSERT_STREQ("/crap/other.txt", paths[6].c_str());
	ASSERT_STREQ("/crap/crap.dat", paths[7].c_str());	
}

TEST(MemoryFileSystem, EnumerateFileStructureAtRootWithFilter)
{
	MemoryFileSystem fs;
	fs.testStoreFileContentText("/root.dat", "Ala ma kota");
	fs.testStoreFileContentText("/test/test.txt", "Ala ma kota");
	fs.testStoreFileContentText("/test/other.txt", "Ala ma kota");
	fs.testStoreFileContentText("/test/crap.txt", "Ala ma kota");
	fs.testStoreFileContentText("/test/deep/crap.dat", "Ala ma kota");
	fs.testStoreFileContentText("/crap/test.txt", "Ala ma kota");
	fs.testStoreFileContentText("/crap/other.txt", "Ala ma kota");
	fs.testStoreFileContentText("/crap/crap.dat", "Ala ma kota");

	InplaceArray<StringBuf, 10> paths;
	fs.testCollectFiles("/", "*.txt", paths);

	ASSERT_EQ(5, paths.size());
	ASSERT_STREQ("/test/test.txt", paths[0].c_str());
	ASSERT_STREQ("/test/other.txt", paths[1].c_str());
	ASSERT_STREQ("/test/crap.txt", paths[2].c_str());
	ASSERT_STREQ("/crap/test.txt", paths[3].c_str());
	ASSERT_STREQ("/crap/other.txt", paths[4].c_str());
}

TEST(MemoryFileSystem, EnumerateFileStructureAtDirectory)
{
	MemoryFileSystem fs;
	fs.testStoreFileContentText("/root.dat", "Ala ma kota");
	fs.testStoreFileContentText("/test/test.txt", "Ala ma kota");
	fs.testStoreFileContentText("/test/other.txt", "Ala ma kota");
	fs.testStoreFileContentText("/test/crap.txt", "Ala ma kota");
	fs.testStoreFileContentText("/test/deep/crap.dat", "Ala ma kota");
	fs.testStoreFileContentText("/crap/test.txt", "Ala ma kota");
	fs.testStoreFileContentText("/crap/other.txt", "Ala ma kota");
	fs.testStoreFileContentText("/crap/crap.dat", "Ala ma kota");

	InplaceArray<StringBuf, 10> paths;
	fs.testCollectFiles("/test/", "*.*", paths);

	ASSERT_EQ(4, paths.size());
	ASSERT_STREQ("/test/test.txt", paths[0].c_str());
	ASSERT_STREQ("/test/other.txt", paths[1].c_str());
	ASSERT_STREQ("/test/crap.txt", paths[2].c_str());
	ASSERT_STREQ("/test/deep/crap.dat", paths[3].c_str());
}

//--

END_INFERNO_NAMESPACE()