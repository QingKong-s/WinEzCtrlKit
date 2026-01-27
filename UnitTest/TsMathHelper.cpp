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
TS_NS_END