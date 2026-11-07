#pragma once

// 主题与字体。所有颜色/尺寸集中在此，面板不直接写死颜色。

#include "imgui.h"

namespace easyav::ui
{
    struct UiColors
    {
        ImVec4 bg;
        ImVec4 panel;
        ImVec4 panelAlt;
        ImVec4 panelHover;
        ImVec4 border;
        ImVec4 text;
        ImVec4 textMuted;
        ImVec4 accent;
        ImVec4 accentHover;
        ImVec4 accentActive;
        ImVec4 accentSoft;
        ImVec4 success;
        ImVec4 warning;
        ImVec4 danger;
        ImVec4 track;
        ImVec4 white;
    };

    const UiColors& Colors();

    // 与字体/样式同步的布局缩放（例如 125% / 150% DPI）。
    float UiScale();
    inline float S(float value) { return value * UiScale(); }

    void ApplyMetrics();
    void ApplyTheme(bool dark);

    // 优先加载思源黑体，其次回退到 Windows 自带中文字体。
    // 返回 false 表示未找到中文字体，已回退到 ImGui 内置字体。
    bool LoadFonts();

    inline ImU32 ColorU32(const ImVec4& color) { return ImGui::GetColorU32(color); }
    inline ImU32 ColorU32(const ImVec4& color, float alpha)
    {
        return ImGui::GetColorU32(ImVec4(color.x, color.y, color.z, color.w * alpha));
    }
}
