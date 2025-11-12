#include "pch.h"
#include "../eck/CalculateExpression.h"

TS_NS_BEGIN
TEST_CLASS(TsCalcExp)
{
public:
    TEST_METHOD(Ts)
    {
        double r;

        eck::CalculateExpression(r, "2 + 3 * 4 / 2 - 1");
        Assert::AreEqual(7., r);

        eck::CalculateExpression(r, L"2 ^ 3 ^ 2");
        Assert::AreEqual(64., r);

        eck::CalculateExpression(r, "2 + 3 * (4 / 2) - 1");
        Assert::AreEqual(7., r);

        eck::CalculateExpression(r, "2 + (6 - 2) * 3");
        Assert::AreEqual(14., r);

        eck::CalculateExpression(r, "2+3*4", 3);
        Assert::AreEqual(5., r);
    }
};
TS_NS_END