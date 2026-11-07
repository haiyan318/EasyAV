#pragma once

// 失败分类与中文文案（纯函数）。
// 技术细节只进日志，界面只展示 UserMessage。

#include "core/TaskContracts.h"

#include <cstddef>
#include <string>

namespace easyav::services
{
    FailureKind ClassifyFailure(int exitCode, const std::string& stderrText, bool outputEmpty);

    // fileNameUtf8 非空时追加“（文件名）”。
    // codec 用于区分“H.265 组件缺失”的提示；hardwareAttempt 表示失败发生在显卡尝试。
    std::string UserMessage(FailureKind kind, const std::string& fileNameUtf8 = {},
                            VideoCodec codec = VideoCodec::H264,
                            bool hardwareAttempt = false);

    // 截取 stderr 末尾 maxChars 个字符，用于日志。
    std::string ShortDetail(const std::string& stderrText, std::size_t maxChars = 2000);
}
