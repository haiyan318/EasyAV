#pragma once

// 与业务无关的格式化工具。禁止在此引用 ImGui。

#include <string>

namespace easyav
{
    std::string FormatBytes(unsigned long long bytes);
    std::string FormatClock(double seconds);         // 01:23 / 1:02:03
    std::string FormatDurationLabel(double seconds); // 01:23 / --:--
    std::string FormatPercent(float value01);        // 12%
    std::string ToUpperAscii(std::string value);
    std::string ToLowerAscii(std::string value);
    std::string FileNameOf(const std::string& path);
    std::string ExtensionOf(const std::string& path);
}
