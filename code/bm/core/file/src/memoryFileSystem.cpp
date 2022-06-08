/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "memoryFileSystem.h"
#include "fileMemoryWriter.h"

#include "bm/core/containers/include/path.h"
#include "bm/core/file/include/fileReader.h"
#include "bm/core/containers/include/inplaceArray.h"

BEGIN_INFERNO_NAMESPACE()

//--

class MemoryFileSystemWatcher : public IDirectoryWatcher
{
public:
	MemoryFileSystemWatcher()
	{}

	void MemoryFileSystemWatcher::dispatchEvent(const DirectoryWatcherEvent& evt)
	{
		IDirectoryWatcher::dispatchEvent(evt);
	}
};

//--

MemoryFileSystem::MemoryFileSystem()
{
	auto* root = m_directoryPool.create();
	root->name = "";
	m_root = root;
}

MemoryFileSystem::~MemoryFileSystem()
{
	cleanupDirectory(m_root);
	m_root = nullptr;
}

void MemoryFileSystem::cleanupDirectory(Directory* dir)
{
	{
		auto* elem = dir->firstFile;
		while (elem)
		{
			auto* next = elem->next;
			m_filePool.free(elem);
			elem = next;
		}
	}

	{
		auto* elem = dir->firstDir;
		while (elem)
		{
			auto* next = elem->next;
			cleanupDirectory(elem);
			elem = next;
		}
	}

	m_directoryPool.free(dir);
}

bool MemoryFileSystem::testUpdateFileContentExternal(StringView path, Buffer data, TimeStamp timestamp /*= TimeStamp::GetNow()*/)
{
	DEBUG_CHECK_RETURN_EX_V(ValidateDepotFilePath(path), "Invalid path", false);

	auto* dir = testDirectoryEntryFind(path.pathParent());
	DEBUG_CHECK_RETURN_EX_V(dir, "Directory entry does not exist yet or is deleted", false);

	const auto fileName = path.pathFileName();

	File* file = nullptr;
	for (auto* entry = dir->firstFile; entry; entry = entry->next)
	{
		if (entry->name == fileName)
		{
			file = entry;
			break;
		}
	}

	if (file)
	{
		file->content = data;
		file->timestamp = timestamp;

		if (file->deleted)
		{
			notiftFileAdded(file);
			file->deleted = false;
		}
		else
		{
			notiftFileChanged(file);
		}
	}
	else
	{
		auto* info = m_filePool.create();
		info->parent = dir;
		info->content = data;
		info->name = StringBuf(path.pathFileName());
		info->readonly = false;
		info->timestamp = timestamp;
		info->next = nullptr;

		if (dir->lastFile)
			dir->lastFile->next = info;
		else
			dir->firstFile = info;
		dir->lastFile = info;

		notiftFileAdded(info);
	}

	return true;
}

bool MemoryFileSystem::testStoreFileContent(StringView path, Buffer data, TimeStamp timestamp /*= TimeStamp::GetNow()*/)
{
	DEBUG_CHECK_RETURN_EX_V(ValidateDepotFilePath(path), "Invalid path", false);

	auto* dir = testCreatePath(path.pathParent(), false);
	DEBUG_CHECK_RETURN_EX_V(dir, "Directory entry does not exist yet or is deleted", false);

	const auto fileName = path.pathFileName();
	for (auto* file = dir->firstFile; file; file = file->next)
	{
		DEBUG_CHECK_RETURN_EX_V(file->name != fileName, "File already exists in directory", false);
	}

	auto* info = m_filePool.create();
	info->parent = dir;
	info->content = data;
	info->name = StringBuf(fileName);
	info->readonly = false;
	info->timestamp = timestamp;
	info->next = nullptr;

	if (dir->lastFile)
		dir->lastFile->next = info;
	else
		dir->firstFile = info;
	dir->lastFile = info;

	return true;
}

bool MemoryFileSystem::testStoreFileContentText(StringView path, StringView data, TimeStamp timestamp /*= TimeStamp::GetNow()*/)
{
	return testStoreFileContent(path, data.toBuffer(), timestamp);
}

