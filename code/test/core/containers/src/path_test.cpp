/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "bm/core/containers/include/path.h"

/*
namespace testing
{
    template<>
    INLINE ::std::string PrintToString<bm::StringBuf>(const bm::StringBuf& value)
    {
        return value.c_str();
    }

    template<>
    INLINE ::std::string PrintToString<bm::StringView>(const bm::StringView& value)
    {
        return std::string(value.data(), value.length());
    }

    template<>
    INLINE ::std::string PrintToString<bm::StringID>(const bm::StringID& value)
    {
        return std::string(value.c_str());
    }

} // testing*/

BEGIN_INFERNO_NAMESPACE()

//--

TEST(DepotFileName, EmptyNotValid)
{
	EXPECT_FALSE(ValidateFileName(""));
}

TEST(DepotFileName, SimpleValid)
{
	EXPECT_TRUE(ValidateFileName("test"));
}

TEST(DepotFileName, SpaceWithSpace)
{
	EXPECT_TRUE(ValidateFileName("test and crap"));
}

TEST(DepotFileName, StartingWithSpaceNotValid)
{
	EXPECT_FALSE(ValidateFileName(" test and crap"));
}

TEST(DepotFileName, EndingWithSpaceNotValid)
{
	EXPECT_FALSE(ValidateFileName("test and crap "));
}

TEST(DepotFileName, UTFValid)
{
	EXPECT_TRUE(ValidateFileName(u8"Слава Україні"));
}

TEST(DepotFileName, StartingFromDotValid)
{
	EXPECT_TRUE(ValidateFileName(".test"));
}

TEST(DepotFileName, SpecialCharasNotValid)
{
	EXPECT_FALSE(ValidateFileName("te.st"));
	EXPECT_FALSE(ValidateFileName("te\nst"));
	EXPECT_FALSE(ValidateFileName("te\rst"));
	EXPECT_FALSE(ValidateFileName("te<st"));
	EXPECT_FALSE(ValidateFileName("te>st"));
	EXPECT_FALSE(ValidateFileName("te:st"));
	EXPECT_FALSE(ValidateFileName("te\"st"));
	EXPECT_FALSE(ValidateFileName("te/st"));
	EXPECT_FALSE(ValidateFileName("te\\st"));
	EXPECT_FALSE(ValidateFileName("te|st"));
	EXPECT_FALSE(ValidateFileName("te?st"));
	EXPECT_FALSE(ValidateFileName("te*st"));
}

TEST(DepotFileName, ReservedNamesNotValid)
{
	EXPECT_FALSE(ValidateFileName("CON"));
	EXPECT_FALSE(ValidateFileName("PRN"));
	EXPECT_FALSE(ValidateFileName("AUX"));
	EXPECT_FALSE(ValidateFileName("NU"));
	EXPECT_FALSE(ValidateFileName("COM1"));
	EXPECT_FALSE(ValidateFileName("COM2"));
	EXPECT_FALSE(ValidateFileName("COM3"));
	EXPECT_FALSE(ValidateFileName("COM4"));
	EXPECT_FALSE(ValidateFileName("COM5"));
	EXPECT_FALSE(ValidateFileName("COM6"));
	EXPECT_FALSE(ValidateFileName("COM7"));
	EXPECT_FALSE(ValidateFileName("COM8"));
	EXPECT_FALSE(ValidateFileName("COM9"));
	EXPECT_FALSE(ValidateFileName("LPT1"));
	EXPECT_FALSE(ValidateFileName("LPT2"));
	EXPECT_FALSE(ValidateFileName("LPT3"));
	EXPECT_FALSE(ValidateFileName("LPT4"));
	EXPECT_FALSE(ValidateFileName("LPT5"));
	EXPECT_FALSE(ValidateFileName("LPT6"));
	EXPECT_FALSE(ValidateFileName("LPT7"));
	EXPECT_FALSE(ValidateFileName("LPT8"));
	EXPECT_FALSE(ValidateFileName("LPT9"));
}

