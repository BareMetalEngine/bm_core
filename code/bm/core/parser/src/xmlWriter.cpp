/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "xmlWriter.h"
#include "textFileWriter.h"

BEGIN_INFERNO_NAMESPACE()

//--

XMLWriter::XMLWriter(IPoolPaged& pool, StringView rootName)
	: m_mem(pool)
{
	m_root = m_mem.createWithoutDestruction<Node>();
	m_root->name = mapString(rootName ? rootName : "document");
}

XMLWriter::~XMLWriter()
{
}

//--

NodeHandle XMLWriter::createNode(NodeHandle parent, StringView name)
{
	DEBUG_CHECK_RETURN_EX_V(parent != 0, "Invalid node parent", 0);
	DEBUG_CHECK_RETURN_EX_V(name, "Invalid node name", 0);

	auto* node = m_mem.createWithoutDestruction<Node>();
	memzero(node, sizeof(Node));

	node->parent = ToNodePtr(parent);
	node->name = mapString(name);

	if (node->parent->lastChild)
		node->parent->lastChild->nextChild = node;
	else
		node->parent->firstChild = node;

	node->parent->lastChild = node;

	return ToNodeId(node);
}

bool XMLWriter::deleteNode(NodeHandle nodeId)
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

bool XMLWriter::deleteAttribute(NodeHandle nodeId, StringView name)
{
	DEBUG_CHECK_RETURN_EX_V(nodeId != 0, "Invalid node", false);

	auto node = ToNodePtr(nodeId);

	auto** prev = &node->firstAttribute;
	auto* cur = *prev;
	while (cur)
	{
		if (cur->name == name)
		{
			*prev = cur->next;
			if (!node->firstAttribute)
				node->lastAttribute = nullptr;
			return true;
		}

		cur = cur->next;
		prev = &cur->next;
	}

	return false;
}

bool XMLWriter::deleteAttribute(AttributeHandle id)
{
	auto attr = ToAttributePtr(id);
	DEBUG_CHECK_RETURN_EX_V(attr, "Invalid node", false);

	auto node = attr->parent;

	auto** prev = &node->firstAttribute;
	auto* cur = *prev;
	while (cur)
	{
		if (cur == attr)
		{
			*prev = cur->next;
			if (!node->firstAttribute)
				node->lastAttribute = nullptr;
			return true;
		}

		cur = cur->next;
		prev = &cur->next;
	}

	return false;
}

void XMLWriter::changeNodeName(NodeHandle id, StringView txt)
{
	auto node = ToNodePtr(id);
	DEBUG_CHECK_RETURN_EX(node, "Invalid node");

	if (node->name != txt)
		node->name = mapString(txt);
}

void XMLWriter::changeNodeText(NodeHandle id, StringView txt)
{
	auto node = ToNodePtr(id);
	DEBUG_CHECK_RETURN_EX(node, "Invalid node");

	if (node->value.text != txt)
	{
		node->value.data = nullptr;
		node->value.text = m_mem.strcpy(txt.data(), txt.length());
	}
}

void XMLWriter::changeNodeData(NodeHandle id, Buffer buffer)
{
	auto node = ToNodePtr(id);
	DEBUG_CHECK_RETURN_EX(node, "Invalid node");

	if (buffer)
		m_buffers.pushBack(buffer);

	node->value.text = nullptr;
	node->value.data = buffer.view();
}

void XMLWriter::changeNodeDataCopy(NodeHandle id, BufferView data)
{
	auto node = ToNodePtr(id);
	DEBUG_CHECK_RETURN_EX(node, "Invalid node");

	node->value.text = nullptr;
	node->value.data = mapData(data);
}

//--

NodeHandle XMLWriter::nodeParent(NodeHandle id) const
{
	auto node = ToNodePtr(id);
	DEBUG_CHECK_RETURN_EX_V(node, "Invalid node", 0);
	return ToNodeId(node->parent);
}

NodeHandle XMLWriter::firstChild(NodeHandle id, StringView childName) const
{
	auto node = ToNodePtr(id);
	DEBUG_CHECK_RETURN_EX_V(node, "Invalid node", 0);

	node = node->firstChild;

	if (!childName)
		return ToNodeId(node);

	while (node)
	{
		if (node->name == childName)
			return ToNodeId(node);

		node = node->nextChild;
	}

	return 0;
}

