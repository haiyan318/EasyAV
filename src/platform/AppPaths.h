#pragma once

// 运行位置查询：进程所在目录与当前工作目录。
// Win32 调用集中在此，对外只暴露 UTF-8 路径，服务层不直接包含 windows.h。

#include <string>

namespace easyav::platform
{
    // 返回可执行文件所在目录；取不到时返回空字符串。
    std::string ExecutableDirUtf8();

    // 返回当前工作目录；取不到时返回空字符串。
    std::string CurrentDirUtf8();
}
