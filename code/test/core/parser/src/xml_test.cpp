/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"

#include "bm/core/parser/include/xmlReader.h"
#include "bm/core/parser/include/xmlWriter.h"

BEGIN_INFERNO_NAMESPACE()

const char* xmlSample =
"<?xml version=\"1.0\" standalone=\"yes\"?>"
"<doc>"
"<node x=\"a\"  y=\"b\">"
"<test/>"
"</node>"
"<node x=\"a\"  y=\"b\">"
"<test/>"
"</node>"
"</doc>";

TEST(XML, LoadFromText)
{
    auto doc  = XMLReader::LoadFromText(ITextErrorReporter::GetDefault(), "test.xml", xmlSample);
    ASSERT_TRUE(!!doc);

    auto root  = doc->root();
    ASSERT_EQ(StringBuf("doc"), doc->nodeName(root));

    auto child  = doc->firstChild(root);
    ASSERT_TRUE(child != 0);

    ASSERT_EQ(StringBuf("node"), doc->nodeName(child));

    ASSERT_EQ(doc->attributeValue(child, "x"), "a");
    ASSERT_EQ(doc->attributeValue(child, "y"), "b");

    //--

    child = doc->nextChild(child);
    ASSERT_EQ(StringBuf("node"), doc->nodeName(child));

    ASSERT_EQ(doc->attributeValue(child, "x"), "a");
    ASSERT_EQ(doc->attributeValue(child, "y"), "b");

    //---


    child = doc->nextChild(child);
    ASSERT_TRUE(child == 0);
}

TEST(XML, CreateManual)
{
    auto doc  = XMLWriter::Create("doc");

    {
        auto id  = doc->createNode(doc->root(), "node");
        ASSERT_TRUE(id != 0);

        doc->createAttribute(id, "x", "a");
        doc->createAttribute(id, "y", "b");
    }

    {
        auto id  = doc->createNode(doc->root(), "node");
        ASSERT_TRUE(id != 0);

        doc->createAttribute(id, "x", "a");
        doc->createAttribute(id, "y", "b");
    }

    ASSERT_TRUE(!!doc);

    auto root  = doc->root();
    ASSERT_EQ(StringBuf("doc"), doc->nodeName(root));

    //---

    auto child  = doc->firstChild(root);
    ASSERT_TRUE(child != 0);

    ASSERT_EQ(doc->attributeValue(child, "x"), "a");
    ASSERT_EQ(doc->attributeValue(child, "y"), "b");

    ASSERT_EQ(StringBuf("node"), doc->nodeName(child));

    //---

    child = doc->nextChild(child);
    ASSERT_EQ(StringBuf("node"), doc->nodeName(child));

    ASSERT_EQ(doc->attributeValue(child, "x"), "a");
    ASSERT_EQ(doc->attributeValue(child, "y"), "b");

    //---

    child = doc->nextChild(child);
    ASSERT_TRUE(child == 0);

    //---
}

TEST(XML, Reparse)
{
    auto docA = XMLReader::LoadFromText(ITextErrorReporter::GetDefault(), "test.xml", xmlSample);
    ASSERT_TRUE(!!docA);

    StringBuilder txtA;
    XMLReader::SaveToTextFile(docA, docA->root(), txtA);

    auto docB  = XMLReader::LoadFromText(ITextErrorReporter::GetDefault(), "test.xml", txtA.view());
    ASSERT_TRUE(!!docB);

    StringBuilder txtB;
	XMLReader::SaveToTextFile(docB, docB->root(), txtB);

    ASSERT_EQ(std::string(txtA.c_str()), std::string(txtB.c_str()));
}

END_INFERNO_NAMESPACE()