#pragma once

// 媒体探测端口：读取时长/编码/分辨率。
// 实现可以是 ffprobe 子进程，也可以换成进程内 libavformat，上层不感知。

#include "core/TaskContracts.h"

#include <string>

namespace easyav::services
{
    class IMediaProbe
    {
    public:
        virtual ~IMediaProbe() = default;

        // 失败时 MediaInfo::ok = false，errorText 仅用于日志。
        virtual MediaInfo Probe(const std::string& utf8Path) = 0;
    };
}
