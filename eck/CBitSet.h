#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
template<size_t N>
class CBitSet
{
public:
#ifdef _WIN64
    using TWord = std::conditional_t<N <= 32, ULONG, ULONGLONG>;
#else
    using TWord = ULONG;
#endif
    constexpr static ptrdiff_t BitsPerWord = 8 * sizeof(TWord);
private:
    TWord m_Bits[(N - 1) / BitsPerWord + 1]{};
public:
    constexpr static ptrdiff_t NB = ARRAYSIZE(m_Bits) - 1;
public:
    class Proxy
    {
        friend class CBitSet;
    private:
        CBitSet& m_That;
        size_t m_idx;

        constexpr Proxy(CBitSet& that, size_t idx) noexcept : m_That(that), m_idx(idx) {}
    public:
        constexpr operator bool() const noexcept { return m_That.Test(m_idx); }
        Proxy& operator=(bool b) noexcept { m_That.Set(m_idx, b); return *this; }
        constexpr bool operator~() noexcept { !m_That.Test(m_idx); return *this; }
        Proxy& operator=(const Proxy& x) noexcept { m_That.Set(m_idx, x); }
    };

    CBitSet() = default;

    CBitSet(_In_reads_bytes_(cb) PCVOID pBin, size_t cb) noexcept
    {
        if (cb > N)
            cb = N;
        memcpy(m_Bits, pBin, cb);
    }

    /// <summary>
    /// 解析二进制文本
    /// </summary>
    /// <param name="psz">二进制文本</param>
    /// <param name="cch">文本长度</param>
    /// <param name="ch1">表示1的字符</param>
    /// <param name="ch0">表示0的字符</param>
    /// <param name="chX">忽略的字符，可用作分隔</param>
    template<CcpStdChar TChar>
    void ParseBinText(_In_reads_(cch) const TChar* psz, int cch,
        TChar ch1, TChar ch0, TChar chX = '\'') noexcept
    {
        EckAssert(ch1 != ch0);
        size_t idxInWord{}, idxCurrWord{};
        TWord u{};
        for (auto p = psz + cch - 1; p >= psz; --p)
        {
            if (*p == chX)
                continue;
            u |= (TWord(*p == ch1) << idxInWord);
            if (++idxInWord == BitsPerWord)
            {
                m_Bits[idxCurrWord++] = u;
                u = TWord{};
                idxInWord = 0;
            }
        }

        if (idxInWord)
            m_Bits[idxCurrWord++] = u;

        for (size_t i = idxCurrWord; i < ARRAYSIZE(m_Bits); ++i)
            m_Bits[i] = TWord{};
    }

    constexpr void Set() noexcept
    {
        for (auto& e : m_Bits)
            e = ~TWord{};
        Trim();
    }
    constexpr void Set(size_t n) noexcept
    {
        EckAssert(n < N);
        m_Bits[n / BitsPerWord] |= (TWord{ 1 } << n % BitsPerWord);
    }
    constexpr void Set(size_t n, BOOL b) noexcept { b ? Set(n) : Clear(n); }

    constexpr void Clear(size_t n) noexcept
    {
        EckAssert(n < N);
        m_Bits[n / BitsPerWord] &= ~(TWord{ 1 } << n % BitsPerWord);
    }
    constexpr void Clear() noexcept
    {
        for (auto& e : m_Bits)
            e = TWord{};
        Trim();
    }

    constexpr CBitSet& operator&=(const CBitSet& x) noexcept
    {
        EckCounter(ARRAYSIZE(m_Bits), i)
            m_Bits[i] &= x.m_Bits[i];
        return *this;
    }
    constexpr CBitSet& operator|=(const CBitSet& x) noexcept
    {
        EckCounter(ARRAYSIZE(m_Bits), i)
            m_Bits[i] |= x.m_Bits[i];
        return *this;
    }
    constexpr CBitSet& operator^=(const CBitSet& x) noexcept
    {
        EckCounter(ARRAYSIZE(m_Bits), i)
            m_Bits[i] ^= x.m_Bits[i];
        return *this;
    }

