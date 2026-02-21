#pragma once
#pragma warning (disable:4996)
#include "CAllocator.h"
#include "PathUtility.h"
#include "Utility.h"

ECK_NAMESPACE_BEGIN
template<CcpStdChar TChar>
using TStringDefaultAllocator = CDefaultAllocator<TChar, int>;

template<CcpStdChar TChar_, class TCharTraits_, class TAllocator_>
class CStringT;

template<class TCharTraits, class TAllocator>
CStringT<WCHAR, TCharTraits, TAllocator> StrX2W(PCSTR pszText, int cch, int uCP) noexcept;

template<CcpStdChar TChar_>
struct CCharTraits
{
    using TChar = void;
};

template<>
struct CCharTraits<WCHAR>
{
    using TChar = WCHAR;
    EckInlineCe static void AssignChar(PWSTR psz, WCHAR ch) noexcept { *psz = ch; }
    EckInlineCe static WCHAR CharNull() noexcept { return L'\0'; }
    EckInlineCe static void Cut(PWSTR psz, int cch) noexcept { AssignChar(psz + cch, CharNull()); }
    EckInlineCe static WCHAR CharSpace() noexcept { return L' '; }
    EckInline static int FormatV(PWSTR pszBuf, PCWSTR pszFmt, va_list vl) noexcept { return vswprintf(pszBuf, pszFmt, vl); }
    EckInline static int GetFormatLengthV(PCWSTR pszFmt, va_list vl) noexcept { return _vscwprintf(pszFmt, vl); }
};

template<>
struct CCharTraits<CHAR>
{
    using TChar = CHAR;
    EckInlineCe static void AssignChar(PSTR psz, CHAR ch) noexcept { *psz = ch; }
    EckInlineCe static CHAR CharNull() noexcept { return '\0'; }
    EckInlineCe static void Cut(PSTR psz, int cch) noexcept { AssignChar(psz + cch, CharNull()); }
    EckInlineCe static CHAR CharSpace() noexcept { return ' '; }
    EckInline static int FormatV(PSTR pszBuf, PCSTR pszFmt, va_list vl) noexcept { return vsprintf(pszBuf, pszFmt, vl); }
    EckInline static int GetFormatLengthV(PCSTR pszFmt, va_list vl) noexcept { return _vscprintf(pszFmt, vl); }
};

template<CcpStdChar TChar>
using TStringDefaultTraits = CCharTraits<TChar>;

template<
    CcpStdChar TChar_,
    class TCharTraits_ = TStringDefaultTraits<TChar_>,
    class TAllocator_ = TStringDefaultAllocator<TChar_>
>
class CStringT
{
public:
    using TChar = TChar_;
    using TCharTraits = TCharTraits_;
    using TAllocator = TAllocator_;

    using TAllocatorTraits = CAllocatorTraits<TAllocator>;
    using TPointer = TChar*;
    using TConstPointer = const TChar*;

    using TIterator = TPointer;
    using TConstIterator = TConstPointer;
    using TReverseIterator = std::reverse_iterator<TIterator>;
    using TConstReverseIterator = std::reverse_iterator<TConstIterator>;

    using TNtString = std::conditional_t<std::is_same_v<TChar, CHAR>, ANSI_STRING, UNICODE_STRING>;

    static_assert(std::is_same_v<typename TAllocatorTraits::size_type, int>);
private:
    union
    {
        TPointer m_pszText;
        TChar m_szLocal[16 / sizeof(TChar)]{};
    };
    int m_cchText{};
    int m_cchCapacity{ ARRAYSIZE(m_szLocal) };

    [[no_unique_address]] TAllocator m_Alloc{};

    static constexpr void ResetThat(CStringT& x)
    {
        x.m_pszText = nullptr;// 保证清空指针和截断sso缓冲区
        x.m_cchText = 0;
        x.m_cchCapacity = LocalBufferSize;
    }
public:
    constexpr static int LocalBufferSize = ARRAYSIZE(m_szLocal);
    constexpr static int EnsureNotLocalBufferSize = ARRAYSIZE(m_szLocal) * 3 / 2;

    CStringT() = default;

    explicit CStringT(const TAllocator& Al) : m_Alloc{ Al } {}

    explicit CStringT(int cchInit)
    {
        ReSize(cchInit);
    }

    CStringT(int cchInit, const TAllocator& Al) : m_Alloc{ Al }
    {
        ReSize(cchInit);
    }

    CStringT(TConstPointer psz, int cchText, const TAllocator& Al = TAllocator{}) : m_Alloc{ Al }
    {
        Assign(psz, cchText);
    }

    CStringT(TConstPointer psz, const TAllocator& Al = TAllocator{})
        : CStringT(psz, psz ? (int)TcsLength(psz) : 0, Al)
    {}

    CStringT(const CStringT& x)
        : m_Alloc{ TAllocatorTraits::select_on_container_copy_construction(x.m_Alloc) }
    {
        Assign(x.Data(), x.Size());
    }

    CStringT(CStringT&& x) noexcept
        : m_cchText{ x.m_cchText }, m_cchCapacity{ x.m_cchCapacity }, m_Alloc{ std::move(x.m_Alloc) }
    {
        std::copy(std::begin(x.m_szLocal), std::end(x.m_szLocal), std::begin(m_szLocal));
        ResetThat(x);
    }

    template<class TTraits, class TAllocator1>
    CStringT(const std::basic_string<TChar, TTraits, TAllocator1>& s, const TAllocator& Al = TAllocator{})
        : CStringT(s.data(), (int)s.size(), Al)
    {}

    template<class TTraits>
    CStringT(std::basic_string_view<TChar, TTraits> sv, const TAllocator& Al = TAllocator{})
        : CStringT(sv.data(), (int)sv.size(), Al)
    {}

    CStringT(TNtString nts, const TAllocator& Al = TAllocator{})
        :CStringT(nts.Buffer, (int)nts.Length, Al)
    {}

    ~CStringT()
    {
        if (!IsLocal())
            m_Alloc.deallocate(m_pszText, m_cchCapacity);
    }

    EckInline CStringT& operator=(TConstPointer pszSrc)
    {
        if (pszSrc)
            Assign(pszSrc);
        else
            Clear();
        return *this;
    }

    EckInline CStringT& operator=(const CStringT& x)
    {
        if (this == &x)
            return *this;
        if constexpr (TAllocatorTraits::propagate_on_container_copy_assignment::value)
        {
            if constexpr (TAllocatorTraits::is_always_equal::value)
                m_Alloc = x.m_Alloc;
            else if (m_Alloc != x.m_Alloc)
                m_Alloc = x.m_Alloc;
        }

        Assign(x.Data(), x.Size());
        return *this;
    }

