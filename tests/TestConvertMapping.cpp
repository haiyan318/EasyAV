#include "TestSupport.h"

#include "model/ConvertMapping.h"

#include <string>

using namespace easyav;

namespace
{
    OutputSettings DefaultOutput()
    {
        OutputSettings output;
        output.autoSuffix = true;
        return output;
    }
}

TEST_CASE(Mapping_H264Default)
{
    ConvertSettings settings;
    const OutputSettings output = DefaultOutput();

    CHECK(FormatFor(settings) == TargetFormat::MP4);
    CHECK(CodecFor(settings) == VideoCodec::H264);
    CHECK_NEAR(EffectiveQuality(settings), 0.85, 0.001);
    CHECK_STR(AutoSuffixFor(settings), "_H264");

    const TaskOptions options = BuildTaskOptions(settings, output, "C:/out");
    CHECK(options.format == TargetFormat::MP4);
    CHECK(options.videoCodec == VideoCodec::H264);
    CHECK(!options.slim);
    CHECK_EQ(options.resolutionIndex, 0);
    CHECK_EQ(options.frameRateIndex, 0);
    CHECK(options.useGpuAccel);
    CHECK_STR(options.suffix, "_H264");
    CHECK_STR(options.outputDir, "C:/out");
}

TEST_CASE(Mapping_H265SlimMkv)
{
    ConvertSettings settings;
    settings.target = ConvertTarget::H265;
    settings.container = TargetFormat::MKV;
    settings.slim = true;
    settings.slimQuality = 0.30f;
    settings.resolutionIndex = 2;
    settings.frameRateIndex = 1;

    const TaskOptions options = BuildTaskOptions(settings, DefaultOutput(), {});
    CHECK(options.format == TargetFormat::MKV);
    CHECK(options.videoCodec == VideoCodec::H265);
    CHECK_NEAR(options.quality, 0.30, 0.001);
    CHECK(options.slim);
    CHECK_EQ(options.resolutionIndex, 2);
    CHECK_EQ(options.frameRateIndex, 1);
    CHECK_STR(options.suffix, "_H265");
}

TEST_CASE(Mapping_AudioForcesMp3)
{
    ConvertSettings settings;
    settings.target = ConvertTarget::Audio;
    settings.container = TargetFormat::MKV;
    settings.slim = true;

    const TaskOptions options = BuildTaskOptions(settings, DefaultOutput(), {});
    CHECK(options.format == TargetFormat::MP3);
    CHECK(options.videoCodec == VideoCodec::None);
    CHECK(!options.slim);
    CHECK(!options.useGpuAccel);
    CHECK_EQ(options.resolutionIndex, 0);
    CHECK_STR(options.suffix, "_音频");
}

TEST_CASE(Mapping_ManualSuffixAndNormalization)
{
    ConvertSettings settings;
    settings.container = TargetFormat::GIF;
    OutputSettings output = DefaultOutput();
    output.autoSuffix = false;
    output.suffix[0] = '\0';

    const TaskOptions options = BuildTaskOptions(settings, output, {});
    CHECK(options.format == TargetFormat::MP4);
    CHECK_STR(options.suffix, "");

    output.autoSuffix = true;
    const TaskOptions autoNamed = BuildTaskOptions(settings, output, {});
    CHECK_STR(autoNamed.suffix, "_H264");
}

TEST_CASE(Mapping_EstimateFactors)
{
    ConvertSettings h264;
    ConvertSettings h265 = h264;
    h265.target = ConvertTarget::H265;
    CHECK(EstimateSizeFactor(h265) < EstimateSizeFactor(h264));

    ConvertSettings slim = h264;
    slim.slim = true;
    slim.slimQuality = 0.10f;
    ConvertSettings better = h264;
    better.slim = true;
    better.slimQuality = 0.90f;
    CHECK(EstimateSizeFactor(slim) < EstimateSizeFactor(better));

    ConvertSettings audio = h264;
    audio.target = ConvertTarget::Audio;
    CHECK(EstimateSizeFactor(audio) < 0.10);
}