TEST(DepotFileName, ReservedNamesNotValidLowerCase)
{
	EXPECT_FALSE(ValidateFileName("con"));
	EXPECT_FALSE(ValidateFileName("prn"));
	EXPECT_FALSE(ValidateFileName("aux"));
	EXPECT_FALSE(ValidateFileName("nu"));
	EXPECT_FALSE(ValidateFileName("com1"));
	EXPECT_FALSE(ValidateFileName("com2"));
	EXPECT_FALSE(ValidateFileName("com3"));
	EXPECT_FALSE(ValidateFileName("com4"));
	EXPECT_FALSE(ValidateFileName("com5"));
	EXPECT_FALSE(ValidateFileName("com6"));
	EXPECT_FALSE(ValidateFileName("com7"));
	EXPECT_FALSE(ValidateFileName("com8"));
	EXPECT_FALSE(ValidateFileName("com9"));
	EXPECT_FALSE(ValidateFileName("lpt1"));
	EXPECT_FALSE(ValidateFileName("lpt2"));
	EXPECT_FALSE(ValidateFileName("lpt3"));
	EXPECT_FALSE(ValidateFileName("lpt4"));
	EXPECT_FALSE(ValidateFileName("lpt5"));
	EXPECT_FALSE(ValidateFileName("lpt6"));
	EXPECT_FALSE(ValidateFileName("lpt7"));
	EXPECT_FALSE(ValidateFileName("lpt8"));
	EXPECT_FALSE(ValidateFileName("lpt9"));
}

//--

TEST(DepotFileNameWithExt, EmptyNotValid)
{
	EXPECT_FALSE(ValidateFileNameWithExtension(""));
}

TEST(DepotFileNameWithExt, MissingExtensionNotValid)
{
	EXPECT_FALSE(ValidateFileNameWithExtension("test"));
}

TEST(DepotFileNameWithExt, WithSpaceMissingExtensionNotValid)
{
	EXPECT_FALSE(ValidateFileNameWithExtension("test and crap"));
}

TEST(DepotFileNameWithExt, StartingWithSpaceNotValid)
{
	EXPECT_FALSE(ValidateFileNameWithExtension(" test and crap"));
}

TEST(DepotFileNameWithExt, EndingWithSpaceNotValid)
{
	EXPECT_FALSE(ValidateFileNameWithExtension("test and crap "));
}

TEST(DepotFileNameWithExt, UTFMissingExtensionNotValid)
{
	EXPECT_FALSE(ValidateFileNameWithExtension(u8"Слава Україні"));
}

TEST(DepotFileNameWithExt, StartingFromDotNotValid)
{
	EXPECT_FALSE(ValidateFileNameWithExtension(".test"));
}

TEST(DepotFileNameWithExt, SimpleWithExtValid)
{
	EXPECT_TRUE(ValidateFileNameWithExtension("test.txt"));
}

TEST(DepotFileNameWithExt, SimpleWithMultiExtValid)
{
	EXPECT_TRUE(ValidateFileNameWithExtension("test.txt.bak"));
}

TEST(DepotFileNameWithExt, SimpleWithEmptyExtNoValid)
{
	EXPECT_FALSE(ValidateFileNameWithExtension("test."));
}

TEST(DepotFileNameWithExt, SimpleWithMissingExtNotValid)
{
	EXPECT_FALSE(ValidateFileNameWithExtension("test.."));
}

TEST(DepotFileNameWithExt, SimpleWithCollapsedExtNotValid)
{
	EXPECT_FALSE(ValidateFileNameWithExtension("test..txt"));
}

TEST(DepotFileNameWithExt, SimpleWithMissingTailingExtNotValid)
{
	EXPECT_FALSE(ValidateFileNameWithExtension("test.txt."));
}

TEST(DepotFileNameWithExt, SpecialCharasNotValid)
{
	EXPECT_FALSE(ValidateFileNameWithExtension("te\nst"));
	EXPECT_FALSE(ValidateFileNameWithExtension("te\rst"));
	EXPECT_FALSE(ValidateFileNameWithExtension("te<st"));
	EXPECT_FALSE(ValidateFileNameWithExtension("te>st"));
	EXPECT_FALSE(ValidateFileNameWithExtension("te:st"));
	EXPECT_FALSE(ValidateFileNameWithExtension("te\"st"));
	EXPECT_FALSE(ValidateFileNameWithExtension("te/st"));
	EXPECT_FALSE(ValidateFileNameWithExtension("te\\st"));
	EXPECT_FALSE(ValidateFileNameWithExtension("te|st"));
	EXPECT_FALSE(ValidateFileNameWithExtension("te?st"));
	EXPECT_FALSE(ValidateFileNameWithExtension("te*st"));
}

