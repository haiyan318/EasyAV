#include "ui/panels/FilterPanel.h"

#include "core/Format.h"
#include "ui/Labels.h"
#include "ui/Theme.h"
#include "ui/Widgets.h"

#include <algorithm>
#include <cstdio>
#include <string>

namespace easyav::ui
{
    namespace
    {
        float PreviewRatio(AspectPreset aspect)
        {
            switch (aspect)
            {
            case AspectPreset::Square:    return 1.0f;
            case AspectPreset::Portrait:  return 9.0f / 16.0f;
            case AspectPreset::Landscape:
            case AspectPreset::Original:
            default:                      return 16.0f / 9.0f;
            }
        }

        void DrawPreview(AppModel& model, float width)
        {
            FilterSettings& settings = model.Filter();
            const UiColors& c = Colors();
            const float base = ImGui::GetFontSize();
            const float ratio = PreviewRatio(settings.aspect);

            const float maxHeight = S(215.0f);
            float previewWidth = std::min(width, maxHeight * ratio);
            float previewHeight = previewWidth / ratio;

            const ImVec2 rowOrigin = ImGui::GetCursorScreenPos();
            const ImVec2 origin(rowOrigin.x + (width - previewWidth) * 0.5f, rowOrigin.y);
            const ImVec2 max(origin.x + previewWidth, origin.y + previewHeight);

            ImGui::SetCursorScreenPos(origin);
            ImGui::InvisibleButton("##preview", ImVec2(previewWidth, previewHeight));
            const bool hovered = ImGui::IsItemHovered();
            if (hovered)
                ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0))
            {
                const ImVec2 mouse = ImGui::GetIO().MousePos;
                settings.watermarkX = std::clamp((mouse.x - origin.x) / previewWidth, 0.04f, 0.96f);
                settings.watermarkY = std::clamp((mouse.y - origin.y) / previewHeight, 0.07f, 0.93f);
            }

            ImDrawList* drawList = ImGui::GetWindowDrawList();
            drawList->AddRectFilled(ImVec2(origin.x - S(1.0f), origin.y - S(1.0f)),
                                    ImVec2(max.x + S(1.0f), max.y + S(1.0f)),
                                    ImGui::GetColorU32(c.border), 13.0f);
            drawList->AddRectFilled(origin, max, IM_COL32(24, 28, 36, 255), 12.0f);

            drawList->PushClipRect(origin, max, true);
            drawList->AddRectFilledMultiColor(origin, max,
                                              IM_COL32(58, 86, 128, 255), IM_COL32(58, 86, 128, 255),
                                              IM_COL32(24, 28, 36, 255), IM_COL32(24, 28, 36, 255));
            drawList->AddCircleFilled(ImVec2(origin.x + previewWidth * 0.76f, origin.y + previewHeight * 0.26f),
                                      previewHeight * 0.07f, IM_COL32(255, 214, 140, 190));

            // 简化的山与地面，仅用于示意画面
            const ImVec2 mountain1[] = {
                ImVec2(origin.x + previewWidth * 0.05f, origin.y + previewHeight * 0.86f),
                ImVec2(origin.x + previewWidth * 0.30f, origin.y + previewHeight * 0.42f),
                ImVec2(origin.x + previewWidth * 0.56f, origin.y + previewHeight * 0.86f)
            };
            const ImVec2 mountain2[] = {
                ImVec2(origin.x + previewWidth * 0.38f, origin.y + previewHeight * 0.86f),
                ImVec2(origin.x + previewWidth * 0.66f, origin.y + previewHeight * 0.55f),
                ImVec2(origin.x + previewWidth * 0.95f, origin.y + previewHeight * 0.86f)
            };
            drawList->AddConvexPolyFilled(mountain1, 3, IM_COL32(38, 54, 78, 255));
            drawList->AddConvexPolyFilled(mountain2, 3, IM_COL32(45, 64, 92, 255));
            drawList->AddRectFilled(ImVec2(origin.x, origin.y + previewHeight * 0.86f), max,
                                    IM_COL32(20, 25, 33, 255));
            drawList->PopClipRect();