NodeHandle XMLWriter::nextChild(NodeHandle id, StringView siblingName) const
{
	auto node = ToNodePtr(id);
	DEBUG_CHECK_RETURN_EX_V(node, "Invalid node", 0);

	node = node->nextChild;

	if (!siblingName)
		return ToNodeId(node);

	while (node)
	{
		if (node->name == siblingName)
			return ToNodeId(node);

		node = node->nextChild;
	}

	return 0;	
}

StringView XMLWriter::nodeValueText(NodeHandle id) const
{
	auto node = ToNodePtr(id);
	DEBUG_CHECK_RETURN_EX_V(node, "Invalid node", nullptr);
	return node->value.text;
}

BufferView XMLWriter::nodeValueBuffer(NodeHandle id) const
{
	auto node = ToNodePtr(id);
	DEBUG_CHECK_RETURN_EX_V(node, "Invalid node", nullptr);
	return node->value.data;
}

StringView XMLWriter::nodeName(NodeHandle id) const
{
	auto node = ToNodePtr(id);
	DEBUG_CHECK_RETURN_EX_V(node, "Invalid node", "");
	return node->name;
}

//--

AttributeHandle XMLWriter::createAttribute(NodeHandle id, StringView name, StringView value)
{
	DEBUG_CHECK_RETURN_EX_V(id, "Invalid node", 0);
	DEBUG_CHECK_RETURN_EX_V(name, "Invalid name", 0);

	auto node = ToNodePtr(id);

	auto* attr = m_mem.createWithoutDestruction<Attribute>();
	memzero(attr, sizeof(Attribute));

	attr->parent = node;
	attr->name = mapString(name);
	attr->value = mapString(value);

	if (node->lastAttribute)
		node->lastAttribute->next = attr;
	else
		node->firstAttribute = attr;

	node->lastAttribute = attr;

	return ToNodeId(node);
}

void XMLWriter::changeAttributeName(AttributeHandle id, StringView name)
{
	DEBUG_CHECK_RETURN_EX(id, "Invalid node");
	DEBUG_CHECK_RETURN_EX(name, "Invalid name");

	auto attr = ToAttributePtr(id);
	if (attr->name != name)
		attr->name = mapString(name);
}

void XMLWriter::changeAttributeValue(AttributeHandle id, StringView value)
{
	DEBUG_CHECK_RETURN_EX(id, "Invalid node");

	auto attr = ToAttributePtr(id);
	if (attr->value != value)
		attr->value = mapString(value);
}

//--

StringView XMLWriter::attributeName(AttributeHandle id) const
{
	DEBUG_CHECK_RETURN_EX_V(id, "Invalid node", "");

	auto attr = ToAttributePtr(id);
	return attr->name;
}

StringView XMLWriter::attributeValue(AttributeHandle id) const
{
	DEBUG_CHECK_RETURN_EX_V(id, "Invalid node", "");

	auto attr = ToAttributePtr(id);
	return attr->value;
}

StringView XMLWriter::attributeValue(NodeHandle id, StringView name, StringView defaultVal) const
{
	DEBUG_CHECK_RETURN_EX_V(id, "Invalid node", "");

	auto node = ToNodePtr(id);	
	auto attr = node->firstAttribute;
	while (attr)
	{
		if (attr->name == name)
			return attr->value;

		attr = attr->next;
	}

	return defaultVal;
}

AttributeHandle XMLWriter::nextAttribute(AttributeHandle id, StringView name) const
{
	DEBUG_CHECK_RETURN_EX_V(id, "Invalid node", 0);

	auto attr = ToAttributePtr(id);

	if (!name)
		name = attr->name;

	while (attr)
	{
		if (attr->name == name)
			return ToAttributeId(attr);
		attr = attr->next;
	}
	
	return 0;
}

AttributeHandle XMLWriter::firstAttribute(NodeHandle id, StringView name) const
{
	DEBUG_CHECK_RETURN_EX_V(id, "Invalid node", 0);

	auto node = ToNodePtr(id);
	return ToAttributeId(node->firstAttribute);
}

//--

BufferView XMLWriter::mapData(BufferView data)
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

StringView XMLWriter::mapString(StringView txt)
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

