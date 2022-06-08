/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "stringView.h"
#include "stringBuf.h"
#include "path.h"
#include "utf8StringFunctions.h"
#include "inplaceArray.h"

BEGIN_INFERNO_NAMESPACE()

#if defined(PLATFORM_WINDOWS)
	static const char SYSTEM_PATH_SEPARATOR = '\\';
	static const char WRONG_SYSTEM_PATH_SEPARATOR = '/';
#else
	static const char SYSTEM_PATH_SEPARATOR = '/';
	static const char WRONG_SYSTEM_PATH_SEPARATOR = '\\';
#endif

//--

static const char* InvalidFileNames[] = {
    "CON", "PRN", "AUX", "NU", "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9", "LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9"
};

bool IsValidPathChar(uint32_t ch)
{
    if (ch <= 31)
        return false;

    if (ch >= 0xFFFE)
        return false;

    switch (ch)
    {
    case '<':
    case '>':
    case ':':
    case '\"':
    case '/':
    case '\\':
    case '|':
    case '?':
    case '*':
        return false;

    case '.': // disallow dot in file names - it's confusing AF
        return false;

    case 0:
        return false;
    }

    return true;
}

bool IsValidExtensionChar(uint32_t ch)
{
	if (ch <= 32)
		return false;

	if (ch >= 0xFFFE)
		return false;

	switch (ch)
	{
	case '<':
	case '>':
	case ':':
	case '\"':
	case '/':
	case '\\':
	case '|':
	case '?':
	case '*':
		return false;

	case '.': // disallow dot in file names - it's confusing AF
		return false;

	case 0:
		return false;
	}

	return true;
}

static bool IsValidName(StringView name)
{
    for (const auto testName : InvalidFileNames)
        if (name.compare(testName, StringCaseComparisonMode::NoCase) == 0)
            return false;

    return true;
}

bool MakeSafeFileName(StringView fileName, StringBuf& outFixedFileName)
{
    if (ValidateFileName(fileName))
    {
        outFixedFileName = StringBuf(fileName);
        return true;
    }

    StringBuilder txt;

    for (utf8::CharIterator it(fileName); it; ++it)
    {
        if (*it < 32)
            continue;

        if (IsValidPathChar(*it))
            txt.appendUTF32(*it);
        else
            txt.append("_");
    }

    if (txt.empty())
        return false; // not a single char was valid

    outFixedFileName = StringBuf(txt);
    return true;
}

bool ValidateFileName(StringView txt)
{
    if (!IsValidName(txt))
        return false;

    if (txt.beginsWith("."))
        txt = txt.subString(1);

	if (txt.empty())
		return false;

    if ((uint8_t)txt.data()[0] <= 32)
        return false;

	if ((uint8_t)txt.data()[txt.length()-1] <= 32)
		return false;

    for (utf8::CharIterator it(txt); it; ++it)
        if (!IsValidPathChar(*it))
            return false;

    return true;
}

bool ValidateExtension(StringView txt)
{
	if (txt.empty())
		return false;

	if ((uint8_t)txt.data()[0] <= 32)
		return false;

	if ((uint8_t)txt.data()[txt.length() - 1] <= 32)
		return false;

	for (utf8::CharIterator it(txt); it; ++it)
		if (!IsValidExtensionChar(*it))
			return false;

    return true;
}

bool ValidateFileNameWithExtension(StringView text)
{
    InplaceArray<StringView, 10> parts;
    text.slice(".", parts, { StringSliceBit::KeepEmpty, StringSliceBit::IgnoreTrim });

    if (parts.size() < 2)
        return false;

    if (!ValidateFileName(parts[0]))
        return false;

    for (uint32_t i=1; i<parts.size(); ++i)
        if (!ValidateExtension(parts[i]))
            return false;

    return true;
}

bool ValidateDepotFilePath(StringView text)
{
    if (!text.beginsWith("/"))
        return false;

	InplaceArray<StringView, 32> parts;
    text.subString(1).slice("/", parts, { StringSliceBit::KeepEmpty, StringSliceBit::IgnoreTrim });

    if (parts.empty())
        return false;

	if (!ValidateFileNameWithExtension(parts.back()))
		return false;
    parts.popBack();

	for (auto part : parts)
        if (!ValidateFileName(part))
            return false;

    return true;
}

bool ValidateDepotDirPath(StringView text)
{
    if (!text.beginsWith("/"))
        return false;
    if (!text.endsWith("/"))
        return false;

	InplaceArray<StringView, 32> parts;
    text.subString(1, text.length()-2).slice("/", parts, { StringSliceBit::KeepEmpty, StringSliceBit::IgnoreTrim });

	for (auto part : parts)
		if (!ValidateFileName(part))
			return false;

	return true;
}

