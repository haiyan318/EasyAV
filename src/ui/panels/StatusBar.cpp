#include "ui/panels/StatusBar.h"

#include "core/Format.h"
#include "ui/Labels.h"
#include "ui/Theme.h"

#include <algorithm>
#include <string>

namespace easyav::ui
{
    void DrawStatusBar(const AppModel& model)
    {
        const UiColors& c = Colors();
        const float base = ImGui::GetFontSize();
        const ImVec2 origin = ImGui::GetCursorScreenPos();
        const float width = ImGui::GetContentRegionAvail().x;
        const float height = S(30.0f);

        ImGui::Dummy(ImVec2(width, height));

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddLine(origin, ImVec2(origin.x + width, origin.y),
                          ImGui::GetColorU32(c.border), 1.0f);

        ImVec4 dotColor = c.success;
        const char* text = "就绪";
        if (model.Task().IsRunning())
        {
            dotColor = c.accent;
            text = "正在处理";
        }
        else if (model.Task().HasFinished())
        {
            dotColor = c.success;
            text = "处理完成";
        }
        else if (model.Task().Phase() == TaskPhase::Canceled)
        {
            dotColor = c.warning;
            text = "已取消";
        }
        else if (!model.Task().Tools().ffmpegFound || !model.Task().Tools().ffprobeFound)
        {
            dotColor = c.warning;
            text = ToolStateLabel(model.Task().Tools());
        }

        drawList->AddCircleFilled(ImVec2(origin.x + S(18.0f), origin.y + height * 0.5f), S(3.5f),
                                  ImGui::GetColorU32(dotColor));
        drawList->AddText(ImGui::GetFont(), base * 0.76f,
                          ImVec2(origin.x + S(28.0f), origin.y + S(8.0f)),
                          ImGui::GetColorU32(c.textMuted), text);
        const float leftEnd = origin.x + S(28.0f)
                              + ImGui::GetFont()
                                    ->CalcTextSizeA(base * 0.76f, FLT_MAX, 0.0f, text)
                                    .x;

        const std::string right = model.QueueSummary() + "   ·   v0.2";
        const ImVec2 rightSize = ImGui::GetFont()->CalcTextSizeA(base * 0.76f, FLT_MAX, 0.0f,
                                                                 right.c_str());
        const float rightStart = origin.x + width - rightSize.x - S(18.0f);
        drawList->AddText(ImGui::GetFont(), base * 0.76f,
                          ImVec2(rightStart, origin.y + S(8.0f)),
                          ImGui::GetColorU32(c.textMuted), right.c_str());

        // 中间区：当前输出目标与估算体积，形成“设置 → 预期结果”的直觉。
        std::string middle;
        if (model.Queue().Empty())
        {
            middle = "等待添加文件";
        }
        else
        {
            const ConvertSettings& convert = model.Convert();
            const char* target = convert.target == ConvertTarget::Audio
                                     ? "MP3"
                                     : (convert.target == ConvertTarget::H265 ? "H.265" : "H.264");
            middle = std::string("输出：") + target;
            if (convert.target != ConvertTarget::Audio)
                middle += convert.container == TargetFormat::MKV ? " · MKV" : " · MP4";
            middle += " · 预计约 " + FormatBytes(model.EstimateOutputBytes());
        }

        const float middleSize = ImGui::GetFont()
                                     ->CalcTextSizeA(base * 0.76f, FLT_MAX, 0.0f,
                                                     middle.c_str())
                                     .x;
        const float middleStart = leftEnd + S(28.0f);
        if (middleStart + middleSize + S(24.0f) < rightStart)
        {
            drawList->AddCircleFilled(
                ImVec2(middleStart - S(9.0f), origin.y + height * 0.5f), S(2.0f),
                ImGui::GetColorU32(c.border));
            drawList->AddText(ImGui::GetFont(), base * 0.76f,
                              ImVec2(middleStart, origin.y + S(8.0f)),
                              ImGui::GetColorU32(c.textMuted), middle.c_str());
        }
    }
}
