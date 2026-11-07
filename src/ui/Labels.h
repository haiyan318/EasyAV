#pragma once

// 面向用户的文案映射（统一放在一处，避免各面板重复且口吻不一致）。

#include "core/Format.h"
#include "core/TaskContracts.h"
#include "core/Types.h"
#include "model/TaskSettings.h"

#include <string>

namespace easyav::ui
{
    inline const char* SceneLabel(SceneId id)
    {
        switch (id)
        {
        case SceneId::Convert:  return "编码转换";
        case SceneId::Clip:     return "剪辑拼接";
        case SceneId::Filter:   return "滤镜水印";
        case SceneId::Record:   return "录屏截图";
        case SceneId::Batch:    return "批量处理";
        default:                return "";
        }
    }

    inline const char* SceneSubtitle(SceneId id)
    {
        switch (id)
        {
        case SceneId::Convert:  return "转码 · 瘦身";
        case SceneId::Clip:     return "裁剪 · 合并";
        case SceneId::Filter:   return "旋转 · 水印";
        case SceneId::Record:   return "屏幕 · 摄像头";
        case SceneId::Batch:    return "一键套用";
        default:                return "";
        }
    }

    inline const char* VideoCodecLabel(VideoCodec codec)
    {
        switch (codec)
        {
        case VideoCodec::H264: return "H.264";
        case VideoCodec::H265: return "H.265";
        case VideoCodec::None:
        default:               return "仅音频";
        }
    }

    // 把 ffprobe 的 codec_name 变成用户看得懂的名字。
    inline std::string SourceCodecLabel(const std::string& codec)
    {
        if (codec.empty())
            return "未知";

        const std::string lower = ToLowerAscii(codec);
        if (lower == "h264" || lower == "avc1" || lower == "avc")
            return "H.264";
        if (lower == "hevc" || lower == "h265" || lower == "hvc1" || lower == "hev1")
            return "H.265";
        if (lower == "av1" || lower == "av01")
            return "AV1";
        if (lower == "vp9")
            return "VP9";
        if (lower == "vp8")
            return "VP8";
        if (lower == "mpeg4")
            return "MPEG-4";
        if (lower == "mpeg2video")
            return "MPEG-2";
        if (lower == "flv1")
            return "FLV(Sorenson)";
        if (lower == "wmv3" || lower == "vc1")
            return "WMV";
        if (lower == "prores")
            return "ProRes";
        if (lower == "mjpeg")
            return "MJPEG";
        if (lower == "aac")
            return "AAC";
        if (lower == "mp3")
            return "MP3";
        if (lower == "opus")
            return "Opus";
        return ToUpperAscii(codec);
    }

    inline const char* ConvertTargetLabel(ConvertTarget target)
    {
        switch (target)
        {
        case ConvertTarget::H264:  return "H.264 通用";
        case ConvertTarget::H265:  return "H.265 更小";
        case ConvertTarget::Audio: return "只要声音";
        default:                   return "";
        }
    }

    inline const char* ConvertTargetSubtitle(ConvertTarget target)
    {
        switch (target)
        {
        case ConvertTarget::H264:  return "几乎所有播放器、手机和电视都能放";
        case ConvertTarget::H265:  return "画质相近、体积更小；老设备可能打不开";
        case ConvertTarget::Audio: return "输出 MP3，只保留声音";
        default:                   return "";
        }
    }

    inline const char* ConvertTargetTag(ConvertTarget target)
    {
        switch (target)
        {
        case ConvertTarget::H264:  return "推荐";
        case ConvertTarget::H265:  return "更小";
        case ConvertTarget::Audio: return "仅音频";
        default:                   return "";
        }
    }

    inline const char* ConvertTargetHint(ConvertTarget target)
    {
        switch (target)
        {
        case ConvertTarget::H264:
            return "打不开的视频，换成 H.264 通常就能正常播放。";
        case ConvertTarget::H265:
            return "H.265 更省空间；如果转换后设备仍打不开，请改用 H.264。";
        case ConvertTarget::Audio:
            return "只导出声音，不保留画面；适合提取音乐、播客或背景音。";
        default:
            return "";
        }
    }

