#include "ui/panels/RecordPanel.h"

#include "core/Format.h"
#include "ui/Labels.h"
#include "ui/Theme.h"
#include "ui/Widgets.h"

#include <algorithm>
#include <cstdio>

namespace easyav::ui
{
    namespace
    {
        void DrawRecordCard(AppModel& model, RecordTarget target, const char* title,
                            const char* subtitle, const ImVec2& size)
        {
            const UiColors& c = Colors();
            const float base = ImGui::GetFontSize();
            RecordSettings& settings = model.Record();
            const bool isThis = settings.target == target;
            const bool recording = settings.recording && isThis;
            const bool otherRecording = settings.recording && !isThis;

            CardBegin(target == RecordTarget::Screen ? "##card_screen" : "##card_camera",
                      size, ImVec2(S(18.0f), S(14.0f)));

            const ImVec2 origin = ImGui::GetCursorScreenPos();
            const float width = ImGui::GetContentRegionAvail().x;
            ImDrawList* drawList = ImGui::GetWindowDrawList();

            drawList->AddText(ImGui::GetFont(), base * 1.12f,
                              ImVec2(origin.x, origin.y),
                              ImGui::GetColorU32(otherRecording ? c.textMuted : c.text), title);

            drawList->AddText(ImGui::GetFont(), base * 0.76f,
                              ImVec2(origin.x, origin.y + S(26.0f)),
                              ImGui::GetColorU32(c.textMuted), subtitle);

            const ImVec2 center(origin.x + width * 0.5f, origin.y + S(108.0f));
            const float radius = S(34.0f);
            ImGui::SetCursorScreenPos(ImVec2(center.x - radius, center.y - radius));
            ImGui::InvisibleButton("##record", ImVec2(radius * 2.0f, radius * 2.0f));
            const bool hovered = ImGui::IsItemHovered() && !otherRecording;
            if (hovered)
                ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

            const ImVec4 accent = recording ? c.danger : (otherRecording ? c.track : c.accent);
            const ImVec4 soft = recording ? ImVec4(c.danger.x, c.danger.y, c.danger.z, 0.14f)
                                          : (otherRecording ? c.panelAlt : c.accentSoft);
            drawList->AddCircleFilled(center, radius + (hovered ? 2.0f : 0.0f),
                                      ImGui::GetColorU32(soft));
            drawList->AddCircle(center, radius, ImGui::GetColorU32(accent), 0, 2.0f);

            if (recording)
                drawList->AddRectFilled(ImVec2(center.x - S(9.0f), center.y - S(9.0f)),
                                        ImVec2(center.x + S(9.0f), center.y + S(9.0f)),
                                        ImGui::GetColorU32(c.danger), 3.0f);
            else
                drawList->AddCircleFilled(center, S(11.0f), ImGui::GetColorU32(accent));

            const char* action = recording ? "停止并保存" : "开始录制";
            const ImVec2 actionSize = ImGui::GetFont()->CalcTextSizeA(base * 0.86f, FLT_MAX, 0.0f, action);
            drawList->AddText(ImGui::GetFont(), base * 0.86f,
                              ImVec2(center.x - actionSize.x * 0.5f, center.y + S(48.0f)),
                              ImGui::GetColorU32(otherRecording ? c.textMuted : c.text), action);

            char timer[32] = {};
            std::snprintf(timer, sizeof(timer), "%s", FormatClock(recording || settings.elapsedSec > 0.0f
                                                                       ? settings.elapsedSec
                                                                       : 0.0).c_str());
            const ImVec2 timerSize = ImGui::GetFont()->CalcTextSizeA(base * 0.78f, FLT_MAX, 0.0f, timer);
            drawList->AddText(ImGui::GetFont(), base * 0.78f,
                              ImVec2(center.x - timerSize.x * 0.5f, center.y + S(68.0f)),
                              ImGui::GetColorU32(recording ? c.danger : c.textMuted), timer);

            if (ImGui::IsItemClicked() && !otherRecording)
            {
                if (recording)
                {
                    settings.recording = false;
                    model.ShowToast("录制已停止，文件已保存（界面演示）", ToastLevel::Success);
                }
                else
                {
                    settings.target = target;
                    settings.recording = true;
                    settings.elapsedSec = 0.0f;
                    model.ShowToast(target == RecordTarget::Screen ? "开始录制屏幕（界面演示）"
                                                                   : "开始录制摄像头（界面演示）",
                                    ToastLevel::Info);
                }
            }

            CardEnd();
        }
    }

    void DrawRecordPanel(AppModel& model)
    {
        RecordSettings& settings = model.Record();
        const UiColors& c = Colors();
        const float width = ImGui::GetContentRegionAvail().x;

        SectionHeader("录屏与截图", "点一下就开始，不用记参数。");

        const float gap = S(12.0f);
        const float cardWidth = (width - gap) * 0.5f;
        const float cardHeight = S(236.0f);
        DrawRecordCard(model, RecordTarget::Screen, "录制屏幕", "把整个桌面或某个窗口录下来", ImVec2(cardWidth, cardHeight));
        ImGui::SameLine(0.0f, gap);
        DrawRecordCard(model, RecordTarget::Camera, "录制摄像头", "用电脑摄像头录一段视频", ImVec2(cardWidth, cardHeight));

        ImGui::Dummy(ImVec2(0.0f, S(4.0f)));
        ImGui::TextUnformatted("录制画质");
        const char* qualities[] = { "标清", "高清", "原画" };
        const float chipWidth = (width - gap * 2.0f) / 3.0f;
        for (int i = 0; i < 3; ++i)
        {
            if (ChoiceChip(qualities[i], settings.qualityIndex == i, ImVec2(chipWidth, S(34.0f))))
                settings.qualityIndex = i;
            if (i < 2)
                ImGui::SameLine(0.0f, gap);
        }

        ImGui::Dummy(ImVec2(0.0f, S(6.0f)));

        CardBegin("##screenshot_card", ImVec2(0.0f, S(158.0f)), ImVec2(S(18.0f), S(14.0f)));
        ToggleSwitch("定时自动截图", &settings.captureFrames);
        ImGui::Dummy(ImVec2(0.0f, S(2.0f)));
        if (settings.captureFrames)
        {
            ImGui::SetNextItemWidth(S(260.0f));
            ImGui::SliderInt("每隔几秒截一张", &settings.captureIntervalSec, 1, 60, "%d 秒");
            ImGui::SameLine(0.0f, S(16.0f));
            if (SecondaryButton("立即截一张", ImVec2(S(140.0f), S(36.0f))))
                model.ShowToast("已截取一张图片（界面演示）", ToastLevel::Success);
            Hint("截图会按顺序保存到输出文件夹，不会覆盖已有文件。");
        }
        else
        {
            if (SecondaryButton("立即截一张", ImVec2(S(140.0f), S(36.0f))))
                model.ShowToast("已截取一张图片（界面演示）", ToastLevel::Success);
            ImGui::SameLine(0.0f, S(12.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, c.textMuted);
            ImGui::TextUnformatted("也可以只截当前画面，不开启连续截图。");
            ImGui::PopStyleColor();
        }
        CardEnd();

        ImGui::Dummy(ImVec2(0.0f, S(4.0f)));
        Hint("录制和截图目前为界面演示，尚未接入屏幕采集与编码功能。");
    }
}