    constexpr bool operator==(const CBitSet& x) const noexcept
    {
        EckCounter(ARRAYSIZE(m_Bits), i)
            if (m_Bits[i] != x.m_Bits[i])
                return FALSE;
        return TRUE;
    }
    constexpr bool operator!=(const CBitSet& x) const noexcept { return !(*this == x); }

    constexpr CBitSet& operator<<=(size_t n) noexcept
    {
        const auto t = n / BitsPerWord;
        if (t)
            for (ptrdiff_t i = NB; 0 <= i; --i)
                m_Bits[i] = (t <= (size_t)i) ? m_Bits[i - t] : 0;

        if (n %= BitsPerWord)
        {
            for (ptrdiff_t i = NB; 0 < i; --i)
                m_Bits[i] = (m_Bits[i] << n) | (m_Bits[i - 1] >> (BitsPerWord - n));
            m_Bits[0] <<= n;
        }
        Trim();
        return *this;
    }
    constexpr CBitSet& operator>>=(size_t n) noexcept
    {
        const auto t = n / BitsPerWord;
        if (t)
            for (ptrdiff_t i = 0; i <= NB; ++i)
                m_Bits[i] = (t <= size_t(NB - i)) ? m_Bits[i + t] : 0;

        if (n %= BitsPerWord)
        {
            for (ptrdiff_t i = 0; i < NB; ++i)
                m_Bits[i] = (m_Bits[i] >> n) | (m_Bits[i + 1] << (BitsPerWord - n));
            m_Bits[NB] >>= n;
        }
        return *this;
    }

    constexpr [[nodiscard]] CBitSet operator~() const noexcept
    {
        CBitSet x{ *this };
        x.Flip();
        return x;
    }

    EckInlineNdCe Proxy operator[](size_t n) noexcept { return Proxy(*this, n); }

    EckInlineCe void Trim() noexcept
    {
        if constexpr ((N == 0) || (N % BitsPerWord))
            m_Bits[NB] &= (TWord{ 1 } << N % BitsPerWord) - 1;
    }

    [[nodiscard]] size_t PopCount() const noexcept
    {
        size_t n{};
        if constexpr (sizeof(TWord) == 8)
        {
#ifdef _WIN64
            EckCounter(ARRAYSIZE(m_Bits), i)
                n += __popcnt64(m_Bits[i]);
#else
            EckCounter(ARRAYSIZE(m_Bits), i)
                n += (__popcnt(m_Bits[i]) + __popcnt(m_Bits[i] >> 32));
#endif
        }
        else
        {
            EckCounter(ARRAYSIZE(m_Bits), i)
                n += __popcnt(m_Bits[i]);
        }
        return n;
    }

    constexpr [[nodiscard]] BOOL AllOne() const noexcept
    {
        constexpr BOOL bNoPadding = N % BitsPerWord == 0;
        for (size_t i = 0; i < NB + bNoPadding; ++i)
            if (m_Bits[i] != ~TWord{})
                return FALSE;
        return bNoPadding || m_Bits[NB] == (TWord{ 1 } << (N % BitsPerWord)) - 1;
    }
    constexpr [[nodiscard]] BOOL AnyOne() const noexcept
    {
        for (size_t i = 0; i <= NB; ++i)
            if (m_Bits[i] != 0)
                return TRUE;
        return FALSE;
    }
    EckInlineNdCe BOOL NoneOne() const noexcept { return !AnyOne(); }

    constexpr [[nodiscard]] BOOL Test(size_t n) const noexcept
    {
        EckAssert(n < N);
        return (m_Bits[n / BitsPerWord] & (TWord{ 1 } << n % BitsPerWord)) != 0;
    }

    constexpr void Flip() noexcept
    {
        for (auto& e : m_Bits)
            e = ~e;
        Trim();
    }
    constexpr void Flip(size_t n) noexcept
    {
        EckAssert(n < N);
        m_Bits[n / BitsPerWord] ^= (TWord{ 1 } << n % BitsPerWord);
    }