TEST(DepotFileNameWithExt, ReservedNamesNotValid)
{
	EXPECT_FALSE(ValidateFileNameWithExtension("CON"));
	EXPECT_FALSE(ValidateFileNameWithExtension("PRN"));
	EXPECT_FALSE(ValidateFileNameWithExtension("AUX"));
	EXPECT_FALSE(ValidateFileNameWithExtension("NU"));
	EXPECT_FALSE(ValidateFileNameWithExtension("COM1"));
	EXPECT_FALSE(ValidateFileNameWithExtension("COM2"));
	EXPECT_FALSE(ValidateFileNameWithExtension("COM3"));
	EXPECT_FALSE(ValidateFileNameWithExtension("COM4"));
	EXPECT_FALSE(ValidateFileNameWithExtension("COM5"));
	EXPECT_FALSE(ValidateFileNameWithExtension("COM6"));
	EXPECT_FALSE(ValidateFileNameWithExtension("COM7"));
	EXPECT_FALSE(ValidateFileNameWithExtension("COM8"));
	EXPECT_FALSE(ValidateFileNameWithExtension("COM9"));
	EXPECT_FALSE(ValidateFileNameWithExtension("LPT1"));
	EXPECT_FALSE(ValidateFileNameWithExtension("LPT2"));
	EXPECT_FALSE(ValidateFileNameWithExtension("LPT3"));
	EXPECT_FALSE(ValidateFileNameWithExtension("LPT4"));
	EXPECT_FALSE(ValidateFileNameWithExtension("LPT5"));
	EXPECT_FALSE(ValidateFileNameWithExtension("LPT6"));
	EXPECT_FALSE(ValidateFileNameWithExtension("LPT7"));
	EXPECT_FALSE(ValidateFileNameWithExtension("LPT8"));
	EXPECT_FALSE(ValidateFileNameWithExtension("LPT9"));
}

TEST(DepotFileNameWithExt, ReservedNamesNotValidLowerCase)
{
	EXPECT_FALSE(ValidateFileNameWithExtension("con"));
	EXPECT_FALSE(ValidateFileNameWithExtension("prn"));
	EXPECT_FALSE(ValidateFileNameWithExtension("aux"));
	EXPECT_FALSE(ValidateFileNameWithExtension("nu"));
	EXPECT_FALSE(ValidateFileNameWithExtension("com1"));
	EXPECT_FALSE(ValidateFileNameWithExtension("com2"));
	EXPECT_FALSE(ValidateFileNameWithExtension("com3"));
	EXPECT_FALSE(ValidateFileNameWithExtension("com4"));
	EXPECT_FALSE(ValidateFileNameWithExtension("com5"));
	EXPECT_FALSE(ValidateFileNameWithExtension("com6"));
	EXPECT_FALSE(ValidateFileNameWithExtension("com7"));
	EXPECT_FALSE(ValidateFileNameWithExtension("com8"));
	EXPECT_FALSE(ValidateFileNameWithExtension("com9"));
	EXPECT_FALSE(ValidateFileNameWithExtension("lpt1"));
	EXPECT_FALSE(ValidateFileNameWithExtension("lpt2"));
	EXPECT_FALSE(ValidateFileNameWithExtension("lpt3"));
	EXPECT_FALSE(ValidateFileNameWithExtension("lpt4"));
	EXPECT_FALSE(ValidateFileNameWithExtension("lpt5"));
	EXPECT_FALSE(ValidateFileNameWithExtension("lpt6"));
	EXPECT_FALSE(ValidateFileNameWithExtension("lpt7"));
	EXPECT_FALSE(ValidateFileNameWithExtension("lpt8"));
	EXPECT_FALSE(ValidateFileNameWithExtension("lpt9"));
}

