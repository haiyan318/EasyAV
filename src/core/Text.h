#pragma once

// UTF-8 <-> wchar_t 与文件系统路径转换。
// 单独成文件，方便 model / platform 层复用，避免重复编码逻辑。

#include <filesystem>
#include <string>

namespace easyav
{
    std::wstring Utf8ToWide(const std::string& text);
    std::string WideToUtf8(const std::wstring& text);
    std::filesystem::path PathFromUtf8(const std::string& text);
    std::string PathToUtf8(const std::filesystem::path& path);
}
