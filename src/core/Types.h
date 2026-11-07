#pragma once

// 纯数据类型：不依赖 ImGui / 平台 API / 业务实现。
// 所有 UI 面板只通过这里定义的结构进行通信。

#include <string>

namespace easyav
{
    enum class SceneId
    {
        Convert = 0,
        Clip,
        Filter,
        Record,
        Batch,
        Count
    };

    enum class MediaKind
    {
        Video,
        Audio,
        Image,
        Other
    };

    enum class TargetFormat
    {
        MP4 = 0,
        MKV,
        GIF,
        MP3
    };

    // 用户选择的输出视频编码（与“硬件/软件实现”区分，后者是 VideoEncoder）。
    enum class VideoCodec
    {
        H264 = 0,
        H265,
        None // 仅用于“只要声音”目标
    };

    enum class Rotation
    {
        None = 0,
        Clockwise90,
        Deg180,
        Deg270
    };

    enum class AspectPreset
    {
        Original = 0,
        Square,
        Landscape,
        Portrait
    };

    enum class WatermarkMode
    {
        None = 0,
        Text,
        Image
    };

    enum class RecordTarget
    {
        Screen = 0,
        Camera
    };

    enum class ToastLevel
    {
        Info,
        Success,
        Warning,
        Error
    };

    enum class TaskPhase
    {
        Idle = 0,
        Running,
        Completed,
        Canceled
    };

    // 媒体探测结果（ffprobe）。纯数据，不包含具体解析实现。
    struct MediaInfo
    {
        bool ok = false;
        bool hasVideo = false;
        bool hasAudio = false;
        double durationSec = 0.0; // 0 表示未知
        int width = 0;
        int height = 0;
        std::string videoCodec;   // ffprobe 原始名，如 h264 / hevc / av1
        std::string audioCodec;
        std::string errorText;    // 诊断用，不直接展示给用户
    };

    struct MediaItem
    {
        int id = 0;
        std::string path;        // UTF-8 绝对路径
        std::string displayName; // UTF-8 文件名
        std::string format;      // 大写扩展名，如 MP4
        unsigned long long sizeBytes = 0;
        double durationSec = 0.0; // 0 表示“待分析”（后续由探测模块填写）
        bool probed = false;      // false = 尚未探测；true 且 durationSec<=0 = 探测过但时长未知
        MediaKind kind = MediaKind::Other;

        // 探测补充信息：0 / 空表示未知。
        int width = 0;
        int height = 0;
        std::string videoCodec;
        std::string audioCodec;
        bool hasVideo = false;
        bool hasAudio = false;
    };

    struct Toast
    {
        std::string text;
        ToastLevel level = ToastLevel::Info;
        float remaining = 3.2f;
    };

    inline constexpr int kSceneCount = static_cast<int>(SceneId::Count);
}
