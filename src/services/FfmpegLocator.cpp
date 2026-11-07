#include "services/FfmpegLocator.h"

#include "core/Text.h"
#include "services/FfmpegProcess.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

namespace easyav::services
{
    namespace
    {
        namespace fs = std::filesystem;

        constexpr DWORD kMaxPathChars = 32768;

        bool IsRegularFile(const std::wstring& path)
        {
            std::error_code ec;
            return fs::is_regular_file(fs::path(path), ec);
        }

        void AddUnique(std::vector<std::wstring>& list, const std::wstring& candidate)
        {
            if (candidate.empty())
                return;
            for (const std::wstring& existing : list)
            {
                if (existing == candidate)
                    return;
            }
            list.push_back(candidate);
        }

        void AddDirectoryCandidates(std::vector<std::wstring>& list,
                                    const std::string& dirUtf8,
                                    const wchar_t* exeName)
        {
            if (dirUtf8.empty())
                return;

            const fs::path dir = PathFromUtf8(dirUtf8);
            AddUnique(list, (dir / exeName).wstring());
            AddUnique(list, (dir / L"bin" / exeName).wstring());
        }

        std::string EnvironmentDirectoryUtf8()
        {
            wchar_t buffer[kMaxPathChars] = {};
            const DWORD length =
                GetEnvironmentVariableW(L"EASYAV_FFMPEG_DIR", buffer, kMaxPathChars);
            if (length == 0 || length >= kMaxPathChars)
                return {};
            return WideToUtf8(std::wstring(buffer, length));
        }

        std::wstring SearchOnPath(const wchar_t* exeName)
        {
            wchar_t buffer[kMaxPathChars] = {};
            const DWORD length = SearchPathW(nullptr, exeName, nullptr, kMaxPathChars,
                                             buffer, nullptr);
            if (length == 0 || length >= kMaxPathChars)
                return {};
            return std::wstring(buffer, length);
        }

        bool ValidateTool(const std::wstring& exePath,
                          const std::string& versionMarker,
                          std::string* versionOut)
        {
            if (exePath.empty() || !IsRegularFile(exePath))
                return false;

            std::string stdoutText;
            std::string stderrText;
            std::string startError;
            int exitCode = -1;

            if (!FfmpegProcess::Capture(exePath, {"-hide_banner", "-version"}, 3000,
                                        &stdoutText, &stderrText, &exitCode, &startError))
            {
                return false;
            }

            if (exitCode != 0)
                return false;

            const std::size_t pos = stdoutText.find(versionMarker);
            if (pos == std::string::npos)
                return false;

            if (versionOut != nullptr)
            {
                const std::size_t start = pos + versionMarker.size();
                const std::size_t end = stdoutText.find_first_of(" \r\n", start);
                *versionOut = stdoutText.substr(
                    start, end == std::string::npos ? std::string::npos : end - start);
            }
            return true;
        }

        std::vector<std::wstring> BuildCandidates(const wchar_t* exeName,
                                                  const std::string& exeDirUtf8,
                                                  const std::string& cwdUtf8)
        {
            std::vector<std::wstring> candidates;

            // 1) 显式覆盖（开发/绿色版可以指向任意目录或直接指向 exe）
            const std::string envDir = EnvironmentDirectoryUtf8();
            if (!envDir.empty())
            {
                const fs::path envPath = PathFromUtf8(envDir);
                if (IsRegularFile(envPath.wstring()))
                    AddUnique(candidates, envPath.wstring());
                else
                    AddDirectoryCandidates(candidates, envDir, exeName);
            }

            // 2) 系统 PATH（用户已安装的 FFmpeg 优先）
            AddUnique(candidates, SearchOnPath(exeName));

            // 3) 内置目录：<exeDir>/assets/ffmpeg{,/bin}
            if (!exeDirUtf8.empty())
            {
                const fs::path bundled = PathFromUtf8(exeDirUtf8) / L"assets" / L"ffmpeg";
                AddUnique(candidates, (bundled / exeName).wstring());
                AddUnique(candidates, (bundled / L"bin" / exeName).wstring());
            }

            // 4) exe 同目录及其 bin
            AddDirectoryCandidates(candidates, exeDirUtf8, exeName);

            // 5) 当前工作目录（从 IDE/仓库根目录运行时）
            if (!cwdUtf8.empty())
            {
                const fs::path bundled = PathFromUtf8(cwdUtf8) / L"assets" / L"ffmpeg";
                AddUnique(candidates, (bundled / exeName).wstring());
                AddUnique(candidates, (bundled / L"bin" / exeName).wstring());
            }
            AddDirectoryCandidates(candidates, cwdUtf8, exeName);

            return candidates;
        }

        std::wstring FindTool(const wchar_t* exeName,
                              const std::string& versionMarker,
                              const std::wstring& preferredDir,
                              const std::string& exeDirUtf8,
                              const std::string& cwdUtf8,
                              std::string* versionOut)
        {
            std::vector<std::wstring> candidates;

            // ffprobe 优先与选中的 ffmpeg 放在同一目录。
            if (!preferredDir.empty())
                AddUnique(candidates, (fs::path(preferredDir) / exeName).wstring());

            for (const std::wstring& candidate :
                 BuildCandidates(exeName, exeDirUtf8, cwdUtf8))
            {
                AddUnique(candidates, candidate);
            }

            for (const std::wstring& candidate : candidates)
            {
                if (ValidateTool(candidate, versionMarker, versionOut))
                    return candidate;
            }
            return {};
        }
    }

    LocatedTools FfmpegLocator::Locate(const std::string& exeDirUtf8,
                                       const std::string& cwdUtf8)
    {
        LocatedTools located;

        std::string ffmpegVersion;
        located.ffmpegExeW = FindTool(L"ffmpeg.exe", "ffmpeg version ", {},
                                      exeDirUtf8, cwdUtf8, &ffmpegVersion);
        located.status.ffmpegFound = !located.ffmpegExeW.empty();
        located.status.version = ffmpegVersion;
        if (located.status.ffmpegFound)
            located.status.ffmpegPath = WideToUtf8(located.ffmpegExeW);

        std::wstring preferredDir;
        if (located.status.ffmpegFound)
            preferredDir = fs::path(located.ffmpegExeW).parent_path().wstring();

        std::string ffprobeVersion;
        located.ffprobeExeW = FindTool(L"ffprobe.exe", "ffprobe version ", preferredDir,
                                       exeDirUtf8, cwdUtf8, &ffprobeVersion);
        located.status.ffprobeFound = !located.ffprobeExeW.empty();
        if (located.status.ffprobeFound)
            located.status.ffprobePath = WideToUtf8(located.ffprobeExeW);

        if (!located.status.ffmpegFound)
        {
            located.status.problem =
                "没有找到处理组件，请重新安装 EasyAV 或先安装 FFmpeg";
        }
        else if (!located.status.ffprobeFound)
        {
            located.status.problem = "没有找到媒体分析组件，队列暂时无法显示时长";
        }

        return located;
    }
}
