#include "ui/panels/SettingsDialog.h"

#include "ui/Labels.h"
#include "ui/Theme.h"
#include "ui/Widgets.h"

#include <cfloat>
#include <cstdio>

namespace easyav::ui
{
    namespace
    {
        void DrawOutputSection(AppModel& model)
        {
            OutputSettings& output = model.Output();
            const UiColors& c = Colors();

            ImGui::TextUnformatted("输出位置");
            ImGui::Checkbox("保存在原文件旁边", &output.sameAsSource);

            if (output.sameAsSource)
            {
                Hint("每个文件都会输出到它所在的文件夹，自动加上后缀，不会覆盖原文件。");
            }
            else
            {
                ImGui::SetNextItemWidth(S(390.0f));
                ImGui::InputTextWithHint("##output_dir", "选择一个文件夹",
                                         output.outputDir, sizeof(output.outputDir));
                ImGui::SameLine(0.0f, S(8.0f));
                if (SecondaryButton("浏览…", ImVec2(S(90.0f), S(36.0f))))
                    model.Ui().requestPickOutputDir = true;
            }

            ImGui::Dummy(ImVec2(0.0f, S(4.0f)));
            ImGui::Checkbox("按结果自动命名（推荐）", &output.autoSuffix);
            if (output.autoSuffix)
            {
                Hint("会自动使用 _H264 / _H265 / _音频 后缀，避免不同结果互相覆盖。");
            }
            else
            {
                ImGui::SetNextItemWidth(S(160.0f));
                ImGui::InputText("文件名后缀", output.suffix, sizeof(output.suffix));
                ImGui::SameLine(0.0f, S(12.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, c.textMuted);
                ImGui::TextUnformatted("例如：旅行_转换.mp4（留空会自动加序号）");
                ImGui::PopStyleColor();
            }

            ImGui::Dummy(ImVec2(0.0f, S(2.0f)));
            ImGui::Checkbox("处理完成后打开输出文件夹", &output.openFolderWhenDone);
        }
    }

    void DrawSettingsDialog(AppModel& model)
    {
        static bool popupOpened = false;

        if (!model.Ui().showSettings)
        {
            popupOpened = false;
            return;
        }

        if (!popupOpened)
        {
            ImGui::OpenPopup("设置");
            popupOpened = true;
        }

        ImGui::SetNextWindowSizeConstraints(ImVec2(S(560.0f), 0.0f), ImVec2(S(560.0f), FLT_MAX));
        if (!ImGui::BeginPopupModal("设置", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            return;

        const UiColors& c = Colors();
        SectionHeader("设置", "只影响这台电脑，随时可以再改。");

        ImGui::Separator();
        DrawOutputSection(model);

        ImGui::Separator();
        ImGui::TextUnformatted("界面");
        bool dark = model.Ui().darkTheme;
        if (ImGui::Checkbox("深色界面", &dark))
        {
            model.Ui().darkTheme = dark;
            ApplyTheme(dark);
        }
        ImGui::SameLine(0.0f, S(12.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, c.textMuted);
        ImGui::TextUnformatted("夜间使用更护眼。");
        ImGui::PopStyleColor();

        ImGui::Separator();
        ImGui::TextUnformatted("关于 EasyAV");
        ImGui::PushStyleColor(ImGuiCol_Text, c.textMuted);
        ImGui::TextWrapped("版本 v0.2。可以真正处理文件：把 HEVC / AV1 / FLV 等常见编码转成 "
                           "H.264（最兼容）或 H.265（更小），输出 MP4 / MKV，也能提取 MP3；"
                           "需要时再打开“顺便压缩体积”。输出不会覆盖原文件。");
        ImGui::TextWrapped("剪辑、滤镜、录屏仍在开发中。");
        ImGui::TextWrapped("界面设计目标：让完全不懂视频的人，也能在 3 步内完成转换和提取音乐。");
        ImGui::PopStyleColor();

        const ToolStatus& tools = model.Task().Tools();
        ImGui::PushStyleColor(ImGuiCol_Text,
                              (tools.ffmpegFound && tools.ffprobeFound) ? c.success
                                                                        : c.warning);
        ImGui::TextUnformatted(ToolStateLabel(tools));
        ImGui::PopStyleColor();

        ImGui::Dummy(ImVec2(0.0f, S(8.0f)));
        const float closeWidth = S(120.0f);
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - closeWidth - ImGui::GetStyle().WindowPadding.x);

        bool close = PrimaryButton("关闭", ImVec2(closeWidth, S(38.0f)));
        if (ImGui::IsKeyPressed(ImGuiKey_Escape))
            close = true;

        if (close)
        {
            model.Ui().showSettings = false;
            popupOpened = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}
