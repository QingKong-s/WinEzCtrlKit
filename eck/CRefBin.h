#pragma once
#include "CAllocator.h"
#include "MemUtility.h"
#include "Utility.h"

#include <initializer_list>

ECK_NAMESPACE_BEGIN
using TRefBinDefAllocator = CDefAllocator<BYTE>;

template<class TAlloc_ = TRefBinDefAllocator>
class CRefBinT
{
public:
    using TAlloc = TAlloc_;
    using TAllocTraits = CAllocatorTraits<TAlloc>;

    using TIterator = BYTE*;
    using TConstIterator = const BYTE*;
    using TReverseIterator = std::reverse_iterator<TIterator>;
    using TConstReverseIterator = std::reverse_iterator<TConstIterator>;
private:
    BYTE* m_pStream{};
    size_t m_cb{};
    size_t m_cbCapacity{};

    [[no_unique_address]] TAlloc m_Alloc{};

    EckInlineCe static void ResetThat(CRefBinT& x)
    {
        x.m_pStream = nullptr;
        x.m_cb = 0;
        x.m_cbCapacity = 0;
    }
public:
    CRefBinT() = default;

    explicit CRefBinT(const TAlloc& Al) : m_Alloc{ Al } {}

    explicit CRefBinT(size_t cb, const TAlloc& Al = TAlloc{})
        : m_cb{ cb }, m_Alloc{ Al }
    {
        Reserve(cb);
    }

    CRefBinT(PCVOID p, size_t cb, const TAlloc& Al = TAlloc{}) : m_Alloc{ Al }
    {
        EckAssert(p ? TRUE : (cb == 0));
        DupStream(p, cb);
    }

    CRefBinT(const CRefBinT& x)
        : m_Alloc{ TAllocTraits::select_on_container_copy_construction(x.m_Alloc) }
    {
        DupStream(x.Data(), x.Size());
    }

    CRefBinT(CRefBinT&& x) noexcept
        :m_pStream{ x.m_pStream }, m_cb{ x.m_cb }, m_cbCapacity{ x.m_cbCapacity },
        m_Alloc{ std::move(x.m_Alloc) }
    {
        ResetThat(x);
    }

    CRefBinT(std::initializer_list<BYTE> x, const TAlloc& Al = TAlloc{}) : m_Alloc{ Al }
    {
        DupStream(x.begin(), x.size());
    }

    ~CRefBinT()
    {
        m_Alloc.deallocate(m_pStream, m_cbCapacity);
    }

    CRefBinT& operator=(const CRefBinT& x)
    {
        if constexpr (!TAllocTraits::is_always_equal::value)
            if (m_Alloc != x.m_Alloc)
            {
                m_Alloc.deallocate(m_pStream, m_cbCapacity);
                ResetThat(*this);
            }
        if constexpr (TAllocTraits::propagate_on_container_copy_assignment::value)
            m_Alloc = x.m_Alloc;

        DupStream(x.Data(), x.Size());
        return *this;
    }
    CRefBinT& operator=(CRefBinT&& x) noexcept
    {
        if (this == &x)
            return *this;
        if constexpr (TAllocTraits::propagate_on_container_move_assignment::value)
        {
            m_Alloc.deallocate(m_pStream, m_cbCapacity);
            m_Alloc = std::move(x.m_Alloc);
            m_pStream = x.m_pStream;
            m_cb = x.m_cb;
            m_cbCapacity = x.m_cbCapacity;
            ResetThat(x);
        }
        else if constexpr (!TAllocTraits::is_always_equal::value)
            if (m_Alloc != x.m_Alloc)
            {
                DupStream(x.Data(), x.Size());
                return *this;
            }

        std::swap(m_pStream, x.m_pStream);
        std::swap(m_cb, x.m_cb);
        std::swap(m_cbCapacity, x.m_cbCapacity);
        return *this;
    }
    EckInline CRefBinT& operator=(std::initializer_list<BYTE> x)
    {
        DupStream(x);
        return *this;
    }
    template<class T, size_t E>
    CRefBinT& operator=(std::span<T, E> x)
    {
        DupStream(x.data(), x.size_bytes());
        return *this;
    }

