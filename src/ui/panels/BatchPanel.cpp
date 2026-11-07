#include "ui/panels/BatchPanel.h"

#include "core/Format.h"
#include "ui/Labels.h"
#include "ui/Theme.h"
#include "ui/Widgets.h"

#include <algorithm>
#include <cstdio>
#include <string>

namespace easyav::ui
{
    void DrawBatchPanel(AppModel& model)
    {
        const UiColors& c = Colors();
        const float width = ImGui::GetContentRegionAvail().x;
        const int count = model.Queue().Count();

        SectionHeader("批量处理", "设一次规则，整个队列都照做。");

        CardBegin("##rule_summary", ImVec2(0.0f, S(132.0f)), ImVec2(S(18.0f), S(14.0f)));
        ImGui::PushStyleColor(ImGuiCol_Text, c.textMuted);
        ImGui::TextUnformatted("当前规则");
        ImGui::PopStyleColor();
        ImGui::Dummy(ImVec2(0.0f, S(2.0f)));

        const auto& convert = model.Convert();
        const auto& filter = model.Filter();
        Badge(ConvertTargetLabel(convert.target), c.accentSoft, c.accent);
        ImGui::SameLine(0.0f, S(6.0f));
        Badge(convert.target == ConvertTarget::Audio
                  ? "MP3"
                  : ContainerShort(convert.container),
              c.panelAlt, c.text);
        ImGui::SameLine(0.0f, 6.0f);
        Badge(convert.slim ? QualityLabel(convert.slimQuality) : "保持画质",
              c.panelAlt, c.text);
        ImGui::SameLine(0.0f, 6.0f);
        Badge(convert.useGpuAccel ? "显卡加速" : "CPU 处理", c.panelAlt, c.text);
        ImGui::SameLine(0.0f, 6.0f);
        Badge(WatermarkLabel(filter.watermark), c.panelAlt, c.text);

        ImGui::Dummy(ImVec2(0.0f, S(4.0f)));
        char summary[256] = {};
        std::snprintf(summary, sizeof(summary), "编码转换 · %s · %s · %s",
                      ConvertTargetLabel(convert.target),
                      convert.slim ? QualityLabel(convert.slimQuality) : "保持画质",
                      WatermarkLabel(filter.watermark));
        Hint(summary);
        CardEnd();

        ImGui::Dummy(ImVec2(0.0f, S(10.0f)));

        if (count == 0)
        {
            Hint("队列还是空的。先添加文件，再回来一键套用规则。");
            ImGui::Dummy(ImVec2(0.0f, S(4.0f)));
            if (SecondaryButton("+ 添加文件", ImVec2(S(160.0f), S(40.0f))))
                model.Ui().requestAddFiles = true;
            return;
        }

        ImGui::Text("将处理队列中的 %d 个文件", count);
        ImGui::Dummy(ImVec2(0.0f, S(2.0f)));

        const auto& items = model.Queue().Items();
        const int previewCount = std::min(count, 5);
        for (int i = 0; i < previewCount; ++i)
        {
            const MediaItem& item = items[static_cast<std::size_t>(i)];
            ImGui::PushStyleColor(ImGuiCol_Text, c.textMuted);
            ImGui::Bullet();
            ImGui::PopStyleColor();
            ImGui::SameLine(0.0f, S(6.0f));
            const std::string name = Ellipsize(item.displayName, width - S(120.0f));
            ImGui::TextUnformatted(name.c_str());
            ImGui::SameLine(0.0f, S(8.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, c.textMuted);
            ImGui::Text("· %s · %s", item.format.c_str(), FormatBytes(item.sizeBytes).c_str());
            ImGui::PopStyleColor();
        }

        if (count > previewCount)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, c.textMuted);
            ImGui::Text("… 还有 %d 个文件", count - previewCount);
            ImGui::PopStyleColor();
        }

        ImGui::Dummy(ImVec2(0.0f, S(8.0f)));

        char buttonLabel[64] = {};
        std::snprintf(buttonLabel, sizeof(buttonLabel), "应用到全部 %d 个文件", count);
        if (PrimaryButton(buttonLabel, ImVec2(-1.0f, S(46.0f))))
        {
            char message[96] = {};
            std::snprintf(message, sizeof(message), "已把当前规则应用到 %d 个文件", count);
            model.ShowToast(message, ToastLevel::Success);
        }

        ImGui::Dummy(ImVec2(0.0f, S(2.0f)));
        Hint("规则对所有文件统一生效；如需单独设置，可在左侧移除不需要的文件。");
    }
}
