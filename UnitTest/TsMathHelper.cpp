#include "pch.h"
#include "../eck/MathHelper.h"

using namespace eck;

TS_NS_BEGIN
TEST_CLASS(TsAngles)
{
public:
    template<typename T>
    void TsDegRad()
    {
        Assert::AreEqual((T)Pi, DegreeToRadian<T>((T)180), (T)1e-6);
        Assert::AreEqual((T)180, RadianToDegree<T>((T)Pi), (T)1e-6);
    }

    TEST_METHOD(TsDegRad_float) { TsDegRad<float>(); }
    TEST_METHOD(TsDegRad_double) { TsDegRad<double>(); }

    TEST_METHOD(TsDegIn0To360)
    {
        Assert::AreEqual(350.0, DegreeIn0To360<double>(-10.0), 1e-12);
        Assert::AreEqual(10.0, DegreeIn0To360<double>(370.0), 1e-12);
    }
};

TEST_CLASS(TsCircle)
{
public:

    TEST_METHOD(TsCalcPointFromCircleAngle)
    {
        float x, y;
        CalculatePointFromCircleAngle(1.0f, 0.0f, x, y);
        Assert::AreEqual(1.0f, x, 1e-6f);
        Assert::AreEqual(0.0f, y, 1e-6f);
    }
};

TEST_CLASS(TsLine)
{
public:

    TEST_METHOD(TsLineLength)
    {
        Assert::AreEqual(5.0f, CalculateLineLength<float>(0, 0, 3, 4), 1e-6f);

        POINT p1{ 0,0 }, p2{ 3,4 };
        Assert::AreEqual(5.0f, CalculateLineLength<float>(p1, p2), 1e-6f);
    }

    TEST_METHOD(TsLineScalePos)
    {
        double x, y;
        CalculatePointFromLineScale(0, 0, 10, 0, 0.5f, x, y);
        Assert::AreEqual(5.0, x, 1e-12);
        Assert::AreEqual(0.0, y, 1e-12);
    }

    TEST_METHOD(TsLineEquationPointSlope)
    {
        double A, B, C;
        CalculateLineEquation(0, 0, 1.0, A, B, C);
        // y = x => -x + y = 0 => A=1,B=-1,C=0
        Assert::AreEqual(1.0, A, 1e-12);
        Assert::AreEqual(-1.0, B, 1e-12);
        Assert::AreEqual(0.0, C, 1e-12);
    }

    TEST_METHOD(TsLineEquationTwoPoints)
    {
        double A, B, C;
        CalculateLineEquation(0, 0, 1, 1, A, B, C);
        // Same line: y=x => x - y = 0 => A=-1,B=1,C=0 (scale OK)
        Assert::IsTrue(fabs(A + 1) < 1e-12);
        Assert::IsTrue(fabs(B - 1) < 1e-12);
    }

    TEST_METHOD(TsPointToLineDistance)
    {
        POINT p{ 0,1 }, p1{ 0,0 }, p2{ 1,0 };
        Assert::AreEqual(1.0f, CalculatePointToLineDistance<float>(p, p1, p2), 1e-6f);
    }
};

TEST_CLASS(TsPolarRect)
{
public:

    TEST_METHOD(TsPolar2Rect_Rect2Polar)
    {
        double x, y, r, theta;

        PolarToRect(1.0, Pi / 2, x, y);
        Assert::AreEqual(0.0, x, 1e-12);
        Assert::AreEqual(1.0, y, 1e-12);

        RectToPolar(0.0, 1.0, r, theta);
        Assert::AreEqual(1.0, r, 1e-12);
        Assert::AreEqual(Pi / 2, theta, 1e-12);
    }
};

TEST_CLASS(TsEllipse)
{
public:

    TEST_METHOD(TsCalcPointFromEllipseAngle)
    {
        double x, y;
        CalculatePointFromEllipseAngle(2.0, 1.0, 0.0, x, y);

        // angle=0 时，应在长轴右端点附近：x≈2
        Assert::IsTrue(fabs(x - 2.0) < 1e-6);
        Assert::IsTrue(fabs(y - 0.0) < 1e-6);
    }
};

