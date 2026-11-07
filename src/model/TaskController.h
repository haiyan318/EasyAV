#pragma once

// 任务状态模型。
// 当前为“界面演示”实现：在 UI 线程按时间推进进度，不做任何媒体处理。
// 后续接入 FFmpeg 时，只需替换本类内部实现（或改为消费后台线程事件），
// 对外暴露的查询接口保持不变，UI 面板无需修改。

#include "core/Types.h"

#include <string>
#include <vector>

namespace easyav
{
    class TaskController
    {
    public:
        bool Start(const std::vector<std::string>& displayNames);
        void Cancel();
        void Reset();
        void Tick(float deltaSeconds);

        TaskPhase Phase() const { return m_phase; }
        bool IsRunning() const { return m_phase == TaskPhase::Running; }
        bool HasFinished() const { return m_phase == TaskPhase::Completed; }

        // 完成事件只上报一次，供 App 弹提示 / 打开输出目录。
        bool ConsumeCompleted();

        float Progress() const { return m_progress; }
        double ElapsedSeconds() const { return m_elapsedSec; }
        double EtaSeconds() const;
        int FileCount() const { return static_cast<int>(m_names.size()); }
        int CurrentFileIndex() const;
        std::string CurrentFileName() const;
        std::string StatusText() const;

    private:
        TaskPhase m_phase = TaskPhase::Idle;
        std::vector<std::string> m_names;
        float m_progress = 0.0f;
        double m_elapsedSec = 0.0;
        bool m_preparing = true;
        bool m_completedEvent = false;
    };
}
