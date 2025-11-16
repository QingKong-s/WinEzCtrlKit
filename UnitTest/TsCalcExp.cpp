#include "pch.h"
#include "../eck/CalculateExpression.h"

using namespace eck;

namespace Microsoft::VisualStudio::CppUnitTestFramework
{
    template<>
    static std::wstring ToString<eck::CalcExpResult>(const eck::CalcExpResult& v)
    {
        switch (v)
        {
        case eck::CalcExpResult::Ok:
            return L"CalcExpResult::Ok";
        case eck::CalcExpResult::InvalidChar:
            return L"CalcExpResult::InvalidChar";
        case eck::CalcExpResult::AdjacentOp:
            return L"CalcExpResult::AdjacentOp";
        case eck::CalcExpResult::UnmatchedParentheses:
            return L"CalcExpResult::UnmatchedParentheses";
        case eck::CalcExpResult::OpError:
            return L"CalcExpResult::OpError";
        default:
            return L"(Unknown CalcExpResult)";
        }
    }
}

TS_NS_BEGIN
TEST_CLASS(TsCalcExp)
{
    static void AssertDoubleEq(double a, double b, double eps = 1e-9)
    {
        Assert::IsTrue(fabs(a - b) < eps);
    }
public:
    TEST_METHOD(Calc_Simple_Add)
    {
        double r{};
        auto ret = CalculateExpression(r, "1+2");
        Assert::AreEqual(CalcExpResult::Ok, ret);
        AssertDoubleEq(3, r);
    }

    TEST_METHOD(Calc_Priority)
    {
        double r{};
        auto ret = CalculateExpression(r, "1+2*3");
        Assert::AreEqual(CalcExpResult::Ok, ret);
        AssertDoubleEq(7, r);
    }

    TEST_METHOD(Calc_Parentheses)
    {
        double r{};
        auto ret = CalculateExpression(r, "(1+2)*3");
        Assert::AreEqual(CalcExpResult::Ok, ret);
        AssertDoubleEq(9, r);
    }

    TEST_METHOD(Calc_UnaryMinus)
    {
        double r{};
        auto ret = CalculateExpression(r, "-1+2");
        Assert::AreEqual(CalcExpResult::Ok, ret);
        AssertDoubleEq(1, r);
    }

    TEST_METHOD(Calc_Exponent)
    {
        double r{};
        auto ret = CalculateExpression(r, "2^4");
        Assert::AreEqual(CalcExpResult::Ok, ret);
        AssertDoubleEq(16, r);
    }

    TEST_METHOD(Calc_Const_Pi)
    {
        double r{};
        auto ret = CalculateExpression(r, "Pi");
        Assert::AreEqual(CalcExpResult::Ok, ret);
        AssertDoubleEq(3.141592653589793, r);
    }

    TEST_METHOD(Calc_Func_Sin)
    {
        double r{};
        auto ret = CalculateExpression(r, "Sin(Pi/2)");
        Assert::AreEqual(CalcExpResult::Ok, ret);
        AssertDoubleEq(1, r);
    }

    TEST_METHOD(Calc_Func_Log)
    {
        double r{};
        auto ret = CalculateExpression(r, "Log(100)");
        Assert::AreEqual(CalcExpResult::Ok, ret);
        AssertDoubleEq(2, r);
    }

    TEST_METHOD(Calc_Func_Sqrt)
    {
        double r{};
        auto ret = CalculateExpression(r, "Sqrt(9)");
        Assert::AreEqual(CalcExpResult::Ok, ret);
        AssertDoubleEq(3, r);
    }

    TEST_METHOD(Calc_InvalidChar)
    {
        double r{};
        auto ret = CalculateExpression(r, "1+2@3");
        Assert::AreEqual(CalcExpResult::InvalidChar, ret);
    }

    // 以下3个测试将返回OpError，
    // 严格检测较复杂，暂不支持

    //TEST_METHOD(Calc_AdjacentOp)
    //{
    //    double r{};
    //    auto ret = CalculateExpression(r, "1*/2");
    //    Assert::AreEqual(CalcExpResult::AdjacentOp, ret);
    //}

    //TEST_METHOD(Calc_AdjacentOp_DoublePlus)
    //{
    //    double r{};
    //    auto ret = CalculateExpression(r, "1++2");
    //    // "++" 中第二个 + 会触发 AdjacentOp
    //    Assert::AreEqual(CalcExpResult::AdjacentOp, ret);
    //}

    //TEST_METHOD(Calc_UnmatchedParentheses_Left)
    //{
    //    double r{};
    //    auto ret = CalculateExpression(r, "(1+2");
    //    Assert::AreEqual(CalcExpResult::UnmatchedParentheses, ret);
    //}

    TEST_METHOD(Calc_UnmatchedParentheses_Right)
    {
        double r{};
        auto ret = CalculateExpression(r, "1+2)");
        Assert::AreEqual(CalcExpResult::UnmatchedParentheses, ret);
    }

    TEST_METHOD(Calc_OpError_MissingOperand)
    {
        double r{};
        auto ret = CalculateExpression(r, "1+");
        Assert::AreEqual(CalcExpResult::OpError, ret);
    }

    TEST_METHOD(Calc_OpError_Empty)
    {
        double r{};
        auto ret = CalculateExpression(r, "");
        Assert::AreEqual(CalcExpResult::OpError, ret);
    }

    TEST_METHOD(Calc_OpError_OnlyOperator)
    {
        double r{};
        auto ret = CalculateExpression(r, "*");
        Assert::AreEqual(CalcExpResult::AdjacentOp, ret);
    }

    TEST_METHOD(Calc_Whitespace)
    {
        double r{};
        auto ret = CalculateExpression(r, " 1 + 2 * ( 3 + 1 ) ");
        Assert::AreEqual(CalcExpResult::Ok, ret);
        AssertDoubleEq(1 + 2 * 4, r);
    }

    TEST_METHOD(Calc_ComplexExpression)
    {
        double r{};
        auto ret = CalculateExpression(
            r, "Sin(Pi/4)^2 + Cos(Pi/4)^2"
        );
        Assert::AreEqual(CalcExpResult::Ok, ret);
        AssertDoubleEq(1.0, r);
    }
};
TS_NS_END