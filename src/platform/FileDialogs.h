#pragma once

// 平台相关能力：文件/文件夹选择、在资源管理器中打开目录。
// UI 面板只发出 request 标志，由 App 调用这里，保持 UI 与平台层解耦。

#include <string>
#include <vector>

namespace easyav::platform
{
    std::vector<std::string> PickMediaFiles();
    std::string PickImageFile();
    std::string PickFolder();
    void OpenFolder(const std::string& utf8Path);
}
