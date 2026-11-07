#pragma once

// 应用外壳：负责把模型、面板、平台能力装配起来。
// 这里不包含 GLFW/window 逻辑，main.cpp 只负责创建窗口与 ImGui 后端。

#include "model/AppModel.h"

#include <string>
#include <vector>

namespace easyav
{
    class App
    {
    public:
        void Init();
        void Frame();
        void AddDroppedPaths(const std::vector<std::string>& utf8Paths);

        AppModel& Model() { return m_model; }
        const AppModel& Model() const { return m_model; }

    private:
        void ProcessRequests();

        AppModel m_model;
    };
}
