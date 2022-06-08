/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "bm/core/containers/include/stringBuf.h"

BEGIN_INFERNO_NAMESPACE()

//--

// generic read only XML document
class BM_CORE_PARSER_API XMLReader : public IReferencable
{
public:
    XMLReader(IPoolUnmanaged& pool, Buffer data, void* doc, void* root);
    virtual ~XMLReader();

    //----

    // get source file/context
    INLINE const StringBuf& sourceContext() const { return m_sourceContext; }

    //---

    // get the root node (always there)
    NodeHandle root() const;

	// get parent node
	NodeHandle nodeParent(NodeHandle id) const;

	// get line number the node lies at
	uint32_t nodeLine(NodeHandle id) const;

    //---

	// get node name
	StringView nodeName(NodeHandle id) const;

    // get node value as text
    StringView nodeValueText(NodeHandle id) const;

    // decode node's value as base64 buffer
    Buffer nodeValueBuffer(NodeHandle id, IPoolUnmanaged* pool = nullptr) const;

    //---

    // get name of the node attribute
    StringView attributeName(AttributeHandle id) const;

    // get value of the node attribute
    StringView attributeValue(AttributeHandle id) const;

	// get value for node's attribute or return default value
	StringView attributeValue(NodeHandle id, StringView name, StringView defaultVal = StringView()) const;

    //--

    // get first node child
	NodeHandle firstChild(NodeHandle id, StringView childName = nullptr) const;

	// get next node sibling
	NodeHandle nextChild(NodeHandle id, StringView siblingName = nullptr) const;

	// get first attribute
	AttributeHandle firstAttribute(NodeHandle id, StringView name = nullptr) const;

    // get next attribute
    AttributeHandle nextAttribute(AttributeHandle id, StringView name = StringView()) const;

    //--

    // load and parse XML document from file on disk (may be text or binary XML)
    static XMLReaderPtr LoadFromFile(ITextErrorReporter& err, StringView absoluteFilePath, TimeStamp* outTimeStamp = nullptr, IPoolUnmanaged& pool = MainPool());

    // load and parse XML document from an abstract reader (may be text or binary XML)
    static XMLReaderPtr LoadFromFile(ITextErrorReporter& err, StringView contextName, IFileReader* file, IPoolUnmanaged& pool = MainPool());

    // parse XML document from text, the input buffer will be copied
    static XMLReaderPtr LoadFromText(ITextErrorReporter& err, StringView contextName, StringView text, IPoolUnmanaged& pool = MainPool());

    // load and parse XML document from memory buffer disk (may be text or binary XML)
    static XMLReaderPtr LoadFromBuffer(ITextErrorReporter& err, StringView contextName, Buffer mem, IPoolUnmanaged& pool = MainPool());

	//---

	// print content to text buffer
	void printToText(NodeHandle id, IFormatStream& f, PrintFlags flags = PrintFlags()) const;

	// print content to a file
	void printToFile(NodeHandle id, IFileWriter* f, PrintFlags flags = PrintFlags()) const;

    //---
    
    // save text to buffer
    Buffer printToBuffer(NodeHandle id, PrintFlags flags = PrintFlags(), IPoolUnmanaged& pool = MainPool()) const;

	//---

	// save document to a text output stream
	static void SaveToTextFile(const XMLReader* ptr, NodeHandle node, IFormatStream& f, PrintFlags flags = PrintFlags());

	// write document to file in text format
	static void SaveToTextFile(const XMLReader* ptr, NodeHandle node, IFileWriter* writer, PrintFlags flags = PrintFlags());

    //---

private:
    IPoolUnmanaged& m_pool;
	Buffer m_data; // original text data

    void* m_doc = nullptr;; // rapidXML document
	void* m_root = nullptr; // rapidXML root

	Array<uint32_t> m_sourceLineStarts; // starting positions of each line, sorted
    StringBuf m_sourceContext; // source file name or other context, needed mostly for debug

	//--

	static void SaveToTextFileNode(const void* node, IFormatStream& f, uint32_t depth, bool prettyText);
};

//--

END_INFERNO_NAMESPACE()
