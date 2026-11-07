#pragma once

// 极简测试框架：自带断言、无第三方依赖。
// 用法：
//   TEST_CASE(SomeName) { CHECK(cond); CHECK_EQ(a, b); }
// TestMain.cpp 负责注册表与 main()。

#include <cmath>
#include <sstream>
#include <string>
#include <vector>

namespace easytest
{
    using TestFn = void (*)();

    struct Failure
    {
        std::string message;
    };

    void Register(const char* name, TestFn fn);
    int RunAll();
    [[noreturn]] void Fail(const char* file, int line, const std::string& what);

    struct Registrar
    {
        Registrar(const char* name, TestFn fn) { Register(name, fn); }
    };
}

#define TEST_CASE(name) \
    static void name(); \
    static ::easytest::Registrar easytest_reg_##name(#name, name); \
    static void name()

#define CHECK(cond) \
    do { \
        if (!(cond)) \
            ::easytest::Fail(__FILE__, __LINE__, "CHECK failed: " #cond); \
    } while (0)

#define CHECK_EQ(actual, expected) \
    do { \
        const auto easytest_actual = (actual); \
        const auto easytest_expected = (expected); \
        if (!(easytest_actual == easytest_expected)) { \
            std::ostringstream easytest_os; \
            easytest_os << "CHECK_EQ failed: " #actual " == " #expected \
                        << " (actual=" << easytest_actual \
                        << ", expected=" << easytest_expected << ")"; \
            ::easytest::Fail(__FILE__, __LINE__, easytest_os.str()); \
        } \
    } while (0)

#define CHECK_STR(actual, expected) \
    do { \
        const std::string easytest_actual = (actual); \
        const std::string easytest_expected = (expected); \
        if (easytest_actual != easytest_expected) { \
            std::ostringstream easytest_os; \
            easytest_os << "CHECK_STR failed: " #actual " == " #expected \
                        << "\n  actual  : [" << easytest_actual << "]" \
                        << "\n  expected: [" << easytest_expected << "]"; \
            ::easytest::Fail(__FILE__, __LINE__, easytest_os.str()); \
        } \
    } while (0)

#define CHECK_NEAR(actual, expected, eps) \
    do { \
        const double easytest_actual = (actual); \
        const double easytest_expected = (expected); \
        if (std::fabs(easytest_actual - easytest_expected) > (eps)) { \
            std::ostringstream easytest_os; \
            easytest_os << "CHECK_NEAR failed: " #actual " ~= " #expected \
                        << " (actual=" << easytest_actual \
                        << ", expected=" << easytest_expected << ")"; \
            ::easytest::Fail(__FILE__, __LINE__, easytest_os.str()); \
        } \
    } while (0)
