#pragma once
#include "CAllocator.h"
#include "MemUtility.h"
#include "Utility.h"

#include <initializer_list>

ECK_NAMESPACE_BEGIN
using TByteBufferDefaultAllocator = CDefaultAllocator<BYTE>;

template<class TAllocator_ = TByteBufferDefaultAllocator>
class CByteBufferT
{
public:
    using TAllocator = TAllocator_;
    using TAllocatorTraits = CAllocatorTraits<TAllocator>;

    using TIterator = BYTE*;
    using TConstIterator = const BYTE*;
    using TReverseIterator = std::reverse_iterator<TIterator>;
    using TConstReverseIterator = std::reverse_iterator<TConstIterator>;
private:
    BYTE* m_pStream{};
    size_t m_cb{};
    size_t m_cbCapacity{};

    [[no_unique_address]] TAllocator m_Alloc{};

    EckInlineCe static void ResetThat(CByteBufferT& x)
    {
        x.m_pStream = nullptr;
        x.m_cb = 0;
        x.m_cbCapacity = 0;
    }
public:
    CByteBufferT() = default;

    explicit CByteBufferT(const TAllocator& Al) : m_Alloc{ Al } {}

    explicit CByteBufferT(size_t cb, const TAllocator& Al = TAllocator{})
        : m_cb{ cb }, m_Alloc{ Al }
    {
        Reserve(cb);
    }

    CByteBufferT(PCVOID p, size_t cb, const TAllocator& Al = TAllocator{}) : m_Alloc{ Al }
    {
        EckAssert(p ? TRUE : (cb == 0));
        Assign(p, cb);
    }

    CByteBufferT(const CByteBufferT& x)
        : m_Alloc{ TAllocatorTraits::select_on_container_copy_construction(x.m_Alloc) }
    {
        Assign(x.Data(), x.Size());
    }

    CByteBufferT(CByteBufferT&& x) noexcept
        :m_pStream{ x.m_pStream }, m_cb{ x.m_cb }, m_cbCapacity{ x.m_cbCapacity },
        m_Alloc{ std::move(x.m_Alloc) }
    {
        ResetThat(x);
    }

    CByteBufferT(std::initializer_list<BYTE> x, const TAllocator& Al = TAllocator{}) : m_Alloc{ Al }
    {
        Assign(x.begin(), x.size());
    }

    ~CByteBufferT()
    {
        m_Alloc.deallocate(m_pStream, m_cbCapacity);
    }

    CByteBufferT& operator=(const CByteBufferT& x)
    {
        if (this == &x)
            return *this;
        if constexpr (TAllocatorTraits::propagate_on_container_copy_assignment::value)
            if constexpr (TAllocatorTraits::is_always_equal::value)
                m_Alloc = x.m_Alloc;
            else if (m_Alloc != x.m_Alloc)
                m_Alloc = x.m_Alloc;

        Assign(x.Data(), x.Size());
        return *this;
    }
    CByteBufferT& operator=(CByteBufferT&& x) noexcept
    {
        if (this == &x)
            return *this;
        if constexpr (TAllocatorTraits::propagate_on_container_move_assignment::value)
        {
            if constexpr (TAllocatorTraits::is_always_equal::value)
                m_Alloc = std::move(x.m_Alloc);
            else if (m_Alloc != x.m_Alloc)
            {
                m_Alloc.deallocate(m_pStream, m_cbCapacity);
                ResetThat(*this);
                m_Alloc = std::move(x.m_Alloc);
            }
        }
        else if constexpr (!TAllocatorTraits::is_always_equal::value)
            if (m_Alloc != x.m_Alloc)
            {
                Assign(x.Data(), x.Size());
                return *this;
            }

        std::swap(m_pStream, x.m_pStream);
        std::swap(m_cb, x.m_cb);
        std::swap(m_cbCapacity, x.m_cbCapacity);
        return *this;
    }
    EckInline CByteBufferT& operator=(std::initializer_list<BYTE> x)
    {
        Assign(x);
        return *this;
    }
    template<class T, size_t E>
    CByteBufferT& operator=(std::span<T, E> x)
    {
        Assign(x.data(), x.size_bytes());
        return *this;
    }