    constexpr void ReverseByte() noexcept
    {
        const auto p = (BYTE*)m_Bits;
        for (size_t i = 0, j = sizeof(m_Bits) - 1; i < j; ++i, --j)
            std::swap(p[i], p[j]);
    }

    EckInlineNdCe const TWord* Data() const noexcept { return m_Bits; }
    EckInlineNdCe TWord* Data() noexcept { return m_Bits; }

    void CopyBits(TWord* pDst, size_t pos, size_t cBits) const noexcept
    {
        EckAssert(pos + cBits <= N);
        if (cBits == 0)
            return;
        const size_t cSrc = ARRAYSIZE(m_Bits);
        const size_t idxSrcInWord = pos / BitsPerWord;
        const size_t nOffset = pos % BitsPerWord;
        const size_t cDst = (cBits + BitsPerWord - 1) / BitsPerWord;
        for (size_t i = 0; i < cDst; ++i)
        {
            const auto l = (idxSrcInWord + i < cSrc) ?
                Data()[idxSrcInWord + i] : TWord{};
            if (nOffset == 0)
                pDst[i] = l;
            else// nOffset in (0, BitsPerWord-1]
            {
                const auto h = (idxSrcInWord + i + 1 < cSrc) ?
                    Data()[idxSrcInWord + i + 1] : TWord{};
                pDst[i] = (l >> nOffset) | (h << (BitsPerWord - nOffset));
            }
        }
        // 清除最后一word中超出cBits的高位
        const auto rem = cBits % BitsPerWord;
        if (rem != 0)
        {
            TWord Mask = (TWord{ 1 } << rem) - 1;
            pDst[cDst - 1] &= Mask;
        }
    }

    [[nodiscard]] CBitSet SubSet(size_t pos, size_t cBits) noexcept
    {
        EckAssert(pos + cBits <= N);
        CBitSet x{};
        CopyBits(x.Data(), pos, cBits);
        x.Trim();
        return x;
    }

    template<size_t NPos, size_t NBits>
        requires requires { NPos < N&& NPos + NBits <= N; }
    [[nodiscard]] CBitSet<NBits> SubSet() noexcept
    {
        CBitSet<NBits> x;
        if constexpr (sizeof(typename CBitSet<NBits>::TWord) < sizeof(TWord))
        {
            CBitSet t{};
            CopyBits(t.Data(), NPos, NBits);
            memcpy(x.Data(), t.Data(), sizeof(CBitSet<NBits>::m_Bits));
        }
        else
            CopyBits((TWord*)x.Data(), NPos, NBits);
        x.Trim();
        return x;
    }

    template<std::integral TInt>
    [[nodiscard]] TInt SubInt(size_t pos) const noexcept
    {
        EckAssert(pos + sizeof(TInt) * 8 < N);
        using TTemp = std::conditional_t < sizeof(TInt) < sizeof(TWord), TWord, ULONGLONG > ;
        TTemp x{};
        CopyBits((TWord*)&x, pos, sizeof(TInt) * 8);
        return (TInt)x;
    }

    template<std::integral TInt>
    EckInlineNdCe TInt ToInt() const noexcept
    {
        if (sizeof(TInt) * 8 <= N)
            return *(TInt*)m_Bits;
        else
            return TInt(*(TWord*)m_Bits);
    }

    void SetMemory(_In_reads_bytes_(cb) PCVOID pBin, size_t cb) noexcept
    {
        if (cb > sizeof(m_Bits))
            cb = sizeof(m_Bits);
        memcpy(m_Bits, pBin, cb);
        const auto pEnd = (BYTE*)m_Bits + sizeof(m_Bits);
        for (BYTE* p = (BYTE*)m_Bits + cb; p < pEnd; ++p)
            *p = 0;
    }
};
ECK_NAMESPACE_END