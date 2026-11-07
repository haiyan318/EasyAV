#include "ui/panels/StatusBar.h"

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

        drawList->AddCircleFilled(ImVec2(origin.x + S(18.0f), origin.y + height * 0.5f), S(3.5f),
                                  ImGui::GetColorU32(dotColor));
        drawList->AddText(ImGui::GetFont(), base * 0.76f,
                          ImVec2(origin.x + S(28.0f), origin.y + S(8.0f)),
                          ImGui::GetColorU32(c.textMuted), text);

        const std::string right = model.QueueSummary() + "   ·   v0.1 界面预览";
        const ImVec2 rightSize = ImGui::GetFont()->CalcTextSizeA(base * 0.76f, FLT_MAX, 0.0f, right.c_str());
        drawList->AddText(ImGui::GetFont(), base * 0.76f,
                          ImVec2(origin.x + width - rightSize.x - S(18.0f), origin.y + S(8.0f)),
                          ImGui::GetColorU32(c.textMuted), right.c_str());
    }
}
