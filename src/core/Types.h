#pragma once

// 纯数据类型：不依赖 ImGui / 平台 API / 业务实现。
// 所有 UI 面板只通过这里定义的结构进行通信。

#include <string>

namespace easyav
{
    enum class SceneId
    {
        Compress = 0,
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

    struct MediaItem
    {
        int id = 0;
        std::string path;        // UTF-8 绝对路径
        std::string displayName; // UTF-8 文件名
        std::string format;      // 大写扩展名，如 MP4
        unsigned long long sizeBytes = 0;
        double durationSec = 0.0; // 0 表示“待分析”（后续由探测模块填写）
        MediaKind kind = MediaKind::Other;
    };

    struct Toast
    {
        std::string text;
        ToastLevel level = ToastLevel::Info;
        float remaining = 3.2f;
    };

    inline constexpr int kSceneCount = static_cast<int>(SceneId::Count);
}
