#include "services/CommandLine.h"

#include "core/Text.h"

namespace easyav::services
{
    namespace
    {
        bool NeedsQuoting(const std::wstring& arg)
        {
            if (arg.empty())
                return true;
            for (const wchar_t c : arg)
            {
                if (c == L' ' || c == L'\t' || c == L'"')
                    return true;
            }
            return false;
        }

        std::wstring JoinArgs(const std::vector<std::string>& argsUtf8)
        {
            std::wstring line;
            for (std::size_t i = 0; i < argsUtf8.size(); ++i)
            {
                if (i != 0)
                    line.push_back(L' ');
                line += QuoteArgument(Utf8ToWide(argsUtf8[i]));
            }
            return line;
        }
    }

    std::wstring QuoteArgument(const std::wstring& arg)
    {
        if (!NeedsQuoting(arg))
            return arg;

        // 空参数必须写成 ""，否则会被解析为空命令行。
        if (arg.empty())
            return L"\"\"";

        std::wstring quoted;
        quoted.reserve(arg.size() + 2);
        quoted.push_back(L'"');

        std::size_t backslashes = 0;
        for (const wchar_t c : arg)
        {
            if (c == L'\\')
            {
                ++backslashes;
                continue;
            }

            if (c == L'"')
            {
                // 引号前连续反斜杠：2n 个变 2n+1 个，再输出引号。
                quoted.append(backslashes * 2 + 1, L'\\');
                quoted.push_back(L'"');
                backslashes = 0;
                continue;
            }

            quoted.append(backslashes, L'\\');
            backslashes = 0;
            quoted.push_back(c);
        }

        // 结尾反斜杠也要加倍，避免转义收尾引号。
        quoted.append(backslashes * 2, L'\\');
        quoted.push_back(L'"');
        return quoted;
    }

    std::wstring BuildCommandLine(const std::vector<std::string>& argsUtf8)
    {
        return JoinArgs(argsUtf8);
    }

    std::wstring BuildCommandLine(const std::wstring& exePath,
                                  const std::vector<std::string>& argsUtf8)
    {
        std::wstring line = QuoteArgument(exePath);
        if (!argsUtf8.empty())
        {
            line.push_back(L' ');
            line += JoinArgs(argsUtf8);
        }
        return line;
    }

    bool CommandLineFits(const std::wstring& commandLine)
    {
        return commandLine.size() < 32767;
    }
}