static void WriteString(IFormatStream& f, StringView txt)
{
	for (auto ch : txt)
	{
		if (ch == '<')
			f.append("&lt;");
		else if (ch == '>')
			f.append("&gt;");
		else if (ch == '&')
			f.append("&amp;");
		else if (ch == '\"')
			f.append("&quot;");
		else if (ch == '\'')
			f.append("&pos;");
		else if (ch < 32)
			f.appendf("&#{};", ch);
		else
		{
			char str[2] = { ch, 0 };
			f.append(str);
		}
	}
}

void XMLWriter::SaveToTextFileNode(const Node* node, IFormatStream& f, uint32_t depth, bool prettyText)
{
	// get name of the node, the invalid nodes have no name
	if (!node || !node->name)
		return;

	// entry
	if (prettyText)
		f.appendPadding(' ', depth * 2);
	f.append("<");
	f << node->name;

	// emit attributes
	for (const auto* attr = node->firstAttribute; attr; attr = attr->next)
	{
		f.append(" ");
		f << attr->name;
		f.append("=\"");
		WriteString(f, attr->value);
		f.append("\"");
	}

	// write text value
	bool writeClosingTag = true;
	if (node->firstChild)
	{
		f.append(">");
		if (prettyText)
			f.append("\n");

		for (const auto* child = node->firstChild; child; child = child->nextChild)
			SaveToTextFileNode(child, f, depth + 1, prettyText);

		if (prettyText)
			f.appendPadding(' ', depth * 2);
	}
	else if (node->value.text)
	{
		f.append(">");
		WriteString(f, node->value.text);
	}
	else if (node->value.data)
	{
		f.append(">");
		node->value.data.encode(EncodingType::Base64, f);
	}
	else
	{
		writeClosingTag = false;
		f.append("/>");
		if (prettyText)
			f.append("\n");
	}

	if (writeClosingTag)
	{
		f.append("</");
		f << node->name;
		f.append(">");
		if (prettyText)
			f.append("\n");
	}
}

void XMLWriter::printToText(NodeHandle id, IFormatStream& f, PrintFlags flags /*= PrintFlags()*/) const
{
	auto node = ToNodePtr(id);
	DEBUG_CHECK_RETURN_EX(node, "Invalid node");

	if (!flags.test(PrintFlagBit::NoHeader))
	{
		f.append("<?xml version=\"1.0\" standalone=\"yes\"?>");
		if (flags.test(PrintFlagBit::PrettyText))
			f.append("\n");
	}

	SaveToTextFileNode(node, f, 0, flags.test(PrintFlagBit::PrettyText));
}

void XMLWriter::printToFile(NodeHandle id, IFileWriter* writer, PrintFlags flags /*= PrintFlags()*/) const
{
	DEBUG_CHECK_RETURN_EX(writer, "Invalid writer");

	TextFileWriter textWriter(writer);
	printToText(id, textWriter, flags);
}

//--

Buffer XMLWriter::printToBuffer(NodeHandle id, PrintFlags flags /*= PrintFlags()*/, IPoolUnmanaged& pool /*= MainPool()*/) const
{
	StringBuilder txt;
	printToText(id, txt, flags);

	return txt.view().toBuffer(pool);
}

//--

XMLWriterPtr XMLWriter::Create(StringView rootName /*= "document"*/, IPoolPaged& pool /*= LocalPagePool()*/)
{
	return RefNew<XMLWriter>(pool, rootName);
}

void XMLWriter::SaveToTextFile(const XMLWriter* ptr, NodeHandle node, IFormatStream& f, PrintFlags flags /*= PrintFlags()*/)
{
	DEBUG_CHECK_RETURN_EX(ptr, "Invalid document");
	DEBUG_CHECK_RETURN_EX(node, "Invalid node");
	ptr->printToText(node, f, flags);
}

void XMLWriter::SaveToTextFile(const XMLWriter* ptr, NodeHandle node, IFileWriter* writer, PrintFlags flags /*= PrintFlags()*/)
{
	DEBUG_CHECK_RETURN_EX(ptr, "Invalid document");
	DEBUG_CHECK_RETURN_EX(node, "Invalid node");
	ptr->printToFile(node, writer, flags);
}

//--

END_INFERNO_NAMESPACE()