bool MemoryFileSystem::testHasFile(StringView path) const
{
	const auto* file = testFileEntryFind(path);
	return file != nullptr;
}

bool MemoryFileSystem::testHasDirectory(StringView path) const
{
	const auto* dir = testDirectoryEntryFind(path);
	return dir != nullptr;
}

Buffer MemoryFileSystem::testLoadFileContent(StringView path) const
{
	const auto* file = testFileEntryFind(path);
	return file ? file->content : nullptr;
}

StringBuf MemoryFileSystem::testLoadFileContentText(StringView path) const
{
	const auto* file = testFileEntryFind(path);
	return file ? StringBuf(file->content) : nullptr;
}

//--

FileReaderPtr MemoryFileSystem::openForReading(StringView absoluteFilePath, FileReadMode mode, TimeStamp* outTimestamp /*= nullptr*/) const
{
	if (const auto* file = testFileEntryFind(absoluteFilePath))
	{
		if (outTimestamp)
			*outTimestamp = file->timestamp;

		return IFileReader::CreateFromBuffer(file->content, StringBuf(absoluteFilePath));
	}

	return nullptr;
}

class MemoryFileSystemFileWriter : public FileMemoryWriter
{
public:
	MemoryFileSystemFileWriter(MemoryFileSystem& fs, StringBuf path)
		: FileMemoryWriter(path)
		, m_path(path)
		, m_fs(fs)
	{}

	virtual ~MemoryFileSystemFileWriter()
	{
		auto content = exportDataToBuffer();
		m_fs.testUpdateFileContentExternal(m_path, content);
	}

private:
	MemoryFileSystem& m_fs;
	StringBuf m_path;
};

FileWriterPtr MemoryFileSystem::openForWriting(StringView absoluteFilePath, FileWriteMode mode)
{
	auto dirPath = absoluteFilePath.pathParent();
	auto* dir = testDirectoryEntryFind(dirPath);
	if (!dir)
		return nullptr;

	auto ret = RefNew<MemoryFileSystemFileWriter>(*this, StringBuf(absoluteFilePath));

	if (mode == FileWriteMode::ReadWrite)
	{
		if (auto* file = testFileEntryFind(absoluteFilePath))
			ret->writeSync(file->content.data(), file->content.size());
	}

	return ret;
}

MemoryFileSystem::File* MemoryFileSystem::testFileEntryFind(StringView path)
{
	return const_cast<MemoryFileSystem::File*>(const_cast<const MemoryFileSystem*>(this)->testFileEntryFind(path));
}

const MemoryFileSystem::File* MemoryFileSystem::testFileEntryFind(StringView path) const
{
	ASSERT(ValidateDepotFilePath(path));

	const auto dirPath = path.pathParent();
	const auto fileName = path.pathFileName();

	if (auto* dir = testDirectoryEntryFind(dirPath))
		for (auto* file = dir->firstFile; file; file = file->next)
			if (!file->deleted && file->name == fileName)
				return file;

	return nullptr;
}

MemoryFileSystem::Directory* MemoryFileSystem::testDirectoryEntryFind(StringView path)
{
	return const_cast<MemoryFileSystem::Directory*>(const_cast<const MemoryFileSystem*>(this)->testDirectoryEntryFind(path));
}

const MemoryFileSystem::Directory* MemoryFileSystem::testDirectoryEntryFind(StringView path) const
{
	ASSERT(ValidateDepotDirPath(path));

	InplaceArray<StringView, 10> parts;
	path.slice('/', parts);

	const Directory* dir = m_root;
	for (auto part : parts)
	{
		const Directory* childDir = nullptr;
		for (const auto* testDir = dir->firstDir; testDir; testDir = testDir->next)
		{
			if (!testDir->deleted && testDir->name == part)
			{
				childDir = testDir;
				break;
			}
		}

		if (!childDir)
			return nullptr;

		dir = childDir;
	}

	return dir;
}

