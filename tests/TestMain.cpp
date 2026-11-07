#include "TestSupport.h"

#include <cstdio>
#include <exception>

namespace easytest
{
    namespace
    {
        struct Entry
        {
            std::string name;
            TestFn fn;
        };

        std::vector<Entry>& Registry()
        {
            static std::vector<Entry> entries;
            return entries;
        }
    }

    void Register(const char* name, TestFn fn)
    {
        Registry().push_back(Entry{name ? name : "<unnamed>", fn});
    }

    void Fail(const char* file, int line, const std::string& what)
    {
        std::ostringstream os;
        os << file << ':' << line << ": " << what;
        throw Failure{os.str()};
    }

    int RunAll()
    {
        int failed = 0;
        for (const Entry& entry : Registry())
        {
            try
            {
                entry.fn();
                std::printf("[PASS] %s\n", entry.name.c_str());
            }
            catch (const Failure& failure)
            {
                ++failed;
                std::printf("[FAIL] %s\n       %s\n", entry.name.c_str(),
                            failure.message.c_str());
            }
            catch (const std::exception& ex)
            {
                ++failed;
                std::printf("[FAIL] %s\n       unexpected exception: %s\n",
                            entry.name.c_str(), ex.what());
            }
            catch (...)
            {
                ++failed;
                std::printf("[FAIL] %s\n       unexpected unknown exception\n",
                            entry.name.c_str());
            }
        }

        const int total = static_cast<int>(Registry().size());
        std::printf("\n%d passed, %d failed, %d total\n", total - failed, failed, total);
        return failed == 0 ? 0 : 1;
    }
}

int main()
{
    return easytest::RunAll();
}
