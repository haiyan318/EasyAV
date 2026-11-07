#include "TestSupport.h"

#include "services/ErrorFormatter.h"

#include <string>

using easyav::FailureKind;
using easyav::VideoCodec;
namespace err = easyav::services;

TEST_CASE(Error_ClassifyCommonPatterns)
{
    CHECK(err::ClassifyFailure(1, "No such file or directory", false) == FailureKind::InputMissing);
    CHECK(err::ClassifyFailure(1, "moov atom not found", false) == FailureKind::InputCorrupt);
    CHECK(err::ClassifyFailure(1, "Invalid data found when processing input", false)
          == FailureKind::InputCorrupt);
    CHECK(err::ClassifyFailure(1, "Permission denied", false) == FailureKind::PermissionDenied);
    CHECK(err::ClassifyFailure(1, "Error opening output file out.mp4: Permission denied", false)
          == FailureKind::PermissionDenied);
    CHECK(err::ClassifyFailure(1, "No space left on device", false) == FailureKind::DiskFull);
    CHECK(err::ClassifyFailure(1, "Unknown encoder 'libx265'", false)
          == FailureKind::EncoderUnavailable);
    CHECK(err::ClassifyFailure(1, "Filename too long", false) == FailureKind::PathTooLong);
}

TEST_CASE(Error_ClassifyFallbacks)
{
    CHECK(err::ClassifyFailure(1, "", true) == FailureKind::EmptyOutput);
    CHECK(err::ClassifyFailure(1, "", false) == FailureKind::Unknown);
    CHECK(err::ClassifyFailure(static_cast<int>(0xC0000005u), "", false)
          == FailureKind::Unknown);
}

TEST_CASE(Error_UserMessagesAreChineseAndActionable)
{
    const FailureKind kinds[] = {
        FailureKind::Unknown, FailureKind::InputMissing, FailureKind::InputCorrupt,
        FailureKind::PermissionDenied, FailureKind::DiskFull, FailureKind::UnsupportedCodec,
        FailureKind::PathTooLong, FailureKind::OutputLocked, FailureKind::EncoderUnavailable,
        FailureKind::CodecUnavailable, FailureKind::ProcessStartFailed,
        FailureKind::Canceled, FailureKind::EmptyOutput};

    for (const FailureKind kind : kinds)
    {
        const std::string message = err::UserMessage(kind);
        CHECK(!message.empty());

        const std::string lower = message;
        CHECK(lower.find("-crf") == std::string::npos);
        CHECK(lower.find("libx264") == std::string::npos);
        CHECK(lower.find("Invalid data") == std::string::npos);
        CHECK(lower.find("moov atom") == std::string::npos);
    }
}

TEST_CASE(Error_CodecUnavailableGuidesCodecChoice)
{
    const std::string hevc = err::UserMessage(FailureKind::CodecUnavailable, {},
                                              VideoCodec::H265);
    CHECK(hevc.find("H.265") != std::string::npos);
    CHECK(hevc.find("H.264") != std::string::npos);

    const std::string h264 = err::UserMessage(FailureKind::CodecUnavailable, {},
                                              VideoCodec::H264);
    CHECK(h264.find("组件") != std::string::npos);

    const std::string gpu = err::UserMessage(FailureKind::EncoderUnavailable, {},
                                             VideoCodec::H264, true);
    CHECK(gpu.find("显卡") != std::string::npos);
}

TEST_CASE(Error_UserMessageAppendsFileName)
{
    const std::string withFile = err::UserMessage(FailureKind::InputCorrupt, "旅行.mp4");
    CHECK(withFile.find("旅行.mp4") != std::string::npos);
    CHECK(withFile.find("（") != std::string::npos);

    const std::string withoutFile = err::UserMessage(FailureKind::InputCorrupt);
    CHECK(withoutFile.find("（") == std::string::npos);
}

TEST_CASE(Error_ShortDetailTrimsAndCaps)
{
    CHECK_STR(err::ShortDetail("line1\r\nline2\r\n  "), "line1\r\nline2");

    std::string longText;
    for (int i = 0; i < 300; ++i)
        longText += "0123456789";

    const std::string detail = err::ShortDetail(longText, 100);
    CHECK(detail.size() <= 103); // 省略号 + 100 字符
    CHECK(detail.find("0123456789") != std::string::npos);
    CHECK(detail.rfind("0123456789") != std::string::npos);
}
