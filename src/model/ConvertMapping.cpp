#include "model/ConvertMapping.h"

#include <algorithm>

namespace easyav
{
    namespace
    {
        bool IsAudioTarget(const ConvertSettings& settings)
        {
            return settings.target == ConvertTarget::Audio;
        }

        bool IsHevc(const ConvertSettings& settings)
        {
            return settings.target == ConvertTarget::H265;
        }
    }

    TargetFormat FormatFor(const ConvertSettings& settings)
    {
        if (IsAudioTarget(settings))
            return TargetFormat::MP3;

        return settings.container == TargetFormat::MKV ? TargetFormat::MKV
                                                       : TargetFormat::MP4;
    }

    VideoCodec CodecFor(const ConvertSettings& settings)
    {
        if (IsAudioTarget(settings))
            return VideoCodec::None;

        return IsHevc(settings) ? VideoCodec::H265 : VideoCodec::H264;
    }

    float EffectiveQuality(const ConvertSettings& settings)
    {
        if (IsAudioTarget(settings))
            return 0.75f; // 提取音频时给较稳定的 160-192k 档

        return settings.slim ? std::clamp(settings.slimQuality, 0.0f, 1.0f) : 0.85f;
    }

    std::string AutoSuffixFor(const ConvertSettings& settings)
    {
        if (IsAudioTarget(settings))
            return "_音频";

        return IsHevc(settings) ? "_H265" : "_H264";
    }

    double EstimateSizeFactor(const ConvertSettings& settings)
    {
        if (IsAudioTarget(settings))
            return 0.08;

        const double hevcScale = IsHevc(settings) ? 0.70 : 1.0;
        if (!settings.slim)
            return (IsHevc(settings) ? 0.62 : 0.95);

        const double base = 0.20 + 0.72 * static_cast<double>(
                                      std::clamp(settings.slimQuality, 0.0f, 1.0f));
        return base * hevcScale;
    }

    TaskOptions BuildTaskOptions(const ConvertSettings& settings,
                                 const OutputSettings& output,
                                 const std::string& resolvedOutputDir)
    {
        TaskOptions options;
        options.format = FormatFor(settings);
        options.videoCodec = CodecFor(settings);
        options.quality = EffectiveQuality(settings);
        options.useGpuAccel = !IsAudioTarget(settings) && settings.useGpuAccel;
        options.suffix = output.autoSuffix ? AutoSuffixFor(settings)
                                           : std::string(output.suffix);
        options.outputDir = resolvedOutputDir;

        // 分辨率/帧率只在“顺便瘦身”时生效，避免编码转换时意外改变画幅。
        const bool slimVideo = settings.slim && !IsAudioTarget(settings);
        options.slim = slimVideo;
        options.resolutionIndex = slimVideo ? settings.resolutionIndex : 0;
        options.frameRateIndex = slimVideo ? settings.frameRateIndex : 0;
        return options;
    }
}
