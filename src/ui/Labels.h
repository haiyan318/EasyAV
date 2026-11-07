#pragma once

// 面向用户的文案映射（统一放在一处，避免各面板重复且口吻不一致）。

#include "core/Types.h"

namespace easyav::ui
{
    inline const char* SceneLabel(SceneId id)
    {
        switch (id)
        {
        case SceneId::Compress: return "视频瘦身";
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
        case SceneId::Compress: return "压缩 · 转格式";
        case SceneId::Clip:     return "裁剪 · 合并";
        case SceneId::Filter:   return "旋转 · 水印";
        case SceneId::Record:   return "屏幕 · 摄像头";
        case SceneId::Batch:    return "一键套用";
        default:                return "";
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
