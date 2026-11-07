#include "services/FfmpegCommandBuilder.h"

#include <algorithm>
#include <string>
#include <utility>

namespace easyav::services
{
    namespace
    {
        float ClampQuality(float quality)
        {
            return std::clamp(quality, 0.0f, 1.0f);
        }

        void Add(std::vector<std::string>& args, std::string value)
        {
            args.push_back(std::move(value));
        }

        void AddPair(std::vector<std::string>& args, const char* key, const char* value)
        {
            args.push_back(key);
            args.push_back(value);
        }

        void AddPair(std::vector<std::string>& args, const char* key, const std::string& value)
        {
            args.push_back(key);
            args.push_back(value);
        }

        int GpuQualityForQuality(float quality)
        {
            if (quality < 0.20f) return 32;
            if (quality < 0.40f) return 30;
            if (quality < 0.60f) return 27;
            if (quality < 0.80f) return 24;
            return 22;
        }

        bool IsHevc(VideoCodec codec)
        {
            return codec == VideoCodec::H265;
        }

        void AddVideoEncoder(std::vector<std::string>& args, VideoEncoder encoder,
                             VideoCodec codec, bool slim, float quality)
        {
            switch (encoder)
            {
            case VideoEncoder::Nvenc:
                AddPair(args, "-c:v", EncoderName(codec, encoder));
                AddPair(args, "-preset", "p5");
                AddPair(args, "-rc", "vbr");
                AddPair(args, "-cq", std::to_string(GpuQualityForQuality(quality)));
                AddPair(args, "-b:v", "0");
                break;

            case VideoEncoder::Qsv:
                AddPair(args, "-c:v", EncoderName(codec, encoder));
                AddPair(args, "-global_quality",
                        std::to_string(GpuQualityForQuality(quality)));
                break;

            case VideoEncoder::Amf:
            {
                AddPair(args, "-c:v", EncoderName(codec, encoder));
                AddPair(args, "-rc", "cqp");
                const std::string qp = std::to_string(GpuQualityForQuality(quality));
                AddPair(args, "-qp_i", qp);
                AddPair(args, "-qp_p", qp);
                break;
            }

            case VideoEncoder::Software:
            default:
                AddPair(args, "-c:v", EncoderName(codec, encoder));
                AddPair(args, "-preset", PresetForCodec(codec, slim, quality));
                AddPair(args, "-crf", std::to_string(CrfForCodec(codec, slim, quality)));
                // x265 默认向 stderr 打印编码信息，会污染日志并干扰错误分类。
                if (IsHevc(codec))
                    AddPair(args, "-x265-params", "log-level=error");
                break;
            }

            AddPair(args, "-pix_fmt", "yuv420p");
        }

        std::string ScaleFilterForResolution(int index)
        {
            switch (index)
            {
            case 1: return "scale=-2:1080";
            case 2: return "scale=-2:720";
            case 3: return "scale=-2:480";
            default: return {};
            }
        }

        std::string FrameRateForIndex(int index)
        {
            switch (index)
            {
            case 1: return "30";
            case 2: return "24";
            default: return {};
            }
        }

        bool NeedsEvenDimensionGuard(int width, int height)
        {
            return width > 0 && height > 0 && ((width % 2) != 0 || (height % 2) != 0);
        }
    }

    bool FormatSupportedInV01(TargetFormat format)
    {
        return format == TargetFormat::MP4
            || format == TargetFormat::MKV
            || format == TargetFormat::MP3;
    }

    int CrfForQuality(float quality)
    {
        const float q = ClampQuality(quality);
        if (q < 0.20f) return 30;
        if (q < 0.40f) return 28;
        if (q < 0.60f) return 25;
        if (q < 0.80f) return 22;
        return 20;
    }

    const char* PresetForQuality(float quality)
    {
        const float q = ClampQuality(quality);
        if (q < 0.20f) return "veryfast";
        if (q < 0.40f) return "veryfast";
        if (q < 0.60f) return "medium";
        if (q < 0.80f) return "slow";
        return "slower";
    }

    int CrfForCodec(VideoCodec codec, bool slim, float quality)
    {
        if (!slim)
            return IsHevc(codec) ? 23 : 20;

        if (!IsHevc(codec))
            return CrfForQuality(quality);

        // HEVC 在同画质下 CRF 通常比 H.264 高 3 左右；上限避免压得太过。
        return std::min(CrfForQuality(quality) + 3, 33);
    }

    const char* PresetForCodec(VideoCodec codec, bool slim, float quality)
    {
        if (!slim)
            return IsHevc(codec) ? "faster" : "fast";

        if (!IsHevc(codec))
            return PresetForQuality(quality);

        // x265 的 slow/slower 对普通用户过慢，瘦身场景封顶 medium。
        const float q = ClampQuality(quality);
        return q < 0.5f ? "fast" : "medium";
    }

