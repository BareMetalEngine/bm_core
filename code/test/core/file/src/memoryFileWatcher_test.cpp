/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"

#include "bm/core/file/include/fileMemoryWriter.h"
#include "bm/core/file/include/memoryFileSystem.h"

BEGIN_INFERNO_NAMESPACE()

//--

TEST(MemoryFileWatcher, FileWatcherCreated)
{
	MemoryFileSystem fs;

	auto watcher = fs.createDirectoryWatcher("/");
	EXPECT_NE(nullptr, watcher);
}

TEST(MemoryFileWatcher, FileWatcherReportsNoEventsAfterCreation)
{
	MemoryFileSystem fs;

	auto watcher = fs.createDirectoryWatcher("/");
	ASSERT_NE(nullptr, watcher);

	//watcher->pu
}

TEST(MemoryFileWatcher, FileWatcherReportsFileAdded)
{
	MemoryFileSystem fs;

	auto watcher = fs.createDirectoryWatcher("/");
	EXPECT_NE(nullptr, watcher);
}
//--

END_INFERNO_NAMESPACE()