TEST_CLASS(RadianInArcTest)
{
public:
    // 测试完整圆弧（扫描角度 >= 2π）
    TEST_METHOD(TsFullCirclePositive)
    {
        Assert::IsTrue(RadianInArc(0.0, 0.0, 2 * Pi));
        Assert::IsTrue(RadianInArc(Pi, 0.0, 2 * Pi));
        Assert::IsTrue(RadianInArc(-Pi, 0.0, 2 * Pi));
        Assert::IsTrue(RadianInArc(3.5, 1.0, 3 * Pi));
    }

    TEST_METHOD(TsFullCircleNegative)
    {
        Assert::IsTrue(RadianInArc(0.0, 0.0, -2 * Pi));
        Assert::IsTrue(RadianInArc(Pi, 0.0, -2 * Pi));
        Assert::IsTrue(RadianInArc(-Pi, 0.0, -2 * Pi));
    }

    // 测试正向扫描（逆时针）
    TEST_METHOD(TsPositiveSweepInRange)
    {
        // 从0开始，扫描π/2
        Assert::IsTrue(RadianInArc(0.0, 0.0, Pi / 2));
        Assert::IsTrue(RadianInArc(Pi / 4, 0.0, Pi / 2));
        Assert::IsTrue(RadianInArc(Pi / 2, 0.0, Pi / 2));
    }

    TEST_METHOD(TsPositiveSweepOutOfRange)
    {
        // 从0开始，扫描π/2
        Assert::IsFalse(RadianInArc(-0.1, 0.0, Pi / 2));
        Assert::IsFalse(RadianInArc(Pi / 2 + 0.1, 0.0, Pi / 2));
        Assert::IsFalse(RadianInArc(Pi, 0.0, Pi / 2));
    }

    // 测试负向扫描（顺时针）
    TEST_METHOD(TsNegativeSweepInRange)
    {
        // 从0开始，扫描-π/2
        Assert::IsTrue(RadianInArc(0.0, 0.0, -Pi / 2));
        Assert::IsTrue(RadianInArc(-Pi / 4, 0.0, -Pi / 2));
        Assert::IsTrue(RadianInArc(-Pi / 2, 0.0, -Pi / 2));
    }

    TEST_METHOD(TsNegativeSweepOutOfRange)
    {
        // 从0开始，扫描-π/2
        Assert::IsFalse(RadianInArc(0.1, 0.0, -Pi / 2));
        Assert::IsFalse(RadianInArc(-Pi / 2 - 0.1, 0.0, -Pi / 2));
        Assert::IsFalse(RadianInArc(Pi, 0.0, -Pi / 2));
    }

    // 测试跨越0度的情况
    TEST_METHOD(TsCrossingZero)
    {
        // 从-π/4开始，扫描π/2，应该包含0
        Assert::IsTrue(RadianInArc(0.0, -Pi / 4, Pi / 2));
        Assert::IsTrue(RadianInArc(-Pi / 4, -Pi / 4, Pi / 2));
        Assert::IsTrue(RadianInArc(Pi / 4, -Pi / 4, Pi / 2));
        Assert::IsFalse(RadianInArc(Pi / 2, -Pi / 4, Pi / 2));
    }

    // 测试跨越±π边界的情况
    TEST_METHOD(TsCrossingPiBoundary)
    {
        // 从π/2开始，扫描π，应该跨越π/-π边界
        Assert::IsTrue(RadianInArc(Pi / 2, Pi / 2, Pi));
        Assert::IsTrue(RadianInArc(Pi, Pi / 2, Pi));
        Assert::IsTrue(RadianInArc(-Pi + 0.5, Pi / 2, Pi));
        Assert::IsTrue(RadianInArc(-Pi / 2, Pi / 2, Pi));
    }

    // 测试不同起始角度
    TEST_METHOD(TsDifferentStartAngles)
    {
        // 从π/4开始，扫描π/2
        Assert::IsTrue(RadianInArc(Pi / 4, Pi / 4, Pi / 2));
        Assert::IsTrue(RadianInArc(Pi / 2, Pi / 4, Pi / 2));
        Assert::IsTrue(RadianInArc(3 * Pi / 4, Pi / 4, Pi / 2));
        Assert::IsFalse(RadianInArc(0.0, Pi / 4, Pi / 2));
        Assert::IsFalse(RadianInArc(Pi, Pi / 4, Pi / 2));
    }

    // 测试大于2π的角度输入（周期性）
    TEST_METHOD(TsAngleNormalization)
    {
        // ag = 2π + π/4 应该等价于 π/4
        Assert::IsTrue(RadianInArc(2 * Pi + Pi / 4, 0.0, Pi / 2));
        Assert::IsTrue(RadianInArc(-2 * Pi + Pi / 4, 0.0, Pi / 2));

        // 起始角度也可以大于2π
        Assert::IsTrue(RadianInArc(Pi / 4, 2 * Pi, Pi / 2));
    }

    // 边界条件测试
    TEST_METHOD(TsBoundaryConditions)
    {
        const double epsilon = 1e-10;

        // 恰好在边界上
        Assert::IsTrue(RadianInArc(0.0, 0.0, Pi));
        Assert::IsTrue(RadianInArc(Pi, 0.0, Pi));

        // 刚好超出边界
        Assert::IsFalse(RadianInArc(Pi + epsilon, 0.0, Pi));
        Assert::IsFalse(RadianInArc(-epsilon, 0.0, Pi));
    }

    // 零扫描角度
    TEST_METHOD(TsZeroSweep)
    {
        Assert::IsTrue(RadianInArc(0.0, 0.0, 0.0));
        Assert::IsFalse(RadianInArc(0.1, 0.0, 0.0));
        Assert::IsFalse(RadianInArc(-0.1, 0.0, 0.0));
    }

    // 测试float类型
    TEST_METHOD(TsFloatType)
    {
        Assert::IsTrue(RadianInArc(0.0f, 0.0f, static_cast<float>(Pi / 2)));
        Assert::IsTrue(RadianInArc(static_cast<float>(Pi / 4), 0.0f, static_cast<float>(Pi / 2)));
        Assert::IsFalse(RadianInArc(static_cast<float>(Pi), 0.0f, static_cast<float>(Pi / 2)));
    }

    // 逆时针完整测试
    TEST_METHOD(TsCounterClockwiseArc)
    {
        // 从90度开始，逆时针扫描180度
        double start = Pi / 2;
        double sweep = Pi;

        Assert::IsTrue(RadianInArc(Pi / 2, start, sweep));      // 起点
        Assert::IsTrue(RadianInArc(Pi, start, sweep));          // 中点
        Assert::IsTrue(RadianInArc(3 * Pi / 2, start, sweep));  // 终点
        Assert::IsFalse(RadianInArc(0.0, start, sweep));        // 在弧外
        Assert::IsFalse(RadianInArc(Pi / 4, start, sweep));     // 在弧外
    }

    // 顺时针完整测试
    TEST_METHOD(TsClockwiseArc)
    {
        // 从90度开始，顺时针扫描180度
        double start = Pi / 2;
        double sweep = -Pi;

        Assert::IsTrue(RadianInArc(Pi / 2, start, sweep));      // 起点
        Assert::IsTrue(RadianInArc(0.0, start, sweep));         // 中点
        Assert::IsTrue(RadianInArc(-Pi / 2, start, sweep));     // 终点
        Assert::IsFalse(RadianInArc(Pi, start, sweep));         // 在弧外
        Assert::IsFalse(RadianInArc(3 * Pi / 4, start, sweep)); // 在弧外
    }
};
TS_NS_END