    EckInline CStringT& operator=(CStringT&& x) noexcept
    {
        if (this == &x)
            return *this;
        if constexpr (TAllocatorTraits::propagate_on_container_move_assignment::value)
        {
            if constexpr (TAllocatorTraits::is_always_equal::value)
                m_Alloc = std::move(x.m_Alloc);
            else if (m_Alloc != x.m_Alloc)
            {
                if (!IsLocal())
                {
                    m_Alloc.deallocate(m_pszText, m_cchCapacity);
                    ResetThat(*this);
                }
                m_Alloc = std::move(x.m_Alloc);
            }
        }
        else if constexpr (!TAllocatorTraits::is_always_equal::value)
            if (m_Alloc != x.m_Alloc)
            {
                Assign(x.Data(), x.Size());
                return *this;
            }
        std::swap(m_szLocal, x.m_szLocal);
        std::swap(m_cchText, x.m_cchText);
        std::swap(m_cchCapacity, x.m_cchCapacity);
        return *this;
    }

    template<class TTraits, class TAllocator1>
    EckInline CStringT& operator=(const std::basic_string<TChar, TTraits, TAllocator1>& x)
    {
        Assign(x.data(), (int)x.size());
        return *this;
    }

    template<class TTraits>
    EckInline CStringT& operator=(std::basic_string_view<TChar, TTraits> x)
    {
        Assign(x.data(), (int)x.size());
        return *this;
    }

    EckInline CStringT& operator=(TNtString x) { Assign(x.Buffer, (int)x.Length); }

    EckInlineNdCe TChar& At(int x) noexcept { EckAssert(x >= 0 && x < Size()); return *(Data() + x); }
    EckInlineNdCe TChar At(int x) const noexcept { EckAssert(x >= 0 && x < Size()); return *(Data() + x); }
    EckInlineNdCe TChar& operator[](int x) noexcept { return At(x); }
    EckInlineNdCe TChar operator[](int x) const noexcept { return At(x); }

    EckInlineNdCe TChar& Front() noexcept { return At(0); }
    EckInlineNdCe TChar Front() const noexcept { return At(0); }
    EckInlineNdCe TChar& Back() noexcept { return At(Size() - 1); }
    EckInlineNdCe TChar Back() const noexcept { return At(Size() - 1); }

    EckInlineNd TAllocator GetAllocator() const noexcept { return m_Alloc; }
    EckInlineNdCe int Size() const noexcept { return m_cchText; }
    EckInlineNdCe BOOL IsEmpty() const noexcept { return Size() == 0; }
    EckInlineNdCe size_t ByteSize() const noexcept { return (m_cchText + 1) * sizeof(TChar); }
    EckInlineNdCe size_t ByteSizePure() const noexcept { return m_cchText * sizeof(TChar); }
    EckInlineNdCe int Capacity() const noexcept { return m_cchCapacity; }
    EckInlineNdCe BOOL IsLocal() const noexcept { return Capacity() == LocalBufferSize; }
    EckInlineNdCe size_t ByteCapacity() const noexcept { return m_cchCapacity * sizeof(TChar); }
    EckInlineNdCe TPointer Data() noexcept { return IsLocal() ? m_szLocal : m_pszText; }
    EckInlineNdCe TConstPointer Data() const noexcept { return IsLocal() ? m_szLocal : m_pszText; }

    // 返回实际复制的字符数
    int Assign(TConstPointer pszSrc, int cchSrc = -1)
    {
        if (!pszSrc)
        {
            EckAssert(!cchSrc);
            Clear();
            return 0;
        }
        if (cchSrc < 0)
            cchSrc = (int)TcsLength(pszSrc);
        ReSizeExtra(cchSrc);
        TcsCopyLengthEnd(Data(), pszSrc, cchSrc);
        return cchSrc;
    }

    EckInline int Assign(TNtString nts)
    {
        return Assign((TConstPointer)nts.Buffer, (int)nts.Length);
    }

    template<class TTraits, class TAllocator1>
    EckInline int Assign(const std::basic_string<TChar, TTraits, TAllocator1>& x)
    {
        return Assign(x.data(), (int)x.size());
    }

    template<class TTraits>
    EckInline int Assign(std::basic_string_view<TChar, TTraits> sv)
    {
        return Assign(sv.data(), (int)sv.size());
    }

    EckInline int AssignBSTR(BSTR bstr)
    {
        if (!bstr)
        {
            Clear();
            return 0;
        }

        const auto cchBstr = (int)SysStringLen(bstr);
        if constexpr (std::is_same_v<TChar, CHAR>)
        {
            const int cch = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, bstr, cchBstr,
                nullptr, 0, nullptr, nullptr);
            if (cch > 1)
            {
                ReSize(cch);
                return WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, bstr, cchBstr,
                    Data(), cch, nullptr, nullptr);
            }
            else
                Clear();
        }
        else
            Assign(bstr, cchBstr);
    }

    int AssignSTRRET(const STRRET& strret, PITEMIDLIST pidl = nullptr)
    {
        switch (strret.uType)
        {
        case STRRET_WSTR:
            return Assign(strret.pOleStr);
        case STRRET_OFFSET:
            EckAssert(pidl);
            return Assign((TConstPointer)((PCBYTE)pidl + strret.uOffset));
        case STRRET_CSTR:
            if constexpr (std::is_same_v<TChar, CHAR>)
                return Assign(strret.cStr);
            else
            {
                const int cch = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED,
                    strret.cStr, -1, nullptr, 0);
                if (cch > 1)
                {
                    ReSize(cch);
                    return MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED,
                        strret.cStr, -1, Data(), cch);
                }
                else
                    Clear();
            }
        default:
            return 0;
        }
    }

    /// <summary>
    /// 依附指针。
    /// 先前的内存将被释放
    /// </summary>
    /// <param name="psz">指针，必须可通过当前分配器解分配</param>
    /// <param name="cchCapacity">容量</param>
    /// <param name="cchText">字符数</param>
    void Attach(TPointer psz, int cchCapacity, int cchText) noexcept
    {
        EckAssert(cchCapacity > 0 && cchText >= 0);
        m_Alloc.deallocate(m_pszText, std::max(m_cchCapacity, LocalBufferSize + 1));
        if (!psz)
        {
            m_pszText = nullptr;
            m_cchText = m_cchCapacity = 0;
        }
        else
        {
            m_cchCapacity = cchCapacity;
            m_cchText = cchText;
            m_pszText = psz;
        }
    }

    /// <summary>
    /// 拆离指针
    /// </summary>
    /// <returns>指针，必须通过与当前分配器相等的分配器解分配</returns>
    EckInlineNd TPointer Detach(int& cchCapacity, int& cchText) noexcept
    {
        if (IsLocal())
        {
            cchCapacity = m_cchCapacity;
            const auto p = m_Alloc.allocate(cchCapacity);
            // 必须在分配完毕后更新cchText，因为cchCapacity与cchText可能为同一个变量
            cchText = m_cchText;
            TcsCopyLength(p, m_szLocal, m_cchText + 1);
            return p;
        }
        else
        {
            const auto pOld = m_pszText;
            m_pszText = nullptr;

            cchCapacity = m_cchCapacity;
            m_cchCapacity = 0;

            cchText = m_cchText;
            m_cchText = 0;
            return pOld;
        }
    }

    CStringT& PushBack(TConstPointer pszSrc, int cchSrc = -1)
    {
        if (!pszSrc)
            return *this;
        if (cchSrc < 0)
            cchSrc = (int)TcsLength(pszSrc);
        ReSizeExtra(Size() + cchSrc);
        TcsCopyLengthEnd(Data() + Size() - cchSrc, pszSrc, cchSrc);
        return *this;
    }

    EckInline CStringT& PushBack(const CStringT& rs)
    {
        return PushBack(rs.Data(), rs.Size());
    }

    EckInline TPointer PushBack(int cch)
    {
        EckAssert(cch >= 0);
        ReSizeExtra(Size() + cch);
        return Data() + Size() - cch;
    }

    template<class TTraits, class TAllocator1>
    EckInline CStringT& PushBack(const std::basic_string<TChar, TTraits>& s)
    {
        return PushBack(s.data(), (int)s.size());
    }

    template<class TTraits>
    EckInline CStringT& PushBack(std::basic_string_view<TChar, TTraits> sv)
    {
        return PushBack(sv.data(), (int)sv.size());
    }

    EckInline CStringT& PushBackChar(TChar ch)
    {
        ReSizeExtra(Size() + 1);
        *(Data() + Size() - 1) = ch;
        return *this;
    }

    EckInline TPointer PushBackNoExtra(int cch)
    {
        EckAssert(cch >= 0);
        ReSize(Size() + cch);
        return Data() + Size() - cch;
    }

    EckInline CStringT& operator+=(TConstPointer psz)
    {
        PushBack(psz);
        return *this;
    }

    EckInline CStringT& operator+=(const CStringT& x)
    {
        PushBack(x.Data(), x.Size());
        return *this;
    }

    template<class TTraits, class TAllocator1>
    EckInline CStringT& operator+=(const std::basic_string<TChar, TTraits, TAllocator1>& x)
    {
        PushBack(x.data(), (int)x.size());
        return *this;
    }

    template<class TTraits>
    EckInline CStringT& operator+=(std::basic_string_view<TChar, TTraits> x)
    {
        PushBack(x.data(), (int)x.size());
        return *this;
    }

    EckInline CStringT& operator+=(TChar ch)
    {
        PushBackChar(ch);
        return *this;
    }

    EckInline CStringT& PopBack(int cch = 1) noexcept
    {
        EckAssert(Size() >= cch && cch >= 0);
        if (!cch)
            return *this;
        m_cchText -= cch;
        TCharTraits::Cut(Data(), Size());
        return *this;
    }

    // 返回实际复制的字符数
    EckInline int CopyTo(TPointer pszDst, int cch = -1) const noexcept
    {
        if (cch < 0 || cch > Size())
            cch = Size();
        TcsCopyLengthEnd(pszDst, Data(), cch);
        return cch;
    }
