#include "ui/panels/SceneTabs.h"

#include "ui/Labels.h"
#include "ui/Theme.h"
#include "ui/Widgets.h"
#include "ui/panels/BatchPanel.h"
#include "ui/panels/ClipPanel.h"
#include "ui/panels/ConvertPanel.h"
#include "ui/panels/FilterPanel.h"
#include "ui/panels/RecordPanel.h"

#include <algorithm>

namespace easyav::ui
{
    namespace
    {
        void DrawSceneIcon(ImDrawList* drawList, SceneId id, const ImVec2& center, float radius, ImU32 color)
        {
            const float t = std::max(radius * 0.14f, 1.4f);

            switch (id)
            {
            case SceneId::Convert:
                // 上下两条反向箭头，表达“换编码”。
                drawList->AddLine(ImVec2(center.x - radius * 0.48f, center.y - radius * 0.26f),
                                  ImVec2(center.x + radius * 0.42f, center.y - radius * 0.26f),
                                  color, t);
                drawList->AddTriangleFilled(
                    ImVec2(center.x + radius * 0.44f, center.y - radius * 0.26f),
                    ImVec2(center.x + radius * 0.18f, center.y - radius * 0.48f),
                    ImVec2(center.x + radius * 0.18f, center.y - radius * 0.04f), color);
                drawList->AddLine(ImVec2(center.x + radius * 0.48f, center.y + radius * 0.30f),
                                  ImVec2(center.x - radius * 0.42f, center.y + radius * 0.30f),
                                  color, t);
                drawList->AddTriangleFilled(
                    ImVec2(center.x - radius * 0.44f, center.y + radius * 0.30f),
                    ImVec2(center.x - radius * 0.18f, center.y + radius * 0.52f),
                    ImVec2(center.x - radius * 0.18f, center.y + radius * 0.08f), color);
                break;

            case SceneId::Clip:
                drawList->AddLine(ImVec2(center.x - radius * 0.45f, center.y - radius * 0.5f),
                                  ImVec2(center.x - radius * 0.45f, center.y + radius * 0.5f), color, t);
                drawList->AddLine(ImVec2(center.x + radius * 0.45f, center.y - radius * 0.5f),
                                  ImVec2(center.x + radius * 0.45f, center.y + radius * 0.5f), color, t);
                drawList->AddTriangleFilled(ImVec2(center.x - radius * 0.16f, center.y - radius * 0.3f),
                                            ImVec2(center.x - radius * 0.16f, center.y + radius * 0.3f),
                                            ImVec2(center.x + radius * 0.22f, center.y), color);
                break;

            case SceneId::Filter:
                drawList->AddCircle(center, radius * 0.5f, color, 0, t);
                drawList->AddCircleFilled(ImVec2(center.x + radius * 0.2f, center.y - radius * 0.18f),
                                          radius * 0.16f, color);
                break;

            case SceneId::Record:
                drawList->AddCircle(center, radius * 0.55f, color, 0, t);
                drawList->AddCircleFilled(center, radius * 0.24f, color);
                break;

            case SceneId::Batch:
            default:
                for (int i = -1; i <= 1; ++i)
                {
                    const float width = radius * (i == 0 ? 0.8f : 0.56f);
                    const float y = center.y + static_cast<float>(i) * radius * 0.34f;
                    drawList->AddRectFilled(ImVec2(center.x - width * 0.5f, y - t * 0.5f),
                                            ImVec2(center.x + width * 0.5f, y + t * 0.5f),
                                            color, t);
                }
                break;
            }
        }

