#pragma once

// 应用状态聚合根。
// UI 面板只读写这里的数据，不互相引用；
// 平台/业务能力以后通过接口注入，避免面板直接依赖具体实现。

#include "core/TaskContracts.h"
#include "model/MediaQueue.h"
#include "model/TaskController.h"
#include "model/TaskSettings.h"

#include <string>
#include <vector>

namespace easyav
{
    class AppModel
    {
    public:
        MediaQueue& Queue() { return m_queue; }
        const MediaQueue& Queue() const { return m_queue; }

        ConvertSettings& Convert() { return m_convert; }
        const ConvertSettings& Convert() const { return m_convert; }

        ClipSettings& Clip() { return m_clip; }
        const ClipSettings& Clip() const { return m_clip; }

        FilterSettings& Filter() { return m_filter; }
        const FilterSettings& Filter() const { return m_filter; }

        RecordSettings& Record() { return m_record; }
        const RecordSettings& Record() const { return m_record; }

        OutputSettings& Output() { return m_output; }
        const OutputSettings& Output() const { return m_output; }

        UiState& Ui() { return m_ui; }
        const UiState& Ui() const { return m_ui; }

        TaskController& Task() { return m_task; }
        const TaskController& Task() const { return m_task; }

        void AddPaths(const std::vector<std::string>& utf8Paths);
        void ApplyProbeResult(const std::string& utf8Path, const MediaInfo& info);

        // 把队列与当前设置拍成快照，交给 TaskController/TaskService。
        TaskRequest MakeTaskRequest() const;
        void Tick(float deltaSeconds);

        void ShowToast(std::string text, ToastLevel level = ToastLevel::Info);
        const std::vector<Toast>& Toasts() const { return m_toasts; }

        unsigned long long EstimateOutputBytes() const;
        std::string QueueSummary() const;
        std::string OutputFolderDisplay() const;
        std::string ResolvedOutputFolder() const;

    private:
        MediaQueue m_queue;
        ConvertSettings m_convert;
        ClipSettings m_clip;
        FilterSettings m_filter;
        RecordSettings m_record;
        OutputSettings m_output;
        UiState m_ui;
        TaskController m_task;
        std::vector<Toast> m_toasts;
    };
}