private:
    void ReserveReal(int cch)
    {
        if (m_cchCapacity >= cch)
            return;
        if (IsLocal() && cch <= LocalBufferSize)
            return;
        const auto pOld = IsLocal() ? m_szLocal : m_pszText;
        const auto pNew = m_Alloc.allocate(cch);
        if (pOld)
        {
            TcsCopyLength(pNew, pOld, m_cchText + 1);// 多拷一个结尾NULL
            if (!IsLocal())
                m_Alloc.deallocate(pOld, m_cchCapacity);
        }
        else
            TCharTraits::Cut(pNew, 0);
        m_pszText = pNew;
        m_cchCapacity = cch;
    }
public:
    EckInline void Reserve(int cch) { ReserveReal(cch + 1); }

    void ReSize(int cch)
    {
        EckAssert(cch >= 0);
        ReserveReal(cch + 1);
        m_cchText = cch;
        TCharTraits::Cut(Data(), cch);
    }

    void ReSizeExtra(int cch)
    {
        EckAssert(cch >= 0);
        if (m_cchCapacity < cch + 1)
            ReserveReal(TAllocatorTraits::MakeCapacity(cch + 1));
        m_cchText = cch;
        TCharTraits::Cut(Data(), cch);
    }

    EckInline int ReCalcLen() noexcept
    {
        return m_cchText = (int)TcsLength(Data());
    }

    /// <summary>
    /// 替换
    /// </summary>
    /// <param name="posStart">替换位置</param>
    /// <param name="cchReplacing">替换长度</param>
    /// <param name="pszNew">用作替换的字符串指针</param>
    /// <param name="cchNew">用作替换的字符串长度</param>
    void Replace(int posStart, int cchReplacing, TConstPointer pszNew, int cchNew)
    {
        EckAssert(pszNew ? TRUE : cchNew == 0);
        if (cchNew < 0)
            cchNew = (int)TcsLength(pszNew);
        const int cchOrg = Size();
        const int cchAfter = Size() + cchNew - cchReplacing;
        Reserve(cchAfter);
        TcsMoveLength(
            Data() + posStart + cchNew,
            Data() + posStart + cchReplacing,
            cchOrg - posStart - cchReplacing);
        if (pszNew)
            TcsCopyLength(Data() + posStart, pszNew, cchNew);
        ReSize(cchAfter);
    }

    /// <summary>
    /// 替换
    /// </summary>
    /// <param name="posStart">替换位置</param>
    /// <param name="cchReplacing">替换长度</param>
    /// <param name="rsNew">用作替换的字符串</param>
    EckInline void Replace(int posStart, int cchReplacing, const CStringT& rsNew)
    {
        Replace(posStart, cchReplacing, rsNew.Data(), rsNew.Size());
    }

    /// <summary>
    /// 子文本替换
    /// </summary>
    /// <param name="pszReplaced">被替换的字符串指针</param>
    /// <param name="cchReplaced">被替换的字符串长度</param>
    /// <param name="pszSrc">用作替换的字符串指针</param>
    /// <param name="cchSrc">用作替换的字符串长度</param>
    /// <param name="posStart">起始位置</param>
    /// <param name="cReplacing">替换进行的次数，0为执行所有替换</param>
    void ReplaceSubString(TConstPointer pszReplaced, int cchReplaced, TConstPointer pszSrc, int cchSrc,
        int posStart = 0, int cReplacing = 0)
    {
        EckAssert(pszReplaced);
        EckAssert(pszSrc ? TRUE : cchSrc == 0);
        if (cchReplaced < 0)
            cchReplaced = (int)TcsLength(pszReplaced);
        if (cchSrc < 0)
            cchSrc = (int)TcsLength(pszSrc);
        int pos = 0;
        for (int c = 1;; ++c)
        {
            pos = FindStringLength(Data(), Size(), pszReplaced, cchReplaced, posStart + pos);
            if (pos < 0)
                break;
            Replace(pos, cchReplaced, pszSrc, cchSrc);
            pos += cchSrc;
            if (c == cReplacing)
                break;
        }
    }

    /// <summary>
    /// 子文本替换
    /// </summary>
    /// <param name="rsReplaced">被替换的字符串</param>
    /// <param name="rsSrc">用作替换的字符串</param>
    /// <param name="posStart">起始位置</param>
    /// <param name="cReplacing">替换进行的次数，0为执行所有替换</param>
    EckInline void ReplaceSubString(const CStringT& rsReplaced, const CStringT& rsSrc,
        int posStart = 0, int cReplacing = 0)
    {
        ReplaceSubString(rsReplaced.Data(), rsReplaced.Size(), rsSrc.Data(), rsSrc.Size(), posStart, cReplacing);
    }

    EckInline void MakeSpace(int cch, int posStart = 0)
    {
        ReSize(posStart + cch);
        TcsSet(Data() + posStart, cch, TCharTraits::CharSpace());
    }

    void MakeRepeatedSequence(int cCount, TConstPointer pszText,
        int cchText = -1, int posStart = 0)
    {
        EckAssert(pszText);
        if (cchText < 0)
            cchText = (int)TcsLength(pszText);
        ReSize(posStart + cchText * cCount);
        auto pszCurr = Data() + posStart;
        for (int i = 0; i < cCount; ++i, pszCurr += cchText)
            TcsCopyLength(pszCurr, pszText, cchText);
        TCharTraits::Cut(Data(), Size());
    }
    EckInline void MakeRepeatedSequence(int cCount, const CStringT& rsText, int posStart = 0)
    {
        MakeRepeatedStrSequence(rsText.Data(), rsText.Size(), cCount, posStart);
    }

    EckInline constexpr void Clear() noexcept
    {
        m_cchText = 0;
        TCharTraits::Cut(Data(), 0);
    }

    EckInline void Insert(int pos, TConstPointer pszText, int cchText = -1)
    {
        EckAssert(pos <= Size() && pos >= 0);
        EckAssert(pszText ? TRUE : (cchText == 0));
        if (cchText < 0)
            cchText = (int)TcsLength(pszText);
        ReSizeExtra(Size() + cchText);
        TcsMoveLengthEnd(
            Data() + pos + cchText,
            Data() + pos,
            Size() - cchText - pos);
        TcsCopyLength(Data() + pos, pszText, cchText);
    }

    template<class TTraits, class TAllocator1>
    EckInline void Insert(int pos, const CStringT<TChar, TTraits, TAllocator1>& rs)
    {
        Insert(pos, rs.Data(), rs.Size());
    }

    template<class TTraits, class TAllocator1>
    EckInline void Insert(int pos, const std::basic_string<TChar, TTraits, TAllocator1>& s)
    {
        Insert(pos, s.data(), (int)s.size());
    }

    template<class TTraits>
    EckInline void Insert(int pos, std::basic_string_view<TChar, TTraits> sv)
    {
        Insert(pos, sv.data(), (int)sv.size());
    }

    EckInline TPointer Insert(int pos, int cchText)
    {
        EckAssert(pos <= Size() && pos >= 0);
        ReSizeExtra(Size() + cchText);
        TcsMoveLengthEnd(
            Data() + pos + cchText,
            Data() + pos,
            Size() - cchText - pos);
        return Data() + pos;
    }

    EckInline void InsertChar(int pos, TChar ch)
    {
        EckAssert(pos <= Size() && pos >= 0);
        ReSizeExtra(Size() + 1);
        TcsMoveLengthEnd(
            Data() + pos + 1,
            Data() + pos,
            Size() - 1 - pos);
        TCharTraits::AssignChar(Data() + pos, ch);
    }

    EckInline void Erase(int pos, int cch = 1) noexcept
    {
        EckAssert(Size() >= pos + cch);
        TcsMoveLengthEnd(
            Data() + pos,
            Data() + pos + cch,
            Size() - pos - cch);
        m_cchText -= cch;
    }

    void ShrinkToFit()
    {
        EckAssert(m_cchCapacity >= m_cchText + 1);
        if (m_cchCapacity == m_cchText + 1)
            return;
        const auto pOld = m_pszText;
        m_pszText = m_Alloc.allocate(m_cchText + 1);
        TcsCopyLength(Data(), pOld, m_cchText + 1);
        m_Alloc.deallocate(pOld, m_cchCapacity);
        m_cchCapacity = m_cchText + 1;
    }

    void ExtendToCapacity() noexcept
    {
        if (Capacity())
        {
            m_cchText = Capacity() - 1;
            TCharTraits::Cut(Data(), Size());
        }
    }

    EckInline int Format(_Printf_format_string_ TConstPointer pszFmt, ...)
    {
        va_list vl;
        va_start(vl, pszFmt);
        const int cch = FormatV(pszFmt, vl);
        va_end(vl);
        return cch;
    }

    EckInline int FormatV(_Printf_format_string_ TConstPointer pszFmt, va_list vl)
    {
        const int cch = TCharTraits::GetFormatLengthV(pszFmt, vl);
        if (cch <= 0)
            return 0;
        ReSizeExtra(cch);
        TCharTraits::FormatV(Data(), pszFmt, vl);
        return cch;
    }

    EckInline int PushBackFormat(_Printf_format_string_ TConstPointer pszFmt, ...)
    {
        va_list vl;
        va_start(vl, pszFmt);
        const int cch = PushBackFormatV(pszFmt, vl);
        va_end(vl);
        return cch;
    }

    EckInline int PushBackFormatV(_Printf_format_string_ TConstPointer pszFmt, va_list vl)
    {
        const int cch = TCharTraits::GetFormatLengthV(pszFmt, vl);
        if (cch <= 0)
            return 0;
        TCharTraits::FormatV(PushBack(cch), pszFmt, vl);
        return cch;
    }

    // 取BSTR。
    // 调用方负责对返回值使用SysFreeString解分配
    EckInlineNd BSTR ToBSTR() const noexcept
    {
        if constexpr (std::is_same_v<TChar, WCHAR>)
            return SysAllocStringLen(Data(), Size());
        else
        {
            const auto cch = MultiByteToWideChar(
                CP_ACP, MB_PRECOMPOSED, Data(), Size(), nullptr, 0);
            if (cch > 0)
            {
                const auto bstr = SysAllocStringLen(nullptr, cch);
                MultiByteToWideChar(
                    CP_ACP, MB_PRECOMPOSED, Data(), Size(), bstr, cch);
                return bstr;
            }
            else
                return SysAllocStringLen(nullptr, 0);
        }
    }

    template<class TTraits = std::char_traits<TChar>, class TAllocator1 = std::allocator<TChar>>
    EckInlineNd auto ToStdString() const
    {
        return std::basic_string<TChar, TTraits, TAllocator1>(Data(), (size_t)Size());
    }

    EckInlineNdCe auto ToStringView() const noexcept
    {
        return std::basic_string_view<TChar>(Data(), Size());
    }

    EckInlineNdCe auto ToSpan() const noexcept
    {
        return std::span<TChar>(Data(), ByteSize());
    }

    EckInlineNdCe TNtString ToNtString() noexcept
    {
        return TNtString{ (USHORT)ByteSizePure(), (USHORT)ByteCapacity(), Data() };
    }

    EckInlineNdCe RTL_UNICODE_STRING_BUFFER ToNtStringBuffer() noexcept
    {
        if constexpr (std::is_same_v<TChar, WCHAR>)
        {
            return
            {
                .String = ToNtString(),
                .ByteBuffer = {.Buffer = (PUCHAR)Data(),.Size = ByteCapacity() },
            };
        }
        else
            return {};
    }

    EckInlineNd int Find(TConstPointer pszSub, int cchSub = -1, int posStart = 0) const noexcept
    {
        if (cchSub < 0)
            cchSub = (int)TcsLength(pszSub);
        return FindStringLength(Data(), Size(), pszSub, cchSub, posStart);
    }
    template<class TTraits, class TAllocator1>
    EckInlineNd int Find(const CStringT<TChar, TTraits, TAllocator1>& rs, int posStart = 0) const noexcept
    {
        return Find(rs.Data(), rs.Size(), posStart);
    }
    template<class TTraits, class TAllocator1>
    EckInlineNd int Find(const std::basic_string<TChar, TTraits, TAllocator1>& s, int posStart = 0) const noexcept
    {
        return Find(s.data(), (int)s.size(), posStart);
    }
    template<class TTraits>
    EckInlineNd int Find(std::basic_string_view<TChar, TTraits> sv, int posStart = 0) const noexcept
    {
        return Find(sv.data(), (int)sv.size(), posStart);
    }

    EckInlineNd int FindI(TConstPointer pszSub, int cchSub = -1, int posStart = 0) const noexcept
    {
        if (cchSub < 0)
            cchSub = (int)TcsLength(pszSub);
        return FindStringLengthI(Data(), Size(), pszSub, cchSub, posStart);
    }
    template<class TTraits, class TAllocator1>
    EckInlineNd int FindI(const CStringT<TChar, TTraits, TAllocator1>& rs, int posStart = 0) const noexcept
    {
        return FindI(rs.Data(), rs.Size(), posStart);
    }
    template<class TTraits, class TAllocator1>
    EckInlineNd int FindI(const std::basic_string<TChar, TTraits, TAllocator1>& s, int posStart = 0) const noexcept
    {
        return FindI(s.data(), (int)s.size(), posStart);
    }
    template<class TTraits>
    EckInlineNd int FindI(std::basic_string_view<TChar, TTraits> sv, int posStart = 0) const noexcept
    {
        return FindI(sv.data(), (int)sv.size(), posStart);
    }

    EckInlineNd int RFind(TConstPointer pszSub, int cchSub = -1, int posStart = -1) const noexcept
    {
        if (cchSub < 0)
            cchSub = (int)TcsLength(pszSub);
        return RFindString(Data(), Size(), pszSub, cchSub, posStart);
    }
    template<class TTraits, class TAllocator1>
    EckInlineNd int RFind(const CStringT<TChar, TTraits, TAllocator1>& rs, int posStart = -1) const noexcept
    {
        return RFind(rs.Data(), rs.Size(), posStart);
    }
    template<class TTraits, class TAllocator1>
    EckInlineNd int RFind(const std::basic_string<TChar, TTraits, TAllocator1>& s, int posStart = -1) const noexcept
    {
        return RFind(s.data(), (int)s.size(), posStart);
    }
    template<class TTraits>
    EckInlineNd int RFind(std::basic_string_view<TChar, TTraits> sv, int posStart = -1) const noexcept
    {
        return RFind(sv.data(), (int)sv.size(), posStart);
    }

    EckInlineNd int RFindI(TConstPointer pszSub, int cchSub = -1, int posStart = -1) const noexcept
    {
        if (cchSub < 0)
            cchSub = (int)TcsLength(pszSub);
        return RFindStringI(Data(), Size(), pszSub, cchSub, posStart);
    }
    template<class TTraits, class TAllocator1>
    EckInlineNd int RFindI(const CStringT<TChar, TTraits, TAllocator1>& rs, int posStart = -1) const noexcept
    {
        return RFindI(rs.Data(), rs.Size(), posStart);
    }
    template<class TTraits, class TAllocator1>
    EckInlineNd int RFindI(const std::basic_string<TChar, TTraits, TAllocator1>& s, int posStart = -1) const noexcept
    {
        return RFindI(s.data(), (int)s.size(), posStart);
    }
    template<class TTraits>
    EckInlineNd int RFindI(std::basic_string_view<TChar, TTraits> sv, int posStart = -1) const noexcept
    {
        return RFindI(sv.data(), (int)sv.size(), posStart);
    }

    EckInlineNd int FindChar(TChar ch, int posStart = 0) const noexcept
    {
        if (IsEmpty())
            return StrNPos;
        return FindCharLength(Data(), Size(), ch, posStart);
    }
    EckInlineNd int RFindChar(TChar ch, int posStart = -1) const noexcept
    {
        if (IsEmpty())
            return StrNPos;
        return RFindCharLength(Data(), Size(), ch, posStart);
    }

    EckInlineNd int FindFirstOf(TConstPointer pszChars, int cchChars = -1, int posStart = 0) const noexcept
    {
        if (cchChars < 0)
            cchChars = (int)TcsLength(pszChars);
        return FindCharFirstOf(Data(), Size(), pszChars, cchChars, posStart);
    }
    EckInlineNd int FindFirstNotOf(TConstPointer pszChars, int cchChars = -1, int posStart = 0) const noexcept
    {
        if (cchChars < 0)
            cchChars = (int)TcsLength(pszChars);
        return FindCharFirstNotOf(Data(), Size(), pszChars, cchChars, posStart);
    }
    EckInlineNd int FindLastOf(TConstPointer pszChars, int cchChars = -1, int posStart = -1) const noexcept
    {
        if (cchChars < 0)
            cchChars = (int)TcsLength(pszChars);
        return FindCharLastOf(Data(), Size(), pszChars, cchChars, posStart);
    }
    EckInlineNd int FindLastNotOf(TConstPointer pszChars, int cchChars = -1, int posStart = -1) const noexcept
    {
        if (cchChars < 0)
            cchChars = (int)TcsLength(pszChars);
        return FindCharLastNotOf(Data(), Size(), pszChars, cchChars, posStart);
    }

    void LTrim() noexcept
    {
        if (IsEmpty())
            return;
        const auto pszBegin = TrimStringLeft(Data(), Size());
        const int cchNew = Size() - int(pszBegin - Data());
        TcsMoveLength(Data(), pszBegin, cchNew);
        ReSize(cchNew);
    }
    void RTrim() noexcept
    {
        if (IsEmpty())
            return;
        const auto pszEnd = TrimStringRight(Data(), Size());
        ReSize(int(pszEnd - Data()));
    }
    void LRTrim() noexcept
    {
        if (IsEmpty())
            return;
        const auto pszBegin = TrimStringLeft(Data(), Size());
        const auto pszEnd = TrimStringRight(Data(), Size());
        if (pszEnd < pszBegin)
            Clear();
        else
        {
            const int cchNew = int(pszEnd - pszBegin);
            TcsMoveLength(Data(), pszBegin, cchNew);
            ReSize(cchNew);
        }
    }
    void AllTrim() noexcept
    {
        if (IsEmpty())
            return;
        const auto pData = Data();
        TPointer p0, p1{ pData }, pCurr{ pData };
        EckLoop()
        {
            if constexpr (std::is_same_v<TChar, WCHAR>)
                p0 = TcsCharFirstNotOf(p1, Size() - (p1 - pData), EckStrAndLen(SpaceCharsW));
            else
                p0 = TcsCharFirstNotOf(p1, Size() - (p1 - pData), EckStrAndLen(SpaceCharsA));
            if (!p0)
                break;
            if constexpr (std::is_same_v<TChar, WCHAR>)
                p1 = TcsCharFirstOf(p0, Size() - (p0 - pData), EckStrAndLen(SpaceCharsW));
            else
                p1 = TcsCharFirstOf(p0, Size() - (p0 - pData), EckStrAndLen(SpaceCharsA));
            if (!p1)
                p1 = pData + Size();
            TcsMoveLength(pCurr, p0, p1 - p0);
            pCurr += (p1 - p0);
        }
        ReSize(int(pCurr - pData));
    }

    [[nodiscard]] BOOL IsStartWith(TConstPointer psz, int cch = -1) const noexcept
    {
        if (cch < 0)
            cch = (int)TcsLength(psz);
        if (cch == 0 || Size() < cch)
            return FALSE;
        return TcsEqualLength(Data(), psz, cch);
    }
    template<class TTraits>
    EckInlineNd int IsStartWith(std::basic_string_view<TChar, TTraits> sv) const noexcept
    {
        return IsStartWith(sv.data(), (int)sv.size());
    }
    [[nodiscard]] BOOL IsStartWithI(TConstPointer psz, int cch = -1) const noexcept
    {
        if (cch < 0)
            cch = (int)TcsLength(psz);
        if (cch == 0 || Size() < cch)
            return FALSE;
        return TcsEqualLengthI(Data(), psz, cch);
    }
    template<class TTraits>
    EckInlineNd int IsStartWithI(std::basic_string_view<TChar, TTraits> sv) const noexcept
    {
        return IsStartWithI(sv.data(), (int)sv.size());
    }

    [[nodiscard]] BOOL IsEndWith(TConstPointer psz, int cch = -1) const noexcept
    {
        if (cch < 0)
            cch = (int)TcsLength(psz);
        if (cch == 0 || Size() < cch)
            return FALSE;
        return TcsEqualLength(Data() + Size() - cch, psz, cch);
    }
    template<class TTraits>
    EckInlineNd int IsEndWith(std::basic_string_view<TChar, TTraits> sv) const noexcept
    {
        return IsEndWith(sv.data(), (int)sv.size());
    }
    [[nodiscard]] BOOL IsEndWithI(TConstPointer psz, int cch = -1) const noexcept
    {
        if (cch < 0)
            cch = (int)TcsLength(psz);
        if (cch == 0 || Size() < cch)
            return FALSE;
        return TcsEqualLengthI(Data() + Size() - cch, psz, cch);
    }
    template<class TTraits>
    EckInlineNd int IsEndWithI(std::basic_string_view<TChar, TTraits> sv) const noexcept
    {
        return IsEndWithI(sv.data(), (int)sv.size());
    }

    EckInlineNd CStringT SubString(int posStart, int cch) const
    {
        EckAssert(posStart >= 0 && posStart < Size());
        if (cch < 0)
            cch = Size() - posStart;
        EckAssert(cch != 0 && posStart + cch <= Size());
        return CStringT(Data() + posStart, cch);
    }
    EckInlineNd auto SubStringView(int posStart, int cch) const noexcept
    {
        EckAssert(posStart >= 0 && posStart < Size());
        if (cch < 0)
            cch = Size() - posStart;
        EckAssert(cch != 0 && posStart + cch <= Size());
        return std::basic_string_view<TChar>(Data() + posStart, cch);
    }
    EckInlineNd auto SubSpan(int posStart, int cch) const noexcept
    {
        EckAssert(posStart >= 0 && posStart < Size());
        if (cch < 0)
            cch = Size() - posStart;
        EckAssert(cch != 0 && posStart + cch <= Size());
        return std::span<TChar>(Data() + posStart, cch);
    }

    EckInline void ToLower() noexcept
    {
        const auto pEnd = Data() + Size();
        for (auto p = Data(); p < pEnd; ++p)
            *p = TchToLower(*p);
    }
    EckInline void ToUpper() noexcept
    {
        const auto pEnd = Data() + Size();
        for (auto p = Data(); p < pEnd; ++p)
            *p = TchToUpper(*p);
    }

    EckInline int Compare(TConstPointer psz, int cch = -1) const noexcept
    {
        if (cch < 0)
            cch = (int)TcsLength(psz);
        return TcsCompareLength2(Data(), Size(), psz, cch);
    }
    template<class TTraits>
    EckInline int Compare(std::basic_string_view<TChar, TTraits> sv) const noexcept
    {
        return Compare(sv.data(), (int)sv.size());
    }
    EckInline int CompareI(TConstPointer psz, int cch = -1) const noexcept
    {
        if (cch < 0)
            cch = (int)TcsLength(psz);
        return TcsCompareLength2I(Data(), Size(), psz, cch);
    }
    template<class TTraits>
    EckInline int CompareI(std::basic_string_view<TChar, TTraits> sv) const noexcept
    {
        return CompareI(sv.data(), (int)sv.size());
    }

    // 返回文件名的位置，注意：若分隔符在开头则返回-1
    [[nodiscard]] int PazFindFileSpec() const noexcept
    {
        if (IsEmpty())
            return -1;
        auto pEnd = Data() + Size() - 1;
        if (Back() == '\\' || Back() == '/')
            --pEnd;// 如果以反斜杠结尾，则跳过
        for (auto p = pEnd; p != Data(); --p)
        {
            const auto ch = *p;
            if (ch == '\\' || ch == '/')
            {
                if (p < Data() + 2)// NT路径或UNC路径的起始
                    return -1;
                return int(p + 1 - Data());
            }
        }
        return -1;
    }

    BOOL PazRemoveFileSpec() noexcept
    {
        const auto pos = PazFindFileSpec();
        if (pos < 0)
            return FALSE;
        EckAssert(pos >= 1);
        ReSize(pos - 1);
        return TRUE;
    }

    BOOL PazRenameFileSpec(TConstPointer pszNewName, int cchNewName = -1) noexcept
    {
        const auto pos = PazFindFileSpec();
        if (pos < 0)
            return FALSE;
        if (cchNewName < 0)
            cchNewName = (int)TcsLength(pszNewName);
        ReSize(pos + cchNewName);
        TcsCopyLength(Data() + pos, pszNewName, cchNewName);
        return TRUE;
    }

    [[nodiscard]] int PazFindExtension() const noexcept
    {
        if (IsEmpty())
            return -1;
        int pos{ -1 };
        for (auto p = Data() + Size() - 1; p != Data(); --p)
        {
            const auto ch = *p;
            if (ch == '.')
                return int(p - Data());
            else if (ch == ' ' /*扩展名内不能有空格*/ ||
                ch == '\\' || ch == '/')
                return -1;
        }
        return -1;
    }

    BOOL PazRemoveExtension() noexcept
    {
        const auto pos = PazFindExtension();
        if (pos < 0)
            return FALSE;
        ReSize(pos);
        return TRUE;
    }

    void PazRenameExtension(TConstPointer pszNewExt, int cchNewExt = -1) noexcept
    {
        const auto pos = PazFindExtension();
        if (pos < 0)
            PushBack(pszNewExt, cchNewExt);
        else
        {
            if (cchNewExt < 0)
                cchNewExt = (int)TcsLength(pszNewExt);
            ReSize(pos + cchNewExt);
            TcsCopyLength(Data() + pos, pszNewExt, cchNewExt);
        }
    }

    // 如果没有反斜杠，则在末尾添加。返回值指示操作前是否已有反斜杠
    BOOL PazAddBackslash()
    {
        if (IsEmpty())
            return FALSE;
        if (Back() == '\\' || Back() == '/')
            return TRUE;
        PushBackChar('\\');
        return FALSE;
    }
    // 去掉末尾的反斜杠
    BOOL PazRemoveBackslash() noexcept
    {
        if (IsEmpty())
            return FALSE;
        if (Back() == '\\' || Back() == '/')
        {
            PopBack();
            return TRUE;
        }
        return FALSE;
    }
    // 将所有斜杠替换为反斜杠
    void PazConvertToBackslash() noexcept
    {
        for (auto& ch : *this)
            if (ch == '/')
                ch = '\\';
    }
    // 将所有反斜杠替换为斜杠
    void PazConvertToSlash() noexcept
    {
        for (auto& ch : *this)
            if (ch == '\\')
                ch = '/';
    }

    HRESULT PazParseCommandLine(_Out_ TPointer& pszFile, _Out_ int& cchFile,
        _Out_ TPointer& pszParam, _Out_ int& cchParam) noexcept
    {
        return eck::PazParseCommandLine(Data(), Size(), pszFile, cchFile, pszParam, cchParam);
    }

    HRESULT PazParseCommandLineAndCut(_Out_ TPointer& pszFile, _Out_ int& cchFile,
        _Out_ TPointer& pszParam, _Out_ int& cchParam) noexcept
    {
        return eck::PazParseCommandLineAndCut(Data(), Size(), pszFile, cchFile, pszParam, cchParam);
    }

    void PazFindFileName(BOOL bKeepExtension, _Out_ int& pos0, _Out_ int& pos1) const noexcept
    {
        pos0 = PazFindFileSpec();
        if (bKeepExtension)
            pos1 = Size();
        else if ((pos1 = PazFindExtension()) < 0)
            pos1 = Size();
        if (pos0 < 0 || pos1 < 0 || pos1 <= pos0)
            pos0 = pos1 = -1;
    }

    BOOL PazTrimToFileName(BOOL bKeepExtension = FALSE) noexcept
    {
        int pos0, pos1;
        PazFindFileName(bKeepExtension, pos0, pos1);
        if (pos0 < 0)
            return FALSE;
        TcsMoveLength(Data(), Data() + pos0, pos1 - pos0);
        ReSize(pos1 - pos0);
        return TRUE;
    }
    BOOL PazTrimToFileName(CStringT& rsFileName, BOOL bKeepExtension = FALSE) const noexcept
    {
        int pos0, pos1;
        PazFindFileName(bKeepExtension, pos0, pos1);
        if (pos0 < 0)
            return FALSE;
        rsFileName.PushBackNoExtra(pos1 - pos0);
        TcsCopyLength(rsFileName.Data(), Data() + pos0, pos1 - pos0);
        return TRUE;
    }

    void PazLegalize(TChar chReplace = '_', BOOL bReplaceDot = FALSE) noexcept
    {
        eck::PazLegalizeLength(Data(), Size(), chReplace, bReplaceDot);
    }

    EckInlineNdCe TIterator begin() noexcept { return Data(); }
    EckInlineNdCe TIterator end() noexcept { return begin() + Size(); }
    EckInlineNdCe TConstIterator begin() const noexcept { return Data(); }
    EckInlineNdCe TConstIterator end() const noexcept { return begin() + Size(); }
    EckInlineNdCe TConstIterator cbegin() const noexcept { return begin(); }
    EckInlineNdCe TConstIterator cend() const noexcept { return end(); }
    EckInlineNdCe TReverseIterator rbegin() noexcept { return TReverseIterator(end()); }
    EckInlineNdCe TReverseIterator rend() noexcept { return TReverseIterator(begin()); }
    EckInlineNdCe TConstReverseIterator rbegin() const noexcept { return TConstReverseIterator(end()); }
    EckInlineNdCe TConstReverseIterator rend() const noexcept { return TConstReverseIterator(begin()); }
    EckInlineNdCe TConstReverseIterator crbegin() const noexcept { return rbegin(); }
    EckInlineNdCe TConstReverseIterator crend() const noexcept { return rend(); }
};

