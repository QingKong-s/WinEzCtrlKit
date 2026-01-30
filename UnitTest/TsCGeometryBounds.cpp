#include "pch.h"
#include "../eck/CGeometryBounds.h"

using namespace eck;

TS_NS_BEGIN
TEST_CLASS(TsCGeometryBounds)
{
private:
    // 辅助函数：比较浮点数
    static bool FloatEqual(float a, float b, float epsilon = 1e-5f)
    {
        return fabsf(a - b) < epsilon;
    }

    // 辅助函数：比较矩形
    static bool RectEqual(const Kw::Rect& r1, const Kw::Rect& r2, float epsilon = 1e-5f)
    {
        return FloatEqual(r1.left, r2.left, epsilon) &&
            FloatEqual(r1.top, r2.top, epsilon) &&
            FloatEqual(r1.right, r2.right, epsilon) &&
            FloatEqual(r1.bottom, r2.bottom, epsilon);
    }

public:

    // ==================== AddPoint 测试 ====================

    TEST_METHOD(TestAddPoint_SinglePoint)
    {
        CGeometryBounds bounds;
        bounds.AddPoint({ 10.f, 20.f });

        auto& rc = bounds.GetBounds();
        Assert::IsTrue(FloatEqual(rc.left, 10.f));
        Assert::IsTrue(FloatEqual(rc.top, 20.f));
        Assert::IsTrue(FloatEqual(rc.right, 10.f));
        Assert::IsTrue(FloatEqual(rc.bottom, 20.f));
    }

    TEST_METHOD(TestAddPoint_MultiplePoints)
    {
        CGeometryBounds bounds;
        bounds.AddPoint({ 10.f, 20.f });
        bounds.AddPoint({ 30.f, 5.f });
        bounds.AddPoint({ 5.f, 40.f });

        auto& rc = bounds.GetBounds();
        Assert::IsTrue(FloatEqual(rc.left, 5.f));
        Assert::IsTrue(FloatEqual(rc.top, 5.f));
        Assert::IsTrue(FloatEqual(rc.right, 30.f));
        Assert::IsTrue(FloatEqual(rc.bottom, 40.f));
    }

    TEST_METHOD(TestAddPoint_NegativeCoordinates)
    {
        CGeometryBounds bounds;
        bounds.AddPoint({ -10.f, -20.f });
        bounds.AddPoint({ 10.f, 20.f });

        auto& rc = bounds.GetBounds();
        Assert::IsTrue(FloatEqual(rc.left, -10.f));
        Assert::IsTrue(FloatEqual(rc.top, -20.f));
        Assert::IsTrue(FloatEqual(rc.right, 10.f));
        Assert::IsTrue(FloatEqual(rc.bottom, 20.f));
    }

    TEST_METHOD(TestAddPoint_Array)
    {
        CGeometryBounds bounds;
        Kw::Vec2 points[] = {
            { 0.f, 0.f },
            { 100.f, 50.f },
            { 50.f, 100.f },
            { -10.f, -10.f }
        };
        bounds.AddPoint(points, 4);

        auto& rc = bounds.GetBounds();
        Assert::IsTrue(FloatEqual(rc.left, -10.f));
        Assert::IsTrue(FloatEqual(rc.top, -10.f));
        Assert::IsTrue(FloatEqual(rc.right, 100.f));
        Assert::IsTrue(FloatEqual(rc.bottom, 100.f));
    }

    // ==================== AddLine 测试 ====================

    TEST_METHOD(TestAddLine_HorizontalLine)
    {
        CGeometryBounds bounds;
        bounds.AddLine({ 10.f, 20.f }, { 50.f, 20.f });

        auto& rc = bounds.GetBounds();
        Assert::IsTrue(FloatEqual(rc.left, 10.f));
        Assert::IsTrue(FloatEqual(rc.top, 20.f));
        Assert::IsTrue(FloatEqual(rc.right, 50.f));
        Assert::IsTrue(FloatEqual(rc.bottom, 20.f));
    }

    TEST_METHOD(TestAddLine_VerticalLine)
    {
        CGeometryBounds bounds;
        bounds.AddLine({ 10.f, 20.f }, { 10.f, 60.f });

        auto& rc = bounds.GetBounds();
        Assert::IsTrue(FloatEqual(rc.left, 10.f));
        Assert::IsTrue(FloatEqual(rc.top, 20.f));
        Assert::IsTrue(FloatEqual(rc.right, 10.f));
        Assert::IsTrue(FloatEqual(rc.bottom, 60.f));
    }

    TEST_METHOD(TestAddLine_DiagonalLine)
    {
        CGeometryBounds bounds;
        bounds.AddLine({ 0.f, 0.f }, { 100.f, 100.f });

        auto& rc = bounds.GetBounds();
        Assert::IsTrue(FloatEqual(rc.left, 0.f));
        Assert::IsTrue(FloatEqual(rc.top, 0.f));
        Assert::IsTrue(FloatEqual(rc.right, 100.f));
        Assert::IsTrue(FloatEqual(rc.bottom, 100.f));
    }

    // ==================== AddRect 测试 ====================

    TEST_METHOD(TestAddRect_SingleRect)
    {
        CGeometryBounds bounds;
        bounds.AddRect({ 10.f, 20.f, 50.f, 60.f });

        auto& rc = bounds.GetBounds();
        Assert::IsTrue(FloatEqual(rc.left, 10.f));
        Assert::IsTrue(FloatEqual(rc.top, 20.f));
        Assert::IsTrue(FloatEqual(rc.right, 50.f));
        Assert::IsTrue(FloatEqual(rc.bottom, 60.f));
    }

    TEST_METHOD(TestAddRect_MultipleRects)
    {
        CGeometryBounds bounds;
        bounds.AddRect({ 10.f, 20.f, 50.f, 60.f });
        bounds.AddRect({ 0.f, 0.f, 30.f, 40.f });
        bounds.AddRect({ 40.f, 50.f, 100.f, 100.f });

        auto& rc = bounds.GetBounds();
        Assert::IsTrue(FloatEqual(rc.left, 0.f));
        Assert::IsTrue(FloatEqual(rc.top, 0.f));
        Assert::IsTrue(FloatEqual(rc.right, 100.f));
        Assert::IsTrue(FloatEqual(rc.bottom, 100.f));
    }

    // ==================== AddQuadraticBezier 测试 ====================

    TEST_METHOD(TestAddQuadraticBezier_StraightLine)
    {
        CGeometryBounds bounds;
        // 控制点在直线上，退化为直线
        bounds.AddQuadraticBezier(
            { 0.f, 0.f },
            { 50.f, 50.f },
            { 100.f, 100.f }
        );

        auto& rc = bounds.GetBounds();
        Assert::IsTrue(FloatEqual(rc.left, 0.f));
        Assert::IsTrue(FloatEqual(rc.top, 0.f));
        Assert::IsTrue(FloatEqual(rc.right, 100.f));
        Assert::IsTrue(FloatEqual(rc.bottom, 100.f));
    }

    TEST_METHOD(TestAddQuadraticBezier_WithExtrema)
    {
        CGeometryBounds bounds;
        // 抛物线，顶点在 t=0.5
        bounds.AddQuadraticBezier(
            { 0.f, 0.f },
            { 50.f, 100.f },
            { 100.f, 0.f }
        );

        auto& rc = bounds.GetBounds();
        Assert::IsTrue(FloatEqual(rc.left, 0.f));
        Assert::IsTrue(FloatEqual(rc.top, 0.f));
        Assert::IsTrue(FloatEqual(rc.right, 100.f));
        Assert::IsTrue(FloatEqual(rc.bottom, 50.f));
    }

    TEST_METHOD(TestAddQuadraticBezier_NoExtrema)
    {
        CGeometryBounds bounds;
        // 单调曲线，无极值
        bounds.AddQuadraticBezier(
            { 0.f, 0.f },
            { 110.f, 20.f },
            { 100.f, 50.f }
        );

        auto& rc = bounds.GetBounds();
        Assert::IsTrue(FloatEqual(rc.left, 0.f));
        Assert::IsTrue(FloatEqual(rc.top, 0.f));
        Assert::IsTrue(FloatEqual(rc.right, 100.833336f));
        Assert::IsTrue(FloatEqual(rc.bottom, 50.f));
    }

    // ==================== AddCubicBezier 测试 ====================

    TEST_METHOD(TestAddCubicBezier_WithExtrema)
    {
        CGeometryBounds bounds;
        // S曲线，有极值点
        bounds.AddCubicBezier(
            { -10.f, -10.f },
            { 25.f, 25.f },
            { -200.f, 60.f },
            { 100.f, 100.f }
        );

        auto& rc = bounds.GetBounds();
        Assert::IsTrue(FloatEqual(rc.left, -58.3533783f));
        Assert::IsTrue(FloatEqual(rc.top, -10.f));
        Assert::IsTrue(FloatEqual(rc.right, 100.f));
        Assert::IsTrue(FloatEqual(rc.bottom, 100.f));
    }

    TEST_METHOD(TestAddCubicBezier_NoExtrema)
    {
        CGeometryBounds bounds;
        // 单调曲线
        bounds.AddCubicBezier(
            { 0.f, 0.f },
            { 25.f, 25.f },
            { 75.f, 75.f },
            { 100.f, 100.f }
        );

        auto& rc = bounds.GetBounds();
        Assert::IsTrue(FloatEqual(rc.left, 0.f));
        Assert::IsTrue(FloatEqual(rc.top, 0.f));
        Assert::IsTrue(FloatEqual(rc.right, 100.f));
        Assert::IsTrue(FloatEqual(rc.bottom, 100.f));
    }

    TEST_METHOD(TestAddCubicBezier_MultipleExtrema)
    {
        CGeometryBounds bounds;
        // S曲线
        bounds.AddCubicBezier(
            { 50.f, 0.f },
            { 0.f, 100.f },
            { 100.f, 0.f },
            { 50.f, 100.f }
        );

        auto& rc = bounds.GetBounds();
        Assert::IsTrue(FloatEqual(rc.left, 35.5662460f));
        Assert::IsTrue(FloatEqual(rc.top, 0.f));
        Assert::IsTrue(FloatEqual(rc.right, 64.4337616f));
        Assert::IsTrue(FloatEqual(rc.bottom, 100.f));
    }

    // ==================== AddArc 测试 ====================

    TEST_METHOD(TestAddArc_FullCircle)
    {
        CGeometryBounds bounds;
        // 完整圆，半径50
        bounds.AddArc(
            { 100.f, 100.f },  // 圆心
            50.f, 50.f,        // 半径 a, b
            0.f,               // 起始角
            2.f * Pi           // 扫描角（完整圆）
        );

        auto& rc = bounds.GetBounds();
        Assert::IsTrue(FloatEqual(rc.left, 50.f));
        Assert::IsTrue(FloatEqual(rc.top, 50.f));
        Assert::IsTrue(FloatEqual(rc.right, 150.f));
        Assert::IsTrue(FloatEqual(rc.bottom, 150.f));
    }

    TEST_METHOD(TestAddArc_QuarterCircle_FirstQuadrant)
    {
        CGeometryBounds bounds;
        // 第一象限的1/4圆弧（0度到90度）
        bounds.AddArc(
            { 0.f, 0.f },      // 圆心
            100.f, 100.f,      // 半径
            0.f,               // 起始角
            Pi / 2.f           // 扫描角（90度）
        );

        auto& rc = bounds.GetBounds();
        Assert::IsTrue(FloatEqual(rc.left, 0.f, 1e-4f));
        Assert::IsTrue(FloatEqual(rc.top, 0.f, 1e-4f));
        Assert::IsTrue(FloatEqual(rc.right, 100.f, 1e-4f));
        Assert::IsTrue(FloatEqual(rc.bottom, 100.f, 1e-4f));
    }

    TEST_METHOD(TestAddArc_QuarterCircle_SecondQuadrant)
    {
        CGeometryBounds bounds;
        // 第二象限的1/4圆弧（90度到180度）
        bounds.AddArc(
            { 0.f, 0.f },
            100.f, 100.f,
            Pi / 2.f,          // 起始角90度
            Pi / 2.f           // 扫描角90度
        );

        auto& rc = bounds.GetBounds();
        Assert::IsTrue(RectEqual(rc, { -100.f, 0, 0, 100 }));
    }

    TEST_METHOD(TestAddArc_CrossingExtrema)
    {
        CGeometryBounds bounds;
        // 跨越右侧极值点（0度）的圆弧
        bounds.AddArc(
            { 0.f, 0.f },
            100.f, 100.f,
            -Pi / 4.f,         // 起始角-45度
            Pi / 2.f           // 扫描角90度（到45度）
        );

        auto& rc = bounds.GetBounds();
        Assert::IsTrue(FloatEqual(rc.right, 100.f, 1e-4f)); // 应该到达右侧极值
    }

    TEST_METHOD(TestAddArc_Ellipse)
    {
        CGeometryBounds bounds;
        // 椭圆弧，a != b
        bounds.AddArc(
            { 0.f, 0.f },
            150.f, 100.f,      // 水平半径150，垂直半径100
            0.f,
            Pi                 // 半个椭圆
        );

        auto& rc = bounds.GetBounds();
        Assert::IsTrue(FloatEqual(rc.right, 150.f, 1e-4f));  // 水平方向
        Assert::IsTrue(FloatEqual(rc.bottom, 100.f, 1e-4f)); // 垂直方向
        Assert::IsTrue(FloatEqual(rc.left, -150.f, 1e-4f));
    }

    TEST_METHOD(TestAddArc_NegativeSweep)
    {
        CGeometryBounds bounds;
        // 顺时针圆弧（负扫描角）
        bounds.AddArc(
            { 0.f, 0.f },
            100.f, 100.f,
            Pi / 2.f,          // 起始角90度
            -Pi / 2.f          // 顺时针扫描90度（到0度）
        );

        auto& rc = bounds.GetBounds();
        Assert::IsTrue(FloatEqual(rc.right, 100.f, 1e-4f)); // 应该到达右侧
        Assert::IsTrue(FloatEqual(rc.top, 0.f, 1e-4f));
    }

    TEST_METHOD(TestAddArc_SmallArc)
    {
        CGeometryBounds bounds;
        // 小圆弧，不跨越极值点
        bounds.AddArc(
            { 0.f, 0.f },
            100.f, 100.f,
            Pi / 6.f,          // 起始角30度
            Pi / 6.f           // 扫描角30度（到60度）
        );

        auto& rc = bounds.GetBounds();
        // 应该只包含端点，不包含极值点
        Assert::IsTrue(rc.right < 100.f);
        Assert::IsTrue(rc.left > 0.f);
    }

    // ==================== Reset 测试 ====================

    TEST_METHOD(TestReset)
    {
        CGeometryBounds bounds;
        bounds.AddPoint({ 10.f, 20.f });
        bounds.AddPoint({ 30.f, 40.f });

        bounds.Reset();

        auto& rc = bounds.GetBounds();
        Assert::IsTrue(FloatEqual(rc.left, FLT_MAX));
        Assert::IsTrue(FloatEqual(rc.top, FLT_MAX));
        Assert::IsTrue(FloatEqual(rc.right, -FLT_MAX));
        Assert::IsTrue(FloatEqual(rc.bottom, -FLT_MAX));
    }

    TEST_METHOD(TestReset_ReuseAfterReset)
    {
        CGeometryBounds bounds;
        bounds.AddPoint({ 10.f, 20.f });
        bounds.Reset();
        bounds.AddPoint({ 5.f, 15.f });

        auto& rc = bounds.GetBounds();
        Assert::IsTrue(FloatEqual(rc.left, 5.f));
        Assert::IsTrue(FloatEqual(rc.top, 15.f));
        Assert::IsTrue(FloatEqual(rc.right, 5.f));
        Assert::IsTrue(FloatEqual(rc.bottom, 15.f));
    }

    // ==================== 综合测试 ====================

    TEST_METHOD(TestEmptyBounds)
    {
        CGeometryBounds bounds;

        auto& rc = bounds.GetBounds();
        Assert::IsTrue(FloatEqual(rc.left, FLT_MAX));
        Assert::IsTrue(FloatEqual(rc.top, FLT_MAX));
        Assert::IsTrue(FloatEqual(rc.right, -FLT_MAX));
        Assert::IsTrue(FloatEqual(rc.bottom, -FLT_MAX));
    }

    TEST_METHOD(TestAddArc_FullCircleNegativeSweep)
    {
        CGeometryBounds bounds;
        // 完整圆，负扫描角
        bounds.AddArc(
            { 100.f, 100.f },
            50.f, 50.f,
            0.f,
            -2.f * Pi          // 负的完整圆
        );

        auto& rc = bounds.GetBounds();
        Assert::IsTrue(FloatEqual(rc.left, 50.f));
        Assert::IsTrue(FloatEqual(rc.top, 50.f));
        Assert::IsTrue(FloatEqual(rc.right, 150.f));
        Assert::IsTrue(FloatEqual(rc.bottom, 150.f));
    }

    TEST_METHOD(TestAddArc_StartAngleNormalization)
    {
        CGeometryBounds bounds1, bounds2;

        // 起始角度 0
        bounds1.AddArc({ 0.f, 0.f }, 100.f, 100.f, 0.f, Pi / 2.f);

        // 起始角度 2π（应该等价于0）
        bounds2.AddArc({ 0.f, 0.f }, 100.f, 100.f, 2.f * Pi, Pi / 2.f);

        Assert::IsTrue(RectEqual(bounds1.GetBounds(), bounds2.GetBounds()));
    }
};
TS_NS_END