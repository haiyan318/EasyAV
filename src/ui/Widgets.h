#pragma once

// 通用 UI 控件：不含任何业务状态，也不引用 AppModel。
// 各面板通过组合这些控件保持一致的外观与交互。

#include "imgui.h"

#include <string>

namespace easyav::ui
{
    void PushFontSize(float pixels);
    void PopFontSize();

    void SectionHeader(const char* title, const char* subtitle = nullptr);
    void Hint(const char* text);

    bool PrimaryButton(const char* label, const ImVec2& size = ImVec2(0.0f, 0.0f));
    bool SecondaryButton(const char* label, const ImVec2& size = ImVec2(0.0f, 0.0f));
    bool ChoiceChip(const char* label, bool selected, const ImVec2& size = ImVec2(0.0f, 0.0f));

    // 主操作按钮：实色、6px 圆角；enabled=false 时自绘为不可用状态。
    bool AccentButton(const char* label, const ImVec2& size, bool enabled = true);

    enum class OptionIcon
    {
        Shield = 0,
        Feather,
        Music
    };

    // 大号可选卡片：标题 + 一句话说明 + 右上角标签。
    bool OptionCard(const char* id, const char* title, const char* subtitle,
                    const char* tag, bool selected, const ImVec2& size,
                    OptionIcon icon);

    // 带圆形序号的步骤标题（1/2/3）。
    void StepHeader(int index, const char* title, const char* hint = nullptr);

    bool ToggleSwitch(const char* label, bool* value);

    // 整行可点击的复选框：18px 方框 + 标签，选中时主色实心、白色对勾。
    // hint 非空时在行尾右对齐显示（空间不足则省略）。
    bool CheckRow(const char* label, bool* value, const char* hint = nullptr);

    void CardBegin(const char* id, const ImVec2& size, const ImVec2& padding = ImVec2(16.0f, 16.0f));
    void CardEnd();

    void Badge(const char* text, const ImVec4& background, const ImVec4& foreground);

    void ProgressBar(const char* id, float fraction, const ImVec2& size,
                     const char* overlay = nullptr, bool animated = false);
    bool QualitySlider(const char* id, float* value, float width);
    bool TimelineRangeSlider(const char* id, float* startPct, float* endPct, float width,
                             const char* startText, const char* endText);

    void DrawDashedRect(ImDrawList* drawList, const ImVec2& min, const ImVec2& max,
                        ImU32 color, float rounding, float thickness, float dash, float gap);

    std::string Ellipsize(const std::string& text, float maxWidth);
}
