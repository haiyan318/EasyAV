#include "services/FfprobeProbe.h"

#include "services/FfmpegProcess.h"
#include "services/ProbeOutputParser.h"

#include <utility>

namespace easyav::services
{
    FfprobeProbe::FfprobeProbe(std::wstring ffprobeExePath)
        : m_exePath(std::move(ffprobeExePath))
    {
    }

    MediaInfo FfprobeProbe::Probe(const std::string& utf8Path)
    {
        MediaInfo info;

        if (m_exePath.empty())
        {
            info.errorText = "没有媒体分析组件";
            return info;
        }

        if (utf8Path.empty())
        {
            info.errorText = "文件路径为空";
            return info;
        }

        std::string stdoutText;
        std::string stderrText;
        std::string startError;
        int exitCode = -1;

        const bool captured = FfmpegProcess::Capture(
            m_exePath,
            {"-hide_banner", "-v", "error",
             "-show_entries", "format=duration:stream=codec_type,codec_name,width,height",
             "-of", "default=noprint_wrappers=0",
             utf8Path},
            20000, &stdoutText, &stderrText, &exitCode, &startError);

        if (!captured)
        {
            info.errorText = startError.empty() ? "无法读取媒体信息" : startError;
            return info;
        }

        if (exitCode != 0)
        {
            info.errorText = stderrText.empty() ? "无法读取媒体信息" : stderrText;
            return info;
        }

        info = ParseFfprobeText(stdoutText);
        if (!info.ok && info.errorText.empty())
            info.errorText = stderrText;
        return info;
    }
}
