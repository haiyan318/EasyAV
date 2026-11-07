#include "ui/Theme.h"

#include <fstream>

namespace easyav::ui
{
    namespace
    {
        UiColors g_colors;

        ImVec4 Rgb(int r, int g, int b, float a = 1.0f)
        {
            return ImVec4(static_cast<float>(r) / 255.0f,
                          static_cast<float>(g) / 255.0f,
                          static_cast<float>(b) / 255.0f, a);
        }

        bool FileExists(const char* path)
        {
            std::ifstream file(path, std::ios::binary);
            return file.good();
        }
    }

    const UiColors& Colors()
    {
        return g_colors;
    }

    float UiScale()
    {
        const float scale = ImGui::GetStyle().FontScaleDpi;
        return scale > 0.1f ? scale : 1.0f;
    }

    void ApplyMetrics()
    {
        ImGuiStyle& style = ImGui::GetStyle();

        style.WindowPadding = ImVec2(16.0f, 16.0f);
        style.WindowRounding = 16.0f;
        style.WindowBorderSize = 0.0f;
        style.ChildRounding = 14.0f;
        style.ChildBorderSize = 1.0f;
        style.PopupRounding = 14.0f;
        style.PopupBorderSize = 1.0f;
        style.FramePadding = ImVec2(12.0f, 8.0f);
        style.FrameRounding = 10.0f;
        style.FrameBorderSize = 0.0f;
        style.ItemSpacing = ImVec2(10.0f, 10.0f);
        style.ItemInnerSpacing = ImVec2(8.0f, 6.0f);
        style.CellPadding = ImVec2(10.0f, 8.0f);
        style.IndentSpacing = 18.0f;
        style.ScrollbarSize = 12.0f;
        style.ScrollbarRounding = 8.0f;
        style.GrabMinSize = 10.0f;
        style.GrabRounding = 6.0f;
        style.TabRounding = 9.0f;
        style.TabBorderSize = 0.0f;
        style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
        style.SelectableTextAlign = ImVec2(0.0f, 0.5f);
        style.SeparatorTextBorderSize = 1.0f;
        style.SeparatorTextPadding = ImVec2(12.0f, 10.0f);
        style.DisabledAlpha = 0.45f;
    }

