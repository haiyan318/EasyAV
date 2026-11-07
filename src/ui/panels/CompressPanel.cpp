#include "ui/panels/CompressPanel.h"

#include "core/Format.h"
#include "ui/Labels.h"
#include "ui/Theme.h"
#include "ui/Widgets.h"

#include <algorithm>
#include <cfloat>
#include <cstdio>

namespace easyav::ui
{
    namespace
    {
        void DrawFormatChips(CompressSettings& settings, float width)
        {
            const TargetFormat formats[] = {
                TargetFormat::MP4, TargetFormat::MKV, TargetFormat::GIF, TargetFormat::MP3
            };
            const float gap = S(8.0f);
            const float chipWidth = (width - gap * 3.0f) / 4.0f;

            for (int i = 0; i < 4; ++i)
            {
                const TargetFormat format = formats[i];
                if (ChoiceChip(FormatLabel(format), settings.format == format,
                               ImVec2(chipWidth, S(40.0f))))
                {
                    settings.format = format;
                }
                if (i < 3)
                    ImGui::SameLine(0.0f, gap);
            }
        }

        void DrawEstimateCard(const AppModel& model)
        {
            const UiColors& c = Colors();
            const float base = ImGui::GetFontSize();
            const unsigned long long estimated = model.EstimateOutputBytes();
            const unsigned long long original = model.Queue().TotalBytes();

            CardBegin("##estimate", ImVec2(0.0f, S(86.0f)), ImVec2(S(18.0f), S(12.0f)));
            const ImVec2 origin = ImGui::GetCursorScreenPos();
            const float width = ImGui::GetContentRegionAvail().x;
            ImGui::Dummy(ImVec2(width, S(58.0f)));

            ImDrawList* drawList = ImGui::GetWindowDrawList();
            drawList->AddText(ImGui::GetFont(), base * 0.78f,
                              ImVec2(origin.x, origin.y),
                              ImGui::GetColorU32(c.textMuted), "预计输出体积");

            if (estimated == 0)
            {
                drawList->AddText(ImGui::GetFont(), base * 1.25f,
                                  ImVec2(origin.x, origin.y + S(22.0f)),
                                  ImGui::GetColorU32(c.textMuted), "添加文件后显示");
            }
            else
            {
                const std::string estimateText = FormatBytes(estimated);
                drawList->AddText(ImGui::GetFont(), base * 1.65f,
                                  ImVec2(origin.x, origin.y + S(18.0f)),
                                  ImGui::GetColorU32(c.accent), estimateText.c_str());

                const std::string originalText = "原始约 " + FormatBytes(original);
                const ImVec2 size = ImGui::GetFont()->CalcTextSizeA(base * 0.82f, FLT_MAX, 0.0f,
                                                                    originalText.c_str());
                drawList->AddText(ImGui::GetFont(), base * 0.82f,
                                  ImVec2(origin.x + width - size.x, origin.y + S(34.0f)),
                                  ImGui::GetColorU32(c.textMuted), originalText.c_str());

                const char* formatHint = "估算值，实际大小取决于视频内容";
                const ImVec2 hintSize = ImGui::GetFont()->CalcTextSizeA(base * 0.72f, FLT_MAX, 0.0f,
                                                                         formatHint);
                drawList->AddText(ImGui::GetFont(), base * 0.72f,
                                  ImVec2(origin.x + width - hintSize.x, origin.y + S(54.0f)),
                                  ImGui::GetColorU32(c.textMuted), formatHint);
            }

            CardEnd();
        }
    }

    void DrawCompressPanel(AppModel& model)
    {
        CompressSettings& settings = model.Compress();
        const UiColors& c = Colors();
        const float width = ImGui::GetContentRegionAvail().x;

        SectionHeader("视频瘦身", "不用懂参数，拖进来、选个大小、点开始。");

        ImGui::TextUnformatted("想要多小？");
        ImGui::SameLine(0.0f, 10.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, c.accent);
        ImGui::TextUnformatted(QualityLabel(settings.quality));
        ImGui::PopStyleColor();

        QualitySlider("quality", &settings.quality, width);
        Hint(QualityHint(settings.quality));
        ImGui::Dummy(ImVec2(0.0f, S(4.0f)));

        ImGui::TextUnformatted("输出成什么？");
        DrawFormatChips(settings, width);
        Hint("MP4 最通用；MKV 适合大文件；GIF 会变成无声动图；MP3 只保留声音。");
        ImGui::Dummy(ImVec2(0.0f, S(6.0f)));

        CardBegin("##gpu_card", ImVec2(0.0f, S(96.0f)), ImVec2(S(16.0f), S(14.0f)));
        ToggleSwitch("使用显卡加速（更快）", &settings.useGpuAccel);
        ImGui::Dummy(ImVec2(0.0f, S(2.0f)));
        Hint(settings.useGpuAccel
                 ? "会自动选择可用的显卡；万一显卡不支持，会悄悄改用 CPU 继续处理。"
                 : "使用 CPU 处理，速度慢一些，但兼容性最好。");
        CardEnd();

        ImGui::Dummy(ImVec2(0.0f, S(4.0f)));

        if (ImGui::CollapsingHeader("高级设置（一般不用改）"))
        {
            static const char* kResolutions[] = { "保持原分辨率", "1080p", "720p", "480p" };
            static const char* kFrameRates[] = { "保持原帧率", "30 帧", "24 帧" };

            ImGui::SetNextItemWidth(S(200.0f));
            ImGui::Combo("分辨率", &settings.resolutionIndex, kResolutions, 4);
            ImGui::SameLine(0.0f, S(18.0f));
            ImGui::SetNextItemWidth(S(200.0f));
            ImGui::Combo("帧率", &settings.frameRateIndex, kFrameRates, 3);
            Hint("这些选项会自动转换成专业参数，你不需要了解它们。");
            ImGui::Dummy(ImVec2(0.0f, S(2.0f)));
        }

        DrawEstimateCard(model);

        ImGui::Dummy(ImVec2(0.0f, S(2.0f)));
        ImGui::PushStyleColor(ImGuiCol_Text, c.textMuted);
        ImGui::TextWrapped("输出文件名会自动加上“%s”，原文件不会被覆盖。", model.Output().suffix);
        ImGui::PopStyleColor();
    }
}
