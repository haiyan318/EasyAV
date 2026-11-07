#pragma once

// FFmpeg 参数构造（纯函数，不启动进程）。
// 输入永远是参数数组，路径保持原样作为单个参数；引号只在 CommandLine 层处理。

#include "core/TaskContracts.h"

#include <string>
#include <vector>

namespace easyav::services
{
    struct BuildRequest
    {
        std::string inputPath;   // UTF-8
        std::string outputPath;  // UTF-8，带最终扩展名（同时也是 muxer 提示）
        TargetFormat format = TargetFormat::MP4;
        float quality = 0.55f;
        int resolutionIndex = 0; // 0 保持 / 1 1080p / 2 720p / 3 480p
        int frameRateIndex = 0;  // 0 保持 / 1 30 帧 / 2 24 帧
        VideoEncoder encoder = VideoEncoder::Software;
        VideoCodec codec = VideoCodec::H264;
        bool slim = false;       // true = 同时压缩体积
        bool hasVideo = true;
        bool hasAudio = true;
        int sourceWidth = 0;     // 0 表示未知；奇数时补偶数缩放兜底
        int sourceHeight = 0;
    };

    struct BuildResult
    {
        bool ok = false;
        std::vector<std::string> args; // 不含 exe 路径
        std::string errorText;         // 中文，可直接提示
    };

    bool FormatSupportedInV01(TargetFormat format);

    int CrfForQuality(float quality);             // H.264 瘦身：30..20
    const char* PresetForQuality(float quality);  // veryfast..slower
    int CrfForCodec(VideoCodec codec, bool slim, float quality);
    const char* PresetForCodec(VideoCodec codec, bool slim, float quality);
    const char* EncoderName(VideoCodec codec, VideoEncoder encoder);

    int AudioBitrateKbps(float quality);          // AAC: 96..192
    int Mp3BitrateKbps(float quality);            // MP3: 96..256

    BuildResult BuildFfmpegArgs(const BuildRequest& request);
}
