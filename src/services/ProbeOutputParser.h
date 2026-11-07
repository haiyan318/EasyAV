#pragma once

// ffprobe 文本输出解析（纯函数）。
// 命令行使用 -of default=noprint_wrappers=0，输出形如：
//   [STREAM]
//   codec_name=h264
//   codec_type=video
//   width=1920
//   height=1080
//   [/STREAM]
//   [FORMAT]
//   duration=12.345000
//   [/FORMAT]

#include "core/TaskContracts.h"

#include <string>

namespace easyav::services
{
    MediaInfo ParseFfprobeText(const std::string& textUtf8);
}
