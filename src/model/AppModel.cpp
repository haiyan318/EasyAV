#include "model/AppModel.h"

#include "core/Format.h"
#include "core/Text.h"

#include <algorithm>
#include <cstdio>
#include <filesystem>

namespace easyav
{
    void AppModel::AddPaths(const std::vector<std::string>& utf8Paths)
    {
        if (utf8Paths.empty())
            return;

        int added = 0;
        int skipped = 0;

        for (const std::string& path : utf8Paths)
        {
            std::error_code ec;
            const std::filesystem::path fsPath = PathFromUtf8(path);
            if (std::filesystem::is_directory(fsPath, ec) && !ec)
                added += m_queue.AddDirectory(path, &skipped);
            else if (m_queue.AddFile(path) > 0)
                ++added;
            else
                ++skipped;
        }

        char message[160] = {};
        if (added > 0 && skipped == 0)
            std::snprintf(message, sizeof(message), "已添加 %d 个文件", added);
        else if (added > 0)
            std::snprintf(message, sizeof(message), "已添加 %d 个文件，跳过 %d 个", added, skipped);
        else
            std::snprintf(message, sizeof(message), "没有可添加的文件（重复或不支持）");

        ShowToast(message, added > 0 ? ToastLevel::Success : ToastLevel::Warning);
    }

    void AppModel::Tick(float deltaSeconds)
    {
        m_task.Tick(deltaSeconds);

        if (m_record.recording)
            m_record.elapsedSec += deltaSeconds;

        for (Toast& toast : m_toasts)
            toast.remaining -= deltaSeconds;

        m_toasts.erase(std::remove_if(m_toasts.begin(), m_toasts.end(),
                                      [](const Toast& toast) { return toast.remaining <= 0.0f; }),
                       m_toasts.end());
    }

    void AppModel::ShowToast(std::string text, ToastLevel level)
    {
        Toast toast;
        toast.text = std::move(text);
        toast.level = level;
        toast.remaining = 3.2f;

        if (m_toasts.size() >= 4)
            m_toasts.erase(m_toasts.begin());
        m_toasts.push_back(std::move(toast));
    }

    unsigned long long AppModel::EstimateOutputBytes() const
    {
        const unsigned long long total = m_queue.TotalBytes();
        if (total == 0)
            return 0;

        double factor = 0.20 + 0.72 * static_cast<double>(m_compress.quality);
        switch (m_compress.format)
        {
        case TargetFormat::MP3:
            factor = 0.08;
            break;
        case TargetFormat::GIF:
            factor = 1.40;
            break;
        case TargetFormat::MKV:
            factor *= 1.05;
            break;
        case TargetFormat::MP4:
        default:
            break;
        }

        return static_cast<unsigned long long>(static_cast<double>(total) * factor);
    }

    std::string AppModel::QueueSummary() const
    {
        if (m_queue.Empty())
            return "队列为空";

        char buffer[96] = {};
        std::snprintf(buffer, sizeof(buffer), "%d 个文件 · 共 %s",
                      m_queue.Count(), FormatBytes(m_queue.TotalBytes()).c_str());
        return buffer;
    }

    std::string AppModel::OutputFolderDisplay() const
    {
        if (m_output.sameAsSource)
            return "与原文件放在一起";
        if (m_output.outputDir[0] == '\0')
            return "尚未选择文件夹";
        return m_output.outputDir;
    }

    std::string AppModel::ResolvedOutputFolder() const
    {
        if (m_output.sameAsSource)
            return {};
        return m_output.outputDir;
    }
}
