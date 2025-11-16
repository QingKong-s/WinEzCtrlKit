#include "pch.h"
#include "../eck/BaseN.h"

using namespace eck;
TS_NS_BEGIN
TEST_CLASS(TsBase64Core)
{
public:
    TEST_METHOD(TsEncode_Empty)
    {
        CRefStrA rs{};
        BYTE buf[1] = {};
        Base64Encode(buf, 0, rs);

        Assert::AreEqual(0, rs.Size());
    }

    TEST_METHOD(TsDecode_Empty)
    {
        CRefBin rb{};
        Base64Decode("", 0, rb);

        Assert::AreEqual((size_t)0, rb.Size());
    }

    TEST_METHOD(TsEncode_Man)
    {
        const char* src = "Man";
        CRefStrA rs{};
        Base64Encode(src, 3, rs);

        Assert::AreEqual(4, rs.Size());
        Assert::AreEqual("TWFu", (const char*)rs.Data());
    }

    TEST_METHOD(TsDecode_TWFu)
    {
        CRefBin rb{};
        Base64Decode("TWFu", 4, rb);

        Assert::AreEqual((size_t)3, rb.Size());
        Assert::AreEqual((BYTE)'M', rb[0]);
        Assert::AreEqual((BYTE)'a', rb[1]);
        Assert::AreEqual((BYTE)'n', rb[2]);
    }

    TEST_METHOD(TsEncode_Ma)
    {
        const char* src = "Ma";
        CRefStrA rs{};
        Base64Encode(src, 2, rs);

        Assert::AreEqual(4, rs.Size());
        Assert::AreEqual("TWE=", (const char*)rs.Data());
    }

    TEST_METHOD(TsDecode_TWE_)
    {
        CRefBin rb{};
        Base64Decode("TWE=", 4, rb);

        Assert::AreEqual((size_t)2, rb.Size());
        Assert::AreEqual((BYTE)'M', rb[0]);
        Assert::AreEqual((BYTE)'a', rb[1]);
    }

    TEST_METHOD(TsEncode_M)
    {
        const char* src = "M";
        CRefStrA rs{};
        Base64Encode(src, 1, rs);

        Assert::AreEqual(4, rs.Size());
        Assert::AreEqual("TQ==", (const char*)rs.Data());
    }

    TEST_METHOD(TsDecode_TQ__)
    {
        CRefBin rb{};
        Base64Decode("TQ==", 4, rb);

        Assert::AreEqual((size_t)1, rb.Size());
        Assert::AreEqual((BYTE)'M', rb[0]);
    }

    TEST_METHOD(TsDecode_NoPad)
    {
        CRefBin rb{};
        Base64Decode("TWE", 3, rb);

        Assert::AreEqual((size_t)2, rb.Size());
        Assert::AreEqual((BYTE)'M', rb[0]);
        Assert::AreEqual((BYTE)'a', rb[1]);
    }

    TEST_METHOD(TsEncodeDecode_RoundTrip)
    {
        BYTE src[] = { 0x01, 0xFE, 0x88, 0x39, 0x7F };

        CRefStrA enc{};
        Base64Encode(src, sizeof(src), enc);

        CRefBin dec{};
        Base64Decode(enc.Data(), (int)enc.Size(), dec);

        Assert::AreEqual(sizeof(src), dec.Size());
        for (size_t i = 0; i < sizeof(src); ++i)
            Assert::AreEqual(src[i], dec[i]);
    }

    TEST_METHOD(TsDecode_StopAtPad)
    {
        CRefBin rb{};
        Base64Decode("AAAA==ZZZZ", 10, rb);

        // "AAAA" → {0,0,0}
        Assert::AreEqual((size_t)3, rb.Size());
        Assert::AreEqual((BYTE)0, rb[0]);
        Assert::AreEqual((BYTE)0, rb[1]);
        Assert::AreEqual((BYTE)0, rb[2]);
    }
};
TS_NS_END