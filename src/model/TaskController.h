#pragma once

// 任务状态模型：只消费后台事件，不执行任何阻塞操作。
// 面板通过这里读取进度/状态；真正的处理由 services::TaskService 完成。

#include "core/ITaskRunner.h"

#include <string>
#include <vector>

namespace easyav
{
    class TaskController
    {
    public:
        // 由 App 在启动时装配；可为空（未接入服务时保持空闲）。
        void AttachRunner(ITaskRunner* runner);

        bool Start(const TaskRequest& request);
        void Cancel();
        void Reset();
        void Tick(float deltaSeconds);

        TaskPhase Phase() const { return m_phase; }
        bool IsRunning() const { return m_phase == TaskPhase::Running; }
        bool HasFinished() const { return m_phase == TaskPhase::Completed; }
        bool Indeterminate() const { return m_indeterminate; }

        // 完成事件只上报一次，供 App 弹提示 / 打开输出目录。
        bool ConsumeCompleted();

        float Progress() const { return m_progress; }
        double ElapsedSeconds() const { return m_elapsedSec; }
        double EtaSeconds() const;
        int FileCount() const { return m_fileCount; }
        int CurrentFileIndex() const;
        float CurrentFileProgress() const { return m_currentFileProgress; }
        std::string CurrentFileName() const;
        std::string StatusText() const;

        const ToolStatus& Tools() const;
        int SucceededCount() const { return m_succeeded; }
        int FailedCount() const { return m_failed; }
        const std::string& StartErrorText() const { return m_startError; }
        const std::string& LastOutputPath() const { return m_lastOutput; }

        // 一次性取走失败列表（用于弹提示）；计数仍保留在 FailedCount。
        std::vector<FileFailure> ConsumeFailures();

    private:
        void ConsumeRunnerEvents();
        void ApplyEvent(const TaskEvent& event);

        ITaskRunner* m_runner = nullptr;

        TaskPhase m_phase = TaskPhase::Idle;
        std::vector<std::string> m_names;
        std::vector<FileFailure> m_failures;

        std::string m_statusText = "准备就绪";
        std::string m_startError;
        std::string m_lastOutput;

        int m_succeeded = 0;
        int m_failed = 0;
        int m_fileCount = 0;
        int m_currentIndex = 0;
        float m_progress = 0.0f;
        float m_currentFileProgress = 0.0f;
        double m_elapsedSec = 0.0;
        bool m_indeterminate = false;
        bool m_completedEvent = false;
        bool m_cancelRequested = false;
    };
}
