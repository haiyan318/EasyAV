#pragma once

// 输出命名策略（纯逻辑，exists 由调用方注入，便于单测）。
// 规则：
// - 最终名 = 原文件名（去扩展名） + 后缀 + 新扩展名
// - 后缀清洗非法字符，结尾的点和空格去掉
// - 与源文件同名 / 已存在时追加 "_1" 或 " (n)"
// - 临时文件与最终文件同目录，保证可原子重命名

#include "core/Types.h"

#include <functional>
#include <string>

namespace easyav::services
{
    struct NamingRequest
    {
        std::string sourcePath; // UTF-8 绝对路径
        std::string outputDir;  // UTF-8；空表示与原文件同目录
        std::string suffix;     // UTF-8，可为空
        TargetFormat format = TargetFormat::MP4;
    };

    struct NamingResult
    {
        bool ok = false;
        std::string finalPath;
        std::string tempPath;
        std::string errorText; // 中文，可直接提示
    };

    std::string SanitizeSuffix(const std::string& suffixUtf8);
    std::string ExtensionFor(TargetFormat format);

    NamingResult Resolve(const NamingRequest& request,
                         const std::function<bool(const std::string&)>& exists,
                         unsigned tempSerial);
}
