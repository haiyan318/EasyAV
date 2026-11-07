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
        const char* StartLabelFor(const AppModel& model, const TaskController& task)
        {
            if (task.HasFinished() || task.Phase() == TaskPhase::Canceled)
                return "再转换一次";

            return model.Convert().target == ConvertTarget::Audio ? "提取音频" : "开始转换";
        }

        void StartCurrentTask(AppModel& model)
        {
            if (model.Ui().scene != SceneId::Convert)
            {
                model.ShowToast("这个功能还在开发中，请先切换到「编码转换」",
                                ToastLevel::Info);
                return;
            }

            if (model.Task().Start(model.MakeTaskRequest()))
            {
                model.ShowToast(model.Convert().target == ConvertTarget::Audio
                                    ? "开始提取音频"
                                    : "开始转换",
                                ToastLevel::Info);
            }
            else
            {
                const std::string& error = model.Task().StartErrorText();
                if (!error.empty())
                    model.ShowToast(error, ToastLevel::Warning);
            }
        }
    }

    void DrawActionBar(AppModel& model)
    {
        const UiColors& c = Colors();
        const float base = ImGui::GetFontSize();
        TaskController& task = model.Task();
        const ToolStatus& tools = task.Tools();

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
        const bool canStart = model.Ui().scene == SceneId::Convert
                              && !model.Queue().Empty() && !task.IsRunning()
                              && tools.ffmpegFound;
        if (AccentButton(StartLabelFor(model, task),
                         ImVec2(buttonWidth, buttonHeight), canStart))
        {
            StartCurrentTask(model);
        }

        float fraction = 0.0f;
        if (task.IsRunning())
            fraction = task.Progress();
        else if (task.HasFinished())
            fraction = 1.0f;
        else if (task.Phase() == TaskPhase::Canceled)
            fraction = task.Progress();

        ImGui::SetCursorScreenPos(ImVec2(origin.x, origin.y + S(48.0f)));
        const char* overlay = nullptr;
        std::string overlayText;
        if (task.IsRunning() && task.Indeterminate())
            overlayText = "处理中…";
        else if (fraction > 0.0f || task.HasFinished())
            overlayText = FormatPercent(fraction);

        if (!overlayText.empty())
            overlay = overlayText.c_str();

        ProgressBar("##global_progress", fraction, ImVec2(width, S(12.0f)), overlay,
                    task.IsRunning());

        char detail[256] = {};
        if (task.IsRunning())
        {
            const int current = std::min(task.CurrentFileIndex() + 1, task.FileCount());
            const double eta = task.EtaSeconds();
            const char* failedSuffix =
                task.FailedCount() > 0 ? "，有文件失败" : "";

            if (task.Indeterminate())
            {
                std::snprintf(detail, sizeof(detail),
                              "%d/%d · 已用 %s · 剩余时间无法预估%s",
                              current, task.FileCount(),
                              FormatClock(task.ElapsedSeconds()).c_str(), failedSuffix);
            }
            else if (eta >= 0.0)
            {
                std::snprintf(detail, sizeof(detail),
                              "%s · %d/%d · 已用 %s · 预计剩余 %s%s",
                              FormatPercent(task.Progress()).c_str(), current,
                              task.FileCount(),
                              FormatClock(task.ElapsedSeconds()).c_str(),
                              FormatClock(eta).c_str(), failedSuffix);
            }
            else
            {
                std::snprintf(detail, sizeof(detail), "%s · %d/%d · 已用 %s%s",
                              FormatPercent(task.Progress()).c_str(), current,
                              task.FileCount(),
                              FormatClock(task.ElapsedSeconds()).c_str(), failedSuffix);
            }
        }
        else if (task.HasFinished())
        {
            if (task.FailedCount() > 0)
            {
                std::snprintf(detail, sizeof(detail),
                              "共 %d 个文件，完成 %d 个，%d 个失败。",
                              task.FileCount(), task.SucceededCount(), task.FailedCount());
            }
            else
            {
                std::snprintf(detail, sizeof(detail),
                              "共 %d 个文件，全部处理完成，已保存到输出位置。",
                              task.FileCount());
            }
        }
        else if (task.Phase() == TaskPhase::Canceled)
        {
            std::snprintf(detail, sizeof(detail), "已取消，进度停在 %s，可随时重新开始。",
                          FormatPercent(task.Progress()).c_str());
        }
        else if (!tools.ffmpegFound)
        {
            std::snprintf(detail, sizeof(detail), "%s",
                          tools.problem.c_str());
        }
        else if (model.Queue().Empty())
        {
            std::snprintf(detail, sizeof(detail), "先在左侧添加文件，再点“开始转换”。");
        }
        else if (model.Ui().scene != SceneId::Convert)
        {
            std::snprintf(detail, sizeof(detail),
                          "当前场景还在开发中，可以先切换到「编码转换」处理文件。");
        }
        else
        {
            const ConvertSettings& convert = model.Convert();
            const char* targetText = convert.target == ConvertTarget::Audio
                                         ? "MP3（只要声音）"
                                         : (convert.target == ConvertTarget::H265
                                                ? "H.265"
                                                : "H.264");
            std::snprintf(detail, sizeof(detail), "共 %d 个文件，准备转换为 %s。",
                          model.Queue().Count(), targetText);
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
                model.ShowToast("正在取消…", ToastLevel::Info);
            }
        }

        CardEnd();
    }
}
