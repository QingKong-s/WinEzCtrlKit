#include "pch.h"
#include "../eck/Utility.h"

using namespace eck;

TS_NS_BEGIN
TEST_CLASS(TsRectOpRECT)
{
public:
    TEST_METHOD(TsIsRectsIntersect_RECT)
    {
        RECT rc1 = { 0, 0, 10, 10 };
        RECT rc2 = { 5, 5, 15, 15 };
        RECT rc3 = { 20, 20, 30, 30 };

        Assert::IsTrue(IsRectsIntersect(rc1, rc2)); // 相交
        Assert::IsFalse(IsRectsIntersect(rc1, rc3)); // 不相交

        // 边界接触（不算相交）
        RECT rc4 = { 10, 0, 20, 10 };
        Assert::IsFalse(IsRectsIntersect(rc1, rc4));
    }

    TEST_METHOD(TsInflateRect_RECT)
    {
        RECT rc = { 10, 10, 20, 20 };
        InflateRect(rc, 5, 3);

        Assert::AreEqual(5L, rc.left);
        Assert::AreEqual(7L, rc.top);
        Assert::AreEqual(25L, rc.right);
        Assert::AreEqual(23L, rc.bottom);
    }

    TEST_METHOD(TsIntersectRect_RECT)
    {
        RECT rc1 = { 0, 0, 10, 10 };
        RECT rc2 = { 5, 5, 15, 15 };
        RECT rcDst;

        BOOL result = IntersectRect(rcDst, rc1, rc2);
        Assert::IsTrue(result);
        Assert::AreEqual(5L, rcDst.left);
        Assert::AreEqual(5L, rcDst.top);
        Assert::AreEqual(10L, rcDst.right);
        Assert::AreEqual(10L, rcDst.bottom);

        // 不相交的情况
        RECT rc3 = { 20, 20, 30, 30 };
        result = IntersectRect(rcDst, rc1, rc3);
        Assert::IsFalse(result);
    }

    TEST_METHOD(TsOffsetRect_RECT)
    {
        RECT rc = { 10, 10, 20, 20 };
        OffsetRect(rc, 5, -3);

        Assert::AreEqual(15L, rc.left);
        Assert::AreEqual(7L, rc.top);
        Assert::AreEqual(25L, rc.right);
        Assert::AreEqual(17L, rc.bottom);
    }

    TEST_METHOD(TsIsRectEmpty_RECT)
    {
        RECT rc1 = { 10, 10, 20, 20 };
        RECT rc2 = { 10, 10, 10, 20 }; // 宽度为0
        RECT rc3 = { 10, 10, 20, 10 }; // 高度为0
        RECT rc4 = { 20, 20, 10, 10 }; // 反向

        Assert::IsFalse(IsRectEmpty(rc1));
        Assert::IsTrue(IsRectEmpty(rc2));
        Assert::IsTrue(IsRectEmpty(rc3));
        Assert::IsTrue(IsRectEmpty(rc4));
    }

    TEST_METHOD(TsUnionRect_RECT)
    {
        RECT rc1 = { 0, 0, 10, 10 };
        RECT rc2 = { 5, 5, 15, 15 };
        RECT rcDst;

        UnionRect(rcDst, rc1, rc2);
        Assert::AreEqual(0L, rcDst.left);
        Assert::AreEqual(0L, rcDst.top);
        Assert::AreEqual(15L, rcDst.right);
        Assert::AreEqual(15L, rcDst.bottom);

        // 一个空矩形
        RECT rcEmpty = { 0, 0, 0, 0 };
        UnionRect(rcDst, rc1, rcEmpty);
        Assert::AreEqual(rc1.left, rcDst.left);
        Assert::AreEqual(rc1.top, rcDst.top);
        Assert::AreEqual(rc1.right, rcDst.right);
        Assert::AreEqual(rc1.bottom, rcDst.bottom);
    }

    TEST_METHOD(TsIsRectInclude_RECT)
    {
        RECT rcOuter = { 0, 0, 20, 20 };
        RECT rcInner = { 5, 5, 15, 15 };
        RECT rcPartial = { 5, 5, 25, 15 };

        Assert::IsTrue(IsRectInclude(rcOuter, rcInner)); // 包含
        Assert::IsFalse(IsRectInclude(rcOuter, rcPartial)); // 部分包含
        Assert::IsTrue(IsRectInclude(rcOuter, rcOuter)); // 自己包含自己
    }

    TEST_METHOD(TsAdjustRectIntoAnother_RECT)
    {
        RECT rc = { -5, -5, 15, 15 };
        RECT rcRef = { 0, 0, 100, 100 };

        BOOL result = AdjustRectIntoAnother(rc, rcRef);
        Assert::IsTrue(result);
        Assert::IsTrue(rc.left >= rcRef.left);
        Assert::IsTrue(rc.top >= rcRef.top);
        Assert::IsTrue(rc.right <= rcRef.right);
        Assert::IsTrue(rc.bottom <= rcRef.bottom);

        // 矩形太大无法调整
        RECT rcTooBig = { 0, 0, 200, 50 };
        result = AdjustRectIntoAnother(rcTooBig, rcRef);
        Assert::IsFalse(result);
    }

    TEST_METHOD(TsAdjustRectToFitAnother_RECT)
    {
        RECT rc = { 0, 0, 200, 100 };
        RECT rcRef = { 0, 0, 100, 100 };

        BOOL result = AdjustRectToFitAnother(rc, rcRef);
        Assert::IsTrue(result);

        int width = rc.right - rc.left;
        int height = rc.bottom - rc.top;
        Assert::AreEqual(100, width); // x对齐
        Assert::AreEqual(50, height);
    }

    TEST_METHOD(TsAdjustRectToFillAnother_RECT)
    {
        RECT rc = { 0, 0, 100, 50 };
        RECT rcRef = { 0, 0, 100, 100 };

        BOOL result = AdjustRectToFillAnother(rc, rcRef);
        Assert::IsTrue(result);

        int width = rc.right - rc.left;
        int height = rc.bottom - rc.top;
        Assert::AreEqual(200, width); // y对齐，需要放大到覆盖
        Assert::AreEqual(100, height);
    }

    TEST_METHOD(TsCenterRect_RECT)
    {
        RECT rc = { 0, 0, 20, 10 };
        RECT rcRef = { 0, 0, 100, 100 };

        CenterRect(rc, rcRef);

        Assert::AreEqual(40L, rc.left); // (100-20)/2
        Assert::AreEqual(45L, rc.top);  // (100-10)/2
        Assert::AreEqual(60L, rc.right);
        Assert::AreEqual(55L, rc.bottom);
    }
};

