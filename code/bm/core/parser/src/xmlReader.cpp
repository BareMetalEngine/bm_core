/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "xmlReader.h"
#include "textToken.h"
#include "textErrorReporter.h"

#include "rapidxml/rapidxml.hpp"
#include "bm/core/file/include/fileReader.h"
#include "bm/core/file/include/fileMapping.h"
#include "textFileWriter.h"

BEGIN_INFERNO_NAMESPACE()

//---

typedef rapidxml::xml_document<char> RapidDoc;
typedef rapidxml::xml_node<char> RapidNode;
typedef rapidxml::xml_attribute<char> RapidAttr;

//---

static NodeHandle ToNodeID(const void* node)
{
	return (NodeHandle)node;
}

static const RapidNode* FromNodeId(NodeHandle id)
{
	return (RapidNode*)id;
}

//---

static AttributeHandle ToAttributeID(const void* attr)
{
	return (AttributeHandle)attr;
}

static const RapidAttr* FromAttributeID(AttributeHandle id)
{
	return (RapidAttr*)id;
}

//---

XMLReader::XMLReader(IPoolUnmanaged& pool, Buffer data, void* doc, void* root)
    : m_data(data)
    , m_pool(pool)
    , m_doc(doc)
    , m_root(root)
{
}

XMLReader::~XMLReader()
{
    PoolDelete(m_pool, m_doc);
}

namespace helper
{
    class RapidXMLErrorForwarder : public rapidxml::ITextErrorReporter
    {
    public:
        RapidXMLErrorForwarder(bm::ITextErrorReporter& ctx, const StringBuf& context)
            : m_ctx(ctx)
            , m_contextName(context)
        {}

        virtual void onError(int line, int pos, const char* txt) override final
        {
            m_hasErrorsReported = true;
            m_ctx.reportError(TextTokenLocation(m_contextName, line, pos), txt);
        }

        INLINE bool hasErrorsReported() const
        {
            return m_hasErrorsReported;
        }

    private:
        bm::ITextErrorReporter& m_ctx;
        bool m_hasErrorsReported = false;
        StringBuf m_contextName;
    };
}

        
XMLReaderPtr XMLReader::LoadFromText(ITextErrorReporter& err, StringView contextName, StringView text, IPoolUnmanaged& pool /*= MainPool()*/)
{
    // empty text buffer produces no results
    if (!text)
        return nullptr;

    // create copy of the text buffer and zero terminate it :(
    auto data = Buffer::CreateEmpty(pool, text.length() + 2);
    DEBUG_CHECK_RETURN_EX_V(data, "Out of memory", nullptr);

    // the buffer for the parser must be zero terminated
    // TODO: fix this!!
    auto bufferBase  = (char*)data.data();
    memcpy(bufferBase, text.data(), text.length());
    bufferBase[text.length() + 0] = 0;
    bufferBase[text.length() + 1] = 0;

    // create XML document wrapper
    auto* doc = PoolNew<RapidDoc>(pool);

    // parse the document
    helper::RapidXMLErrorForwarder errorForwarder(err, StringBuf(contextName));
    if (!doc->parse<0>(bufferBase, errorForwarder))
    {
        // make sure at least one error is reported if we've failed parsing
        if (!errorForwarder.hasErrorsReported())
            err.reportError(TextTokenLocation(StringBuf(contextName), 1, 1), "Unspecified error parsing XML data");

        PoolDelete(pool, doc);
        return nullptr;
    }

    // get the root node
    auto* root = doc->first_node();
    if (!root)
    {
        PoolDelete(pool, doc);
        TRACE_ERROR("XML has no root node and cannot be loaded");
        return nullptr;
    }

    // create wrapper
	return RefNew<XMLReader>(pool, data, doc, root);
}

//---

NodeHandle XMLReader::root() const
{
    return ToNodeID(m_root);
}

NodeHandle XMLReader::firstChild(NodeHandle id, StringView childName /*= nullptr*/) const
{
    auto node  = FromNodeId(id);
    if (!node)
        return 0;

    auto childNode  = childName.empty() ? node->first_node() : node->first_node(childName.data(), childName.length());
    while (childNode)
    {
        if (childNode->type() == rapidxml::node_element)
            break;
        childNode = childName.empty() ? childNode->next_sibling() : childNode->next_sibling(childName.data(), childName.length());
    }

    return ToNodeID(childNode);
}