//---

StringBuf ConformDepotFilePath(StringView path)
{
    path = path.trim();
    if (path.endsWith("/") || path.endsWith("\\"))
        return "";

    InplaceArray<StringView, 20> parts;
    path.slice("/\\", parts);

    if (parts.empty())
        return "";

    InplaceArray<StringView, 20> normalizedParts;
    for (const auto& part : parts)
    {
        if (part == "..")
        {
            if (normalizedParts.empty())
                return "";
            else
                normalizedParts.popBack();
        }
        else if (part != ".")
        {
            normalizedParts.pushBack(part);
        }
    }

	StringBuilder txt;
    for (const auto& part : normalizedParts)
    {
        txt << "/";
        txt << part;
    }

    return StringBuf(txt);
}

StringBuf ConformDepotDirectoryPath(StringView path)
{
	InplaceArray<StringView, 20> parts;
	path.slice("/\\", parts);

	InplaceArray<StringView, 20> normalizedParts;
	for (const auto& part : parts)
	{
		if (part == "..")
		{
			if (normalizedParts.empty())
				return "";
			else
				normalizedParts.popBack();
		}
		else if (part != ".")
		{
            if (part.findFirstChar('.') != INDEX_NONE)
                return "";

			normalizedParts.pushBack(part);
		}
	}

	StringBuilder txt;
	for (const auto& part : normalizedParts)
	{
		txt << "/";
		txt << part;
	}

    txt << "/";
	return StringBuf(txt);
}

//---

bool BuildRelativePath(StringView basePath, StringView targetPath, StringBuf& outRelativePath)
{
    InplaceArray<StringView, 20> basePathParts;
    basePath.slice("/\\", basePathParts);

    InplaceArray<StringView, 20> targetPathParts;
    targetPath.slice("/\\", targetPathParts);

    StringView finalFileName;
    if (!targetPath.endsWith("/") && !targetPath.endsWith("\\"))
    {
        finalFileName = targetPathParts.back();
        targetPathParts.popBack();
    }

    // check how many parts are the same
    uint32_t numSame = 0;
    {
        uint32_t maxTest = std::min<uint32_t>(basePathParts.size(), targetPathParts.size());
        for (uint32_t i = 0; i < maxTest; ++i)
        {
            auto basePart = basePathParts[i];
            auto targetPart = targetPathParts[i];
            if (basePart.compare(targetPart, StringCaseComparisonMode::NoCase) == 0)
                numSame += 1;
            else
                break;
        }
    }

    // for all remaining base path parts we will have to exit the directories via ..
    InplaceArray<StringView, 20> finalParts;
    for (uint32_t i = numSame; i < basePathParts.size(); ++i)
        finalParts.pushBack("..");

    // in similar fashion all different parts of the target path has to be added as well
    for (uint32_t i = numSame; i < targetPathParts.size(); ++i)
        finalParts.pushBack(targetPathParts[i]);

    // build the relative path
    StringBuilder ret;
    if (finalParts.empty())
    {
        if (!finalFileName)
            ret << "./";
    }
    else
    {
        for (const auto& part : finalParts)
        {
            ret << part;
            ret << "/";
        }
    }

    if (finalFileName)
        ret << finalFileName;

    outRelativePath = StringBuf(ret);
    return true;
}

bool ApplyRelativePath(StringView contextPath, StringView relativePath, StringBuf& outPath)
{
    // starts with absolute marker ?
    const auto contextAbsolute = contextPath.beginsWith("/");
    if (contextAbsolute)
        contextPath = contextPath.subString(1);

    // split path into parts
    InplaceArray<StringView, 20> referencePathParts;
    contextPath.slice("/\\", referencePathParts);

    // remove the last path that's usually the file name
    if (!referencePathParts.empty() && !(contextPath.endsWith("/") || contextPath.endsWith("\\")))
        referencePathParts.popBack();

    // if relative path is in fact absolute path (starts with path separator) discard all context data
    if (relativePath.beginsWith("/") || relativePath.beginsWith("\\"))
        referencePathParts.reset();

    // split control path
    InplaceArray<StringView, 20> pathParts;
    relativePath.slice("/\\", pathParts);

    // append
    for (auto& part : pathParts)
    {
        // single path, nothing
        if (part == ".")
            continue;

        // go up
        if (part == "..")
        {
            if (referencePathParts.empty())
                return false;
            referencePathParts.popBack();
            continue;
        }

        // append
        referencePathParts.pushBack(part);
    }

    // reassemble into a full path
    StringBuilder reassemblePathBuilder;
    if (contextAbsolute)
        reassemblePathBuilder.appendch(SYSTEM_PATH_SEPARATOR);

    const auto relativePathIsDir = relativePath.endsWith("/") || relativePath.endsWith("\\");
    for (auto i : referencePathParts.indexRange())
    {
        const auto& part = referencePathParts[i];

        reassemblePathBuilder << part;

        if (relativePathIsDir || i < referencePathParts.lastValidIndex())
            reassemblePathBuilder.appendch(SYSTEM_PATH_SEPARATOR);
    }

    // export created path
    outPath = StringBuf(reassemblePathBuilder);
    return true;
}