using CStringW = CStringT<WCHAR, CCharTraits<WCHAR>>;
using CStringA = CStringT<CHAR, CCharTraits<CHAR>>;

#undef EckTemp
#define EckTemp CStringT<TChar, TCharTraits, TAllocator>

#pragma region Operator
template<class TChar, class TCharTraits, class TAllocator>
EckInline EckTemp operator+(const EckTemp& rs1, const EckTemp& rs2)
{
    EckTemp x(rs1.Size() + rs2.Size());
    TcsCopyLength(x.Data(), rs1.Data(), rs1.Size());
    TcsCopyLengthEnd(x.Data() + rs1.Size(), rs2.Data(), rs2.Size());
    return x;
}
template<class TChar, class TCharTraits, class TAllocator>
EckInline EckTemp operator+(const EckTemp& rs, const TChar* psz)
{
    const int cch = (psz ? (int)TcsLength(psz) : 0);
    EckTemp x(rs.Size() + cch);
    TcsCopyLength(x.Data(), rs.Data(), rs.Size());
    TcsCopyLengthEnd(x.Data() + rs.Size(), psz, cch);
    return x;
}

template<class TChar, class TCharTraits, class TAllocator>
EckInlineNd bool operator==(const EckTemp& rs1, const TChar* psz2) noexcept
{
    if (rs1.IsEmpty())
        return !psz2;
    else if (!psz2)
        return false;
    else
        return TcsCompareLength2(rs1.Data(), rs1.Size(), psz2, (int)TcsLength(psz2)) == 0;
}
template<class TChar, class TCharTraits, class TAllocator>
EckInlineNd bool operator==(const TChar* psz2, const EckTemp& rs1) noexcept
{
    return operator==(rs1, psz2);
}

