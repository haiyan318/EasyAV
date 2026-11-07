#include "ui/panels/ToastOverlay.h"

#include "ui/Theme.h"

#include <algorithm>
#include <cfloat>

namespace easyav::ui
{
    namespace
    {
        ImVec4 LevelColor(ToastLevel level)
        {
            switch (level)
            {
            case ToastLevel::Success: return Colors().success;
            case ToastLevel::Warning: return Colors().warning;
            case ToastLevel::Error:   return Colors().danger;
            case ToastLevel::Info:
            default:                  return Colors().accent;
            }
        }
    }

    void DrawToastOverlay(const AppModel& model)
    {
        const auto& toasts = model.Toasts();
        if (toasts.empty())
            return;

        const UiColors& c = Colors();
        const float base = ImGui::GetFontSize();
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        const float width = S(386.0f);
        const float paddingX = S(18.0f);
        float bottom = viewport->WorkPos.y + viewport->WorkSize.y - S(46.0f);

        ImDrawList* drawList = ImGui::GetForegroundDrawList();

        for (int i = static_cast<int>(toasts.size()) - 1; i >= 0; --i)
        {
            const Toast& toast = toasts[static_cast<std::size_t>(i)];
            const float alpha = std::clamp(toast.remaining / 0.45f, 0.0f, 1.0f);
            const ImVec2 textSize = ImGui::CalcTextSize(toast.text.c_str(), nullptr, false,
                                                        width - paddingX * 2.0f - 10.0f);
            const float height = std::max(S(46.0f), textSize.y + S(24.0f));
            const ImVec2 min(viewport->WorkPos.x + viewport->WorkSize.x - width - S(18.0f),
                             bottom - height);
            const ImVec2 max(min.x + width, bottom);

            drawList->AddRectFilled(ImVec2(min.x + S(2.0f), min.y + S(3.0f)), ImVec2(max.x + S(2.0f), max.y + S(3.0f)),
                                    ImGui::GetColorU32(ImVec4(0.05f, 0.07f, 0.10f, 0.08f * alpha)), S(8.0f));
            drawList->AddRectFilled(min, max,
                                    ImGui::GetColorU32(ImVec4(c.panel.x, c.panel.y, c.panel.z, alpha)), S(8.0f));
            drawList->AddRect(min, max,
                              ImGui::GetColorU32(ImVec4(c.border.x, c.border.y, c.border.z, alpha)),
                              S(8.0f), 0, 1.0f);

            const ImVec4 level = LevelColor(toast.level);
            drawList->AddRectFilled(ImVec2(min.x + S(1.0f), min.y + S(10.0f)),
                                    ImVec2(min.x + S(5.0f), max.y - S(10.0f)),
                                    ImGui::GetColorU32(ImVec4(level.x, level.y, level.z, alpha)), 2.0f);
            drawList->AddCircleFilled(ImVec2(min.x + S(24.0f), min.y + height * 0.5f), S(4.0f),
                                      ImGui::GetColorU32(ImVec4(level.x, level.y, level.z, alpha)));

            drawList->AddText(ImGui::GetFont(), base * 0.86f,
                              ImVec2(min.x + paddingX + S(10.0f),
                                     min.y + (height - textSize.y) * 0.5f),
                              ImGui::GetColorU32(ImVec4(c.text.x, c.text.y, c.text.z, alpha)),
                              toast.text.c_str(), nullptr, width - paddingX * 2.0f - 10.0f);

            bottom -= height + S(8.0f);
        }
    }
}