bool NormalizePath(StringView path, StringBuf& outPath)
{
    InplaceArray<StringView, 20> parts;
    path.slice("/\\", parts);

    const bool isDepotPath = path.beginsWith("/") || path.beginsWith("\\");
    const bool isDirPath = path.endsWith("/") || path.endsWith("\\");

    InplaceArray<StringView, 20> finalParts;
    for (const auto& part : parts)
    {
        if (part == ".")
            continue;

        if (part == "..")
        {
            if (finalParts.empty())
                return false;
            finalParts.popBack();
            continue;
        }

        finalParts.pushBack(part);
    }

    StringBuilder txt;

    if (isDepotPath)
        txt.appendch(SYSTEM_PATH_SEPARATOR);

    bool firstPart = true;
    for (const auto& part : finalParts)
    {
        if (!firstPart)
            txt.appendch(SYSTEM_PATH_SEPARATOR);

        txt << part;
        firstPart = false;
    }

    if (isDirPath)
        txt.appendch(SYSTEM_PATH_SEPARATOR);

    outPath = StringBuf(txt);
    return true;
}

//--

bool ScanRelativePaths(StringView contextPath, StringView pathPartsStr, uint32_t maxScanDepth, StringBuf& outPath, const std::function<bool(StringView)>& testFunc)
{
    // slice the path parts that are given, we don't assume much about their structure
    InplaceArray<StringView, 20> pathParts;
    pathPartsStr.slice("\\/", pathParts);
    if (pathParts.empty())
        return false; // nothing was given

    // slice the context path
    InplaceArray<StringView, 20> contextParts;
    contextPath.slice("/", contextParts);

    // remove the file name of the reference pat
    if (!contextPath.endsWith("/") && !contextParts.empty())
        contextParts.popBack();

    // outer search (on the reference path)
    for (uint32_t i = 0; i < maxScanDepth; ++i)
    {
        // try all allowed combinations of reference path as well
        auto innerSearchDepth = std::min<uint32_t>(maxScanDepth, pathParts.size());
        for (uint32_t j = 0; j < innerSearchDepth; ++j)
        {
            StringBuilder pathBuilder;

            // build base part of the path
            auto printSeparator = contextPath.beginsWith("/");
            for (auto& str : contextParts)
            {
                if (printSeparator) pathBuilder << "/";
                pathBuilder << str;
                printSeparator = true;
            }

            // add rest of the parts
            auto firstInputPart = pathParts.size() - j - 1;
            for (uint32_t k = firstInputPart; k < pathParts.size(); ++k)
            {
                if (printSeparator) pathBuilder << "/";
                pathBuilder << pathParts[k];
                printSeparator = true;
            }

            // does the file exist ?
            if (testFunc(pathBuilder.view()))
            {
                outPath = StringBuf(pathBuilder);
                return true;
            }
        }

        // ok, we didn't found anything, retry with less base directories
        if (contextParts.empty())
            break;
        contextParts.popBack();
    }

    // no matching file found
    return false;
}

//--

StringBuf ReplaceExtension(StringView path, StringView newExtension)
{
    if (newExtension)
        return TempString("{}.{}", path.pathStripExtensions(), newExtension);
    else
        return StringBuf(path.pathStripExtensions());
}

//--

void ConformPathSeparators(StringBuf& path)
{
    path = path.replaceChar(WRONG_SYSTEM_PATH_SEPARATOR, SYSTEM_PATH_SEPARATOR);
}


namespace prv
{
#if defined(PLATFORM_WINDOWS)
    static const char SYSTEM_PATH_SEPARATOR = '\\';
    static const char WRONG_SYSTEM_PATH_SEPARATOR = '/';
#else
    static const char SYSTEM_PATH_SEPARATOR = '/';
    static const char WRONG_SYSTEM_PATH_SEPARATOR = '\\';
#endif
}

//--

END_INFERNO_NAMESPACE()