TEST(DepotFileNameWithExt, ReservedNamesNotValidEvenWithExt)
{
	EXPECT_FALSE(ValidateFileNameWithExtension("CON.txt"));
	EXPECT_FALSE(ValidateFileNameWithExtension("PRN.txt"));
	EXPECT_FALSE(ValidateFileNameWithExtension("AUX.txt"));
	EXPECT_FALSE(ValidateFileNameWithExtension("NU.txt"));
	EXPECT_FALSE(ValidateFileNameWithExtension("COM1.txt"));
	EXPECT_FALSE(ValidateFileNameWithExtension("COM2.txt"));
	EXPECT_FALSE(ValidateFileNameWithExtension("COM3.txt"));
	EXPECT_FALSE(ValidateFileNameWithExtension("COM4.txt"));
	EXPECT_FALSE(ValidateFileNameWithExtension("COM5.txt"));
	EXPECT_FALSE(ValidateFileNameWithExtension("COM6.txt"));
	EXPECT_FALSE(ValidateFileNameWithExtension("COM7.txt"));
	EXPECT_FALSE(ValidateFileNameWithExtension("COM8.txt"));
	EXPECT_FALSE(ValidateFileNameWithExtension("COM9.txt"));
	EXPECT_FALSE(ValidateFileNameWithExtension("LPT1.txt"));
	EXPECT_FALSE(ValidateFileNameWithExtension("LPT2.txt"));
	EXPECT_FALSE(ValidateFileNameWithExtension("LPT3.txt"));
	EXPECT_FALSE(ValidateFileNameWithExtension("LPT4.txt"));
	EXPECT_FALSE(ValidateFileNameWithExtension("LPT5.txt"));
	EXPECT_FALSE(ValidateFileNameWithExtension("LPT6.txt"));
	EXPECT_FALSE(ValidateFileNameWithExtension("LPT7.txt"));
	EXPECT_FALSE(ValidateFileNameWithExtension("LPT8.txt"));
	EXPECT_FALSE(ValidateFileNameWithExtension("LPT9.txt"));
}

TEST(DepotFileNameWithExt, ReservedNamesValidAsExtension)
{
	EXPECT_TRUE(ValidateFileNameWithExtension("text.CON"));
	EXPECT_TRUE(ValidateFileNameWithExtension("text.PRN"));
	EXPECT_TRUE(ValidateFileNameWithExtension("text.AUX"));
	EXPECT_TRUE(ValidateFileNameWithExtension("text.NU"));
	EXPECT_TRUE(ValidateFileNameWithExtension("text.COM1"));
	EXPECT_TRUE(ValidateFileNameWithExtension("text.COM2"));
	EXPECT_TRUE(ValidateFileNameWithExtension("text.COM3"));
	EXPECT_TRUE(ValidateFileNameWithExtension("text.COM4"));
	EXPECT_TRUE(ValidateFileNameWithExtension("text.COM5"));
	EXPECT_TRUE(ValidateFileNameWithExtension("text.COM6"));
	EXPECT_TRUE(ValidateFileNameWithExtension("text.COM7"));
	EXPECT_TRUE(ValidateFileNameWithExtension("text.COM8"));
	EXPECT_TRUE(ValidateFileNameWithExtension("text.COM9"));
	EXPECT_TRUE(ValidateFileNameWithExtension("text.LPT1"));
	EXPECT_TRUE(ValidateFileNameWithExtension("text.LPT2"));
	EXPECT_TRUE(ValidateFileNameWithExtension("text.LPT3"));
	EXPECT_TRUE(ValidateFileNameWithExtension("text.LPT4"));
	EXPECT_TRUE(ValidateFileNameWithExtension("text.LPT5"));
	EXPECT_TRUE(ValidateFileNameWithExtension("text.LPT6"));
	EXPECT_TRUE(ValidateFileNameWithExtension("text.LPT7"));
	EXPECT_TRUE(ValidateFileNameWithExtension("text.LPT8"));
	EXPECT_TRUE(ValidateFileNameWithExtension("text.LPT9"));
}

TEST(DepotFileNameWithExt, ExtForUTFValid)
{
	EXPECT_TRUE(ValidateFileNameWithExtension(u8"Слава Україні.txt"));
}

TEST(DepotFileNameWithExt, UTFExtValid)
{
	EXPECT_TRUE(ValidateFileNameWithExtension(u8"Слава Україні.Слава"));
}

//--

TEST(DepotPath, EmptyStringDoesNotValidateAsPath)
{
    EXPECT_FALSE(ValidateDepotFilePath(""));
}

