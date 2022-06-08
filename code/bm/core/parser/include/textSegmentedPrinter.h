/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//----

/// Text printer built from partial blocks that can be replaced and are finally glued together
/// Inspired by Django blocks
class BM_CORE_PARSER_API TextSegmentedPrinter : public IFormatStream
{
public:
	TextSegmentedPrinter();
	virtual ~TextSegmentedPrinter();

	void clear();

	bool enterBlock(StringView name, bool redefinition); // returns false if it would be recursive
	bool exitBlock();

	bool appendSuperBlock();

	void print(IFormatStream& f) const;

	//--

	virtual IFormatStream& append(const char* str, uint32_t len = INDEX_MAX) override;

private:
	struct Block;

	// segment is either text or a reference to a block
	struct Segment
	{
		StringBuf block;
		Block* explicitBlock = nullptr;
		StringBuf txt;
	};

	// block is a named list of segments, block content can be redefined
	struct Block
	{
		Block* prev = nullptr;
		StringBuf name;
		Array<Segment*> segments;
	};

	Array<Segment*> m_allSegments;
	Array<Block*> m_allBlocks;
	Array<Block*> m_activeBlocks;
	Array<Block*> m_blockStack;

	UniquePtr<Block> m_root;
	UniquePtr<StringBuilder> m_printer;

	void pushTextToSegment();

	const Block* findBlock(const StringBuf& name) const;

	void printBlock(IFormatStream& f, const Block* b) const;
	void printSegment(IFormatStream& f, const Segment* s) const;
};

//----

END_INFERNO_NAMESPACE()
