#include "TestSupport.h"

#include "services/FfmpegCommandBuilder.h"

#include <algorithm>
#include <string>
#include <vector>

using easyav::TargetFormat;
using easyav::VideoCodec;
using easyav::VideoEncoder;
namespace builder = easyav::services;

namespace
{
    bool Has(const std::vector<std::string>& args, const std::string& value)
    {
        return std::find(args.begin(), args.end(), value) != args.end();
    }

    bool HasPair(const std::vector<std::string>& args, const std::string& key,
                 const std::string& value)
    {
        for (std::size_t i = 0; i + 1 < args.size(); ++i)
        {
            if (args[i] == key && args[i + 1] == value)
                return true;
        }
        return false;
    }

    builder::BuildRequest MakeRequest(TargetFormat format = TargetFormat::MP4)
    {
        builder::BuildRequest request;
        request.inputPath = "C:/素材/a b(纯).mp4";
        request.outputPath = "C:/out/a b(纯)_压缩.mp4";
        request.format = format;
        request.quality = 0.55f;
        return request;
    }
}

TEST_CASE(Builder_Mp4Default)
{
    const builder::BuildResult result = builder::BuildFfmpegArgs(MakeRequest());
    CHECK(result.ok);
    if (!result.ok)
        return;

    const std::vector<std::string>& args = result.args;
    CHECK(!args.empty());
    CHECK_STR(args.front(), "-hide_banner");
    CHECK_STR(args.back(), "C:/out/a b(纯)_压缩.mp4");
    CHECK(HasPair(args, "-progress", "pipe:1"));
    CHECK(HasPair(args, "-i", "C:/素材/a b(纯).mp4"));
    CHECK(HasPair(args, "-c:v", "libx264"));
    CHECK(HasPair(args, "-preset", "fast"));
    CHECK(HasPair(args, "-crf", "20"));
    CHECK(HasPair(args, "-c:a", "aac"));
    CHECK(HasPair(args, "-b:a", "128k"));
    CHECK(HasPair(args, "-movflags", "+faststart"));
    CHECK(HasPair(args, "-f", "mp4"));
}

TEST_CASE(Builder_QualityMapping)
{
    CHECK_EQ(builder::CrfForQuality(0.19f), 30);
    CHECK_EQ(builder::CrfForQuality(0.20f), 28);
    CHECK_EQ(builder::CrfForQuality(0.40f), 25);
    CHECK_EQ(builder::CrfForQuality(0.60f), 22);
    CHECK_EQ(builder::CrfForQuality(0.80f), 20);
    CHECK_EQ(builder::CrfForQuality(-1.0f), 30);
    CHECK_EQ(builder::CrfForQuality(2.0f), 20);

    CHECK_STR(builder::PresetForQuality(0.10f), "veryfast");
    CHECK_STR(builder::PresetForQuality(0.50f), "medium");
    CHECK_STR(builder::PresetForQuality(0.70f), "slow");
    CHECK_STR(builder::PresetForQuality(0.90f), "slower");

    CHECK_EQ(builder::AudioBitrateKbps(0.10f), 96);
    CHECK_EQ(builder::AudioBitrateKbps(0.90f), 192);
    CHECK_EQ(builder::Mp3BitrateKbps(0.10f), 96);
    CHECK_EQ(builder::Mp3BitrateKbps(0.90f), 256);
}

TEST_CASE(Builder_MkvContainer)
{
    const builder::BuildResult result =
        builder::BuildFfmpegArgs(MakeRequest(TargetFormat::MKV));
    CHECK(result.ok);
    CHECK(HasPair(result.args, "-f", "matroska"));
    CHECK(!Has(result.args, "-movflags"));
}

TEST_CASE(Builder_Mp3Extraction)
{
    builder::BuildRequest request = MakeRequest(TargetFormat::MP3);
    request.outputPath = "C:/out/a.mp3";

    const builder::BuildResult result = builder::BuildFfmpegArgs(request);
    CHECK(result.ok);
    CHECK(Has(result.args, "-vn"));
    CHECK(HasPair(result.args, "-c:a", "libmp3lame"));
    CHECK(HasPair(result.args, "-b:a", "160k"));
    CHECK(HasPair(result.args, "-f", "mp3"));
    CHECK(!Has(result.args, "-vf"));
    CHECK(!Has(result.args, "-r"));
    CHECK(HasPair(result.args, "-map", "0:a:0"));
    CHECK(!HasPair(result.args, "-map", "0:v:0?"));
}

