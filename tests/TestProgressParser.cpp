#include "TestSupport.h"

#include "services/ProgressParser.h"

#include <string>

namespace pp = easyav::services;

TEST_CASE(Progress_WholeBlock)
{
    pp::ProgressParser parser(10.0);
    const std::string block = "frame=30\nout_time_us=5000000\nprogress=continue\n";
    parser.Feed(block.data(), block.size());

    CHECK_NEAR(parser.Fraction(), 0.5, 0.0001);
    CHECK_NEAR(parser.OutTimeSeconds(), 5.0, 0.0001);
    CHECK(!parser.Ended());
}

TEST_CASE(Progress_ByteAtATime)
{
    pp::ProgressParser parser(10.0);
    const std::string block = "out_time_us=2500000\nprogress=continue\n";
    for (const char ch : block)
        parser.Feed(&ch, 1);

    CHECK_NEAR(parser.Fraction(), 0.25, 0.0001);
}

TEST_CASE(Progress_SplitMidValue)
{
    pp::ProgressParser parser(10.0);
    const std::string part1 = "out_time_us=50";
    const std::string part2 = "00000\nprogress=continue\n";
    parser.Feed(part1.data(), part1.size());
    parser.Feed(part2.data(), part2.size());

    CHECK_NEAR(parser.Fraction(), 0.5, 0.0001);
}

TEST_CASE(Progress_CrlfBlankAndUnknownKeys)
{
    pp::ProgressParser parser(20.0);
    const std::string block =
        "\r\n"
        "frame=1\r\n"
        "unknown_key=42\r\n"
        "out_time_us=10000000\r\n"
        "\r\n"
        "progress=continue\r\n";
    parser.Feed(block.data(), block.size());

    CHECK_NEAR(parser.OutTimeSeconds(), 10.0, 0.0001);
    CHECK_NEAR(parser.Fraction(), 0.5, 0.0001);
}

TEST_CASE(Progress_AlternativeTimeKeys)
{
    pp::ProgressParser milliseconds(10.0);
    const std::string ms = "out_time_ms=2500000\n";
    milliseconds.Feed(ms.data(), ms.size());
    CHECK_NEAR(milliseconds.OutTimeSeconds(), 2.5, 0.0001);

    pp::ProgressParser clock(10.0);
    const std::string clockText = "out_time=00:00:05.500000\n";
    clock.Feed(clockText.data(), clockText.size());
    CHECK_NEAR(clock.OutTimeSeconds(), 5.5, 0.0001);
    CHECK_NEAR(clock.Fraction(), 0.55, 0.0001);
}

TEST_CASE(Progress_EndMarker)
{
    pp::ProgressParser parser(10.0);
    const std::string block = "out_time_us=10000000\nprogress=end\n";
    parser.Feed(block.data(), block.size());

    CHECK(parser.Ended());
    CHECK_NEAR(parser.Fraction(), 1.0, 0.0001);
}

TEST_CASE(Progress_UnknownDuration)
{
    pp::ProgressParser parser(0.0);
    const std::string block = "out_time_us=5000000\n";
    parser.Feed(block.data(), block.size());

    CHECK_NEAR(parser.Fraction(), -1.0, 0.0001);
    CHECK_NEAR(parser.OutTimeSeconds(), 5.0, 0.0001);
}

TEST_CASE(Progress_ClampsAndIgnoresGarbage)
{
    pp::ProgressParser parser(10.0);
    const std::string over = "out_time_us=20000000\n";
    parser.Feed(over.data(), over.size());
    CHECK_NEAR(parser.Fraction(), 1.0, 0.0001);

    pp::ProgressParser invalid(10.0);
    const std::string first = "out_time_us=3000000\n";
    invalid.Feed(first.data(), first.size());
    const std::string bad = "out_time_us=abc\n";
    invalid.Feed(bad.data(), bad.size());
    CHECK_NEAR(invalid.OutTimeSeconds(), 3.0, 0.0001);
}

TEST_CASE(Progress_LastBlockWins)
{
    pp::ProgressParser parser(10.0);
    const std::string block =
        "out_time_us=1000000\nprogress=continue\n"
        "out_time_us=4000000\nprogress=continue\n";
    parser.Feed(block.data(), block.size());

    CHECK_NEAR(parser.Fraction(), 0.4, 0.0001);
}

TEST_CASE(Progress_ResetClearsState)
{
    pp::ProgressParser parser(10.0);
    const std::string block = "out_time_us=5000000\nprogress=end\n";
    parser.Feed(block.data(), block.size());
    parser.Reset(20.0);

    CHECK_NEAR(parser.OutTimeSeconds(), 0.0, 0.0001);
    CHECK(!parser.Ended());
    CHECK_NEAR(parser.Fraction(), 0.0, 0.0001);
}
