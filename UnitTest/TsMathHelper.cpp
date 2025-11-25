#include "pch.h"
#include "../eck/MathHelper.h"

using namespace eck;

TS_NS_BEGIN
TEST_CLASS(TestAngles)
{
public:
    template<typename T>
    void TestDegRad()
    {
        Assert::AreEqual((T)Pi, Deg2Rad<T>((T)180), (T)1e-6);
        Assert::AreEqual((T)180, Rad2Deg<T>((T)Pi), (T)1e-6);
    }

    TEST_METHOD(Test_DegRad_float) { TestDegRad<float>(); }
    TEST_METHOD(Test_DegRad_double) { TestDegRad<double>(); }

    TEST_METHOD(Test_DegIn0To360)
    {
        Assert::AreEqual(350.0, DegIn0To360<double>(-10.0), 1e-12);
        Assert::AreEqual(10.0, DegIn0To360<double>(370.0), 1e-12);
    }
};

TEST_CLASS(TestCircle)
{
public:

    TEST_METHOD(Test_CalcPointFromCircleAngle)
    {
        float x, y;
        CalcPointFromCircleAngle(1.0f, 0.0f, x, y);
        Assert::AreEqual(1.0f, x, 1e-6f);
        Assert::AreEqual(0.0f, y, 1e-6f);
    }
};

TEST_CLASS(TestLine)
{
public:

    TEST_METHOD(Test_LineLength)
    {
        Assert::AreEqual(5.0f, CalcLineLength<float>(0, 0, 3, 4), 1e-6f);

        POINT p1{ 0,0 }, p2{ 3,4 };
        Assert::AreEqual(5.0f, CalcLineLength<float>(p1, p2), 1e-6f);
    }

    TEST_METHOD(Test_LineScalePos)
    {
        double x, y;
        CalcPointFromLineScalePos(0, 0, 10, 0, 0.5f, x, y);
        Assert::AreEqual(5.0, x, 1e-12);
        Assert::AreEqual(0.0, y, 1e-12);
    }

    TEST_METHOD(Test_LineEquationPointSlope)
    {
        double A, B, C;
        CalcLineEquation(0, 0, 1.0, A, B, C);
        // y = x => -x + y = 0 => A=1,B=-1,C=0
        Assert::AreEqual(1.0, A, 1e-12);
        Assert::AreEqual(-1.0, B, 1e-12);
        Assert::AreEqual(0.0, C, 1e-12);
    }

    TEST_METHOD(Test_LineEquationTwoPoints)
    {
        double A, B, C;
        CalcLineEquation(0, 0, 1, 1, A, B, C);
        // Same line: y=x => x - y = 0 => A=-1,B=1,C=0 (scale OK)
        Assert::IsTrue(fabs(A + 1) < 1e-12);
        Assert::IsTrue(fabs(B - 1) < 1e-12);
    }

    TEST_METHOD(Test_PointToLineDistance)
    {
        POINT p{ 0,1 }, p1{ 0,0 }, p2{ 1,0 };
        Assert::AreEqual(1.0f, CalcPointToLineDistance<float>(p, p1, p2), 1e-6f);
    }
};

TEST_CLASS(TestPolarRect)
{
public:

    TEST_METHOD(Test_Polar2Rect_Rect2Polar)
    {
        double x, y, r, theta;

        Polar2Rect(1.0, Pi / 2, x, y);
        Assert::AreEqual(0.0, x, 1e-12);
        Assert::AreEqual(1.0, y, 1e-12);

        Rect2Polar(0.0, 1.0, r, theta);
        Assert::AreEqual(1.0, r, 1e-12);
        Assert::AreEqual(Pi / 2, theta, 1e-12);
    }
};

TEST_CLASS(TestEllipse)
{
public:

    TEST_METHOD(Test_CalcPointFromEllipseAngle)
    {
        double x, y;
        CalcPointFromEllipseAngle(2.0, 1.0, 0.0, x, y);

        // angle=0 时，应在长轴右端点附近：x≈2
        Assert::IsTrue(fabs(x - 2.0) < 1e-6);
        Assert::IsTrue(fabs(y - 0.0) < 1e-6);
    }
};
TS_NS_END