TEST_CASE(Builder_ResolutionAndFrameRate)
{
    builder::BuildRequest request = MakeRequest();
    request.resolutionIndex = 2;
    request.frameRateIndex = 1;

    const builder::BuildResult result = builder::BuildFfmpegArgs(request);
    CHECK(result.ok);
    CHECK(HasPair(result.args, "-vf", "scale=-2:720"));
    CHECK(HasPair(result.args, "-r", "30"));

    builder::BuildRequest original = MakeRequest();
    const builder::BuildResult plain = builder::BuildFfmpegArgs(original);
    CHECK(!Has(plain.args, "-vf"));
    CHECK(!Has(plain.args, "-r"));
}

TEST_CASE(Builder_GpuEncoders)
{
    builder::BuildRequest request = MakeRequest();
    request.encoder = VideoEncoder::Nvenc;
    builder::BuildResult result = builder::BuildFfmpegArgs(request);
    CHECK(result.ok);
    CHECK(HasPair(result.args, "-c:v", "h264_nvenc"));
    CHECK(HasPair(result.args, "-cq", "27"));
    CHECK(!Has(result.args, "-crf"));

    request.encoder = VideoEncoder::Qsv;
    result = builder::BuildFfmpegArgs(request);
    CHECK(result.ok);
    CHECK(HasPair(result.args, "-c:v", "h264_qsv"));
    CHECK(HasPair(result.args, "-global_quality", "27"));
    CHECK(!Has(result.args, "-crf"));

    request.encoder = VideoEncoder::Amf;
    result = builder::BuildFfmpegArgs(request);
    CHECK(result.ok);
    CHECK(HasPair(result.args, "-c:v", "h264_amf"));
    CHECK(HasPair(result.args, "-qp_i", "27"));
    CHECK(!Has(result.args, "-crf"));
}

TEST_CASE(Builder_RejectsUnsupportedCombinations)
{
    builder::BuildRequest audioOnly = MakeRequest(TargetFormat::MP4);
    audioOnly.hasVideo = false;
    builder::BuildResult result = builder::BuildFfmpegArgs(audioOnly);
    CHECK(!result.ok);
    CHECK(result.errorText.find("只要声音") != std::string::npos);

    builder::BuildRequest gif = MakeRequest(TargetFormat::GIF);
    result = builder::BuildFfmpegArgs(gif);
    CHECK(!result.ok);
    CHECK(result.errorText.find("GIF") != std::string::npos);

    builder::BuildRequest noAudio = MakeRequest(TargetFormat::MP3);
    noAudio.hasAudio = false;
    result = builder::BuildFfmpegArgs(noAudio);
    CHECK(!result.ok);
    CHECK(!result.errorText.empty());

    builder::BuildRequest empty = MakeRequest();
    empty.inputPath.clear();
    CHECK(!builder::BuildFfmpegArgs(empty).ok);
}

TEST_CASE(Builder_NoAudioAddsAn)
{
    builder::BuildRequest silent = MakeRequest();
    silent.hasAudio = false;
    const builder::BuildResult result = builder::BuildFfmpegArgs(silent);
    CHECK(result.ok);
    CHECK(Has(result.args, "-an"));
    CHECK(!Has(result.args, "-c:a"));
}

TEST_CASE(Builder_H265Mp4)
{
    builder::BuildRequest request = MakeRequest();
    request.codec = VideoCodec::H265;

    const builder::BuildResult result = builder::BuildFfmpegArgs(request);
    CHECK(result.ok);
    CHECK(HasPair(result.args, "-c:v", "libx265"));
    CHECK(HasPair(result.args, "-preset", "faster"));
    CHECK(HasPair(result.args, "-crf", "23"));
    CHECK(HasPair(result.args, "-x265-params", "log-level=error"));
    CHECK(HasPair(result.args, "-tag:v", "hvc1"));
    CHECK(HasPair(result.args, "-movflags", "+faststart"));
    CHECK(HasPair(result.args, "-f", "mp4"));
    CHECK(HasPair(result.args, "-pix_fmt", "yuv420p"));
}

