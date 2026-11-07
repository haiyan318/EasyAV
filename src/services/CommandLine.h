#pragma once

// Windows 命令行构造（纯函数，便于单元测试）。
// 全项目只有这里负责引号与反斜杠转义；业务层始终使用参数数组。

#include <string>
#include <vector>

namespace easyav::services
{
    // 按 CommandLineToArgvW / CRT 规则给单个参数加引号。
    // 不含空格、制表符、双引号时原样返回。
    std::wstring QuoteArgument(const std::wstring& arg);

    // 把 UTF-8 参数数组拼成命令行（不含 exe 路径，用于测试/日志）。
    std::wstring BuildCommandLine(const std::vector<std::string>& argsUtf8);

    // 把 exe 与参数拼成可直接传给 CreateProcessW 的 lpCommandLine。
    std::wstring BuildCommandLine(const std::wstring& exePath,
                                  const std::vector<std::string>& argsUtf8);

    // Windows 命令行上限 32767 个字符（含结尾空字符）。
    bool CommandLineFits(const std::wstring& commandLine);
}
