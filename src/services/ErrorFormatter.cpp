#include "services/ErrorFormatter.h"

#include <cctype>

namespace easyav::services
{
    namespace
    {
        std::string ToLowerAscii(std::string text)
        {
            for (char& ch : text)
            {
                const unsigned char c = static_cast<unsigned char>(ch);
                if (c < 0x80)
                    ch = static_cast<char>(std::tolower(c));
            }
            return text;
        }

        bool Contains(const std::string& haystack, const char* needle)
        {
            return haystack.find(needle) != std::string::npos;
        }

        std::string TrimEnd(std::string text)
        {
            while (!text.empty()
                   && (text.back() == ' ' || text.back() == '\t'
                       || text.back() == '\r' || text.back() == '\n'))
            {
                text.pop_back();
            }
            return text;
        }
    }

    FailureKind ClassifyFailure(int exitCode, const std::string& stderrText, bool outputEmpty)
    {
        const std::string text = ToLowerAscii(stderrText);

        if (Contains(text, "no such file or directory")
            || Contains(text, "could not open input"))
        {
            return FailureKind::InputMissing;
        }

        if (Contains(text, "permission denied")
            || Contains(text, "access is denied")
            || Contains(text, "error opening output"))
        {
            return FailureKind::PermissionDenied;
        }

        if (Contains(text, "no space left on device")
            || Contains(text, "not enough space"))
        {
            return FailureKind::DiskFull;
        }

        if (Contains(text, "filename too long")
            || Contains(text, "path too long")
            || Contains(text, "result too large"))
        {
            return FailureKind::PathTooLong;
        }

        if (Contains(text, "invalid data found")
            || Contains(text, "moov atom not found")
            || Contains(text, "does not contain any stream")
            || Contains(text, "error while decoding"))
        {
            return FailureKind::InputCorrupt;
        }

        if (Contains(text, "unknown encoder")
            || Contains(text, "encoder not found")
            || Contains(text, "cannot load")
            || Contains(text, "encoder '"))
        {
            return FailureKind::EncoderUnavailable;
        }

        if (outputEmpty)
            return FailureKind::EmptyOutput;

        // Windows 异常退出码，例如 0xC0000005。
        if ((static_cast<unsigned int>(exitCode) & 0xC0000000u) == 0xC0000000u)
            return FailureKind::Unknown;

        return FailureKind::Unknown;
    }

    std::string UserMessage(FailureKind kind, const std::string& fileNameUtf8,
                            VideoCodec codec, bool hardwareAttempt)
    {
        std::string message;
        switch (kind)
        {
        case FailureKind::InputMissing:
            message = "找不到这个文件，可能已被移动或删除";
            break;
        case FailureKind::InputCorrupt:
            message = "这个文件打不开，可能已损坏";
            break;
        case FailureKind::PermissionDenied:
            message = "没有权限写入这个文件夹，请在设置里换一个位置";
            break;
        case FailureKind::DiskFull:
            message = "磁盘空间不足，清理一些文件后再试";
            break;
        case FailureKind::UnsupportedCodec:
            message = "这个文件的编码暂不支持";
            break;
        case FailureKind::PathTooLong:
            message = "文件路径太长，请把文件移到更短的文件夹";
            break;
        case FailureKind::OutputLocked:
            message = "输出文件正被其他程序占用，请关闭后重试";
            break;
        case FailureKind::EncoderUnavailable:
            message = hardwareAttempt
                          ? "这台电脑的显卡加速不可用，请关闭加速后再试"
                          : "视频编码器不可用，请重新放置处理组件后重试";
            break;
        case FailureKind::CodecUnavailable:
            message = codec == VideoCodec::H265
                          ? "这台电脑的处理组件不含 H.265 编码，请改用 H.264（兼容性也更好）"
                          : "处理组件缺少视频编码器，请重新放置 ffmpeg 或使用完整版本";
            break;
        case FailureKind::ProcessStartFailed:
            message = "处理组件无法启动，请重新安装 EasyAV";
            break;
        case FailureKind::Canceled:
            message = "已取消";
            break;
        case FailureKind::EmptyOutput:
            message = "处理没有产生有效文件，请换一个文件重试";
            break;
        case FailureKind::Unknown:
        default:
            message = "处理失败，请重试一次";
            break;
        }

        if (!fileNameUtf8.empty())
            message += "（" + fileNameUtf8 + "）";
        return message;
    }

    std::string ShortDetail(const std::string& stderrText, std::size_t maxChars)
    {
        std::string text = TrimEnd(stderrText);
        if (maxChars == 0 || text.size() <= maxChars)
            return text;

        std::size_t start = text.size() - maxChars;

        // 避免切在 UTF-8 续字节上。
        while (start < text.size()
               && (static_cast<unsigned char>(text[start]) & 0xC0) == 0x80)
        {
            ++start;
        }

        return "…" + text.substr(start);
    }
}
