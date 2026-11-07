#include "platform/FileDialogs.h"

#include "core/Text.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>
#include <shobjidl.h>

#include <cstring>
#endif

namespace easyav::platform
{
#ifdef _WIN32
    namespace
    {
        const wchar_t kMediaFilter[] =
            L"媒体文件\0"
            L"*.mp4;*.mkv;*.mov;*.avi;*.flv;*.webm;*.wmv;*.m4v;*.mpg;*.mpeg;*.ts;*.3gp;"
            L"*.mp3;*.wav;*.flac;*.aac;*.m4a;*.ogg;*.wma;*.opus;*.aiff;"
            L"*.gif;*.png;*.jpg;*.jpeg;*.bmp;*.webp;*.tif;*.tiff\0"
            L"所有文件\0*.*\0";

        const wchar_t kImageFilter[] =
            L"图片文件\0*.png;*.jpg;*.jpeg;*.bmp;*.webp;*.tif;*.tiff\0"
            L"所有文件\0*.*\0";

        std::vector<std::string> RunOpenDialog(const wchar_t* filter, const wchar_t* title,
                                               bool allowMultiple)
        {
            std::vector<wchar_t> buffer(65536, L'\0');

            OPENFILENAMEW ofn = {};
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = nullptr;
            ofn.lpstrFilter = filter;
            ofn.lpstrFile = buffer.data();
            ofn.nMaxFile = static_cast<DWORD>(buffer.size());
            ofn.lpstrTitle = title;
            ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
            if (allowMultiple)
                ofn.Flags |= OFN_ALLOWMULTISELECT;

            if (!GetOpenFileNameW(&ofn))
                return {};

            std::vector<std::string> results;
            const wchar_t* cursor = buffer.data();
            const std::wstring first = cursor;
            cursor += first.size() + 1;

            if (*cursor == L'\0')
            {
                results.push_back(WideToUtf8(first));
                return results;
            }

            const std::wstring directory = first;
            while (*cursor != L'\0')
            {
                const std::wstring name = cursor;
                cursor += name.size() + 1;
                results.push_back(WideToUtf8(directory + L"\\" + name));
            }
            return results;
        }
    }

    std::vector<std::string> PickMediaFiles()
    {
        return RunOpenDialog(kMediaFilter, L"选择要处理的文件", true);
    }

    std::string PickImageFile()
    {
        const std::vector<std::string> files = RunOpenDialog(kImageFilter, L"选择水印图片", false);
        return files.empty() ? std::string() : files.front();
    }

    std::string PickFolder()
    {
        std::string result;
        const HRESULT initialized = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

        IFileOpenDialog* dialog = nullptr;
        if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER,
                                       IID_PPV_ARGS(&dialog))))
        {
            DWORD options = 0;
            if (SUCCEEDED(dialog->GetOptions(&options)))
            {
                dialog->SetOptions(options | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST);
            }

            if (SUCCEEDED(dialog->Show(nullptr)))
            {
                IShellItem* item = nullptr;
                if (SUCCEEDED(dialog->GetResult(&item)))
                {
                    PWSTR path = nullptr;
                    if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &path)))
                    {
                        result = WideToUtf8(path);
                        CoTaskMemFree(path);
                    }
                    item->Release();
                }
            }
            dialog->Release();
        }

        if (SUCCEEDED(initialized))
            CoUninitialize();

        return result;
    }

    void OpenFolder(const std::string& utf8Path)
    {
        if (utf8Path.empty())
            return;
        const std::wstring path = Utf8ToWide(utf8Path);
        ShellExecuteW(nullptr, L"open", path.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    }
#else
    std::vector<std::string> PickMediaFiles()
    {
        return {};
    }

    std::string PickImageFile()
    {
        return {};
    }

    std::string PickFolder()
    {
        return {};
    }

    void OpenFolder(const std::string& utf8Path)
    {
        (void)utf8Path;
    }
#endif
}
