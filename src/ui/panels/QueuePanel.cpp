#include "ui/panels/QueuePanel.h"

#include "core/Format.h"
#include "ui/Labels.h"
#include "ui/Theme.h"
#include "ui/Widgets.h"

#include <algorithm>
#include <cstdio>
#include <string>

namespace easyav::ui
{
    namespace
    {
        ImVec4 KindColor(MediaKind kind)
        {
            switch (kind)
            {
            case MediaKind::Video: return Colors().accent;
            case MediaKind::Audio: return Colors().success;
            case MediaKind::Image: return Colors().warning;
            default:               return Colors().textMuted;
            }
        }

        void DrawDropZone(AppModel& model, const ImVec2& origin, const ImVec2& size)
        {
            const UiColors& c = Colors();
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            const ImVec2 max(origin.x + size.x, origin.y + size.y);

            drawList->AddRectFilled(origin, max, ColorU32(c.panelAlt, 0.65f), 14.0f);
            DrawDashedRect(drawList, ImVec2(origin.x + 1.0f, origin.y + 1.0f),
                           ImVec2(max.x - 1.0f, max.y - 1.0f),
                           ImGui::GetColorU32(c.border), 14.0f, 1.5f, S(9.0f), S(7.0f));

            const float base = ImGui::GetFontSize();
            const float centerX = origin.x + size.x * 0.5f;
            const float iconY = origin.y + size.y * 0.26f;

            drawList->AddCircleFilled(ImVec2(centerX, iconY), S(26.0f),
                                      ImGui::GetColorU32(c.accentSoft));
            drawList->AddLine(ImVec2(centerX - S(9.0f), iconY), ImVec2(centerX + S(9.0f), iconY),
                              ImGui::GetColorU32(c.accent), 2.4f);
            drawList->AddLine(ImVec2(centerX, iconY - S(9.0f)), ImVec2(centerX, iconY + S(9.0f)),
                              ImGui::GetColorU32(c.accent), 2.4f);

            const char* title = "把视频、音乐或文件夹拖到这里";
            const ImVec2 titleSize = ImGui::CalcTextSize(title);
            drawList->AddText(ImGui::GetFont(), base * 1.02f,
                              ImVec2(centerX - titleSize.x * 0.5f, iconY + S(40.0f)),
                              ImGui::GetColorU32(c.text), title);

            const char* subtitle = "支持 MP4、MKV、MP3、GIF 等常见格式";
            const float hintSize = base * 0.78f;
            const ImVec2 subtitleSize = ImGui::GetFont()->CalcTextSizeA(hintSize, FLT_MAX, 0.0f, subtitle);
            drawList->AddText(ImGui::GetFont(), hintSize,
                              ImVec2(centerX - subtitleSize.x * 0.5f, iconY + S(68.0f)),
                              ImGui::GetColorU32(c.textMuted), subtitle);

            const float buttonWidth = S(148.0f);
            const float buttonHeight = S(38.0f);
            ImGui::SetCursorScreenPos(ImVec2(centerX - buttonWidth * 0.5f, origin.y + size.y - S(62.0f)));
            if (SecondaryButton("选择文件…", ImVec2(buttonWidth, buttonHeight)))
                model.Ui().requestAddFiles = true;
        }
    }

    void DrawQueuePanel(AppModel& model)
    {
        const UiColors& c = Colors();
        const float base = ImGui::GetFontSize();
        const ImVec2 contentOrigin = ImGui::GetCursorScreenPos();
        const float contentWidth = ImGui::GetContentRegionAvail().x;

        // 标题行
        PushFontSize(base * 1.22f);
        ImGui::TextUnformatted("文件队列");
        PopFontSize();

        ImGui::SameLine(0.0f, 8.0f);
        char countText[8] = {};
        std::snprintf(countText, sizeof(countText), "%d", model.Queue().Count());
        Badge(countText, c.accentSoft, c.accent);

        const float clearWidth = S(58.0f);
        const float headerHeight = S(30.0f);
        ImGui::SetCursorScreenPos(ImVec2(contentOrigin.x + contentWidth - clearWidth, contentOrigin.y));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(S(8.0f), S(4.0f)));
        ImGui::BeginDisabled(model.Queue().Empty() || model.Task().IsRunning());
        const bool clearClicked = SecondaryButton("清空", ImVec2(clearWidth, headerHeight));
        ImGui::EndDisabled();
        ImGui::PopStyleVar();
        if (clearClicked && !model.Queue().Empty())
        {
            model.Queue().Clear();
            model.ShowToast("队列已清空", ToastLevel::Info);
        }

        ImGui::SetCursorScreenPos(ImVec2(contentOrigin.x, contentOrigin.y + S(38.0f)));
        if (model.Queue().Empty())
        {
            Hint("文件只会在本机处理，不会上传。");
            ImGui::Dummy(ImVec2(0.0f, S(6.0f)));

            const ImVec2 avail = ImGui::GetContentRegionAvail();
            const float dropHeight = std::min(std::max(avail.y - S(8.0f), S(240.0f)), S(330.0f));
            const ImVec2 dropOrigin = ImGui::GetCursorScreenPos();
            ImGui::Dummy(ImVec2(avail.x, dropHeight));
            DrawDropZone(model, dropOrigin, ImVec2(avail.x, dropHeight));
            return;
        }

