/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"
#include "outputStream.h"
#include "thread.h"
#include <assert.h>

BEGIN_INFERNO_NAMESPACE()

//---

void SinkTable::attach(ILogSink* sink)
{
    if (sink)
    {
        auto lock = CreateLock(m_lock);
        if (m_numSinks < MAX_SINKS)
        {
            m_sinks[m_numSinks] = sink;
            m_numSinks += 1;
        }
    }
}

void SinkTable::detach(ILogSink* sink)
{
    auto lock = CreateLock(m_lock);
    for (uint32_t i = 0; i < m_numSinks; ++i)
    {
        if (m_sinks[i] == sink)
        {
            m_sinks[i] = m_sinks[m_numSinks - 1];
            m_sinks[m_numSinks - 1] = nullptr;
            m_numSinks -= 1;
            break;
        }
    }
}

void SinkTable::print(LogOutputLevel level, const char* file, uint32_t line, const char* text)
{
    auto lock = CreateLock(m_lock);
    for (uint32_t i = 0; i < m_numSinks; ++i)
        if (m_sinks[i]->print(level, file, line, text))
            break;
}

SinkTable& SinkTable::GetInstance()
{
    static SinkTable* theSinkTable = new SinkTable();
    return *theSinkTable;
}

//---

LineBuffer::LineBuffer()
    : m_index(0)
{}

void LineBuffer::clear()
{
    m_index = 0;
}

bool LineBuffer::append(const char*& str, const char* endStr)
{
    // write shit
    while (str < endStr)
    {
        // end of line ?
        if (*str == '\r' || *str == '\n')
        {
            m_line[m_index++] = 0; // eat the line endings, they are added in the log sinks (if needed)

            if (*str == '\r' && str[1] == '\n')
                str += 2;
            else if (*str == '\n' && str[1] == '\r')
                str += 2;
            else
                str += 1;

            return true; // line was captured
        }

        // to much data, print ...
        if (m_index == MAX_LINE_LENGTH)
        {
            m_line[m_index++] = '.';
            m_line[m_index++] = '.';
            m_line[m_index++] = '.';
            str += 1;
            continue;
        }

        // add to line
        if (m_index < MAX_LINE_LENGTH)
            m_line[m_index++] = *str;

        // advance input stream
        str += 1;
    }

    // no line signaled
    return false;
}

//---

char LineAssembler::PREFIX_BUFFER[64];

LineAssembler::LineAssembler()
{
    memset(PREFIX_BUFFER, 0, sizeof(PREFIX_BUFFER));
    PREFIX_BUFFER[ARRAY_COUNT(PREFIX_BUFFER) - 1] = 0;
}

ILogSink* LineAssembler::mountLocalSink(ILogSink* localSink)
{
    auto old = m_localSink;
    m_localSink = localSink;
    return old;
}

ILogSink* LineAssembler::localSink()
{
    return m_localSink;
}

void LineAssembler::takeOwnership(LogOutputLevel level, const char* contextFile, uint32_t contextLine)
{
    /*auto prevOwningThread = m_owningThread.exchange(GetCurrentThreadID());
    assert(prevOwningThread == 0);*/

    m_currentLevel = level;
    m_curentFile = contextFile;
    m_currentLine = contextLine;
}

void LineAssembler::releaseOwnership()
{
    /*auto prevOwningThread = m_owningThread.exchange(0);
    assert(prevOwningThread == GetCurrentThreadID());*/
}

IFormatStream& LineAssembler::append(const char* str, uint32_t len)
{
    if (len && str && *str)
    {
        int prefixSize = -1;

        auto end = (len == INDEX_MAX) ? (str + strlen(str)) : (str + len);
        while (str < end)
        {
            if (m_line.empty() && prefixSize > 0)
            {
                const char* prefixStr = PREFIX_BUFFER;
                m_line.append(prefixStr, prefixStr + prefixSize);
            }

            if (m_line.append(str, end))
            {
                if (prefixSize == -1)
                {
                    const auto* str = m_line.c_str();
                    const auto* strMax = str + ARRAY_COUNT(PREFIX_BUFFER);
                    while (*str && *str <= ' ' && str < strMax)
                        ++str;
                    prefixSize = str - m_line.c_str();
                }

                // let the local sink consume it first
                bool consumed = false;
                if (m_localSink)
                    consumed = m_localSink->print(m_currentLevel, m_curentFile, m_currentLine, m_line.c_str());

                // if it was not consumed by the local sink pass to to global ones
                if (!consumed)
                    SinkTable::GetInstance().print(m_currentLevel, m_curentFile, m_currentLine, m_line.c_str());

                m_line.clear();
            }
        }
    }

    return *this;
}

//---

END_INFERNO_NAMESPACE()
