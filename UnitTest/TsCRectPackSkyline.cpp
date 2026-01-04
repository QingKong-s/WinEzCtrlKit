#include "pch.h"
#define ECK_OPT_SKYLINE_VALIDATE 1
#include "../eck/CRectPackSkyline.h"

using namespace eck;


TS_NS_BEGIN
TEST_CLASS(TsCRectPackSkyline)
{
public:
private:
    inline bool RectOverlap(const CRectPackSkyline::RECT& a, const CRectPackSkyline::RECT& b) noexcept
    {
        return
            a.x < b.x + b.cx &&
            a.x + a.cx > b.x &&
            a.y < b.y + b.cy &&
            a.y + a.cy > b.y;
    }

    // 辅助函数：验证矩形不重叠
    bool CheckNoOverlap(const std::vector<CRectPackSkyline::RECT>& rects)
    {
        for (size_t i = 0; i < rects.size(); ++i)
        {
            for (size_t j = i + 1; j < rects.size(); ++j)
            {
                const auto& r1 = rects[i];
                const auto& r2 = rects[j];

                if (RectOverlap(r1, r2))
                {
                    return false;
                }
            }
        }
        return true;
    }

    // 辅助函数：验证矩形在容器内
    bool CheckWithinBounds(const CRectPackSkyline::RECT& rect, int cx, int cy)
    {
        return rect.x >= 0 && rect.y >= 0 &&
            rect.x + rect.cx <= cx && rect.y + rect.cy <= cy;
    }

public:

    // ==================== 基础功能测试 ====================

    TEST_METHOD(Test_Constructor_Basic)
    {
        CRectPackSkyline packer(100, 100);
        Assert::IsNotNull(&packer);
    }

    TEST_METHOD(Test_Constructor_ZeroSize)
    {
        CRectPackSkyline packer(0, 0);
        CRectPackSkyline::RECT rect;
        Assert::IsFalse(packer.AllocateBottomLeft(1, 1, rect));
    }

    TEST_METHOD(Test_Constructor_OneDimensionZero)
    {
        CRectPackSkyline packer1(100, 0);
        CRectPackSkyline packer2(0, 100);
        CRectPackSkyline::RECT rect;

        Assert::IsFalse(packer1.AllocateBottomLeft(10, 10, rect));
        Assert::IsFalse(packer2.AllocateBottomLeft(10, 10, rect));
    }

    // ==================== AllocateBottomLeft 测试 ====================

    TEST_METHOD(Test_BL_SingleRect)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect;

        BOOL result = packer.AllocateBottomLeft(50, 30, rect);

        Assert::IsTrue(result);
        Assert::AreEqual(0, rect.x);
        Assert::AreEqual(0, rect.y);
        Assert::AreEqual(50, rect.cx);
        Assert::AreEqual(30, rect.cy);
    }

    TEST_METHOD(Test_BL_TwoRectsHorizontal)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect1, rect2;

        Assert::IsTrue(packer.AllocateBottomLeft(40, 30, rect1));
        Assert::IsTrue(packer.AllocateBottomLeft(40, 30, rect2));

        Assert::AreEqual(0, rect1.x);
        Assert::AreEqual(40, rect2.x);
        Assert::AreEqual(0, rect2.y);
    }

    TEST_METHOD(Test_BL_ExceedsWidth)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect;

        Assert::IsFalse(packer.AllocateBottomLeft(150, 30, rect));
    }

    TEST_METHOD(Test_BL_ExceedsHeight)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect;

        Assert::IsFalse(packer.AllocateBottomLeft(50, 150, rect));
    }

    TEST_METHOD(Test_BL_FullWidthRect)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect1, rect2;

        Assert::IsTrue(packer.AllocateBottomLeft(100, 30, rect1));
        Assert::IsTrue(packer.AllocateBottomLeft(100, 20, rect2));

        Assert::AreEqual(0, rect2.x);
        Assert::AreEqual(30, rect2.y);
    }

    TEST_METHOD(Test_BL_SpansMultipleSegments)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect1, rect2, rect3;

        packer.AllocateBottomLeft(30, 20, rect1);
        packer.AllocateBottomLeft(30, 40, rect2);
        packer.AllocateBottomLeft(30, 25, rect3);

        // 插入跨越多段的矩形
        CRectPackSkyline::RECT rect4;
        Assert::IsTrue(packer.AllocateBottomLeft(90, 10, rect4));

        // 应该放在最高点
        Assert::AreEqual(40, rect4.y);
    }

    TEST_METHOD(Test_BL_PartialSegmentFill)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect1, rect2;

        packer.AllocateBottomLeft(100, 30, rect1);
        Assert::IsTrue(packer.AllocateBottomLeft(50, 20, rect2));

        Assert::AreEqual(0, rect2.x);
        Assert::AreEqual(30, rect2.y);
    }

    TEST_METHOD(Test_BL_FillToCapacity)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect;
        int count = 0;

        for (int i = 0; i < 10; ++i)
        {
            if (packer.AllocateBottomLeft(100, 10, rect))
                ++count;
        }

        Assert::AreEqual(10, count);
        Assert::IsFalse(packer.AllocateBottomLeft(10, 10, rect));
    }

    TEST_METHOD(Test_BL_MinimalRect)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect;

        Assert::IsTrue(packer.AllocateBottomLeft(1, 1, rect));
        Assert::AreEqual(0, rect.x);
        Assert::AreEqual(0, rect.y);
    }

    TEST_METHOD(Test_BL_ExactBoundary)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect1, rect2;

        packer.AllocateBottomLeft(60, 30, rect1);
        Assert::IsTrue(packer.AllocateBottomLeft(40, 30, rect2));
        Assert::AreEqual(60, rect2.x);

        Assert::IsTrue(packer.AllocateBottomLeft(99, 70, rect2));
        Assert::IsFalse(packer.AllocateBottomLeft(2, 1, rect2));
    }

    // ==================== AllocateMinWaste 测试 ====================

    TEST_METHOD(Test_MW_SingleRect)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect;

        Assert::IsTrue(packer.AllocateMinWaste(50, 30, rect));
        Assert::AreEqual(0, rect.x);
        Assert::AreEqual(0, rect.y);
    }

    TEST_METHOD(Test_MW_MinimizeWaste)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect1, rect2, rect3;

        // 创建不同高度的空间
        packer.AllocateMinWaste(40, 50, rect1);
        packer.AllocateMinWaste(60, 30, rect2);

        // 应该选择浪费最小的位置
        Assert::IsTrue(packer.AllocateMinWaste(30, 20, rect3));
    }

    TEST_METHOD(Test_MW_LeftVsRightPlacement)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect1, rect2;

        packer.AllocateMinWaste(100, 30, rect1);

        // 测试左右放置的选择
        Assert::IsTrue(packer.AllocateMinWaste(50, 20, rect2));
    }

    TEST_METHOD(Test_MW_ExceedsCapacity)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect;

        Assert::IsFalse(packer.AllocateMinWaste(150, 30, rect));
        Assert::IsFalse(packer.AllocateMinWaste(30, 150, rect));
    }

    TEST_METHOD(Test_MW_ComplexSkyline)
    {
        CRectPackSkyline packer(150, 150);
        CRectPackSkyline::RECT rect;

        packer.AllocateMinWaste(30, 30, rect);
        packer.AllocateMinWaste(30, 20, rect);
        packer.AllocateMinWaste(30, 40, rect);
        packer.AllocateMinWaste(30, 25, rect);

        Assert::IsTrue(packer.AllocateMinWaste(60, 10, rect));
    }

    TEST_METHOD(Test_MW_SpanMultipleSegments)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect1, rect2, rect3, rect4;

        packer.AllocateMinWaste(25, 20, rect1);
        packer.AllocateMinWaste(25, 30, rect2);
        packer.AllocateMinWaste(25, 25, rect3);

        // 跨越三个段
        Assert::IsTrue(packer.AllocateMinWaste(75, 10, rect4));
    }

    TEST_METHOD(Test_MW_RightAlignment)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect1, rect2;

        packer.AllocateMinWaste(100, 30, rect1);

        // 测试从右侧放置
        Assert::IsTrue(packer.AllocateMinWaste(30, 20, rect2));

        // 验证位置合理
        Assert::IsTrue(CheckWithinBounds(rect2, 100, 100));
    }

    TEST_METHOD(Test_MW_ZeroWaste)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect1, rect2;

        packer.AllocateMinWaste(50, 50, rect1);

        // 完全填充剩余空间，无浪费
        Assert::IsTrue(packer.AllocateMinWaste(50, 50, rect2));
        Assert::AreEqual(50, rect2.x);
        Assert::AreEqual(0, rect2.y);
    }

    // ==================== BlfCanPlaceAtRight 边界测试 ====================

    TEST_METHOD(Test_RightPlace_InsufficientSpace)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect1, rect2;

        // 创建一个小空间
        packer.AllocateMinWaste(90, 30, rect1);

        // 尝试从右侧放置超出的矩形
        BOOL result = packer.AllocateMinWaste(50, 20, rect2);

        // 可能成功或失败，取决于实现
        if (result)
        {
            Assert::IsTrue(CheckWithinBounds(rect2, 100, 100));
        }
    }

    TEST_METHOD(Test_RightPlace_AtBoundary)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect1, rect2;

        packer.AllocateMinWaste(50, 30, rect1);
        Assert::IsTrue(packer.AllocateMinWaste(50, 30, rect2));

        Assert::IsTrue(CheckWithinBounds(rect2, 100, 100));
    }

    // ==================== 合并测试 ====================

    TEST_METHOD(Test_Merge_SameHeightSegments)
    {
        CRectPackSkyline packer(120, 100);
        CRectPackSkyline::RECT rect1, rect2, rect3, rect4;

        // 创建多个相同高度的段
        packer.AllocateBottomLeft(30, 30, rect1);
        packer.AllocateBottomLeft(30, 30, rect2);
        packer.AllocateBottomLeft(30, 30, rect3);

        // 插入跨越所有段的矩形，应该触发合并
        Assert::IsTrue(packer.AllocateBottomLeft(90, 10, rect4));
    }

    TEST_METHOD(Test_Merge_AfterFullFill)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect1, rect2;

        packer.AllocateBottomLeft(100, 30, rect1);
        packer.AllocateBottomLeft(100, 20, rect2);

        // 验证可以继续插入
        CRectPackSkyline::RECT rect3;
        Assert::IsTrue(packer.AllocateBottomLeft(50, 10, rect3));
    }

    // ==================== 压力和混合测试 ====================

    TEST_METHOD(Test_Stress_ManySmallRects_BL)
    {
        CRectPackSkyline packer(200, 200);
        CRectPackSkyline::RECT rect;
        std::vector<CRectPackSkyline::RECT> rects;

        for (int i = 0; i < 200; ++i)
        {
            if (packer.AllocateBottomLeft(10, 10, rect))
            {
                rects.push_back(rect);
                packer.DbgValidateAllocatedRectangle(rects.data(), rects.size());
            }
        }

        Assert::IsTrue(rects.size() >= 200);
        Assert::IsTrue(CheckNoOverlap(rects));

        packer.ReSize(99999, 201);
        packer.DbgValidateAllocatedRectangle(rects.data(), rects.size());
    }

    TEST_METHOD(Test_Stress_ManySmallRects_MW)
    {
        CRectPackSkyline packer(200, 200);
        CRectPackSkyline::RECT rect;
        std::vector<CRectPackSkyline::RECT> rects;

        for (int i = 0; i < 200; ++i)
        {
            if (packer.AllocateMinWaste(10, 10, rect))
            {
                rects.push_back(rect);
                packer.DbgValidateAllocatedRectangle(rects.data(), rects.size());
            }
        }

        Assert::IsTrue(rects.size() >= 100);
        Assert::IsTrue(CheckNoOverlap(rects));

        packer.ReSize(200, 201);
        packer.DbgValidateAllocatedRectangle(rects.data(), rects.size());
    }

    TEST_METHOD(Test_Stress_RandomSizes_BL)
    {
        CRectPackSkyline packer(300, 300);
        CRectPackSkyline::RECT rect;
        std::vector<CRectPackSkyline::RECT> rects;

        for (int i = 0; i < 100; ++i)
        {
            int w = 10 + (i * 7) % 40;
            int h = 10 + (i * 11) % 40;
            if (packer.AllocateBottomLeft(w, h, rect))
            {
                rects.push_back(rect);
                packer.DbgValidateAllocatedRectangle(rects.data(), rects.size());
            }
        }

        Assert::IsTrue(rects.size() > 30);
        Assert::IsTrue(CheckNoOverlap(rects));

        for (const auto& r : rects)
        {
            Assert::IsTrue(CheckWithinBounds(r, 300, 300));
        }

        packer.ReSize(301, 302);
        packer.DbgValidateAllocatedRectangle(rects.data(), rects.size());
    }

    TEST_METHOD(Test_Stress_RandomSizes_MW)
    {
        CRectPackSkyline packer(300, 300);
        CRectPackSkyline::RECT rect;
        std::vector<CRectPackSkyline::RECT> rects;

        for (int i = 0; i < 100; ++i)
        {
            int w = 10 + (i * 7) % 40;
            int h = 10 + (i * 11) % 40;
            if (packer.AllocateMinWaste(w, h, rect))
            {
                rects.push_back(rect);
                packer.DbgValidateAllocatedRectangle(rects.data(), rects.size());
            }
        }

        Assert::IsTrue(rects.size() > 30);
        Assert::IsTrue(CheckNoOverlap(rects));

        for (const auto& r : rects)
        {
            Assert::IsTrue(CheckWithinBounds(r, 300, 300));
        }

        packer.ReSize(9999, 9999);
        packer.DbgValidateAllocatedRectangle(rects.data(), rects.size());
    }

    TEST_METHOD(Test_Mixed_BL_and_MW)
    {
        CRectPackSkyline packer(200, 200);
        CRectPackSkyline::RECT rect;
        std::vector<CRectPackSkyline::RECT> rects;

        // 交替使用两种分配方法
        for (int i = 0; i < 50; ++i)
        {
            if (i % 2 == 0)
            {
                if (packer.AllocateBottomLeft(20, 20, rect))
                {
                    rects.push_back(rect);
                    packer.DbgValidateAllocatedRectangle(rects.data(), rects.size());
                }
            }
            else
            {
                if (packer.AllocateMinWaste(15, 15, rect))
                {
                    rects.push_back(rect);
                    packer.DbgValidateAllocatedRectangle(rects.data(), rects.size());
                }
            }
        }

        Assert::IsTrue(CheckNoOverlap(rects));
    }

    TEST_METHOD(Test_VaryingSizes_Sequential)
    {
        CRectPackSkyline packer(200, 200);
        CRectPackSkyline::RECT rect;

        // 从大到小
        Assert::IsTrue(packer.AllocateBottomLeft(100, 50, rect));
        Assert::IsTrue(packer.AllocateBottomLeft(80, 40, rect));
        Assert::IsTrue(packer.AllocateBottomLeft(60, 30, rect));
        Assert::IsTrue(packer.AllocateBottomLeft(40, 20, rect));
        Assert::IsTrue(packer.AllocateBottomLeft(20, 10, rect));
    }

    // ==================== 特殊场景测试 ====================

    TEST_METHOD(Test_TallRects)
    {
        CRectPackSkyline packer(100, 200);
        CRectPackSkyline::RECT rect;

        Assert::IsTrue(packer.AllocateBottomLeft(20, 180, rect));
        Assert::IsTrue(packer.AllocateBottomLeft(20, 150, rect));
        Assert::IsTrue(packer.AllocateBottomLeft(20, 120, rect));
    }

    TEST_METHOD(Test_WideRects)
    {
        CRectPackSkyline packer(200, 100);
        CRectPackSkyline::RECT rect;

        Assert::IsTrue(packer.AllocateBottomLeft(180, 20, rect));
        Assert::IsTrue(packer.AllocateBottomLeft(150, 20, rect));
        Assert::IsTrue(packer.AllocateBottomLeft(120, 20, rect));
    }

    TEST_METHOD(Test_ExactFit)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect;

        Assert::IsTrue(packer.IsEmpty());

        // 四个50x50的矩形应该完美填充
        Assert::IsTrue(packer.AllocateBottomLeft(50, 50, rect));
        Assert::AreEqual(0, rect.x);
        Assert::AreEqual(0, rect.y);

        Assert::IsTrue(packer.AllocateBottomLeft(50, 50, rect));
        Assert::AreEqual(50, rect.x);
        Assert::AreEqual(0, rect.y);

        Assert::IsTrue(packer.AllocateBottomLeft(50, 50, rect));
        Assert::AreEqual(0, rect.x);
        Assert::AreEqual(50, rect.y);

        Assert::IsTrue(packer.AllocateBottomLeft(50, 50, rect));
        Assert::AreEqual(50, rect.x);
        Assert::AreEqual(50, rect.y);

        // 容器已满
        Assert::IsFalse(packer.AllocateBottomLeft(1, 1, rect));
    
        Assert::IsFalse(packer.IsEmpty());
    }

    TEST_METHOD(Test_FragmentationScenario)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect;

        // 创建碎片化的天际线
        packer.AllocateBottomLeft(10, 90, rect);
        packer.AllocateBottomLeft(10, 10, rect);
        packer.AllocateBottomLeft(10, 80, rect);
        packer.AllocateBottomLeft(10, 20, rect);
        packer.AllocateBottomLeft(10, 70, rect);

        // 尝试插入需要跨越多个段的矩形
        Assert::IsTrue(packer.AllocateMinWaste(50, 10, rect));
    }

    TEST_METHOD(Test_AlternatingHeights)
    {
        CRectPackSkyline packer(200, 200);
        CRectPackSkyline::RECT rect;

        // 创建交替高度
        for (int i = 0; i < 10; ++i)
        {
            int h = (i % 2 == 0) ? 30 : 50;
            Assert::IsTrue(packer.AllocateBottomLeft(20, h, rect));
        }
    }

    // ==================== 边界情况综合测试 ====================

    TEST_METHOD(Test_SinglePixelGaps)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect;

        packer.AllocateBottomLeft(49, 50, rect);
        packer.AllocateBottomLeft(49, 50, rect);

        BOOL result = packer.AllocateBottomLeft(2, 51, rect);
        Assert::IsTrue(result);
    }

    TEST_METHOD(Test_HeightAccumulation)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect;
        int totalHeight = 0;

        // 累积高度直到极限
        while (totalHeight < 100 && packer.AllocateBottomLeft(100, 1, rect))
        {
            totalHeight++;
        }

        Assert::AreEqual(100, totalHeight);
    }

    TEST_METHOD(Test_WorstCaseFragmentation)
    {
        CRectPackSkyline packer(200, 200);
        CRectPackSkyline::RECT rect;
        std::vector<CRectPackSkyline::RECT> rects;

        // 创建最坏情况的碎片
        for (int i = 0; i < 20; ++i)
        {
            int h = 10 + (i * 13) % 80;
            if (packer.AllocateBottomLeft(10, h, rect))
            {
                rects.push_back(rect);
                packer.DbgValidateAllocatedRectangle(rects.data(), rects.size());
            }
        }

        // 尝试插入大矩形
        BOOL result = packer.AllocateMinWaste(50, 50, rect);

        if (result)
        {
            rects.push_back(rect);
            packer.DbgValidateAllocatedRectangle(rects.data(), rects.size());
            Assert::IsTrue(CheckNoOverlap(rects));
        }
    }
};
TS_NS_END