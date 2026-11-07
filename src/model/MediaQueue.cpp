#include "model/MediaQueue.h"

#include "core/Format.h"
#include "core/Text.h"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <iterator>
#include <system_error>

namespace easyav
{
    namespace
    {
        namespace fs = std::filesystem;

        constexpr int kMaxQueuedFiles = 500;

        const char* const kVideoExtensions[] = {
            "mp4", "mkv", "mov", "avi", "flv", "webm", "wmv", "m4v", "mpg", "mpeg", "ts", "3gp"
        };
        const char* const kAudioExtensions[] = {
            "mp3", "wav", "flac", "aac", "m4a", "ogg", "wma", "opus", "aiff"
        };
        const char* const kImageExtensions[] = {
            "gif", "png", "jpg", "jpeg", "bmp", "webp", "tif", "tiff"
        };

        bool ExtensionInList(const std::string& ext, const char* const* list, std::size_t count)
        {
            for (std::size_t i = 0; i < count; ++i)
            {
                if (ext == list[i])
                    return true;
            }
            return false;
        }

        bool IsSupportedExtension(const std::string& ext)
        {
            return ExtensionInList(ext, kVideoExtensions, std::size(kVideoExtensions))
                || ExtensionInList(ext, kAudioExtensions, std::size(kAudioExtensions))
                || ExtensionInList(ext, kImageExtensions, std::size(kImageExtensions));
        }

        MediaKind KindForExtension(const std::string& ext)
        {
            if (ExtensionInList(ext, kVideoExtensions, std::size(kVideoExtensions)))
                return MediaKind::Video;
            if (ExtensionInList(ext, kAudioExtensions, std::size(kAudioExtensions)))
                return MediaKind::Audio;
            if (ExtensionInList(ext, kImageExtensions, std::size(kImageExtensions)))
                return MediaKind::Image;
            return MediaKind::Other;
        }

        std::string NormalizePath(const fs::path& path)
        {
            std::error_code ec;
            fs::path absolute = fs::absolute(path, ec);
            if (ec)
                absolute = path;
            return PathToUtf8(absolute.lexically_normal());
        }
    }

    int MediaQueue::AddFile(const std::string& utf8Path)
    {
        if (static_cast<int>(m_items.size()) >= kMaxQueuedFiles)
            return 0;

        const fs::path path = PathFromUtf8(utf8Path);
        std::error_code ec;
        if (!fs::is_regular_file(path, ec) || ec)
            return 0;

        const std::string normalized = NormalizePath(path);
        if (normalized.empty() || ContainsPath(normalized))
            return 0;

        const std::string extension = PathToUtf8(path.extension());
        if (extension.size() < 2)
            return 0;

        const std::string extensionLower = ToLowerAscii(extension.substr(1));
        if (!IsSupportedExtension(extensionLower))
            return 0;

        MediaItem item;
        item.id = m_nextId++;
        item.path = normalized;
        item.displayName = FileNameOf(normalized);
        item.format = ToUpperAscii(extensionLower);
        item.kind = KindForExtension(extensionLower);

        const std::uintmax_t size = fs::file_size(path, ec);
        item.sizeBytes = ec ? 0ull : static_cast<unsigned long long>(size);
        item.durationSec = 0.0; // 预留：由后续媒体探测模块填充

        m_items.push_back(std::move(item));
        return 1;
    }

    int MediaQueue::AddDirectory(const std::string& utf8Dir, int* skipped)
    {
        const fs::path root = PathFromUtf8(utf8Dir);
        std::error_code ec;
        if (!fs::is_directory(root, ec) || ec)
        {
            if (skipped)
                ++(*skipped);
            return 0;
        }

        std::vector<fs::path> candidates;
        fs::recursive_directory_iterator it(root, fs::directory_options::skip_permission_denied, ec);
        const fs::recursive_directory_iterator end;
        for (; it != end; it.increment(ec))
        {
            if (ec)
            {
                ec.clear();
                continue;
            }

            std::error_code fileEc;
            if (!it->is_regular_file(fileEc) || fileEc)
                continue;

            const std::string extension = PathToUtf8(it->path().extension());
            if (extension.size() >= 2 && IsSupportedExtension(ToLowerAscii(extension.substr(1))))
                candidates.push_back(it->path());
        }

        std::sort(candidates.begin(), candidates.end(),
                  [](const fs::path& a, const fs::path& b) { return a.wstring() < b.wstring(); });

        int added = 0;
        for (const fs::path& path : candidates)
        {
            if (static_cast<int>(m_items.size()) >= kMaxQueuedFiles)
            {
                if (skipped)
                    ++(*skipped);
                continue;
            }

            if (AddFile(PathToUtf8(path)))
                ++added;
            else if (skipped)
                ++(*skipped);
        }
        return added;
    }

    void MediaQueue::RemoveAt(int index)
    {
        if (index < 0 || index >= Count())
            return;
        m_items.erase(m_items.begin() + index);
    }

    void MediaQueue::Clear()
    {
        m_items.clear();
    }

    unsigned long long MediaQueue::TotalBytes() const
    {
        unsigned long long total = 0;
        for (const MediaItem& item : m_items)
            total += item.sizeBytes;
        return total;
    }

    bool MediaQueue::ContainsPath(const std::string& normalizedPath) const
    {
        for (const MediaItem& item : m_items)
        {
            if (item.path == normalizedPath)
                return true;
        }
        return false;
    }
}
