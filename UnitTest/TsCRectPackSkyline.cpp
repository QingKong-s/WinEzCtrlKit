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
        CRectPackSkyline::RECT rect{ .cx = 1, .cy = 1 };
        Assert::IsFalse(packer.AllocateBottomLeft(rect));
    }

    TEST_METHOD(Test_Constructor_OneDimensionZero)
    {
        CRectPackSkyline packer1(100, 0);
        CRectPackSkyline packer2(0, 100);
        CRectPackSkyline::RECT rect{ .cx = 10, .cy = 10 };

        Assert::IsFalse(packer1.AllocateBottomLeft(rect));
        rect.cx = rect.cy = 10;
        Assert::IsFalse(packer2.AllocateBottomLeft(rect));
    }

    // ==================== AllocateBottomLeft 测试 ====================

    TEST_METHOD(Test_BL_SingleRect)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect
        {
            .cx = 50,
            .cy = 30
        };
        
        BOOL result = packer.AllocateBottomLeft(rect);

        Assert::IsTrue(result);
        Assert::AreEqual(0u, rect.x);
        Assert::AreEqual(0u, rect.y);
        Assert::AreEqual(50u, rect.cx);
        Assert::AreEqual(30u, rect.cy);
    }

    TEST_METHOD(Test_BL_TwoRectsHorizontal)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect1, rect2;
        rect1.cx = 40;
        rect1.cy = 30;
        rect2.cx = 40;
        rect2.cy = 30;

        Assert::IsTrue(packer.AllocateBottomLeft(rect1));
        Assert::IsTrue(packer.AllocateBottomLeft(rect2));

        Assert::AreEqual(0u, rect1.x);
        Assert::AreEqual(40u, rect2.x);
        Assert::AreEqual(0u, rect2.y);
    }

    TEST_METHOD(Test_BL_ExceedsWidth)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect{ .cx = 150, .cy = 30 };

        Assert::IsFalse(packer.AllocateBottomLeft(rect));
    }

    TEST_METHOD(Test_BL_ExceedsHeight)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect{ .cx = 50, .cy = 150 };

        Assert::IsFalse(packer.AllocateBottomLeft(rect));
    }

    TEST_METHOD(Test_BL_FullWidthRect)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect1{ .cx = 100, .cy = 30 };
        CRectPackSkyline::RECT rect2{ .cx = 100, .cy = 20 };

        Assert::IsTrue(packer.AllocateBottomLeft(rect1));
        Assert::IsTrue(packer.AllocateBottomLeft(rect2));

        Assert::AreEqual(0u, rect2.x);
        Assert::AreEqual(30u, rect2.y);
    }

    TEST_METHOD(Test_BL_SpansMultipleSegments)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect1{ .cx = 30, .cy = 20 };
        CRectPackSkyline::RECT rect2{ .cx = 30, .cy = 40 };
        CRectPackSkyline::RECT rect3{ .cx = 30, .cy = 25 };

        packer.AllocateBottomLeft(rect1);
        packer.AllocateBottomLeft(rect2);
        packer.AllocateBottomLeft(rect3);

        // 插入跨越多段的矩形
        CRectPackSkyline::RECT rect4{ .cx = 90, .cy = 10 };
        Assert::IsTrue(packer.AllocateBottomLeft(rect4));

        // 应该放在最高点
        Assert::AreEqual(40u, rect4.y);
    }

    TEST_METHOD(Test_BL_PartialSegmentFill)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect1{ .cx = 100, .cy = 30 };
        CRectPackSkyline::RECT rect2{ .cx = 50, .cy = 20 };

        packer.AllocateBottomLeft(rect1);
        Assert::IsTrue(packer.AllocateBottomLeft(rect2));

        Assert::AreEqual(0u, rect2.x);
        Assert::AreEqual(30u, rect2.y);
    }

    TEST_METHOD(Test_BL_FillToCapacity)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect{ .cx = 100, .cy = 10 };
        int count = 0;

        for (int i = 0; i < 10; ++i)
        {
            rect.cx = 100;
            rect.cy = 10;
            if (packer.AllocateBottomLeft(rect))
                ++count;
        }

        Assert::AreEqual(10, count);
        rect.cx = 10;
        rect.cy = 10;
        Assert::IsFalse(packer.AllocateBottomLeft(rect));
    }

    TEST_METHOD(Test_BL_MinimalRect)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect{ .cx = 1, .cy = 1 };

        Assert::IsTrue(packer.AllocateBottomLeft(rect));
        Assert::AreEqual(0u, rect.x);
        Assert::AreEqual(0u, rect.y);
    }

    TEST_METHOD(Test_BL_ExactBoundary)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect1{ .cx = 60, .cy = 30 };
        CRectPackSkyline::RECT rect2{ .cx = 40, .cy = 30 };

        packer.AllocateBottomLeft(rect1);
        Assert::IsTrue(packer.AllocateBottomLeft(rect2));
        Assert::AreEqual(60u, rect2.x);

        rect2.cx = 99;
        rect2.cy = 70;
        Assert::IsTrue(packer.AllocateBottomLeft(rect2));
        rect2.cx = 2;
        rect2.cy = 1;
        Assert::IsFalse(packer.AllocateBottomLeft(rect2));
    }

    // ==================== AllocateMinimumWaste 测试 ====================

    TEST_METHOD(Test_MW_SingleRect)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect{ .cx = 50, .cy = 30 };

        Assert::IsTrue(packer.AllocateMinimumWaste(rect));
        Assert::AreEqual(0u, rect.x);
        Assert::AreEqual(0u, rect.y);
    }

    TEST_METHOD(Test_MW_MinimizeWaste)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect1{ .cx = 40, .cy = 50 };
        CRectPackSkyline::RECT rect2{ .cx = 60, .cy = 30 };
        CRectPackSkyline::RECT rect3{ .cx = 30, .cy = 20 };

        // 创建不同高度的空间
        packer.AllocateMinimumWaste(rect1);
        packer.AllocateMinimumWaste(rect2);

        // 应该选择浪费最小的位置
        Assert::IsTrue(packer.AllocateMinimumWaste(rect3));
    }

    TEST_METHOD(Test_MW_LeftVsRightPlacement)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect1{ .cx = 100, .cy = 30 };
        CRectPackSkyline::RECT rect2{ .cx = 50, .cy = 20 };

        packer.AllocateMinimumWaste(rect1);

        // 测试左右放置的选择
        Assert::IsTrue(packer.AllocateMinimumWaste(rect2));
    }

    TEST_METHOD(Test_MW_ExceedsCapacity)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect1{ .cx = 150, .cy = 30 };
        CRectPackSkyline::RECT rect2{ .cx = 30, .cy = 150 };

        Assert::IsFalse(packer.AllocateMinimumWaste(rect1));
        Assert::IsFalse(packer.AllocateMinimumWaste(rect2));
    }

    TEST_METHOD(Test_MW_ComplexSkyline)
    {
        CRectPackSkyline packer(150, 150);
        CRectPackSkyline::RECT rect{ .cx = 30, .cy = 30 };

        packer.AllocateMinimumWaste(rect);
        rect.cx = 30; rect.cy = 20;
        packer.AllocateMinimumWaste(rect);
        rect.cx = 30; rect.cy = 40;
        packer.AllocateMinimumWaste(rect);
        rect.cx = 30; rect.cy = 25;
        packer.AllocateMinimumWaste(rect);

        rect.cx = 60; rect.cy = 10;
        Assert::IsTrue(packer.AllocateMinimumWaste(rect));
    }

    TEST_METHOD(Test_MW_SpanMultipleSegments)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect1{ .cx = 25, .cy = 20 };
        CRectPackSkyline::RECT rect2{ .cx = 25, .cy = 30 };
        CRectPackSkyline::RECT rect3{ .cx = 25, .cy = 25 };
        CRectPackSkyline::RECT rect4{ .cx = 75, .cy = 10 };

        packer.AllocateMinimumWaste(rect1);
        packer.AllocateMinimumWaste(rect2);
        packer.AllocateMinimumWaste(rect3);

        // 跨越三个段
        Assert::IsTrue(packer.AllocateMinimumWaste(rect4));
    }

    TEST_METHOD(Test_MW_RightAlignment)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect1{ .cx = 100, .cy = 30 };
        CRectPackSkyline::RECT rect2{ .cx = 30, .cy = 20 };

        packer.AllocateMinimumWaste(rect1);

        // 测试从右侧放置
        Assert::IsTrue(packer.AllocateMinimumWaste(rect2));
    }

    TEST_METHOD(Test_MW_ZeroWaste)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect1{ .cx = 50, .cy = 50 };
        CRectPackSkyline::RECT rect2{ .cx = 50, .cy = 50 };

        packer.AllocateMinimumWaste(rect1);

        // 完全填充剩余空间，无浪费
        Assert::IsTrue(packer.AllocateMinimumWaste(rect2));
        Assert::AreEqual(50u, rect2.x);
        Assert::AreEqual(0u, rect2.y);
    }

    // ==================== 合并测试 ====================

    TEST_METHOD(Test_Merge_SameHeightSegments)
    {
        CRectPackSkyline packer(120, 100);
        CRectPackSkyline::RECT rect1{ .cx = 30, .cy = 30 };
        CRectPackSkyline::RECT rect2{ .cx = 30, .cy = 30 };
        CRectPackSkyline::RECT rect3{ .cx = 30, .cy = 30 };
        CRectPackSkyline::RECT rect4{ .cx = 90, .cy = 10 };

        // 创建多个相同高度的段
        packer.AllocateBottomLeft(rect1);
        packer.AllocateBottomLeft(rect2);
        packer.AllocateBottomLeft(rect3);

        // 插入跨越所有段的矩形，应该触发合并
        Assert::IsTrue(packer.AllocateBottomLeft(rect4));
    }

    TEST_METHOD(Test_Merge_AfterFullFill)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect1{ .cx = 100, .cy = 30 };
        CRectPackSkyline::RECT rect2{ .cx = 100, .cy = 20 };

        packer.AllocateBottomLeft(rect1);
        packer.AllocateBottomLeft(rect2);

        // 验证可以继续插入
        CRectPackSkyline::RECT rect3{ .cx = 50, .cy = 10 };
        Assert::IsTrue(packer.AllocateBottomLeft(rect3));
    }

    // ==================== 压力和混合测试 ====================

    TEST_METHOD(Test_Stress_ManySmallRects_BL)
    {
        CRectPackSkyline packer(200, 200);
        CRectPackSkyline::RECT rect{ .cx = 10, .cy = 10 };
        std::vector<CRectPackSkyline::RECT> rects;

        for (int i = 0; i < 200; ++i)
        {
            rect.cx = 10;
            rect.cy = 10;
            if (packer.AllocateBottomLeft(rect))
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
        CRectPackSkyline::RECT rect{ .cx = 10, .cy = 10 };
        std::vector<CRectPackSkyline::RECT> rects;

        for (int i = 0; i < 200; ++i)
        {
            rect.cx = 10;
            rect.cy = 10;
            if (packer.AllocateMinimumWaste(rect))
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
            rect.cx = 10 + (i * 7) % 40;
            rect.cy = 10 + (i * 11) % 40;
            if (packer.AllocateBottomLeft(rect))
            {
                rects.push_back(rect);
                packer.DbgValidateAllocatedRectangle(rects.data(), rects.size());
            }
        }

        Assert::IsTrue(rects.size() > 30);
        Assert::IsTrue(CheckNoOverlap(rects));

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
            rect.cx = 10 + (i * 7) % 40;
            rect.cy = 10 + (i * 11) % 40;
            if (packer.AllocateMinimumWaste(rect))
            {
                rects.push_back(rect);
                packer.DbgValidateAllocatedRectangle(rects.data(), rects.size());
            }
        }

        Assert::IsTrue(rects.size() > 30);
        Assert::IsTrue(CheckNoOverlap(rects));

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
                rect.cx = 20;
                rect.cy = 20;
                if (packer.AllocateBottomLeft(rect))
                {
                    rects.push_back(rect);
                    packer.DbgValidateAllocatedRectangle(rects.data(), rects.size());
                }
            }
            else
            {
                rect.cx = 15;
                rect.cy = 15;
                if (packer.AllocateMinimumWaste(rect))
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
        rect.cx = 100; rect.cy = 50;
        Assert::IsTrue(packer.AllocateBottomLeft(rect));
        rect.cx = 80; rect.cy = 40;
        Assert::IsTrue(packer.AllocateBottomLeft(rect));
        rect.cx = 60; rect.cy = 30;
        Assert::IsTrue(packer.AllocateBottomLeft(rect));
        rect.cx = 40; rect.cy = 20;
        Assert::IsTrue(packer.AllocateBottomLeft(rect));
        rect.cx = 20; rect.cy = 10;
        Assert::IsTrue(packer.AllocateBottomLeft(rect));
    }

    // ==================== 特殊场景测试 ====================

    TEST_METHOD(Test_TallRects)
    {
        CRectPackSkyline packer(100, 200);
        CRectPackSkyline::RECT rect;

        rect.cx = 20; rect.cy = 180;
        Assert::IsTrue(packer.AllocateBottomLeft(rect));
        rect.cx = 20; rect.cy = 150;
        Assert::IsTrue(packer.AllocateBottomLeft(rect));
        rect.cx = 20; rect.cy = 120;
        Assert::IsTrue(packer.AllocateBottomLeft(rect));
    }

    TEST_METHOD(Test_WideRects)
    {
        CRectPackSkyline packer(200, 100);
        CRectPackSkyline::RECT rect;

        rect.cx = 180; rect.cy = 20;
        Assert::IsTrue(packer.AllocateBottomLeft(rect));
        rect.cx = 150; rect.cy = 20;
        Assert::IsTrue(packer.AllocateBottomLeft(rect));
        rect.cx = 120; rect.cy = 20;
        Assert::IsTrue(packer.AllocateBottomLeft(rect));
    }

    TEST_METHOD(Test_ExactFit)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect;

        Assert::IsTrue(packer.IsEmpty());

        // 四个50x50的矩形应该完美填充
        rect.cx = 50; rect.cy = 50;
        Assert::IsTrue(packer.AllocateBottomLeft(rect));
        Assert::AreEqual(0u, rect.x);
        Assert::AreEqual(0u, rect.y);

        rect.cx = 50; rect.cy = 50;
        Assert::IsTrue(packer.AllocateBottomLeft(rect));
        Assert::AreEqual(50u, rect.x);
        Assert::AreEqual(0u, rect.y);

        rect.cx = 50; rect.cy = 50;
        Assert::IsTrue(packer.AllocateBottomLeft(rect));
        Assert::AreEqual(0u, rect.x);
        Assert::AreEqual(50u, rect.y);

        rect.cx = 50; rect.cy = 50;
        Assert::IsTrue(packer.AllocateBottomLeft(rect));
        Assert::AreEqual(50u, rect.x);
        Assert::AreEqual(50u, rect.y);

        // 容器已满
        rect.cx = 1; rect.cy = 1;
        Assert::IsFalse(packer.AllocateBottomLeft(rect));

        Assert::IsFalse(packer.IsEmpty());
    }

    TEST_METHOD(Test_FragmentationScenario)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect;

        // 创建碎片化的天际线
        rect.cx = 10; rect.cy = 90;
        packer.AllocateBottomLeft(rect);
        rect.cx = 10; rect.cy = 10;
        packer.AllocateBottomLeft(rect);
        rect.cx = 10; rect.cy = 80;
        packer.AllocateBottomLeft(rect);
        rect.cx = 10; rect.cy = 20;
        packer.AllocateBottomLeft(rect);
        rect.cx = 10; rect.cy = 70;
        packer.AllocateBottomLeft(rect);

        // 尝试插入需要跨越多个段的矩形
        rect.cx = 50; rect.cy = 10;
        Assert::IsTrue(packer.AllocateMinimumWaste(rect));
    }

    TEST_METHOD(Test_AlternatingHeights)
    {
        CRectPackSkyline packer(200, 200);
        CRectPackSkyline::RECT rect;

        // 创建交替高度
        for (int i = 0; i < 10; ++i)
        {
            rect.cx = 20;
            rect.cy = (i % 2 == 0) ? 30 : 50;
            Assert::IsTrue(packer.AllocateBottomLeft(rect));
        }
    }

    // ==================== 边界情况综合测试 ====================

    TEST_METHOD(Test_SinglePixelGaps)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect;

        rect.cx = 49; rect.cy = 50;
        packer.AllocateBottomLeft(rect);
        rect.cx = 49; rect.cy = 50;
        packer.AllocateBottomLeft(rect);

        rect.cx = 2; rect.cy = 51;
        BOOL result = packer.AllocateBottomLeft(rect);
        Assert::IsTrue(result);
    }

    TEST_METHOD(Test_HeightAccumulation)
    {
        CRectPackSkyline packer(100, 100);
        CRectPackSkyline::RECT rect;
        int totalHeight = 0;

        // 累积高度直到极限
        rect.cx = 100; rect.cy = 1;
        while (totalHeight < 100 && packer.AllocateBottomLeft(rect))
        {
            totalHeight++;
            rect.cx = 100; rect.cy = 1;
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
            rect.cx = 10;
            rect.cy = 10 + (i * 13) % 80;
            if (packer.AllocateBottomLeft(rect))
            {
                rects.push_back(rect);
                packer.DbgValidateAllocatedRectangle(rects.data(), rects.size());
            }
        }

        // 尝试插入大矩形
        rect.cx = 50; rect.cy = 50;
        BOOL result = packer.AllocateMinimumWaste(rect);

        if (result)
        {
            rects.push_back(rect);
            packer.DbgValidateAllocatedRectangle(rects.data(), rects.size());
            Assert::IsTrue(CheckNoOverlap(rects));
        }
    }
};
TS_NS_END