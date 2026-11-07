#pragma once

// 任务执行端口：model 层只依赖本接口，services::TaskService 负责实现。
// 所有方法都在 UI 线程调用，实现内部不得执行阻塞操作（只加锁入队/取出事件）。

#include "core/TaskContracts.h"

#include <string>
#include <vector>

namespace easyav
{
    class ITaskRunner
    {
    public:
        virtual ~ITaskRunner() = default;

        // 返回 false 时，原因由 StartErrorText() 提供。
        virtual bool Start(const TaskRequest& request) = 0;
        virtual void Cancel() = 0;

        virtual void DrainTaskEvents(std::vector<TaskEvent>& out) = 0;
        virtual void EnqueueProbe(const std::vector<std::string>& utf8Paths) = 0;
        virtual void DrainProbeResults(std::vector<ProbeResult>& out) = 0;

        virtual const ToolStatus& Tools() const = 0;
        virtual const std::string& StartErrorText() const = 0;
    };
}
