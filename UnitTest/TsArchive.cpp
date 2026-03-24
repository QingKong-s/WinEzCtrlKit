#include "pch.h"
#include "../eck/Archive.h"

using namespace eck;

#define ASSERT_THROWS_ECK(expr, ExcType) \
    { bool _caught = false; \
      try { expr; } \
      catch (const ExcType&) { _caught = true; } \
      Assert::IsTrue(_caught, L"Expected " #ExcType " was not thrown"); }

TS_NS_BEGIN
static CByteBuffer MakeArchive(auto writeBody)
{
    CByteBuffer buf;
    CArchiveWriter w(buf);
    w.BeginHeader();
    writeBody(w);
    w.EndHeader();
    return buf;
}

static CByteBuffer MakeArchiveCrc(auto writeBody)
{
    CByteBuffer buf;
    CArchiveWriter w(buf);
    w.BeginHeader(ACVF_CRC32);
    writeBody(w);
    w.EndHeader();
    return buf;
}

TEST_CLASS(TsArchive)
{
public:

    TEST_METHOD(HeaderSignatureAndVersionAreCorrect)
    {
        CByteBuffer buf;
        CArchiveWriter w(buf);
        w.BeginHeader();
        w.EndHeader();

        const auto* hdr = reinterpret_cast<const ARCHIVE_HEADER*>(buf.Data());
        Assert::AreEqual(ArchiveSignature, hdr->uSignature);
        Assert::AreEqual(ArchiveVersion, hdr->uVersion);
    }

    TEST_METHOD(CbDataReflectsWrittenBytes)
    {
        CByteBuffer buf;
        CArchiveWriter w(buf);
        w.BeginHeader();
        UINT payload = 0xDEADBEEF;
        w << payload;
        w.EndHeader();

        const auto* hdr = reinterpret_cast<const ARCHIVE_HEADER*>(buf.Data());
        Assert::AreEqual((UINT)sizeof(UINT), hdr->cbData);
    }

    TEST_METHOD(EmptyBodyHasCbDataZero)
    {
        CByteBuffer buf;
        CArchiveWriter w(buf);
        w.BeginHeader();
        w.EndHeader();

        const auto* hdr = reinterpret_cast<const ARCHIVE_HEADER*>(buf.Data());
        Assert::AreEqual((UINT)0, hdr->cbData);
    }

    TEST_METHOD(CbExtraIsStoredCorrectly)
    {
        constexpr size_t cbExtra = 16;
        CByteBuffer buf;
        CArchiveWriter w(buf);
        void* pExtra = (void*)(size_t)w.BeginHeader(0, cbExtra); // 先记pos
        // EndHeader 返回 extra 指针
        void* pExtraRet = w.EndHeader();

        const auto* hdr = reinterpret_cast<const ARCHIVE_HEADER*>(buf.Data());
        Assert::AreEqual((UINT)cbExtra, hdr->cbExtra);
        // 额外数据紧跟 ARCHIVE_HEADER
        Assert::AreEqual(
            (uintptr_t)(buf.Data() + sizeof(ARCHIVE_HEADER)),
            (uintptr_t)pExtraRet);
    }

    TEST_METHOD(ExtraDataIsIncludedInBuffer)
    {
        constexpr size_t cbExtra = 8;
        CByteBuffer buf;
        CArchiveWriter w(buf);
        w.BeginHeader(0, cbExtra);
        UINT payload = 42;
        w << payload;
        void* pExtra = w.EndHeader();

        // 写入额外数据
        memset(pExtra, 0xAB, cbExtra);

        const BYTE* pExpected = buf.Data() + sizeof(ARCHIVE_HEADER);
        for (size_t i = 0; i < cbExtra; ++i)
            Assert::AreEqual((BYTE)0xAB, pExpected[i]);
    }

    TEST_METHOD(Crc32FlagSetsCrc32Field)
    {
        auto buf = MakeArchiveCrc([](CArchiveWriter& w)
            {
                UINT v = 1234;
                w << v;
            });

        const auto* hdr = reinterpret_cast<const ARCHIVE_HEADER*>(buf.Data());
        Assert::IsTrue((hdr->uFlags & ACVF_CRC32) != 0);
        Assert::AreNotEqual((UINT)0, hdr->crc32);
    }

    TEST_METHOD(NoCrcFlagLeavesCrc32Zero)
    {
        auto buf = MakeArchive([](CArchiveWriter& w)
            {
                UINT v = 1234;
                w << v;
            });

        const auto* hdr = reinterpret_cast<const ARCHIVE_HEADER*>(buf.Data());
        Assert::IsFalse((hdr->uFlags & ACVF_CRC32) != 0);
        Assert::AreEqual((UINT)0, hdr->crc32);
    }

    TEST_METHOD(EndHeaderWithoutBeginHeaderReturnsNullptr)
    {
        CByteBuffer buf;
        CArchiveWriter w(buf);
        // m_posHeader 初始为 0，不是 MaxSizeT，
        // 此处验证不崩溃且返回 nullptr
        // 注：初始值 0 != MaxSizeT，需确认构造时置为 MaxSizeT
        // 若实现中构造不置 MaxSizeT，此用例可改为验证 Assert 触发
        void* ret = w.EndHeader();
        Assert::IsNull(ret);
    }

    TEST_METHOD(SetBufferSwitchesBacking)
    {
        CByteBuffer buf1, buf2;
        CArchiveWriter w(buf1);
        w.SetBuffer(buf2);

        w.BeginHeader();
        UINT v = 99;
        w << v;
        w.EndHeader();

        Assert::IsTrue(buf1.Size() == 0);
        Assert::IsTrue(buf2.Size() > 0);
    }

    TEST_METHOD(SetBufferWithSamePointerIsNoop)
    {
        CByteBuffer buf;
        CArchiveWriter w(buf);
        w.BeginHeader();
        w.SetBuffer(buf);   // 不应重置 m_posHeader
        UINT v = 7;
        w << v;
        w.EndHeader();       // 应正常完成

        const auto* hdr = reinterpret_cast<const ARCHIVE_HEADER*>(buf.Data());
        Assert::AreEqual((UINT)sizeof(UINT), hdr->cbData);
    }

    TEST_METHOD(TrivialUINT)
    {
        CByteBuffer buf;
        CArchiveWriter w(buf);
        w.BeginHeader();
        UINT val = 0xCAFEBABE;
        w << val;
        w.EndHeader();

        const auto* hdr = reinterpret_cast<const ARCHIVE_HEADER*>(buf.Data());
        Assert::AreEqual((UINT)sizeof(UINT), hdr->cbData);
        const UINT* pData = reinterpret_cast<const UINT*>(
            buf.Data() + sizeof(ARCHIVE_HEADER));
        Assert::AreEqual(0xCAFEBABEu, *pData);
    }

    TEST_METHOD(TrivialStruct)
    {
        struct Pod { int a; float b; };
        CByteBuffer buf;
        CArchiveWriter w(buf);
        w.BeginHeader();
        Pod p{ 42, 3.14f };
        w << p;
        w.EndHeader();

        const auto* hdr = reinterpret_cast<const ARCHIVE_HEADER*>(buf.Data());
        Assert::AreEqual((UINT)sizeof(Pod), hdr->cbData);
    }

    TEST_METHOD(StringViewNarrow)
    {
        CByteBuffer buf;
        CArchiveWriter w(buf);
        w.BeginHeader();
        std::string_view sv = "hello";
        w << sv;
        w.EndHeader();

        // 格式: UINT cch + chars + null terminator
        const BYTE* p = buf.Data() + sizeof(ARCHIVE_HEADER);
        UINT cch = *reinterpret_cast<const UINT*>(p);
        Assert::AreEqual(5u, cch);
        Assert::AreEqual('h', (char)p[sizeof(UINT)]);
        Assert::AreEqual('\0', (char)p[sizeof(UINT) + 5]);
    }

    TEST_METHOD(StringViewWide)
    {
        CByteBuffer buf;
        CArchiveWriter w(buf);
        w.BeginHeader();
        std::wstring_view sv = L"abc";
        w << sv;
        w.EndHeader();

        const BYTE* p = buf.Data() + sizeof(ARCHIVE_HEADER);
        UINT cch = *reinterpret_cast<const UINT*>(p);
        Assert::AreEqual(3u, cch);
    }

    TEST_METHOD(StdString)
    {
        CByteBuffer buf;
        CArchiveWriter w(buf);
        w.BeginHeader();
        std::string s = "world";
        w << s;
        w.EndHeader();

        const BYTE* p = buf.Data() + sizeof(ARCHIVE_HEADER);
        UINT cch = *reinterpret_cast<const UINT*>(p);
        Assert::AreEqual(5u, cch);
    }

    TEST_METHOD(SpanOfInt)
    {
        CByteBuffer buf;
        CArchiveWriter w(buf);
        w.BeginHeader();
        std::vector<int> v = { 1, 2, 3, 4 };
        w << std::span<int>{ v };
        w.EndHeader();

        // 格式: UINT count + UINT cbElem + data
        const BYTE* p = buf.Data() + sizeof(ARCHIVE_HEADER);
        Assert::AreEqual(4u, *reinterpret_cast<const UINT*>(p));
        Assert::AreEqual((UINT)sizeof(int),
            *reinterpret_cast<const UINT*>(p + sizeof(UINT)));
    }

    TEST_METHOD(VectorUsesSpanPath)
    {
        CByteBuffer buf1, buf2;
        CArchiveWriter w1(buf1), w2(buf2);
        std::vector<int> v = { 10, 20 };
        w1.BeginHeader(); w1 << v;            w1.EndHeader();
        w2.BeginHeader(); w2 << std::span<int>{ v }; w2.EndHeader();
        Assert::AreEqual(buf1.Size(), buf2.Size());
        Assert::AreEqual(0, memcmp(buf1.Data(), buf2.Data(), buf1.Size()));
    }

    TEST_METHOD(WriteRawRawBytes)
    {
        CByteBuffer buf;
        CArchiveWriter w(buf);
        w.BeginHeader();
        const BYTE raw[] = { 0xDE, 0xAD, 0xBE, 0xEF };
        w.WriteRaw(raw, sizeof(raw));
        w.EndHeader();

        const BYTE* p = buf.Data() + sizeof(ARCHIVE_HEADER);
        Assert::AreEqual((BYTE)0xDE, p[0]);
        Assert::AreEqual((BYTE)0xEF, p[3]);
    }

    TEST_METHOD(MultipleFieldsAreConcatenated)
    {
        CByteBuffer buf;
        CArchiveWriter w(buf);
        w.BeginHeader();
        BYTE a = 1; UINT b = 2; USHORT c = 3;
        w << a << b << c;
        w.EndHeader();

        const auto* hdr = reinterpret_cast<const ARCHIVE_HEADER*>(buf.Data());
        Assert::AreEqual(
            (UINT)(sizeof(BYTE) + sizeof(UINT) + sizeof(USHORT)),
            hdr->cbData);
    }

    TEST_METHOD(BadSignatureThrowsXptBadFormat)
    {
        auto buf = MakeArchive([](CArchiveWriter&) {});
        // 破坏签名
        reinterpret_cast<ARCHIVE_HEADER*>(buf.Data())->uSignature = 0;

        ASSERT_THROWS_ECK(
            CArchiveReader(buf.Data(), buf.Size()),
            CArchiveReader::XptBadFormat);
    }

    TEST_METHOD(WrongVersionThrowsXptUnsupportedVersion)
    {
        auto buf = MakeArchive([](CArchiveWriter&) {});
        reinterpret_cast<ARCHIVE_HEADER*>(buf.Data())->uVersion = 0xFF;

        bool caught = false;
        try { CArchiveReader(buf.Data(), buf.Size()); }
        catch (const CArchiveReader::XptUnsupportedVersion& e)
        {
            caught = true;
            Assert::AreEqual((USHORT)0xFF, e.uVersion);
        }
        Assert::IsTrue(caught);
    }

    TEST_METHOD(CorruptedCrcThrowsXptCrcError)
    {
        auto buf = MakeArchiveCrc([](CArchiveWriter& w)
            {
                UINT v = 12345;
                w << v;
            });
        // 破坏 CRC
        reinterpret_cast<ARCHIVE_HEADER*>(buf.Data())->crc32 ^= 0xFFFFFFFF;

        ASSERT_THROWS_ECK(
            CArchiveReader(buf.Data(), buf.Size()),
            CArchiveReader::XptCrcError);
    }

    TEST_METHOD(ValidCrcDoesNotThrow)
    {
        auto buf = MakeArchiveCrc([](CArchiveWriter& w)
            {
                UINT v = 99;
                w << v;
            });
        // 不应抛出
        CArchiveReader r(buf.Data(), buf.Size());
        (void)r;
    }

    TEST_METHOD(TruncatedBufferThrowsXptRange)
    {
        auto buf = MakeArchive([](CArchiveWriter&) {});
        // 提供比头更小的缓冲区
        ASSERT_THROWS_ECK(
            CArchiveReader(buf.Data(), sizeof(ARCHIVE_HEADER) - 1),
            CArchiveReader::XptRange);
    }

    TEST_METHOD(UINTRoundTrip)
    {
        auto buf = MakeArchive([](CArchiveWriter& w)
            {
                UINT v = 0xABCD1234;
                w << v;
            });

        CArchiveReader r(buf.Data(), buf.Size());
        UINT out{};
        r >> out;
        Assert::AreEqual(0xABCD1234u, out);
    }

    TEST_METHOD(MultipleTrivialsRoundTrip)
    {
        auto buf = MakeArchive([](CArchiveWriter& w)
            {
                BYTE  a = 0xAA;
                UINT  b = 0xBBBBBBBB;
                float c = 1.5f;
                w << a << b << c;
            });

        CArchiveReader r(buf.Data(), buf.Size());
        BYTE a{}; UINT b{}; float c{};
        r >> a >> b >> c;
        Assert::AreEqual((BYTE)0xAA, a);
        Assert::AreEqual(0xBBBBBBBBu, b);
        Assert::AreEqual(1.5f, c);
    }

    TEST_METHOD(NarrowStringRoundTrip)
    {
        const std::string original = "hello, archive";
        auto buf = MakeArchive([&](CArchiveWriter& w) { w << original; });

        CArchiveReader r(buf.Data(), buf.Size());
        std::string out;
        r >> out;
        Assert::AreEqual(original, out);
    }

    TEST_METHOD(WideStringRoundTrip)
    {
        const std::wstring original = L"宽字符测试";
        auto buf = MakeArchive([&](CArchiveWriter& w) { w << original; });

        CArchiveReader r(buf.Data(), buf.Size());
        std::wstring out;
        r >> out;
        Assert::AreEqual(original, out);
    }

    TEST_METHOD(EmptyStringRoundTrip)
    {
        const std::string original;
        auto buf = MakeArchive([&](CArchiveWriter& w) { w << original; });

        CArchiveReader r(buf.Data(), buf.Size());
        std::string out;
        r >> out;
        Assert::AreEqual(original, out);
    }

    TEST_METHOD(VectorOfIntRoundTrip)
    {
        const std::vector<int> original = { 10, 20, 30, 40, 50 };
        auto buf = MakeArchive([&](CArchiveWriter& w) { w << original; });

        CArchiveReader r(buf.Data(), buf.Size());
        std::vector<int> out;
        r >> out;
        Assert::AreEqual(original.size(), out.size());
        for (size_t i = 0; i < original.size(); ++i)
            Assert::AreEqual(original[i], out[i]);
    }

    TEST_METHOD(VectorOfFloatRoundTrip)
    {
        const std::vector<float> original = { 1.1f, 2.2f, 3.3f };
        auto buf = MakeArchive([&](CArchiveWriter& w) { w << original; });

        CArchiveReader r(buf.Data(), buf.Size());
        std::vector<float> out;
        r >> out;
        for (size_t i = 0; i < original.size(); ++i)
            Assert::AreEqual(original[i], out[i]);
    }

    TEST_METHOD(SpanRoundTrip)
    {
        const std::vector<UINT> src = { 1, 2, 3 };
        auto buf = MakeArchive([&](CArchiveWriter& w)
            {
                w << std::span<const UINT>{ src };
            });

        CArchiveReader r(buf.Data(), buf.Size());
        std::span<const UINT> out;
        r >> out;
        Assert::AreEqual(src.size(), out.size());
        for (size_t i = 0; i < src.size(); ++i)
            Assert::AreEqual(src[i], out[i]);
    }

    TEST_METHOD(MixedTypesRoundTrip)
    {
        const UINT   u = 42;
        const float  f = 2.718f;
        const std::string s = "mixed";
        const std::vector<int> v = { -1, 0, 1 };

        auto buf = MakeArchive([&](CArchiveWriter& w)
            {
                w << u << f << s << v;
            });

        CArchiveReader r(buf.Data(), buf.Size());
        UINT   ou{}; float of{}; std::string os; std::vector<int> ov;
        r >> ou >> of >> os >> ov;

        Assert::AreEqual(u, ou);
        Assert::AreEqual(f, of);
        Assert::AreEqual(s, os);
        Assert::AreEqual(v.size(), ov.size());
    }

    TEST_METHOD(CrcArchiveRoundTrip)
    {
        const UINT original = 9999;
        auto buf = MakeArchiveCrc([&](CArchiveWriter& w) { w << original; });

        CArchiveReader r(buf.Data(), buf.Size());
        UINT out{};
        r >> out;
        Assert::AreEqual(original, out);
    }

    TEST_METHOD(ExtraDataDoesNotAffectPayloadRead)
    {
        constexpr size_t cbExtra = 12;
        CByteBuffer buf;
        CArchiveWriter w(buf);
        w.BeginHeader(0, cbExtra);
        UINT payload = 777;
        w << payload;
        void* pExtra = w.EndHeader();
        memset(pExtra, 0xFF, cbExtra);   // 填充额外数据

        CArchiveReader r(buf.Data(), buf.Size());
        UINT out{};
        r >> out;
        Assert::AreEqual(777u, out);
    }

    TEST_METHOD(ReadPastEndThrowsXptRange)
    {
        auto buf = MakeArchive([](CArchiveWriter& w)
            {
                BYTE b = 1;
                w << b;
            });

        CArchiveReader r(buf.Data(), buf.Size());
        BYTE b1{};
        r >> b1;   // 正常
        ASSERT_THROWS_ECK(r >> b1, CArchiveReader::XptRange);
    }

    TEST_METHOD(PositionRolledBackAfterXptRange)
    {
        auto buf = MakeArchive([](CArchiveWriter& w)
            {
                BYTE b = 0xCC;
                w << b;
            });

        CArchiveReader r(buf.Data(), buf.Size());
        UINT big{};
        // 尝试读 4 字节但只有 1 字节，应回滚
        try { r >> big; }
        catch (const CArchiveReader::XptRange&) {}

        // 回滚后仍能读 1 字节
        BYTE b{};
        r >> b;
        Assert::AreEqual((BYTE)0xCC, b);
    }

    TEST_METHOD(ReadStringPastEndRollsBack)
    {
        // 写 cch=100 但后面数据不足
        CByteBuffer buf;
        CArchiveWriter w(buf);
        w.BeginHeader();
        UINT fakeLen = 100;
        w << fakeLen;  // 只写长度，不写字符串内容
        w.EndHeader();

        CArchiveReader r(buf.Data(), buf.Size());
        std::string s;
        ASSERT_THROWS_ECK(r >> s, CArchiveReader::XptRange);
    }

    TEST_METHOD(SetBufferResetsReaderState)
    {
        auto buf = MakeArchive([](CArchiveWriter& w)
            {
                UINT v = 55;
                w << v;
            });

        CArchiveReader r(buf.Data(), buf.Size());
        UINT v1{};
        r >> v1;
        Assert::AreEqual(55u, v1);

        // 重新设置 buffer，游标重置到数据起点
        r.SetBuffer(buf.Data(), buf.Size());
        UINT v2{};
        r >> v2;
        Assert::AreEqual(55u, v2);
    }

    TEST_METHOD(ElementSizeMismatchThrowsXptElementSize)
    {
        // 写入 int（4 字节），用 short 读取
        const std::vector<int> src = { 1, 2, 3 };
        auto buf = MakeArchive([&](CArchiveWriter& w) { w << src; });

        CArchiveReader r(buf.Data(), buf.Size());
        bool caught = false;
        try { r.ReadArray<short>(); }
        catch (const CArchiveReader::XptElementSize& e)
        {
            caught = true;
            Assert::AreEqual((UINT)sizeof(int), e.cbCurr);
            Assert::AreEqual((UINT)sizeof(short), e.cbDesired);
        }
        Assert::IsTrue(caught);
    }

    TEST_METHOD(ElementSizeMismatchRollsBackPosition)
    {
        const std::vector<int> src = { 7, 8 };
        auto buf = MakeArchive([&](CArchiveWriter& w) { w << src; });

        CArchiveReader r(buf.Data(), buf.Size());
        try { r.ReadArray<short>(); }
        catch (const CArchiveReader::XptElementSize&) {}

        // 回滚后可正确读取
        std::vector<int> out;
        r >> out;
        Assert::AreEqual(2u, (UINT)out.size());
        Assert::AreEqual(7, out[0]);
        Assert::AreEqual(8, out[1]);
    }

    TEST_METHOD(ReadByteStreamReturnsCorrectByteCount)
    {
        const std::vector<UINT> src = { 0xAABBCCDD, 0x11223344 };
        auto buf = MakeArchive([&](CArchiveWriter& w) { w << src; });

        CArchiveReader r(buf.Data(), buf.Size());
        const auto sp = r.ReadByteStream();
        Assert::AreEqual(2 * sizeof(UINT), sp.size());
    }

    TEST_METHOD(SingleByteRoundTrip)
    {
        auto buf = MakeArchive([](CArchiveWriter& w)
            {
                BYTE b = 0xFF;
                w << b;
            });
        CArchiveReader r(buf.Data(), buf.Size());
        BYTE b{};
        r >> b;
        Assert::AreEqual((BYTE)0xFF, b);
    }

    TEST_METHOD(ZeroValueRoundTrip)
    {
        auto buf = MakeArchive([](CArchiveWriter& w)
            {
                UINT v = 0;
                w << v;
            });
        CArchiveReader r(buf.Data(), buf.Size());
        UINT v{};
        r >> v;
        Assert::AreEqual(0u, v);
    }

    TEST_METHOD(EmptyVectorRoundTrip)
    {
        const std::vector<int> src;
        auto buf = MakeArchive([&](CArchiveWriter& w) { w << src; });

        CArchiveReader r(buf.Data(), buf.Size());
        std::vector<int> out;
        r >> out;
        Assert::IsTrue(out.empty());
    }

    TEST_METHOD(LargeStringRoundTrip)
    {
        const std::string original(4096, 'X');
        auto buf = MakeArchive([&](CArchiveWriter& w) { w << original; });

        CArchiveReader r(buf.Data(), buf.Size());
        std::string out;
        r >> out;
        Assert::AreEqual(original, out);
    }

    TEST_METHOD(LargeVectorRoundTrip)
    {
        std::vector<int> original(10000);
        for (int i = 0; i < 10000; ++i) original[i] = i;
        auto buf = MakeArchive([&](CArchiveWriter& w) { w << original; });

        CArchiveReader r(buf.Data(), buf.Size());
        std::vector<int> out;
        r >> out;
        Assert::AreEqual(original.size(), out.size());
        Assert::AreEqual(original[9999], out[9999]);
    }
};
TS_NS_END