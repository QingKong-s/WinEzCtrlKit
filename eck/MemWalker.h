#pragma once
#include "CByteBuffer.h"
#include "CString.h"

ECK_NAMESPACE_BEGIN
namespace Priv
{
    struct XptMemWalker {};
    struct XptMemWalkerRange : XptMemWalker
    {
        PCBYTE pBase{};
        size_t cbMax{};
        PCBYTE pCurr{};
        constexpr XptMemWalkerRange(PCBYTE b, size_t m, PCBYTE c) noexcept
            : pBase{ b }, cbMax{ m }, pCurr{ c }
        {
        }
    };

    struct CMemoryReaderBase
    {
    protected:
        PCBYTE m_pMem{};
        PCBYTE m_pBase{};
        size_t m_cbMax{};

        EckInline void CheckRange(PCBYTE p)
        {
            if (m_pBase + m_cbMax < p)
                throw XptMemWalkerRange{ m_pBase, m_cbMax, p };
        }
    public:
        CMemoryReaderBase() = default;
        constexpr CMemoryReaderBase(_In_reads_bytes_(cbMax) PCVOID p, size_t cbMax) noexcept
            : m_pMem{ (PCBYTE)p }, m_pBase{ (PCBYTE)p }, m_cbMax{ cbMax }
        {
        }

        constexpr void SetData(PCVOID p, size_t cbMax) noexcept
        {
            m_pMem = (BYTE*)p;
            m_pBase = m_pMem;
            m_cbMax = cbMax;
        }
    };

    class CMemoryWalkerBase
    {
    protected:
        BYTE* m_pMem{};
        BYTE* m_pBase{};
        size_t m_cbMax{};

        EckInline void CheckRange(BYTE* p)
        {
            if (m_pBase + m_cbMax < p)
                throw XptMemWalkerRange{ m_pBase, m_cbMax, p };
        }
    public:
        CMemoryWalkerBase() = default;
        constexpr CMemoryWalkerBase(_Inout_updates_bytes_(cbMax) void* p, size_t cbMax) noexcept
            : m_pMem{ (BYTE*)p }, m_pBase{ (BYTE*)p }, m_cbMax{ cbMax }
        {
        }

        EckInline auto& Write(_In_reads_bytes_(cb) PCVOID pSrc, size_t cb)
        {
            CheckRange(m_pMem + cb);
            memcpy(m_pMem, pSrc, cb);
            m_pMem += cb;
            return *this;
        }
        EckInline auto& WriteRev(_In_reads_bytes_(cb) PCVOID pSrc, size_t cb)
        {
            CheckRange(m_pMem + cb);
            const auto p = (PCBYTE)pSrc;
            for (size_t i = 0; i < cb; ++i)
                *m_pMem++ = p[cb - 1 - i];
            return *this;
        }

        template<class T>
        EckInline auto& operator<<(const T& Data) { return Write(&Data, sizeof(T)); }

        template<class T>
        EckInline auto& WriteRev(const T& Data) { return WriteRev(&Data, sizeof(T)); }

        template<class T, class U>
        EckInline auto& operator<<(const std::basic_string<T, U>& Data)
        {
            return Write(Data.c_str(), (Data.size() + 1) * sizeof(T));
        }
        template<class T, class U>
        EckInline auto& operator<<(std::basic_string_view<T, U> Data)
        {
            return Write(Data.data(), Data.size() * sizeof(T)) << T{};
        }
        template<class T, class U>
        EckInline auto& operator<<(const std::vector<T, U>& Data)
        {
            return Write(Data.data(), Data.size() * sizeof(T));
        }
        template<class T, size_t N>
        EckInline auto& operator<<(std::span<T, N> Data)
        {
            return Write(Data.data(), Data.size() * sizeof(T));
        }
        template<class T>
        EckInline auto& operator<<(const CByteBufferT<T>& Data)
        {
            return Write(Data.Data(), Data.Size());
        }
        template<class T, class U, class V>
        EckInline auto& operator<<(const CStringT<T, U, V>& Data)
        {
            return Write(Data.Data(), Data.ByteSize());
        }

