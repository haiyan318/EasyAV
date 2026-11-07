#include "services/FfmpegProcess.h"

#include "core/Text.h"
#include "services/CommandLine.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <cstddef>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace easyav::services
{
    namespace
    {
        HANDLE ToHandle(void* value)
        {
            return static_cast<HANDLE>(value);
        }

        void* FromHandle(HANDLE value)
        {
            return static_cast<void*>(value);
        }

        void CloseIfValid(HANDLE& handle)
        {
            if (handle != nullptr && handle != INVALID_HANDLE_VALUE)
                CloseHandle(handle);
            handle = nullptr;
        }

        std::string LastErrorText(DWORD code)
        {
            std::string text = "错误码 " + std::to_string(code);
            LPWSTR buffer = nullptr;
            const DWORD length = FormatMessageW(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
                    | FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                reinterpret_cast<LPWSTR>(&buffer), 0, nullptr);
            if (length != 0 && buffer != nullptr)
            {
                text += "（" + WideToUtf8(std::wstring(buffer, length)) + "）";
                LocalFree(buffer);
            }
            return text;
        }

        void AppendTail(std::string& tail, const char* data, std::size_t size)
        {
            constexpr std::size_t kMaxBytes = 64u * 1024u;
            tail.append(data, size);
            if (tail.size() > kMaxBytes)
                tail.erase(0, tail.size() - kMaxBytes);
        }
    }

    FfmpegProcess::~FfmpegProcess()
    {
        HANDLE process = ToHandle(m_process);
        if (process != nullptr && !m_waited)
        {
            TerminateProcess(process, 1);
            WaitForSingleObject(process, 1000);
        }
        CloseHandles();
    }

    void FfmpegProcess::CloseHandles()
    {
        HANDLE process = ToHandle(m_process);
        if (process != nullptr)
            CloseHandle(process);
        m_process = nullptr;

        HANDLE job = ToHandle(m_job);
        if (job != nullptr)
            CloseHandle(job);
        m_job = nullptr;

        HANDLE outRead = ToHandle(m_stdoutRead);
        if (outRead != nullptr)
            CloseHandle(outRead);
        m_stdoutRead = nullptr;

        HANDLE errRead = ToHandle(m_stderrRead);
        if (errRead != nullptr)
            CloseHandle(errRead);
        m_stderrRead = nullptr;
    }

    bool FfmpegProcess::Start(const std::wstring& exePath,
                              const std::vector<std::string>& argsUtf8,
                              std::string* errorText)
    {
        CloseHandles();
        m_waited = false;
        if (errorText)
            errorText->clear();

        if (exePath.empty())
        {
            if (errorText)
                *errorText = "找不到处理组件，请重新安装 EasyAV";
            return false;
        }

        const std::wstring commandLine = BuildCommandLine(exePath, argsUtf8);
        if (!CommandLineFits(commandLine))
        {
            if (errorText)
                *errorText = "要处理的文件路径太长，处理组件无法启动";
            return false;
        }

        SECURITY_ATTRIBUTES sa{};
        sa.nLength = sizeof(sa);
        sa.bInheritHandle = TRUE;
        sa.lpSecurityDescriptor = nullptr;

        HANDLE outRead = nullptr;
        HANDLE outWrite = nullptr;
        HANDLE errRead = nullptr;
        HANDLE errWrite = nullptr;
        HANDLE nul = nullptr;
        HANDLE job = nullptr;

        PROCESS_INFORMATION pi{};
        STARTUPINFOEXW si{};
        bool attrInitialized = false;
        bool ok = false;
        std::string failure;

        do
        {
            if (!CreatePipe(&outRead, &outWrite, &sa, 0)
                || !CreatePipe(&errRead, &errWrite, &sa, 0))
            {
                failure = "无法创建输出管道：" + LastErrorText(GetLastError());
                break;
            }

            // 读端不能被子进程继承，否则管道永远不会收到 EOF。
            SetHandleInformation(outRead, HANDLE_FLAG_INHERIT, 0);
            SetHandleInformation(errRead, HANDLE_FLAG_INHERIT, 0);

            nul = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                              &sa, OPEN_EXISTING, 0, nullptr);
            if (nul == INVALID_HANDLE_VALUE)
            {
                nul = nullptr;
                failure = "无法准备标准输入：" + LastErrorText(GetLastError());
                break;
            }

            si.StartupInfo.cb = sizeof(STARTUPINFOEXW);
            si.StartupInfo.dwFlags = STARTF_USESTDHANDLES;
            si.StartupInfo.hStdInput = nul;
            si.StartupInfo.hStdOutput = outWrite;
            si.StartupInfo.hStdError = errWrite;

            HANDLE inherited[] = {nul, outWrite, errWrite};

            SIZE_T attributeSize = 0;
            InitializeProcThreadAttributeList(nullptr, 1, 0, &attributeSize);
            if (attributeSize == 0)
            {
                failure = "无法初始化进程属性：" + LastErrorText(GetLastError());
                break;
            }

            std::vector<unsigned char> attributeBuffer(attributeSize);
            si.lpAttributeList =
                reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(attributeBuffer.data());
            if (!InitializeProcThreadAttributeList(si.lpAttributeList, 1, 0, &attributeSize))
            {
                failure = "无法初始化进程属性：" + LastErrorText(GetLastError());
                break;
            }
            attrInitialized = true;

            if (!UpdateProcThreadAttribute(
                    si.lpAttributeList, 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST,
                    inherited, sizeof(inherited), nullptr, nullptr))
            {
                failure = "无法设置句柄继承：" + LastErrorText(GetLastError());
                break;
            }

            std::vector<wchar_t> commandBuffer(commandLine.begin(), commandLine.end());
            commandBuffer.push_back(L'\0');

            if (!CreateProcessW(
                    exePath.c_str(), commandBuffer.data(), nullptr, nullptr, TRUE,
                    CREATE_NO_WINDOW | CREATE_SUSPENDED | EXTENDED_STARTUPINFO_PRESENT,
                    nullptr, nullptr, reinterpret_cast<LPSTARTUPINFOW>(&si), &pi))
            {
                failure = "无法启动处理组件：" + LastErrorText(GetLastError());
                break;
            }

            // 先加入 Job Object 再恢复执行，保证取消时整棵进程树被回收。
            job = CreateJobObjectW(nullptr, nullptr);
            if (job != nullptr)
            {
                JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};
                limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
                SetInformationJobObject(job, JobObjectExtendedLimitInformation,
                                        &limits, sizeof(limits));
                AssignProcessToJobObject(job, pi.hProcess);
            }

            if (ResumeThread(pi.hThread) == static_cast<DWORD>(-1))
            {
                failure = "无法启动处理组件线程：" + LastErrorText(GetLastError());
                TerminateProcess(pi.hProcess, 1);
                break;
            }

            m_process = FromHandle(pi.hProcess);
            pi.hProcess = nullptr;
            m_job = FromHandle(job);
            job = nullptr;
            m_stdoutRead = FromHandle(outRead);
            outRead = nullptr;
            m_stderrRead = FromHandle(errRead);
            errRead = nullptr;
            ok = true;
        } while (false);

        if (attrInitialized && si.lpAttributeList != nullptr)
            DeleteProcThreadAttributeList(si.lpAttributeList);

        if (pi.hThread != nullptr)
            CloseHandle(pi.hThread);
        CloseIfValid(nul);
        CloseIfValid(outWrite);
        CloseIfValid(errWrite);

        if (!ok)
        {
            CloseIfValid(outRead);
            CloseIfValid(errRead);
            CloseIfValid(pi.hProcess);
            CloseIfValid(job);
            if (errorText)
                *errorText = failure;
        }
        else
        {
            // 父进程必须关闭写端，否则读线程等不到 EOF。
            if (errorText)
                errorText->clear();
        }

        return ok;
    }

    FfmpegProcess::Result FfmpegProcess::Wait(
        const std::atomic<bool>* cancelFlag,
        const std::function<void(const char*, std::size_t)>& onStdout,
        const std::function<void(const char*, std::size_t)>& onStderr,
        int timeoutMs)
    {
        Result result;
        HANDLE process = ToHandle(m_process);
        if (process == nullptr)
            return result;

        result.started = true;

        const HANDLE outRead = ToHandle(m_stdoutRead);
        const HANDLE errRead = ToHandle(m_stderrRead);
        std::string stderrTail;
        stderrTail.reserve(64u * 1024u);

        std::thread outReader([outRead, &onStdout]() {
            char buffer[4096];
            DWORD read = 0;
            while (ReadFile(outRead, buffer, static_cast<DWORD>(sizeof(buffer)),
                            &read, nullptr)
                   && read > 0)
            {
                if (onStdout)
                    onStdout(buffer, static_cast<std::size_t>(read));
            }
        });

        std::thread errReader([errRead, &onStderr, &stderrTail]() {
            char buffer[4096];
            DWORD read = 0;
            while (ReadFile(errRead, buffer, static_cast<DWORD>(sizeof(buffer)),
                            &read, nullptr)
                   && read > 0)
            {
                if (onStderr)
                    onStderr(buffer, static_cast<std::size_t>(read));
                AppendTail(stderrTail, buffer, static_cast<std::size_t>(read));
            }
        });

        const ULONGLONG startMs = GetTickCount64();
        bool canceled = false;
        bool timedOut = false;

        for (;;)
        {
            const DWORD wait = WaitForSingleObject(process, 100);
            if (wait == WAIT_OBJECT_0)
                break;

            if (cancelFlag != nullptr && cancelFlag->load())
            {
                canceled = true;
                break;
            }

            if (timeoutMs > 0
                && GetTickCount64() - startMs >= static_cast<ULONGLONG>(timeoutMs))
            {
                timedOut = true;
                break;
            }
        }

        if (canceled || timedOut)
        {
            TerminateProcess(process, 1);
            WaitForSingleObject(process, 2000);

            HANDLE job = ToHandle(m_job);
            if (job != nullptr)
                TerminateJobObject(job, 1);
        }

        outReader.join();
        errReader.join();

        DWORD exitCode = 1;
        if (!GetExitCodeProcess(process, &exitCode))
            exitCode = 1;

        result.exitCode = static_cast<int>(exitCode);
        result.canceled = canceled;
        result.timedOut = timedOut;
        result.stderrTail = std::move(stderrTail);

        CloseHandles();
        m_waited = true;
        return result;
    }

    bool FfmpegProcess::Capture(const std::wstring& exePath,
                                const std::vector<std::string>& argsUtf8,
                                int timeoutMs,
                                std::string* stdoutUtf8,
                                std::string* stderrUtf8,
                                int* exitCode,
                                std::string* errorText)
    {
        if (stdoutUtf8)
            stdoutUtf8->clear();
        if (stderrUtf8)
            stderrUtf8->clear();
        if (exitCode)
            *exitCode = -1;
        if (errorText)
            errorText->clear();

        FfmpegProcess process;
        std::string startError;
        if (!process.Start(exePath, argsUtf8, &startError))
        {
            if (errorText)
                *errorText = startError;
            return false;
        }

        std::string stdoutText;
        std::string stderrText;
        const Result result = process.Wait(
            nullptr,
            [&stdoutText](const char* data, std::size_t size) {
                stdoutText.append(data, size);
            },
            [&stderrText](const char* data, std::size_t size) {
                stderrText.append(data, size);
            },
            timeoutMs);

        if (stdoutUtf8)
            *stdoutUtf8 = std::move(stdoutText);
        if (stderrUtf8)
            *stderrUtf8 = stderrText.empty() ? result.stderrTail : stderrText;
        if (exitCode)
            *exitCode = result.exitCode;

        if (result.timedOut)
        {
            if (errorText)
                *errorText = "处理组件响应超时，请重试";
            return false;
        }

        return result.started && !result.canceled;
    }
}
