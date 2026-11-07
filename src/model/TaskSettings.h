#pragma once

// 场景参数的纯数据定义。
// 这里只存放“用户意图”，不包含任何命令行参数或 FFmpeg 概念。

#include "core/Types.h"

namespace easyav
{
    // 用户想要的结果：最兼容的视频 / 更小的视频 / 只要声音。
    enum class ConvertTarget
    {
        H264 = 0,
        H265,
        Audio
    };

    struct ConvertSettings
    {
        ConvertTarget target = ConvertTarget::H264; // 默认最兼容
        TargetFormat container = TargetFormat::MP4; // 仅视频目标使用：MP4 / MKV
        bool slim = false;                          // 顺便压缩体积
        float slimQuality = 0.55f;                  // 0 = 体积优先，1 = 画质优先
        bool useGpuAccel = true;
        int resolutionIndex = 0; // 0 保持原样 / 1 1080p / 2 720p / 3 480p
        int frameRateIndex = 0;  // 0 保持原样 / 1 30 帧 / 2 24 帧
    };

    struct ClipSettings
    {
        float startPct = 0.0f; // 0..1
        float endPct = 1.0f;   // 0..1
        bool mergeFiles = false;
        bool extractAudio = false;
        bool stripAudio = false;
    };

    struct FilterSettings
    {
        Rotation rotation = Rotation::None;
        AspectPreset aspect = AspectPreset::Original;

        WatermarkMode watermark = WatermarkMode::None;
        char watermarkText[64] = "EasyAV";
        char watermarkImagePath[512] = {};
        float watermarkOpacity = 0.85f;
        float watermarkScale = 0.20f;
        float watermarkX = 0.82f; // 归一化坐标，0..1
        float watermarkY = 0.82f;
    };

    struct RecordSettings
    {
        RecordTarget target = RecordTarget::Screen;
        bool recording = false;
        float elapsedSec = 0.0f;
        int qualityIndex = 1; // 0 标清 / 1 高清 / 2 原画
        bool captureFrames = false;
        int captureIntervalSec = 5;
    };

    struct OutputSettings
    {
        bool sameAsSource = true;
        char outputDir[512] = {};
        bool autoSuffix = true;
        char suffix[64] = "_转换";
        bool openFolderWhenDone = false;
    };

    struct UiState
    {
        SceneId scene = SceneId::Convert;
        bool showSettings = false;
        bool darkTheme = false;

        // UI 只发出“请求”，由 App/平台层执行，避免面板直接依赖操作系统 API。
        bool requestAddFiles = false;
        bool requestAddFolder = false;
        bool requestPickWatermarkImage = false;
        bool requestPickOutputDir = false;
        bool requestOpenOutputDir = false;
    };
}
