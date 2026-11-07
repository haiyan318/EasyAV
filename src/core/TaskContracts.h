#pragma once

// 任务执行层与模型层共享的纯数据契约。
// 不依赖 ImGui / 平台 API / 具体服务实现：services 与 model 都只依赖这里。

#include "core/Types.h"

#include <string>
#include <vector>

namespace easyav
{
    // 视频编码器的实现方式：优先尝试显卡，失败后由服务层回退到软件编码。
    enum class VideoEncoder
    {
        Software = 0,
        Nvenc,
        Qsv,
        Amf
    };

    // 失败分类只用于选择中文文案，技术细节仅写日志。
    enum class FailureKind
    {
        Unknown = 0,
        InputMissing,
        InputCorrupt,
        PermissionDenied,
        DiskFull,
        UnsupportedCodec,
        PathTooLong,
        OutputLocked,
        EncoderUnavailable,
        CodecUnavailable,
        ProcessStartFailed,
        Canceled,
        EmptyOutput
    };

    struct ToolStatus
    {
        bool ffmpegFound = false;
        bool ffprobeFound = false;
        std::string ffmpegPath;   // UTF-8 绝对路径
        std::string ffprobePath;  // UTF-8 绝对路径
        std::string version;
        std::string problem;      // 面向用户的中文说明
    };

    struct TaskItem
    {
        std::string path;         // UTF-8 绝对路径
        std::string displayName;
        double durationSec = 0.0; // 入队时的探测结果，可为零
        MediaKind kind = MediaKind::Other;
    };

    struct TaskOptions
    {
        TargetFormat format = TargetFormat::MP4;
        float quality = 0.55f;
        bool useGpuAccel = true;
        int resolutionIndex = 0;  // 0 保持原样 / 1 1080p / 2 720p / 3 480p
        int frameRateIndex = 0;   // 0 保持原样 / 1 30 帧 / 2 24 帧
        std::string suffix;       // UTF-8，可为空
        std::string outputDir;    // UTF-8；空表示与原文件同目录

        VideoCodec videoCodec = VideoCodec::H264; // 目标视频编码
        bool slim = false;                        // true = 同时压缩体积
    };

    struct TaskRequest
    {
        std::vector<TaskItem> items;
        TaskOptions options;
    };

    struct FileFailure
    {
        int fileIndex = -1;
        std::string fileName;
        FailureKind kind = FailureKind::Unknown;
        std::string userMessage;      // 已经汉化，可直接提示
        std::string technicalDetail;  // 仅写日志
        int exitCode = 0;
    };

    enum class TaskEventKind
    {
        TaskStarted = 0,
        FileStarted,
        FileProgress,
        FileSucceeded,
        FileFailed,
        TaskFinished,
        TaskCanceled
    };

    struct TaskEvent
    {
        TaskEventKind kind = TaskEventKind::TaskStarted;
        int fileIndex = -1;
        int fileCount = 0;
        std::string fileName;
        std::string outputPath;
        float overallProgress = 0.0f;
        float fileProgress = 0.0f;
        bool indeterminate = false; // 时长未知时为 true
        FileFailure failure;        // kind == FileFailed 时有效
    };

    struct ProbeResult
    {
        std::string path;
        MediaInfo info;
    };
}