MemoryFileSystem::Directory* MemoryFileSystem::testCreatePath(StringView path, bool notify)
{
	DEBUG_CHECK_RETURN_EX_V(ValidateDepotDirPath(path), "Invalid path", nullptr);

	InplaceArray<StringView, 10> parts;
	path.slice('/', parts);

	if (!path.endsWith("/") && !parts.empty())
		parts.popBack();

	Directory* dir = m_root;
	for (auto part : parts)
	{
		Directory* childDir = nullptr;
		for (auto* testDir = dir->firstDir; testDir; testDir = testDir->next)
		{
			if (testDir->name == part)
			{
				childDir = testDir;
				break;
			}
		}

		if (childDir)
		{
			if (childDir->deleted)
			{
				childDir->deleted = false;

				if (notify)
					notiftDirectoryAdded(childDir);
			}
		}
		else
		{
			childDir = m_directoryPool.create();
			childDir->parent = dir;
			childDir->name = StringBuf(part);

			if (dir->lastDir)
				dir->lastDir->next = childDir;
			else
				dir->firstDir = childDir;
			dir->lastDir = childDir;

			if (notify)
				notiftDirectoryAdded(childDir);
		}

		dir = childDir;
	}

	return dir;
}

bool MemoryFileSystem::createPath(StringView path)
{
	return nullptr != testCreatePath(path, true);
}

bool MemoryFileSystem::moveFile(StringView srcAbsolutePath, StringView destAbsolutePath)
{
	auto* srcEntry = testFileEntryFind(srcAbsolutePath);
	if (!srcEntry || srcEntry->deleted)
		return false;

	if (testFileEntryFind(destAbsolutePath))
		return false;

	const auto destAbsoluteDirPath = destAbsolutePath.pathParent();
	auto* destDir = testDirectoryEntryFind(destAbsoluteDirPath);
	if (!destDir)
		return false;

	srcEntry->deleted = true;
	notiftFileRemoved(srcEntry);

	if (!testUpdateFileContentExternal(destAbsolutePath, srcEntry->content, srcEntry->timestamp))
	{
		srcEntry->deleted = false;
		notiftFileAdded(srcEntry);
		return false;
	}

	return true;
}

bool MemoryFileSystem::copyFile(StringView srcAbsolutePath, StringView destAbsolutePath)
{
	auto* srcEntry = testFileEntryFind(srcAbsolutePath);
	if (!srcEntry || srcEntry->deleted)
		return false;

	if (testFileEntryFind(destAbsolutePath))
		return false;

	const auto destAbsoluteDirPath = destAbsolutePath.pathParent();
	auto* destDir = testDirectoryEntryFind(destAbsoluteDirPath);
	if (!destDir)
		return false;

	testUpdateFileContentExternal(destAbsolutePath, srcEntry->content, srcEntry->timestamp);
	return true;
}

bool MemoryFileSystem::deleteFile(StringView absoluteFilePath)
{
	auto* srcEntry = testFileEntryFind(absoluteFilePath);
	if (!srcEntry || srcEntry->deleted)
		return false;

	srcEntry->deleted = true;
	notiftFileRemoved(srcEntry);

	return true;
}

bool MemoryFileSystem::deleteDir(StringView absoluteDirPath)
{
	auto* srcEntry = testDirectoryEntryFind(absoluteDirPath);
	if (!srcEntry || srcEntry->deleted || srcEntry == m_root)
		return false;

	for (auto* file = srcEntry->firstFile; file; file = file->next)
		if (!file->deleted)
			return false;

	for (auto* dir = srcEntry->firstDir; dir; dir = dir->next)
		if (!dir->deleted)
			return false;

	srcEntry->deleted = true;
	notiftDirectoryRemoved(srcEntry);

	return true;
}

bool MemoryFileSystem::fileInfo(StringView absoluteFilePath, TimeStamp* outTimeStamp /*= nullptr*/, uint64_t* outFileSize /*= nullptr*/) const
{
	const auto* srcEntry = testFileEntryFind(absoluteFilePath);
	if (!srcEntry || srcEntry->deleted)
		return false;

	if (outTimeStamp)
		*outTimeStamp = srcEntry->timestamp;

	if (outFileSize)
		*outFileSize = srcEntry->content.size();

	return true;
}

