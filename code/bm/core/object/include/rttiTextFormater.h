/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#pragma once

BEGIN_INFERNO_NAMESPACE()

//--

/// helper class for nice printing of data structures to text
class BM_CORE_OBJECT_API IDataTextFormatter : public MainPoolData<NoCopy>
{
public:
	IDataTextFormatter(DataTextFormatterFlags flags = DataTextFormatterFlagBit::Relaxed);
	virtual ~IDataTextFormatter();

	//--

	virtual void beingArray(IFormatStream& f) = 0;
	virtual void endArray(IFormatStream& f) = 0;
	virtual void beingArrayElement(IFormatStream& f) = 0;
	virtual void endArrayElement(IFormatStream& f) = 0;

	virtual void beginStruct(IFormatStream& f) = 0;
	virtual void endStruct(IFormatStream& f) = 0;
	virtual void beginStructElement(IFormatStream& f, StringView name) = 0;
	virtual void endStructElement(IFormatStream& f, StringView name) = 0;

	virtual void beginValue(IFormatStream& f, Type type) = 0;
	virtual void endValue(IFormatStream& f, Type type) = 0;
	virtual void printValue(IFormatStream& f, StringView value) = 0;
	virtual void printBuffer(IFormatStream& f, const void* data, uint32_t size) = 0;

protected:
	DataTextFormatterFlags m_flags;
};

//--

/// text formatter 
class BM_CORE_OBJECT_API DataTextFormatter_JSON : public IDataTextFormatter
{
public:
	DataTextFormatter_JSON(DataTextFormatterFlags flags = DataTextFormatterFlagBit::Relaxed)
		: IDataTextFormatter(flags)
	{}

	virtual void beingArray(IFormatStream& f) override final;
	virtual void endArray(IFormatStream& f) override final;
	virtual void beingArrayElement(IFormatStream& f) override final;
	virtual void endArrayElement(IFormatStream& f) override final;

	virtual void beginStruct(IFormatStream& f) override final;
	virtual void endStruct(IFormatStream& f) override final;
	virtual void beginStructElement(IFormatStream& f, StringView name) override final;
	virtual void endStructElement(IFormatStream& f, StringView name) override final;

	virtual void beginValue(IFormatStream& f, Type type) override final;
	virtual void endValue(IFormatStream& f, Type type) override final;
	virtual void printValue(IFormatStream& f, StringView value) override final;
	virtual void printBuffer(IFormatStream& f, const void* data, uint32_t size) override final;

private:
	InplaceArray<bool, 10> m_needsSeparator;
	bool m_needsNewLine = false;
	int m_indentation = 0; // internal indentation counter

	void makeNewLine(IFormatStream& f);
	void makeNewLineIfNeeded(IFormatStream& f);
	void makeElementSeparator(IFormatStream& f);
	void makeBlockStart(IFormatStream& f, const char* ch);
	void makeBlockEnd(IFormatStream& f, const char* ch);
};

//--

END_INFERNO_NAMESPACE()