        constexpr void SetData(void* p, size_t cbMax) noexcept
        {
            m_pMem = (BYTE*)p;
            m_pBase = m_pMem;
            m_cbMax = cbMax;
        }
    };

    template<class T>
    class CMemoryWalkerWarpper : public T
    {
    public:
        using T::T;

        using Xpt = XptMemWalker;
        using XptRange = XptMemWalkerRange;

        EckInline auto& Read(_Out_writes_bytes_all_(cb) void* pDst, size_t cb)
        {
            this->CheckRange(this->m_pMem + cb);
            memcpy(pDst, this->m_pMem, cb);
            this->m_pMem += cb;
            return *this;
        }

        EckInline auto& ReadRev(_Out_writes_bytes_all_(cb) void* pDst, size_t cb)
        {
            this->CheckRange(this->m_pMem + cb);
            const auto p = (BYTE*)pDst;
            for (size_t i = 0; i < cb; ++i)
                p[cb - 1 - i] = *this->m_pMem++;
            return *this;
        }

        template<class T>
        EckInline auto& operator>>(_Out_ T& Data) { return Read(&Data, sizeof(Data)); }

        template<class T>
        EckInline auto& ReadRev(_Out_ T& Data) { return ReadRev(&Data, sizeof(T)); }

        template<class T, class U, class V>
        EckInline auto& operator>>(CStringT<T, U, V>& x)
        {
            const int cch = CountStringLength<T>();
            x.ReSize(cch);
            return Read(x.Data(), (cch + 1) * sizeof(T));
        }

        template<class T>
        int CountStringLength()
        {
            auto p = (const T*)Data();
            const auto pEnd = (const T*)(this->m_pBase + this->m_cbMax);
            BOOL bFoundNull{};
            for (; p < pEnd; ++p)
                if (*p == T{})
                {
                    bFoundNull = TRUE;
                    break;
                }
            if (!bFoundNull)
                throw XptMemWalkerRange{ this->m_pBase, this->m_cbMax, (PCBYTE)p };
            return int(p - (const T*)Data());
        }

        template<class T>
        int CountStringLengthSafe() noexcept
        {
            auto p = (const T*)Data();
            const auto pEnd = (const T*)(this->m_pBase + this->m_cbMax);
            for (; p < pEnd; ++p)
                if (*p == T{})
                    break;
            return int(p - (const T*)Data());
        }

        template<class T>
        auto& SkipPointer(_Out_ T*& p)
        {
            this->CheckRange(this->m_pMem + sizeof(T));
            p = (T*)this->m_pMem;
            this->m_pMem += sizeof(T);
            return *this;
        }

        EckInlineNdCe auto Data() noexcept { return this->m_pMem; }

        EckInline auto& operator+=(size_t cb)
        {
            this->CheckRange(this->m_pMem + cb);
            this->m_pMem += cb;
            return *this;
        }
        EckInline auto& operator-=(size_t cb)
        {
            this->CheckRange(this->m_pMem - cb);
            this->m_pMem -= cb;
            return *this;
        }

        EckInlineCe auto& MoveToBegin() noexcept
        {
            this->m_pMem = this->m_pBase;
            return *this;
        }
        EckInlineCe auto& MoveToEnd() noexcept
        {
            this->m_pMem = this->m_pBase + this->m_cbMax;
            return *this;
        }
        EckInline auto& MoveTo(size_t pos)
        {
            this->CheckRange(this->m_pBase + pos);
            this->m_pMem = this->m_pBase + pos;
            return *this;
        }
        EckInlineCe size_t GetRemainingSize() const noexcept { return this->m_pBase + this->m_cbMax - this->m_pMem; }
        EckInlineCe BOOL IsEnd() const noexcept { return this->m_pMem >= this->m_pBase + this->m_cbMax; }
        EckInlineCe size_t GetPosition() const noexcept { return this->m_pMem - this->m_pBase; }
    };
}

using CMemoryReader = Priv::CMemoryWalkerWarpper<Priv::CMemoryReaderBase>;
using CMemoryWalker = Priv::CMemoryWalkerWarpper<Priv::CMemoryWalkerBase>;
ECK_NAMESPACE_END