bool MemoryFileSystem::touchFile(StringView absoluteFilePath)
{
	auto* srcEntry = testFileEntryFind(absoluteFilePath);
	if (!srcEntry || srcEntry->deleted)
		return false;

	srcEntry->timestamp = TimeStamp::GetNow();
	notiftFileChanged(srcEntry);
	return true;
}

bool MemoryFileSystem::isFileReadOnly(StringView absoluteFilePath) const
{
	const auto* srcEntry = testFileEntryFind(absoluteFilePath);
	if (!srcEntry || srcEntry->deleted)
		return false;

	return srcEntry->readonly;
}

bool MemoryFileSystem::readOnlyFlag(StringView absoluteFilePath, bool flag)
{
	auto* srcEntry = testFileEntryFind(absoluteFilePath);
	if (!srcEntry || srcEntry->deleted)
		return false;

	srcEntry->readonly = flag;
	return true;
}

//--

bool MemoryFileSystem::enumFiles(StringView absoluteFilePath, StringView searchPattern, const std::function<bool(StringView fullPath, StringView fileName)>& enumFunc, bool recurse) const
{
	DEBUG_CHECK_RETURN_EX_V(ValidateDepotDirPath(absoluteFilePath), "Invalid path", false);

	InplaceArray<const Directory*, 64> dirStack;

	{
		const auto* entry = testDirectoryEntryFind(absoluteFilePath);
		if (entry && !entry->deleted)
			dirStack.pushBack(entry);
	}

	while (!dirStack.empty())
	{
		auto* dir = dirStack.back();
		dirStack.popBack();

		for (const auto* file = dir->firstFile; file; file = file->next)
		{
			if (file->deleted)
				continue;

			if (!file->name.view().matchPattern(searchPattern))
				continue;

			const auto fullPath = fileAbsolutePath(file);
			if (enumFunc(fullPath, file->name))
				return true;
		}

		if (recurse)
		{
			auto start = dirStack.size();

			for (const auto* childDir = dir->firstDir; childDir; childDir = childDir->next)
			{
				if (childDir->deleted)
					continue;

				dirStack.pushBack(childDir);
			}

			std::reverse(dirStack.begin() + start, dirStack.end());
		}
	}

	return false;
}

bool MemoryFileSystem::enumSubDirs(StringView absoluteFilePath, const std::function<bool(StringView name)>& enumFunc) const
{
	DEBUG_CHECK_RETURN_EX_V(ValidateDepotDirPath(absoluteFilePath), "Invalid path", false);

	const auto* entry = testDirectoryEntryFind(absoluteFilePath);
	if (!entry || entry->deleted)
		return false;

	for (const auto* childDir = entry->firstDir; childDir; childDir = childDir->next)
	{
		if (childDir->deleted)
			continue;

		if (enumFunc(childDir->name))
			return true;
	}

	return false;
}

bool MemoryFileSystem::enumLocalFiles(StringView absoluteFilePath, StringView searchPattern, const std::function<bool(StringView name)>& enumFunc) const
{
	DEBUG_CHECK_RETURN_EX_V(ValidateDepotDirPath(absoluteFilePath), "Invalid path", false);

	const auto* entry = testDirectoryEntryFind(absoluteFilePath);
	if (!entry || entry->deleted)
		return false;

	for (const auto* file = entry->firstFile; file; file = file->next)
	{
		if (file->deleted)
			continue;

		if (!file->name.view().matchPattern(searchPattern))
			continue;

		if (enumFunc(file->name))
			return true; 
	}

	return false;
}

bool MemoryFileSystem::enumFileSystemRoots(const std::function<bool(StringView name)>& enumFunc, bool allowNetworkDrives) const
{
	return enumFunc("/");
}

//--

DirectoryWatcherPtr MemoryFileSystem::createDirectoryWatcher(StringView path)
{
	auto watcher = RefNew<MemoryFileSystemWatcher>();
	m_watchers.pushBack(watcher.get());
	return watcher;
}

