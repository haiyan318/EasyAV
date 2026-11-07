#pragma once

// 轻量文件日志：仅服务层使用，界面从不直接调用。
// 日志失败必须静默，不能影响主流程；所有方法线程安全。
//
// 文件位置：<baseDir>/logs/easyav.log，超过 2MB 轮转为 easyav.log.1。

#include <string>
#include <vector>

namespace easyav::services
{
    class Logger
    {
    public:
        // baseDirUtf8 为空或目录不可写时自动降级为"不记录"，不抛异常。
        static void Init(const std::string& baseDirUtf8);
        static void Shutdown();

        static void Info(const std::string& messageUtf8);
        static void Warn(const std::string& messageUtf8);
        static void Error(const std::string& messageUtf8);

        // 记录一次子进程调用：exe 一行，之后每个参数单独一行，避免引号歧义。
        static void Command(const std::wstring& exePath, const std::vector<std::string>& argsUtf8);
    };
}
