/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "private.h"
#include "directoryIteratorWinApi.h"
#include "bm/core/containers/include/utf8StringFunctions.h"

#ifdef PLATFORM_WINAPI

BEGIN_INFERNO_NAMESPACE_EX(windows)

//--

DirectoryIterator::DirectoryIterator(const wchar_t* pathWithPattern, bool allowFiles, bool allowDirs)
    : m_allowDirs(allowDirs)
    , m_allowFiles(allowFiles)
{
    // Format the search path
    m_findHandle = FindFirstFileW(pathWithPattern, &m_findData);

    // Get first valid entry
    while (!validateEntry())
        if (!nextEntry())
            break;
}

DirectoryIterator::~DirectoryIterator()
{
    // Close search handle
    if (m_findHandle != INVALID_HANDLE_VALUE)
    {
        FindClose(m_findHandle);
        m_findHandle = INVALID_HANDLE_VALUE;
    }
}

const wchar_t* DirectoryIterator::fileNameRaw() const
{
    if (m_findHandle != INVALID_HANDLE_VALUE)
        return m_findData.cFileName;

    return nullptr;
}

const char* DirectoryIterator::fileName() const
{
    if (m_findHandle != INVALID_HANDLE_VALUE)
    {
        utf8::FromUniChar(m_fileName, MAX_PATH-1, m_findData.cFileName, wcslen(m_findData.cFileName));
        return m_fileName;
    }

    return nullptr;
}

bool DirectoryIterator::validateEntry() const
{
    if (m_findHandle == INVALID_HANDLE_VALUE)
        return false;

    if (0 == wcscmp(m_findData.cFileName, L"."))
        return false;

    if (0 == wcscmp(m_findData.cFileName, L".."))
        return false;

    // Skip filtered
    bool isDirectory = 0 != (m_findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
    if ((isDirectory && !m_allowDirs) || (!isDirectory && !m_allowFiles))
        return false;

    return true;
}

bool DirectoryIterator::nextEntry()
{
    if (m_findHandle == INVALID_HANDLE_VALUE)
        return false;

    if (!FindNextFile(m_findHandle, &m_findData))
    {
        FindClose(m_findHandle);
        m_findHandle = INVALID_HANDLE_VALUE;
        return false;
    }

    return true;
}

void DirectoryIterator::operator++(int)
{
    while (nextEntry())
        if (validateEntry())
            break;
}

void DirectoryIterator::operator++()
{
    while (nextEntry())
        if (validateEntry())
            break;
}

DirectoryIterator::operator bool() const
{
    return m_findHandle != INVALID_HANDLE_VALUE;
}

//--

END_INFERNO_NAMESPACE_EX(windows)

#endif
