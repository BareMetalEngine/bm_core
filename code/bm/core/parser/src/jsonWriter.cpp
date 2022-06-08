/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "jsonWriter.h"
#include "textFileWriter.h"
#include "bm/core/containers/include/utf8StringFunctions.h"

BEGIN_INFERNO_NAMESPACE()

//--

JSONWriter::JSONWriter(IPoolPaged& pool)
	: m_mem(pool)
{
	m_root = m_mem.createWithoutDestruction<Node>();
	m_root->type = JSONNodeType::None;
}

JSONWriter::~JSONWriter()
{
}

//--

NodeHandle JSONWriter::createNode(NodeHandle parent, JSONNodeType type /*= JSONNodeType::None*/)
{
	DEBUG_CHECK_RETURN_EX_V(parent != 0, "Invalid node parent", 0);

	auto* node = m_mem.createWithoutDestruction<Node>();
	memzero(node, sizeof(Node));

	node->parent = ToNodePtr(parent);
	node->type = type;// ] = mapString(name);

	if (node->parent->lastChild)
		node->parent->lastChild->nextChild = node;
	else
		node->parent->firstChild = node;

	node->parent->lastChild = node;

	return ToNodeId(node);
}

bool JSONWriter::deleteNode(NodeHandle nodeId)
{
	DEBUG_CHECK_RETURN_EX_V(nodeId != 0, "Invalid node", false);

	auto* node = ToNodePtr(nodeId);
	auto* parentNode = node->parent;
	DEBUG_CHECK_RETURN_EX_V(parentNode, "Cannot delete root node", false);

	auto** prev = &parentNode->firstChild;
	auto* cur = *prev;
	while (cur)
	{
		if (cur == node)
		{
			*prev = cur->nextChild;
			if (!parentNode->firstChild)
				parentNode->lastChild = nullptr;
			return true;
		}

		cur = cur->nextChild;
		prev = &cur->nextChild;
	}

	return false;
}

void JSONWriter::changeNodeType(NodeHandle id, JSONNodeType type)
{
	auto node = ToNodePtr(id);
	DEBUG_CHECK_RETURN_EX(node, "Invalid node");

	node->type = type;
}

void JSONWriter::changeNodeName(NodeHandle id, StringView txt)
{
	auto node = ToNodePtr(id);
	DEBUG_CHECK_RETURN_EX(node, "Invalid node");

	if (node->name != txt)
		node->name = mapString(txt);
}

void JSONWriter::changeNodeText(NodeHandle id, StringView txt)
{
	auto node = ToNodePtr(id);
	DEBUG_CHECK_RETURN_EX(node, "Invalid node");

	node->type = JSONNodeType::Value;

	if (node->value.text != txt)
	{
		node->value.data = nullptr;
		node->value.text = m_mem.strcpy(txt.data(), txt.length());
	}
}

void JSONWriter::changeNodeData(NodeHandle id, Buffer buffer)
{
	auto node = ToNodePtr(id);
	DEBUG_CHECK_RETURN_EX(node, "Invalid node");

	if (buffer)
		m_buffers.pushBack(buffer);

	node->type = JSONNodeType::Value;
	node->value.text = nullptr;
	node->value.data = buffer.view();
}

void JSONWriter::changeNodeDataCopy(NodeHandle id, BufferView data)
{
	auto node = ToNodePtr(id);
	DEBUG_CHECK_RETURN_EX(node, "Invalid node");

	node->type = JSONNodeType::Value;
	node->value.text = nullptr;
	node->value.data = mapData(data);
}

//--

NodeHandle JSONWriter::nodeParent(NodeHandle id) const
{
	auto node = ToNodePtr(id);
	DEBUG_CHECK_RETURN_EX_V(node, "Invalid node", 0);
	return ToNodeId(node->parent);
}

NodeHandle JSONWriter::firstChild(NodeHandle id) const
{
	auto node = ToNodePtr(id);
	DEBUG_CHECK_RETURN_EX_V(node, "Invalid node", 0);
	return ToNodeId(node->firstChild);
}

