/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "bm/core/containers/include/stringView.h"

BEGIN_INFERNO_NAMESPACE()

//--

TEST(StrigView, FileName)
{
	const StringView path("/home/rexdex/textures/lena.png");
	const StringView test = path.pathFileName();
	EXPECT_EQ("lena.png", test);
}

TEST(StrigView, FileNameEmptyInDirPath)
{
	const StringView path("/home/rexdex/textures/");
	const StringView test = path.pathFileName();
	EXPECT_EQ("", test);
}

TEST(StrigView, FileStem)
{
	const StringView path("/home/rexdex/textures/lena.png");
	const StringView test = path.pathFileStem();
	EXPECT_EQ("lena", test);
}

TEST(StrigView, FileStemEmptyInDirPath)
{
	const StringView path("/home/rexdex/textures/");
	const StringView test = path.pathFileStem();
	EXPECT_EQ("", test);
}

TEST(StrigView, FileSingleExtension)
{
	const StringView path("/home/rexdex/textures/lena.png");
	const StringView test = path.pathFileExtensions();
	EXPECT_EQ("png", test);
}

TEST(StrigView, FileExtensions)
{
	const StringView path("/home/rexdex/textures/lena.png.bak");
	const StringView test = path.pathFileExtensions();
	EXPECT_EQ("png.bak", test);
}

TEST(StrigView, FileExtensionsEmptyFileName)
{
	const StringView path("/home/rexdex/textures/");
	const StringView test = path.pathFileExtensions();
	EXPECT_EQ("", test);
}

TEST(StrigView, FileExtensionsEmptyFileNameEvenIfFileNameHasDot)
{
	const StringView path("/home/rexdex/textures/.test");
	const StringView test = path.pathFileExtensions();
	EXPECT_EQ("", test);
}

TEST(StrigView, FileLastExtension)
{
	const StringView path("/home/rexdex/textures/lena.png");
	const StringView test = path.pathFileLastExtension();
	EXPECT_EQ("png", test);
}

TEST(StrigView, FileLastExtensionFromMultipleExtensions)
{
	const StringView path("/home/rexdex/textures/lena.png.bak");
	const StringView test = path.pathFileLastExtension();
	EXPECT_EQ("bak", test);
}

TEST(StrigView, DirectoryNameInPath)
{
	const StringView path("/home/rexdex/textures/lena.png.bak");
	const StringView test = path.pathDirectoryName();
	EXPECT_EQ("textures", test);
}

TEST(StrigView, DirectoryNameInNoFileName)
{
	const StringView path("/home/rexdex/textures/");
	const StringView test = path.pathDirectoryName();
	EXPECT_EQ("textures", test);
}

TEST(StrigView, DirectoryNameInRootPath)
{
	const StringView path("");
	const StringView test = path.pathDirectoryName();
	EXPECT_EQ("", test);
}

TEST(StrigView, DirectoryNameInEmptyPath)
{
	const StringView path("");
	const StringView test = path.pathDirectoryName();
	EXPECT_EQ("", test);
}

TEST(StrigView, FileLastExtensionEmptyFileName)
{
	const StringView path("/home/rexdex/textures/");
	const StringView test = path.pathFileLastExtension();
	EXPECT_EQ("", test);
}

TEST(StrigView, FileParentPathFromFileReturnsParentDir)
{
	const StringView path("/home/rexdex/textures/lena.txt");
	const StringView test = path.pathParent();
	EXPECT_EQ("/home/rexdex/textures/", test);
}

TEST(StrigView, FileParentPathFromDirReturnsDir)
{
	const StringView path("/home/rexdex/textures/");
	const StringView test = path.pathParent();
	EXPECT_EQ("/home/rexdex/", test);
}

TEST(StrigView, FileParentInRootReturnsEmptyString)
{
	const StringView path("/");
	const StringView test = path.pathParent();
	EXPECT_EQ("", test);
}

TEST(StrigView, FileParentOfEmptyStringIsEmptyString)
{
	const StringView path("");
	const StringView test = path.pathParent();
	EXPECT_EQ("", test);
}

TEST(StrigView, StripExtensionSingle)
{
	const StringView path("/home/rexdex/textures/lena.txt");
	const StringView test = path.pathStripExtensions();
	EXPECT_EQ("/home/rexdex/textures/lena", test);
}

TEST(StrigView, StripExtensionSingleMultiple)
{
	const StringView path("/home/rexdex/textures/lena.txt.bak");
	const StringView test = path.pathStripExtensions();
	EXPECT_EQ("/home/rexdex/textures/lena", test);
}

TEST(StrigView, StripExtensionSingleNone)
{
	const StringView path("/home/rexdex/textures/lena");
	const StringView test = path.pathStripExtensions();
	EXPECT_EQ("/home/rexdex/textures/lena", test);
}

TEST(StrigView, StripExtensionNotFile)
{
	const StringView path("/home/rexdex/textures/");
	const StringView test = path.pathStripExtensions();
	EXPECT_EQ("/home/rexdex/textures/", test);
}

//--

END_INFERNO_NAMESPACE()
