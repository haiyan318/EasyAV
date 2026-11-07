#include "core/Format.h"

#include <cctype>
#include <cstdio>

namespace easyav
{
    std::string FormatBytes(unsigned long long bytes)
    {
        static const char* kUnits[] = { "B", "KB", "MB", "GB", "TB" };
        double value = static_cast<double>(bytes);
        int unit = 0;
        while (value >= 1024.0 && unit < 4)
        {
            value /= 1024.0;
            ++unit;
        }

        char buffer[64] = {};
        if (unit == 0)
            std::snprintf(buffer, sizeof(buffer), "%llu B", bytes);
        else
            std::snprintf(buffer, sizeof(buffer), "%.1f %s", value, kUnits[unit]);
        return buffer;
    }

    std::string FormatClock(double seconds)
    {
        if (seconds < 0.0)
            return "--:--";

        const int total = static_cast<int>(seconds + 0.5);
        const int hours = total / 3600;
        const int minutes = (total % 3600) / 60;
        const int secs = total % 60;

        char buffer[32] = {};
        if (hours > 0)
            std::snprintf(buffer, sizeof(buffer), "%d:%02d:%02d", hours, minutes, secs);
        else
            std::snprintf(buffer, sizeof(buffer), "%02d:%02d", minutes, secs);
        return buffer;
    }

    std::string FormatDurationLabel(double seconds)
    {
        return seconds > 0.0 ? FormatClock(seconds) : std::string("--:--");
    }

    std::string FormatPercent(float value01)
    {
        const int percent = static_cast<int>(value01 * 100.0f + 0.5f);
        char buffer[16] = {};
        std::snprintf(buffer, sizeof(buffer), "%d%%", percent);
        return buffer;
    }

    std::string ToUpperAscii(std::string value)
    {
        for (char& ch : value)
            ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
        return value;
    }

    std::string ToLowerAscii(std::string value)
    {
        for (char& ch : value)
            ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        return value;
    }

    std::string FileNameOf(const std::string& path)
    {
        const std::size_t pos = path.find_last_of("/\\");
        return pos == std::string::npos ? path : path.substr(pos + 1);
    }

    std::string ExtensionOf(const std::string& path)
    {
        const std::string name = FileNameOf(path);
        const std::size_t dot = name.find_last_of('.');
        if (dot == std::string::npos || dot + 1 >= name.size())
            return {};
        return name.substr(dot + 1);
    }
}
