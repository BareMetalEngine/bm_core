/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "bm/core/system/include/output.h"

#undef TRACE_INFO
#define TRACE_INFO( x, ... ) TRACE_STREAM_INFO().appendf(x, ##__VA_ARGS__).append("\n");

BEGIN_INFERNO_NAMESPACE()

//--

class LocalSink : public ILogSink
{
public:
	std::vector<std::string> m_linesInfo;
	std::vector<std::string> m_linesError;
	std::vector<std::string> m_linesWarn;
	bool m_shouldKill = false;

	LocalSink()
	{
		Log::AttachGlobalSink(this);
	}

	~LocalSink()
	{
		Log::DetachGlobalSink(this);
	}

	virtual bool print(LogOutputLevel level, const char* file, uint32_t line, const char* text) override
	{
		if (level == LogOutputLevel::Error)
			m_linesError.push_back(text);
		else if (level == LogOutputLevel::Warning)
			m_linesWarn.push_back(text);
		else if (level == LogOutputLevel::Info)
			m_linesInfo.push_back(text);

		return m_shouldKill;
	}
};

//--

TEST(Output, SinkRoutesInfo)
{
	LocalSink sink;
	TRACE_INFO("This is text");
	ASSERT_EQ(1, sink.m_linesInfo.size());
	ASSERT_EQ(0, sink.m_linesError.size());
	ASSERT_EQ(0, sink.m_linesWarn.size());
}

TEST(Output, SinkRoutesError)
{
	LocalSink sink;
	TRACE_ERROR("This is text");
	ASSERT_EQ(0, sink.m_linesInfo.size());
	ASSERT_EQ(1, sink.m_linesError.size());
	ASSERT_EQ(0, sink.m_linesWarn.size());
}

TEST(Output, SinkRoutesWarning)
{
	LocalSink sink;
	TRACE_WARNING("This is text");
	ASSERT_EQ(0, sink.m_linesInfo.size());
	ASSERT_EQ(0, sink.m_linesError.size());
	ASSERT_EQ(1, sink.m_linesWarn.size());
}

TEST(Output, SinkTextMatchesLineInfo)
{
	LocalSink sink;
	TRACE_INFO("This is info");
	ASSERT_EQ(1, sink.m_linesInfo.size());
	EXPECT_STREQ("This is info", sink.m_linesInfo[0].c_str());
}

TEST(Output, SinkTextMatchesLineWarning)
{
	LocalSink sink;
	TRACE_WARNING("This is warning");
	ASSERT_EQ(1, sink.m_linesWarn.size());
	EXPECT_STREQ("This is warning", sink.m_linesWarn[0].c_str());
}

TEST(Output, SinkTextMatchesLineError)
{
	LocalSink sink;
	TRACE_ERROR("This is NOT and error, just log, don't panic");
	ASSERT_EQ(1, sink.m_linesError.size());
	EXPECT_STREQ("This is NOT and error, just log, don't panic", sink.m_linesError[0].c_str());
}

TEST(Output, SinkNoNewLine)
{
	LocalSink sink;
	TRACE_INFO("This is text");
	ASSERT_EQ(1, sink.m_linesInfo.size());
}

TEST(Output, SinkHandlesLineSplitting)
{
	LocalSink sink;
	TRACE_INFO("This is text\nand another line");
	ASSERT_EQ(2, sink.m_linesInfo.size());
}

//--

END_INFERNO_NAMESPACE()