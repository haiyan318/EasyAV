#include "services/ProbeOutputParser.h"

#include <cstdlib>

namespace easyav::services
{
    namespace
    {
        double ParseDouble(const std::string& text)
        {
            if (text.empty() || text == "N/A")
                return 0.0;

            char* end = nullptr;
            const double value = std::strtod(text.c_str(), &end);
            if (end == text.c_str() || value < 0.0)
                return 0.0;
            return value;
        }

        int ParseInt(const std::string& text)
        {
            if (text.empty() || text == "N/A")
                return 0;

            char* end = nullptr;
            const long value = std::strtol(text.c_str(), &end, 10);
            if (end == text.c_str() || value < 0)
                return 0;
            return static_cast<int>(value);
        }
    }

    MediaInfo ParseFfprobeText(const std::string& textUtf8)
    {
        MediaInfo info;

        enum class Section
        {
            None,
            Stream,
            Format
        };

        Section section = Section::None;

        // 一个 stream 的字段可能按任意顺序出现，先收集，遇到 [/STREAM] 再归档。
        bool inStream = false;
        std::string streamType;
        std::string streamCodec;
        int streamWidth = 0;
        int streamHeight = 0;

        std::size_t pos = 0;
        while (pos < textUtf8.size())
        {
            std::size_t lineEnd = textUtf8.find('\n', pos);
            if (lineEnd == std::string::npos)
                lineEnd = textUtf8.size();

            std::string line = textUtf8.substr(pos, lineEnd - pos);
            pos = lineEnd + 1;

            if (!line.empty() && line.back() == '\r')
                line.pop_back();

            if (line.empty())
                continue;

            if (line == "[STREAM]")
            {
                section = Section::Stream;
                inStream = true;
                streamType.clear();
                streamCodec.clear();
                streamWidth = 0;
                streamHeight = 0;
                continue;
            }

            if (line == "[/STREAM]")
            {
                if (inStream)
                {
                    if (streamType == "video" && !info.hasVideo)
                    {
                        info.hasVideo = true;
                        info.videoCodec = streamCodec;
                        info.width = streamWidth;
                        info.height = streamHeight;
                    }
                    else if (streamType == "audio" && !info.hasAudio)
                    {
                        info.hasAudio = true;
                        info.audioCodec = streamCodec;
                    }
                }
                inStream = false;
                section = Section::None;
                continue;
            }

            if (line == "[FORMAT]")
            {
                section = Section::Format;
                continue;
            }

            if (line == "[/FORMAT]")
            {
                section = Section::None;
                continue;
            }

            const std::size_t equals = line.find('=');
            if (equals == std::string::npos)
                continue;

            const std::string key = line.substr(0, equals);
            const std::string value = line.substr(equals + 1);

            if (section == Section::Format)
            {
                if (key == "duration")
                    info.durationSec = ParseDouble(value);
                continue;
            }

            if (section == Section::Stream)
            {
                if (key == "codec_type")
                    streamType = value;
                else if (key == "codec_name")
                    streamCodec = value;
                else if (key == "width")
                    streamWidth = ParseInt(value);
                else if (key == "height")
                    streamHeight = ParseInt(value);
            }
        }

        info.ok = info.hasVideo || info.hasAudio;
        return info;
    }
}