        Hint(model.Task().IsRunning() ? "处理中，队列暂时不能修改。"
                                      : "拖入新文件会继续加入队列。");
        ImGui::Dummy(ImVec2(0.0f, S(2.0f)));

        const float listHeight = ImGui::GetContentRegionAvail().y - S(48.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::BeginChild("##queue_list", ImVec2(0.0f, std::max(listHeight, S(80.0f))),
                          ImGuiChildFlags_None, ImGuiWindowFlags_None);

        int removeIndex = -1;
        const auto& items = model.Queue().Items();
        const float rowHeight = S(64.0f);
        const float rowGap = S(4.0f);

        for (int i = 0; i < static_cast<int>(items.size()); ++i)
        {
            const MediaItem& item = items[static_cast<std::size_t>(i)];
            ImGui::PushID(item.id);

            const ImVec2 rowOrigin = ImGui::GetCursorScreenPos();
            const float rowWidth = ImGui::GetContentRegionAvail().x;

            ImGui::Selectable("##row", false, ImGuiSelectableFlags_AllowOverlap,
                              ImVec2(rowWidth, rowHeight));
            const bool hovered = ImGui::IsItemHovered();

            ImDrawList* drawList = ImGui::GetWindowDrawList();
            const ImU32 rowBg = hovered ? ImGui::GetColorU32(c.panelAlt)
                                        : ImGui::GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            drawList->AddRectFilled(rowOrigin, ImVec2(rowOrigin.x + rowWidth, rowOrigin.y + rowHeight),
                                    rowBg, 10.0f);

            const ImVec2 dotCenter(rowOrigin.x + S(14.0f), rowOrigin.y + rowHeight * 0.5f);
            drawList->AddCircleFilled(dotCenter, S(4.5f), ImGui::GetColorU32(KindColor(item.kind)));

            const float textX = rowOrigin.x + S(28.0f);
            const float textWidth = std::max(rowWidth - S(64.0f), S(40.0f));
            const std::string name = Ellipsize(item.displayName, textWidth);
            drawList->AddText(ImGui::GetFont(), base,
                              ImVec2(textX, rowOrigin.y + S(10.0f)),
                              ImGui::GetColorU32(c.text), name.c_str());

            char meta[192] = {};
            std::snprintf(meta, sizeof(meta), "%s · %s · %s · %s",
                          item.format.c_str(), KindLabel(item.kind),
                          FormatBytes(item.sizeBytes).c_str(),
                          item.durationSec > 0.0 ? FormatClock(item.durationSec).c_str() : "时长待分析");
            drawList->AddText(ImGui::GetFont(), base * 0.76f,
                              ImVec2(textX, rowOrigin.y + S(36.0f)),
                              ImGui::GetColorU32(c.textMuted), meta);

            // 移除按钮
            ImGui::SetCursorScreenPos(ImVec2(rowOrigin.x + rowWidth - S(32.0f), rowOrigin.y + S(20.0f)));
            ImGui::BeginDisabled(model.Task().IsRunning());
            ImGui::InvisibleButton("##remove", ImVec2(S(24.0f), S(24.0f)));
            const bool removeHovered = ImGui::IsItemHovered();
            if (removeHovered)
                ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            if (ImGui::IsItemClicked())
                removeIndex = i;
            ImGui::EndDisabled();
            if (removeHovered)
            {
                drawList->AddCircleFilled(ImVec2(rowOrigin.x + rowWidth - S(20.0f), rowOrigin.y + S(32.0f)),
                                          S(13.0f), ImGui::GetColorU32(c.panelHover));
            }
            drawList->AddText(ImGui::GetFont(), base * 1.02f,
                              ImVec2(rowOrigin.x + rowWidth - S(25.0f), rowOrigin.y + S(20.0f)),
                              ImGui::GetColorU32(removeHovered ? c.danger : c.textMuted), "×");

            ImGui::SetCursorScreenPos(ImVec2(rowOrigin.x, rowOrigin.y + rowHeight + rowGap));
            ImGui::PopID();
        }

        ImGui::EndChild();
        ImGui::PopStyleVar();

        if (removeIndex >= 0)
        {
            model.Queue().RemoveAt(removeIndex);
            model.ShowToast("已移除 1 个文件", ToastLevel::Info);
        }

        // 底部添加按钮
        const float buttonWidth = (ImGui::GetContentRegionAvail().x - S(8.0f)) * 0.5f;
        ImGui::BeginDisabled(model.Task().IsRunning());
        if (SecondaryButton("+ 添加文件", ImVec2(buttonWidth, S(40.0f))))
            model.Ui().requestAddFiles = true;
        ImGui::SameLine(0.0f, S(8.0f));
        if (SecondaryButton("添加文件夹", ImVec2(buttonWidth, S(40.0f))))
            model.Ui().requestAddFolder = true;
        ImGui::EndDisabled();
    }
}
