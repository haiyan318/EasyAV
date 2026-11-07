#pragma once

// 后台任务服务：唯一 worker 线程，串行处理探测与任务。
// UI 线程只做入队/取事件；子进程、文件写入全部在 worker 上。
//
// 线程模型：
// - Start/Cancel/Enqueue/ Drain 加锁交换数据，不阻塞
// - worker 按“任务优先、探测其次”取命令
// - 任务逐文件执行：探测 -> 命名 -> 试写 -> ffmpeg -> 进度 -> 临时文件 -> 原子重命名

#include "core/ITaskRunner.h"
#include "services/FfmpegLocator.h"

#include <atomic>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace easyav::services
{
    class IMediaProbe;

    class TaskService final : public ITaskRunner
    {
    public:
        TaskService();
        ~TaskService() override;

        TaskService(const TaskService&) = delete;
        TaskService& operator=(const TaskService&) = delete;

        // 启动 worker 并定位工具；工具缺失不算失败，结果体现在 Tools()。
        bool Initialize(const std::string& exeDirUtf8, const std::string& cwdUtf8);
        void Shutdown();

        bool Start(const TaskRequest& request) override;
        void Cancel() override;
        void DrainTaskEvents(std::vector<TaskEvent>& out) override;

        void EnqueueProbe(const std::vector<std::string>& utf8Paths) override;
        void DrainProbeResults(std::vector<ProbeResult>& out) override;

        const ToolStatus& Tools() const override { return m_tools; }
        const std::string& StartErrorText() const override { return m_startError; }

    private:
        void WorkerLoop();
        void RunTask(const TaskRequest& request);
        bool ProcessFile(int index, int count, const TaskRequest& request, bool& canceled);
        MediaInfo ProbeForTask(const TaskItem& item);
        void PushEvent(TaskEvent event);
        void SetActiveTemp(const std::string& utf8Path);
        void DeleteActiveTemp();

        LocatedTools m_located;
        ToolStatus m_tools;
        std::unique_ptr<IMediaProbe> m_probe;

        std::thread m_worker;
        std::mutex m_mutex;
        std::condition_variable m_wake;

        std::deque<TaskRequest> m_taskQueue;
        std::deque<std::string> m_probeQueue;
        std::unordered_set<std::string> m_pending;
        std::unordered_map<std::string, MediaInfo> m_cache;

        std::vector<TaskEvent> m_taskEvents;
        std::vector<ProbeResult> m_probeResults;

        std::atomic<bool> m_stop{false};
        std::atomic<bool> m_cancel{false};
        bool m_initialized = false;
        bool m_taskBusy = false;
        unsigned m_tempSerial = 1;
        std::string m_activeTemp;
        std::string m_startError;
    };
}