NodeHandle XMLReader::nextChild(NodeHandle id, StringView siblingName /*= nullptr*/) const
{
    auto node  = FromNodeId(id);
    if (!node)
        return 0;

    auto childNode = siblingName.empty() ? node->next_sibling() : node->next_sibling(siblingName.data(), siblingName.length());
    while (childNode)
    {
        if (childNode->type() == rapidxml::node_element)
            break;
        childNode = siblingName.empty() ? childNode->next_sibling() : childNode->next_sibling(siblingName.data(), siblingName.length());
    }

    return ToNodeID(childNode);
}

NodeHandle XMLReader::nodeParent(NodeHandle id) const
{
    auto node = FromNodeId(id);
    if (!node)
        return 0;

    return ToNodeID(node->parent());
}

uint32_t XMLReader::nodeLine(NodeHandle id) const
{
	auto node = FromNodeId(id);
    if (node)
        return node->location().line();
    else
        return 1;
}

//---

StringView XMLReader::nodeValueText(NodeHandle id) const
{
    auto node  = FromNodeId(id);
    if (!node)
        return StringBuf::EMPTY();

    return StringView((const char*)node->value(), (uint32_t)node->value_size());
}

Buffer XMLReader::nodeValueBuffer(NodeHandle id, IPoolUnmanaged* pool) const
{
    auto node = FromNodeId(id);
    if (!node || !node->value_size())
        return nullptr;

    uint32_t decodedDataSize = 0;

    auto valueStr = StringView(node->value(), node->value() + node->value_size());
    return valueStr.decode(pool ? *pool : m_pool, EncodingType::Base64);
}

StringView XMLReader::nodeName(NodeHandle id) const
{
    auto node  = FromNodeId(id);
    if (!node)
        return StringBuf::EMPTY();

    return StringView(node->name(), (uint32_t)node->name_size());
}

StringView XMLReader::attributeValue(NodeHandle id, StringView name, StringView defaultVal) const
{
    auto node  = FromNodeId(id);
    if (!node)
        return defaultVal;

    auto attr  = name.empty() ? node->first_attribute() : node->first_attribute(name.data(), name.length());
    if (!attr)
        return defaultVal;

    return StringView(attr->value(), (uint32_t)attr->value_size());
}

AttributeHandle XMLReader::firstAttribute(NodeHandle id, StringView name /*= nullptr*/) const
{
    auto node  = FromNodeId(id);
    if (!node)
        return 0;

    return ToAttributeID(name.empty() ? node->first_attribute() : node->first_attribute(name.data(), name.length()));
}

//---

StringView XMLReader::attributeName(AttributeHandle id) const
{
    auto attr  = FromAttributeID(id);
    if (!attr)
        return StringView();

    return StringView(attr->name(), (uint32_t)attr->name_size());
}

StringView XMLReader::attributeValue(AttributeHandle id) const
{
    auto attr  = FromAttributeID(id);
    if (!attr)
        return StringView();

    return StringView(attr->value(), (uint32_t)attr->value_size());
}

AttributeHandle XMLReader::nextAttribute(AttributeHandle id, StringView name /*= nullptr*/) const
{
    auto attr  = FromAttributeID(id);
    if (!attr)
        return 0;

    return ToAttributeID(name.empty() ? attr->next_attribute() : attr->next_attribute(name.data(), name.length()));
}

//---

XMLReaderPtr XMLReader::LoadFromFile(ITextErrorReporter& ctx, StringView absoluteFilePath, TimeStamp* outTimeStamp/*= nullptr*/, IPoolUnmanaged& pool /*= MainPool()*/)
{
	Buffer buffer;
	if (!FileSystem().loadFileToBuffer(absoluteFilePath, MainPool(), buffer, outTimeStamp))
		return nullptr;

	return LoadFromBuffer(ctx, absoluteFilePath, buffer);
}

XMLReaderPtr XMLReader::LoadFromFile(ITextErrorReporter& err, StringView contextName, IFileReader* file, IPoolUnmanaged& pool /*= MainPool()*/)
{
	DEBUG_CHECK_RETURN_EX_V(file, "Invalid file", nullptr);

	const auto data = file->createMapping(file->fullRange());
	DEBUG_CHECK_RETURN_EX_V(data, "Read failed", nullptr);

	const auto buffer = data->createBuffer();
	DEBUG_CHECK_RETURN_EX_V(buffer, "Read failed", nullptr);

	return LoadFromBuffer(err, contextName, buffer);
}