    void ApplyTheme(bool dark)
    {
        if (dark)
        {
            g_colors.bg = Rgb(19, 21, 26);
            g_colors.panel = Rgb(29, 32, 39);
            g_colors.panelAlt = Rgb(36, 40, 48);
            g_colors.panelHover = Rgb(45, 50, 60);
            g_colors.border = Rgb(55, 60, 71);
            g_colors.text = Rgb(232, 236, 241);
            g_colors.textMuted = Rgb(160, 169, 182);
            g_colors.accent = Rgb(74, 139, 255);
            g_colors.accentHover = Rgb(93, 154, 255);
            g_colors.accentActive = Rgb(52, 118, 235);
            g_colors.accentSoft = Rgb(37, 56, 92);
            g_colors.success = Rgb(62, 189, 131);
            g_colors.warning = Rgb(246, 174, 60);
            g_colors.danger = Rgb(242, 102, 102);
            g_colors.track = Rgb(54, 59, 69);
            g_colors.white = Rgb(255, 255, 255);

            g_colors.accentGradTop = Rgb(101, 160, 255);
            g_colors.accentGradBottom = Rgb(63, 122, 240);
            g_colors.accentGlow = Rgb(74, 139, 255, 0.30f);
            g_colors.cardShadow = Rgb(0, 0, 0, 0.38f);
            g_colors.cardHover = Rgb(73, 81, 95);
            g_colors.selectedSoft = Rgb(74, 139, 255, 0.14f);
            g_colors.successSoft = Rgb(62, 189, 131, 0.16f);
            g_colors.dangerSoft = Rgb(242, 102, 102, 0.16f);
        }
        else
        {
            g_colors.bg = Rgb(242, 245, 250);
            g_colors.panel = Rgb(255, 255, 255);
            g_colors.panelAlt = Rgb(246, 249, 253);
            g_colors.panelHover = Rgb(236, 241, 249);
            g_colors.border = Rgb(224, 230, 239);
            g_colors.text = Rgb(28, 36, 50);
            g_colors.textMuted = Rgb(106, 116, 132);
            g_colors.accent = Rgb(45, 112, 240);
            g_colors.accentHover = Rgb(62, 128, 252);
            g_colors.accentActive = Rgb(33, 92, 210);
            g_colors.accentSoft = Rgb(228, 238, 255);
            g_colors.success = Rgb(20, 158, 100);
            g_colors.warning = Rgb(240, 156, 32);
            g_colors.danger = Rgb(226, 72, 72);
            g_colors.track = Rgb(227, 232, 240);
            g_colors.white = Rgb(255, 255, 255);

            g_colors.accentGradTop = Rgb(84, 146, 255);
            g_colors.accentGradBottom = Rgb(41, 104, 232);
            g_colors.accentGlow = Rgb(45, 112, 240, 0.26f);
            g_colors.cardShadow = Rgb(28, 44, 78, 0.10f);
            g_colors.cardHover = Rgb(203, 214, 231);
            g_colors.selectedSoft = Rgb(45, 112, 240, 0.09f);
            g_colors.successSoft = Rgb(20, 158, 100, 0.13f);
            g_colors.dangerSoft = Rgb(226, 72, 72, 0.13f);
        }

        const UiColors& c = g_colors;
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        colors[ImGuiCol_Text] = c.text;
        colors[ImGuiCol_TextDisabled] = c.textMuted;
        colors[ImGuiCol_WindowBg] = c.bg;
        colors[ImGuiCol_ChildBg] = c.panel;
        colors[ImGuiCol_PopupBg] = c.panel;
        colors[ImGuiCol_Border] = c.border;
        colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

        colors[ImGuiCol_FrameBg] = c.panelAlt;
        colors[ImGuiCol_FrameBgHovered] = c.panelHover;
        colors[ImGuiCol_FrameBgActive] = c.track;

        colors[ImGuiCol_TitleBg] = c.panel;
        colors[ImGuiCol_TitleBgActive] = c.panel;
        colors[ImGuiCol_TitleBgCollapsed] = c.panel;
        colors[ImGuiCol_MenuBarBg] = c.panel;

        colors[ImGuiCol_ScrollbarBg] = ImVec4(0, 0, 0, 0);
        colors[ImGuiCol_ScrollbarGrab] = c.border;
        colors[ImGuiCol_ScrollbarGrabHovered] = c.textMuted;
        colors[ImGuiCol_ScrollbarGrabActive] = c.accent;

        colors[ImGuiCol_CheckMark] = c.accent;
        colors[ImGuiCol_SliderGrab] = c.accent;
        colors[ImGuiCol_SliderGrabActive] = c.accentActive;

        colors[ImGuiCol_Button] = c.panelAlt;
        colors[ImGuiCol_ButtonHovered] = c.panelHover;
        colors[ImGuiCol_ButtonActive] = c.track;

        colors[ImGuiCol_Header] = c.accentSoft;
        colors[ImGuiCol_HeaderHovered] = c.panelHover;
        colors[ImGuiCol_HeaderActive] = c.track;

        colors[ImGuiCol_Separator] = c.border;
        colors[ImGuiCol_SeparatorHovered] = c.accent;
        colors[ImGuiCol_SeparatorActive] = c.accentActive;

        colors[ImGuiCol_ResizeGrip] = c.border;
        colors[ImGuiCol_ResizeGripHovered] = c.accent;
        colors[ImGuiCol_ResizeGripActive] = c.accentActive;

        colors[ImGuiCol_Tab] = c.panelAlt;
        colors[ImGuiCol_TabHovered] = c.accentSoft;
        colors[ImGuiCol_TabSelected] = c.accentSoft;
        colors[ImGuiCol_TabSelectedOverline] = c.accent;
        colors[ImGuiCol_TabDimmed] = c.panelAlt;
        colors[ImGuiCol_TabDimmedSelected] = c.panelHover;
        colors[ImGuiCol_TabDimmedSelectedOverline] = c.border;

        colors[ImGuiCol_PlotLines] = c.accent;
        colors[ImGuiCol_PlotLinesHovered] = c.accentHover;
        colors[ImGuiCol_PlotHistogram] = c.accent;
        colors[ImGuiCol_PlotHistogramHovered] = c.accentHover;

        colors[ImGuiCol_TableHeaderBg] = c.panelAlt;
        colors[ImGuiCol_TableBorderStrong] = c.border;
        colors[ImGuiCol_TableBorderLight] = c.border;
        colors[ImGuiCol_TableRowBg] = ImVec4(0, 0, 0, 0);
        colors[ImGuiCol_TableRowBgAlt] = c.panelAlt;

        colors[ImGuiCol_TextSelectedBg] = c.accentSoft;
        colors[ImGuiCol_DragDropTarget] = c.accent;
        colors[ImGuiCol_NavHighlight] = c.accent;
        colors[ImGuiCol_NavWindowingHighlight] = c.accent;
        colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0, 0, 0, 0.25f);
        colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.08f, 0.09f, 0.11f, 0.45f);
    }

    bool LoadFonts()
    {
        ImGuiIO& io = ImGui::GetIO();

        static const char* kCandidates[] = {
            "assets/fonts/SourceHanSansCN-Regular.ttf",
            "C:/Windows/Fonts/msyh.ttc",
            "C:/Windows/Fonts/msyhl.ttc",
            "C:/Windows/Fonts/simhei.ttf",
            "C:/Windows/Fonts/Deng.ttf"
        };

        for (const char* path : kCandidates)
        {
            if (!path || !FileExists(path))
                continue;

            ImFontConfig config;
            config.OversampleH = 2;
            config.OversampleV = 2;
            config.PixelSnapH = true;

            if (ImFont* font = io.Fonts->AddFontFromFileTTF(path, 18.0f, &config,
                                                           io.Fonts->GetGlyphRangesChineseFull()))
            {
                io.FontDefault = font;
                return true;
            }
        }

        io.FontDefault = io.Fonts->AddFontDefault();
        return false;
    }
}