template<class TChar, class TCharTraits, class TAllocator>
EckInlineNd std::weak_ordering operator<=>(const EckTemp& rs1, const TChar* psz2) noexcept
{
    if (rs1.IsEmpty())
        return psz2 ? std::weak_ordering::less : std::weak_ordering::equivalent;
    else if (!psz2)
        return std::weak_ordering::greater;
    else
        return TcsCompareLength2(rs1.Data(), rs1.Size(), psz2, (int)TcsLength(psz2)) <=> 0;
}
template<class TChar, class TCharTraits, class TAllocator>
EckInlineNd std::weak_ordering operator<=>(const TChar* psz2, const EckTemp& rs1) noexcept
{
    if (!psz2)
        return rs1.IsEmpty() ? std::weak_ordering::equivalent : std::weak_ordering::less;
    else if (rs1.IsEmpty())
        return std::weak_ordering::greater;
    else
        return TcsCompareLength2(psz2, (int)TcsLength(psz2), rs1.Data(), rs1.Size()) <=> 0;
}

template<class TChar, class TCharTraits, class TAllocator>
EckInlineNd bool operator==(const EckTemp& rs1, const EckTemp& rs2) noexcept
{
    return TcsCompareLength2(rs1.Data(), rs1.Size(), rs2.Data(), rs2.Size()) == 0;
}