    template<class TAllocator1>
    EckInline CByteBufferT& operator+=(const CByteBufferT<TAllocator1>& x)
    {
        PushBack(x.Data(), x.Size());
        return *this;
    }
    template<class T, class TAllocator1>
    EckInline CByteBufferT& operator+=(const std::vector<T, TAllocator1>& x)
    {
        PushBack(x.data(), x.size() * sizeof(T));
        return *this;
    }
    template<class T, size_t E>
    EckInline CByteBufferT& operator+=(const std::span<T, E>& x)
    {
        PushBack(x.data(), x.size_bytes());
        return *this;
    }

    EckInlineNd TAllocator GetAllocator() const { return m_Alloc; }
    EckInlineNdCe BYTE& At(size_t idx) noexcept { EckAssert(idx < Size()); return *(Data() + idx); }
    EckInlineNdCe BYTE At(size_t idx) const noexcept { EckAssert(idx < Size()); return *(Data() + idx); }
    EckInlineNdCe BYTE& operator[](size_t idx) noexcept { return At(idx); }
    EckInlineNdCe BYTE operator[](size_t idx) const noexcept { return At(idx); }
    EckInlineNdCe BYTE& Front() noexcept { return At(0); }
    EckInlineNdCe BYTE Front() const noexcept { return At(0); }
    EckInlineNdCe BYTE& Back() noexcept { return At(Size() - 1); }
    EckInlineNdCe BYTE Back() const noexcept { return At(Size() - 1); }
    EckInlineNdCe BYTE* Data() noexcept { return m_pStream; }
    EckInlineNdCe const BYTE* Data() const noexcept { return m_pStream; }
    EckInlineNdCe size_t Capacity() const noexcept { return m_cbCapacity; }
    EckInlineNdCe size_t Size() const noexcept { return m_cb; }
    EckInlineNdCe BOOL IsEmpty() const noexcept { return Size() == 0; }

    EckInline void Assign(PCVOID p, size_t cb)
    {
        ReSizeExtra(cb);
        memcpy(Data(), p, cb);
    }
    EckInline void Assign(std::initializer_list<BYTE> x)
    {
        Assign(x.begin(), x.size());
    }
    template<class T, size_t E>
    EckInline void Assign(std::span<T, E> x)
    {
        Assign(x.data(), x.size_bytes());
    }

    /// <summary>
    /// 依附指针。
    /// 先前的内存将被释放
    /// </summary>
    /// <param name="p">指针，必须可通过当前分配器解分配</param>
    /// <param name="cbCapacity">容量</param>
    /// <param name="cb">当前长度</param>
    void Attach(BYTE* p, size_t cbCapacity, size_t cb) noexcept
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
    EckInlineNd BYTE* Detach(size_t& cbCapacity, size_t& cb) noexcept
    {
        const auto pTemp = m_pStream;
        m_pStream = nullptr;

        cbCapacity = m_cbCapacity;
        m_cbCapacity = 0u;

        cb = m_cb;
        m_cb = 0u;
        return pTemp;
    }