TEST(DepotPath, RootPathDoesNotValidate)
{
	EXPECT_FALSE(ValidateDepotFilePath("/"));
}

TEST(DepotPath, FileWithNoExtensionValidates)
{
    EXPECT_FALSE(ValidateDepotFilePath("/test"));
}

TEST(DepotPath, FileWithNoExtensionValidatesWithSpace)
{
	EXPECT_FALSE(ValidateDepotFilePath("/test and crap"));
}

TEST(DepotPath, FileWithExtensionValidates)
{
    EXPECT_TRUE(ValidateDepotFilePath("/test.txt"));
}

TEST(DepotPath, FileWithExtensionValidatesWithSpaces)
{
	EXPECT_TRUE(ValidateDepotFilePath("/test and crap.txt"));
}

TEST(DepotPath, FileWithMultipleExtensionValidates)
{
	EXPECT_TRUE(ValidateDepotFilePath("/test.smesh.lod0"));
}

/*TEST(DepotPath, FileWithExtensionDoesNotValidateIfExtensionDisabled)
{
	EXPECT_FALSE(ValidateDepotFilePath("/test.txt", false));
}*/

TEST(DepotPath, FileWithExtensionDoesNotValidateWithStartSpace)
{
	EXPECT_FALSE(ValidateDepotFilePath("/test. txt"));
}

TEST(DepotPath, FileWithExtensionDoesNotValidateWithEndSpace)
{
	EXPECT_FALSE(ValidateDepotFilePath("/test.txt "));
}

TEST(DepotPath, FileWithExtensionDoesNotValidateWithMidSpace)
{
	EXPECT_FALSE(ValidateDepotFilePath("/test.txt txt"));
}

/*TEST(DepotPath, FileWithExtensionsDoesNotValidateIfExtensionDisabled)
{
	EXPECT_FALSE(ValidateDepotFilePath("/test.smesh.lod0", false));
}

TEST(DepotPath, FileWithNoExtensionsStillValidatesIfExtensionDisabled)
{
	EXPECT_TRUE(ValidateDepotFilePath("/test", false));
}

TEST(DepotPath, FileWithNoExtensionsStillValidatesIfExtensionDisabled)
{
	EXPECT_TRUE(ValidateDepotFilePath("/test", false));
}*/

TEST(DepotPath, FileInDirectoryValidates)
{
	EXPECT_TRUE(ValidateDepotFilePath("/crap/test.txt"));
}

TEST(DepotPath, FileInDirectoryWithSpacesValidates)
{
	EXPECT_TRUE(ValidateDepotFilePath("/crap and other crap/test.txt"));
}

TEST(DepotPath, FileWithspacesInDirectoryWithSpacesValidates)
{
	EXPECT_TRUE(ValidateDepotFilePath("/crap and other crap/test with crap.txt"));
}

TEST(DepotPath, FileWithCollapsedDirsDoesNotValidate)
{
	EXPECT_FALSE(ValidateDepotFilePath("/crap//test.txt"));
}

TEST(DepotPath, FileUTFNameValidates)
{
	EXPECT_FALSE(ValidateDepotDirPath(u8"/test/Слава Україні.txt"));
}

//--

TEST(DepotDirPath, EmptyStringDoesNotValidate)
{
	EXPECT_FALSE(ValidateDepotDirPath(""));
}

TEST(DepotDirPath, RootPathValidates)
{
	EXPECT_TRUE(ValidateDepotDirPath("/"));
}

TEST(DepotDirPath, SingleDirValidates)
{
	EXPECT_TRUE(ValidateDepotDirPath("/test/"));
}

TEST(DepotDirPath, FileNameDoestNotValidate)
{
	EXPECT_FALSE(ValidateDepotDirPath("/test.txt"));
}

TEST(DepotDirPath, SingleDirValidatesWithSpace)
{
	EXPECT_TRUE(ValidateDepotDirPath("/test crap/"));
}

TEST(DepotDirPath, MultiDirValidatesWithSpace)
{
	EXPECT_TRUE(ValidateDepotDirPath("/test crap/other crap/"));
}

TEST(DepotDirPath, UTFDirValidates)
{
	EXPECT_TRUE(ValidateDepotDirPath(u8"/zażółć gęsią/Слава Україні/"));
}

