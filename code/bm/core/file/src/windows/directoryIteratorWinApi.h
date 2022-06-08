/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#ifdef PLATFORM_WINAPI

BEGIN_INFERNO_NAMESPACE_EX(windows)

//--

/// File iterator for enumerating directory structure
class DirectoryIterator : public NoCopy
{
public:
    INLINE bool areDirectoriesAllowed() const { return m_allowDirs; }
    INLINE bool areFilesAllowed() const { return m_allowFiles; }

    //---

    DirectoryIterator(const wchar_t* pathWithPattern, bool allowFiles, bool allowDirs);
    ~DirectoryIterator();

    void operator++(int);
    void operator++();

    operator bool() const;

    const wchar_t* fileNameRaw() const;
    const char* fileName() const;

private:
    bool validateEntry() const;
    bool nextEntry();

    WIN32_FIND_DATA m_findData;
    HANDLE m_findHandle = NULL;
    bool m_allowDirs = false;
    bool m_allowFiles = false;

    mutable char m_fileName[MAX_PATH];
};

//--

END_INFERNO_NAMESPACE_EX(windows)

#endif
