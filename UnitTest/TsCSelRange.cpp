#include "pch.h"
#include "../eck/CSelRange.h"

using namespace eck;

TS_NS_BEGIN
void AssertRanges(const CSelectionRangeT<int>& sel, std::vector<std::pair<int, int>> expected)
{
    const auto& ranges = sel.GetList();
    Assert::AreEqual(expected.size(), ranges.size(), L"Range count mismatch");

    for (size_t i = 0; i < expected.size(); ++i)
    {
        Assert::AreEqual(expected[i].first, (int)ranges[i].idxBegin);
        Assert::AreEqual(expected[i].second, (int)ranges[i].idxEnd);
    }
}

TEST_CLASS(TsCSelRange)
{
public:
    TEST_METHOD(Test_EmptyConstruction)
    {
        CSelectionRangeT<int> sel;
        Assert::IsTrue(sel.IsEmpty(), L"Should be empty on construction");
        Assert::AreEqual(-1, sel.GetFirstSelected(), L"First selected should be -1");
        Assert::AreEqual(-1, sel.GetLastSelected(), L"Last selected should be -1");
        Assert::AreEqual(0, sel.CountIncluded(), L"Count should be 0");
    }

    TEST_METHOD(Test_InitializerListConstruction)
    {
        CSelectionRangeT<int> sel{ {0, 2}, {5, 7}, {10, 10} };
        AssertRanges(sel, { {0, 2}, {5, 7}, {10, 10} });
    }

    TEST_METHOD(Test_IncludeRange_SingleRange)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(5, 10);
        AssertRanges(sel, { {5, 10} });
        Assert::AreEqual(6, sel.CountIncluded(), L"Should have 6 items");
    }

    TEST_METHOD(Test_IncludeRange_MergeAdjacent_Left)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(10, 15);
        sel.IncludeRange(5, 9);  // 相邻，应合并
        AssertRanges(sel, { {5, 15} });
    }

    TEST_METHOD(Test_IncludeRange_MergeAdjacent_Right)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(5, 10);
        sel.IncludeRange(11, 15);  // 相邻，应合并
        AssertRanges(sel, { {5, 15} });
    }

    TEST_METHOD(Test_IncludeRange_NoMerge_Gap)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(5, 10);
        sel.IncludeRange(12, 15);  // 有间隙，不合并
        AssertRanges(sel, { {5, 10}, {12, 15} });
    }

    TEST_METHOD(Test_IncludeRange_MergeOverlapping)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(5, 10);
        sel.IncludeRange(8, 15);  // 重叠
        AssertRanges(sel, { {5, 15} });
    }

    TEST_METHOD(Test_IncludeRange_MergeMultiple)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(5, 7);
        sel.IncludeRange(10, 12);
        sel.IncludeRange(15, 17);
        sel.IncludeRange(6, 16);  // 跨越多个区间
        AssertRanges(sel, { {5, 17} });
    }

    TEST_METHOD(Test_IncludeRange_InsertBetween)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(5, 7);
        sel.IncludeRange(15, 17);
        sel.IncludeRange(10, 12);  // 插入中间
        AssertRanges(sel, { {5, 7}, {10, 12}, {15, 17} });
    }

    TEST_METHOD(Test_IncludeRange_CompletelyInside)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(5, 20);
        sel.IncludeRange(10, 15);  // 完全在内部
        AssertRanges(sel, { {5, 20} });
    }

    TEST_METHOD(Test_IncludeRange_ExtendLeft)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(10, 15);
        sel.IncludeRange(5, 12);  // 向左扩展
        AssertRanges(sel, { {5, 15} });
    }

    TEST_METHOD(Test_IncludeRange_ExtendRight)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(10, 15);
        sel.IncludeRange(12, 20);  // 向右扩展
        AssertRanges(sel, { {10, 20} });
    }

    TEST_METHOD(Test_IncludeRange_AtEnd)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(5, 10);
        sel.IncludeRange(20, 25);  // 大于所有现有区间
        AssertRanges(sel, { {5, 10}, {20, 25} });
    }

    TEST_METHOD(Test_IncludeRange_AtBeginning)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(20, 25);
        sel.IncludeRange(5, 10);  // 小于所有现有区间
        AssertRanges(sel, { {5, 10}, {20, 25} });
    }

    TEST_METHOD(Test_IncludeItem)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeItem(5);
        sel.IncludeItem(7);
        sel.IncludeItem(6);  // 填充间隙
        AssertRanges(sel, { {5, 7} });
    }

    TEST_METHOD(Test_ExcludeRange_FromEmpty)
    {
        CSelectionRangeT<int> sel;
        sel.ExcludeRange(5, 10);
        Assert::IsTrue(sel.IsEmpty(), L"Should remain empty");
    }

    TEST_METHOD(Test_ExcludeRange_NoOverlap)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(10, 15);
        sel.ExcludeRange(20, 25);  // 不重叠
        AssertRanges(sel, { {10, 15} });
    }

    TEST_METHOD(Test_ExcludeRange_CompletelyRemove)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(10, 15);
        sel.ExcludeRange(10, 15);  // 完全移除
        Assert::IsTrue(sel.IsEmpty(), L"Should be empty");
    }

    TEST_METHOD(Test_ExcludeRange_CompletelyRemove_Larger)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(10, 15);
        sel.ExcludeRange(5, 20);  // 范围更大
        Assert::IsTrue(sel.IsEmpty(), L"Should be empty");
    }

    TEST_METHOD(Test_ExcludeRange_SplitInMiddle)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(5, 20);
        sel.ExcludeRange(10, 15);  // 从中间切除
        AssertRanges(sel, { {5, 9}, {16, 20} });
    }

    TEST_METHOD(Test_ExcludeRange_TrimLeft)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(10, 20);
        sel.ExcludeRange(5, 15);  // 修剪左边
        AssertRanges(sel, { {16, 20} });
    }

    TEST_METHOD(Test_ExcludeRange_TrimRight)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(10, 20);
        sel.ExcludeRange(15, 25);  // 修剪右边
        AssertRanges(sel, { {10, 14} });
    }

    TEST_METHOD(Test_ExcludeRange_RemoveMultiple)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(5, 10);
        sel.IncludeRange(15, 20);
        sel.IncludeRange(25, 30);
        sel.ExcludeRange(8, 27);  // 跨越多个区间
        AssertRanges(sel, { {5, 7}, {28, 30} });
    }

    TEST_METHOD(Test_ExcludeRange_EdgeCase_LeftBoundary)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(10, 20);
        sel.ExcludeRange(10, 10);  // 只移除左边界
        AssertRanges(sel, { {11, 20} });
    }

    TEST_METHOD(Test_ExcludeRange_EdgeCase_RightBoundary)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(10, 20);
        sel.ExcludeRange(20, 20);  // 只移除右边界
        AssertRanges(sel, { {10, 19} });
    }

    TEST_METHOD(Test_ExcludeRange_SingleElement)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(10, 10);
        sel.ExcludeRange(10, 10);  // 移除单元素区间
        Assert::IsTrue(sel.IsEmpty(), L"Should be empty");
    }

    TEST_METHOD(Test_ExcludeItem)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(5, 10);
        sel.ExcludeItem(7);
        AssertRanges(sel, { {5, 6}, {8, 10} });
    }

    TEST_METHOD(Test_InvertRange_Empty)
    {
        CSelectionRangeT<int> sel;
        sel.InvertRange(5, 10);
        AssertRanges(sel, { {5, 10} });
    }

    TEST_METHOD(Test_InvertRange_FullySelected)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(5, 10);
        sel.InvertRange(5, 10);
        Assert::IsTrue(sel.IsEmpty(), L"Should be empty after invert");
    }

    TEST_METHOD(Test_InvertRange_PartialOverlap_Left)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(5, 10);
        sel.InvertRange(3, 7);  // [3,4] 新增, [5,7] 移除
        AssertRanges(sel, { {3, 4}, {8, 10} });
    }

    TEST_METHOD(Test_InvertRange_PartialOverlap_Right)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(5, 10);
        sel.InvertRange(8, 12);  // [8,10] 移除, [11,12] 新增
        AssertRanges(sel, { {5, 7}, {11, 12} });
    }

    TEST_METHOD(Test_InvertRange_SplitInMiddle)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(5, 15);
        sel.InvertRange(8, 12);  // 中间反转
        AssertRanges(sel, { {5, 7}, {13, 15} });
    }

    TEST_METHOD(Test_InvertRange_FillGap)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(5, 7);
        sel.IncludeRange(10, 12);
        sel.InvertRange(6, 11);  // 填充间隙并修剪边缘
        AssertRanges(sel, { {5, 5}, {8, 9}, {12, 12} });
    }

    TEST_METHOD(Test_InvertRange_ComplexPattern)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(5, 7);
        sel.IncludeRange(10, 12);
        sel.IncludeRange(15, 17);
        sel.InvertRange(6, 16);  // 跨越多个区间
        // [5,5]保留, [6,7]移除, [8,9]新增, [10,12]移除, [13,14]新增, [15,16]移除
        AssertRanges(sel, { {5, 5}, {8, 9}, {13, 14}, {17, 17} });
    }

    TEST_METHOD(Test_InvertItem_Selected)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(5, 10);
        sel.InvertItem(7);
        AssertRanges(sel, { {5, 6}, {8, 10} });
    }

    TEST_METHOD(Test_InvertItem_Unselected)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(5, 7);
        sel.IncludeRange(10, 12);
        sel.InvertItem(9);
        AssertRanges(sel, { {5, 7}, {9, 12} });
    }

    TEST_METHOD(Test_InsertItem_BeforeAll)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(10, 15);
        sel.IncludeRange(20, 25);
        sel.InsertItem(5);  // 在所有区间之前插入
        AssertRanges(sel, { {11, 16}, {21, 26} });  // 所有区间索引+1
    }

    TEST_METHOD(Test_InsertItem_AfterAll)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(10, 15);
        sel.IncludeRange(20, 25);
        sel.InsertItem(30);  // 在所有区间之后插入
        AssertRanges(sel, { {10, 15}, {20, 25} });  // 无变化
    }

    TEST_METHOD(Test_InsertItem_AtRangeBegin)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(10, 15);
        sel.IncludeRange(20, 25);
        sel.InsertItem(10);  // 在区间开始处插入
        // [10,15] 分裂为 [10,10] (插入点之前) 和 [11,16] (原区间平移)
        AssertRanges(sel, { {11, 16}, {21, 26} });
    }

    TEST_METHOD(Test_InsertItem_InMiddleOfRange)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(10, 20);
        sel.InsertItem(15);  // 在区间中间插入
        // [10,20] 分裂为 [10,14] 和 [16,21]
        AssertRanges(sel, { {10, 14}, {16, 21} });
    }

    TEST_METHOD(Test_InsertItem_BetweenRanges)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(10, 15);
        sel.IncludeRange(20, 25);
        sel.InsertItem(17);  // 在两个区间之间插入
        AssertRanges(sel, { {10, 15}, {21, 26} });  // 只有第二个区间平移
    }

    TEST_METHOD(Test_RemoveItem_NotInRange)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(10, 15);
        sel.IncludeRange(20, 25);
        sel.RemoveItem(17);  // 移除未选中项
        AssertRanges(sel, { {10, 15}, {19, 24} });  // 第二个区间索引-1
    }

    TEST_METHOD(Test_RemoveItem_InRange)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(10, 20);
        sel.RemoveItem(15);  // 移除已选中项
        // [10,20] 的端点调整：idxEnd-1 = 19
        AssertRanges(sel, { {10, 19} });
    }

    TEST_METHOD(Test_RemoveItem_RangeBegin)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(10, 20);
        sel.RemoveItem(10);  // 移除区间起点
        AssertRanges(sel, { {10, 19} });  // 区间收缩
    }

    TEST_METHOD(Test_RemoveItem_RangeEnd)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(10, 20);
        sel.RemoveItem(20);  // 移除区间终点
        AssertRanges(sel, { {10, 19} });
    }

    TEST_METHOD(Test_RemoveItem_SingleElement)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(10, 10);
        sel.RemoveItem(10);  // 移除单元素区间
        Assert::IsTrue(sel.IsEmpty(), L"Should be empty");
    }

    TEST_METHOD(Test_RemoveItem_MergeRanges)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(10, 12);
        sel.IncludeRange(14, 16);
        sel.RemoveItem(13);  // 移除间隙中的项，使两个区间相邻
        // [10,12] 和 [14,16] 变为 [10,12] 和 [13,15]，应合并为 [10,15]
        AssertRanges(sel, { {10, 15} });
    }

    TEST_METHOD(Test_InsertRemove_Sequence)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(10, 15);
        sel.InsertItem(12);   // [10,11], [13,16]
        sel.RemoveItem(14);   // [10,11], [13,15]
        sel.InsertItem(11);   // [10,10], [12,12], [14,16]

        // 验证最终状态
        Assert::IsFalse(sel.IsEmpty());
    }

    TEST_METHOD(Test_IsSelected)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(5, 10);
        sel.IncludeRange(15, 20);

        Assert::IsTrue(sel.IsSelected(5), L"5 should be selected");
        Assert::IsTrue(sel.IsSelected(7), L"7 should be selected");
        Assert::IsTrue(sel.IsSelected(10), L"10 should be selected");
        Assert::IsFalse(sel.IsSelected(12), L"12 should not be selected");
        Assert::IsTrue(sel.IsSelected(15), L"15 should be selected");
        Assert::IsFalse(sel.IsSelected(25), L"25 should not be selected");
    }

    TEST_METHOD(Test_NextSelected)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(5, 10);
        sel.IncludeRange(15, 20);

        Assert::AreEqual(5, sel.NextSelected(3), L"Next from 3");
        Assert::AreEqual(5, sel.NextSelected(5), L"Next from 5 (selected)");
        Assert::AreEqual(7, sel.NextSelected(7), L"Next from 7 (selected)");
        Assert::AreEqual(15, sel.NextSelected(12), L"Next from 12");
        Assert::AreEqual(-1, sel.NextSelected(25), L"Next from 25 (none)");
    }

    TEST_METHOD(Test_NextUnSelected)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(5, 10);
        sel.IncludeRange(15, 20);

        Assert::AreEqual(3, sel.NextUnSelected(3), L"3 is unselected");
        Assert::AreEqual(11, sel.NextUnSelected(5), L"Next unselected from 5");
        Assert::AreEqual(11, sel.NextUnSelected(7), L"Next unselected from 7");
        Assert::AreEqual(12, sel.NextUnSelected(12), L"12 is unselected");
        Assert::AreEqual(21, sel.NextUnSelected(15), L"Next unselected from 15");
    }

    TEST_METHOD(Test_CountIncluded)
    {
        CSelectionRangeT<int> sel;
        Assert::AreEqual(0, sel.CountIncluded(), L"Empty count");

        sel.IncludeRange(5, 10);  // 6 items
        Assert::AreEqual(6, sel.CountIncluded());

        sel.IncludeRange(15, 20);  // +6 items
        Assert::AreEqual(12, sel.CountIncluded());

        sel.ExcludeRange(7, 8);  // -2 items
        Assert::AreEqual(10, sel.CountIncluded());
    }

    TEST_METHOD(Test_GetFirstLastSelected)
    {
        CSelectionRangeT<int> sel;
        Assert::AreEqual(-1, sel.GetFirstSelected(), L"Empty first");
        Assert::AreEqual(-1, sel.GetLastSelected(), L"Empty last");

        sel.IncludeRange(10, 15);
        Assert::AreEqual(10, sel.GetFirstSelected());
        Assert::AreEqual(15, sel.GetLastSelected());

        sel.IncludeRange(5, 7);
        Assert::AreEqual(5, sel.GetFirstSelected());
        Assert::AreEqual(15, sel.GetLastSelected());

        sel.IncludeRange(20, 25);
        Assert::AreEqual(5, sel.GetFirstSelected());
        Assert::AreEqual(25, sel.GetLastSelected());
    }

    TEST_METHOD(Test_ZeroIndex)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(0, 5);
        AssertRanges(sel, { {0, 5} });
        Assert::IsTrue(sel.IsSelected(0));
    }

    TEST_METHOD(Test_SinglePoint)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(10, 10);
        AssertRanges(sel, { {10, 10} });
        Assert::AreEqual(1, sel.CountIncluded());
    }

    TEST_METHOD(Test_Clear)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(5, 10);
        sel.IncludeRange(15, 20);
        sel.Clear();
        Assert::IsTrue(sel.IsEmpty());
    }

    TEST_METHOD(Test_Clear_WithVisibleRange)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(5, 10);
        sel.IncludeRange(15, 20);
        sel.IncludeRange(25, 30);

        int changedBegin, changedEnd;
        sel.Clear(12, 22, changedBegin, changedEnd);

        Assert::IsTrue(sel.IsEmpty());
        Assert::AreEqual(15, changedBegin);
        Assert::AreEqual(20, changedEnd);
    }

    TEST_METHOD(Test_OnSetItemCount)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(5, 10);
        sel.IncludeRange(15, 20);
        sel.IncludeRange(25, 30);

        sel.OnSetItemCount(18);  // 移除索引 >= 18 的所有项
        AssertRanges(sel, { {5, 10}, {15, 17} });
    }

    TEST_METHOD(Test_ManyRanges)
    {
        CSelectionRangeT<int> sel;
        for (int i = 0; i < 100; i += 5)
        {
            sel.IncludeRange(i, i + 2);
        }
        Assert::AreEqual((size_t)20, sel.GetList().size());
        Assert::AreEqual(60, sel.CountIncluded());  // 20 ranges * 3 items each
    }

    TEST_METHOD(Test_AlternatingPattern)
    {
        CSelectionRangeT<int> sel;
        for (int i = 0; i < 20; ++i)
        {
            if (i % 2 == 0)
                sel.IncludeItem(i);
        }
        Assert::AreEqual(10, sel.CountIncluded());

        // 反转所有奇数位置
        for (int i = 0; i < 20; ++i)
        {
            if (i % 2 == 1)
                sel.InvertItem(i);
        }
        Assert::AreEqual(20, sel.CountIncluded());
        AssertRanges(sel, { {0, 19} });  // 应该合并成一个大区间
    }

    TEST_METHOD(Test_LargeRange)
    {
        CSelectionRangeT<int> sel;
        sel.IncludeRange(0, 100000);
        Assert::AreEqual(100001, sel.CountIncluded());

        sel.ExcludeRange(50000, 50000);
        Assert::AreEqual(100000, sel.CountIncluded());
        AssertRanges(sel, { {0, 49999}, {50001, 100000} });
    }

    TEST_METHOD(Test_StressTest_RandomOperations)
    {
        CSelectionRangeT<int> sel;

        // 执行一系列随机操作
        sel.IncludeRange(10, 20);
        sel.IncludeRange(30, 40);
        sel.ExcludeRange(15, 35);
        sel.InvertRange(5, 45);
        sel.IncludeItem(25);
        sel.ExcludeItem(12);

        // 验证最终状态的一致性
        int count = 0;
        for (const auto& range : sel.GetList())
        {
            Assert::IsTrue(range.idxEnd >= range.idxBegin,
                L"Range should be valid");
            count += (range.idxEnd - range.idxBegin + 1);
        }
        Assert::AreEqual(count, sel.CountIncluded(),
            L"Count should match sum of ranges");
    }

    TEST_METHOD(Test_Issue_AdjacentRangeMerge)
    {
        // 测试相邻区间是否正确合并
        CSelectionRangeT<int> sel;
        sel.IncludeRange(10, 15);
        sel.IncludeRange(16, 20);
        AssertRanges(sel, { {10, 20} });
    }

    TEST_METHOD(Test_Issue_TripleMerge)
    {
        // 测试三个区间同时合并
        CSelectionRangeT<int> sel;
        sel.IncludeRange(5, 6);
        sel.IncludeRange(8, 9);
        sel.IncludeRange(11, 12);
        sel.IncludeRange(6, 11);  // 应该合并成一个区间
        AssertRanges(sel, { {5, 12} });
    }

    TEST_METHOD(Test_Issue_InvertBoundary)
    {
        // 测试边界反转的特殊情况
        CSelectionRangeT<int> sel;
        sel.IncludeRange(10, 20);
        sel.InvertRange(10, 10);  // 只反转起点
        AssertRanges(sel, { {11, 20} });

        sel.Clear();
        sel.IncludeRange(10, 20);
        sel.InvertRange(20, 20);  // 只反转终点
        AssertRanges(sel, { {10, 19} });
    }

    TEST_METHOD(Test_Issue_RemoveAndMerge)
    {
        // 移除项导致两个区间合并
        CSelectionRangeT<int> sel;
        sel.IncludeRange(10, 12);
        sel.IncludeRange(14, 16);
        Assert::AreEqual((size_t)2, sel.GetList().size());

        sel.RemoveItem(13);  // 移除间隙，应该触发合并
        AssertRanges(sel, { {10, 15} });
    }

    TEST_METHOD(Test_Issue_MultipleInserts)
    {
        // 多次插入的累积效果
        CSelectionRangeT<int> sel;
        sel.IncludeRange(10, 15);
        sel.InsertItem(10);  // [11, 16]
        sel.InsertItem(11);  // [12, 17]
        sel.InsertItem(12);  // [13, 18]
        AssertRanges(sel, { {13, 18} });
    }
};
TS_NS_END