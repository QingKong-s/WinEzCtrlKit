#pragma once
#include "MemWalker.h"
#include "Crc.h"

ECK_NAMESPACE_BEGIN
constexpr USHORT ArchiveVersion = 1;
constexpr UINT ArchiveSignature = 'a7rV';

struct ARCHIVE_HEADER
{
    UINT uSignature;
    USHORT uVersion;
    USHORT uFlags;
    UINT cbData;    // 不含头
    UINT cbExtra;   // 头后紧跟的额外数据长度
    UINT crc32;
};
enum : USHORT
{
    ACVF_CRC32 = 1 << 0,
};


class CArchiveWriter
{
private:
    CByteBuffer* m_prb{};
    size_t m_posHeader{};
public:
    CArchiveWriter() = default;
    CArchiveWriter(CByteBuffer& rb) noexcept : m_prb{ &rb } {}

    EckInlineCe void SetBuffer(CByteBuffer* prb) noexcept
    {
        if (m_prb != prb)
        {
            m_prb = prb;
            m_posHeader = MaxSizeT;
        }
    }
    EckInlineCe void SetBuffer(CByteBuffer& rb) noexcept { SetBuffer(&rb); }

    // 返回头位置
    size_t BeginHeader(USHORT uFlags = 0, size_t cbExtra = 0) noexcept
    {
        EckAssert(m_posHeader == MaxSizeT);
        const auto pos = Buffer().Size();
        const auto pHdr = (ARCHIVE_HEADER*)
            Buffer().PushBack(sizeof(ARCHIVE_HEADER) + cbExtra);
        *pHdr =
        {
            .uSignature = ArchiveSignature,
            .uVersion = ArchiveVersion,
            .uFlags = uFlags,
            .cbExtra = (UINT)cbExtra,
        };
        m_posHeader = pos;
        return pos;
    }
    // 返回头后的额外数据指针
    void* EndHeader() noexcept
    {
        if (m_posHeader == MaxSizeT)
        {
            EckDbgBreak();
            return nullptr;
        }
        EckAssert(m_posHeader + sizeof(ARCHIVE_HEADER) <= Buffer().Size());
        const auto pHdr = (ARCHIVE_HEADER*)(Buffer().Data() + m_posHeader);
        const auto posHdrEnd = m_posHeader + sizeof(ARCHIVE_HEADER) + pHdr->cbExtra;
        EckAssert(posHdrEnd <= Buffer().Size());
        pHdr->cbData = UINT(Buffer().Size() - posHdrEnd);
        if (pHdr->uFlags & ACVF_CRC32)
            pHdr->crc32 = CalculateCrc32(Buffer().Data() + posHdrEnd, pHdr->cbData);
        m_posHeader = MaxSizeT;
        return Buffer().Data() + posHdrEnd - pHdr->cbExtra;
    }

    EckInlineNdCe CByteBuffer& Buffer() const noexcept { return *m_prb; }
    EckInlineNdCe CByteBuffer& Buffer() noexcept { return *m_prb; }

    template<CcpTrivial T>
    CArchiveWriter& operator<<(const T& x) noexcept
    {
        Buffer().PushBack(&x, sizeof(T));
        return *this;
    }

    template<class T>
    CArchiveWriter& operator<<(std::basic_string_view<T> x) noexcept
    {
        auto p = Buffer().PushBack(sizeof(UINT) + (x.size() + 1) * sizeof(T));
        *(UINT*)p = (UINT)x.size();
        p += sizeof(UINT);
        memcpy(p, x.data(), x.size() * sizeof(T));
        p += (x.size() * sizeof(T));
        *(T*)p = {};
        return *this;
    }
    template<class T, class U, class V>
    CArchiveWriter& operator<<(const std::basic_string<T, U, V>& x) noexcept
    {
        return (*this) << std::basic_string_view<T>{ x };
    }
    template<class T, class U, class V>
    CArchiveWriter& operator<<(const CStringT<T, U, V>& x) noexcept
    {
        return (*this) << x.ToStringView();
    }

    template<class T, size_t N>
    CArchiveWriter& operator<<(std::span<T, N> x) noexcept
    {
        auto p = Buffer().PushBack(sizeof(UINT) * 2 + x.size() * sizeof(T));
        *(UINT*)p = (UINT)x.size();
        p += sizeof(UINT);
        *(UINT*)p = sizeof(T);
        p += sizeof(UINT);
        memcpy(p, x.data(), x.size() * sizeof(T));
        return *this;
    }
    template<class T, class U>
    CArchiveWriter& operator<<(const std::vector<T, U>& x) noexcept
    {
        return (*this) << std::span<const T>{ x.begin(), x.end() };
    }
    template<class U>
    CArchiveWriter& operator<<(const CByteBufferT<U>& x) noexcept
    {
        return (*this) << x.ToSpan();
    }

    CArchiveWriter& WriteRaw(PCVOID p, size_t cb) noexcept
    {
        Buffer().PushBack(p, cb);
        return *this;
    }
    template<class T, size_t N>
    CArchiveWriter& WriteRaw(std::span<T, N> x) noexcept
    {
        return WriteRaw(x.data(), x.size() * sizeof(T));
    }
};

