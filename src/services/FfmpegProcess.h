#pragma once

// 子进程封装：CreateProcessW + 隐藏窗口 + 匿名管道 + Job Object。
// Start/Wait 只允许在 worker 线程调用；Capture 是同步一次性调用（启动探测/版本校验）。
//
// 约定：
// - 命令行走 services/CommandLine 的参数数组，不接受裸 shell 字符串。
// - Wait 会持续排空 stdout/stderr，避免管道写满导致子进程卡死。
// - onStdout / onStderr 分别在各自的读线程调用，可能并发，回调不得抛出异常。

#include <atomic>
#include <functional>
#include <string>
#include <vector>

namespace easyav::services
{
    class FfmpegProcess
    {
    public:
        struct Result
        {
            bool started = false;
            bool canceled = false;
            bool timedOut = false;
            int exitCode = -1;
            std::string stderrTail; // 最近约 64KB
        };

        FfmpegProcess() = default;
        ~FfmpegProcess();

        FfmpegProcess(const FfmpegProcess&) = delete;
        FfmpegProcess& operator=(const FfmpegProcess&) = delete;

        bool Start(const std::wstring& exePath,
                   const std::vector<std::string>& argsUtf8,
                   std::string* errorText);

        Result Wait(const std::atomic<bool>* cancelFlag,
                    const std::function<void(const char*, std::size_t)>& onStdout,
                    const std::function<void(const char*, std::size_t)>& onStderr,
                    int timeoutMs = 0);

        // 启动并等待结束，收集输出；用于 ffmpeg -version / ffprobe。
        static bool Capture(const std::wstring& exePath,
                            const std::vector<std::string>& argsUtf8,
                            int timeoutMs,
                            std::string* stdoutUtf8,
                            std::string* stderrUtf8,
                            int* exitCode,
                            std::string* errorText);

    private:
        void CloseHandles();

        // 避免在头文件引入 windows.h。
        void* m_process = nullptr;
        void* m_job = nullptr;
        void* m_stdoutRead = nullptr;
        void* m_stderrRead = nullptr;
        bool m_waited = false;
    };
}