NodeHandle JSONWriter::nextChild(NodeHandle id) const
{
	auto node = ToNodePtr(id);
	DEBUG_CHECK_RETURN_EX_V(node, "Invalid node", 0);
	return ToNodeId(node->nextChild);
}

StringView JSONWriter::nodeValueText(NodeHandle id) const
{
	auto node = ToNodePtr(id);
	DEBUG_CHECK_RETURN_EX_V(node, "Invalid node", nullptr);
	return node->value.text;
}

BufferView JSONWriter::nodeValueBuffer(NodeHandle id) const
{
	auto node = ToNodePtr(id);
	DEBUG_CHECK_RETURN_EX_V(node, "Invalid node", nullptr);
	return node->value.data;
}

StringView JSONWriter::nodeName(NodeHandle id) const
{
	auto node = ToNodePtr(id);
	DEBUG_CHECK_RETURN_EX_V(node, "Invalid node", "");
	return node->name;
}

//--

BufferView JSONWriter::mapData(BufferView data)
{
	if (!data)
		return nullptr;

	if (data.size() > 1024)
	{
		auto buffer = Buffer::CreateFromCopy(MainPool(), data);
		DEBUG_CHECK_RETURN_EX_V(buffer, "Out of memory", nullptr);

		m_buffers.pushBack(buffer);
		return buffer.view();
	}

	auto mem = m_mem.alloc(data.size(), 1);
	DEBUG_CHECK_RETURN_EX_V(mem, "Out of memory", nullptr);

	memcpy(mem, data.data(), data.size());

	return BufferView(mem, data.size());
}

StringView JSONWriter::mapString(StringView txt)
{
	if (!txt)
		return "";

	StringView copy;
	if (m_stringMap.find(txt, copy))
		return copy;

	copy = m_mem.strcpy(txt.data(), txt.length());
	m_stringMap[copy] = copy;

	return copy;
}

//--


static bool JSonNeedsQuotes(StringView view)
{
	const auto* pos = view.data();
	const auto* end = pos + view.length();

	while (pos < end)
	{
		const auto ch = *pos++;

		switch (ch)
		{
		case '"':
		case '\\':
		case '/':
		case '[':
		case ']':
		case '{':
		case '}':
		case '\'':
		case '`':
			return true;

		default:
			if (ch <= ' ')
				return true;
		}
	}

	return view.empty(); // empty strings need quotes
}

static void WriteString(IFormatStream& f, StringView txt, bool relaxed)
{
	bool needsQuotes = true;

	if (relaxed && !txt.empty())
		needsQuotes = JSonNeedsQuotes(txt);

	if (needsQuotes)
		f << "\"";

	for (utf8::CharIterator it(txt); it; ++it)
	{
		auto ch = *it;

		if (ch == '\"')
			f.append("\\\"");
		else if (ch == '\\')
			f.append("\\\\");
		else if (ch == '/')
			f.append("\\/");
		else if (ch == '\b')
			f.append("\\b");
		else if (ch == '\f')
			f.append("\\f");
		else if (ch == '\n')
			f.append("\\n");
		else if (ch == '\r')
			f.append("\\r");
		else if (ch == '\t')
			f.append("\\t");
		else if (ch < 32 || ch > 127)
			f.appendf("\\{}", PaddedHex<4, '0'>(ch));
		else
		{
			char str[2] = { ch, 0 };
			f.append(str);
		}
	}

	if (needsQuotes)
		f << "\"";
}

void JSONWriter::MakeNewLine(IFormatStream& f, PrintState& state)
{
	if (state.prettyText)
	{
		f.append("\n");
		f.appendPadding(' ', state.indentation * 4);
	}

	state.needsNewLine = false;
}

void JSONWriter::MakeNewLineIfNeeded(IFormatStream& f, PrintState& state)
{
	if (state.needsNewLine)
		MakeNewLine(f, state);
}

void JSONWriter::MakeBlockStart(IFormatStream& f, const char* ch, PrintState& state)
{
	f << ch;

	state.indentation += 1;
	state.needsNewLine = true;
	state.needsSeparator.pushBack(false);
}

