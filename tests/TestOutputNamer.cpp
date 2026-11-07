#include "TestSupport.h"

#include "core/Format.h"
#include "core/Text.h"
#include "services/OutputNamer.h"

#include <filesystem>
#include <string>

using easyav::PathFromUtf8;
using easyav::PathToUtf8;
namespace namer = easyav::services;

namespace
{
    std::string Join(const std::string& dir, const std::string& name)
    {
        return PathToUtf8(PathFromUtf8(dir) / PathFromUtf8(name));
    }

    std::string NameOf(const std::string& path)
    {
        return easyav::FileNameOf(path);
    }

    std::string ParentOf(const std::string& path)
    {
        return PathToUtf8(PathFromUtf8(path).parent_path());
    }

    namer::NamingRequest MakeRequest(const std::string& source,
                                     const std::string& outputDir,
                                     const std::string& suffix,
                                     easyav::TargetFormat format = easyav::TargetFormat::MP4)
    {
        namer::NamingRequest request;
        request.sourcePath = source;
        request.outputDir = outputDir;
        request.suffix = suffix;
        request.format = format;
        return request;
    }
}

TEST_CASE(Namer_BasicSuffix)
{
    const namer::NamingResult result = namer::Resolve(
        MakeRequest("C:/src/a.mp4", "C:/out", "_压缩"),
        [](const std::string&) { return false; }, 7);

    CHECK(result.ok);
    CHECK_STR(NameOf(result.finalPath), "a_压缩.mp4");
    CHECK_STR(ParentOf(result.finalPath), ParentOf("C:/out/x"));
    CHECK_STR(ParentOf(result.tempPath), ParentOf(result.finalPath));
    CHECK_STR(PathFromUtf8(result.tempPath).extension().string(), ".mp4");
    CHECK(result.tempPath.find(".easyav-tmp-") != std::string::npos);
}

TEST_CASE(Namer_EmptySuffixNeverOverwritesSource)
{
    const namer::NamingResult result = namer::Resolve(
        MakeRequest("C:/src/a.mp4", "C:/src", ""),
        [](const std::string&) { return false; }, 1);

    CHECK(result.ok);
    CHECK_STR(NameOf(result.finalPath), "a_1.mp4");
}

TEST_CASE(Namer_CollisionAddsNumber)
{
    const std::string finalName = Join("C:/out", "a_压缩.mp4");
    const std::string first = Join("C:/out", "a_压缩 (1).mp4");

    const namer::NamingResult result = namer::Resolve(
        MakeRequest("C:/src/a.mp4", "C:/out", "_压缩"),
        [&](const std::string& path) { return path == finalName || path == first; }, 2);

    CHECK(result.ok);
    CHECK_STR(NameOf(result.finalPath), "a_压缩 (2).mp4");
}

TEST_CASE(Namer_CaseInsensitiveSelfCollision)
{
    const namer::NamingResult result = namer::Resolve(
        MakeRequest("C:/src/A.MP4", "C:/src", ""),
        [](const std::string&) { return false; }, 1);

    CHECK(result.ok);
    CHECK_STR(NameOf(result.finalPath), "A_1.mp4");
}

TEST_CASE(Namer_IllegalSuffixCharacters)
{
    CHECK_STR(namer::SanitizeSuffix("a:b?c"), "a_b_c");
    CHECK_STR(namer::SanitizeSuffix("标签 . "), "标签");
    CHECK_STR(namer::SanitizeSuffix("x<>/\\|*"), "x______");

    const namer::NamingResult result = namer::Resolve(
        MakeRequest("C:/src/x.mp4", "C:/out", "a:b?c"),
        [](const std::string&) { return false; }, 1);
    CHECK(result.ok);
    CHECK_STR(NameOf(result.finalPath), "xa_b_c.mp4");
}

TEST_CASE(Namer_KeepsDotsInStem)
{
    const namer::NamingResult result = namer::Resolve(
        MakeRequest("C:/src/a.b.mp4", "C:/out", "_压缩"),
        [](const std::string&) { return false; }, 1);

    CHECK(result.ok);
    CHECK_STR(NameOf(result.finalPath), "a.b_压缩.mp4");
}

TEST_CASE(Namer_Mp3Extension)
{
    const namer::NamingResult result = namer::Resolve(
        MakeRequest("C:/src/a.mp4", "C:/out", "_音频", easyav::TargetFormat::MP3),
        [](const std::string&) { return false; }, 1);

    CHECK(result.ok);
    CHECK_STR(NameOf(result.finalPath), "a_音频.mp3");
    CHECK_STR(namer::ExtensionFor(easyav::TargetFormat::MKV), "mkv");
}

TEST_CASE(Namer_EmptyOutputDirUsesSourceParent)
{
    const namer::NamingResult result = namer::Resolve(
        MakeRequest("C:/src/sub/a.mp4", "", "_压缩"),
        [](const std::string&) { return false; }, 1);

    CHECK(result.ok);
    CHECK_STR(ParentOf(result.finalPath), ParentOf("C:/src/sub/x"));
}

TEST_CASE(Namer_LongPathRejected)
{
    const std::string longDir = "C:/" + std::string(250, 'a');
    const namer::NamingResult result = namer::Resolve(
        MakeRequest("C:/src/a.mp4", longDir, "_压缩"),
        [](const std::string&) { return false; }, 1);

    CHECK(!result.ok);
    CHECK(!result.errorText.empty());
}

TEST_CASE(Namer_EmptySourceRejected)
{
    const namer::NamingResult result = namer::Resolve(
        MakeRequest("", "C:/out", "_x"),
        [](const std::string&) { return false; }, 1);

    CHECK(!result.ok);
    CHECK(!result.errorText.empty());
}

TEST_CASE(Namer_SuffixCappedAtFortyChars)
{
    std::string repeated;
    for (int i = 0; i < 50; ++i)
        repeated += "中";

    const std::string sanitized = namer::SanitizeSuffix(repeated);
    CHECK_EQ(sanitized.size(), static_cast<std::size_t>(40 * 3));
}
