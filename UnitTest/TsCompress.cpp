#include "pch.h"

#include "../eck/Compress.h"
#include "../eck/Random.h"

using namespace eck;

CXorShift32 Rd{};

static std::vector<uint8_t> GenRandomData(size_t n)
{
    std::vector<uint8_t> buf(n);
    for (size_t i = 0; i < n; i++)
        buf[i] = (Rd.Next<BYTE>() & 0xFF);
    return buf;
}

static void AssertBinEqual(const CRefBin& a, const std::vector<uint8_t>& b)
{
    Assert::AreEqual((size_t)a.Size(), b.size(), L"Size mismatch");
    Assert::IsTrue(memcmp(a.Data(), b.data(), b.size()) == 0, L"Binary content mismatch");
}

TS_NS_BEGIN
TEST_CLASS(TsCompress)
{
public:

    TEST_METHOD(TestZLibSuccess)
    {
        Assert::IsTrue(ZLibSuccess(Z_OK));
        Assert::IsTrue(ZLibSuccess(Z_STREAM_END));
        Assert::IsFalse(ZLibSuccess(Z_DATA_ERROR));
    }

    TEST_METHOD(TestZLibCompressDecompress)
    {
        std::string src = "Hello Zlib Compression!";
        CRefBin compressed, decompressed;

        int ret = ZLibCompress(src.data(), src.size(), compressed);
        Assert::AreEqual(Z_STREAM_END, ret);

        ret = ZLibDecompress(compressed.Data(), compressed.Size(), decompressed);
        Assert::AreEqual(Z_STREAM_END, ret);

        std::vector<uint8_t> expect(src.begin(), src.end());
        AssertBinEqual(decompressed, expect);
    }

    TEST_METHOD(TestZLibRandomData)
    {
        auto data = GenRandomData(4096);
        CRefBin compressed, decompressed;

        int ret = ZLibCompress(data.data(), data.size(), compressed);
        Assert::AreEqual(Z_STREAM_END, ret);

        ret = ZLibDecompress(compressed.Data(), compressed.Size(), decompressed);
        Assert::AreEqual(Z_STREAM_END, ret);

        AssertBinEqual(decompressed, data);
    }

    TEST_METHOD(TestZLibEmpty)
    {
        uint8_t dummy = 1;
        CRefBin compressed, decompressed;

        int ret = ZLibCompress(nullptr, 0, compressed);
        Assert::AreEqual(Z_STREAM_END, ret);

        ret = ZLibDecompress(compressed.Data(), compressed.Size(), decompressed);
        Assert::AreEqual(Z_STREAM_END, ret);

        Assert::AreEqual((size_t)0, decompressed.Size());
    }

    TEST_METHOD(TestGZipCompressDecompress)
    {
        std::string src = "Hello GZip Compression!";
        CRefBin compressed, decompressed;

        int ret = GZipCompress(src.data(), src.size(), compressed);
        Assert::AreEqual(Z_STREAM_END, ret);

        ret = GZipDecompress(compressed.Data(), compressed.Size(), decompressed);
        Assert::AreEqual(Z_STREAM_END, ret);

        std::vector<uint8_t> expect(src.begin(), src.end());
        AssertBinEqual(decompressed, expect);
    }

    TEST_METHOD(TestGZipRandomData)
    {
        auto data = GenRandomData(10000);
        CRefBin compressed, decompressed;

        int ret = GZipCompress(data.data(), data.size(), compressed);
        Assert::AreEqual(Z_STREAM_END, ret);

        ret = GZipDecompress(compressed.Data(), compressed.Size(), decompressed);
        Assert::AreEqual(Z_STREAM_END, ret);

        AssertBinEqual(decompressed, data);
    }

    TEST_METHOD(TestNTCompressDecompress)
    {
        std::string src = "NT native compression test.";
        CRefBin compressed, decompressed;

        NTSTATUS nts = Compress(src.data(), src.size(), compressed, COMPRESSION_FORMAT_LZNT1);
        Assert::IsTrue(NT_SUCCESS(nts));

        nts = Decompress(compressed.Data(), compressed.Size(), decompressed);
        Assert::IsTrue(NT_SUCCESS(nts));

        std::vector<uint8_t> expect(src.begin(), src.end());
        AssertBinEqual(decompressed, expect);
    }

    TEST_METHOD(TestNTRandomData)
    {
        auto data = GenRandomData(8000);

        CRefBin compressed, decompressed;

        NTSTATUS nts = Compress(data.data(), data.size(), compressed, COMPRESSION_FORMAT_LZNT1);
        Assert::IsTrue(NT_SUCCESS(nts));

        nts = Decompress(compressed.Data(), compressed.Size(), decompressed);
        Assert::IsTrue(NT_SUCCESS(nts));

        AssertBinEqual(decompressed, data);
    }

    TEST_METHOD(TestNTBadMagic)
    {
        // 伪造错误数据
        uint8_t fakeData[16] = { 0 };
        CRefBin out;

        NTSTATUS nts = Decompress(fakeData, sizeof(fakeData), out);
        Assert::AreEqual((NTSTATUS)STATUS_UNSUPPORTED_COMPRESSION, nts);
    }
};
TS_NS_END