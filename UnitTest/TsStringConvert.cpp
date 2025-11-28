#include "pch.h"

#include "../eck/StringConvert.h"

#include <charconv>

using namespace eck;

TS_NS_BEGIN
TEST_CLASS(TsStringConvert)
{
public:
    TEST_METHOD(TsTcsToInt_BasicDecimal)
    {
        int result;
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToInt("123", 3, result));
        Assert::AreEqual(123, result);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToInt("0", 1, result));
        Assert::AreEqual(0, result);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToInt("-456", 4, result));
        Assert::AreEqual(-456, result);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToInt("+789", 4, result));
        Assert::AreEqual(789, result);
    }

    TEST_METHOD(TsTcsToInt_HexadecimalWithPrefix)
    {
        int result;
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToInt("0x1A", 4, result));
        Assert::AreEqual(26, result);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToInt("0xFF", 4, result));
        Assert::AreEqual(255, result);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToInt("0X10", 4, result));
        Assert::AreEqual(16, result);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToInt("-0x10", 5, result));
        Assert::AreEqual(-16, result);
    }

    TEST_METHOD(TsTcsToInt_BinaryWithPrefix)
    {
        int result;
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToInt("0b1010", 6, result));
        Assert::AreEqual(10, result);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToInt("0B1111", 6, result));
        Assert::AreEqual(15, result);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToInt("-0b100", 6, result));
        Assert::AreEqual(-4, result);
    }

    TEST_METHOD(TsTcsToInt_ExplicitRadix)
    {
        int result;
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToInt("FF", 2, result, 16));
        Assert::AreEqual(255, result);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToInt("77", 2, result, 8));
        Assert::AreEqual(63, result);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToInt("1010", 4, result, 2));
        Assert::AreEqual(10, result);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToInt("Z", 1, result, 36));
        Assert::AreEqual(35, result);
    }

    TEST_METHOD(TsTcsToInt_Overflow)
    {
        int8_t result;
        auto err = TcsToInt("256", 3, result, 10);
        Assert::AreEqual((int)TcsCvtErr::Overflow, (int)err);
        Assert::AreEqual((int)std::numeric_limits<int8_t>::max(), (int)result);

        err = TcsToInt("-129", 4, result, 10);
        Assert::AreEqual((int)TcsCvtErr::Overflow, (int)err);
        Assert::AreEqual((int)std::numeric_limits<int8_t>::min(), (int)result);
    }

    TEST_METHOD(TsTcsToInt_EmptyString)
    {
        int result;
        auto err = TcsToInt("", 0, result);
        Assert::AreEqual((int)TcsCvtErr::Nothing, (int)err);
    }

    TEST_METHOD(TsTcsToInt_OnlySign)
    {
        int result;
        auto err = TcsToInt("-", 1, result);
        Assert::AreEqual((int)TcsCvtErr::OnlySign, (int)err);
    }

    TEST_METHOD(TsTcsToInt_OnlyRadixPrefix)
    {
        int result;
        auto err = TcsToInt("0x", 2, result);
        Assert::AreEqual((int)TcsCvtErr::OnlyRadixPrefix, (int)err);
    }

    TEST_METHOD(TsTcsToInt_PartialParsing)
    {
        int result;
        const char* input = "123abc";
        const char* end;
        auto err = TcsToInt(input, strlen(input), result, 10, &end);
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        Assert::AreEqual(123, result);
        Assert::AreEqual((size_t)3, (size_t)(end - input));
    }

    TEST_METHOD(TsTcsToInt_UnsignedTypes)
    {
        uint32_t result;
        auto err = TcsToInt("4294967295", 10, result, 10);
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        Assert::AreEqual(std::numeric_limits<uint32_t>::max(), result);
    }

    TEST_METHOD(TsTcsToInt_MaxAndMinValues)
    {
        int32_t result32;
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToInt("2147483647", 10, result32));
        Assert::AreEqual(std::numeric_limits<int32_t>::max(), result32);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToInt("-2147483648", 11, result32));
        Assert::AreEqual(std::numeric_limits<int32_t>::min(), result32);

        int64_t result64;
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToInt("9223372036854775807", 19, result64));
        Assert::AreEqual(std::numeric_limits<int64_t>::max(), result64);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToInt("-9223372036854775808", 20, result64));
        Assert::AreEqual(std::numeric_limits<int64_t>::min(), result64);
    }

    // ==================== TcsFromInt Tests ====================

    TEST_METHOD(TsTcsFromInt_BasicDecimal)
    {
        char buffer[128];
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsFromInt(buffer, sizeof(buffer), 123));
        Assert::AreEqual("123", buffer);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsFromInt(buffer, sizeof(buffer), 0));
        Assert::AreEqual("0", buffer);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsFromInt(buffer, sizeof(buffer), -456));
        Assert::AreEqual("-456", buffer);
    }

    TEST_METHOD(TsTcsFromInt_Hexadecimal)
    {
        char buffer[128];
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsFromInt(buffer, sizeof(buffer), 255, 16, TRUE));
        Assert::AreEqual("FF", buffer);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsFromInt(buffer, sizeof(buffer), 255, 16, FALSE));
        Assert::AreEqual("ff", buffer);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsFromInt(buffer, sizeof(buffer), 26, 16));
        Assert::AreEqual("1A", buffer);
    }

    TEST_METHOD(TsTcsFromInt_Binary)
    {
        char buffer[128];
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsFromInt(buffer, sizeof(buffer), 10, 2));
        Assert::AreEqual("1010", buffer);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsFromInt(buffer, sizeof(buffer), 15, 2));
        Assert::AreEqual("1111", buffer);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsFromInt(buffer, sizeof(buffer), 0, 2));
        Assert::AreEqual("0", buffer);
    }

    TEST_METHOD(TsTcsFromInt_Octal)
    {
        char buffer[128];
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsFromInt(buffer, sizeof(buffer), 63, 8));
        Assert::AreEqual("77", buffer);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsFromInt(buffer, sizeof(buffer), 8, 8));
        Assert::AreEqual("10", buffer);
    }

    TEST_METHOD(TsTcsFromInt_NegativeValues)
    {
        char buffer[128];
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsFromInt(buffer, sizeof(buffer), -16, 16));
        Assert::AreEqual("-10", buffer);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsFromInt(buffer, sizeof(buffer), -10, 2));
        Assert::AreEqual("-1010", buffer);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsFromInt(buffer, sizeof(buffer), -123, 10));
        Assert::AreEqual("-123", buffer);
    }

    TEST_METHOD(TsTcsFromInt_Padding)
    {
        char buffer[128];
        PCH a;
        auto err = TcsFromInt(buffer, sizeof(buffer), 5, 10, TRUE, (PCH*)nullptr, 3, '0');
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        Assert::AreEqual("005", buffer);

        err = TcsFromInt(buffer, sizeof(buffer), 123, 10, TRUE, (PCH*)nullptr, 5, '0');
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        Assert::AreEqual("00123", buffer);
    }

    TEST_METHOD(TsTcsFromInt_BufferTooSmall)
    {
        char buffer[3];
        auto err = TcsFromInt(buffer, sizeof(buffer), 12345, 10);
        Assert::AreEqual((int)TcsCvtErr::BufferTooSmall, (int)err);
    }

    TEST_METHOD(TsTcsFromInt_MaxAndMinValues)
    {
        char buffer[128];

        auto err = TcsFromInt(buffer, sizeof(buffer), std::numeric_limits<int32_t>::max());
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        Assert::AreEqual("2147483647", buffer);

        err = TcsFromInt(buffer, sizeof(buffer), std::numeric_limits<int32_t>::min());
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        Assert::AreEqual("-2147483648", buffer);
    }

    TEST_METHOD(TsTcsFromInt_EndPointer)
    {
        char buffer[128];
        char* end;
        auto err = TcsFromInt(buffer, sizeof(buffer), 123, 10, TRUE, &end);
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        Assert::AreEqual((size_t)3, (size_t)(end - buffer));
    }

    // ==================== TcsToFloat Tests ====================

    TEST_METHOD(TsTcsToFloat_BasicFloat)
    {
        double result;
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToFloat("123.456", 7, result));
        Assert::AreEqual(123.456, result, 1e-6);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToFloat("0.0", 3, result));
        Assert::AreEqual(0.0, result, 1e-6);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToFloat("-789.012", 8, result));
        Assert::AreEqual(-789.012, result, 1e-6);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToFloat("+456.789", 8, result));
        Assert::AreEqual(456.789, result, 1e-6);
    }

    TEST_METHOD(TsTcsToFloat_ScientificNotation)
    {
        double result;
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToFloat("1.23e2", 6, result));
        Assert::AreEqual(123.0, result, 1e-6);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToFloat("1.5E-3", 6, result));
        Assert::AreEqual(0.0015, result, 1e-8);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToFloat("-2.5e+2", 7, result));
        Assert::AreEqual(-250.0, result, 1e-6);
    }

    TEST_METHOD(TsTcsToFloat_SpecialValues)
    {
        float result;

        auto err = TcsToFloat("inf", 3, result);
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        Assert::IsTrue(std::isinf(result));

        err = TcsToFloat("-inf", 4, result);
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        Assert::IsTrue(std::isinf(result) && result < 0);

        err = TcsToFloat("nan", 3, result);
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        Assert::IsTrue(std::isnan(result));
    }

    TEST_METHOD(TsTcsToFloat_EdgeCases)
    {
        double result;
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToFloat("0.000001", 8, result));
        Assert::AreEqual(0.000001, result, 1e-9);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToFloat("1000000.0", 9, result));
        Assert::AreEqual(1000000.0, result, 1e-6);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToFloat(".5", 2, result));
        Assert::AreEqual(0.5, result, 1e-6);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToFloat("5.", 2, result));
        Assert::AreEqual(5.0, result, 1e-6);
    }

    TEST_METHOD(TsTcsToFloat_PartialParsing)
    {
        double result;
        const char* input = "123.456abc";
        const char* end;
        auto err = TcsToFloat(input, strlen(input), result, &end);
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        Assert::AreEqual(123.456, result, 1e-6);
        Assert::AreEqual((size_t)7, (size_t)(end - input));
    }

    // ==================== TcsFromFloat Tests ====================

    TEST_METHOD(TsTcsFromFloat_BasicFloat)
    {
        char buffer[128];

        auto err = TcsFromFloat(buffer, sizeof(buffer), 123.456);
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        double parsed;
        TcsToFloat(buffer, strlen(buffer), parsed);
        Assert::AreEqual(123.456, parsed, 1e-5);
    }

    TEST_METHOD(TsTcsFromFloat_ScientificFormat)
    {
        char buffer[128];
        auto err = TcsFromFloat(buffer, sizeof(buffer), 1234.5, (PCH*)nullptr, TcsFloatFmt::Scientific);
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        Assert::IsTrue(strstr(buffer, "e") != nullptr);

        err = TcsFromFloat(buffer, sizeof(buffer), 0.00012, (PCH*)nullptr, TcsFloatFmt::Scientific);
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        Assert::IsTrue(strstr(buffer, "e") != nullptr);
    }

    TEST_METHOD(TsTcsFromFloat_FixedFormat)
    {
        char buffer[128];
        auto err = TcsFromFloat(buffer, sizeof(buffer), 123.456789, (PCH*)nullptr, TcsFloatFmt::Fixed, 2);
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        Assert::IsTrue(strstr(buffer, "123.45") != nullptr || strstr(buffer, "123.46") != nullptr);
    }

    TEST_METHOD(TsTcsFromFloat_SpecialValues)
    {
        char buffer[128];

        auto err = TcsFromFloat(buffer, sizeof(buffer), std::numeric_limits<double>::infinity());
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        Assert::IsTrue(strstr(buffer, "inf") != nullptr);

        err = TcsFromFloat(buffer, sizeof(buffer), std::numeric_limits<double>::quiet_NaN());
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        Assert::IsTrue(strstr(buffer, "nan") != nullptr);
    }

    TEST_METHOD(TsTcsFromFloat_BufferTooSmall)
    {
        char buffer[2];
        auto err = TcsFromFloat(buffer, sizeof(buffer), 123.456);
        Assert::AreEqual((int)TcsCvtErr::BufferTooSmall, (int)err);
    }

    // ==================== Utility Tests ====================

    TEST_METHOD(TsBufferSizeCalculation)
    {
        Assert::IsTrue(TcsCvtCalcBufferSize<int32_t>(10) >= 12);
        Assert::IsTrue(TcsCvtCalcBufferSize<int64_t>(10) >= 21);
        Assert::IsTrue(TcsCvtCalcBufferSize<uint32_t>(16) >= 10);
        Assert::IsTrue(TcsCvtCalcBufferSize<int32_t>(2) >= 34);
    }

    // ==================== Round-trip Tests ====================

    TEST_METHOD(TsIntegerRoundTrip)
    {
        char buffer[128];
        int testValues[] = { 0, 1, -1, 123, -456, 2147483647, -2147483648 };

        for (int val : testValues)
        {
            TcsFromInt(buffer, sizeof(buffer), val);
            int parsed;
            auto err = TcsToInt(buffer, strlen(buffer), parsed);
            Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
            Assert::AreEqual(val, parsed);
        }
    }

    TEST_METHOD(TsFloatRoundTrip)
    {
        char buffer[128];
        double testValues[] = { 0.0, 1.0, -1.0, 123.456, -789.012, 1e10, 1e-10 };

        for (double val : testValues)
        {
            TcsFromFloat(buffer, sizeof(buffer), val);
            double parsed;
            auto err = TcsToFloat(buffer, strlen(buffer), parsed);
            Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
            Assert::AreEqual(val, parsed, std::abs(val) * 1e-6);
        }
    }

    TEST_METHOD(TsTcsToInt_LeadingZeros)
    {
        int result;
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToInt("00123", 5, result));
        Assert::AreEqual(123, result);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToInt("0x00FF", 6, result));
        Assert::AreEqual(255, result);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToInt("0b0001", 6, result));
        Assert::AreEqual(1, result);
    }

    TEST_METHOD(TsTcsToInt_SingleDigit)
    {
        int result;
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToInt("0", 1, result));
        Assert::AreEqual(0, result);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToInt("5", 1, result));
        Assert::AreEqual(5, result);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToInt("-9", 2, result));
        Assert::AreEqual(-9, result);
    }

    TEST_METHOD(TsTcsToInt_AllRadixes)
    {
        int result;
        // 测试所有支持的进制 (2-36)
        for (int radix = 2; radix <= 36; ++radix)
        {
            auto err = TcsToInt("10", 2, result, radix);
            Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
            Assert::AreEqual(radix, result); // "10" 在任何进制下都是该进制的值
        }
    }

    TEST_METHOD(TsTcsToInt_InvalidCharacters)
    {
        int result;
        const char* end;

        // 遇到非法字符停止
        auto err = TcsToInt("123xyz", 6, result, 10, &end);
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        Assert::AreEqual(123, result);
        Assert::AreEqual('x', *end);

        // 十六进制中的非法字符
        err = TcsToInt("ABG", 3, result, 16, &end);
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        Assert::AreEqual(0xAB, result);
        Assert::AreEqual('G', *end);
    }

    TEST_METHOD(TsTcsToInt_MultipleSignsAndPrefixes)
    {
        int result;
        const char* end;

        // 双重符号（应该只识别第一个）
        auto err = TcsToInt("--5", 3, result, 10, &end);
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        Assert::AreEqual(0, result);
        Assert::AreEqual('-', *end); // 第二个 '-' 不是数字
    }

    TEST_METHOD(TsTcsToInt_WhitespaceHandling)
    {
        int result;
        const char* end;

        // 前导空格（应该停止，因为函数不跳过空格）
        auto err = TcsToInt(" 123", 4, result, 10, &end);
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        Assert::AreEqual(0, result);
        Assert::AreEqual(' ', *end);
    }

    TEST_METHOD(TsTcsToInt_NearOverflowValues)
    {
        // 测试接近溢出边界的值
        int32_t result32;

        // 最大值 - 1
        auto err = TcsToInt("2147483646", 10, result32);
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        Assert::AreEqual(2147483646, result32);

        // 最小值 + 1
        err = TcsToInt("-2147483647", 11, result32);
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        Assert::AreEqual(-2147483647, result32);
    }

    TEST_METHOD(TsTcsFromInt_ZeroInAllRadixes)
    {
        char buffer[128];

        for (int radix = 2; radix <= 36; ++radix)
        {
            auto err = TcsFromInt(buffer, sizeof(buffer), 0, radix);
            Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
            Assert::AreEqual("0", buffer);
        }
    }

    TEST_METHOD(TsTcsFromInt_MaxDigitsAllRadixes)
    {
        char buffer[128];

        // 在各种进制下转换最大值
        auto err = TcsFromInt(buffer, sizeof(buffer), std::numeric_limits<int32_t>::max(), 2);
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        Assert::AreEqual("1111111111111111111111111111111", buffer); // 31个1

        err = TcsFromInt(buffer, sizeof(buffer), std::numeric_limits<int32_t>::max(), 36);
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        Assert::AreEqual("ZIK0ZJ", buffer);
    }

    TEST_METHOD(TsTcsFromInt_PaddingEdgeCases)
    {
        char buffer[128];

        // 填充宽度等于实际宽度
        auto err = TcsFromInt(buffer, sizeof(buffer), 123, 10, TRUE, (PCH*)nullptr, 3);
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        Assert::AreEqual("123", buffer);

        // 填充宽度小于实际宽度（不应填充）
        err = TcsFromInt(buffer, sizeof(buffer), 12345, 10, TRUE, (PCH*)nullptr, 3);
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        Assert::AreEqual("12345", buffer);

        // 负数的填充
        err = TcsFromInt(buffer, sizeof(buffer), -5, 10, TRUE, (PCH*)nullptr, 3, '0');
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        Assert::AreEqual("-005", buffer);
    }

    TEST_METHOD(TsTcsFromInt_CustomFillCharacter)
    {
        char buffer[128];

        auto err = TcsFromInt(buffer, sizeof(buffer), 5, 10, TRUE, (PCH*)nullptr, 5, '_');
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        Assert::AreEqual("____5", buffer);

        err = TcsFromInt(buffer, sizeof(buffer), 42, 10, TRUE, (PCH*)nullptr, 5, ' ');
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        Assert::AreEqual("   42", buffer);
    }

    // ==================== 宽字符测试 - 整数转换 ====================

    TEST_METHOD(TsTcsToInt_WideChar_BasicDecimal)
    {
        int result;
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToInt(L"123", 3, result));
        Assert::AreEqual(123, result);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToInt(L"-456", 4, result));
        Assert::AreEqual(-456, result);
    }

    TEST_METHOD(TsTcsToInt_WideChar_Hexadecimal)
    {
        int result;
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToInt(L"0xFF", 4, result));
        Assert::AreEqual(255, result);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToInt(L"ABCD", 4, result, 16));
        Assert::AreEqual(0xABCD, result);
    }

    TEST_METHOD(TsTcsToInt_WideChar_Binary)
    {
        int result;
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToInt(L"0b1010", 6, result));
        Assert::AreEqual(10, result);
    }

    TEST_METHOD(TsTcsFromInt_WideChar_BasicDecimal)
    {
        wchar_t buffer[128];
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsFromInt(buffer, 128, 123));
        Assert::AreEqual(L"123", buffer);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsFromInt(buffer, 128, -456));
        Assert::AreEqual(L"-456", buffer);
    }

    TEST_METHOD(TsTcsFromInt_WideChar_Hexadecimal)
    {
        wchar_t buffer[128];
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsFromInt(buffer, 128, 255, 16));
        Assert::AreEqual(L"FF", buffer);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsFromInt(buffer, 128, 0xABCD, 16, FALSE));
        Assert::AreEqual(L"abcd", buffer);
    }

    TEST_METHOD(TsTcsFromInt_WideChar_Padding)
    {
        wchar_t buffer[128];
        auto err = TcsFromInt(buffer, 128, 5, 10, TRUE, (PWCH*)nullptr, 4, L'0');
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        Assert::AreEqual(L"0005", buffer);
    }

    // ==================== 边界情况测试 - 浮点转换 ====================

    TEST_METHOD(TsTcsToFloat_VerySmallNumbers)
    {
        double result;

        // 非常接近零的数
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToFloat("1e-300", 6, result));
        Assert::AreEqual(1e-300, result, 1e-305);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToFloat("2.2250738585072014e-308", 23, result));
        Assert::IsTrue(result > 0 && result < 1e-300);
    }

    TEST_METHOD(TsTcsToFloat_VeryLargeNumbers)
    {
        double result;

        // 非常大的数
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToFloat("1e308", 5, result));
        Assert::AreEqual(1e308, result, 1e303);

        // 接近 double 最大值
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToFloat("1.7976931348623157e308", 22, result));
        Assert::IsTrue(result > 1e307);
    }

    TEST_METHOD(TsTcsToFloat_DifferentNotations)
    {
        double result;

        // 无小数点
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToFloat("123", 3, result));
        Assert::AreEqual(123.0, result, 1e-6);

        // 只有小数点和小数部分
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToFloat(".123", 4, result));
        Assert::AreEqual(0.123, result, 1e-6);

        // 只有整数部分和小数点
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToFloat("123.", 4, result));
        Assert::AreEqual(123.0, result, 1e-6);
    }

    TEST_METHOD(TsTcsToFloat_ZeroVariants)
    {
        double result;

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToFloat("0", 1, result));
        Assert::AreEqual(0.0, result);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToFloat("0.0", 3, result));
        Assert::AreEqual(0.0, result);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToFloat("-0.0", 4, result));
        Assert::AreEqual(0.0, result);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToFloat("0e10", 4, result));
        Assert::AreEqual(0.0, result);
    }

    TEST_METHOD(TsTcsToFloat_ExponentVariants)
    {
        double result;

        // 正指数
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToFloat("1e+5", 4, result));
        Assert::AreEqual(100000.0, result, 1e-6);

        // 负指数
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToFloat("1e-5", 4, result));
        Assert::AreEqual(0.00001, result, 1e-10);

        // 无符号指数
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToFloat("1e5", 3, result));
        Assert::AreEqual(100000.0, result, 1e-6);
    }

    TEST_METHOD(TsTcsToFloat_LeadingTrailingZeros)
    {
        double result;

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToFloat("00123.45000", 11, result));
        Assert::AreEqual(123.45, result, 1e-6);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToFloat("0.00100", 7, result));
        Assert::AreEqual(0.001, result, 1e-8);
    }

    TEST_METHOD(TsTcsToFloat_NegativeZero)
    {
        double result;
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToFloat("-0", 2, result));
        // -0.0 和 0.0 在 IEEE 754 中是不同的，但相等比较为真
        Assert::AreEqual(0.0, result);
    }

    TEST_METHOD(TsTcsFromFloat_VerySmallNumbers)
    {
        char buffer[128];

        auto err = TcsFromFloat(buffer, sizeof(buffer), 1e-100, (PCH*)nullptr, TcsFloatFmt::Scientific);
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        Assert::IsTrue(strstr(buffer, "e-") != nullptr);

        double parsed;
        TcsToFloat(buffer, strlen(buffer), parsed);
        Assert::AreEqual(1e-100, parsed, 1e-105);
    }

    TEST_METHOD(TsTcsFromFloat_VeryLargeNumbers)
    {
        char buffer[128];

        auto err = TcsFromFloat(buffer, sizeof(buffer), 1e100, (PCH*)nullptr, TcsFloatFmt::Scientific);
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        Assert::IsTrue(strstr(buffer, "e+") != nullptr || strstr(buffer, "e") != nullptr);
    }

    TEST_METHOD(TsTcsFromFloat_Zero)
    {
        char buffer[128];

        auto err = TcsFromFloat(buffer, sizeof(buffer), 0.0);
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        Assert::IsTrue(strstr(buffer, "0") != nullptr);
    }

    TEST_METHOD(TsTcsFromFloat_NegativeZero)
    {
        char buffer[128];

        auto err = TcsFromFloat(buffer, sizeof(buffer), -0.0);
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
    }

    TEST_METHOD(TsTcsFromFloat_PrecisionVariations)
    {
        char buffer[128];
        double value = 3.14159265358979;

        // 不同精度
        for (int precision = 0; precision <= 15; ++precision)
        {
            auto err = TcsFromFloat(buffer, sizeof(buffer), value, (PCH*)nullptr, TcsFloatFmt::Fixed, precision);
            Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
            // 验证输出不为空
            Assert::IsTrue(strlen(buffer) > 0);
        }
    }

    // ==================== 宽字符测试 - 浮点转换 ====================

    TEST_METHOD(TsTcsToFloat_WideChar_BasicFloat)
    {
        double result;
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToFloat(L"123.456", 7, result));
        Assert::AreEqual(123.456, result, 1e-6);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToFloat(L"-789.012", 8, result));
        Assert::AreEqual(-789.012, result, 1e-6);
    }

    TEST_METHOD(TsTcsToFloat_WideChar_ScientificNotation)
    {
        double result;
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToFloat(L"1.23e5", 6, result));
        Assert::AreEqual(123000.0, result, 1e-6);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToFloat(L"5.67E-3", 7, result));
        Assert::AreEqual(0.00567, result, 1e-8);
    }

    TEST_METHOD(TsTcsToFloat_WideChar_SpecialValues)
    {
        float result;

        auto err = TcsToFloat(L"inf", 3, result);
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        Assert::IsTrue(std::isinf(result));

        err = TcsToFloat(L"-inf", 4, result);
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        Assert::IsTrue(std::isinf(result) && result < 0);

        err = TcsToFloat(L"nan", 3, result);
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        Assert::IsTrue(std::isnan(result));
    }

    TEST_METHOD(TsTcsToFloat_WideChar_EdgeCases)
    {
        double result;

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToFloat(L".5", 2, result));
        Assert::AreEqual(0.5, result, 1e-6);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToFloat(L"5.", 2, result));
        Assert::AreEqual(5.0, result, 1e-6);

        Assert::AreEqual((int)TcsCvtErr::Ok, (int)TcsToFloat(L"0.0", 3, result));
        Assert::AreEqual(0.0, result);
    }

    TEST_METHOD(TsTcsFromFloat_WideChar_BasicFloat)
    {
        wchar_t buffer[128];

        auto err = TcsFromFloat(buffer, 128, 123.456);
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        double parsed;
        TcsToFloat(buffer, wcslen(buffer), parsed);
        Assert::AreEqual(123.456, parsed, 1e-5);
    }

    TEST_METHOD(TsTcsFromFloat_WideChar_ScientificFormat)
    {
        wchar_t buffer[128];

        auto err = TcsFromFloat(buffer, 128, 1234.5, (PWCH*)nullptr, TcsFloatFmt::Scientific);
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        Assert::IsTrue(wcsstr(buffer, L"e") != nullptr);
    }

    TEST_METHOD(TsTcsFromFloat_WideChar_FixedFormat)
    {
        wchar_t buffer[128];

        auto err = TcsFromFloat(buffer, 128, 123.456789, (PWCH*)nullptr, TcsFloatFmt::Fixed, 2);
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        Assert::IsTrue(wcsstr(buffer, L"123.45") != nullptr || wcsstr(buffer, L"123.46") != nullptr);
    }

    TEST_METHOD(TsTcsFromFloat_WideChar_SpecialValues)
    {
        wchar_t buffer[128];

        auto err = TcsFromFloat(buffer, 128, std::numeric_limits<double>::infinity());
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        Assert::IsTrue(wcsstr(buffer, L"inf") != nullptr);

        err = TcsFromFloat(buffer, 128, -std::numeric_limits<double>::infinity());
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        Assert::IsTrue(wcsstr(buffer, L"inf") != nullptr);

        err = TcsFromFloat(buffer, 128, std::numeric_limits<double>::quiet_NaN());
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        Assert::IsTrue(wcsstr(buffer, L"nan") != nullptr);
    }

    TEST_METHOD(TsTcsFromFloat_WideChar_Zero)
    {
        wchar_t buffer[128];

        auto err = TcsFromFloat(buffer, 128, 0.0);
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        Assert::IsTrue(wcsstr(buffer, L"0") != nullptr);
    }

    TEST_METHOD(TsTcsFromFloat_WideChar_Precision)
    {
        wchar_t buffer[128];

        auto err = TcsFromFloat(buffer, 128, 3.14159, (PWCH*)nullptr, TcsFloatFmt::Fixed, 2);
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        Assert::IsTrue(wcsstr(buffer, L"3.14") != nullptr);

        err = TcsFromFloat(buffer, 128, 3.14159, (PWCH*)nullptr, TcsFloatFmt::Fixed, 4);
        Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
        Assert::IsTrue(wcsstr(buffer, L"3.1415") != nullptr || wcsstr(buffer, L"3.1416") != nullptr);
    }

    // ==================== 往返测试 - 宽字符 ====================

    TEST_METHOD(TsRoundTrip_WideChar_Integer)
    {
        wchar_t buffer[128];
        int testValues[] = { 0, 1, -1, 42, -999, 123456, -123456 };

        for (int val : testValues)
        {
            TcsFromInt(buffer, 128, val);
            int parsed;
            auto err = TcsToInt(buffer, wcslen(buffer), parsed);
            Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
            Assert::AreEqual(val, parsed);
        }
    }

    TEST_METHOD(TsRoundTrip_WideChar_Float)
    {
        wchar_t buffer[128];
        double testValues[] = { 0.0, 1.0, -1.0, 3.14159, -2.71828, 1e6, 1e-6 };

        for (double val : testValues)
        {
            TcsFromFloat(buffer, 128, val);
            double parsed;
            auto err = TcsToFloat(buffer, wcslen(buffer), parsed);
            Assert::AreEqual((int)TcsCvtErr::Ok, (int)err);
            Assert::AreEqual(val, parsed, std::abs(val) * 1e-5);
        }
    }
};
TS_NS_END