    template<class TAlloc1>
    EckInline CRefBinT& operator+=(const CRefBinT<TAlloc1>& x)
    {
        PushBack(x.Data(), x.Size());
        return *this;
    }
    template<class T, class TAlloc1>
    EckInline CRefBinT& operator+=(const std::vector<T, TAlloc1>& x)
    {
        PushBack(x.data(), x.size() * sizeof(T));
        return *this;
    }
    template<class T, size_t E>
    EckInline CRefBinT& operator+=(const std::span<T, E>& x)
    {
        PushBack(x.data(), x.size_bytes());
        return *this;
    }

    EckInlineNd TAlloc GetAllocator() const { return m_Alloc; }
    EckInlineNdCe BYTE& At(size_t idx) { EckAssert(idx < Size()); return *(Data() + idx); }
    EckInlineNdCe BYTE At(size_t idx) const { EckAssert(idx < Size()); return *(Data() + idx); }
    EckInlineNdCe BYTE& operator[](size_t idx) { return At(idx); }
    EckInlineNdCe BYTE operator[](size_t idx) const { return At(idx); }
    EckInlineNdCe BYTE& Front() { return At(0); }
    EckInlineNdCe BYTE Front() const { return At(0); }
    EckInlineNdCe BYTE& Back() { return At(Size() - 1); }
    EckInlineNdCe BYTE Back() const { return At(Size() - 1); }
    EckInlineNdCe BYTE* Data() { return m_pStream; }
    EckInlineNdCe const BYTE* Data() const { return m_pStream; }
    EckInlineNdCe size_t Capacity() const { return m_cbCapacity; }
    EckInlineNdCe size_t Size() const { return m_cb; }
    EckInlineNdCe BOOL IsEmpty() const { return Size() == 0; }

    EckInline void DupStream(PCVOID p, size_t cb)
    {
        ReSizeExtra(cb);
        memcpy(Data(), p, cb);
    }
    EckInline void DupStream(std::initializer_list<BYTE> x)
    {
        DupStream(x.begin(), x.size());
    }
    template<class T, size_t E>
    EckInline void DupStream(std::span<T, E> x)
    {
        DupStream(x.data(), x.size_bytes());
    }

    /// <summary>
    /// 依附指针。
    /// 先前的内存将被释放
    /// </summary>
    /// <param name="p">指针，必须可通过当前分配器解分配</param>
    /// <param name="cbCapacity">容量</param>
    /// <param name="cb">当前长度</param>
    void Attach(BYTE* p, size_t cbCapacity, size_t cb)
    {
        m_Alloc.deallocate(m_pStream, m_cbCapacity);
        if (!p)
        {
            m_pStream = nullptr;
            m_cb = m_cbCapacity = 0u;
        }
        else
        {
            m_pStream = p;
            m_cbCapacity = cbCapacity;
            m_cb = cb;
        }
    }

    /// <summary>
    /// 拆离指针
    /// </summary>
    /// <param name="cbCapacity">容量</param>
    /// <param name="cb">长度</param>
    /// <returns>指针，必须通过与当前分配器相等的分配器解分配</returns>
    [[nodiscard]] EckInline BYTE* Detach(size_t& cbCapacity, size_t& cb)
    {
        const auto pTemp = m_pStream;
        m_pStream = nullptr;

        cbCapacity = m_cbCapacity;
        m_cbCapacity = 0u;

        cb = m_cb;
        m_cb = 0u;
        return pTemp;
    }

    EckInline size_t CopyTo(void* pDst, size_t cbMax) const
    {
        if (cbMax > m_cb)
            cbMax = m_cb;
        memcpy(pDst, m_pStream, cbMax);
        return cbMax;
    }

    EckInline void Reserve(size_t cb)
    {
        if (m_cbCapacity >= cb)
            return;
        const auto pOld = m_pStream;
        m_pStream = m_Alloc.allocate(cb);
        if (pOld)
        {
            memcpy(Data(), pOld, Size());
            m_Alloc.deallocate(pOld, m_cbCapacity);
        }
        m_cbCapacity = cb;
    }

    EckInline void ReSize(size_t cb)
    {
        Reserve(cb);
        m_cb = cb;
    }
    EckInline void ReSizeExtra(size_t cb)
    {
        if (m_cbCapacity < cb)
            Reserve(TAllocTraits::MakeCapacity(cb));
        m_cb = cb;
    }

