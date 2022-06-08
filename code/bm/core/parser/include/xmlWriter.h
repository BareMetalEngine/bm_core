/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "bm/core/memory/include/localAllocator.h"
#include "bm/core/containers/include/hashMap.h"

BEGIN_INFERNO_NAMESPACE()

//--

// Writable XML document, allows to build data
class BM_CORE_PARSER_API XMLWriter : public IReferencable
{
public:
	XMLWriter(IPoolPaged& pool, StringView rootName);
    virtual ~XMLWriter();

	//---

	// root node
	INLINE NodeHandle root() const { return ToNodeId(m_root); }

    //---

	// create child node
	NodeHandle createNode(NodeHandle parentNodeID, StringView name);

	// delete node (and all it's children)
	bool deleteNode(NodeHandle id);

	// set node value
	void changeNodeText(NodeHandle id, StringView value);

	// set node value as base64 buffer
	void changeNodeData(NodeHandle id, Buffer data);

	// set node value as base64 buffer
	void changeNodeDataCopy(NodeHandle id, BufferView data);

	// set node name
	void changeNodeName(NodeHandle id, StringView name);

	//--
	 
	// create node's attribute
	AttributeHandle createAttribute(NodeHandle id, StringView name, StringView value);

	// delete node attribute by name
	bool deleteAttribute(AttributeHandle id);

	// delete node attribute by name
	bool deleteAttribute(NodeHandle id, StringView name);

	// set new attribute name
	void changeAttributeName(AttributeHandle id, StringView name);

	// set new attribute value
	void changeAttributeValue(AttributeHandle id, StringView value);

	//---

	// parent of given node
	NodeHandle nodeParent(NodeHandle nodeId) const;

	// first child of given name
	NodeHandle firstChild(NodeHandle id, StringView childName = "") const;

	// next child of given name
	NodeHandle nextChild(NodeHandle id, StringView siblingName = "") const;

	// node's value as text
	StringView nodeValueText(NodeHandle id) const;

	// node value as BASE64 buffer
	BufferView nodeValueBuffer(NodeHandle id) const;

	// name of the node
	StringView nodeName(NodeHandle id) const;

	//---

	// get name of the attribute
	StringView attributeName(AttributeHandle id) const;

	// get value of the attribute
	StringView attributeValue(AttributeHandle id) const;

	// find value of attribute in a node
	StringView attributeValue(NodeHandle id, StringView name, StringView defaultVal = "") const;

	// first node's attribute
	AttributeHandle firstAttribute(NodeHandle id, StringView name = "") const;
	
	// next attribute
	AttributeHandle nextAttribute(AttributeHandle id, StringView name = "") const;
	
	//---

	// print content to text buffer
	void printToText(NodeHandle id, IFormatStream& f, PrintFlags flags = PrintFlags()) const;

	// print content to a file
	void printToFile(NodeHandle id, IFileWriter* f, PrintFlags flags = PrintFlags()) const;

	//---

	// save text to buffer
	Buffer printToBuffer(NodeHandle id, PrintFlags flags = PrintFlags(), IPoolUnmanaged& pool = MainPool()) const;

	//---

	// create empty document
	static XMLWriterPtr Create(StringView rootName = "document", IPoolPaged& pool = LocalPagePool());

	//---

	// save document to a text output stream
	static void SaveToTextFile(const XMLWriter* ptr, NodeHandle node, IFormatStream& f, PrintFlags flags = PrintFlags());

	// write document to file in text format
	static void SaveToTextFile(const XMLWriter* ptr, NodeHandle node, IFileWriter* writer, PrintFlags flags = PrintFlags());

	//---

private:
	struct Node;

	struct Attribute
	{
		Node* parent = nullptr;
		StringView name;
		StringView value;
		Attribute* next = nullptr;
	};

	struct Value
	{
		StringView text;
		BufferView data;
	};

	struct Node
	{
		StringView name;

		Node* parent = nullptr;
		Node* firstChild = nullptr;
		Node* lastChild = nullptr;
		Node* nextChild = nullptr;
		Attribute* firstAttribute = nullptr;
		Attribute* lastAttribute = nullptr;

		Value value;
	};

	//--

	LocalAllocator m_mem;

	Node* m_root = nullptr;

	Array<Buffer> m_buffers;
	HashMap<StringView, StringView> m_stringMap;

	StringView mapString(StringView txt);
	BufferView mapData(BufferView data);

	//--

	INLINE static Node* ToNodePtr(NodeHandle id) { return (Node*)id; }
	INLINE static Attribute* ToAttributePtr(AttributeHandle id) { return (Attribute*)id; }

	INLINE static NodeHandle ToNodeId(const Node* ptr) { return (NodeHandle)ptr; }
	INLINE static AttributeHandle ToAttributeId(const Attribute* ptr) { return (AttributeHandle)ptr; }

	//--

	static void SaveToTextFileNode(const Node* node, IFormatStream& f, uint32_t depth, bool prettyText);
};

//--

END_INFERNO_NAMESPACE()