class CArchiveReader
{
public:
    struct Xpt {};
    struct XptElementSize : Xpt
    {
        UINT cbCurr;
        UINT cbDesired;
        constexpr XptElementSize(UINT c, UINT d) noexcept
            : cbCurr{ c }, cbDesired{ d }
        {}
    };
    struct XptRange : Xpt
    {
        PCBYTE pBase{};
        size_t cbMax{};
        PCBYTE pCurr{};
        constexpr XptRange(PCBYTE b, size_t m, PCBYTE c) noexcept
            : pBase{ b }, cbMax{ m }, pCurr{ c }
        {}
    };
    struct XptBadFormat : Xpt {};
    struct XptUnsupportedVersion : Xpt
    {
        USHORT uVersion;
        constexpr XptUnsupportedVersion(USHORT v) noexcept : uVersion{ v } {}
    };
    struct XptCrcError : XptBadFormat {};
private:
    CMemoryReader m_Reader{};
    const ARCHIVE_HEADER* m_pHeader{};

    void CheckHeader()
    {
        m_pHeader = nullptr;
        const ARCHIVE_HEADER* pHdr;
        m_Reader.SkipPointer(pHdr);
        if (pHdr->uSignature != ArchiveSignature)
            throw XptBadFormat{};
        if (pHdr->uVersion != ArchiveVersion)
            throw XptUnsupportedVersion{ pHdr->uVersion };
        if (pHdr->uFlags & ACVF_CRC32)
        {
            const auto pData = m_Reader.Data() + pHdr->cbExtra;
            if (CalculateCrc32(pData, pHdr->cbData) != pHdr->crc32)
                throw XptCrcError{};
        }
        m_pHeader = pHdr;
        m_Reader += pHdr->cbExtra;
    }
public:
    CArchiveReader() = default;
    CArchiveReader(PCVOID p, size_t cb) : m_Reader{ p, cb }
    {
        CheckHeader();
    }

    void SetBuffer(PCVOID p, size_t cb)
    {
        m_Reader.SetData(p, cb);
        CheckHeader();
    }

    EckInlineNdCe auto& MemoryReader() const noexcept { return m_Reader; }
    EckInlineNdCe auto& MemoryReader() noexcept { return m_Reader; }

    template<CcpTrivial T>
    CArchiveReader& operator>>(T& x)
    {
        const auto pos = m_Reader.GetPosition();
        try { m_Reader >> x; }
        catch (const CMemoryReader::XptRange& e)
        {
            m_Reader.Seek(pos);
            throw XptRange{ e.pBase, e.cbMax, e.pCurr };
        }
        return *this;
    }
    template<class T, class U, class V>
    CArchiveReader& operator>>(std::basic_string<T, U, V>& x)
    {
        const auto pos = m_Reader.GetPosition();
        try
        {
            UINT cch;
            m_Reader >> cch;
            x.append((T*)m_Reader.Data(), cch);
            m_Reader += ((cch + 1) * sizeof(T));
        }
        catch (const CMemoryReader::XptRange& e)
        {
            m_Reader.Seek(pos);
            throw XptRange{ e.pBase, e.cbMax, e.pCurr };
        }
        return *this;
    }
    template<class T, class U, class V>
    CArchiveReader& operator>>(CStringT<T, U, V>& x)
    {
        const auto pos = m_Reader.GetPosition();
        try
        {
            UINT cch;
            m_Reader >> cch;
            x.PushBack((T*)m_Reader.Data(), (int)cch);
            m_Reader += ((cch + 1) * sizeof(T));
        }
        catch (const CMemoryReader::XptRange& e)
        {
            m_Reader.Seek(pos);
            throw XptRange{ e.pBase, e.cbMax, e.pCurr };
        }
        return *this;
    }

    template<class T>
    std::span<const T> ReadArray()
    {
        const auto pos = m_Reader.GetPosition();
        try
        {
            UINT c, cbElem;
            m_Reader >> c >> cbElem;
            if (cbElem != sizeof(T))
            {
                m_Reader.Seek(pos);
                throw XptElementSize{ cbElem, sizeof(T) };
            }
            const std::span<const T> r{ (const T*)m_Reader.Data(), c };
            m_Reader += (c * cbElem);
            return r;
        }
        catch (const CMemoryReader::XptRange& e)
        {
            m_Reader.Seek(pos);
            throw XptRange{ e.pBase, e.cbMax, e.pCurr };
        }
    }

    std::span<const BYTE> ReadByteStream()
    {
        const auto pos = m_Reader.GetPosition();
        try
        {
            UINT c, cbElem;
            m_Reader >> c >> cbElem;
            const std::span<const BYTE> r{ m_Reader.Data(), c * cbElem };
            m_Reader += (c * cbElem);
            return r;
        }
        catch (const CMemoryReader::XptRange& e)
        {
            m_Reader.Seek(pos);
            throw XptRange{ e.pBase, e.cbMax, e.pCurr };
        }
    }

    template<class T, class U>
    CArchiveReader& operator>>(std::vector<T, U>& x)
    {
        const auto sp = ReadArray<T>();
        x.insert(x.end(), sp.begin(), sp.end());
        return *this;
    }
    template<class T>
    CArchiveReader& operator>>(std::span<T>& x)
    {
        x = ReadArray<T>();
        return *this;
    }
    template<class U>
    CArchiveReader& operator>>(CByteBufferT<U>& x)
    {
        const auto sp = ReadByteStream();
        x.PushBack(sp.data(), sp.size());
        return *this;
    }
};
ECK_NAMESPACE_END