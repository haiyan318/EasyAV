#pragma once

// 应用外壳：负责把模型、面板、平台能力装配起来。
// 这里不包含 GLFW/window 逻辑，main.cpp 只负责创建窗口与 ImGui 后端。

#include "model/AppModel.h"
#include "services/TaskService.h"

#include <string>
#include <vector>

namespace easyav
{
    class App
    {
    public:
        void Init();
        void Frame();
        // 进程退出前调用：后续在此取消后台任务、写退出日志。
        void Shutdown();
        void AddDroppedPaths(const std::vector<std::string>& utf8Paths);

        AppModel& Model() { return m_model; }
        const AppModel& Model() const { return m_model; }

    private:
        void ProcessRequests();
        // 把当前队列中的文件交给后台探测（服务内部会去重）。
        void RequestProbes();

        AppModel m_model;
        services::TaskService m_services;
    };
}