    /// <summary>
    /// 替换
    /// </summary>
    /// <param name="posStart">替换位置</param>
    /// <param name="cbReplacing">替换长度</param>
    /// <param name="pNew">用作替换的字节集指针</param>
    /// <param name="cbNew">用作替换的字节集长度</param>
    EckInline void Replace(size_t posStart, size_t cbReplacing, PCVOID pNew = nullptr, size_t cbNew = 0u)
    {
        EckAssert(cbNew ? (!!pNew) : TRUE);
        const size_t cbOrg = Size();
        ReSizeExtra(Size() + cbNew - cbReplacing);
        memmove(
            Data() + posStart + cbNew,
            Data() + posStart + cbReplacing,
            cbOrg - posStart - cbReplacing);
        memcpy(Data() + posStart, pNew, cbNew);
    }

    /// <summary>
    /// 替换
    /// </summary>
    /// <param name="posStart">替换位置</param>
    /// <param name="cbReplacing">替换长度</param>
    /// <param name="rb">用作替换的字节集</param>
    template<class TAlloc1>
    EckInline void Replace(size_t posStart, size_t cbReplacing, const CRefBinT<TAlloc1>& rb)
    {
        Replace(posStart, cbReplacing, rb.Data(), rb.Size());
    }
    EckInline void Replace(size_t posStart, size_t cbReplacing, std::initializer_list<BYTE> x)
    {
        Replace(posStart, cbReplacing, x.begin(), x.size());
    }

    /// <summary>
    /// 子字节集替换
    /// </summary>
    /// <param name="pReplacedBin">被替换的字节集指针</param>
    /// <param name="cbReplacedBin">被替换的字节集长度</param>
    /// <param name="pSrcBin">用作替换的字节集指针</param>
    /// <param name="cbSrcBin">用作替换的字节集长度</param>
    /// <param name="posStart">起始位置</param>
    /// <param name="cReplacing">替换进行的次数，0为执行所有替换</param>
    void ReplaceSubBin(PCVOID pReplacedBin, size_t cbReplacedBin,
        PCVOID pSrcBin, size_t cbSrcBin, size_t posStart = 0, int cReplacing = 0)
    {
        size_t pos = 0u;
        for (int c = 1;; ++c)
        {
            pos = FindBin(m_pStream, m_cb, pReplacedBin, cbReplacedBin, posStart + pos);
            if (pos == BinNPos)
                break;
            Replace(pos, cbReplacedBin, pSrcBin, cbSrcBin);
            pos += cbSrcBin;
            if (c == cReplacing)
                break;
        }
    }

    /// <summary>
    /// 子字节集替换
    /// </summary>
    /// <param name="rbReplacedBin">被替换的字节集</param>
    /// <param name="rbSrcBin">用作替换的字节集</param>
    /// <param name="posStart">起始位置</param>
    /// <param name="cReplacing">替换进行的次数，0为执行所有替换</param>
    template<class TAlloc1, class TAlloc2>
    EckInline void ReplaceSubBin(const CRefBinT<TAlloc1>& rbReplacedBin,
        const CRefBinT<TAlloc2>& rbSrcBin, size_t posStart = 0, int cReplacing = -1)
    {
        ReplaceSubBin(rbReplacedBin.Data(), rbReplacedBin.Size(), rbSrcBin.Data(), rbSrcBin.Size(),
            posStart, cReplacing);
    }
    EckInline void ReplaceSubBin(std::initializer_list<BYTE> ilReplacedBin,
        std::initializer_list<BYTE> ilSrcBin, size_t posStart = 0, int cReplacing = -1)
    {
        ReplaceSubBin(ilReplacedBin.begin(), ilReplacedBin.size(), ilSrcBin.begin(), ilSrcBin.size(),
            posStart, cReplacing);
    }

    EckInline void MakeSpace(size_t cbSize, size_t posStart = 0u)
    {
        ReSizeExtra(cbSize + posStart);
        ZeroMemory(Data() + posStart, cbSize);
    }

    void MakeRepeatedBinSequence(size_t cCount, PCVOID pBin, size_t cbBin, size_t posStart = 0u)
    {
        ReSizeExtra(posStart + cCount * cbBin);
        BYTE* pCurr{ Data() + posStart };
        size_t i;
        for (size_t i = 0; i < cCount; ++i, pCurr += cbBin)
            memcpy(pCurr, pBin, cbBin);
    }

