#pragma once

#include <Arduino.h>

#include "config.h"

/*
 * Everything the firmware logs goes through Log rather than Serial. It passes
 * each line straight on to the output (USB serial) and also keeps the most
 * recent LOG_LINE_COUNT lines in memory, so they can be read back later,
 * e.g. by a web page.
 *
 * Each line starts with the local time once NTP has set the clock, or with
 * the uptime (+hh:mm:ss) before that.
 */

class Logger : public Print
{
public:
    void init(Print& out);

    size_t write(uint8_t c) override;
    size_t write(const uint8_t* buffer, size_t size) override;

    size_t      lineCount() const;
    const char* line(size_t index) const;
    uint32_t    totalLines() const;

private:
    void put(char c);
    void writeTimestamp();
    void commitLine();

    Print* m_out = nullptr;

    char   m_lines[LOG_LINE_COUNT][LOG_LINE_LENGTH] = {};
    size_t m_oldest = 0;
    size_t m_count  = 0;

    char   m_current[LOG_LINE_LENGTH] = {};
    size_t m_currentLength = 0;

    uint32_t m_totalLines  = 0;
    bool     m_atLineStart = true;
};

extern Logger Log;
