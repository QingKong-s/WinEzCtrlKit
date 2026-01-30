#include "pch.h"
#include "../eck/CGeometryPath.h"

using namespace eck;

TS_NS_BEGIN
TEST_CLASS(TsCGeometryPath)
{
private:
    // 辅助函数：验证点和类型的数量是否匹配
    void AssertPointTypeCountMatch(CGeometryPath& path)
    {
        auto& vPoint = path.DbgGetPointList();
        auto& vType = path.DbgGetTypeList();
        Assert::AreEqual(vPoint.Size(), vType.Size(), L"Point count should match Type count");
    }

    // 辅助函数：检查PathType是否包含指定标志
    bool HasFlag(PathType type, PathType flag)
    {
        return (static_cast<BYTE>(type) & static_cast<BYTE>(flag)) == static_cast<BYTE>(flag);
    }

public:
    TEST_METHOD(TestSetFillMode)
    {
        CGeometryPath path;

        // 测试设置WINDING填充模式
        path.SetFillMode(D2D1_FILL_MODE_WINDING);
        Assert::AreEqual((int)D2D1_FILL_MODE_WINDING, (int)path.GetFillMode());

        // 测试设置ALTERNATE填充模式
        path.SetFillMode(D2D1_FILL_MODE_ALTERNATE);
        Assert::AreEqual((int)D2D1_FILL_MODE_ALTERNATE, (int)path.GetFillMode());
    }

    TEST_METHOD(TestBeginFigure_Filled)
    {
        CGeometryPath path;
        D2D1_POINT_2F startPoint = { 10.0f, 20.0f };

        path.BeginFigure(startPoint, D2D1_FIGURE_BEGIN_FILLED);

        auto& vPoint = path.DbgGetPointList();
        auto& vType = path.DbgGetTypeList();

        Assert::AreEqual((size_t)1, vPoint.Size(), L"Should have 1 point after BeginFigure");
        Assert::AreEqual((size_t)1, vType.Size(), L"Should have 1 type after BeginFigure");

        // 验证点坐标
        Assert::AreEqual(10.0f, vPoint[0].x, L"X coordinate should match");
        Assert::AreEqual(20.0f, vPoint[0].y, L"Y coordinate should match");

        // 验证类型标志
        Assert::IsTrue(HasFlag(vType[0], PathType::FgBegin), L"Should have FgBegin flag");
        Assert::IsTrue(HasFlag(vType[0], PathType::FgFill), L"Should have FgFill flag");
    }

    TEST_METHOD(TestBeginFigure_Hollow)
    {
        CGeometryPath path;
        D2D1_POINT_2F startPoint = { 30.0f, 40.0f };

        path.BeginFigure(startPoint, D2D1_FIGURE_BEGIN_HOLLOW);

        auto& vType = path.DbgGetTypeList();

        // 验证类型标志
        Assert::IsTrue(HasFlag(vType[0], PathType::FgBegin), L"Should have FgBegin flag");
        Assert::IsFalse(HasFlag(vType[0], PathType::FgFill), L"Should NOT have FgFill flag");
    }

    TEST_METHOD(TestAddLines_NoSegmentFlags)
    {
        CGeometryPath path;
        D2D1_POINT_2F startPoint = { 0.0f, 0.0f };
        path.BeginFigure(startPoint, D2D1_FIGURE_BEGIN_FILLED);

        D2D1_POINT_2F points[] = {
            { 10.0f, 10.0f },
            { 20.0f, 20.0f },
            { 30.0f, 30.0f }
        };

        path.AddLines(points, 3);

        auto& vPoint = path.DbgGetPointList();
        auto& vType = path.DbgGetTypeList();

        Assert::AreEqual((size_t)4, vPoint.Size(), L"Should have 4 points total (1 start + 3 lines)");
        AssertPointTypeCountMatch(path);

        // 验证所有添加的线段都有GmLine标志
        for (size_t i = 1; i < vType.Size(); ++i)
        {
            Assert::IsTrue(HasFlag(vType[i], PathType::GmLine), L"Line segment should have GmLine flag");
        }

        // 验证点坐标
        Assert::AreEqual(10.0f, vPoint[1].x);
        Assert::AreEqual(20.0f, vPoint[2].x);
        Assert::AreEqual(30.0f, vPoint[3].x);
    }

    TEST_METHOD(TestAddLines_WithUnstrokedSegmentFlag)
    {
        CGeometryPath path;
        D2D1_POINT_2F startPoint = { 0.0f, 0.0f };
        path.BeginFigure(startPoint, D2D1_FIGURE_BEGIN_FILLED);

        path.SetSegmentFlags(D2D1_PATH_SEGMENT_FORCE_UNSTROKED);

        D2D1_POINT_2F points[] = { { 10.0f, 10.0f } };
        path.AddLines(points, 1);

        auto& vType = path.DbgGetTypeList();

        // 验证段标志
        Assert::IsTrue(HasFlag(vType[1], PathType::GmLine), L"Should have GmLine flag");
        Assert::IsTrue(HasFlag(vType[1], PathType::SgUnstroked), L"Should have SgUnstroked flag");
    }

    TEST_METHOD(TestAddLines_WithRoundJoinSegmentFlag)
    {
        CGeometryPath path;
        D2D1_POINT_2F startPoint = { 0.0f, 0.0f };
        path.BeginFigure(startPoint, D2D1_FIGURE_BEGIN_FILLED);

        path.SetSegmentFlags(D2D1_PATH_SEGMENT_FORCE_ROUND_LINE_JOIN);

        D2D1_POINT_2F points[] = { { 10.0f, 10.0f } };
        path.AddLines(points, 1);

        auto& vType = path.DbgGetTypeList();

        // 验证段标志
        Assert::IsTrue(HasFlag(vType[1], PathType::GmLine), L"Should have GmLine flag");
        Assert::IsTrue(HasFlag(vType[1], PathType::SgRoundJoin), L"Should have SgRoundJoin flag");
    }

    TEST_METHOD(TestAddBeziers)
    {
        CGeometryPath path;
        D2D1_POINT_2F startPoint = { 0.0f, 0.0f };
        path.BeginFigure(startPoint, D2D1_FIGURE_BEGIN_FILLED);

        D2D1_BEZIER_SEGMENT beziers[] = {
            { { 10.0f, 10.0f }, { 20.0f, 20.0f }, { 30.0f, 30.0f } },
            { { 40.0f, 40.0f }, { 50.0f, 50.0f }, { 60.0f, 60.0f } }
        };

        path.AddBeziers(beziers, 2);

        auto& vPoint = path.DbgGetPointList();
        auto& vType = path.DbgGetTypeList();

        // 1个起点 + 2个贝塞尔曲线(每个3个点) = 7个点
        Assert::AreEqual((size_t)7, vPoint.Size(), L"Should have 7 points total");
        AssertPointTypeCountMatch(path);

        // 验证所有贝塞尔曲线点都有GmBezier标志
        for (size_t i = 1; i < vType.Size(); ++i)
        {
            Assert::IsTrue(HasFlag(vType[i], PathType::GmBezier), L"Bezier point should have GmBezier flag");
        }

        // 验证点坐标
        Assert::AreEqual(10.0f, vPoint[1].x);
        Assert::AreEqual(30.0f, vPoint[3].x);
        Assert::AreEqual(60.0f, vPoint[6].x);
    }

    TEST_METHOD(TestAddBeziers_WithSegmentFlags)
    {
        CGeometryPath path;
        D2D1_POINT_2F startPoint = { 0.0f, 0.0f };
        path.BeginFigure(startPoint, D2D1_FIGURE_BEGIN_FILLED);

        path.SetSegmentFlags(D2D1_PATH_SEGMENT_FORCE_UNSTROKED);

        D2D1_BEZIER_SEGMENT beziers[] = {
            { { 10.0f, 10.0f }, { 20.0f, 20.0f }, { 30.0f, 30.0f } }
        };

        path.AddBeziers(beziers, 1);

        auto& vType = path.DbgGetTypeList();

        // 验证所有3个贝塞尔曲线点都有正确的标志
        for (size_t i = 1; i <= 3; ++i)
        {
            Assert::IsTrue(HasFlag(vType[i], PathType::GmBezier), L"Should have GmBezier flag");
            Assert::IsTrue(HasFlag(vType[i], PathType::SgUnstroked), L"Should have SgUnstroked flag");
        }
    }

    TEST_METHOD(TestEndFigure_Open)
    {
        CGeometryPath path;
        D2D1_POINT_2F startPoint = { 0.0f, 0.0f };
        path.BeginFigure(startPoint, D2D1_FIGURE_BEGIN_FILLED);

        D2D1_POINT_2F points[] = { { 10.0f, 10.0f } };
        path.AddLines(points, 1);

        path.EndFigure(D2D1_FIGURE_END_OPEN);

        auto& vType = path.DbgGetTypeList();

        // 最后一个点应该有FgEnd标志但没有FgClose标志
        Assert::IsTrue(HasFlag(vType.Back(), PathType::FgEnd), L"Last point should have FgEnd flag");
        Assert::IsFalse(HasFlag(vType.Back(), PathType::FgClose), L"Last point should NOT have FgClose flag");
    }

    TEST_METHOD(TestEndFigure_Closed)
    {
        CGeometryPath path;
        D2D1_POINT_2F startPoint = { 0.0f, 0.0f };
        path.BeginFigure(startPoint, D2D1_FIGURE_BEGIN_FILLED);

        D2D1_POINT_2F points[] = { { 10.0f, 10.0f } };
        path.AddLines(points, 1);

        path.EndFigure(D2D1_FIGURE_END_CLOSED);

        auto& vType = path.DbgGetTypeList();

        // 最后一个点应该同时有FgEnd和FgClose标志
        Assert::IsTrue(HasFlag(vType.Back(), PathType::FgEnd), L"Last point should have FgEnd flag");
        Assert::IsTrue(HasFlag(vType.Back(), PathType::FgClose), L"Last point should have FgClose flag");
    }

    TEST_METHOD(TestComplexPath_MixedSegments)
    {
        CGeometryPath path;

        // 第一个子图形：实心、闭合
        path.BeginFigure({ 0.0f, 0.0f }, D2D1_FIGURE_BEGIN_FILLED);
        D2D1_POINT_2F lines1[] = { { 10.0f, 0.0f }, { 10.0f, 10.0f }, { 0.0f, 10.0f } };
        path.AddLines(lines1, 3);
        path.EndFigure(D2D1_FIGURE_END_CLOSED);

        // 第二个子图形：空心、开放
        path.BeginFigure({ 20.0f, 20.0f }, D2D1_FIGURE_BEGIN_HOLLOW);
        D2D1_BEZIER_SEGMENT bezier = { { 25.0f, 20.0f }, { 30.0f, 25.0f }, { 30.0f, 30.0f } };
        path.AddBeziers(&bezier, 1);
        path.EndFigure(D2D1_FIGURE_END_OPEN);

        path.Close();

        auto& vPoint = path.DbgGetPointList();
        auto& vType = path.DbgGetTypeList();

        AssertPointTypeCountMatch(path);

        // 验证第一个子图形的起点
        Assert::IsTrue(HasFlag(vType[0], PathType::FgBegin), L"First figure start should have FgBegin");
        Assert::IsTrue(HasFlag(vType[0], PathType::FgFill), L"First figure should be filled");

        // 验证第一个子图形的终点
        Assert::IsTrue(HasFlag(vType[3], PathType::FgEnd), L"First figure end should have FgEnd");
        Assert::IsTrue(HasFlag(vType[3], PathType::FgClose), L"First figure should be closed");

        // 验证第二个子图形的起点
        Assert::IsTrue(HasFlag(vType[4], PathType::FgBegin), L"Second figure start should have FgBegin");
        Assert::IsFalse(HasFlag(vType[4], PathType::FgFill), L"Second figure should be hollow");

        // 验证第二个子图形包含贝塞尔曲线
        Assert::IsTrue(HasFlag(vType[5], PathType::GmBezier), L"Should have bezier segment");

        // 验证第二个子图形的终点
        Assert::IsTrue(HasFlag(vType[7], PathType::FgEnd), L"Second figure end should have FgEnd");
        Assert::IsFalse(HasFlag(vType[7], PathType::FgClose), L"Second figure should be open");
    }

    TEST_METHOD(TestOffsetTransform)
    {
        CGeometryPath path;

        // 启用偏移
        path.GstEnableOffset(TRUE);
        path.GstSetOffset(5.0f, 10.0f);

        BOOL isEnabled = FALSE;
        path.GstIsOffsetEnabled(&isEnabled);
        Assert::IsTrue(isEnabled, L"Offset should be enabled");

        // 添加点
        path.BeginFigure({ 0.0f, 0.0f }, D2D1_FIGURE_BEGIN_FILLED);
        D2D1_POINT_2F points[] = { { 10.0f, 20.0f } };
        path.AddLines(points, 1);

        auto& vPoint = path.DbgGetPointList();

        // 验证点已被偏移
        Assert::AreEqual(5.0f, vPoint[0].x, 0.001f, L"Start X should be offset");
        Assert::AreEqual(10.0f, vPoint[0].y, 0.001f, L"Start Y should be offset");
        Assert::AreEqual(15.0f, vPoint[1].x, 0.001f, L"Line X should be offset");
        Assert::AreEqual(30.0f, vPoint[1].y, 0.001f, L"Line Y should be offset");
    }

    TEST_METHOD(TestOffsetTransform_Bezier)
    {
        CGeometryPath path;

        // 启用偏移
        path.GstEnableOffset(TRUE);
        path.GstSetOffset(2.0f, 3.0f);

        path.BeginFigure({ 0.0f, 0.0f }, D2D1_FIGURE_BEGIN_FILLED);

        D2D1_BEZIER_SEGMENT bezier = { { 10.0f, 10.0f }, { 20.0f, 20.0f }, { 30.0f, 30.0f } };
        path.AddBeziers(&bezier, 1);

        auto& vPoint = path.DbgGetPointList();

        // 验证贝塞尔曲线的所有控制点都被偏移
        Assert::AreEqual(12.0f, vPoint[1].x, 0.001f, L"Bezier control point 1 X should be offset");
        Assert::AreEqual(13.0f, vPoint[1].y, 0.001f, L"Bezier control point 1 Y should be offset");
        Assert::AreEqual(22.0f, vPoint[2].x, 0.001f, L"Bezier control point 2 X should be offset");
        Assert::AreEqual(23.0f, vPoint[2].y, 0.001f, L"Bezier control point 2 Y should be offset");
        Assert::AreEqual(32.0f, vPoint[3].x, 0.001f, L"Bezier end point X should be offset");
        Assert::AreEqual(33.0f, vPoint[3].y, 0.001f, L"Bezier end point Y should be offset");
    }

    TEST_METHOD(TestSegmentFlags_Multiple)
    {
        CGeometryPath path;
        path.BeginFigure({ 0.0f, 0.0f }, D2D1_FIGURE_BEGIN_FILLED);

        // 添加默认段
        path.SetSegmentFlags(D2D1_PATH_SEGMENT_NONE);
        D2D1_POINT_2F pt1[] = { { 10.0f, 10.0f } };
        path.AddLines(pt1, 1);

        // 添加不描边段
        path.SetSegmentFlags(D2D1_PATH_SEGMENT_FORCE_UNSTROKED);
        D2D1_POINT_2F pt2[] = { { 20.0f, 20.0f } };
        path.AddLines(pt2, 1);

        // 添加圆角联接段
        path.SetSegmentFlags(D2D1_PATH_SEGMENT_FORCE_ROUND_LINE_JOIN);
        D2D1_POINT_2F pt3[] = { { 30.0f, 30.0f } };
        path.AddLines(pt3, 1);

        auto& vType = path.DbgGetTypeList();

        // 验证第一个段（默认）
        Assert::IsFalse(HasFlag(vType[1], PathType::SgUnstroked), L"First segment should not have SgUnstroked");
        Assert::IsFalse(HasFlag(vType[1], PathType::SgRoundJoin), L"First segment should not have SgRoundJoin");

        // 验证第二个段（不描边）
        Assert::IsTrue(HasFlag(vType[2], PathType::SgUnstroked), L"Second segment should have SgUnstroked");

        // 验证第三个段（圆角联接）
        Assert::IsTrue(HasFlag(vType[3], PathType::SgRoundJoin), L"Third segment should have SgRoundJoin");
    }

    TEST_METHOD(TestEmptyFigure_ShouldBeIgnored)
    {
        CGeometryPath path;

        // 创建一个空子图形（只有起点，没有其他点）
        path.BeginFigure({ 0.0f, 0.0f }, D2D1_FIGURE_BEGIN_FILLED);
        path.EndFigure(D2D1_FIGURE_END_CLOSED);

        auto& vPoint = path.DbgGetPointList();
        auto& vType = path.DbgGetTypeList();

        // 空子图形应该只有起点，且不添加FgEnd标志
        Assert::AreEqual((size_t)1, vPoint.Size(), L"Empty figure should only have start point");
        Assert::IsFalse(HasFlag(vType[0], PathType::FgEnd), L"Empty figure should not have FgEnd flag");
    }

    TEST_METHOD(TestMultipleFigures)
    {
        CGeometryPath path;

        // 第一个子图形
        path.BeginFigure({ 0.0f, 0.0f }, D2D1_FIGURE_BEGIN_FILLED);
        D2D1_POINT_2F pts1[] = { { 10.0f, 0.0f } };
        path.AddLines(pts1, 1);
        path.EndFigure(D2D1_FIGURE_END_CLOSED);

        // 第二个子图形
        path.BeginFigure({ 20.0f, 20.0f }, D2D1_FIGURE_BEGIN_HOLLOW);
        D2D1_POINT_2F pts2[] = { { 30.0f, 20.0f } };
        path.AddLines(pts2, 1);
        path.EndFigure(D2D1_FIGURE_END_OPEN);

        // 第三个子图形
        path.BeginFigure({ 40.0f, 40.0f }, D2D1_FIGURE_BEGIN_FILLED);
        D2D1_POINT_2F pts3[] = { { 50.0f, 40.0f } };
        path.AddLines(pts3, 1);
        path.EndFigure(D2D1_FIGURE_END_CLOSED);

        path.Close();

        auto& vType = path.DbgGetTypeList();

        // 应该有3个BeginFigure和3个EndFigure
        int beginCount = 0;
        int endCount = 0;
        for (size_t i = 0; i < vType.Size(); ++i)
        {
            if (HasFlag(vType[i], PathType::FgBegin))
                beginCount++;
            if (HasFlag(vType[i], PathType::FgEnd))
                endCount++;
        }

        Assert::AreEqual(3, beginCount, L"Should have 3 figure begins");
        Assert::AreEqual(3, endCount, L"Should have 3 figure ends");
    }

    TEST_METHOD(TsFlattenBezierFlags)
    {

    }
};
TS_NS_END