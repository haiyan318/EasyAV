#include "services/TaskService.h"

#include "core/Text.h"
#include "services/ErrorFormatter.h"
#include "services/FfmpegCommandBuilder.h"
#include "services/FfmpegProcess.h"
#include "services/FfprobeProbe.h"
#include "services/Logger.h"
#include "services/OutputNamer.h"
#include "services/ProgressParser.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <system_error>
#include <utility>

namespace easyav::services
{
    namespace
    {
        namespace fs = std::filesystem;

        FileFailure MakeFailure(int index, const TaskItem& item, FailureKind kind,
                                const std::string& detail, int exitCode,
                                VideoCodec codec = VideoCodec::H264,
                                bool hardwareAttempt = false)
        {
            FileFailure failure;
            failure.fileIndex = index;
            failure.fileName = item.displayName;
            failure.kind = kind;
            failure.userMessage = UserMessage(kind, item.displayName, codec,
                                              hardwareAttempt);
            failure.technicalDetail = ShortDetail(detail);
            failure.exitCode = exitCode;
            return failure;
        }

        bool IsInputProblem(FailureKind kind)
        {
            return kind == FailureKind::InputMissing
                || kind == FailureKind::InputCorrupt
                || kind == FailureKind::PermissionDenied
                || kind == FailureKind::DiskFull
                || kind == FailureKind::PathTooLong;
        }

        bool FileHasContent(const fs::path& path)
        {
            std::error_code ec;
            const auto size = fs::file_size(path, ec);
            return !ec && size > 0;
        }

        bool PrepareOutputDirectory(const fs::path& directory, const fs::path& tempPath,
                                    std::string* detail)
        {
            std::error_code ec;
            fs::create_directories(directory, ec);
            if (ec)
            {
                *detail = ec.message();
                return false;
            }

            {
                std::ofstream probe(tempPath, std::ios::binary | std::ios::trunc);
                if (!probe.is_open())
                {
                    *detail = "无法创建输出文件";
                    return false;
                }
            }

            std::error_code removeEc;
            fs::remove(tempPath, removeEc);
            return true;
        }

        bool MoveFileUtf8(const fs::path& from, const fs::path& to, DWORD* errorCode)
        {
            if (MoveFileExW(from.wstring().c_str(), to.wstring().c_str(),
                            MOVEFILE_WRITE_THROUGH))
            {
                return true;
            }

            *errorCode = GetLastError();
            return false;
        }

        FailureKind MoveFailureKind(DWORD errorCode)
        {
            switch (errorCode)
            {
            case ERROR_ACCESS_DENIED:
            case ERROR_WRITE_PROTECT:
                return FailureKind::PermissionDenied;
            case ERROR_SHARING_VIOLATION:
            case ERROR_LOCK_VIOLATION:
                return FailureKind::OutputLocked;
            default:
                return FailureKind::Unknown;
            }
        }
    }

    TaskService::TaskService() = default;

    TaskService::~TaskService()
    {
        Shutdown();
    }

    bool TaskService::Initialize(const std::string& exeDirUtf8,
                                 const std::string& cwdUtf8)
    {
        if (m_initialized)
            return true;

        m_located = FfmpegLocator::Locate(exeDirUtf8, cwdUtf8);
        m_tools = m_located.status;

        if (m_tools.ffprobeFound)
            m_probe = std::make_unique<FfprobeProbe>(m_located.ffprobeExeW);

        m_stop.store(false);
        m_cancel.store(false);
        m_worker = std::thread(&TaskService::WorkerLoop, this);
        m_initialized = true;
        return true;
    }

    void TaskService::Shutdown()
    {
        if (!m_initialized)
            return;

        m_cancel.store(true);
        m_stop.store(true);
        m_wake.notify_all();

        if (m_worker.joinable())
            m_worker.join();

        DeleteActiveTemp();

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_taskQueue.clear();
            m_probeQueue.clear();
            m_pending.clear();
            m_taskEvents.clear();
            m_probeResults.clear();
            m_cache.clear();
            m_taskBusy = false;
        }

