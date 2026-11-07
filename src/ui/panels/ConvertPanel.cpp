#include "ui/panels/ConvertPanel.h"

#include "model/ConvertMapping.h"
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
        void DrawTargetCards(ConvertSettings& settings, float width, float cardHeight)
        {
            const float gap = S(10.0f);
            const float cardWidth = std::max((width - gap * 2.0f) / 3.0f, S(120.0f));
            const ImVec2 cardSize(cardWidth, cardHeight);

            for (int i = 0; i < 3; ++i)
            {
                const ConvertTarget target = static_cast<ConvertTarget>(i);
                const OptionIcon icon = i == 0 ? OptionIcon::Shield
                                             : (i == 1 ? OptionIcon::Feather : OptionIcon::Music);
                char id[16] = {};
                std::snprintf(id, sizeof(id), "##target_%d", i);

                if (OptionCard(id, ConvertTargetLabel(target),
                               ConvertTargetSubtitle(target), ConvertTargetTag(target),
                               settings.target == target, cardSize, icon))
                {
                    settings.target = target;
                }

                if (i < 2)
                    ImGui::SameLine(0.0f, gap);
            }
        }

        void DrawSlimCard(AppModel& model, float width, bool compact)
        {
            ConvertSettings& settings = model.Convert();
            const UiColors& c = Colors();

            const float height = settings.slim ? S(186.0f)
                                               : (compact ? S(52.0f) : S(76.0f));
            CardBegin("##slim_card", ImVec2(0.0f, height), ImVec2(S(16.0f), S(13.0f)));

            ToggleSwitch("顺便压缩体积（可选）", &settings.slim);
            if (!compact)
                ImGui::Dummy(ImVec2(0.0f, S(2.0f)));

            if (settings.slim)
            {
                Hint(QualityHint(settings.slimQuality));
                ImGui::Dummy(ImVec2(0.0f, S(2.0f)));
                ImGui::TextUnformatted("想要多小？");
                ImGui::SameLine(0.0f, 10.0f);
                ImGui::PushStyleColor(ImGuiCol_Text, c.accent);
                ImGui::TextUnformatted(QualityLabel(settings.slimQuality));
                ImGui::PopStyleColor();
                QualitySlider("##convert_quality", &settings.slimQuality,
                              std::max(width - S(32.0f), S(120.0f)));
            }
            else if (!compact)
            {
                Hint("保持画质，只换编码；文件大小会和原文件接近。");
            }

            CardEnd();
        }

        void DrawContainerRow(ConvertSettings& settings, float width, float chipHeight)
        {
            const TargetFormat formats[] = { TargetFormat::MP4, TargetFormat::MKV };
            const float gap = S(10.0f);
            const float chipWidth = (width - gap) * 0.5f;

            for (int i = 0; i < 2; ++i)
            {
                if (ChoiceChip(ContainerLabel(formats[i]), settings.container == formats[i],
                               ImVec2(chipWidth, chipHeight)))
                {
                    settings.container = formats[i];
                }
                if (i == 0)
                    ImGui::SameLine(0.0f, gap);
            }
        }

        void DrawSourceSummary(const AppModel& model)
        {
            const UiColors& c = Colors();
            const auto& items = model.Queue().Items();
            if (items.empty())
            {
                Hint("添加文件后会显示它们的原编码，方便判断哪种结果最合适。");
                return;
            }

            int hevc = 0;
            int h264 = 0;
            int otherVideo = 0;
            int audioOnly = 0;
            int pending = 0;

            for (const MediaItem& item : items)
            {
                if (!item.probed)
                {
                    ++pending;
                    continue;
                }

                if (!item.hasVideo && item.hasAudio)
                {
                    ++audioOnly;
                    continue;
                }

                const std::string label = SourceCodecLabel(item.videoCodec);
                if (label == "H.265")
                    ++hevc;
                else if (label == "H.264")
                    ++h264;
                else
                    ++otherVideo;
            }

            std::string summary = "已识别 " + std::to_string(items.size()) + " 个文件：";
            bool first = true;
            auto append = [&](const std::string& text) {
                if (!first)
                    summary += " · ";
                summary += text;
                first = false;
            };
            if (hevc > 0) append("H.265 ×" + std::to_string(hevc));
            if (h264 > 0) append("H.264 ×" + std::to_string(h264));
            if (otherVideo > 0) append("其他编码 ×" + std::to_string(otherVideo));
            if (audioOnly > 0) append("纯音频 ×" + std::to_string(audioOnly));
            if (pending > 0) append("分析中 ×" + std::to_string(pending));

            Hint(summary.c_str());

            const ConvertTarget target = model.Convert().target;
            if (target != ConvertTarget::Audio && audioOnly > 0 && hevc + h264 + otherVideo == 0)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, c.warning);
                ImGui::TextWrapped("队列里只有声音文件，请选择“只要声音”，否则会转换失败。");
                ImGui::PopStyleColor();
            }
            else if (target == ConvertTarget::H264 && hevc + otherVideo == 0 && h264 > 0)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, c.textMuted);
                ImGui::TextWrapped("这些文件已经是 H.264；如果仍然打不开，可能只是封装不兼容，转成 MP4 试试。");
                ImGui::PopStyleColor();
            }
        }

    }

    void DrawConvertPanel(AppModel& model)
    {
        ConvertSettings& settings = model.Convert();
        const UiColors& c = Colors();
        const float width = ImGui::GetContentRegionAvail().x;
        const bool audioTarget = settings.target == ConvertTarget::Audio;

        // 最小窗口下压缩纵向空间，保证三步都看得到；正常窗口保持完整标题与说明。
        const bool compact = ImGui::GetContentRegionAvail().y < S(380.0f);
        const float sectionGap = S(6.0f);

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(S(10.0f), S(6.0f)));

        if (compact)
        {
            const float base = ImGui::GetFontSize();
            PushFontSize(base * 1.22f);
            ImGui::TextUnformatted("编码转换");
            PopFontSize();
        }
        else
        {
            SectionHeader("编码转换", "播放器打不开？换成 H.264 通常就能播；想更省空间就选 H.265。");
        }

        if (model.Task().IsRunning())
        {
            ImGui::PushStyleColor(ImGuiCol_Text, c.accent);
            ImGui::TextWrapped("正在处理：修改设置会在下一次开始时生效。");
            ImGui::PopStyleColor();
            ImGui::Dummy(ImVec2(0.0f, S(2.0f)));
        }

        StepHeader(1, "想要什么结果？", "三选一");
        DrawTargetCards(settings, width, compact ? S(96.0f) : S(104.0f));
        if (!compact)
            ImGui::Dummy(ImVec2(0.0f, sectionGap));

        StepHeader(2, "要不要顺便瘦身？", settings.slim ? "会明显变小" : "默认保持画质");
        DrawSlimCard(model, width, compact);
        if (!compact)
            ImGui::Dummy(ImVec2(0.0f, sectionGap));

        StepHeader(3, "输出成什么文件？", audioTarget ? "固定 MP3" : "MP4 最通用");
        if (audioTarget)
        {
            CardBegin("##audio_note", ImVec2(0.0f, S(62.0f)), ImVec2(S(16.0f), S(12.0f)));
            Hint("只导出声音，输出为 MP3；不会保留画面。");
            CardEnd();
        }
        else
        {
            DrawContainerRow(settings, width, compact ? S(34.0f) : S(40.0f));
            if (!compact)
            {
                Hint(settings.container == TargetFormat::MKV
                         ? "MKV 适合大文件和多个音轨，部分老设备不支持。"
                         : "MP4 兼容性最好，推荐用于手机、电视和社交软件。");
            }
        }

        ImGui::Dummy(ImVec2(0.0f, S(4.0f)));
        DrawSourceSummary(model);
        ImGui::Dummy(ImVec2(0.0f, S(6.0f)));

        if (!audioTarget)
        {
            CardBegin("##gpu_card", ImVec2(0.0f, S(94.0f)), ImVec2(S(16.0f), S(13.0f)));
            ToggleSwitch("使用显卡加速（更快）", &settings.useGpuAccel);
            ImGui::Dummy(ImVec2(0.0f, S(2.0f)));
            Hint(settings.useGpuAccel
                     ? "会自动选择可用的显卡；万一显卡不支持，会悄悄改用 CPU 继续处理。"
                     : "使用 CPU 处理，速度慢一些，但兼容性最好。");
            CardEnd();
            ImGui::Dummy(ImVec2(0.0f, S(4.0f)));
        }

        if (ImGui::CollapsingHeader("高级设置（一般不用改）"))
        {
            static const char* kResolutions[] = { "保持原分辨率", "1080p", "720p", "480p" };
            static const char* kFrameRates[] = { "保持原帧率", "30 帧", "24 帧" };

            ImGui::BeginDisabled(!settings.slim || audioTarget);
            ImGui::SetNextItemWidth(S(190.0f));
            ImGui::Combo("分辨率", &settings.resolutionIndex, kResolutions, 4);
            ImGui::SameLine(0.0f, S(18.0f));
            ImGui::SetNextItemWidth(S(190.0f));
            ImGui::Combo("帧率", &settings.frameRateIndex, kFrameRates, 3);
            ImGui::EndDisabled();
            Hint(settings.slim ? "这些选项会自动转换成专业参数，你不需要了解它们。"
                               : "只有打开“顺便压缩体积”后，分辨率和帧率才会改变。");
            ImGui::Dummy(ImVec2(0.0f, S(2.0f)));
        }

        ImGui::Dummy(ImVec2(0.0f, S(2.0f)));
        ImGui::PushStyleColor(ImGuiCol_Text, c.textMuted);
        const std::string suffix = model.Output().autoSuffix
                                       ? AutoSuffixFor(settings)
                                       : std::string(model.Output().suffix);
        ImGui::TextWrapped("输出文件名会自动加上“%s”，原文件不会被覆盖。", suffix.c_str());
        ImGui::PopStyleColor();

        ImGui::PopStyleVar();
    }
}