//--

TEST(ConformDepotFilePath, EmptyStaysEmpty)
{
	auto path = ConformDepotFilePath("");
	EXPECT_STREQ("", path.c_str());
}

TEST(ConformDepotFilePath, AutoAddStartingSlash)
{
	auto path = ConformDepotFilePath("test.txt");
	EXPECT_STREQ("/test.txt", path.c_str());
}

TEST(ConformDepotFilePath, DirectoryPathNotValid)
{
	auto path = ConformDepotFilePath("/test/");
	EXPECT_STREQ("", path.c_str());
}

TEST(ConformDepotFilePath, ConformedSeparators)
{
	auto path = ConformDepotFilePath("\\test\\crap\\file.txt");
	EXPECT_STREQ("/test/crap/file.txt", path.c_str());
}

TEST(ConformDepotFilePath, NormalizedEmpty)
{
	auto path = ConformDepotFilePath("/test/crap//file.txt");
	EXPECT_STREQ("/test/crap/file.txt", path.c_str());
}

TEST(ConformDepotFilePath, NormalizedDot)
{
	auto path = ConformDepotFilePath("/test/crap/./file.txt");
	EXPECT_STREQ("/test/crap/file.txt", path.c_str());
}

TEST(ConformDepotFilePath, NormalizedDoubleDot)
{
	auto path = ConformDepotFilePath("/test/music/../crap/file.txt");
	EXPECT_STREQ("/test/crap/file.txt", path.c_str());
}

TEST(ConformDepotFilePath, NormalizedUpToRoot)
{
	auto path = ConformDepotFilePath("/test/crap/../../file.txt");
	EXPECT_STREQ("/file.txt", path.c_str());
}

TEST(ConformDepotFilePath, NormalizedFails)
{
	auto path = ConformDepotFilePath("/test/music/../../../file.txt");
	EXPECT_STREQ("", path.c_str());
}

//--

TEST(ConformDepotDirectoryPath, EmptyChangesIntoRoot)
{
	auto path = ConformDepotDirectoryPath("");
	EXPECT_STREQ("/", path.c_str());
}

TEST(ConformDepotDirectoryPath, FilePathInvalid)
{
	auto path = ConformDepotDirectoryPath("test.txt");
	EXPECT_STREQ("", path.c_str());
}

TEST(ConformDepotDirectoryPath, AutoAddRootSeparator)
{
	auto path = ConformDepotDirectoryPath("test/test/");
	EXPECT_STREQ("/test/test/", path.c_str());
}

TEST(ConformDepotDirectoryPath, ConformedSeparators)
{
	auto path = ConformDepotDirectoryPath("\\test\\crap\\");
	EXPECT_STREQ("/test/crap/", path.c_str());
}

TEST(ConformDepotDirectoryPath, NormalizedEmpty)
{
	auto path = ConformDepotDirectoryPath("/test/crap//files/");
	EXPECT_STREQ("/test/crap/files/", path.c_str());
}

TEST(ConformDepotDirectoryPath, NormalizedDot)
{
	auto path = ConformDepotDirectoryPath("/test/crap/./files/");
	EXPECT_STREQ("/test/crap/files/", path.c_str());
}

TEST(ConformDepotDirectoryPath, NormalizedDoubleDot)
{
	auto path = ConformDepotDirectoryPath("/test/music/../crap/files/");
	EXPECT_STREQ("/test/crap/files/", path.c_str());
}

TEST(ConformDepotDirectoryPath, NormalizedUpToRoot)
{
	auto path = ConformDepotDirectoryPath("/test/music/../../crap/files/");
	EXPECT_STREQ("/crap/files/", path.c_str());
}

TEST(ConformDepotDirectoryPath, NormalizedFails)
{
	auto path = ConformDepotDirectoryPath("/test/music/../../../crap/files/");
	EXPECT_STREQ("", path.c_str());
}

//--

TEST(NormalizePath, NormalizeSimpleFile)
{
	StringBuf path;
	EXPECT_TRUE(NormalizePath("/test/music/test.txt", path));

#ifdef PLATFORM_WINDOWS
	EXPECT_STREQ("\\test\\music\\test.txt", path.c_str());
#else
	EXPECT_STREQ("/test/music/test.txt", path.c_str());
#endif
}

