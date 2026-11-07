#include "ui/panels/ClipPanel.h"

#include "core/Format.h"
#include "ui/Theme.h"
#include "ui/Widgets.h"

#include <algorithm>
#include <cstdio>
#include <string>

namespace easyav::ui
{
    void DrawClipPanel(AppModel& model)
    {
        ClipSettings& settings = model.Clip();
        const UiColors& c = Colors();
        const float width = ImGui::GetContentRegionAvail().x;

        SectionHeader("剪辑与拼接", "拖动两个把手，框住想要的那一段。");

        double duration = 0.0;
        if (!model.Queue().Empty())
            duration = model.Queue().Items().front().durationSec;

        const std::string startText = duration > 0.0
                                          ? FormatClock(duration * settings.startPct)
                                          : FormatPercent(settings.startPct);
        const std::string endText = duration > 0.0
                                        ? FormatClock(duration * settings.endPct)
                                        : FormatPercent(settings.endPct);

        TimelineRangeSlider("clip_timeline", &settings.startPct, &settings.endPct, width,
                            startText.c_str(), endText.c_str());

        ImGui::Dummy(ImVec2(0.0f, S(2.0f)));
        char readout[192] = {};
        if (duration > 0.0)
        {
            const double kept = std::max(0.0, duration * (settings.endPct - settings.startPct));
            std::snprintf(readout, sizeof(readout), "保留这一段：%s 到 %s，共 %s",
                          startText.c_str(), endText.c_str(), FormatClock(kept).c_str());
        }
        else
        {
            std::snprintf(readout, sizeof(readout), "保留这一段：从 %s 到 %s（添加文件后显示具体时间）",
                          startText.c_str(), endText.c_str());
        }
        ImGui::PushStyleColor(ImGuiCol_Text, c.textMuted);
        ImGui::TextWrapped("%s", readout);
        ImGui::PopStyleColor();

        ImGui::Dummy(ImVec2(0.0f, S(8.0f)));

        const float gap = S(8.0f);
        const float quickWidth = (width - gap * 2.0f) / 3.0f;
        if (SecondaryButton("整段保留", ImVec2(quickWidth, S(34.0f))))
        {
            settings.startPct = 0.0f;
            settings.endPct = 1.0f;
        }
        ImGui::SameLine(0.0f, gap);
        if (SecondaryButton("去掉开头 10%", ImVec2(quickWidth, S(34.0f))))
        {
            settings.startPct = 0.10f;
            settings.endPct = std::max(settings.endPct, 0.12f);
        }
        ImGui::SameLine(0.0f, gap);
        if (SecondaryButton("去掉结尾 10%", ImVec2(quickWidth, S(34.0f))))
        {
            settings.endPct = 0.90f;
            settings.startPct = std::min(settings.startPct, 0.89f);
        }

        ImGui::Dummy(ImVec2(0.0f, S(10.0f)));

        CardBegin("##clip_options", ImVec2(0.0f, S(150.0f)), ImVec2(S(16.0f), S(14.0f)));
        ToggleSwitch("把队列里的文件按顺序拼成一个", &settings.mergeFiles);
        if (settings.mergeFiles)
        {
            if (model.Queue().Count() < 2)
                Hint("队列里至少要有 2 个文件才能拼接；现在只会处理单个文件。");
            else
                Hint("拼接要求文件格式一致，开始前会自动检查。");
        }
        else
        {
            Hint("每个文件单独剪辑，不会合并。");
        }

        ImGui::Dummy(ImVec2(0.0f, S(6.0f)));
        if (ToggleSwitch("只要声音（提取成音频）", &settings.extractAudio) && settings.extractAudio)
            settings.stripAudio = false;

        if (settings.extractAudio)
        {
            Hint("开始后只导出声音，画面会被丢弃，适合提取背景音乐。");
        }
        else
        {
            if (ToggleSwitch("去掉声音（画面保留）", &settings.stripAudio) && settings.stripAudio)
                settings.extractAudio = false;
            Hint(settings.stripAudio ? "导出的视频会静音。" : "画面和声音都保留。");
        }
        CardEnd();

        ImGui::Dummy(ImVec2(0.0f, S(6.0f)));
        Hint("剪辑采用智能重编码，保证画面和声音对齐；分辨率、帧率等参数由软件自动决定。");
    }
}
