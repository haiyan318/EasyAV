#include "model/TaskController.h"

#include <algorithm>

namespace easyav
{
    void TaskController::AttachRunner(ITaskRunner* runner)
    {
        m_runner = runner;
    }

    bool TaskController::Start(const TaskRequest& request)
    {
        m_startError.clear();

        if (m_runner == nullptr)
        {
            m_startError = "处理组件还没有准备好，请稍后重试";
            return false;
        }

        if (m_phase == TaskPhase::Running)
        {
            m_startError = "正在处理，请等当前任务结束";
            return false;
        }

        if (!m_runner->Start(request))
        {
            m_startError = m_runner->StartErrorText();
            return false;
        }

        m_names.clear();
        m_names.reserve(request.items.size());
        for (const TaskItem& item : request.items)
            m_names.push_back(item.displayName);

        m_fileCount = static_cast<int>(request.items.size());
        m_phase = TaskPhase::Running;
        m_progress = 0.0f;
        m_elapsedSec = 0.0;
        m_succeeded = 0;
        m_failed = 0;
        m_currentIndex = 0;
        m_currentFileProgress = 0.0f;
        m_indeterminate = false;
        m_completedEvent = false;
        m_cancelRequested = false;
        m_failures.clear();
        m_lastOutput.clear();
        m_statusText = "正在准备…";
        return true;
    }

    void TaskController::Cancel()
    {
        if (m_phase != TaskPhase::Running)
            return;

        m_cancelRequested = true;
        if (m_runner != nullptr)
            m_runner->Cancel();
    }

    void TaskController::Reset()
    {
        m_phase = TaskPhase::Idle;
        m_names.clear();
        m_failures.clear();
        m_statusText = "准备就绪";
        m_startError.clear();
        m_lastOutput.clear();
        m_succeeded = 0;
        m_failed = 0;
        m_fileCount = 0;
        m_currentIndex = 0;
        m_progress = 0.0f;
        m_currentFileProgress = 0.0f;
        m_elapsedSec = 0.0;
        m_indeterminate = false;
        m_completedEvent = false;
        m_cancelRequested = false;
    }

    void TaskController::Tick(float deltaSeconds)
    {
        if (m_phase != TaskPhase::Running)
            return;

        ConsumeRunnerEvents();
        m_elapsedSec += static_cast<double>(deltaSeconds);
    }

    void TaskController::ConsumeRunnerEvents()
    {
        if (m_runner == nullptr)
            return;

        std::vector<TaskEvent> events;
        m_runner->DrainTaskEvents(events);
        for (const TaskEvent& event : events)
            ApplyEvent(event);
    }

    void TaskController::ApplyEvent(const TaskEvent& event)
    {
        switch (event.kind)
        {
        case TaskEventKind::TaskStarted:
            m_fileCount = event.fileCount;
            m_progress = 0.0f;
            m_indeterminate = false;
            break;

        case TaskEventKind::FileStarted:
            m_currentIndex = std::max(event.fileIndex, 0);
            m_currentFileProgress = 0.0f;
            m_indeterminate = false;
            break;

        case TaskEventKind::FileProgress:
            m_progress = event.overallProgress;
            m_currentFileProgress = event.fileProgress;
            m_indeterminate = event.indeterminate;
            break;

        case TaskEventKind::FileSucceeded:
            ++m_succeeded;
            m_lastOutput = event.outputPath;
            m_progress = event.overallProgress;
            m_currentFileProgress = 1.0f;
            break;

        case TaskEventKind::FileFailed:
            ++m_failed;
            m_failures.push_back(event.failure);
            break;

        case TaskEventKind::TaskFinished:
            m_phase = TaskPhase::Completed;
            m_progress = 1.0f;
            m_indeterminate = false;
            m_completedEvent = true;
            break;

        case TaskEventKind::TaskCanceled:
            m_phase = TaskPhase::Canceled;
            m_indeterminate = false;
            break;
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
        if (m_phase != TaskPhase::Running || m_indeterminate || m_progress <= 0.02f)
            return -1.0;

        return m_elapsedSec * (1.0 - static_cast<double>(m_progress))
               / static_cast<double>(m_progress);
    }

    int TaskController::CurrentFileIndex() const
    {
        if (m_fileCount <= 0)
            return 0;
        return std::clamp(m_currentIndex, 0, m_fileCount - 1);
    }

    std::string TaskController::CurrentFileName() const
    {
        if (m_names.empty())
            return {};
        const std::size_t index =
            static_cast<std::size_t>(CurrentFileIndex());
        return m_names[std::min(index, m_names.size() - 1)];
    }

    std::string TaskController::StatusText() const
    {
        switch (m_phase)
        {
        case TaskPhase::Idle:
            return "准备就绪";

        case TaskPhase::Running:
            if (m_cancelRequested)
                return "正在取消…";
            if (!m_names.empty())
                return "正在转换：" + CurrentFileName();
            return "正在转换…";

        case TaskPhase::Completed:
            if (m_failed > 0)
            {
                return "完成 " + std::to_string(m_succeeded) + " 个，"
                       + std::to_string(m_failed) + " 个失败";
            }
            return "全部完成";

        case TaskPhase::Canceled:
            return "已取消";
        }
        return {};
    }

    const ToolStatus& TaskController::Tools() const
    {
        static const ToolStatus kNoTools;
        return m_runner != nullptr ? m_runner->Tools() : kNoTools;
    }

    std::vector<FileFailure> TaskController::ConsumeFailures()
    {
        std::vector<FileFailure> failures;
        failures.swap(m_failures);
        return failures;
    }
}