template<class TChar, class TCharTraits, class TAllocator>
EckInlineNd std::weak_ordering operator<=>(const EckTemp& rs1, const EckTemp& rs2) noexcept
{
    return TcsCompareLength2(rs1.Data(), rs1.Size(), rs2.Data(), rs2.Size()) <=> 0;
}
#pragma endregion Operator

template<class TCharTraits, class TAllocator>
EckInline void DbgPrint(const CStringT<WCHAR, TCharTraits, TAllocator>& rs, int iType = 1, BOOL bNewLine = TRUE) noexcept
{
    OutputDebugStringW(rs.Data());
    if (bNewLine)
        OutputDebugStringW(L"\n");
}

template<class TCharTraits, class TAllocator>
EckInline void DbgPrint(const CStringT<CHAR, TCharTraits, TAllocator>& rs, int iType = 1, BOOL bNewLine = TRUE) noexcept
{
    OutputDebugStringA(rs.Data());
    if (bNewLine)
        OutputDebugStringA("\n");
}

EckInlineNd CStringW ToString(int x, int iRadix = 10) noexcept
{
    CStringW rs(CchI32ToStrBuf);
    _itow(x, rs.Data(), iRadix);
    rs.ReCalcLen();
    return rs;
}

EckInlineNd CStringW ToString(UINT x, int iRadix = 10) noexcept
{
    CStringW rs(CchI32ToStrBuf);
    _ultow(x, rs.Data(), iRadix);
    rs.ReCalcLen();
    return rs;
}

