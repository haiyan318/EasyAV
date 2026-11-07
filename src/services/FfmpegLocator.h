#pragma once

// 查找 ffmpeg/ffprobe 并校验可运行。
// 顺序：%EASYAV_FFMPEG_DIR% -> 系统 PATH -> 内置 assets/ffmpeg -> exe 同目录 -> 当前工作目录。
// 每个候选都会执行 "-version" 校验，损坏的安装会被跳过。

#include "core/TaskContracts.h"

#include <string>

namespace easyav::services
{
    struct LocatedTools
    {
        ToolStatus status;
        std::wstring ffmpegExeW;
        std::wstring ffprobeExeW;
    };

    class FfmpegLocator
    {
    public:
        static LocatedTools Locate(const std::string& exeDirUtf8, const std::string& cwdUtf8);
    };
}
