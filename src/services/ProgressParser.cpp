#include "services/ProgressParser.h"

#include <algorithm>
#include <cstdlib>

namespace easyav::services
{
    namespace
    {
        bool ParseDouble(const std::string& text, double* value)
        {
            if (text.empty() || text == "N/A")
                return false;

            char* end = nullptr;
            const double parsed = std::strtod(text.c_str(), &end);
            if (end == text.c_str() || *end != '\0')
                return false;

            *value = parsed;
            return true;
        }

        bool ParseClock(const std::string& text, double* seconds)
        {
            // HH:MM:SS.ffffff
            const std::size_t first = text.find(':');
            const std::size_t second =
                first == std::string::npos ? std::string::npos : text.find(':', first + 1);
            if (first == std::string::npos || second == std::string::npos)
                return false;

            double hours = 0.0;
            double minutes = 0.0;
            double secs = 0.0;
            if (!ParseDouble(text.substr(0, first), &hours)
                || !ParseDouble(text.substr(first + 1, second - first - 1), &minutes)
                || !ParseDouble(text.substr(second + 1), &secs))
            {
                return false;
            }

            *seconds = hours * 3600.0 + minutes * 60.0 + secs;
            return true;
        }

        bool ParseTimeValue(const std::string& text, double* seconds)
        {
            if (text.find(':') != std::string::npos)
                return ParseClock(text, seconds);
            return ParseDouble(text, seconds);
        }
    }

    ProgressParser::ProgressParser(double durationSec)
        : m_durationSec(std::max(durationSec, 0.0))
    {
    }

    void ProgressParser::Reset(double durationSec)
    {
        m_durationSec = std::max(durationSec, 0.0);
        m_outTimeSec = 0.0;
        m_ended = false;
        m_buffer.clear();
    }

    void ProgressParser::Feed(const char* data, std::size_t size)
    {
        if (data == nullptr || size == 0)
            return;

        m_buffer.append(data, size);

        std::size_t start = 0;
        for (;;)
        {
            const std::size_t newline = m_buffer.find('\n', start);
            if (newline == std::string::npos)
                break;

            ProcessLine(m_buffer.substr(start, newline - start));
            start = newline + 1;
        }

        if (start > 0)
            m_buffer.erase(0, start);

        // 防御异常输出：保留最后 64KB，避免无限增长。
        constexpr std::size_t kMaxPending = 64u * 1024u;
        if (m_buffer.size() > kMaxPending)
            m_buffer.erase(0, m_buffer.size() - kMaxPending);
    }

    void ProgressParser::ProcessLine(const std::string& rawLine)
    {
        std::string line = rawLine;
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        if (line.empty())
            return;

        const std::size_t equals = line.find('=');
        if (equals == std::string::npos)
            return;

        const std::string key = line.substr(0, equals);
        const std::string value = line.substr(equals + 1);

        if (key == "out_time_us")
        {
            double textValue = 0.0;
            if (ParseDouble(value, &textValue))
                m_outTimeSec = std::max(textValue / 1000000.0, 0.0);
            return;
        }

        if (key == "out_time_ms")
        {
            // ffmpeg 的 out_time_ms 实为微秒。
            double textValue = 0.0;
            if (ParseDouble(value, &textValue))
                m_outTimeSec = std::max(textValue / 1000000.0, 0.0);
            return;
        }

        if (key == "out_time")
        {
            double textValue = 0.0;
            if (ParseTimeValue(value, &textValue))
                m_outTimeSec = std::max(textValue, 0.0);
            return;
        }

        if (key == "progress")
        {
            m_ended = (value == "end");
        }
    }

    double ProgressParser::Fraction() const
    {
        if (m_durationSec <= 0.0)
            return -1.0;

        return std::clamp(m_outTimeSec / m_durationSec, 0.0, 1.0);
    }

    double ProgressParser::OutTimeSeconds() const
    {
        return m_outTimeSec;
    }

    bool ProgressParser::Ended() const
    {
        return m_ended;
    }
}
