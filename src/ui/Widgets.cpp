#include "ui/Widgets.h"

#include "ui/Theme.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace easyav::ui
{
    namespace
    {
        ImVec2 TextSize(ImFont* font, float size, const char* text)
        {
            return font->CalcTextSizeA(size, FLT_MAX, 0.0f, text);
        }

        float Clamp01(float value)
        {
            return std::clamp(value, 0.0f, 1.0f);
        }

        void DrawPill(ImDrawList* drawList, const ImVec2& pos, const char* text,
                      const ImVec4& background, const ImVec4& foreground, float fontSize)
        {
            if (!drawList || !text || text[0] == '\0')
                return;

            const ImVec2 textSize = ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, text);
            const ImVec2 padding(S(7.0f), S(3.0f));
            const ImVec2 max(pos.x + textSize.x + padding.x * 2.0f,
                             pos.y + textSize.y + padding.y * 2.0f);
            drawList->AddRectFilled(pos, max, ImGui::GetColorU32(background), S(3.0f));
            drawList->AddText(ImGui::GetFont(), fontSize,
                              ImVec2(pos.x + padding.x, pos.y + padding.y),
                              ImGui::GetColorU32(foreground), text);
        }

        void DrawOptionIcon(ImDrawList* drawList, OptionIcon icon, const ImVec2& center,
                            float radius, ImU32 color)
        {
            const float t = std::max(radius * 0.16f, 1.4f);
            switch (icon)
            {
            case OptionIcon::Shield:
                drawList->AddCircle(center, radius * 0.56f, color, 0, t);
                drawList->AddLine(ImVec2(center.x - radius * 0.32f, center.y),
                                  ImVec2(center.x - radius * 0.06f, center.y + radius * 0.28f),
                                  color, t);
                drawList->AddLine(ImVec2(center.x - radius * 0.06f, center.y + radius * 0.28f),
                                  ImVec2(center.x + radius * 0.40f, center.y - radius * 0.26f),
                                  color, t);
                break;

            case OptionIcon::Feather:
                drawList->AddLine(ImVec2(center.x - radius * 0.38f, center.y + radius * 0.40f),
                                  ImVec2(center.x + radius * 0.36f, center.y - radius * 0.42f),
                                  color, t);
                for (int i = 0; i < 3; ++i)
                {
                    const float k = 0.25f + 0.2f * static_cast<float>(i);
                    drawList->AddLine(ImVec2(center.x - radius * 0.05f + radius * 0.4f * k,
                                             center.y + radius * 0.10f - radius * 0.5f * k),
                                      ImVec2(center.x - radius * 0.30f + radius * 0.4f * k,
                                             center.y + radius * 0.28f - radius * 0.5f * k),
                                      color, t * 0.8f);
                }
                break;

            case OptionIcon::Music:
            default:
                drawList->AddCircleFilled(ImVec2(center.x - radius * 0.20f,
                                                 center.y + radius * 0.34f),
                                          radius * 0.20f, color);
                drawList->AddLine(ImVec2(center.x - radius * 0.02f, center.y + radius * 0.32f),
                                  ImVec2(center.x - radius * 0.02f, center.y - radius * 0.42f),
                                  color, t);
                drawList->AddLine(ImVec2(center.x - radius * 0.02f, center.y - radius * 0.42f),
                                  ImVec2(center.x + radius * 0.40f, center.y - radius * 0.30f),
                                  color, t);
                break;
            }
        }
    }

    void PushFontSize(float pixels)
    {
        ImGui::PushFont(nullptr, pixels);
    }

    void PopFontSize()
    {
        ImGui::PopFont();
    }

    void SectionHeader(const char* title, const char* subtitle)
    {
        const float base = ImGui::GetFontSize();
        PushFontSize(base * 1.45f);
        ImGui::TextUnformatted(title);
        PopFontSize();

        if (subtitle && subtitle[0] != '\0')
        {
            ImGui::PushStyleColor(ImGuiCol_Text, Colors().textMuted);
            ImGui::TextWrapped("%s", subtitle);
            ImGui::PopStyleColor();
        }
        ImGui::Dummy(ImVec2(0.0f, S(4.0f)));
    }

    void Hint(const char* text)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, Colors().textMuted);
        ImGui::TextWrapped("%s", text);
        ImGui::PopStyleColor();
    }

    bool PrimaryButton(const char* label, const ImVec2& size)
    {
        const UiColors& c = Colors();
        ImGui::PushStyleColor(ImGuiCol_Button, c.accent);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, c.accentHover);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, c.accentActive);
        ImGui::PushStyleColor(ImGuiCol_Text, c.white);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, S(6.0f));
        PushFontSize(ImGui::GetFontSize());
        const bool clicked = ImGui::Button(label, size);
        PopFontSize();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(4);
        return clicked;
    }

    bool SecondaryButton(const char* label, const ImVec2& size)
    {
        const UiColors& c = Colors();
        ImGui::PushStyleColor(ImGuiCol_Button, c.panelAlt);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, c.panelHover);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, c.track);
        ImGui::PushStyleColor(ImGuiCol_Text, c.text);
        ImGui::PushStyleColor(ImGuiCol_Border, c.border);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, S(6.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
        const bool clicked = ImGui::Button(label, size);
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(5);
        return clicked;
    }

    bool ChoiceChip(const char* label, bool selected, const ImVec2& size)
    {
        const UiColors& c = Colors();
        ImGui::PushStyleColor(ImGuiCol_Button, selected ? c.selectedSoft : c.panelAlt);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, selected ? c.accentSoft : c.panelHover);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, selected ? c.accentSoft : c.track);
        ImGui::PushStyleColor(ImGuiCol_Text, selected ? c.accent : c.text);
        ImGui::PushStyleColor(ImGuiCol_Border, selected ? c.accent : c.border);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, S(6.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
        const bool clicked = ImGui::Button(label, size);
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(5);
        return clicked;
    }

    bool ToggleSwitch(const char* label, bool* value)
    {
        const UiColors& c = Colors();
        const float base = ImGui::GetFontSize();
        const float height = base * 1.06f;
        const float width = height * 1.85f;
        const ImVec2 labelSize = ImGui::CalcTextSize(label);
        const ImVec2 origin = ImGui::GetCursorScreenPos();
        const ImVec2 totalSize(width + S(10.0f) + labelSize.x, height);

        ImGui::PushID(label);
        ImGui::InvisibleButton("##toggle", totalSize);
        bool changed = false;
        if (ImGui::IsItemClicked())
        {
            *value = !*value;
            changed = true;
        }
        if (ImGui::IsItemHovered())
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        const ImU32 trackColor = ImGui::GetColorU32(*value ? c.accent : c.track);
        drawList->AddRectFilled(origin, ImVec2(origin.x + width, origin.y + height),
                                trackColor, height * 0.5f);

        const float knobRadius = height * 0.5f - S(3.0f);
        const float knobX = *value ? origin.x + width - height * 0.5f : origin.x + height * 0.5f;
        drawList->AddCircleFilled(ImVec2(knobX, origin.y + height * 0.5f), knobRadius,
                                  ImGui::GetColorU32(c.white));

        drawList->AddText(ImGui::GetFont(), base,
                          ImVec2(origin.x + width + S(10.0f), origin.y + (height - base) * 0.5f - S(1.0f)),
                          ImGui::GetColorU32(c.text), label);

        ImGui::PopID();
        return changed;
    }

    void CardBegin(const char* id, const ImVec2& size, const ImVec2& padding)
    {
        const UiColors& c = Colors();
        ImGui::PushStyleColor(ImGuiCol_ChildBg, c.panel);
        ImGui::PushStyleColor(ImGuiCol_Border, c.border);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, S(8.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, padding);
        ImGui::BeginChild(id, size, ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding,
                          ImGuiWindowFlags_None);
    }

    void CardEnd()
    {
        ImGui::EndChild();
        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(2);
    }

    void Badge(const char* text, const ImVec4& background, const ImVec4& foreground)
    {
        const ImVec2 textSize = ImGui::CalcTextSize(text);
        const ImVec2 padding(S(8.0f), S(3.0f));
        const ImVec2 size(textSize.x + padding.x * 2.0f, textSize.y + padding.y * 2.0f);
        const ImVec2 origin = ImGui::GetCursorScreenPos();
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddRectFilled(origin, ImVec2(origin.x + size.x, origin.y + size.y),
                                ImGui::GetColorU32(background), S(3.0f));
        drawList->AddText(ImVec2(origin.x + padding.x, origin.y + padding.y),
                          ImGui::GetColorU32(foreground), text);
        ImGui::Dummy(size);
    }

    void ProgressBar(const char* id, float fraction, const ImVec2& size, const char* overlay,
                     bool animated)
    {
        ImGui::PushID(id);
        const ImVec2 origin = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton("##bar", size);

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        const UiColors& c = Colors();
        const float rounding = S(3.0f);
        const ImVec2 end(origin.x + size.x, origin.y + size.y);

        // 动效只存在 ImGui 内部状态，不进入 AppModel。
        ImGuiStorage* storage = ImGui::GetStateStorage();
        float* displayed = storage->GetFloatRef(ImGui::GetID("##displayed"), 0.0f);
        const float dt = std::clamp(ImGui::GetIO().DeltaTime, 0.0f, 0.1f);
        const float target = Clamp01(fraction);
        if (animated)
            *displayed += (target - *displayed) * std::clamp(dt * 7.0f, 0.0f, 1.0f);
        else
            *displayed = target;

        drawList->AddRectFilled(origin, end, ImGui::GetColorU32(c.track), rounding);

        const float clamped = Clamp01(*displayed);
        if (clamped > 0.0f)
        {
            float filledWidth = size.x * clamped;
            filledWidth = std::max(filledWidth, size.y);
            drawList->AddRectFilled(origin, ImVec2(origin.x + filledWidth, origin.y + size.y),
                                    ImGui::GetColorU32(c.accent), rounding);
        }

        if (overlay && overlay[0] != '\0')
        {
            const float fontSize = ImGui::GetFontSize();
            const ImVec2 textSize = TextSize(ImGui::GetFont(), fontSize, overlay);
            const ImVec2 textPos(origin.x + (size.x - textSize.x) * 0.5f,
                                 origin.y + (size.y - textSize.y) * 0.5f);
            const ImU32 textColor = clamped > 0.55f ? ImGui::GetColorU32(c.white)
                                                    : ImGui::GetColorU32(c.textMuted);
            drawList->AddText(ImGui::GetFont(), fontSize, textPos, textColor, overlay);
        }

        ImGui::PopID();
    }

    bool QualitySlider(const char* id, float* value, float width)
    {
        const UiColors& c = Colors();
        const float base = ImGui::GetFontSize();
        const float height = S(54.0f);
        const float pad = S(10.0f);
        const float knobRadius = S(9.0f);
        const float trackHeight = S(8.0f);

        ImGui::PushID(id);
        const ImVec2 origin = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton("##slider", ImVec2(width, height));

        bool changed = false;
        const float trackLeft = origin.x + pad;
        const float trackRight = origin.x + width - pad;
        if (ImGui::IsItemActive())
        {
            const float t = Clamp01((ImGui::GetIO().MousePos.x - trackLeft) / (trackRight - trackLeft));
            if (std::fabs(t - *value) > 0.0005f)
            {
                *value = t;
                changed = true;
            }
        }
        if (ImGui::IsItemHovered())
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        const float centerY = origin.y + S(14.0f);
        const ImVec2 trackMin(trackLeft, centerY - trackHeight * 0.5f);
        const ImVec2 trackMax(trackRight, centerY + trackHeight * 0.5f);

        drawList->AddRectFilled(trackMin, trackMax, ImGui::GetColorU32(c.track), trackHeight * 0.5f);
        drawList->AddRectFilledMultiColor(trackMin, trackMax,
                                          ImGui::GetColorU32(c.warning), ImGui::GetColorU32(c.accent),
                                          ImGui::GetColorU32(c.accent), ImGui::GetColorU32(c.warning));

        const float knobX = trackLeft + Clamp01(*value) * (trackRight - trackLeft);
        drawList->AddCircleFilled(ImVec2(knobX, centerY), knobRadius + 2.5f,
                                  ImGui::GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 0.10f)));
        drawList->AddCircleFilled(ImVec2(knobX, centerY), knobRadius + 1.0f,
                                  ImGui::GetColorU32(c.white));
        drawList->AddCircle(ImVec2(knobX, centerY), knobRadius + 1.0f,
                            ImGui::GetColorU32(c.accent), 0, 2.5f);

        const float labelSize = base * 0.78f;
        drawList->AddText(ImGui::GetFont(), labelSize, ImVec2(trackLeft, centerY + S(16.0f)),
                          ImGui::GetColorU32(c.textMuted), "体积优先");
        const ImVec2 rightSize = TextSize(ImGui::GetFont(), labelSize, "画质优先");
        drawList->AddText(ImGui::GetFont(), labelSize,
                          ImVec2(trackRight - rightSize.x, centerY + S(16.0f)),
                          ImGui::GetColorU32(c.textMuted), "画质优先");

        ImGui::PopID();
        return changed;
    }

    bool TimelineRangeSlider(const char* id, float* startPct, float* endPct, float width,
                             const char* startText, const char* endText)
    {
        const UiColors& c = Colors();
        const float base = ImGui::GetFontSize();
        const float pad = S(12.0f);
        const float trackHeight = S(46.0f);
        const float totalHeight = trackHeight + S(26.0f);

        ImGui::PushID(id);
        const ImVec2 origin = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton("##timeline", ImVec2(width, totalHeight));

        const float trackLeft = origin.x + pad;
        const float trackRight = origin.x + width - pad;
        const float trackWidth = std::max(trackRight - trackLeft, 1.0f);

        bool changed = false;
        ImGuiStorage* storage = ImGui::GetStateStorage();
        const ImGuiID handleKey = ImGui::GetID("##handle");

        if (ImGui::IsItemActivated())
        {
            const float mouseX = ImGui::GetIO().MousePos.x;
            const float startX = trackLeft + *startPct * trackWidth;
            const float endX = trackLeft + *endPct * trackWidth;
            storage->SetInt(handleKey, std::fabs(mouseX - startX) <= std::fabs(mouseX - endX) ? 0 : 1);
        }

        if (ImGui::IsItemActive())
        {
            const int handle = storage->GetInt(handleKey, 0);
            const float t = Clamp01((ImGui::GetIO().MousePos.x - trackLeft) / trackWidth);
            if (handle == 0)
                *startPct = std::min(t, *endPct - 0.02f);
            else
                *endPct = std::max(t, *startPct + 0.02f);
            *startPct = Clamp01(*startPct);
            *endPct = Clamp01(*endPct);
            changed = true;
        }
        if (ImGui::IsItemHovered())
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        const ImVec2 trackMin(trackLeft, origin.y + 2.0f);
        const ImVec2 trackMax(trackRight, origin.y + 2.0f + trackHeight);

        drawList->AddRectFilled(trackMin, trackMax, ImGui::GetColorU32(c.track), 9.0f);

        // 模拟胶片格，让"时间轴"更直观。
        const int tileCount = std::max(2, static_cast<int>(trackWidth / S(52.0f)));
        const float tileGap = S(3.0f);
        const float tileWidth = (trackWidth - tileGap * (tileCount + 1)) / static_cast<float>(tileCount);
        for (int i = 0; i < tileCount; ++i)
        {
            const float x0 = trackLeft + tileGap + static_cast<float>(i) * (tileWidth + tileGap);
            const ImVec2 a(x0, trackMin.y + S(5.0f));
            const ImVec2 b(x0 + tileWidth, trackMax.y - S(5.0f));
            const ImU32 color = ImGui::GetColorU32((i % 2 == 0) ? c.panelHover : c.panelAlt);
            drawList->AddRectFilled(a, b, color, 5.0f);
        }

        const float startX = trackLeft + Clamp01(*startPct) * trackWidth;
        const float endX = trackLeft + Clamp01(*endPct) * trackWidth;

        // 未选区域压暗
        const ImU32 dimColor = ImGui::GetColorU32(ImVec4(0.06f, 0.08f, 0.11f, 0.32f));
        if (startX > trackLeft)
            drawList->AddRectFilled(trackMin, ImVec2(startX, trackMax.y), dimColor, 9.0f);
        if (endX < trackRight)
            drawList->AddRectFilled(ImVec2(endX, trackMin.y), trackMax, dimColor, 9.0f);

        drawList->AddRect(ImVec2(startX, trackMin.y), ImVec2(endX, trackMax.y),
                          ImGui::GetColorU32(c.accent), 9.0f, 0, 2.0f);

        // 双手柄
        auto drawHandle = [&](float x)
        {
            const ImVec2 a(x - S(5.0f), trackMin.y - S(3.0f));
            const ImVec2 b(x + S(5.0f), trackMax.y + S(3.0f));
            drawList->AddRectFilled(ImVec2(a.x - S(1.5f), a.y - S(1.5f)), ImVec2(b.x + S(1.5f), b.y + S(1.5f)),
                                    ImGui::GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 0.12f)), 6.0f);
            drawList->AddRectFilled(a, b, ImGui::GetColorU32(c.white), 5.0f);
            drawList->AddRect(a, b, ImGui::GetColorU32(c.accent), 5.0f, 0, 2.0f);
            drawList->AddLine(ImVec2(x, a.y + S(14.0f)), ImVec2(x, b.y - S(14.0f)),
                              ImGui::GetColorU32(c.border), 1.5f);
        };
        drawHandle(startX);
        drawHandle(endX);

        const float labelSize = base * 0.78f;
        const ImVec2 startSize = TextSize(ImGui::GetFont(), labelSize, startText ? startText : "");
        float labelY = trackMax.y + S(7.0f);
        float startLabelX = std::clamp(startX - startSize.x * 0.5f, trackLeft, trackRight - startSize.x);
        drawList->AddText(ImGui::GetFont(), labelSize, ImVec2(startLabelX, labelY),
                          ImGui::GetColorU32(c.textMuted), startText ? startText : "");

        if (endText)
        {
            const ImVec2 endSize = TextSize(ImGui::GetFont(), labelSize, endText);
            float endLabelX = std::clamp(endX - endSize.x * 0.5f, trackLeft, trackRight - endSize.x);
            drawList->AddText(ImGui::GetFont(), labelSize, ImVec2(endLabelX, labelY),
                              ImGui::GetColorU32(c.textMuted), endText);
        }

        ImGui::PopID();
        return changed;
    }

    void DrawDashedRect(ImDrawList* drawList, const ImVec2& min, const ImVec2& max,
                        ImU32 color, float rounding, float thickness, float dash, float gap)
    {
        (void)rounding;
        if (!drawList)
            return;

        const float step = std::max(dash + gap, 1.0f);

        for (float x = min.x; x < max.x; x += step)
        {
            const float x1 = std::min(x + dash, max.x);
            drawList->AddLine(ImVec2(x, min.y), ImVec2(x1, min.y), color, thickness);
            drawList->AddLine(ImVec2(x, max.y), ImVec2(x1, max.y), color, thickness);
        }

        for (float y = min.y; y < max.y; y += step)
        {
            const float y1 = std::min(y + dash, max.y);
            drawList->AddLine(ImVec2(min.x, y), ImVec2(min.x, y1), color, thickness);
            drawList->AddLine(ImVec2(max.x, y), ImVec2(max.x, y1), color, thickness);
        }
    }

    std::string Ellipsize(const std::string& text, float maxWidth)
    {
        if (maxWidth <= 0.0f || ImGui::CalcTextSize(text.c_str()).x <= maxWidth)
            return text;

        const char* ellipsis = "…";
        const float ellipsisWidth = ImGui::CalcTextSize(ellipsis).x;
        const float budget = std::max(maxWidth - ellipsisWidth, 0.0f);

        std::string result;
        result.reserve(text.size());
        for (std::size_t i = 0; i < text.size();)
        {
            const unsigned char lead = static_cast<unsigned char>(text[i]);
            std::size_t length = 1;
            if ((lead & 0xE0) == 0xC0) length = 2;
            else if ((lead & 0xF0) == 0xE0) length = 3;
            else if ((lead & 0xF8) == 0xF0) length = 4;
            length = std::min(length, text.size() - i);

            const std::string candidate = result + text.substr(i, length);
            if (ImGui::CalcTextSize(candidate.c_str()).x > budget)
                break;

            result += text.substr(i, length);
            i += length;
        }

        result += ellipsis;
        return result;
    }

    bool AccentButton(const char* label, const ImVec2& size, bool enabled)
    {
        const UiColors& c = Colors();
        const float base = ImGui::GetFontSize();

        ImGui::PushID(label);
        const ImVec2 origin = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton("##accent", size, ImGuiButtonFlags_None);

        const bool hovered = ImGui::IsItemHovered() && enabled;
        const bool held = ImGui::IsItemActive() && enabled;
        const bool clicked = ImGui::IsItemClicked() && enabled;
        if (hovered)
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

        ImVec2 min = origin;
        ImVec2 max(origin.x + size.x, origin.y + size.y);
        if (held)
        {
            min.y += S(1.0f);
            max.y += S(1.0f);
        }

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        const float rounding = S(6.0f);

        ImVec4 fill = c.track;
        if (enabled)
            fill = held ? c.accentActive : (hovered ? c.accentHover : c.accent);

        drawList->AddRectFilled(min, max, ImGui::GetColorU32(fill), rounding);
        if (enabled)
            drawList->AddRect(min, max, ImGui::GetColorU32(c.accentActive), rounding, 0, 1.0f);

        const ImVec2 textSize = TextSize(ImGui::GetFont(), base, label);
        drawList->AddText(ImGui::GetFont(), base,
                          ImVec2(min.x + (size.x - textSize.x) * 0.5f,
                                 min.y + (size.y - textSize.y) * 0.5f),
                          ImGui::GetColorU32(enabled ? c.white : c.textMuted), label);

        ImGui::PopID();
        return clicked;
    }

    bool OptionCard(const char* id, const char* title, const char* subtitle,
                    const char* tag, bool selected, const ImVec2& size, OptionIcon icon)
    {
        const UiColors& c = Colors();
        const float base = ImGui::GetFontSize();

        ImGui::PushID(id);
        const ImVec2 origin = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton("##option", size, ImGuiButtonFlags_None);

        const bool hovered = ImGui::IsItemHovered();
        const bool clicked = ImGui::IsItemClicked();
        if (hovered)
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

        const ImVec2 min = origin;
        const ImVec2 max(origin.x + size.x, origin.y + size.y);
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        const float rounding = S(6.0f);

        const ImVec4 background = selected ? c.selectedSoft
                                           : (hovered ? c.panelHover : c.panelAlt);
        drawList->AddRectFilled(min, max, ImGui::GetColorU32(background), rounding);

        const ImVec4 border = selected ? c.accent : (hovered ? c.cardHover : c.border);
        drawList->AddRect(min, max, ImGui::GetColorU32(border), rounding, 0, 1.0f);

        // 图标使用小圆角方块，避免整套界面都由圆形/胶囊构成。
        const ImVec2 tileMin(min.x + S(14.0f), min.y + S(16.0f));
        const ImVec2 tileMax(tileMin.x + S(28.0f), tileMin.y + S(28.0f));
        drawList->AddRectFilled(tileMin, tileMax,
                                ImGui::GetColorU32(selected ? c.accentSoft : c.panel),
                                S(5.0f));
        DrawOptionIcon(drawList, icon,
                       ImVec2((tileMin.x + tileMax.x) * 0.5f,
                              (tileMin.y + tileMax.y) * 0.5f),
                       S(13.0f), ImGui::GetColorU32(c.accent));

        const float tagFont = base * 0.69f;
        float tagWidth = 0.0f;
        if (tag && tag[0] != '\0')
        {
            const ImVec2 tagSize = ImGui::GetFont()->CalcTextSizeA(tagFont, FLT_MAX, 0.0f, tag);
            tagWidth = tagSize.x + S(14.0f);
            DrawPill(drawList, ImVec2(max.x - tagWidth - S(12.0f), min.y + S(12.0f)), tag,
                     c.accentSoft, c.accent, tagFont);
        }

        const float titleX = min.x + S(52.0f);
        const float titleWidth = std::max(max.x - titleX - tagWidth - S(18.0f), S(40.0f));
        const std::string shownTitle = Ellipsize(title ? title : "", titleWidth);
        drawList->AddText(ImGui::GetFont(), base * 1.02f,
                          ImVec2(titleX, min.y + S(17.0f)),
                          ImGui::GetColorU32(selected ? c.accent : c.text),
                          shownTitle.c_str());

        if (subtitle && subtitle[0] != '\0')
        {
            drawList->AddText(ImGui::GetFont(), base * 0.75f,
                              ImVec2(min.x + S(15.0f), min.y + S(54.0f)),
                              ImGui::GetColorU32(c.textMuted), subtitle, nullptr,
                              size.x - S(30.0f));
        }

        if (selected)
        {
            const ImVec2 check(max.x - S(16.0f), max.y - S(14.0f));
            drawList->AddLine(ImVec2(check.x - S(4.0f), check.y),
                              ImVec2(check.x - S(1.0f), check.y + S(3.0f)),
                              ImGui::GetColorU32(c.accent), S(1.8f));
            drawList->AddLine(ImVec2(check.x - S(1.0f), check.y + S(3.0f)),
                              ImVec2(check.x + S(5.0f), check.y - S(4.0f)),
                              ImGui::GetColorU32(c.accent), S(1.8f));
        }

        ImGui::PopID();
        return clicked;
    }

    void StepHeader(int index, const char* title, const char* hint)
    {
        const UiColors& c = Colors();
        const float base = ImGui::GetFontSize();
        const float circle = S(22.0f);
        const ImVec2 origin = ImGui::GetCursorScreenPos();
        const float width = ImGui::GetContentRegionAvail().x;

        ImGui::Dummy(ImVec2(width, circle));

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        const ImVec2 center(origin.x + circle * 0.5f, origin.y + circle * 0.5f);
        drawList->AddCircleFilled(center, circle * 0.5f, ImGui::GetColorU32(c.accentSoft));

        char number[8] = {};
        std::snprintf(number, sizeof(number), "%d", index);
        const ImVec2 numberSize = TextSize(ImGui::GetFont(), base * 0.78f, number);
        drawList->AddText(ImGui::GetFont(), base * 0.78f,
                          ImVec2(center.x - numberSize.x * 0.5f,
                                 center.y - numberSize.y * 0.5f),
                          ImGui::GetColorU32(c.accent), number);

        const float titleSize = base * 1.04f;
        drawList->AddText(ImGui::GetFont(), titleSize,
                          ImVec2(origin.x + circle + S(9.0f),
                                 origin.y + (circle - titleSize) * 0.5f - S(1.0f)),
                          ImGui::GetColorU32(c.text), title);

        if (hint && hint[0] != '\0')
        {
            const float hintSize = base * 0.72f;
            const ImVec2 hintTextSize = TextSize(ImGui::GetFont(), hintSize, hint);
            const float hintX = origin.x + width - hintTextSize.x;
            if (hintX > origin.x + circle + S(120.0f))
            {
                drawList->AddText(ImGui::GetFont(), hintSize,
                                  ImVec2(hintX, origin.y + (circle - hintTextSize.y) * 0.5f),
                                  ImGui::GetColorU32(c.textMuted), hint);
            }
        }
    }
}