TEST(NormalizePath, NormalizeEmptyFile)
{
	StringBuf path;
	EXPECT_TRUE(NormalizePath("/test/music//test.txt", path));

#ifdef PLATFORM_WINDOWS
	EXPECT_STREQ("\\test\\music\\test.txt", path.c_str());
#else
	EXPECT_STREQ("/test/music/test.txt", path.c_str());
#endif
}

TEST(NormalizePath, NormalizeFileDot)
{
	StringBuf path;
	EXPECT_TRUE(NormalizePath("/test/music/./test.txt", path));

#ifdef PLATFORM_WINDOWS
	EXPECT_STREQ("\\test\\music\\test.txt", path.c_str());
#else
	EXPECT_STREQ("/test/music/test.txt", path.c_str());
#endif
}

TEST(NormalizePath, NormalizeFileDoubleDot)
{
	StringBuf path;
	EXPECT_TRUE(NormalizePath("/test/crap/../music/test.txt", path));

#ifdef PLATFORM_WINDOWS
	EXPECT_STREQ("\\test\\music\\test.txt", path.c_str());
#else
	EXPECT_STREQ("/test/music/test.txt", path.c_str());
#endif
}

TEST(NormalizePath, NormalizeSimpleDir)
{
	StringBuf path;
	EXPECT_TRUE(NormalizePath("/test/music/test/", path));

#ifdef PLATFORM_WINDOWS
	EXPECT_STREQ("\\test\\music\\test\\", path.c_str());
#else
	EXPECT_STREQ("/test/music/test/", path.c_str());
#endif
}

TEST(NormalizePath, NormalizeEmptyDir)
{
	StringBuf path;
	EXPECT_TRUE(NormalizePath("/test/music//test/", path));

#ifdef PLATFORM_WINDOWS
	EXPECT_STREQ("\\test\\music\\test\\", path.c_str());
#else
	EXPECT_STREQ("/test/music/test/", path.c_str());
#endif
}

TEST(NormalizePath, NormalizeDirDot)
{
	StringBuf path;
	EXPECT_TRUE(NormalizePath("/test/music/./test/", path));

#ifdef PLATFORM_WINDOWS
	EXPECT_STREQ("\\test\\music\\test\\", path.c_str());
#else
	EXPECT_STREQ("/test/music/test/", path.c_str());
#endif
}

TEST(NormalizePath, NormalizeDirDoubleDot)
{
	StringBuf path;
	EXPECT_TRUE(NormalizePath("/test/music/../crap/", path));

#ifdef PLATFORM_WINDOWS
	EXPECT_STREQ("\\test\\crap\\", path.c_str());
#else
	EXPECT_STREQ("/test/crap/", path.c_str());
#endif
}

//--

#ifdef PLATFORM_WINDOWS

TEST(NormalizePath, NormalizeDirPathWindows)
{
	StringBuf path;
	EXPECT_TRUE(NormalizePath("C:\\test\\music\\..\\crap\\", path));
	EXPECT_STREQ("C:\\test\\crap\\", path.c_str());
}

TEST(NormalizePath, NormalizeFilePathWindows)
{
	StringBuf path;
	EXPECT_TRUE(NormalizePath("C:\\test\\music\\..\\crap\\test.txt", path));
	EXPECT_STREQ("C:\\test\\crap\\test.txt", path.c_str());
}

TEST(NormalizePath, NormalizePathRootWindows)
{
	StringBuf path;
	EXPECT_TRUE(NormalizePath("C:\\test\\music\\..\\..\\test.txt", path));
	EXPECT_STREQ("C:\\test.txt", path.c_str());
}

TEST(NormalizePath, NormalizePathDriveWindows)
{
	StringBuf path;
	EXPECT_TRUE(NormalizePath("C:\\test\\music\\..\\..\\..\\test.txt", path));
	EXPECT_STREQ("test.txt", path.c_str());
}

TEST(NormalizePath, NormalizePathToMuchWindows)
{
	StringBuf path;
	EXPECT_FALSE(NormalizePath("C:\\test\\music\\..\\..\\..\\..\\test.txt", path));
}

#endif

//--

END_INFERNO_NAMESPACE()