            drawList->AddRect(origin, max, ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 0.10f)),
                              12.0f, 0, 1.0f);

            const char* tag = "预览示意";
            const ImVec2 tagSize = ImGui::GetFont()->CalcTextSizeA(base * 0.72f, FLT_MAX, 0.0f, tag);
            drawList->AddRectFilled(ImVec2(origin.x + S(12.0f), origin.y + S(12.0f)),
                                    ImVec2(origin.x + S(12.0f) + tagSize.x + S(16.0f), origin.y + S(12.0f) + tagSize.y + S(10.0f)),
                                    ImGui::GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 0.35f)), 8.0f);
            drawList->AddText(ImGui::GetFont(), base * 0.72f,
                              ImVec2(origin.x + S(20.0f), origin.y + S(17.0f)),
                              ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 0.75f)), tag);

            // 水印
            if (settings.watermark != WatermarkMode::None)
            {
                const float alpha = std::clamp(settings.watermarkOpacity, 0.05f, 1.0f);
                const float centerX = origin.x + settings.watermarkX * previewWidth;
                const float centerY = origin.y + settings.watermarkY * previewHeight;

                ImVec2 boxMin;
                ImVec2 boxMax;
                if (settings.watermark == WatermarkMode::Text)
                {
                    const char* text = settings.watermarkText[0] != '\0' ? settings.watermarkText : "水印";
                    float fontSize = previewHeight * 0.13f * (settings.watermarkScale / 0.20f);
                    fontSize = std::clamp(fontSize, 9.0f, previewHeight * 0.40f);
                    const ImVec2 textSize = ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, text);
                    const ImVec2 textPos(centerX - textSize.x * 0.5f, centerY - textSize.y * 0.5f);
                    boxMin = ImVec2(textPos.x - S(6.0f), textPos.y - S(4.0f));
                    boxMax = ImVec2(textPos.x + textSize.x + S(6.0f), textPos.y + textSize.y + S(4.0f));

                    drawList->AddText(ImGui::GetFont(), fontSize, textPos,
                                      ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, alpha)), text);
                }
                else
                {
                    const float boxWidth = previewWidth * settings.watermarkScale * 1.4f;
                    const float boxHeight = boxWidth * 0.62f;
                    boxMin = ImVec2(centerX - boxWidth * 0.5f, centerY - boxHeight * 0.5f);
                    boxMax = ImVec2(centerX + boxWidth * 0.5f, centerY + boxHeight * 0.5f);

                    drawList->AddRectFilled(boxMin, boxMax,
                                            ImGui::GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 0.18f * alpha)), 5.0f);
                    drawList->AddRect(boxMin, boxMax,
                                      ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, alpha)), 5.0f, 0, 1.5f);
                    const char* imageTag = "图片水印";
                    const float tagFont = std::max(base * 0.72f, 10.0f);
                    const ImVec2 tagBox = ImGui::GetFont()->CalcTextSizeA(tagFont, FLT_MAX, 0.0f, imageTag);
                    drawList->AddText(ImGui::GetFont(), tagFont,
                                      ImVec2((boxMin.x + boxMax.x - tagBox.x) * 0.5f,
                                             (boxMin.y + boxMax.y - tagBox.y) * 0.5f),
                                      ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, alpha)), imageTag);
                }

                const float handleAlpha = hovered ? 0.85f : 0.35f;
                DrawDashedRect(drawList, ImVec2(boxMin.x - S(4.0f), boxMin.y - S(4.0f)),
                               ImVec2(boxMax.x + S(4.0f), boxMax.y + S(4.0f)),
                               ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, handleAlpha)),
                               6.0f, 1.0f, 6.0f, 4.0f);
            }
            else
            {
                const char* hint = "选择水印后，可在这里直接拖动位置";
                const float hintFont = base * 0.76f;
                const ImVec2 hintSize = ImGui::GetFont()->CalcTextSizeA(hintFont, FLT_MAX, 0.0f, hint);
                drawList->AddText(ImGui::GetFont(), hintFont,
                                  ImVec2((origin.x + max.x - hintSize.x) * 0.5f, max.y - S(26.0f)),
                                  ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 0.55f)), hint);
            }

            ImGui::SetCursorScreenPos(ImVec2(rowOrigin.x, rowOrigin.y + previewHeight + S(6.0f)));
        }
    }

    void DrawFilterPanel(AppModel& model)
    {
        FilterSettings& settings = model.Filter();
        const UiColors& c = Colors();
        const float width = ImGui::GetContentRegionAvail().x;

        SectionHeader("滤镜与美化", "转一转、裁一裁，再加个水印。");

        ImGui::TextUnformatted("预览");
        ImGui::SameLine(0.0f, 10.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, c.textMuted);
        ImGui::TextUnformatted("先选水印，再按住画面里的水印拖动位置");
        ImGui::PopStyleColor();

        DrawPreview(model, width);

        if (SecondaryButton("把水印放回右下角", ImVec2(S(160.0f), S(32.0f))))
        {
            settings.watermarkX = 0.82f;
            settings.watermarkY = 0.82f;
        }

        ImGui::Dummy(ImVec2(0.0f, S(6.0f)));

        ImGui::TextUnformatted("旋转画面");
        const Rotation rotations[] = { Rotation::None, Rotation::Clockwise90, Rotation::Deg180, Rotation::Deg270 };
        const float gap = S(8.0f);
        const float chipWidth = (width - gap * 3.0f) / 4.0f;
        for (int i = 0; i < 4; ++i)
        {
            if (ChoiceChip(RotationLabel(rotations[i]), settings.rotation == rotations[i],
                           ImVec2(chipWidth, S(36.0f))))
            {
                settings.rotation = rotations[i];
            }
            if (i < 3)
                ImGui::SameLine(0.0f, gap);
        }

        ImGui::Dummy(ImVec2(0.0f, S(4.0f)));
        ImGui::TextUnformatted("画面比例");
        const AspectPreset aspects[] = { AspectPreset::Original, AspectPreset::Square,
                                         AspectPreset::Landscape, AspectPreset::Portrait };
        for (int i = 0; i < 4; ++i)
        {
            if (ChoiceChip(AspectLabel(aspects[i]), settings.aspect == aspects[i],
                           ImVec2(chipWidth, S(36.0f))))
            {
                settings.aspect = aspects[i];
            }
            if (i < 3)
                ImGui::SameLine(0.0f, gap);
        }

        ImGui::Dummy(ImVec2(0.0f, S(4.0f)));
        ImGui::TextUnformatted("水印");
        const WatermarkMode modes[] = { WatermarkMode::None, WatermarkMode::Text, WatermarkMode::Image };
        const float modeWidth = (width - gap * 2.0f) / 3.0f;
        for (int i = 0; i < 3; ++i)
        {
            if (ChoiceChip(WatermarkLabel(modes[i]), settings.watermark == modes[i],
                           ImVec2(modeWidth, S(36.0f))))
            {
                settings.watermark = modes[i];
            }
            if (i < 2)
                ImGui::SameLine(0.0f, gap);
        }

        if (settings.watermark == WatermarkMode::Text)
        {
            ImGui::Dummy(ImVec2(0.0f, S(4.0f)));
            ImGui::SetNextItemWidth(width);
            ImGui::InputTextWithHint("##watermark_text", "输入水印文字，例如：我的旅行日记",
                                     settings.watermarkText, sizeof(settings.watermarkText));
        }
        else if (settings.watermark == WatermarkMode::Image)
        {
            ImGui::Dummy(ImVec2(0.0f, S(4.0f)));
            if (SecondaryButton("选择图片…", ImVec2(S(140.0f), S(36.0f))))
                model.Ui().requestPickWatermarkImage = true;
            ImGui::SameLine(0.0f, S(10.0f));
            const std::string imageName = settings.watermarkImagePath[0] != '\0'
                                              ? FileNameOf(settings.watermarkImagePath)
                                              : std::string("尚未选择图片");
            ImGui::PushStyleColor(ImGuiCol_Text, c.textMuted);
            ImGui::TextUnformatted(imageName.c_str());
            ImGui::PopStyleColor();
        }

        if (settings.watermark != WatermarkMode::None)
        {
            int opacity = static_cast<int>(settings.watermarkOpacity * 100.0f + 0.5f);
            int scale = static_cast<int>(settings.watermarkScale * 100.0f + 0.5f);

            ImGui::SetNextItemWidth(S(200.0f));
            if (ImGui::SliderInt("透明度", &opacity, 10, 100, "%d%%"))
                settings.watermarkOpacity = static_cast<float>(opacity) / 100.0f;

            ImGui::SameLine(0.0f, S(20.0f));
            ImGui::SetNextItemWidth(S(200.0f));
            if (ImGui::SliderInt("大小", &scale, 5, 60, "%d%%"))
                settings.watermarkScale = static_cast<float>(scale) / 100.0f;

            ImGui::Dummy(ImVec2(0.0f, S(2.0f)));
        }

        ImGui::Dummy(ImVec2(0.0f, S(2.0f)));
        Hint("预览仅用于说明位置和大小，实际效果以输出文件为准。");
    }
}