    const char* EncoderName(VideoCodec codec, VideoEncoder encoder)
    {
        if (IsHevc(codec))
        {
            switch (encoder)
            {
            case VideoEncoder::Nvenc: return "hevc_nvenc";
            case VideoEncoder::Qsv:   return "hevc_qsv";
            case VideoEncoder::Amf:   return "hevc_amf";
            case VideoEncoder::Software:
            default:                  return "libx265";
            }
        }

        switch (encoder)
        {
        case VideoEncoder::Nvenc: return "h264_nvenc";
        case VideoEncoder::Qsv:   return "h264_qsv";
        case VideoEncoder::Amf:   return "h264_amf";
        case VideoEncoder::Software:
        default:                  return "libx264";
        }
    }

    int AudioBitrateKbps(float quality)
    {
        const float q = ClampQuality(quality);
        if (q < 0.20f) return 96;
        if (q < 0.40f) return 128;
        if (q < 0.60f) return 128;
        if (q < 0.80f) return 160;
        return 192;
    }

    int Mp3BitrateKbps(float quality)
    {
        const float q = ClampQuality(quality);
        if (q < 0.20f) return 96;
        if (q < 0.40f) return 128;
        if (q < 0.60f) return 160;
        if (q < 0.80f) return 192;
        return 256;
    }

    BuildResult BuildFfmpegArgs(const BuildRequest& request)
    {
        BuildResult result;

        if (request.inputPath.empty() || request.outputPath.empty())
        {
            result.errorText = "找不到要处理的文件或输出位置";
            return result;
        }

        if (!FormatSupportedInV01(request.format))
        {
            result.errorText = "GIF 动图会在后续版本支持，请先选择 MP4 或 MKV";
            return result;
        }

        if (request.format == TargetFormat::MP3)
        {
            if (!request.hasAudio)
            {
                result.errorText = "这个文件里没有声音，无法提取";
                return result;
            }
        }
        else
        {
            if (!request.hasVideo)
            {
                result.errorText = "这个文件没有画面，请选择“只要声音”";
                return result;
            }

            if (request.codec == VideoCodec::None)
            {
                result.errorText = "这个文件只有声音，请选择“只要声音”";
                return result;
            }
        }

        const float quality = ClampQuality(request.quality);
        std::vector<std::string>& args = result.args;

        Add(args, "-hide_banner");
        Add(args, "-nostdin");
        AddPair(args, "-loglevel", "error");
        Add(args, "-nostats");
        AddPair(args, "-progress", "pipe:1");
        Add(args, "-y");
        AddPair(args, "-i", request.inputPath);

        if (request.format == TargetFormat::MP3)
        {
            Add(args, "-vn");
            AddPair(args, "-map", "0:a:0");
            AddPair(args, "-c:a", "libmp3lame");
            AddPair(args, "-b:a", std::to_string(Mp3BitrateKbps(quality)) + "k");
            AddPair(args, "-f", "mp3");
            Add(args, request.outputPath);
            result.ok = true;
            return result;
        }

        AddPair(args, "-map", "0:v:0?");
        AddPair(args, "-map", "0:a:0?");
        Add(args, "-sn");
        Add(args, "-dn");

        AddVideoEncoder(args, request.encoder, request.codec, request.slim, quality);

        // 只产生一条 -vf：分辨率目标优先，否则对奇数尺寸做偶数兜底。
        std::string filterChain = ScaleFilterForResolution(request.resolutionIndex);
        if (filterChain.empty() && NeedsEvenDimensionGuard(request.sourceWidth,
                                                           request.sourceHeight))
        {
            filterChain = "scale=trunc(iw/2)*2:trunc(ih/2)*2";
        }
        if (!filterChain.empty())
            AddPair(args, "-vf", filterChain);

        const std::string frameRate = FrameRateForIndex(request.frameRateIndex);
        if (!frameRate.empty())
            AddPair(args, "-r", frameRate);

        if (request.hasAudio)
        {
            AddPair(args, "-c:a", "aac");
            AddPair(args, "-b:a", std::to_string(AudioBitrateKbps(quality)) + "k");
            AddPair(args, "-ac", "2");
        }
        else
        {
            Add(args, "-an");
        }

        if (request.format == TargetFormat::MP4)
        {
            // HEVC in MP4 需要 hvc1 标记，否则苹果设备 / Win11 照片可能无法播放。
            if (IsHevc(request.codec))
                AddPair(args, "-tag:v", "hvc1");
            AddPair(args, "-movflags", "+faststart");
            AddPair(args, "-f", "mp4");
        }
        else
        {
            AddPair(args, "-f", "matroska");
        }

        Add(args, request.outputPath);
        result.ok = true;
        return result;
    }
}