        void DrawSceneTab(AppModel& model, SceneId id, const ImVec2& size)
        {
            const UiColors& c = Colors();
            const bool selected = model.Ui().scene == id;
            const float base = ImGui::GetFontSize();

            ImGui::PushID(static_cast<int>(id));
            const ImVec2 origin = ImGui::GetCursorScreenPos();
            ImGui::InvisibleButton("##scene", size);
            const bool hovered = ImGui::IsItemHovered();
            if (ImGui::IsItemClicked())
                model.Ui().scene = id;
            if (hovered)
                ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

            ImDrawList* drawList = ImGui::GetWindowDrawList();
            const ImVec2 max(origin.x + size.x, origin.y + size.y);
            const float rounding = S(12.0f);

            if (selected)
            {
                for (int layer = 3; layer >= 1; --layer)
                {
                    const float spread = S(1.4f) * static_cast<float>(layer);
                    ImVec4 glow = c.accentGlow;
                    glow.w *= 0.32f;
                    drawList->AddRectFilled(ImVec2(origin.x - spread, origin.y - spread),
                                            ImVec2(max.x + spread, max.y + spread),
                                            ImGui::GetColorU32(glow), rounding + spread);
                }
            }

            const ImU32 background = ImGui::GetColorU32(selected ? c.accentSoft
                                                                 : (hovered ? c.panelHover : c.panel));
            drawList->AddRectFilled(origin, max, background, rounding);
            drawList->AddRect(origin, max, ImGui::GetColorU32(selected ? c.accent : c.border),
                              rounding, 0, selected ? 2.0f : 1.0f);

            if (selected)
            {
                drawList->AddRectFilled(
                    ImVec2(origin.x + size.x * 0.5f - S(18.0f), max.y - S(3.0f)),
                    ImVec2(origin.x + size.x * 0.5f + S(18.0f), max.y - S(1.0f)),
                    ImGui::GetColorU32(c.accent), S(1.5f));
            }

            const bool compact = size.x < S(165.0f);
            const float iconX = compact ? S(24.0f) : S(30.0f);
            const float iconRadius = compact ? S(15.0f) : S(17.0f);
            const float textX = compact ? S(46.0f) : S(54.0f);
            const float titleSize = compact ? base * 0.88f : base;
            const float subtitleSize = compact ? base * 0.66f : base * 0.74f;

            const ImVec2 iconCenter(origin.x + iconX, origin.y + size.y * 0.5f);
            drawList->AddCircleFilled(iconCenter, iconRadius,
                                      ImGui::GetColorU32(selected ? c.white : c.panelAlt));
            DrawSceneIcon(drawList, id, iconCenter, iconRadius,
                          ImGui::GetColorU32(selected ? c.accent : c.textMuted));

            drawList->PushClipRect(ImVec2(origin.x + S(3.0f), origin.y),
                                   ImVec2(max.x - S(5.0f), max.y - S(5.0f)), true);
            drawList->AddText(ImGui::GetFont(), titleSize,
                              ImVec2(origin.x + textX, origin.y + S(16.0f)),
                              ImGui::GetColorU32(c.text), SceneLabel(id));
            drawList->AddText(ImGui::GetFont(), subtitleSize,
                              ImVec2(origin.x + textX, origin.y + S(39.0f)),
                              ImGui::GetColorU32(c.textMuted), SceneSubtitle(id));
            drawList->PopClipRect();

            ImGui::PopID();
        }
    }

    void DrawSceneTabs(AppModel& model)
    {
        const float gap = S(10.0f);
        const float width = ImGui::GetContentRegionAvail().x;
        const float cardHeight = S(72.0f);
        const float cardWidth = std::max((width - gap * 4.0f) / 5.0f, S(96.0f));

        for (int i = 0; i < kSceneCount; ++i)
        {
            DrawSceneTab(model, static_cast<SceneId>(i), ImVec2(cardWidth, cardHeight));
            if (i + 1 < kSceneCount)
                ImGui::SameLine(0.0f, gap);
        }

        ImGui::Dummy(ImVec2(0.0f, S(4.0f)));

        const float contentHeight = std::max(ImGui::GetContentRegionAvail().y, S(120.0f));
        CardBegin("##scene_content", ImVec2(0.0f, contentHeight), ImVec2(S(20.0f), S(18.0f)));

        switch (model.Ui().scene)
        {
        case SceneId::Convert:  DrawConvertPanel(model); break;
        case SceneId::Clip:     DrawClipPanel(model); break;
        case SceneId::Filter:   DrawFilterPanel(model); break;
        case SceneId::Record:   DrawRecordPanel(model); break;
        case SceneId::Batch:    DrawBatchPanel(model); break;
        default:                break;
        }

        CardEnd();
    }
}
