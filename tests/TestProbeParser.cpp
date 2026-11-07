#include "TestSupport.h"

#include "services/ProbeOutputParser.h"

#include <string>

using easyav::services::ParseFfprobeText;

TEST_CASE(Probe_VideoWithAudio)
{
    const std::string text = R"([STREAM]
index=0
codec_name=h264
codec_type=video
width=1920
height=1080
[/STREAM]
[STREAM]
index=1
codec_name=aac
codec_type=audio
[/STREAM]
[FORMAT]
nb_streams=2
format_name=mov,mp4,m4a,3gp,3g2,mj2
duration=12.500000
[/FORMAT]
)";

    const easyav::MediaInfo info = ParseFfprobeText(text);
    CHECK(info.ok);
    CHECK(info.hasVideo);
    CHECK(info.hasAudio);
    CHECK_EQ(info.width, 1920);
    CHECK_EQ(info.height, 1080);
    CHECK_NEAR(info.durationSec, 12.5, 0.001);
    CHECK_STR(info.videoCodec, "h264");
    CHECK_STR(info.audioCodec, "aac");
}

TEST_CASE(Probe_AudioOnly)
{
    const std::string text = R"([STREAM]
index=0
codec_name=mp3
codec_type=audio
[/STREAM]
[FORMAT]
duration=305.123456
[/FORMAT]
)";

    const easyav::MediaInfo info = ParseFfprobeText(text);
    CHECK(info.ok);
    CHECK(!info.hasVideo);
    CHECK(info.hasAudio);
    CHECK_NEAR(info.durationSec, 305.123456, 0.001);
    CHECK_EQ(info.width, 0);
    CHECK_STR(info.audioCodec, "mp3");
}

TEST_CASE(Probe_DurationNotAvailable)
{
    const std::string text = R"([STREAM]
codec_name=png
codec_type=video
width=64
height=64
[/STREAM]
[FORMAT]
duration=N/A
[/FORMAT]
)";

    const easyav::MediaInfo info = ParseFfprobeText(text);
    CHECK(info.ok);
    CHECK(info.hasVideo);
    CHECK_NEAR(info.durationSec, 0.0, 0.0001);
}

TEST_CASE(Probe_CrlfAndUnknownKeys)
{
    const std::string text =
        "[STREAM]\r\n"
        "index=0\r\n"
        "codec_name=hevc\r\n"
        "codec_type=video\r\n"
        "width=3840\r\n"
        "height=2160\r\n"
        "unknown_key=value\r\n"
        "[/STREAM]\r\n"
        "[FORMAT]\r\n"
        "duration=60.000000\r\n"
        "[/FORMAT]\r\n";

    const easyav::MediaInfo info = ParseFfprobeText(text);
    CHECK(info.ok);
    CHECK(info.hasVideo);
    CHECK_EQ(info.width, 3840);
    CHECK_EQ(info.height, 2160);
    CHECK_NEAR(info.durationSec, 60.0, 0.001);
}

TEST_CASE(Probe_EmptyOrGarbageIsNotOk)
{
    CHECK(!ParseFfprobeText("").ok);
    CHECK(!ParseFfprobeText("hello\nworld\n").ok);
    CHECK(!ParseFfprobeText("[FORMAT]\nduration=1.0\n[/FORMAT]\n").ok);
}

TEST_CASE(Probe_FirstVideoStreamWins)
{
    const std::string text = R"([STREAM]
codec_name=h264
codec_type=video
width=1280
height=720
[/STREAM]
[STREAM]
codec_name=h264
codec_type=video
width=640
height=360
[/STREAM]
[STREAM]
codec_name=aac
codec_type=audio
[/STREAM]
[FORMAT]
duration=5.000000
[/FORMAT]
)";

    const easyav::MediaInfo info = ParseFfprobeText(text);
    CHECK(info.ok);
    CHECK_EQ(info.width, 1280);
    CHECK_EQ(info.height, 720);
}
