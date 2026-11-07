#include "services/Logger.h"

#include "core/Text.h"

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <thread>

namespace easyav::services
{
    namespace
    {
        namespace fs = std::filesystem;

        constexpr std::uintmax_t kMaxLogBytes = 2u * 1024u * 1024u;

        std::mutex g_mutex;
        std::ofstream g_file;

        std::string Timestamp()
        {
            using namespace std::chrono;

            const auto now = system_clock::now();
            const std::time_t seconds = system_clock::to_time_t(now);

            std::tm local{};
#ifdef _WIN32
            localtime_s(&local, &seconds);
#else
            localtime_r(&seconds, &local);
#endif

            const auto millis =
                duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

            char buffer[40] = {};
            std::snprintf(buffer, sizeof(buffer),
                          "[%04d-%02d-%02d %02d:%02d:%02d.%03d]",
                          local.tm_year + 1900, local.tm_mon + 1, local.tm_mday,
                          local.tm_hour, local.tm_min, local.tm_sec,
                          static_cast<int>(millis.count()));
            return buffer;
        }

        void RotateIfNeeded(const fs::path& file)
        {
            std::error_code ec;
            const std::uintmax_t size = fs::file_size(file, ec);
            if (ec || size <= kMaxLogBytes)
                return;

            fs::path backup = file;
            backup += ".1";

            std::error_code removeEc;
            fs::remove(backup, removeEc);

            std::error_code renameEc;
            fs::rename(file, backup, renameEc);
        }

        // 调用方必须持有 g_mutex。
        void WriteLocked(const char* level, const std::string& messageUtf8)
        {
            if (!g_file.is_open())
                return;

            g_file << Timestamp() << '[' << level << "]["
                   << std::this_thread::get_id() << "] " << messageUtf8 << '\n';
            g_file.flush();
        }
    }

    void Logger::Init(const std::string& baseDirUtf8)
    {
        std::lock_guard<std::mutex> lock(g_mutex);

        if (g_file.is_open())
            g_file.close();

        if (baseDirUtf8.empty())
            return;

        const fs::path logDir = PathFromUtf8(baseDirUtf8) / L"logs";
        std::error_code ec;
        fs::create_directories(logDir, ec);
        if (ec)
            return;

        const fs::path file = logDir / L"easyav.log";
        RotateIfNeeded(file);

        g_file.open(file, std::ios::out | std::ios::app | std::ios::binary);
        if (!g_file.is_open())
            return;

        WriteLocked("INFO", "日志已启动");
    }

    void Logger::Shutdown()
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        if (g_file.is_open())
        {
            g_file.flush();
            g_file.close();
        }
    }

    void Logger::Info(const std::string& messageUtf8)
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        WriteLocked("INFO", messageUtf8);
    }

    void Logger::Warn(const std::string& messageUtf8)
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        WriteLocked("WARN", messageUtf8);
    }

    void Logger::Error(const std::string& messageUtf8)
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        WriteLocked("ERROR", messageUtf8);
    }

    void Logger::Command(const std::wstring& exePath, const std::vector<std::string>& argsUtf8)
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        if (!g_file.is_open())
            return;

        WriteLocked("INFO", "命令: " + WideToUtf8(exePath));
        for (std::size_t i = 0; i < argsUtf8.size(); ++i)
        {
            char prefix[32] = {};
            std::snprintf(prefix, sizeof(prefix), "  arg[%zu] = ", i);
            WriteLocked("INFO", std::string(prefix) + argsUtf8[i]);
        }
    }
}
