#pragma once

#include "services/IMediaProbe.h"

#include <string>

namespace easyav::services
{
    class FfprobeProbe final : public IMediaProbe
    {
    public:
        explicit FfprobeProbe(std::wstring ffprobeExePath);

        MediaInfo Probe(const std::string& utf8Path) override;

    private:
        std::wstring m_exePath;
    };
}
