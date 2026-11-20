#include <Arduino.h>

#include "logger.h"

Logger Log;

void Logger::init(Print& out)
{
    m_out = &out;
}

size_t Logger::write(uint8_t c)
{
    return write(&c, 1);
}

size_t Logger::write(const uint8_t* buffer, size_t size)
{
    if (m_out != nullptr)
    {
        m_out->write(buffer, size);
    }

    for (size_t i = 0; i < size; i++)
    {
        char c = (char)buffer[i];

        if (c == '\r')
        {
            continue;
        }

        if (c == '\n')
        {
            commitLine();
            continue;
        }

        m_current[m_currentLength++] = c;

        // Leave room for the terminator; an overlong line just wraps.
        if (m_currentLength == LOG_LINE_LENGTH - 1)
        {
            commitLine();
        }
    }

    return size;
}

size_t Logger::lineCount() const
{
    return m_count;
}

const char* Logger::line(size_t index) const
{
    if (index >= m_count)
    {
        return "";
    }

    return m_lines[(m_oldest + index) % LOG_LINE_COUNT];
}

uint32_t Logger::totalLines() const
{
    return m_totalLines;
}

void Logger::commitLine()
{
    m_current[m_currentLength] = '\0';

    size_t slot = (m_oldest + m_count) % LOG_LINE_COUNT;
    memcpy(m_lines[slot], m_current, m_currentLength + 1);

    if (m_count < LOG_LINE_COUNT)
    {
        m_count++;
    }
    else
    {
        m_oldest = (m_oldest + 1) % LOG_LINE_COUNT;
    }

    m_currentLength = 0;
    m_totalLines++;
}
