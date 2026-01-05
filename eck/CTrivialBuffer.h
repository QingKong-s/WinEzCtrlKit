#pragma once
#include "CAllocator.h"

ECK_NAMESPACE_BEGIN
template<class T_, class TAlloc_ = CDefAllocator<T_>>
    requires std::is_trivial<T_>::value
class CTrivialBuffer
{
public:
    using TElem = T_;
    using TAlloc = TAlloc_;

    using TAllocTraits = CAllocatorTraits<TAlloc>;

    using TPointer = TElem*;
    using TConstPointer = const TElem*;

    using TIterator = TPointer;
    using TConstIterator = TConstPointer;
    using TReverseIterator = std::reverse_iterator<TIterator>;
    using TConstReverseIterator = std::reverse_iterator<TConstIterator>;
private:
    TElem* m_pData{};
    size_t m_cSize{};
    size_t m_cCapacity{};

    [[no_unique_address]] TAlloc m_Alloc{};
public:
    CTrivialBuffer() = default;

    explicit CTrivialBuffer(size_t cInit) noexcept
    {
        ReSize(cInit);
    }
    CTrivialBuffer(_In_reads_(c) TConstPointer p, size_t c) noexcept
    {
        DupBuffer(p, c);
    }

    CTrivialBuffer(const CTrivialBuffer& x) noexcept
        : m_Alloc{ TAllocTraits::select_on_container_copy_construction(x.m_Alloc) }
    {
        DupBuffer(x.Data(), x.Size());
    }
    CTrivialBuffer(CTrivialBuffer&& x) noexcept
        : m_Alloc{ std::move(x.m_Alloc) }
    {
        std::swap(m_pData, x.m_pData);
        std::swap(m_cSize, x.m_cSize);
        std::swap(m_cCapacity, x.m_cCapacity);
    }

    ~CTrivialBuffer()
    {
        m_Alloc.deallocate(m_pData, m_cCapacity);
    }

    CTrivialBuffer& operator=(const CTrivialBuffer& x) noexcept
    {
        if (this == &x)
            return *this;
        if constexpr (!TAllocTraits::is_always_equal::value)
            if (m_Alloc != x.m_Alloc)
            {
                m_Alloc.deallocate(m_pData, m_cCapacity);
                m_pData = nullptr;
                m_cSize = 0;
                m_cCapacity = 0;
            }
        if constexpr (TAllocTraits::propagate_on_container_copy_assignment::value)
            m_Alloc = x.m_Alloc;

        DupBuffer(x.Data(), x.Size());
        return *this;
    }
    CTrivialBuffer& operator=(CTrivialBuffer&& x) noexcept
    {
        if (this == &x)
            return *this;
        if constexpr (TAllocTraits::propagate_on_container_move_assignment::value)
        {
            if constexpr (TAllocTraits::is_always_equal::value)
                m_Alloc = std::move(x.m_Alloc);
            else if (m_Alloc != x.m_Alloc)
            {
                m_Alloc.deallocate(m_pData, m_cCapacity);
                m_Alloc = std::move(x.m_Alloc);
                x.m_pData = nullptr;
                x.m_cSize = 0;
                x.m_cCapacity = 0;
            }
            return *this;
        }
        else if constexpr (!TAllocTraits::is_always_equal::value)
            if (m_Alloc != x.m_Alloc)
            {
                DupBuffer(x.Data(), x.Size());
                return *this;
            }
        std::swap(m_pData, x.m_pData);
        std::swap(m_cSize, x.m_cSize);
        std::swap(m_cCapacity, x.m_cCapacity);
        return *this;
    }

    void DupBuffer(_In_reads_(c) TConstPointer p, size_t c) noexcept
    {
        ReSizeExtra(c);
        memcpy(m_pData, p, c * sizeof(TElem));
    }

    void Reserve(size_t c) noexcept
    {
        if (m_cCapacity >= c)
            return;
        const auto pOld = m_pData;
        m_pData = m_Alloc.allocate(c);
        if (pOld)
        {
            memcpy(m_pData, pOld, m_cSize * sizeof(TElem));
            m_Alloc.deallocate(pOld, m_cCapacity);
        }
        m_cCapacity = c;
    }

    EckInline void ReSize(size_t c) noexcept
    {
        Reserve(c);
        m_cSize = c;
    }
    EckInline void ReSizeExtra(size_t c) noexcept
    {
        if (m_cCapacity < c)
            Reserve(TAllocTraits::MakeCapacity(c));
        m_cSize = c;
    }

    EckInlineCe void Clear() noexcept { m_cSize = 0; }

    EckInlineNdCe TPointer Data() noexcept { return m_pData; }
    EckInlineNdCe TConstPointer Data() const noexcept { return m_pData; }

    EckInlineNdCe size_t Size() const noexcept { return m_cSize; }
    EckInlineNdCe size_t ByteSize() const noexcept { return m_cSize * sizeof(TElem); }

