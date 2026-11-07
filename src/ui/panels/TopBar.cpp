#include "ui/panels/TopBar.h"

#include "ui/Theme.h"
#include "ui/Widgets.h"

#include <cfloat>

namespace easyav::ui
{
    void DrawTopBar(AppModel& model)
    {
        const UiColors& c = Colors();
        const float base = ImGui::GetFontSize();

        ImGui::PushStyleColor(ImGuiCol_ChildBg, c.panel);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(S(18.0f), 0.0f));
        ImGui::BeginChild("##topbar", ImVec2(0.0f, S(76.0f)), ImGuiChildFlags_None,
                          ImGuiWindowFlags_NoScrollbar);

        const ImVec2 origin = ImGui::GetCursorScreenPos();
        const float contentWidth = ImGui::GetContentRegionAvail().x;
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        // 品牌标识
        const ImVec2 logoMin(origin.x, origin.y + S(16.0f));
        const ImVec2 logoMax(logoMin.x + S(44.0f), logoMin.y + S(44.0f));
        for (int layer = 3; layer >= 1; --layer)
        {
            const float spread = S(1.4f) * static_cast<float>(layer);
            ImVec4 glow = c.accentGlow;
            glow.w *= 0.30f;
            drawList->AddRectFilled(ImVec2(logoMin.x - spread, logoMin.y - spread),
                                    ImVec2(logoMax.x + spread, logoMax.y + spread),
                                    ImGui::GetColorU32(glow), S(13.0f) + spread);
        }
        GradientRect(drawList, logoMin, logoMax, c.accentGradTop, c.accentGradBottom,
                     S(13.0f));

        const ImVec2 avSize = ImGui::GetFont()->CalcTextSizeA(base * 1.02f, FLT_MAX, 0.0f, "AV");
        drawList->AddText(ImGui::GetFont(), base * 1.02f,
                          ImVec2(logoMin.x + (S(44.0f) - avSize.x) * 0.5f,
                                 logoMin.y + (S(44.0f) - avSize.y) * 0.5f),
                          ImGui::GetColorU32(c.white), "AV");

        drawList->AddText(ImGui::GetFont(), base * 1.30f,
                          ImVec2(logoMax.x + S(12.0f), origin.y + S(14.0f)),
                          ImGui::GetColorU32(c.text), "EasyAV");
        drawList->AddText(ImGui::GetFont(), base * 0.78f,
                          ImVec2(logoMax.x + S(13.0f), origin.y + S(42.0f)),
                          ImGui::GetColorU32(c.textMuted),
                          "编码转换 · 压缩 · 提取音频（全部在本机完成）");

        // 右上角操作
        const float buttonHeight = S(38.0f);
        const float gap = S(8.0f);
        const float fileWidth = S(108.0f);
        const float folderWidth = S(112.0f);
        const float settingsWidth = S(76.0f);
        const float pillWidth = S(132.0f);
        const float pillGap = S(10.0f);
        const float totalWidth = fileWidth + folderWidth + settingsWidth + gap * 2.0f
                                 + pillWidth + pillGap;
        float x = origin.x + contentWidth - totalWidth;
        const float y = origin.y + (S(76.0f) - buttonHeight) * 0.5f;

        // 引擎状态胶囊
        const ToolStatus& tools = model.Task().Tools();
        const bool engineReady = tools.ffmpegFound && tools.ffprobeFound;
        const ImVec4 engineColor = engineReady ? c.success : c.warning;
        const ImVec2 pillMin(x, origin.y + (S(76.0f) - S(32.0f)) * 0.5f);
        const ImVec2 pillMax(pillMin.x + pillWidth, pillMin.y + S(32.0f));
        drawList->AddRectFilled(pillMin, pillMax,
                                ImGui::GetColorU32(engineReady ? c.successSoft : c.panelAlt),
                                S(16.0f));
        drawList->AddCircleFilled(ImVec2(pillMin.x + S(14.0f), pillMin.y + S(16.0f)),
                                  S(4.0f), ImGui::GetColorU32(engineColor));
        drawList->AddText(ImGui::GetFont(), base * 0.76f,
                          ImVec2(pillMin.x + S(24.0f), pillMin.y + S(8.0f)),
                          ImGui::GetColorU32(engineColor),
                          engineReady ? "处理组件已就绪" : "处理组件未就绪");

        ImGui::SetCursorScreenPos(pillMin);
        ImGui::InvisibleButton("##engine_status", ImVec2(pillWidth, S(32.0f)));
        if (ImGui::IsItemHovered())
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            if (!engineReady && !tools.problem.empty())
                ImGui::SetTooltip("%s", tools.problem.c_str());
        }
        if (ImGui::IsItemClicked())
            model.Ui().showSettings = true;

        x += pillWidth + pillGap;

        ImGui::BeginDisabled(model.Task().IsRunning());

        ImGui::SetCursorScreenPos(ImVec2(x, y));
        if (SecondaryButton("+ 添加文件", ImVec2(fileWidth, buttonHeight)))
            model.Ui().requestAddFiles = true;

        x += fileWidth + gap;
        ImGui::SetCursorScreenPos(ImVec2(x, y));
        if (SecondaryButton("添加文件夹", ImVec2(folderWidth, buttonHeight)))
            model.Ui().requestAddFolder = true;

        ImGui::EndDisabled();

        x += folderWidth + gap;
        ImGui::SetCursorScreenPos(ImVec2(x, y));
        if (SecondaryButton("设置", ImVec2(settingsWidth, buttonHeight)))
            model.Ui().showSettings = true;

        // 底部分隔线
        drawList->AddLine(ImVec2(origin.x - S(18.0f), origin.y + S(75.0f)),
                          ImVec2(origin.x + contentWidth + S(18.0f), origin.y + S(75.0f)),
                          ImGui::GetColorU32(c.border), 1.0f);

        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
    }
}
