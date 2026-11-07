#pragma once

// “用户意图 → 任务参数”的纯映射，便于单元测试。
// 只依赖 core 与 TaskSettings，不引用 ImGui / services。

#include "core/TaskContracts.h"
#include "model/TaskSettings.h"

#include <string>

namespace easyav
{
    TargetFormat FormatFor(const ConvertSettings& settings);
    VideoCodec CodecFor(const ConvertSettings& settings);
    float EffectiveQuality(const ConvertSettings& settings);
    std::string AutoSuffixFor(const ConvertSettings& settings);
    double EstimateSizeFactor(const ConvertSettings& settings);

    TaskOptions BuildTaskOptions(const ConvertSettings& settings,
                                 const OutputSettings& output,
                                 const std::string& resolvedOutputDir);
}