    inline const char* ContainerLabel(TargetFormat format)
    {
        switch (format)
        {
        case TargetFormat::MKV: return "MKV 大文件";
        case TargetFormat::MP4: return "MP4 最通用";
        default:                return "";
        }
    }

    inline const char* ContainerShort(TargetFormat format)
    {
        switch (format)
        {
        case TargetFormat::MKV: return "MKV";
        case TargetFormat::MP4: return "MP4";
        case TargetFormat::MP3: return "MP3";
        default:                return "MP4";
        }
    }

    inline const char* FormatLabel(TargetFormat format)
    {
        switch (format)
        {
        case TargetFormat::MP4: return "MP4 视频";
        case TargetFormat::MKV: return "MKV 高清";
        case TargetFormat::GIF: return "GIF 动图";
        case TargetFormat::MP3: return "MP3 只要声音";
        default:                return "";
        }
    }

    inline const char* FormatShort(TargetFormat format)
    {
        switch (format)
        {
        case TargetFormat::MP4: return "MP4";
        case TargetFormat::MKV: return "MKV";
        case TargetFormat::GIF: return "GIF";
        case TargetFormat::MP3: return "MP3";
        default:                return "";
        }
    }

    inline const char* QualityLabel(float quality)
    {
        if (quality < 0.25f) return "极致压缩";
        if (quality < 0.45f) return "体积优先";
        if (quality < 0.65f) return "均衡推荐";
        if (quality < 0.85f) return "画质优先";
        return "接近原画质";
    }

    inline const char* QualityHint(float quality)
    {
        if (quality < 0.25f) return "体积最小，适合发微信、传网盘；细节会有所丢失。";
        if (quality < 0.45f) return "明显变小，日常观看基本够用。";
        if (quality < 0.65f) return "体积与画质兼顾，大多数视频推荐这个位置。";
        if (quality < 0.85f) return "体积略大，画面更清楚，适合留存。";
        return "几乎保持原画质，文件会比较大。";
    }

    inline const char* RotationLabel(Rotation rotation)
    {
        switch (rotation)
        {
        case Rotation::None:        return "不旋转";
        case Rotation::Clockwise90: return "顺时针 90°";
        case Rotation::Deg180:      return "转 180°";
        case Rotation::Deg270:      return "逆时针 90°";
        default:                    return "";
        }
    }

    inline const char* AspectLabel(AspectPreset aspect)
    {
        switch (aspect)
        {
        case AspectPreset::Original:  return "原始比例";
        case AspectPreset::Square:    return "1:1 方形";
        case AspectPreset::Landscape: return "16:9 横屏";
        case AspectPreset::Portrait:  return "9:16 竖屏";
        default:                      return "";
        }
    }

    inline const char* AspectShortLabel(AspectPreset aspect)
    {
        switch (aspect)
        {
        case AspectPreset::Original:  return "原始";
        case AspectPreset::Square:    return "1:1";
        case AspectPreset::Landscape: return "16:9";
        case AspectPreset::Portrait:  return "9:16";
        default:                      return "";
        }
    }

    inline const char* WatermarkLabel(WatermarkMode mode)
    {
        switch (mode)
        {
        case WatermarkMode::None:  return "无水印";
        case WatermarkMode::Text:  return "文字水印";
        case WatermarkMode::Image: return "图片水印";
        default:                   return "";
        }
    }

    inline const char* RecordQualityLabel(int index)
    {
        switch (index)
        {
        case 0:  return "标清";
        case 1:  return "高清";
        case 2:  return "原画";
        default: return "";
        }
    }

    inline const char* FormatUnsupportedHint(TargetFormat format)
    {
        return format == TargetFormat::GIF
                   ? "GIF 动图暂未开放，请先选择 MP4、MKV 或 MP3。"
                   : "";
    }

    inline const char* ToolStateLabel(const ToolStatus& tools)
    {
        if (!tools.ffmpegFound)
            return "处理组件未就绪";
        if (!tools.ffprobeFound)
            return "时长分析不可用";
        return "处理组件已就绪";
    }

    inline const char* KindLabel(MediaKind kind)
    {
        switch (kind)
        {
        case MediaKind::Video: return "视频";
        case MediaKind::Audio: return "音频";
        case MediaKind::Image: return "图片";
        default:               return "文件";
        }
    }
}
