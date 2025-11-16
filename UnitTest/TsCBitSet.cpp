#include "pch.h"
#include "../eck/CBitSet.h"

TS_NS_BEGIN
TEST_CLASS(TsCBitSet)
{
    using BS = eck::CBitSet<64>;

public:

    TEST_METHOD(TsDefaultConstruct)
    {
        BS bs{};
        for (size_t i = 0; i < 64; ++i)
            Assert::IsFalse(bs.Test(i));
        Assert::IsTrue(bs.NoneOne());
    }

    TEST_METHOD(TsSetClearSingleBit)
    {
        BS bs{};
        bs.Set(3);
        Assert::IsTrue(bs.Test(3));
        Assert::AreEqual((size_t)1, bs.PopCount());

        bs.Clear(3);
        Assert::IsFalse(bs.Test(3));
        Assert::IsTrue(bs.NoneOne());
    }

    TEST_METHOD(TsSetAllClearAll)
    {
        BS bs{};
        bs.Set();
        Assert::IsTrue(bs.AllOne());
        Assert::AreEqual((size_t)64, bs.PopCount());

        bs.Clear();
        Assert::IsTrue(bs.NoneOne());
        Assert::AreEqual((size_t)0, bs.PopCount());
    }

    TEST_METHOD(TsFlipSingle)
    {
        BS bs{};

        bs.Flip(10);
        Assert::IsTrue(bs.Test(10));

        bs.Flip(10);
        Assert::IsFalse(bs.Test(10));
    }

    TEST_METHOD(TsFlipAll)
    {
        BS bs{};
        bs.Set();     // all ones
        bs.Flip();    // now all zeros
        Assert::IsTrue(bs.NoneOne());
    }

    TEST_METHOD(TsProxyWriteRead)
    {
        BS bs{};
        bs[5] = true;
        Assert::IsTrue(bs[5]);

        bs[5] = false;
        Assert::IsFalse(bs[5]);
    }

    TEST_METHOD(TsAndOrXor)
    {
        BS a{}, b{};
        a.Set(1);
        a.Set(3);

        b.Set(3);
        b.Set(4);

        BS x = a;
        x &= b;        // intersection = {3}
        Assert::IsTrue(x.Test(3));
        Assert::AreEqual((size_t)1, x.PopCount());

        x = a;
        x |= b;        // union = {1,3,4}
        Assert::IsTrue(x.Test(1));
        Assert::IsTrue(x.Test(3));
        Assert::IsTrue(x.Test(4));
        Assert::AreEqual((size_t)3, x.PopCount());

        x = a;
        x ^= b;        // symmetric = {1,4}
        Assert::IsTrue(x.Test(1));
        Assert::IsTrue(x.Test(4));
        Assert::AreEqual((size_t)2, x.PopCount());
    }

    TEST_METHOD(TsShiftLeftRight)
    {
        BS bs{};
        bs.Set(1);
        bs <<= 2;         // expect bit3
        Assert::IsTrue(bs.Test(3));

        bs >>= 1;         // expect bit2
        Assert::IsTrue(bs.Test(2));
    }

    TEST_METHOD(TsParseBinText)
    {
        BS bs{};
        bs.ParseBinText(EckStrAndLen("1'0'1'1"), '1', '0', '\'');

        Assert::IsTrue(bs.Test(0));
        Assert::IsTrue(bs.Test(1));
        Assert::IsFalse(bs.Test(2));
        Assert::IsTrue(bs.Test(3));
        Assert::AreEqual((size_t)3, bs.PopCount());
    }

    TEST_METHOD(TsToInt)
    {
        BS bs{};
        ULONGLONG v = 0x1122334455667788ULL;
        bs.SetMemory(&v, sizeof(v));

        Assert::AreEqual(v, bs.ToInt<ULONGLONG>());
    }

    TEST_METHOD(TsSubInt)
    {
        BS bs{};
        ULONGLONG v = 0xAABBCCDDEEFF0011ULL;
        bs.SetMemory(&v, sizeof(v));

        // 取低 64 bits 的前 16 bits
        int x = bs.SubInt<USHORT>(0);
        Assert::AreEqual(0x0011, x);
    }

    TEST_METHOD(TsSubSet)
    {
        BS bs{};
        bs.Set(3);
        bs.Set(4);
        bs.Set(5);

        auto sub = bs.SubSet(3, 3);

        Assert::IsTrue(sub.Test(0));     // original bit 3
        Assert::IsTrue(sub.Test(1));     // original bit 4
        Assert::IsTrue(sub.Test(2));     // original bit 5
        Assert::AreEqual((size_t)3, sub.PopCount());
    }

    TEST_METHOD(TsTemplateSubSet)
    {
        BS bs{};
        bs.Set(10);
        bs.Set(11);

        auto sub = bs.SubSet<10, 4>();
        //Assert::IsTrue(sub.Test(0));   // original bit10
        //Assert::IsTrue(sub.Test(1));   // original bit11
        //Assert::AreEqual((size_t)2, sub.PopCount());
    }

    TEST_METHOD(TsReverseByte)
    {
        BS bs{};
        BYTE buffer[8] = { 1,2,3,4,5,6,7,8 };
        bs.SetMemory(buffer, 8);
        bs.ReverseByte();

        BYTE* p = (BYTE*)bs.Data();
        Assert::AreEqual((BYTE)1, p[7]);
        Assert::AreEqual((BYTE)8, p[0]);
    }
};
TS_NS_END