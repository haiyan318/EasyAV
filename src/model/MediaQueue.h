#pragma once

// 队列模型：只负责“有哪些文件”，不涉及绘制、线程和编码。
// 所有成员均在 UI 线程访问；后续接入后台任务时再增加同步策略。

#include "core/Types.h"

#include <vector>

namespace easyav
{
    class MediaQueue
    {
    public:
        // 返回 1 表示添加成功，0 表示重复 / 不支持 / 不存在。
        int AddFile(const std::string& utf8Path);

        // 递归扫描文件夹，返回成功数量，并累加被跳过的数量。
        int AddDirectory(const std::string& utf8Dir, int* skipped);

        void RemoveAt(int index);
        void Clear();

        bool Empty() const { return m_items.empty(); }
        int Count() const { return static_cast<int>(m_items.size()); }
        const std::vector<MediaItem>& Items() const { return m_items; }
        unsigned long long TotalBytes() const;

    private:
        bool ContainsPath(const std::string& normalizedPath) const;

        std::vector<MediaItem> m_items;
        int m_nextId = 1;
    };
}