void JSONWriter::MakeBlockEnd(IFormatStream& f, const char* ch, PrintState& state)
{
	ASSERT(!state.needsSeparator.empty());
	state.needsSeparator.popBack();
	state.indentation -= 1;
	MakeNewLineIfNeeded(f, state);

	f << ch;
}

void JSONWriter::MakeElementSeparator(IFormatStream& f, PrintState& state)
{
	if (state.needsSeparator.back())
	{
		if (state.relaxed)
			f << " ";
		else
			f << (state.prettyText ? "," : ", ");

		state.needsSeparator.back() = false;
	}

	state.needsNewLine = true;
}

void JSONWriter::SaveToTextFileNode(const Node* node, IFormatStream& f, PrintState& state)
{
	// get name of the node, the invalid nodes have no name
	if (!node || node->type == JSONNodeType::None)
		return;

	// value
	if (node->type == JSONNodeType::Compound)
	{
		MakeBlockStart(f, "{", state);

		for (const auto* child = node->firstChild; child; child = child->nextChild)
		{
			if (!child->name || child->type == JSONNodeType::None)
				continue;

			MakeElementSeparator(f, state);
			MakeNewLineIfNeeded(f, state);
			WriteString(f, child->name, state.relaxed);
			f << (state.relaxed ? ":" : ": ");

			SaveToTextFileNode(child, f, state);

			state.needsSeparator.back() = true;
			state.needsNewLine = true;
		}

		MakeBlockEnd(f, "}", state);
	}
	else if (node->type == JSONNodeType::Array)
	{
		MakeBlockStart(f, "[", state);

		for (const auto* child = node->firstChild; child; child = child->nextChild)
		{
			if (child->type == JSONNodeType::None)
				continue;

			MakeElementSeparator(f, state);
			MakeNewLineIfNeeded(f, state);

			SaveToTextFileNode(child, f, state);

			state.needsSeparator.back() = true;
			state.needsNewLine = true;
		}

		MakeBlockEnd(f, "]", state);
	}
	else if (node->type == JSONNodeType::Value)
	{
		if (node->value.data)
		{
			if (!state.relaxed) f << "\"";
			node->value.data.encode(EncodingType::Base64, f);
			if (!state.relaxed) f << "\"";
		}
		else
		{
			WriteString(f, node->value.text, state.relaxed);
		}
	}
}

void JSONWriter::printToText(NodeHandle id, IFormatStream& f, PrintFlags flags /*= PrintFlags()*/) const
{
	auto node = ToNodePtr(id);
	DEBUG_CHECK_RETURN_EX(node, "Invalid node");

	PrintState state;
	state.indentation = 0;
	state.needsNewLine = flags.test(PrintFlagBit::PrettyText);
	state.relaxed = flags.test(PrintFlagBit::Relaxed);
	SaveToTextFileNode(node, f, state);
}

void JSONWriter::printToFile(NodeHandle id, IFileWriter* writer, PrintFlags flags /*= PrintFlags()*/) const
{
	DEBUG_CHECK_RETURN_EX(writer, "Invalid writer");

	TextFileWriter textWriter(writer);
	printToText(id, textWriter, flags);
}

//--

JSONWriterPtr JSONWriter::Create(IPoolPaged& pool /*= LocalPagePool()*/)
{
	return RefNew<JSONWriter>(pool);
}

void JSONWriter::SaveToTextFile(const JSONWriter* ptr, NodeHandle node, IFormatStream& f, PrintFlags flags /*= PrintFlags()*/)
{
	DEBUG_CHECK_RETURN_EX(ptr, "Invalid document");
	DEBUG_CHECK_RETURN_EX(node, "Invalid node");
	ptr->printToText(node, f, flags);
}

void JSONWriter::SaveToTextFile(const JSONWriter* ptr, NodeHandle node, IFileWriter* writer, PrintFlags flags /*= PrintFlags()*/)
{
	DEBUG_CHECK_RETURN_EX(ptr, "Invalid document");
	DEBUG_CHECK_RETURN_EX(node, "Invalid node");
	ptr->printToFile(node, writer, flags);
}

//--

END_INFERNO_NAMESPACE()
