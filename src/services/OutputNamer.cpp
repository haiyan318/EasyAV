#include "services/OutputNamer.h"

#include "core/Text.h"

#include <cctype>
#include <cstddef>
#include <filesystem>
#include <string>
#include <utility>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace easyav::services
{
    namespace
    {
        namespace fs = std::filesystem;

        constexpr std::size_t kMaxPathChars = 240;
        constexpr std::size_t kMaxSuffixChars = 40;

        bool IsIllegalFileNameChar(unsigned char c)
        {
            if (c < 0x20)
                return true;
            switch (c)
            {
            case '<':
            case '>':
            case ':':
            case '"':
            case '/':
            case '\\':
            case '|':
            case '?':
            case '*':
                return true;
            default:
                return false;
            }
        }

        std::size_t Utf8SequenceLength(unsigned char lead)
        {
            if ((lead & 0x80) == 0)
                return 1;
            if ((lead & 0xE0) == 0xC0)
                return 2;
            if ((lead & 0xF0) == 0xE0)
                return 3;
            if ((lead & 0xF8) == 0xF0)
                return 4;
            return 1;
        }

        std::string TrimTrailingDotsAndSpaces(std::string text)
        {
            while (!text.empty() && (text.back() == ' ' || text.back() == '.'))
                text.pop_back();
            return text;
        }

        std::string ComparisonKey(const fs::path& path)
        {
            // 统一分隔符后再比较：用户输入可能带正斜杠，组合出的路径用反斜杠。
            std::string text = PathToUtf8(path.lexically_normal());
            for (char& ch : text)
            {
                if (ch == '/')
                    ch = '\\';
            }
            return text;
        }

        bool EqualsIgnoreCaseAscii(const std::string& a, const std::string& b)
        {
            if (a.size() != b.size())
                return false;
            for (std::size_t i = 0; i < a.size(); ++i)
            {
                const unsigned char ca = static_cast<unsigned char>(a[i]);
                const unsigned char cb = static_cast<unsigned char>(b[i]);
                if (std::tolower(ca) != std::tolower(cb))
                    return false;
            }
            return true;
        }

        bool TooLong(const fs::path& path)
        {
            const std::wstring wide = path.wstring();
            return wide.size() > kMaxPathChars;
        }

        unsigned CurrentProcessId()
        {
#ifdef _WIN32
            return static_cast<unsigned>(GetCurrentProcessId());
#else
            return 0u;
#endif
        }
    }

    std::string SanitizeSuffix(const std::string& suffixUtf8)
    {
        std::string sanitized;
        sanitized.reserve(suffixUtf8.size());

        for (const char ch : suffixUtf8)
        {
            const unsigned char c = static_cast<unsigned char>(ch);
            sanitized.push_back(IsIllegalFileNameChar(c) ? '_' : ch);
        }

        sanitized = TrimTrailingDotsAndSpaces(std::move(sanitized));

        // 按 UTF-8 码点截断，避免切坏中文。
        std::size_t cut = 0;
        std::size_t count = 0;
        while (cut < sanitized.size() && count < kMaxSuffixChars)
        {
            const std::size_t length =
                Utf8SequenceLength(static_cast<unsigned char>(sanitized[cut]));
            if (cut + length > sanitized.size())
                break;
            cut += length;
            ++count;
        }
        sanitized.resize(cut);

        return sanitized;
    }

    std::string ExtensionFor(TargetFormat format)
    {
        switch (format)
        {
        case TargetFormat::MKV: return "mkv";
        case TargetFormat::MP3: return "mp3";
        case TargetFormat::GIF: return "gif";
        case TargetFormat::MP4:
        default: return "mp4";
        }
    }

    NamingResult Resolve(const NamingRequest& request,
                         const std::function<bool(const std::string&)>& exists,
                         unsigned tempSerial)
    {
        NamingResult result;

        if (request.sourcePath.empty())
        {
            result.errorText = "找不到原文件";
            return result;
        }

        const fs::path source = PathFromUtf8(request.sourcePath);
        const fs::path directory =
            request.outputDir.empty() ? source.parent_path()
                                      : PathFromUtf8(request.outputDir);

        if (directory.empty())
        {
            result.errorText = "无法确定输出位置，请在设置里选择文件夹";
            return result;
        }

        const std::string stem = PathToUtf8(source.stem());
        const std::string suffix = SanitizeSuffix(request.suffix);
        const std::string extension = ExtensionFor(request.format);
        const std::string sourceKey = ComparisonKey(source);

        const auto fileExists = [&exists](const fs::path& path) {
            const std::string text = PathToUtf8(path);
            return exists ? exists(text) : fs::exists(path);
        };

        std::string base = stem + suffix;
        fs::path candidate = directory / PathFromUtf8(base + "." + extension);

        if (EqualsIgnoreCaseAscii(ComparisonKey(candidate), sourceKey))
        {
            base += "_1";
            candidate = directory / PathFromUtf8(base + "." + extension);
        }

        constexpr int kMaxAttempts = 999;
        int attempt = 1;
        while (fileExists(candidate))
        {
            if (attempt > kMaxAttempts)
            {
                result.errorText = "同名文件太多，请换一个输出位置";
                return result;
            }

            base = stem + suffix + " (" + std::to_string(attempt) + ")";
            candidate = directory / PathFromUtf8(base + "." + extension);
            ++attempt;
        }

        if (TooLong(candidate))
        {
            result.errorText = "文件路径太长，请把文件移到更短的文件夹";
            return result;
        }

        const std::string tempName =
            ".easyav-tmp-" + std::to_string(CurrentProcessId()) + "-"
            + std::to_string(tempSerial) + "." + extension;

        result.finalPath = PathToUtf8(candidate);
        result.tempPath = PathToUtf8(directory / PathFromUtf8(tempName));
        result.ok = true;
        return result;
    }
}