XMLReaderPtr XMLReader::LoadFromBuffer(ITextErrorReporter& ctx, StringView contextName, Buffer mem, IPoolUnmanaged& pool /*= MainPool()*/)
{
	// to small
	if (!mem || mem.size() < 7)
		return nullptr;

	// Check the data header
	auto header = (const char*)mem.data();
	if (0 == strncmp(header, "<?xml ", 6))
	{
		return XMLReader::LoadFromText(ctx, StringBuf(contextName), StringView(mem));
	}
	else if (0 == strncmp(header, "BINXML", 6))
	{
		//return StaticBinaryXMLDocument::Load(ctx, mem);
		return nullptr;
	}
	else
	{
		TRACE_ERROR("Unable to load XML data from buffer");
		return nullptr;
	}
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

void XMLReader::SaveToTextFileNode(const void* ptr, IFormatStream& f, uint32_t depth, bool prettyText)
{
    const auto* node = (const RapidNode*)ptr;

	// get name of the node, the invalid nodes have no name
	if (!node || !node->name_size())
		return;

	// entry
	if (prettyText)
		f.appendPadding(' ', depth * 2);
	f.append("<");
    f.append(node->name(), node->name_size());

	// emit attributes
	for (const auto* attr = node->first_attribute(); attr; attr = attr->next_attribute())
	{
		f.append(" ");
        f.append(attr->name(), attr->name_size());
		f.append("=\"");
		WriteString(f, StringView(attr->value(), attr->value_size()));
		f.append("\"");
	}

	// write text value
	bool writeClosingTag = true;
	if (node->first_node())
	{
		f.append(">");
		if (prettyText)
			f.append("\n");

		for (const auto* child = node->first_node(); child; child = child->next_sibling())
			SaveToTextFileNode(child, f, depth + 1, prettyText);

		if (prettyText)
			f.appendPadding(' ', depth * 2);
	}
	else if (node->value_size())
	{
		f.append(">");
        WriteString(f, StringView(node->value(), node->value_size()));
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
        f.append(node->name(), node->name_size());
		f.append(">");
		if (prettyText)
			f.append("\n");
	}
}

void XMLReader::printToText(NodeHandle id, IFormatStream& f, PrintFlags flags /*= PrintFlags()*/) const
{
	DEBUG_CHECK_RETURN_EX(id, "Invalid node");

	if (!flags.test(PrintFlagBit::NoHeader))
	{
		f.append("<?xml version=\"1.0\" standalone=\"yes\"?>");
		if (flags.test(PrintFlagBit::PrettyText))
			f.append("\n");
	}

	SaveToTextFileNode((const void*)id, f, 0, flags.test(PrintFlagBit::PrettyText));
}

void XMLReader::printToFile(NodeHandle id, IFileWriter* writer, PrintFlags flags /*= PrintFlags()*/) const
{
	DEBUG_CHECK_RETURN_EX(writer, "Invalid writer");

	TextFileWriter textWriter(writer);
	printToText(id, textWriter, flags);
}

//--

Buffer XMLReader::printToBuffer(NodeHandle id, PrintFlags flags /*= PrintFlags()*/, IPoolUnmanaged& pool /*= MainPool()*/) const
{
    StringBuilder txt;
    printToText(id, txt, flags);

    return txt.view().toBuffer(pool);
}

//--

void XMLReader::SaveToTextFile(const XMLReader* ptr, NodeHandle node, IFormatStream& f, PrintFlags flags /*= PrintFlags()*/)
{
	DEBUG_CHECK_RETURN_EX(ptr, "Invalid document");
	DEBUG_CHECK_RETURN_EX(node, "Invalid node");
	ptr->printToText(node, f, flags);
}

void XMLReader::SaveToTextFile(const XMLReader* ptr, NodeHandle node, IFileWriter* writer, PrintFlags flags /*= PrintFlags()*/)
{
	DEBUG_CHECK_RETURN_EX(ptr, "Invalid document");
	DEBUG_CHECK_RETURN_EX(node, "Invalid node");
	ptr->printToFile(node, writer, flags);
}

//--

END_INFERNO_NAMESPACE()
