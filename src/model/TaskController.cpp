#include "model/TaskController.h"

#include <algorithm>
#include <cstdio>

namespace easyav
{
    namespace
    {
        constexpr float kPrepareSeconds = 0.6f;
        constexpr float kSecondsPerFile = 2.2f;
    }

    bool TaskController::Start(const std::vector<std::string>& displayNames)
    {
        if (displayNames.empty() || m_phase == TaskPhase::Running)
            return false;

        m_names = displayNames;
        m_phase = TaskPhase::Running;
        m_progress = 0.0f;
        m_elapsedSec = 0.0;
        m_preparing = true;
        m_completedEvent = false;
        return true;
    }

    void TaskController::Cancel()
    {
        if (m_phase != TaskPhase::Running)
            return;

        m_phase = TaskPhase::Canceled;
        m_preparing = false;
    }

    void TaskController::Reset()
    {
        m_phase = TaskPhase::Idle;
        m_names.clear();
        m_progress = 0.0f;
        m_elapsedSec = 0.0;
        m_preparing = true;
        m_completedEvent = false;
    }

    void TaskController::Tick(float deltaSeconds)
    {
        if (m_phase != TaskPhase::Running)
            return;

        m_elapsedSec += static_cast<double>(deltaSeconds);

        if (m_preparing)
        {
            if (m_elapsedSec >= kPrepareSeconds)
                m_preparing = false;
            return;
        }

        const float totalSeconds = std::max(kSecondsPerFile * static_cast<float>(m_names.size()), 1.0f);
        m_progress = std::min(1.0f, m_progress + deltaSeconds / totalSeconds);

        if (m_progress >= 1.0f)
        {
            m_progress = 1.0f;
            m_phase = TaskPhase::Completed;
            m_completedEvent = true;
        }
    }

    bool TaskController::ConsumeCompleted()
    {
        const bool completed = m_completedEvent;
        m_completedEvent = false;
        return completed;
    }

    double TaskController::EtaSeconds() const
    {
        if (m_phase != TaskPhase::Running || m_progress <= 0.02f)
            return -1.0;
        return m_elapsedSec * (1.0 - static_cast<double>(m_progress)) / static_cast<double>(m_progress);
    }

    int TaskController::CurrentFileIndex() const
    {
        if (m_names.empty())
            return 0;
        const int index = static_cast<int>(m_progress * static_cast<float>(m_names.size()));
        return std::clamp(index, 0, static_cast<int>(m_names.size()) - 1);
    }

    std::string TaskController::CurrentFileName() const
    {
        if (m_names.empty())
            return {};
        return m_names[static_cast<std::size_t>(CurrentFileIndex())];
    }

    std::string TaskController::StatusText() const
    {
        switch (m_phase)
        {
        case TaskPhase::Idle:
            return "准备就绪";
        case TaskPhase::Running:
            if (m_preparing)
                return "正在准备…";
            return "正在处理：" + CurrentFileName();
        case TaskPhase::Completed:
            return "全部完成";
        case TaskPhase::Canceled:
            return "已取消";
        }
        return {};
    }
}
