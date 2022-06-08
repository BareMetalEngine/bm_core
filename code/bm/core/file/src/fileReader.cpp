/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "fileReader.h"
#include "fileSystem.h"
#include "bm/core/task/include/taskSignal.h"

BEGIN_INFERNO_NAMESPACE()

//--

IFileReader::IFileReader(FileFlags flags, StringBuf info, uint64_t size)
    : m_flags(flags)
    , m_info(info)
    , m_size(size)
{}

IFileReader::~IFileReader()
{}

//--

bool IFileReader::readAsync(TaskContext& tc, FileAbsoluteRange range, void* ptr, uint32_t& outNumRead)
{
	auto signal = TaskSignal::Create(1, "AsyncIO"_id);
	bool status = true;

	uint32_t numRead = 0;
	readAsync(range, ptr, [&numRead, &status, &signal, range](int actualReadSize)
		{
			numRead = actualReadSize;
			if (actualReadSize <= 0)
				status = false;
			signal.trip();
		});

	signal.waitWithYeild(tc);

	if (status)
	{
		outNumRead = numRead;
		return true;
	}

	return false;
}

//--


END_INFERNO_NAMESPACE()
