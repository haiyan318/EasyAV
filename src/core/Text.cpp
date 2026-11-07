#include "core/Text.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace easyav
{
    std::wstring Utf8ToWide(const std::string& text)
    {
#ifdef _WIN32
        if (text.empty())
            return {};

        const int length = MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0);
        if (length <= 0)
            return {};

        std::wstring result(static_cast<std::size_t>(length), L'\0');
        MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), result.data(), length);
        return result;
#else
        return std::wstring(text.begin(), text.end());
#endif
    }

    std::string WideToUtf8(const std::wstring& text)
    {
#ifdef _WIN32
        if (text.empty())
            return {};

        const int length = WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()),
                                               nullptr, 0, nullptr, nullptr);
        if (length <= 0)
            return {};

        std::string result(static_cast<std::size_t>(length), '\0');
        WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()),
                            result.data(), length, nullptr, nullptr);
        return result;
#else
        return std::string(text.begin(), text.end());
#endif
    }

    std::filesystem::path PathFromUtf8(const std::string& text)
    {
#ifdef _WIN32
        return std::filesystem::path(Utf8ToWide(text));
#else
        return std::filesystem::u8path(text);
#endif
    }

    std::string PathToUtf8(const std::filesystem::path& path)
    {
#ifdef _WIN32
        return WideToUtf8(path.wstring());
#else
        return path.u8string();
#endif
    }
}
