/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "textSegmentedPrinter.h"
#include "bm/core/containers/include/stringBuilder.h"

BEGIN_INFERNO_NAMESPACE()

//--

TextSegmentedPrinter::TextSegmentedPrinter()
{
	m_printer.create();
	m_root.create();

	m_blockStack.pushBack(m_root.get());
}

TextSegmentedPrinter::~TextSegmentedPrinter()
{
	clear();
}

void TextSegmentedPrinter::clear()
{
	m_allSegments.clearPtr();
	m_allBlocks.clearPtr();
	m_activeBlocks.clear();
	m_blockStack.clear();

	m_root->segments.clear();
	m_blockStack.pushBack(m_root.get());

	m_printer->clear();
}

bool TextSegmentedPrinter::enterBlock(StringView name, bool redefinition)
{
	// add a text segment to current top-level block if we printed anything
	pushTextToSegment();

	// add block entry to current block
	if (!redefinition)
	{
		auto* topBlock = m_blockStack.back();

		auto* blockSegment = new Segment();
		m_allSegments.pushBack(blockSegment);

		blockSegment->block = StringBuf(name);
		topBlock->segments.pushBack(blockSegment);
	}

	// look if we already have this block
	Block* prevBlock = nullptr;
	for (auto* block : m_activeBlocks)
	{
		if (block->name == name)
		{
			prevBlock = block;
			m_activeBlocks.remove(block);

			if (m_blockStack.contains(block))
				return false;

			break;
		}
	}

	// create new block entry
	auto* block = new Block();
	m_allBlocks.pushBack(block);
	block->name = StringBuf(name);
	block->prev = prevBlock; // link to previous block so we can reference it
	m_activeBlocks.pushBack(block);

	// set as new stack top
	m_blockStack.pushBack(block);	
	return true;
}

bool TextSegmentedPrinter::exitBlock()
{
	// cannot pop root block
	if (m_blockStack.size() <= 1)
		return false;

	// finish currently printed text
	pushTextToSegment();

	// pop the last block on stack
	m_blockStack.popBack();	
	return true;
}

bool TextSegmentedPrinter::appendSuperBlock()
{
	auto* topBlock = m_blockStack.back();

	if (topBlock->prev)
		return false;

	pushTextToSegment();

	auto* blockSegment = new Segment();
	m_allSegments.pushBack(blockSegment);

	blockSegment->explicitBlock = topBlock->prev;
	topBlock->segments.pushBack(blockSegment);

	return true;
}

//--

void TextSegmentedPrinter::print(IFormatStream& f) const
{
	const_cast<TextSegmentedPrinter*>(this)->pushTextToSegment();

	if (m_root)
		printBlock(f, m_root.get());
}

void TextSegmentedPrinter::printBlock(IFormatStream& f, const Block* b) const
{
	if (b)
	{
		for (const auto* s : b->segments)
			printSegment(f, s);
	}
}

const TextSegmentedPrinter::Block* TextSegmentedPrinter::findBlock(const StringBuf& name) const
{
	for (const auto* b : m_activeBlocks)
		if (b->name == name)
			return b;

	return nullptr;
}

void TextSegmentedPrinter::printSegment(IFormatStream& f, const Segment* s) const
{
	if (s->explicitBlock)
		printBlock(f, s->explicitBlock);
	else if (s->block)
		printBlock(f, findBlock(s->block));
	else if (s->txt)
		f << s->txt;
}

//--

void TextSegmentedPrinter::pushTextToSegment()
{
	auto* topBlock = m_blockStack.back();

	if (!m_printer->empty())
	{
		auto* printSegment = new Segment();
		m_allSegments.pushBack(printSegment);

		printSegment->txt = StringBuf(m_printer->view());

		topBlock->segments.pushBack(printSegment);

		m_printer->clear();
	}
}

IFormatStream& TextSegmentedPrinter::append(const char* str, uint32_t len)
{
	m_printer->append(str, len);
	return *this;
}

//--

END_INFERNO_NAMESPACE()