    EckInline size_t CopyTo(void* pDst, size_t cbMax) const noexcept
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
            Reserve(TAllocatorTraits::MakeCapacity(cb));
        m_cb = cb;
    }

    /// <summary>
    /// 替换
    /// </summary>
    /// <param name="posStart">替换位置</param>
    /// <param name="cbReplacing">替换长度</param>
    /// <param name="pNew">用作替换的字节集指针</param>
    /// <param name="cbNew">用作替换的字节集长度</param>
    EckInline void Replace(size_t posStart, size_t cbReplacing,
        PCVOID pNew = nullptr, size_t cbNew = 0u)
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
    template<class TAllocator1>
    EckInline void Replace(size_t posStart, size_t cbReplacing, const CByteBufferT<TAllocator1>& rb)
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
    void ReplaceSub(PCVOID pReplacedBin, size_t cbReplacedBin,
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
    template<class TAllocator1, class TAllocator2>
    EckInline void ReplaceSub(const CByteBufferT<TAllocator1>& rbReplacedBin,
        const CByteBufferT<TAllocator2>& rbSrcBin, size_t posStart = 0, int cReplacing = -1)
    {
        ReplaceSub(rbReplacedBin.Data(), rbReplacedBin.Size(), rbSrcBin.Data(), rbSrcBin.Size(),
            posStart, cReplacing);
    }
    EckInline void ReplaceSub(std::initializer_list<BYTE> ilReplacedBin,
        std::initializer_list<BYTE> ilSrcBin, size_t posStart = 0, int cReplacing = -1)
    {
        ReplaceSub(ilReplacedBin.begin(), ilReplacedBin.size(), ilSrcBin.begin(), ilSrcBin.size(),
            posStart, cReplacing);
    }

    EckInline void MakeSpace(size_t cbSize, size_t posStart = 0u)
    {
        ReSizeExtra(cbSize + posStart);
        ZeroMemory(Data() + posStart, cbSize);
    }

    void MakeRepeatSequence(size_t cCount, PCVOID pBin, size_t cbBin, size_t posStart = 0u)
    {
        ReSizeExtra(posStart + cCount * cbBin);
        BYTE* pCurr{ Data() + posStart };
        size_t i;
        for (size_t i = 0; i < cCount; ++i, pCurr += cbBin)
            memcpy(pCurr, pBin, cbBin);
    }

    EckInline CByteBufferT& PushBack(PCVOID p, size_t cb)
    {
        ReSizeExtra(m_cb + cb);
        memcpy(Data() + Size() - cb, p, cb);
        return *this;
    }
    EckInline CByteBufferT& PushBack(const CByteBufferT& rb)
    {
        return PushBack(rb.Data(), rb.Size());
    }
    EckInline CByteBufferT& PushBack(std::initializer_list<BYTE> x)
    {
        return PushBack(x.begin(), x.size());
    }
    template<class T, class TAllocator1>
    EckInline CByteBufferT& PushBack(const std::vector<T, TAllocator1>& x)
    {
        return PushBack(x.data(), x.size() * sizeof(T));
    }
    template<class T, size_t E>
    EckInline CByteBufferT& PushBack(std::span<T, E> x)
    {
        return PushBack(x.data(), x.size_bytes());
    }
    EckInline CByteBufferT& PushBackByte(BYTE by)
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

    EckInline constexpr void Clear() noexcept { m_cb = 0u; }

    EckInline void Zero() noexcept { RtlZeroMemory(Data(), Size()); }

    EckInline CByteBufferT& Insert(size_t pos, PCVOID p, size_t cb)
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
    template<class TAllocator1 = TAllocator>
    EckInline CByteBufferT& Insert(size_t pos, const CByteBufferT<TAllocator1>& rb)
    {
        return Insert(pos, rb.Data(), rb.Size());
    }
    EckInline CByteBufferT& Insert(size_t pos, std::initializer_list<BYTE> il)
    {
        return Insert(pos, il.begin(), il.size());
    }
    template<class T, class TAllocator1>
    EckInline CByteBufferT& Insert(size_t pos, const std::vector<T, TAllocator1>& x)
    {
        return Insert(pos, x.data(), x.size() * sizeof(T));
    }
    template<class T, size_t E>
    EckInline CByteBufferT& Insert(size_t pos, std::span<T, E> x)
    {
        return Insert(pos, x.data(), x.size_bytes());
    }
    EckInline CByteBufferT& Insert(size_t pos, BYTE by)
    {
        return Insert(pos, &by, 1u);
    }

    EckInline void Erase(size_t pos, size_t cb) noexcept
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

    EckInlineCe void ExtendToCapacity() noexcept { m_cb = Capacity(); }

    EckInline size_t Find(PCVOID pSub, size_t cbSub, size_t posStart = 0) const noexcept
    {
        return FindBin(Data(), Size(), pSub, cbSub, posStart);
    }
    EckInline size_t RFind(PCVOID pSub, size_t cbSub, size_t posStart = SizeTMax) const noexcept
    {
        return FindBinRev(Data(), Size(), pSub, cbSub, posStart);
    }

    EckInline size_t FindByte(BYTE by, size_t posStart = 0) const noexcept
    {
        const auto p = memchr(Data() + posStart, by, Size());
        return p ? size_t(p - Data()) : BinNPos;
    }

    EckInlineNdCe std::span<BYTE> ToSpan() noexcept
    {
        return std::span<BYTE>(Data(), Size());
    }
    EckInlineNdCe std::span<const BYTE> ToSpan() const noexcept
    {
        return std::span<const BYTE>(Data(), Size());
    }

    EckInlineNdCe std::span<BYTE> SubSpan(size_t posBegin, size_t cb) noexcept
    {
        return std::span<BYTE>(Data() + posBegin, cb);
    }
    EckInlineNdCe std::span<const BYTE> SubSpan(size_t posBegin, size_t cb) const noexcept
    {
        return std::span<const BYTE>(Data() + posBegin, cb);
    }

    template<class TAllocator1 = TAllocator>
    EckInlineNd CByteBufferT<TAllocator1> SubByteBuffer(size_t posBegin, size_t cb) const
    {
        return CByteBufferT<TAllocator1>(Data() + posBegin, cb);
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

using CByteBuffer = CByteBufferT<TByteBufferDefaultAllocator>;

template<class TAllocator = TByteBufferDefaultAllocator, class TAllocator1, class TAllocator2>
CByteBufferT<TAllocator> operator+(const CByteBufferT<TAllocator1>& rb1, const CByteBufferT<TAllocator2>& rb2)
{
    CByteBufferT<TAllocator> rb(rb1.Size() + rb2.Size());
    memcpy(rb.Data(), rb1.Data(), rb1.Size());
    memcpy(rb.Data() + rb1.Size(), rb2.Data(), rb2.Size());
    return rb;
}

template<class TAllocator1, class TAllocator2>
EckInlineNd bool operator==(const CByteBufferT<TAllocator1>& rb1, const CByteBufferT<TAllocator2>& rb2) noexcept
{
    if (rb1.Size() != rb2.Size())
        return false;
    else
        return memcmp(rb1.Data(), rb2.Data(), rb1.Size()) == 0;
}
template<class TAllocator>
EckInlineNd bool operator==(const CByteBufferT<TAllocator>& rb1, std::initializer_list<BYTE> il) noexcept
{
    if (rb1.Size() != il.size())
        return false;
    else
        return memcmp(rb1.Data(), il.begin(), il.size()) == 0;
}

template<class TAllocator1, class TAllocator2>
EckInlineNd size_t FindBin(const CByteBufferT<TAllocator1>& rbMain,
    const CByteBufferT<TAllocator2>& rbSub, size_t posStart = 0) noexcept
{
    return FindBin(rbMain.Data(), rbMain.Size(), rbSub.Data(), rbSub.Size(), posStart);
}
template<class TAllocator1, class TAllocator2>
EckInlineNd size_t FindBinRev(const CByteBufferT<TAllocator1>& rbMain,
    const CByteBufferT<TAllocator2>& rbSub, size_t posStart = 0) noexcept
{
    return FindBinRev(rbMain.Data(), rbMain.Size(), rbSub.Data(), rbSub.Size(), posStart);
}
ECK_NAMESPACE_END

template<class TAllocator>
struct std::hash<::eck::CByteBufferT<TAllocator>>
{
    EckInlineNd size_t operator()(
        const ::eck::CByteBufferT<TAllocator>& rb) const noexcept
    {
        return ::eck::Fnv1aHash(rb.Data(), rb.Size());
    }
};