    EckInlineNdCe BOOL IsEmpty() const noexcept { return !m_cSize; }

    EckInlineNdCe auto& Front() noexcept { EckAssert(!IsEmpty()); return *m_pData; }
    EckInlineNdCe auto& Front() const noexcept { EckAssert(!IsEmpty()); return *m_pData; }

    EckInlineNdCe auto& Back() noexcept { EckAssert(!IsEmpty()); return m_pData[m_cSize - 1]; }
    EckInlineNdCe auto& Back() const noexcept { EckAssert(!IsEmpty()); return m_pData[m_cSize - 1]; }

    EckInlineNdCe auto& At(size_t idx) noexcept
    {
        EckAssert(idx < Size());
        return Data()[idx];
    }
    EckInlineNdCe auto& At(size_t idx) const noexcept
    {
        EckAssert(idx < Size());
        return Data()[idx];
    }
    EckInlineNdCe auto& operator[](size_t idx) noexcept { return At(idx); }
    EckInlineNdCe auto& operator[](size_t idx) const noexcept { return At(idx); }

    EckInline void PushBack(_In_reads_(c) TConstPointer p, size_t c) noexcept
    {
        const auto cOld = Size();
        ReSizeExtra(Size() + c);
        memcpy(Data() + cOld, p, c * sizeof(TElem));
    }

    EckInline void PushBack(const TElem& e) noexcept
    {
        const auto cOld = Size();
        ReSizeExtra(Size() + 1);
        Data()[cOld] = e;
    }

    template<class T, size_t N>
    EckInline void PushBack(std::span<T, N> sp) noexcept { PushBack(sp.data(), sp.size()); }

    auto PushBackSize(size_t c) noexcept
    {
        const auto cOld = Size();
        ReSizeExtra(Size() + c);
        return Data() + cOld;
    }

    template<class... T>
    auto& EmplaceBack(T&&... Args) noexcept
    {
        const auto cOld = Size();
        ReSizeExtra(Size() + 1);
        new (Data() + cOld) TElem(std::forward<T>(Args)...);
        return Data()[cOld];
    }

    EckInlineCe void PopBack(size_t c = 1) noexcept
    {
        EckAssert(Size() >= c);
        m_cSize -= c;
    }

    void Erase(size_t pos, size_t c = 1) noexcept
    {
        EckAssert(Size() >= pos + c);
        memmove(
            Data() + pos,
            Data() + pos + c,
            (Size() - pos - c) * sizeof(TElem));
        m_cSize -= c;
    }

    void Replace(size_t posStart, size_t cReplacing,
        _In_reads_(cNew) TConstPointer pNew, size_t cNew) noexcept
    {
        EckAssert(posStart + cReplacing <= Size());
        const auto cOld = Size();
        ReSizeExtra(Size() + cNew - cReplacing);
        memmove(
            Data() + posStart + cNew,
            Data() + posStart + cReplacing,
            (cOld - posStart - cReplacing) * sizeof(TElem));
        memcpy(Data() + posStart, pNew, cNew * sizeof(TElem));
    }

    void Insert(size_t pos, _In_reads_(c) TConstPointer p, size_t c) noexcept
    {
        EckAssert(pos <= Size());
        const auto cOld = Size();
        ReSizeExtra(Size() + c);
        memmove(
            Data() + pos + c,
            Data() + pos,
            (cOld - pos) * sizeof(TElem));
        memcpy(Data() + pos, p, c * sizeof(TElem));
    }

    void Insert(size_t pos, const TElem& e) noexcept
    {
        EckAssert(pos <= Size());
        const auto cOld = Size();
        ReSizeExtra(Size() + 1);
        memmove(
            Data() + pos + 1,
            Data() + pos,
            (cOld - pos) * sizeof(TElem));
        Data()[pos] = e;
    }

    EckInlineNdCe TIterator begin() noexcept { return Data(); }
    EckInlineNdCe TIterator end() noexcept { return begin() + Size(); }
    EckInlineNdCe TConstIterator begin() const noexcept { return Data(); }
    EckInlineNdCe TConstIterator end() const noexcept { return begin() + Size(); }
    EckInlineNdCe TConstIterator cbegin() const noexcept { return begin(); }
    EckInlineNdCe TConstIterator cend() const noexcept { return end(); }
    EckInlineNdCe TReverseIterator rbegin() noexcept { return TReverseIterator(begin()); }
    EckInlineNdCe TReverseIterator rend() noexcept { return TReverseIterator(end()); }
    EckInlineNdCe TConstReverseIterator rbegin() const noexcept { return TConstReverseIterator(begin()); }
    EckInlineNdCe TConstReverseIterator rend() const noexcept { return TConstReverseIterator(end()); }
    EckInlineNdCe TConstReverseIterator crbegin() const noexcept { return rbegin(); }
    EckInlineNdCe TConstReverseIterator crend() const noexcept { return rend(); }
};
ECK_NAMESPACE_END