EckInlineNd CStringW ToString(LONGLONG x, int iRadix = 10) noexcept
{
    CStringW rs(CchI64ToStrBuf);
    _i64tow(x, rs.Data(), iRadix);
    rs.ReCalcLen();
    return rs;
}

EckInlineNd CStringW ToString(ULONGLONG x, int iRadix = 10) noexcept
{
    CStringW rs(CchI64ToStrBuf);
    _ui64tow(x, rs.Data(), iRadix);
    rs.ReCalcLen();
    return rs;
}

EckInlineNd CStringW ToString(double x, int iPrecision = 6) noexcept
{
    CStringW rs{};
    rs.Format(L"%.*g", iPrecision, x);
    return rs;
}

namespace Literals
{
    EckInline auto operator""_rs(PCWSTR psz, size_t cch) { return CStringW(psz, (int)cch); }
    EckInline auto operator""_rs(PCSTR psz, size_t cch) { return CStringA(psz, (int)cch); }
}

EckInlineNd void ToFullWidth(CStringW& rs, PCWSTR pszText, int cchText = -1)
{
    const int cchResult = LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_FULLWIDTH,
        pszText, cchText, nullptr, 0, nullptr, nullptr, 0);
    LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_FULLWIDTH,
        pszText, cchText, rs.PushBack(cchResult), cchResult, nullptr, nullptr, 0);
}
EckInlineNd void ToHalfWidth(CStringW& rs, PCWSTR pszText, int cchText = -1)
{
    const int cchResult = LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_HALFWIDTH,
        pszText, cchText, nullptr, 0, nullptr, nullptr, 0);
    LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_HALFWIDTH,
        pszText, cchText, rs.PushBack(cchResult), cchResult, nullptr, nullptr, 0);
}

