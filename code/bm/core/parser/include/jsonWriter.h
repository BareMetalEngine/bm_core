/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

#include "bm/core/memory/include/localAllocator.h"
#include "bm/core/containers/include/hashMap.h"
#include "bm/core/containers/include/inplaceArray.h"

BEGIN_INFERNO_NAMESPACE()

//--

// Writable JSON document, allows to build data
class BM_CORE_PARSER_API JSONWriter : public IReferencable
{
public:
	JSONWriter(IPoolPaged& pool);
    virtual ~JSONWriter();

	//---

	// root node
	INLINE NodeHandle root() const { return ToNodeId(m_root); }

    //---

	// create child node
	NodeHandle createNode(NodeHandle parentNodeID, JSONNodeType type = JSONNodeType::None);

	// delete node (and all it's children)
	bool deleteNode(NodeHandle id);

	// change node type
	void changeNodeType(NodeHandle id, JSONNodeType type);

	// set node value (changes node type to value)
	void changeNodeText(NodeHandle id, StringView value);

	// set node value as base64 buffer
	void changeNodeData(NodeHandle id, Buffer data);

	// set node value as base64 buffer
	void changeNodeDataCopy(NodeHandle id, BufferView data);

	// set node name (for named keys) - DOES NOT CHANGE THE NODE's TYPE
	// NOTE: name is only printed if parent node type is Compound
	void changeNodeName(NodeHandle id, StringView name);

	//---

	// parent of given node
	NodeHandle nodeParent(NodeHandle nodeId) const;

	// type of the node
	JSONNodeType nodeType(NodeHandle nodeId) const;

	// first child of given name
	NodeHandle firstChild(NodeHandle id) const;

	// next child of given name
	NodeHandle nextChild(NodeHandle id) const;

	// node's value as text
	StringView nodeValueText(NodeHandle id) const;

	// node value as BASE64 buffer
	BufferView nodeValueBuffer(NodeHandle id) const;

	// name of the node
	StringView nodeName(NodeHandle id) const;

	//---

	// print content to text buffer
	void printToText(NodeHandle id, IFormatStream& f, PrintFlags flags = PrintFlags()) const;

	// print content to a file
	void printToFile(NodeHandle id, IFileWriter* f, PrintFlags flags = PrintFlags()) const;

	//---

	// create empty document
	static JSONWriterPtr Create(IPoolPaged& pool = LocalPagePool());

	//---

	// save document to a text output stream
	static void SaveToTextFile(const JSONWriter* ptr, NodeHandle node, IFormatStream& f, PrintFlags flags = PrintFlags());

	// write document to file in text format
	static void SaveToTextFile(const JSONWriter* ptr, NodeHandle node, IFileWriter* writer, PrintFlags flags = PrintFlags());

	//---

private:
	struct Value
	{
		StringView text;
		BufferView data;
	};

	struct Node
	{
		JSONNodeType type = JSONNodeType::None;
		StringView name;

		Node* parent = nullptr;
		Node* firstChild = nullptr;
		Node* lastChild = nullptr;
		Node* nextChild = nullptr;

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
	INLINE static NodeHandle ToNodeId(const Node* ptr) { return (NodeHandle)ptr; }

	//--

	struct PrintState
	{
		bool prettyText = false;
		bool relaxed = false;

		uint32_t indentation = 0;
		bool needsNewLine = false;
		InplaceArray<bool, 32> needsSeparator;
	};

	static void SaveToTextFileNode(const Node* node, IFormatStream& f, PrintState& state);

	static void MakeNewLine(IFormatStream& f, PrintState& state);
	static void MakeNewLineIfNeeded(IFormatStream& f, PrintState& state);
	static void MakeBlockStart(IFormatStream& f, const char* ch, PrintState& state);
	static void MakeBlockEnd(IFormatStream& f, const char* ch, PrintState& state);
	static void MakeElementSeparator(IFormatStream& f, PrintState& state);
};

//--

END_INFERNO_NAMESPACE()