        m_probe.reset();
        m_initialized = false;
        m_cancel.store(false);
        m_stop.store(false);
    }

    bool TaskService::Start(const TaskRequest& request)
    {
        m_startError.clear();

        if (!m_initialized)
        {
            m_startError = "处理组件还没有准备好，请稍后重试";
            return false;
        }

        if (request.items.empty())
        {
            m_startError = "请先添加要处理的文件";
            return false;
        }

        if (!m_tools.ffmpegFound)
        {
            m_startError = m_tools.problem.empty() ? "找不到处理组件" : m_tools.problem;
            return false;
        }

        if (!FormatSupportedInV01(request.options.format))
        {
            m_startError = request.options.format == TargetFormat::GIF
                               ? "GIF 动图会在后续版本支持，请先选择 MP4 或 MKV"
                               : "暂时不支持这个输出格式";
            return false;
        }

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_taskBusy)
            {
                m_startError = "正在处理，请等当前任务结束";
                return false;
            }

            m_taskBusy = true;
            m_cancel.store(false);
            m_taskQueue.push_back(request);
        }

        m_wake.notify_one();
        return true;
    }

    void TaskService::Cancel()
    {
        m_cancel.store(true);
        m_wake.notify_all();
    }

    void TaskService::DrainTaskEvents(std::vector<TaskEvent>& out)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        out.clear();
        out.swap(m_taskEvents);
    }

    void TaskService::EnqueueProbe(const std::vector<std::string>& utf8Paths)
    {
        if (!m_initialized || utf8Paths.empty())
            return;

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            for (const std::string& path : utf8Paths)
            {
                if (path.empty())
                    continue;
                if (m_pending.find(path) != m_pending.end())
                    continue;

                // 已探测过的路径直接回放缓存，避免重新入队后一直停在“待分析”。
                const auto cached = m_cache.find(path);
                if (cached != m_cache.end())
                {
                    ProbeResult replay;
                    replay.path = path;
                    replay.info = cached->second;
                    m_probeResults.push_back(std::move(replay));
                    continue;
                }

                m_pending.insert(path);
                m_probeQueue.push_back(path);
            }
        }

        m_wake.notify_one();
    }

    void TaskService::DrainProbeResults(std::vector<ProbeResult>& out)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        out.clear();
        out.swap(m_probeResults);
    }

    void TaskService::PushEvent(TaskEvent event)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_taskEvents.push_back(std::move(event));
    }

    void TaskService::SetActiveTemp(const std::string& utf8Path)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_activeTemp = utf8Path;
    }

    void TaskService::DeleteActiveTemp()
    {
        std::string path;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            path.swap(m_activeTemp);
        }

        if (path.empty())
            return;

        std::error_code ec;
        fs::remove(PathFromUtf8(path), ec);
    }

    MediaInfo TaskService::ProbeForTask(const TaskItem& item)
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            const auto cached = m_cache.find(item.path);
            if (cached != m_cache.end())
                return cached->second;
        }

        MediaInfo info;
        if (m_probe)
        {
            info = m_probe->Probe(item.path);
        }
        else
        {
            info.ok = false;
            info.errorText = "没有媒体分析组件";
        }

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_cache[item.path] = info;
        }

        return info;
    }

    void TaskService::WorkerLoop()
    {
        for (;;)
        {
            TaskRequest task;
            std::string probePath;
            bool hasTask = false;
            bool hasProbe = false;

            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_wake.wait(lock, [this]() {
                    return m_stop.load() || !m_taskQueue.empty() || !m_probeQueue.empty();
                });

                if (m_stop.load() && m_taskQueue.empty() && m_probeQueue.empty())
                    return;

                if (!m_taskQueue.empty())
                {
                    task = std::move(m_taskQueue.front());
                    m_taskQueue.pop_front();
                    hasTask = true;
                }
                else
                {
                    probePath = std::move(m_probeQueue.front());
                    m_probeQueue.pop_front();
                    hasProbe = true;
                }
            }

            if (hasTask)
            {
                RunTask(task);
                m_cancel.store(false);
                continue;
            }

            if (!hasProbe)
                continue;

            ProbeResult result;
            result.path = probePath;

            if (m_probe)
            {
                result.info = m_probe->Probe(probePath);
            }
            else
            {
                result.info.ok = false;
                result.info.errorText = "没有媒体分析组件";
            }

            if (!result.info.ok && !result.info.errorText.empty())
                Logger::Warn("媒体探测失败: " + probePath + " - " + result.info.errorText);

            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_cache[probePath] = result.info;
                m_pending.erase(probePath);
                m_probeResults.push_back(std::move(result));
            }
        }
    }

    void TaskService::RunTask(const TaskRequest& request)
    {
        const int count = static_cast<int>(request.items.size());
        const char* codecText = request.options.format == TargetFormat::MP3 ? "音频"
                                : (request.options.videoCodec == VideoCodec::H265 ? "H.265"
                                                                                   : "H.264");
        Logger::Info("任务开始: " + std::to_string(count) + " 个文件，输出格式 "
                     + ExtensionFor(request.options.format) + "，目标编码 " + codecText
                     + (request.options.slim ? "，同时瘦身" : ""));

        TaskEvent started;
        started.kind = TaskEventKind::TaskStarted;
        started.fileCount = count;
        PushEvent(started);

        int succeeded = 0;
        int failed = 0;
        bool canceled = false;

        for (int index = 0; index < count; ++index)
        {
            if (m_cancel.load())
            {
                canceled = true;
                break;
            }

            const bool fileOk = ProcessFile(index, count, request, canceled);
            if (fileOk)
                ++succeeded;
            else if (!canceled)
                ++failed;

            if (canceled)
                break;
        }

        if (m_cancel.load())
            canceled = true;

        // 先标记空闲，再发结束事件：界面看到“已取消/已结束”时就能立即重新开始。
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_taskBusy = false;
        }

        TaskEvent finalEvent;
        finalEvent.kind = canceled ? TaskEventKind::TaskCanceled
                                   : TaskEventKind::TaskFinished;
        finalEvent.fileCount = count;
        finalEvent.overallProgress = canceled ? 0.0f : 1.0f;
        PushEvent(finalEvent);

        Logger::Info("任务结束: 成功 " + std::to_string(succeeded) + " 个，失败 "
                     + std::to_string(failed) + " 个" + (canceled ? "，已取消" : ""));
    }

    bool TaskService::ProcessFile(int index, int count, const TaskRequest& request,
                                  bool& canceled)
    {
        const TaskItem& item = request.items[static_cast<std::size_t>(index)];

        TaskEvent started;
        started.kind = TaskEventKind::FileStarted;
        started.fileIndex = index;
        started.fileCount = count;
        started.fileName = item.displayName;
        started.overallProgress = count > 0 ? static_cast<float>(index) / count : 0.0f;
        PushEvent(started);

        // 1) 探测（没有 ffprobe 时按扩展名推测，保证仍可处理）
        MediaInfo info = ProbeForTask(item);
        if (!info.ok && !m_tools.ffprobeFound)
        {
            info.ok = true;
            info.hasVideo = item.kind == MediaKind::Video;
            info.hasAudio = item.kind == MediaKind::Video || item.kind == MediaKind::Audio;
            info.durationSec = item.durationSec;
        }

        if (!info.ok)
        {
            TaskEvent failed;
            failed.kind = TaskEventKind::FileFailed;
            failed.fileIndex = index;
            failed.fileCount = count;
            failed.fileName = item.displayName;
            failed.overallProgress = count > 0 ? static_cast<float>(index) / count : 0.0f;
            failed.failure = MakeFailure(index, item, FailureKind::InputCorrupt,
                                         info.errorText, 0);
            PushEvent(failed);
            return false;
        }

        // 2) 命名
        NamingRequest namingRequest;
        namingRequest.sourcePath = item.path;
        namingRequest.outputDir = request.options.outputDir;
        namingRequest.suffix = request.options.suffix;
        namingRequest.format = request.options.format;

        NamingResult naming = Resolve(namingRequest, {}, m_tempSerial++);
        if (!naming.ok)
        {
            const FailureKind kind =
                naming.errorText.find("路径太长") != std::string::npos
                    ? FailureKind::PathTooLong
                    : FailureKind::Unknown;

            TaskEvent failed;
            failed.kind = TaskEventKind::FileFailed;
            failed.fileIndex = index;
            failed.fileCount = count;
            failed.fileName = item.displayName;
            failed.overallProgress = count > 0 ? static_cast<float>(index) / count : 0.0f;
            failed.failure = MakeFailure(index, item, kind, naming.errorText, 0);
            PushEvent(failed);
            return false;
        }

        const fs::path tempPath = PathFromUtf8(naming.tempPath);
        fs::path finalPath = PathFromUtf8(naming.finalPath);

        // 3) 输出目录与可写性预检
        std::string prepareDetail;
        if (!PrepareOutputDirectory(finalPath.parent_path(), tempPath, &prepareDetail))
        {
            TaskEvent failed;
            failed.kind = TaskEventKind::FileFailed;
            failed.fileIndex = index;
            failed.fileCount = count;
            failed.fileName = item.displayName;
            failed.overallProgress = count > 0 ? static_cast<float>(index) / count : 0.0f;
            failed.failure = MakeFailure(index, item, FailureKind::PermissionDenied,
                                         prepareDetail, 0);
            PushEvent(failed);
            return false;
        }

        // 4) 逐编码器尝试（显卡失败回退软件）
        std::vector<VideoEncoder> attempts;
        if (request.options.useGpuAccel
            && request.options.format != TargetFormat::MP3
            && info.hasVideo)
        {
            attempts.push_back(VideoEncoder::Nvenc);
            attempts.push_back(VideoEncoder::Software);
        }
        else
        {
            attempts.push_back(VideoEncoder::Software);
        }

        FailureKind lastKind = FailureKind::Unknown;
        std::string lastDetail;
        int lastExitCode = -1;
        bool produced = false;

        for (std::size_t attempt = 0; attempt < attempts.size(); ++attempt)
        {
            if (m_cancel.load())
            {
                canceled = true;
                DeleteActiveTemp();
                return false;
            }

            BuildRequest build;
            build.inputPath = item.path;
            build.outputPath = naming.tempPath;
            build.format = request.options.format;
            build.quality = request.options.quality;
            build.resolutionIndex = request.options.resolutionIndex;
            build.frameRateIndex = request.options.frameRateIndex;
            build.encoder = attempts[attempt];
            build.codec = request.options.videoCodec;
            build.slim = request.options.slim;
            build.hasVideo = info.hasVideo;
            build.hasAudio = info.hasAudio;
            build.sourceWidth = info.width;
            build.sourceHeight = info.height;

            const BuildResult args = BuildFfmpegArgs(build);
            if (!args.ok)
            {
                TaskEvent failed;
                failed.kind = TaskEventKind::FileFailed;
                failed.fileIndex = index;
                failed.fileCount = count;
                failed.fileName = item.displayName;
                failed.failure = MakeFailure(index, item, FailureKind::UnsupportedCodec,
                                             args.errorText, 0);
                PushEvent(failed);
                return false;
            }

            Logger::Command(m_located.ffmpegExeW, args.args);
            SetActiveTemp(naming.tempPath);

            FfmpegProcess process;
            std::string startError;
            if (!process.Start(m_located.ffmpegExeW, args.args, &startError))
            {
                DeleteActiveTemp();
                TaskEvent failed;
                failed.kind = TaskEventKind::FileFailed;
                failed.fileIndex = index;
                failed.fileCount = count;
                failed.fileName = item.displayName;
                failed.failure = MakeFailure(index, item, FailureKind::ProcessStartFailed,
                                             startError, 0);
                PushEvent(failed);
                return false;
            }

            ProgressParser parser(info.durationSec);
            auto lastEmit = std::chrono::steady_clock::now()
                            - std::chrono::milliseconds(200);

            const auto onProgress = [&](const char* data, std::size_t size) {
                parser.Feed(data, size);

                const auto now = std::chrono::steady_clock::now();
                if (now - lastEmit < std::chrono::milliseconds(100))
                    return;
                lastEmit = now;

                const double fraction = parser.Fraction();
                TaskEvent progress;
                progress.kind = TaskEventKind::FileProgress;
                progress.fileIndex = index;
                progress.fileCount = count;
                progress.fileName = item.displayName;
                progress.fileProgress =
                    fraction < 0.0 ? 0.0f : static_cast<float>(fraction);
                progress.indeterminate = fraction < 0.0;
                progress.overallProgress =
                    count > 0
                        ? (static_cast<float>(index) + progress.fileProgress)
                              / static_cast<float>(count)
                        : 0.0f;
                PushEvent(progress);
            };

            const FfmpegProcess::Result result =
                process.Wait(&m_cancel, onProgress, nullptr, 0);

            if (result.canceled || m_cancel.load())
            {
                canceled = true;
                DeleteActiveTemp();
                return false;
            }

            const bool hasContent = FileHasContent(tempPath);
            if (result.exitCode == 0 && hasContent)
            {
                produced = true;
                break;
            }

            lastKind = ClassifyFailure(result.exitCode, result.stderrTail, !hasContent);
            lastDetail = result.stderrTail;
            lastExitCode = result.exitCode;

            Logger::Warn("编码失败(" + item.displayName + "): exit="
                         + std::to_string(result.exitCode) + " - "
                         + ShortDetail(lastDetail, 500));
            DeleteActiveTemp();

            const bool gpuAttempt = attempts[attempt] != VideoEncoder::Software;
            if (gpuAttempt && !IsInputProblem(lastKind) && attempt + 1 < attempts.size())
            {
                Logger::Warn("显卡编码不可用，回退软件编码");
                continue;
            }

            // 软件编码也报“编码器不可用”，说明是处理组件缺少该编码，而不是显卡问题。
            FailureKind finalKind = lastKind;
            const bool softwareAttempt = attempts[attempt] == VideoEncoder::Software;
            if (softwareAttempt && finalKind == FailureKind::EncoderUnavailable)
                finalKind = FailureKind::CodecUnavailable;

            TaskEvent failed;
            failed.kind = TaskEventKind::FileFailed;
            failed.fileIndex = index;
            failed.fileCount = count;
            failed.fileName = item.displayName;
            failed.failure = MakeFailure(index, item, finalKind, lastDetail, lastExitCode,
                                         request.options.videoCodec, !softwareAttempt);
            PushEvent(failed);
            return false;
        }

        if (!produced)
        {
            TaskEvent failed;
            failed.kind = TaskEventKind::FileFailed;
            failed.fileIndex = index;
            failed.fileCount = count;
            failed.fileName = item.displayName;
            failed.failure = MakeFailure(index, item, lastKind, lastDetail, lastExitCode,
                                         request.options.videoCodec, false);
            PushEvent(failed);
            return false;
        }

        // 5) 原子落盘（不覆盖已有文件）
        DWORD moveError = 0;
        if (!MoveFileUtf8(tempPath, finalPath, &moveError))
        {
            if (moveError == ERROR_ALREADY_EXISTS || moveError == ERROR_FILE_EXISTS)
            {
                const NamingResult renamed = Resolve(namingRequest, {}, m_tempSerial++);
                if (renamed.ok)
                {
                    finalPath = PathFromUtf8(renamed.finalPath);
                    moveError = 0;
                    if (!MoveFileUtf8(tempPath, finalPath, &moveError))
                        moveError = moveError == 0 ? ERROR_GEN_FAILURE : moveError;
                }
            }
        }

        if (moveError != 0)
        {
            DeleteActiveTemp();

            TaskEvent failed;
            failed.kind = TaskEventKind::FileFailed;
            failed.fileIndex = index;
            failed.fileCount = count;
            failed.fileName = item.displayName;
            failed.failure = MakeFailure(index, item, MoveFailureKind(moveError),
                                         "移动输出文件失败: " + std::to_string(moveError), 0);
            PushEvent(failed);
            return false;
        }

        SetActiveTemp({});
        Logger::Info("文件完成: " + item.displayName + " -> " + PathToUtf8(finalPath));

        TaskEvent succeeded;
        succeeded.kind = TaskEventKind::FileSucceeded;
        succeeded.fileIndex = index;
        succeeded.fileCount = count;
        succeeded.fileName = item.displayName;
        succeeded.outputPath = PathToUtf8(finalPath);
        succeeded.overallProgress =
            count > 0 ? static_cast<float>(index + 1) / static_cast<float>(count) : 1.0f;
        PushEvent(succeeded);
        return true;
    }
}
