#include <Arduino.h>
#include <time.h>

#include "logger.h"

Logger Log;

// Before NTP has run the clock counts up from 1970, so anything earlier than
// this means "not set yet"
static constexpr time_t CLOCK_VALID_AFTER = 1700000000;

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
    for (size_t i = 0; i < size; i++)
    {
        char c = (char)buffer[i];

        // Stamp a line when its first character arrives; blank lines stay blank
        if (m_atLineStart && c != '\r' && c != '\n')
        {
            m_atLineStart = false;
            writeTimestamp();
        }

        put(c);
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

void Logger::put(char c)
{
    if (m_out != nullptr)
    {
        m_out->write((uint8_t)c);
    }

    if (c == '\r')
    {
        return;
    }

    if (c == '\n')
    {
        commitLine();
        m_atLineStart = true;
        return;
    }

    m_current[m_currentLength++] = c;

    // Leave room for the terminator; an overlong line just wraps.
    if (m_currentLength == LOG_LINE_LENGTH - 1)
    {
        commitLine();
    }
}

void Logger::writeTimestamp()
{
    char text[16];
    time_t now = time(nullptr);

    if (now > CLOCK_VALID_AFTER)
    {
        struct tm timeinfo {};
        localtime_r(&now, &timeinfo);
        strftime(text, sizeof(text), "<%H:%M:%S> ", &timeinfo);
    }
    else
    {
        unsigned long seconds = millis() / 1000;
        snprintf(text, sizeof(text), "<+%02lu:%02lu:%02lu> ",
            seconds / 3600, seconds / 60 % 60, seconds % 60);   
    }

    for (const char* p = text; *p != '\0'; p++)
    {
        put(*p);
    }
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
