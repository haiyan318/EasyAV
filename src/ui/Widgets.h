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

    bool ToggleSwitch(const char* label, bool* value);

    void CardBegin(const char* id, const ImVec2& size, const ImVec2& padding = ImVec2(16.0f, 16.0f));
    void CardEnd();

    void Badge(const char* text, const ImVec4& background, const ImVec4& foreground);

    void ProgressBar(const char* id, float fraction, const ImVec2& size, const char* overlay = nullptr);
    bool QualitySlider(const char* id, float* value, float width);
    bool TimelineRangeSlider(const char* id, float* startPct, float* endPct, float width,
                             const char* startText, const char* endText);

    void DrawDashedRect(ImDrawList* drawList, const ImVec2& min, const ImVec2& max,
                        ImU32 color, float rounding, float thickness, float dash, float gap);

    std::string Ellipsize(const std::string& text, float maxWidth);
}
