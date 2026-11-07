#include "platform/AppPaths.h"

#include "core/Text.h"

#include <cstddef>
#include <filesystem>
#include <system_error>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace easyav::platform
{
#ifdef _WIN32
    namespace
    {
        std::wstring ModulePathW()
        {
            std::wstring buffer(MAX_PATH, L'\0');
            for (;;)
            {
                const DWORD length = GetModuleFileNameW(
                    nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
                if (length == 0)
                    return {};

                if (static_cast<std::size_t>(length) < buffer.size())
                {
                    buffer.resize(length);
                    return buffer;
                }

                // 返回长度等于缓冲区大小说明可能被截断，扩大后重试。
                if (buffer.size() >= 32768)
                    return {};
                buffer.resize(buffer.size() * 2);
            }
        }
    }
#endif

    std::string ExecutableDirUtf8()
    {
#ifdef _WIN32
        const std::wstring modulePath = ModulePathW();
        if (modulePath.empty())
            return {};
        return PathToUtf8(std::filesystem::path(modulePath).parent_path());
#else
        // 非 Windows 平台暂以工作目录代替，打包阶段再补 /proc/self/exe 等实现。
        return CurrentDirUtf8();
#endif
    }

    std::string CurrentDirUtf8()
    {
        std::error_code ec;
        const std::filesystem::path dir = std::filesystem::current_path(ec);
        if (ec)
            return {};
        return PathToUtf8(dir);
    }
}