TEST_CLASS(TsRectOpRCWH)
{
public:
    TEST_METHOD(TsIsRectsIntersect_RCWH)
    {
        RCWH rc1 = { 0, 0, 10, 10 };
        RCWH rc2 = { 5, 5, 10, 10 };
        RCWH rc3 = { 20, 20, 10, 10 };

        Assert::IsTrue(IsRectsIntersect(rc1, rc2));
        Assert::IsFalse(IsRectsIntersect(rc1, rc3));

        // 边界接触
        RCWH rc4 = { 10, 0, 10, 10 };
        Assert::IsFalse(IsRectsIntersect(rc1, rc4));
    }

    TEST_METHOD(TsInflateRect_RCWH)
    {
        RCWH rc = { 10, 10, 10, 10 };
        InflateRect(rc, 5, 3);

        Assert::AreEqual(5, rc.x);
        Assert::AreEqual(7, rc.y);
        Assert::AreEqual(20, rc.cx);
        Assert::AreEqual(16, rc.cy);
    }

    TEST_METHOD(TsIntersectRect_RCWH)
    {
        RCWH rc1 = { 0, 0, 10, 10 };
        RCWH rc2 = { 5, 5, 10, 10 };
        RCWH rcDst;

        BOOL result = IntersectRect(rcDst, rc1, rc2);
        Assert::IsTrue(result);
        Assert::AreEqual(5, rcDst.x);
        Assert::AreEqual(5, rcDst.y);
        Assert::AreEqual(5, rcDst.cx);
        Assert::AreEqual(5, rcDst.cy);

        // 不相交
        RCWH rc3 = { 20, 20, 10, 10 };
        result = IntersectRect(rcDst, rc1, rc3);
        Assert::IsFalse(result);
    }

    TEST_METHOD(TsOffsetRect_RCWH)
    {
        RCWH rc = { 10, 10, 10, 10 };
        OffsetRect(rc, 5, -3);

        Assert::AreEqual(15, rc.x);
        Assert::AreEqual(7, rc.y);
        Assert::AreEqual(10, rc.cx);
        Assert::AreEqual(10, rc.cy);
    }

    TEST_METHOD(TsIsRectEmpty_RCWH)
    {
        RCWH rc1 = { 10, 10, 10, 10 };
        RCWH rc2 = { 10, 10, 0, 10 };
        RCWH rc3 = { 10, 10, 10, 0 };
        RCWH rc4 = { 10, 10, -5, 10 };

        Assert::IsFalse(IsRectEmpty(rc1));
        Assert::IsTrue(IsRectEmpty(rc2));
        Assert::IsTrue(IsRectEmpty(rc3));
        Assert::IsTrue(IsRectEmpty(rc4));
    }

    TEST_METHOD(TsUnionRect_RCWH)
    {
        RCWH rc1 = { 0, 0, 10, 10 };
        RCWH rc2 = { 5, 5, 10, 10 };
        RCWH rcDst;

        UnionRect(rcDst, rc1, rc2);
        Assert::AreEqual(0, rcDst.x);
        Assert::AreEqual(0, rcDst.y);
        Assert::AreEqual(15, rcDst.cx);
        Assert::AreEqual(15, rcDst.cy);

        // 空矩形
        RCWH rcEmpty = { 0, 0, 0, 0 };
        UnionRect(rcDst, rc1, rcEmpty);
        Assert::IsTrue(rcDst == rc1);
    }

    TEST_METHOD(TsIsRectInclude_RCWH)
    {
        RCWH rcOuter = { 0, 0, 20, 20 };
        RCWH rcInner = { 5, 5, 10, 10 };
        RCWH rcPartial = { 5, 5, 20, 10 };

        Assert::IsTrue(IsRectInclude(rcOuter, rcInner));
        Assert::IsFalse(IsRectInclude(rcOuter, rcPartial));
        Assert::IsTrue(IsRectInclude(rcOuter, rcOuter));
    }

    TEST_METHOD(TsAdjustRectIntoAnother_RCWH)
    {
        RCWH rc = { -5, -5, 20, 20 };
        RCWH rcRef = { 0, 0, 100, 100 };

        BOOL result = AdjustRectIntoAnother(rc, rcRef);
        Assert::IsTrue(result);
        Assert::IsTrue(rc.x >= rcRef.x);
        Assert::IsTrue(rc.y >= rcRef.y);
        Assert::IsTrue(rc.x + rc.cx <= rcRef.x + rcRef.cx);
        Assert::IsTrue(rc.y + rc.cy <= rcRef.y + rcRef.cy);
    }

    TEST_METHOD(TsAdjustRectToFitAnother_RCWH)
    {
        RCWH rc = { 0, 0, 200, 100 };
        RCWH rcRef = { 0, 0, 100, 100 };

        BOOL result = AdjustRectToFitAnother(rc, rcRef);
        Assert::IsTrue(result);
        Assert::AreEqual(100, rc.cx);
        Assert::AreEqual(50, rc.cy);
    }

    TEST_METHOD(TsAdjustRectToFillAnother_RCWH)
    {
        RCWH rc = { 0, 0, 100, 50 };
        RCWH rcRef = { 0, 0, 100, 100 };

        BOOL result = AdjustRectToFillAnother(rc, rcRef);
        Assert::IsTrue(result);
        Assert::AreEqual(200, rc.cx);
        Assert::AreEqual(100, rc.cy);
    }

    TEST_METHOD(TsCenterRect_RCWH)
    {
        RCWH rc = { 0, 0, 20, 10 };
        RCWH rcRef = { 0, 0, 100, 100 };

        CenterRect(rc, rcRef);

        Assert::AreEqual(40, rc.x);
        Assert::AreEqual(45, rc.y);
        Assert::AreEqual(20, rc.cx);
        Assert::AreEqual(10, rc.cy);
    }
};

