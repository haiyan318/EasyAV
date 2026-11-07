#include "ui/panels/ActionBar.h"

#include "core/Format.h"
#include "ui/Theme.h"
#include "ui/Widgets.h"

#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>

namespace easyav::ui
{
    namespace
    {
        void StartCurrentTask(AppModel& model)
        {
            std::vector<std::string> names;
            names.reserve(model.Queue().Items().size());
            for (const MediaItem& item : model.Queue().Items())
                names.push_back(item.displayName);

            if (model.Task().Start(names))
                model.ShowToast("开始处理（界面演示，尚未接入编码器）", ToastLevel::Info);
        }
    }

    void DrawActionBar(AppModel& model)
    {
        const UiColors& c = Colors();
        const float base = ImGui::GetFontSize();
        TaskController& task = model.Task();

        CardBegin("##action_bar", ImVec2(0.0f, S(120.0f)), ImVec2(S(18.0f), S(12.0f)));

        const ImVec2 origin = ImGui::GetCursorScreenPos();
        const float width = ImGui::GetContentRegionAvail().x;
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        ImVec4 phaseColor = c.textMuted;
        if (task.IsRunning())
            phaseColor = c.accent;
        else if (task.HasFinished())
            phaseColor = c.success;
        else if (task.Phase() == TaskPhase::Canceled)
            phaseColor = c.warning;

        drawList->AddCircleFilled(ImVec2(origin.x + S(5.0f), origin.y + S(17.0f)), S(4.5f),
                                  ImGui::GetColorU32(phaseColor));

        const std::string status = task.StatusText();
        const float statusWidth = std::max(width - S(200.0f), S(120.0f));
        const std::string statusShown = Ellipsize(status, statusWidth);
        drawList->AddText(ImGui::GetFont(), base * 1.04f,
                          ImVec2(origin.x + S(18.0f), origin.y + S(6.0f)),
                          ImGui::GetColorU32(c.text), statusShown.c_str());

        const float buttonWidth = S(164.0f);
        const float buttonHeight = S(40.0f);
        ImGui::SetCursorScreenPos(ImVec2(origin.x + width - buttonWidth, origin.y - S(2.0f)));
        const bool canStart = !model.Queue().Empty() && !task.IsRunning();
        ImGui::BeginDisabled(!canStart);
        const char* startLabel = (task.HasFinished() || task.Phase() == TaskPhase::Canceled)
                                     ? "重新开始"
                                     : "开始处理";
        if (PrimaryButton(startLabel, ImVec2(buttonWidth, buttonHeight)))
            StartCurrentTask(model);
        ImGui::EndDisabled();

        float fraction = 0.0f;
        if (task.IsRunning())
            fraction = task.Progress();
        else if (task.HasFinished())
            fraction = 1.0f;
        else if (task.Phase() == TaskPhase::Canceled)
            fraction = task.Progress();

        ImGui::SetCursorScreenPos(ImVec2(origin.x, origin.y + S(48.0f)));
        ProgressBar("##global_progress", fraction, ImVec2(width, S(12.0f)), nullptr);

        char detail[256] = {};
        if (task.IsRunning())
        {
            const int current = std::min(task.CurrentFileIndex() + 1, task.FileCount());
            const double eta = task.EtaSeconds();
            if (eta >= 0.0)
            {
                std::snprintf(detail, sizeof(detail), "%s · %d/%d · 已用 %s · 预计剩余 %s",
                              FormatPercent(task.Progress()).c_str(), current, task.FileCount(),
                              FormatClock(task.ElapsedSeconds()).c_str(), FormatClock(eta).c_str());
            }
            else
            {
                std::snprintf(detail, sizeof(detail), "%s · %d/%d · 已用 %s",
                              FormatPercent(task.Progress()).c_str(), current, task.FileCount(),
                              FormatClock(task.ElapsedSeconds()).c_str());
            }
        }
        else if (task.HasFinished())
        {
            std::snprintf(detail, sizeof(detail), "共 %d 个文件，全部处理完成，已保存到输出位置。", task.FileCount());
        }
        else if (task.Phase() == TaskPhase::Canceled)
        {
            std::snprintf(detail, sizeof(detail), "已取消，进度停在 %s，可随时重新开始。",
                          FormatPercent(task.Progress()).c_str());
        }
        else if (model.Queue().Empty())
        {
            std::snprintf(detail, sizeof(detail), "先在左侧添加文件，再点“开始处理”。");
        }
        else
        {
            std::snprintf(detail, sizeof(detail), "共 %d 个文件，准备就绪。", model.Queue().Count());
        }

        const float detailWidth = task.IsRunning() ? width - S(110.0f) : width;
        drawList->AddText(ImGui::GetFont(), base * 0.78f,
                          ImVec2(origin.x, origin.y + S(70.0f)),
                          ImGui::GetColorU32(c.textMuted), Ellipsize(detail, detailWidth).c_str());

        if (task.IsRunning())
        {
            const float cancelWidth = S(92.0f);
            ImGui::SetCursorScreenPos(ImVec2(origin.x + width - cancelWidth, origin.y + S(64.0f)));
            if (SecondaryButton("取消", ImVec2(cancelWidth, S(30.0f))))
            {
                task.Cancel();
                model.ShowToast("已取消处理", ToastLevel::Warning);
            }
        }

        CardEnd();
    }
}
