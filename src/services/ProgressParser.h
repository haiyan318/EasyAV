#pragma once

// ffmpeg "-progress pipe:1" 流式解析（纯逻辑）。
// 输出是 key=value 行，可能被任意切分；Feed 负责拼行。
// out_time_ms 在 ffmpeg 中实际是微秒（历史命名问题），按微秒处理。

#include <cstddef>
#include <string>

namespace easyav::services
{
    class ProgressParser
    {
    public:
        explicit ProgressParser(double durationSec = 0.0);

        void Feed(const char* data, std::size_t size);
        void Reset(double durationSec);

        double Fraction() const;       // 时长未知返回 -1，否则 0..1
        double OutTimeSeconds() const;
        bool Ended() const;            // 收到 progress=end

    private:
        void ProcessLine(const std::string& line);

        double m_durationSec = 0.0;
        double m_outTimeSec = 0.0;
        bool m_ended = false;
        std::string m_buffer;
    };
}