    EckInline CRefBinT& PushBack(PCVOID p, size_t cb)
    {
        ReSizeExtra(m_cb + cb);
        memcpy(Data() + Size() - cb, p, cb);
        return *this;
    }
    EckInline CRefBinT& PushBack(const CRefBinT& rb)
    {
        return PushBack(rb.Data(), rb.Size());
    }
    EckInline CRefBinT& PushBack(std::initializer_list<BYTE> x)
    {
        return PushBack(x.begin(), x.size());
    }
    template<class T, class TAlloc1>
    EckInline CRefBinT& PushBack(const std::vector<T, TAlloc1>& x)
    {
        return PushBack(x.data(), x.size() * sizeof(T));
    }
    template<class T, size_t E>
    EckInline CRefBinT& PushBack(std::span<T, E> x)
    {
        return PushBack(x.data(), x.size_bytes());
    }
    EckInline CRefBinT& PushBackByte(BYTE by)
    {
        ReSizeExtra(Size() + 1);
        *(Data() + Size() - 1) = by;
        return *this;
    }

    EckInline BYTE* PushBack(size_t cb)
    {
        ReSizeExtra(m_cb + cb);
        return Data() + m_cb - cb;
    }
    template<class T>
    EckInline T* PushBack()
    {
        return (T*)PushBack(sizeof(T));
    }
    EckInline BYTE* PushBackNoExtra(size_t cb)
    {
        ReSize(m_cb + cb);
        return Data() + m_cb - cb;
    }

    EckInline void PopBack(size_t cb = 1)
    {
        EckAssert(m_cb >= cb);
        m_cb -= cb;
    }

    EckInline constexpr void Clear() { m_cb = 0u; }

    EckInline void Zero() { RtlZeroMemory(Data(), Size()); }

    EckInline CRefBinT& Insert(size_t pos, PCVOID p, size_t cb)
    {
        EckAssert(pos <= Size());
        EckAssert(p ? TRUE : (cb == 0));
        ReSizeExtra(Size() + cb);
        memmove(
            Data() + pos + cb,
            Data() + pos,
            Size() - cb - pos);
        memcpy(Data() + pos, p, cb);
        return *this;
    }
    template<class TAlloc1 = TAlloc>
    EckInline CRefBinT& Insert(size_t pos, const CRefBinT<TAlloc1>& rb)
    {
        return Insert(pos, rb.Data(), rb.Size());
    }
    EckInline CRefBinT& Insert(size_t pos, std::initializer_list<BYTE> il)
    {
        return Insert(pos, il.begin(), il.size());
    }
    template<class T, class TAlloc1>
    EckInline CRefBinT& Insert(size_t pos, const std::vector<T, TAlloc1>& x)
    {
        return Insert(pos, x.data(), x.size() * sizeof(T));
    }
    template<class T, size_t E>
    EckInline CRefBinT& Insert(size_t pos, std::span<T, E> x)
    {
        return Insert(pos, x.data(), x.size_bytes());
    }
    EckInline CRefBinT& Insert(size_t pos, BYTE by)
    {
        return Insert(pos, &by, 1u);
    }

    EckInline void Erase(size_t pos, size_t cb)
    {
        EckAssert(Size() >= pos + cb);
        memmove(
            Data() + pos,
            Data() + pos + cb,
            Size() - pos - cb);
        m_cb -= cb;
    }

    void ShrinkToFit()
    {
        EckAssert(m_cbCapacity >= m_cb);
        if (m_cbCapacity == m_cb)
            return;
        const auto pOld = m_pStream;
        m_pStream = m_Alloc.allocate(m_cb);
        memcpy(Data(), pOld, Size());
        m_Alloc.deallocate(pOld, m_cbCapacity);
        m_cbCapacity = m_cb;
    }

    EckInlineCe void ExtendToCapacity() { m_cb = Capacity(); }

    EckInline size_t Find(PCVOID pSub, size_t cbSub, size_t posStart = 0) const
    {
        return FindBin(Data(), Size(), pSub, cbSub, posStart);
    }
    EckInline size_t RFind(PCVOID pSub, size_t cbSub, size_t posStart = SizeTMax) const
    {
        return FindBinRev(Data(), Size(), pSub, cbSub, posStart);
    }

    EckInline size_t FindByte(BYTE by, size_t posStart = 0) const noexcept
    {
        const auto p = memchr(Data() + posStart, by, Size());
        return p ? size_t(p - Data()) : BinNPos;
    }