//--

bool MemoryFileSystem::loadFileToBuffer(StringView absoluteFilePath, IPoolUnmanaged& pool, Buffer& outBuffer, TimeStamp* outTimestamp /*= nullptr*/, FileReadMode mode /*= FileReadMode::MemoryMapped*/) const
{
	const auto* entry = testFileEntryFind(absoluteFilePath);
	if (!entry || entry->deleted)
		return false;

	outBuffer = entry->content;

	if (outTimestamp)
		*outTimestamp = entry->timestamp;

	return true;
}

bool MemoryFileSystem::saveFileFromBuffer(StringView absoluteFilePath, BufferView data, const TimeStamp* timestampToAssign /*= nullptr*/)
{
	if (auto* file = testFileEntryFind(absoluteFilePath))
	{
		file->content = Buffer::CreateFromCopy(MainPool(), data);
		file->timestamp = timestampToAssign ? *timestampToAssign : TimeStamp::GetNow();

		notiftFileChanged(file);
	}
	else
	{
		if (!testStoreFileContent(absoluteFilePath, Buffer::CreateFromCopy(MainPool(), data)))
			return false;

		if (auto* file = testFileEntryFind(absoluteFilePath))
			notiftFileAdded(file);
	}

	return true;
}

//--

void MemoryFileSystem::dispatchFileSystemEvent(const DirectoryWatcherEvent& evt)
{
	for (const auto& watcher : m_watchers)
		if (auto locked = watcher.lock())
			locked->dispatchEvent(evt);
}

void MemoryFileSystem::dirAbsolutePath(const Directory* dir, IFormatStream& f) const
{
	DEBUG_CHECK_RETURN_EX(dir, "Invalid directory");

	if (dir == m_root)
	{
		f << "/";
	}
	else
	{
		ASSERT(dir->parent != nullptr);
		dirAbsolutePath(dir->parent, f);

		f << dir->name;
		f << "/";
	}
}

StringBuf MemoryFileSystem::fileAbsolutePath(const File* file) const
{
	DEBUG_CHECK_RETURN_EX_V(file, "Invalid file", "");

	TempString txt;
	dirAbsolutePath(file->parent, txt);
	txt << file->name;	
	return txt;
}

StringBuf MemoryFileSystem::dirAbsolutePath(const Directory* dir) const
{
	DEBUG_CHECK_RETURN_EX_V(dir, "Invalid directory", "");

	TempString txt;
	dirAbsolutePath(dir, txt);
	return txt;
}

void MemoryFileSystem::notiftFileAdded(const File* file)
{
	DirectoryWatcherEvent evt;
	evt.type = DirectoryWatcherEventType::FileAdded;
	evt.path = fileAbsolutePath(file);
	dispatchFileSystemEvent(evt);	
}

void MemoryFileSystem::notiftFileRemoved(const File* file)
{
	DirectoryWatcherEvent evt;
	evt.type = DirectoryWatcherEventType::FileRemoved;
	evt.path = fileAbsolutePath(file);
	dispatchFileSystemEvent(evt);
}

void MemoryFileSystem::notiftFileChanged(const File* file)
{
	DirectoryWatcherEvent evt;
	evt.type = DirectoryWatcherEventType::FileContentChanged;
	evt.path = fileAbsolutePath(file);
	dispatchFileSystemEvent(evt);
}

void MemoryFileSystem::notiftDirectoryAdded(const Directory* dir)
{
	DirectoryWatcherEvent evt;
	evt.type = DirectoryWatcherEventType::DirectoryAdded;
	evt.path = dirAbsolutePath(dir);
	dispatchFileSystemEvent(evt);
}

void MemoryFileSystem::notiftDirectoryRemoved(const Directory* dir)
{
	DirectoryWatcherEvent evt;
	evt.type = DirectoryWatcherEventType::DirectoryRemoved;
	evt.path = dirAbsolutePath(dir);
	dispatchFileSystemEvent(evt);
}

//--

END_INFERNO_NAMESPACE()