TEST_CASE(Builder_H265Mkv)
{
    builder::BuildRequest request = MakeRequest(TargetFormat::MKV);
    request.codec = VideoCodec::H265;
    request.outputPath = "C:/out/a.mkv";

    const builder::BuildResult result = builder::BuildFfmpegArgs(request);
    CHECK(result.ok);
    CHECK(HasPair(result.args, "-f", "matroska"));
    CHECK(!Has(result.args, "-tag:v"));
    CHECK(!Has(result.args, "-movflags"));
}

TEST_CASE(Builder_SlimUsesQualityMapping)
{
    builder::BuildRequest request = MakeRequest();
    request.slim = true;
    request.quality = 0.19f;
    builder::BuildResult result = builder::BuildFfmpegArgs(request);
    CHECK(result.ok);
    CHECK(HasPair(result.args, "-crf", "30"));
    CHECK(HasPair(result.args, "-preset", "veryfast"));
    CHECK(!Has(result.args, "-x265-params"));

    request.codec = VideoCodec::H265;
    request.quality = 0.90f;
    result = builder::BuildFfmpegArgs(request);
    CHECK(result.ok);
    CHECK(HasPair(result.args, "-crf", "23"));
    CHECK(HasPair(result.args, "-preset", "medium"));
}

TEST_CASE(Builder_HevcHardwareMatrix)
{
    builder::BuildRequest request = MakeRequest();
    request.codec = VideoCodec::H265;
    request.quality = 0.55f;

    request.encoder = VideoEncoder::Nvenc;
    builder::BuildResult result = builder::BuildFfmpegArgs(request);
    CHECK(result.ok);
    CHECK(HasPair(result.args, "-c:v", "hevc_nvenc"));
    CHECK(HasPair(result.args, "-cq", "27"));
    CHECK(!Has(result.args, "-crf"));
    CHECK(!Has(result.args, "-x265-params"));

    request.encoder = VideoEncoder::Qsv;
    result = builder::BuildFfmpegArgs(request);
    CHECK(result.ok);
    CHECK(HasPair(result.args, "-c:v", "hevc_qsv"));
    CHECK(HasPair(result.args, "-global_quality", "27"));

    request.encoder = VideoEncoder::Amf;
    result = builder::BuildFfmpegArgs(request);
    CHECK(result.ok);
    CHECK(HasPair(result.args, "-c:v", "hevc_amf"));
    CHECK(HasPair(result.args, "-qp_i", "27"));
}

TEST_CASE(Builder_EvenDimensionGuard)
{
    builder::BuildRequest request = MakeRequest();
    request.sourceWidth = 1919;
    request.sourceHeight = 1079;

    builder::BuildResult result = builder::BuildFfmpegArgs(request);
    CHECK(result.ok);
    CHECK(HasPair(result.args, "-vf", "scale=trunc(iw/2)*2:trunc(ih/2)*2"));
    CHECK_EQ(static_cast<int>(std::count(result.args.begin(), result.args.end(), "-vf")), 1);

    request.resolutionIndex = 2;
    result = builder::BuildFfmpegArgs(request);
    CHECK(result.ok);
    CHECK(HasPair(result.args, "-vf", "scale=-2:720"));
    CHECK_EQ(static_cast<int>(std::count(result.args.begin(), result.args.end(), "-vf")), 1);

    builder::BuildRequest even = MakeRequest();
    even.sourceWidth = 1920;
    even.sourceHeight = 1080;
    result = builder::BuildFfmpegArgs(even);
    CHECK(!Has(result.args, "-vf"));
}

TEST_CASE(Builder_CodecValidation)
{
    builder::BuildRequest noCodec = MakeRequest();
    noCodec.codec = VideoCodec::None;
    builder::BuildResult result = builder::BuildFfmpegArgs(noCodec);
    CHECK(!result.ok);
    CHECK(result.errorText.find("只要声音") != std::string::npos);

    builder::BuildRequest audioWithCodec = MakeRequest(TargetFormat::MP3);
    audioWithCodec.codec = VideoCodec::H265;
    audioWithCodec.outputPath = "C:/out/a.mp3";
    result = builder::BuildFfmpegArgs(audioWithCodec);
    CHECK(result.ok);
    CHECK(!Has(result.args, "-c:v"));
    CHECK(!Has(result.args, "-tag:v"));
}