TEST_CLASS(TsRectOpD2DRECT)
{
public:
    TEST_METHOD(TsIsRectsIntersect_D2D1_RECT_F)
    {
        D2D1_RECT_F rc1 = { 0.0f, 0.0f, 10.0f, 10.0f };
        D2D1_RECT_F rc2 = { 5.0f, 5.0f, 15.0f, 15.0f };
        D2D1_RECT_F rc3 = { 20.0f, 20.0f, 30.0f, 30.0f };

        Assert::IsTrue(IsRectsIntersect(rc1, rc2));
        Assert::IsFalse(IsRectsIntersect(rc1, rc3));

        // 边界接触
        D2D1_RECT_F rc4 = { 10.0f, 0.0f, 20.0f, 10.0f };
        Assert::IsFalse(IsRectsIntersect(rc1, rc4));
    }

    TEST_METHOD(TsInflateRect_D2D1_RECT_F)
    {
        D2D1_RECT_F rc = { 10.0f, 10.0f, 20.0f, 20.0f };
        InflateRect(rc, 5.0f, 3.0f);

        Assert::AreEqual(5.0f, rc.left, 0.001f);
        Assert::AreEqual(7.0f, rc.top, 0.001f);
        Assert::AreEqual(25.0f, rc.right, 0.001f);
        Assert::AreEqual(23.0f, rc.bottom, 0.001f);
    }

    TEST_METHOD(TsIntersectRect_D2D1_RECT_F)
    {
        D2D1_RECT_F rc1 = { 0.0f, 0.0f, 10.0f, 10.0f };
        D2D1_RECT_F rc2 = { 5.0f, 5.0f, 15.0f, 15.0f };
        D2D1_RECT_F rcDst;

        BOOL result = IntersectRect(rcDst, rc1, rc2);
        Assert::IsTrue(result);
        Assert::AreEqual(5.0f, rcDst.left, 0.001f);
        Assert::AreEqual(5.0f, rcDst.top, 0.001f);
        Assert::AreEqual(10.0f, rcDst.right, 0.001f);
        Assert::AreEqual(10.0f, rcDst.bottom, 0.001f);

        // 不相交
        D2D1_RECT_F rc3 = { 20.0f, 20.0f, 30.0f, 30.0f };
        result = IntersectRect(rcDst, rc1, rc3);
        Assert::IsFalse(result);
    }

    TEST_METHOD(TsOffsetRect_D2D1_RECT_F)
    {
        D2D1_RECT_F rc = { 10.0f, 10.0f, 20.0f, 20.0f };
        OffsetRect(rc, 5.5f, -3.5f);

        Assert::AreEqual(15.5f, rc.left, 0.001f);
        Assert::AreEqual(6.5f, rc.top, 0.001f);
        Assert::AreEqual(25.5f, rc.right, 0.001f);
        Assert::AreEqual(16.5f, rc.bottom, 0.001f);
    }

    TEST_METHOD(TsIsRectEmpty_D2D1_RECT_F)
    {
        D2D1_RECT_F rc1 = { 10.0f, 10.0f, 20.0f, 20.0f };
        D2D1_RECT_F rc2 = { 10.0f, 10.0f, 10.0f, 20.0f };
        D2D1_RECT_F rc3 = { 10.0f, 10.0f, 20.0f, 10.0f };
        D2D1_RECT_F rc4 = { 20.0f, 20.0f, 10.0f, 10.0f };

        Assert::IsFalse(IsRectEmpty(rc1));
        Assert::IsTrue(IsRectEmpty(rc2));
        Assert::IsTrue(IsRectEmpty(rc3));
        Assert::IsTrue(IsRectEmpty(rc4));
    }

    TEST_METHOD(TsUnionRect_D2D1_RECT_F)
    {
        D2D1_RECT_F rc1 = { 0.0f, 0.0f, 10.0f, 10.0f };
        D2D1_RECT_F rc2 = { 5.0f, 5.0f, 15.0f, 15.0f };
        D2D1_RECT_F rcDst;

        UnionRect(rcDst, rc1, rc2);
        Assert::AreEqual(0.0f, rcDst.left, 0.001f);
        Assert::AreEqual(0.0f, rcDst.top, 0.001f);
        Assert::AreEqual(15.0f, rcDst.right, 0.001f);
        Assert::AreEqual(15.0f, rcDst.bottom, 0.001f);
    }

    TEST_METHOD(TsIsRectInclude_D2D1_RECT_F)
    {
        D2D1_RECT_F rcOuter = { 0.0f, 0.0f, 20.0f, 20.0f };
        D2D1_RECT_F rcInner = { 5.0f, 5.0f, 15.0f, 15.0f };
        D2D1_RECT_F rcPartial = { 5.0f, 5.0f, 25.0f, 15.0f };

        Assert::IsTrue(IsRectInclude(rcOuter, rcInner));
        Assert::IsFalse(IsRectInclude(rcOuter, rcPartial));
    }

    TEST_METHOD(TsAdjustRectIntoAnother_D2D1_RECT_F)
    {
        D2D1_RECT_F rc = { -5.0f, -5.0f, 15.0f, 15.0f };
        D2D1_RECT_F rcRef = { 0.0f, 0.0f, 100.0f, 100.0f };

        BOOL result = AdjustRectIntoAnother(rc, rcRef);
        Assert::IsTrue(result);
        Assert::IsTrue(rc.left >= rcRef.left);
        Assert::IsTrue(rc.top >= rcRef.top);
        Assert::IsTrue(rc.right <= rcRef.right);
        Assert::IsTrue(rc.bottom <= rcRef.bottom);
    }

    TEST_METHOD(TsAdjustRectToFitAnother_D2D1_RECT_F)
    {
        D2D1_RECT_F rc = { 0.0f, 0.0f, 200.0f, 100.0f };
        D2D1_RECT_F rcRef = { 0.0f, 0.0f, 100.0f, 100.0f };

        BOOL result = AdjustRectToFitAnother(rc, rcRef);
        Assert::IsTrue(result);

        float width = rc.right - rc.left;
        float height = rc.bottom - rc.top;
        Assert::AreEqual(100.0f, width, 0.1f);
        Assert::AreEqual(50.0f, height, 0.1f);
    }

    TEST_METHOD(TsAdjustRectToFillAnother_D2D1_RECT_F)
    {
        D2D1_RECT_F rc = { 0.0f, 0.0f, 100.0f, 50.0f };
        D2D1_RECT_F rcRef = { 0.0f, 0.0f, 100.0f, 100.0f };

        BOOL result = AdjustRectToFillAnother(rc, rcRef);
        Assert::IsTrue(result);

        float width = rc.right - rc.left;
        float height = rc.bottom - rc.top;
        Assert::AreEqual(200.0f, width, 0.1f);
        Assert::AreEqual(100.0f, height, 0.1f);
    }

    TEST_METHOD(TsCenterRect_D2D1_RECT_F)
    {
        D2D1_RECT_F rc = { 0.0f, 0.0f, 20.0f, 10.0f };
        D2D1_RECT_F rcRef = { 0.0f, 0.0f, 100.0f, 100.0f };

        CenterRect(rc, rcRef);

        Assert::AreEqual(40.0f, rc.left, 0.001f);
        Assert::AreEqual(45.0f, rc.top, 0.001f);
        Assert::AreEqual(60.0f, rc.right, 0.001f);
        Assert::AreEqual(55.0f, rc.bottom, 0.001f);
    }
};
TS_NS_END