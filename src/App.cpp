#include "App.h"

#include "core/Text.h"
#include "platform/FileDialogs.h"
#include "ui/Theme.h"
#include "ui/Widgets.h"
#include "ui/panels/ActionBar.h"
#include "ui/panels/QueuePanel.h"
#include "ui/panels/SceneTabs.h"
#include "ui/panels/SettingsDialog.h"
#include "ui/panels/StatusBar.h"
#include "ui/panels/ToastOverlay.h"
#include "ui/panels/TopBar.h"

#include <algorithm>
#include <cstdio>
#include <filesystem>

namespace easyav
{
    namespace
    {
        void CopyToBuffer(char* buffer, std::size_t size, const std::string& text)
        {
            if (size == 0)
                return;
            std::snprintf(buffer, size, "%s", text.c_str());
        }
    }

    void App::Init()
    {
        if (!ui::LoadFonts())
        {
            m_model.ShowToast("未找到中文字体，已改用内置字体", ToastLevel::Warning);
        }
    }

    void App::Frame()
    {
        // 最小化/卡顿恢复时避免一次推进过多进度。
        const float deltaSeconds = std::min(ImGui::GetIO().DeltaTime, 0.1f);
        m_model.Tick(deltaSeconds);

        if (m_model.Task().ConsumeCompleted())
        {
            m_model.ShowToast("全部处理完成，输出文件已保存", ToastLevel::Success);
            if (m_model.Output().openFolderWhenDone)
                m_model.Ui().requestOpenOutputDir = true;
        }

        ProcessRequests();

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ui::Colors().bg);

        const ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
                                     | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse
                                     | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
                                     | ImGuiWindowFlags_NoSavedSettings
                                     | ImGuiWindowFlags_NoBringToFrontOnFocus;

        ImGui::Begin("##EasyAVRoot", nullptr, flags);
        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);

        const float windowWidth = ImGui::GetWindowWidth();
        const float windowHeight = ImGui::GetWindowHeight();
        const float pagePadding = ui::S(14.0f);
        const float topBarHeight = ui::S(76.0f);
        const float statusBarHeight = ui::S(30.0f);
        const float contentGap = ui::S(12.0f);
        const float actionBarHeight = ui::S(120.0f);

        ImGui::SetCursorPos(ImVec2(0.0f, 0.0f));
        ui::DrawTopBar(m_model);

        const float bodyTop = topBarHeight + ui::S(10.0f);
        const float bodyHeight = std::max(windowHeight - bodyTop - statusBarHeight, ui::S(200.0f));
        const float leftWidth = std::clamp(windowWidth * 0.28f, ui::S(330.0f), ui::S(420.0f));

        ImGui::SetCursorPos(ImVec2(pagePadding, bodyTop));
        ui::CardBegin("##queue_card", ImVec2(leftWidth, bodyHeight), ImVec2(ui::S(16.0f), ui::S(16.0f)));
        ui::DrawQueuePanel(m_model);
        ui::CardEnd();

        const float mainX = pagePadding + leftWidth + contentGap;
        const float mainWidth = std::max(windowWidth - mainX - pagePadding, ui::S(200.0f));

        ImGui::SetCursorPos(ImVec2(mainX, bodyTop));
        ImGui::BeginChild("##scene_area",
                          ImVec2(mainWidth, std::max(bodyHeight - actionBarHeight - ui::S(10.0f), ui::S(120.0f))),
                          ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar);
        ui::DrawSceneTabs(m_model);
        ImGui::EndChild();

        ImGui::SetCursorPos(ImVec2(mainX, bodyTop + bodyHeight - actionBarHeight));
        ui::DrawActionBar(m_model);

        ImGui::SetCursorPos(ImVec2(0.0f, windowHeight - statusBarHeight));
        ui::DrawStatusBar(m_model);

        ImGui::End();

        ui::DrawSettingsDialog(m_model);
        ui::DrawToastOverlay(m_model);
    }

    void App::AddDroppedPaths(const std::vector<std::string>& utf8Paths)
    {
        if (m_model.Task().IsRunning())
        {
            m_model.ShowToast("处理中，暂时不能修改队列", ToastLevel::Warning);
            return;
        }
        m_model.AddPaths(utf8Paths);
    }

    void App::ProcessRequests()
    {
        UiState& ui = m_model.Ui();

        if (ui.requestAddFiles)
        {
            ui.requestAddFiles = false;
            const std::vector<std::string> files = platform::PickMediaFiles();
            if (!files.empty())
                m_model.AddPaths(files);
        }

        if (ui.requestAddFolder)
        {
            ui.requestAddFolder = false;
            const std::string folder = platform::PickFolder();
            if (!folder.empty())
                m_model.AddPaths({ folder });
        }

        if (ui.requestPickWatermarkImage)
        {
            ui.requestPickWatermarkImage = false;
            const std::string file = platform::PickImageFile();
            if (!file.empty())
            {
                CopyToBuffer(m_model.Filter().watermarkImagePath,
                             sizeof(m_model.Filter().watermarkImagePath), file);
                m_model.Filter().watermark = WatermarkMode::Image;
                m_model.ShowToast("水印图片已选择", ToastLevel::Success);
            }
        }

        if (ui.requestPickOutputDir)
        {
            ui.requestPickOutputDir = false;
            const std::string folder = platform::PickFolder();
            if (!folder.empty())
            {
                CopyToBuffer(m_model.Output().outputDir, sizeof(m_model.Output().outputDir), folder);
                m_model.Output().sameAsSource = false;
            }
        }

        if (ui.requestOpenOutputDir)
        {
            ui.requestOpenOutputDir = false;

            std::string folder = m_model.ResolvedOutputFolder();
            if (folder.empty() && !m_model.Queue().Empty())
            {
                const std::filesystem::path path = PathFromUtf8(m_model.Queue().Items().front().path);
                folder = PathToUtf8(path.parent_path());
            }

            if (!folder.empty())
                platform::OpenFolder(folder);
            else
                m_model.ShowToast("还没有可打开的输出文件夹", ToastLevel::Warning);
        }
    }
}