    EckInlineNdCe std::span<BYTE> ToSpan()
    {
        return std::span<BYTE>(Data(), Size());
    }
    EckInlineNdCe std::span<const BYTE> ToSpan() const
    {
        return std::span<const BYTE>(Data(), Size());
    }

    template<class TAlloc1 = TAlloc>
    EckInlineNd CRefBinT<TAlloc1> SubBin(size_t posBegin, size_t cb) const
    {
        return CRefBinT<TAlloc1>(Data() + posBegin, cb);
    }
    EckInlineNdCe std::span<BYTE> SubSpan(size_t posBegin, size_t cb)
    {
        return std::span<BYTE>(Data() + posBegin, cb);
    }
    EckInlineNdCe std::span<const BYTE> SubSpan(size_t posBegin, size_t cb) const
    {
        return std::span<const BYTE>(Data() + posBegin, cb);
    }

    EckInlineNdCe TIterator begin() { return Data(); }
    EckInlineNdCe TIterator end() { return begin() + Size(); }
    EckInlineNdCe TConstIterator begin() const { return Data(); }
    EckInlineNdCe TConstIterator end() const { return begin() + Size(); }
    EckInlineNdCe TConstIterator cbegin() const { return begin(); }
    EckInlineNdCe TConstIterator cend() const { return end(); }
    EckInlineNdCe TReverseIterator rbegin() { return TReverseIterator(begin()); }
    EckInlineNdCe TReverseIterator rend() { return TReverseIterator(end()); }
    EckInlineNdCe TConstReverseIterator rbegin() const { return TConstReverseIterator(begin()); }
    EckInlineNdCe TConstReverseIterator rend() const { return TConstReverseIterator(end()); }
    EckInlineNdCe TConstReverseIterator crbegin() const { return rbegin(); }
    EckInlineNdCe TConstReverseIterator crend() const { return rend(); }
};

using CRefBin = CRefBinT<TRefBinDefAllocator>;

template<class TAlloc = TRefBinDefAllocator, class TAlloc1, class TAlloc2>
CRefBinT<TAlloc> operator+(const CRefBinT<TAlloc1>& rb1, const CRefBinT<TAlloc2>& rb2)
{
    CRefBinT<TAlloc> rb(rb1.Size() + rb2.Size());
    memcpy(rb.Data(), rb1.Data(), rb1.Size());
    memcpy(rb.Data() + rb1.Size(), rb2.Data(), rb2.Size());
    return rb;
}

template<class TAlloc1, class TAlloc2>
[[nodiscard]] EckInline bool operator==(const CRefBinT<TAlloc1>& rb1, const CRefBinT<TAlloc2>& rb2)
{
    if (rb1.Size() != rb2.Size())
        return false;
    else
        return memcmp(rb1.Data(), rb2.Data(), rb1.Size()) == 0;
}
template<class TAlloc>
[[nodiscard]] EckInline bool operator==(const CRefBinT<TAlloc>& rb1, std::initializer_list<BYTE> il)
{
    if (rb1.Size() != il.size())
        return false;
    else
        return memcmp(rb1.Data(), il.begin(), il.size()) == 0;
}

template<class TAlloc1, class TAlloc2>
[[nodiscard]] EckInline size_t FindBin(const CRefBinT<TAlloc1>& rbMain,
    const CRefBinT<TAlloc2>& rbSub, size_t posStart = 0)
{
    return FindBin(rbMain.Data(), rbMain.Size(), rbSub.Data(), rbSub.Size(), posStart);
}
template<class TAlloc1, class TAlloc2>
[[nodiscard]] EckInline size_t FindBinRev(const CRefBinT<TAlloc1>& rbMain,
    const CRefBinT<TAlloc2>& rbSub, size_t posStart = 0)
{
    return FindBinRev(rbMain.Data(), rbMain.Size(), rbSub.Data(), rbSub.Size(), posStart);
}
ECK_NAMESPACE_END

template<class TAlloc>
struct std::hash<::eck::CRefBinT<TAlloc>>
{
    [[nodiscard]] EckInline size_t operator()(
        const ::eck::CRefBinT<TAlloc>& rb) const noexcept
    {
        return ::eck::Fnv1aHash(rb.Data(), rb.Size());
    }
};