template<class TChar, class TTraits, class TAllocator>
    requires (sizeof(TChar) == 1)
void StrW2X(CStringT<TChar, TTraits, TAllocator>& rsResult,
    _In_reads_or_z_(cch) PCWCH pszText,
    int cch = -1,
    UINT uCP = CP_ACP,
    UINT uFlags = WC_COMPOSITECHECK) noexcept
{
    int cchBuf = WideCharToMultiByte(uCP, uFlags,
        pszText, cch, nullptr, 0, nullptr, nullptr);
    if (!cchBuf)
        return;
    if (cch < 0)
        --cchBuf;
    WideCharToMultiByte(uCP, uFlags, pszText, cch,
        (PCH)rsResult.PushBackNoExtra(cchBuf), cchBuf, nullptr, nullptr);
}
template<class TTraits = CCharTraits<CHAR>, class TAllocator = TStringDefaultAllocator<CHAR>>
EckInlineNd auto StrW2X(_In_reads_or_z_(cch) PCWCH pszText, int cch = -1, int uCP = CP_ACP) noexcept
{
    CStringT<CHAR, TTraits, TAllocator> rs{};
    StrW2X(rs, pszText, cch, uCP);
    return rs;
}

template<class TChar, class TTraits, class TAllocator>
    requires (sizeof(TChar) == 2)
void StrX2W(CStringT<TChar, TTraits, TAllocator>& rsResult,
    _In_reads_or_z_(cch) PCCH pszText,
    int cch = -1,
    UINT uCP = CP_ACP,
    UINT uFlags = MB_PRECOMPOSED) noexcept
{
    int cchBuf = MultiByteToWideChar(uCP, uFlags,
        pszText, cch, nullptr, 0);
    if (!cchBuf)
        return;
    if (cch < 0)
        --cchBuf;
    MultiByteToWideChar(uCP, uFlags,
        pszText, cch, rsResult.PushBackNoExtra(cchBuf), cchBuf);
}
template<class TTraits = CCharTraits<WCHAR>, class TAllocator = TStringDefaultAllocator<WCHAR>>
EckInlineNd auto StrX2W(_In_reads_or_z_(cch) PCCH pszText, int cch = -1, int uCP = CP_ACP) noexcept
{
    CStringT<WCHAR, TTraits, TAllocator> rs{};
    StrX2W(rs, pszText, cch, uCP);
    return rs;
}

EckInlineNd CStringW Format(PCWSTR pszFmt, ...)
{
    CStringW rs{};
    va_list vl;
    va_start(vl, pszFmt);
    rs.FormatV(pszFmt, vl);
    va_end(vl);
    return rs;
}
EckInlineNd CStringA Format(PCSTR pszFmt, ...)
{
    CStringA rs{};
    va_list vl;
    va_start(vl, pszFmt);
    rs.FormatV(pszFmt, vl);
    va_end(vl);
    return rs;
}
ECK_NAMESPACE_END

template<class TChar, class TTraits, class TAllocator>
struct std::hash<::eck::CStringT<TChar, TTraits, TAllocator>>
{
    EckInlineNdCe size_t operator()(
        const ::eck::CStringT<TChar, TTraits, TAllocator>& rs) const noexcept
    {
        return ::eck::Fnv1aHash((::eck::PCBYTE)rs.Data(), rs.Size() * sizeof(TChar));
    }
};