#include "TestSupport.h"

#include "core/Text.h"
#include "services/CommandLine.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <shellapi.h>

#include <string>
#include <vector>

using easyav::WideToUtf8;
namespace cmd = easyav::services;

namespace
{
    std::string N(const std::wstring& text)
    {
        return WideToUtf8(text);
    }

    // 用 CommandLineToArgvW 反向解析，确保与 Windows 规则一致。
    void ExpectRoundTrip(const std::vector<std::string>& args)
    {
        const std::wstring exePath = L"C:\\Program Files\\EasyAV\\ffmpeg.exe";
        const std::wstring line = cmd::BuildCommandLine(exePath, args);
        CHECK(cmd::CommandLineFits(line));

        int argc = 0;
        LPWSTR* argv = CommandLineToArgvW(line.c_str(), &argc);
        CHECK(argv != nullptr);
        if (!argv)
            return;

        CHECK_EQ(argc, static_cast<int>(args.size()) + 1);
        CHECK_STR(N(argv[0]), N(exePath));
        for (int i = 1; i < argc; ++i)
            CHECK_STR(N(argv[i]), args[static_cast<std::size_t>(i - 1)]);

        LocalFree(argv);
    }
}

TEST_CASE(QuoteArgument_PlainUnchanged)
{
    CHECK_STR(N(cmd::QuoteArgument(L"-hide_banner")), "-hide_banner");
    CHECK_STR(N(cmd::QuoteArgument(L"C:\\ffmpeg.exe")), "C:\\ffmpeg.exe");
}

TEST_CASE(QuoteArgument_SpacesAndTabs)
{
    CHECK_STR(N(cmd::QuoteArgument(L"a b")), "\"a b\"");
    CHECK_STR(N(cmd::QuoteArgument(L"a\tb")), "\"a\tb\"");
}

TEST_CASE(QuoteArgument_Empty)
{
    CHECK_STR(N(cmd::QuoteArgument(L"")), "\"\"");
}

TEST_CASE(QuoteArgument_EmbeddedQuote)
{
    CHECK_STR(N(cmd::QuoteArgument(L"a\"b")), "\"a\\\"b\"");
}

TEST_CASE(QuoteArgument_TrailingBackslash)
{
    // 只有需要加引号时，结尾反斜杠才必须加倍。
    CHECK_STR(N(cmd::QuoteArgument(L"C:\\my dir\\")), "\"C:\\my dir\\\\\"");
}

TEST_CASE(BuildCommandLine_WithExeAndArgs)
{
    const std::vector<std::string> args = {"-i", "a b", ""};
    CHECK_STR(N(cmd::BuildCommandLine(L"C:\\ffmpeg.exe", args)),
              "C:\\ffmpeg.exe -i \"a b\" \"\"");
    CHECK_STR(N(cmd::BuildCommandLine(args)), "-i \"a b\" \"\"");
}

TEST_CASE(RoundTrip_CommonArguments)
{
    ExpectRoundTrip({});
    ExpectRoundTrip({"-hide_banner", "-nostdin", "-loglevel", "error"});
    ExpectRoundTrip({"a b", "c\td", ""});
    ExpectRoundTrip({"C:\\dir\\", "C:\\dir\\file", "C:\\Program Files\\x y\\z", "C:\\my dir\\"});
}

TEST_CASE(RoundTrip_QuotesAndBackslashes)
{
    ExpectRoundTrip({"quote\"inside", "two\\\"quotes", "\\\"", "\\\\"});
    ExpectRoundTrip({"tail\\", "tail\\\\", "C:\\dir\\\\"});
}

TEST_CASE(RoundTrip_UnicodeAndShellChars)
{
    ExpectRoundTrip({"中文 路径.mp4", "表情😀", "&|^%$#@!"});
}

TEST_CASE(CommandLineFits_Limit)
{
    CHECK(cmd::CommandLineFits(std::wstring(32000, L'a')));
    CHECK(!cmd::CommandLineFits(std::wstring(32767, L'a')));
    CHECK(!cmd::CommandLineFits(std::wstring(40000, L'a')));
}
