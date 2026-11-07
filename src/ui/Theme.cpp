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
        style.WindowRounding = 14.0f;
        style.WindowBorderSize = 0.0f;
        style.ChildRounding = 12.0f;
        style.ChildBorderSize = 1.0f;
        style.PopupRounding = 12.0f;
        style.PopupBorderSize = 1.0f;
        style.FramePadding = ImVec2(12.0f, 8.0f);
        style.FrameRounding = 9.0f;
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
            g_colors = UiColors{
                Rgb(20, 22, 27),
                Rgb(30, 33, 40),
                Rgb(37, 41, 49),
                Rgb(46, 51, 60),
                Rgb(55, 60, 71),
                Rgb(232, 236, 241),
                Rgb(160, 169, 182),
                Rgb(74, 139, 255),
                Rgb(93, 154, 255),
                Rgb(52, 118, 235),
                Rgb(37, 56, 92),
                Rgb(62, 189, 131),
                Rgb(246, 174, 60),
                Rgb(242, 102, 102),
                Rgb(54, 59, 69),
                Rgb(255, 255, 255)
            };
        }
        else
        {
            g_colors = UiColors{
                Rgb(244, 246, 249),
                Rgb(255, 255, 255),
                Rgb(247, 249, 252),
                Rgb(237, 241, 247),
                Rgb(226, 231, 238),
                Rgb(30, 38, 52),
                Rgb(107, 116, 132),
                Rgb(47, 115, 241),
                Rgb(36, 99, 224),
                Rgb(27, 82, 192),
                Rgb(229, 238, 255),
                Rgb(22, 163, 103),
                Rgb(242, 158, 33),
                Rgb(230, 76, 76),
                Rgb(229, 233, 240),
                Rgb(255, 255, 255)
            };
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
