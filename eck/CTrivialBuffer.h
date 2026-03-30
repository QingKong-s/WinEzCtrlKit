#pragma once
#include "Allocator.h"

ECK_NAMESPACE_BEGIN
template<class T_, class TAllocator_ = CDefaultAllocator<T_>>
    requires std::is_trivial<T_>::value
class CTrivialBuffer
{
public:
    using T = T_;
    using TAllocator = TAllocator_;

    using TAllocatorTraits = CAllocatorTraits<TAllocator>;

    using TPointer = T*;
    using TConstPointer = const T*;

    using TIterator = TPointer;
    using TConstIterator = TConstPointer;
    using TReverseIterator = std::reverse_iterator<TIterator>;
    using TConstReverseIterator = std::reverse_iterator<TConstIterator>;
private:
    T* m_pData{};
    size_t m_cSize{};
    size_t m_cCapacity{};

    [[no_unique_address]] TAllocator m_Alloc{};
public:
    CTrivialBuffer() = default;

    explicit CTrivialBuffer(size_t cInit) noexcept
    {
        ReSize(cInit);
    }
    CTrivialBuffer(_In_reads_(c) TConstPointer p, size_t c) noexcept
    {
        Assign(p, c);
    }

    CTrivialBuffer(const CTrivialBuffer& x) noexcept
        : m_Alloc{ TAllocatorTraits::select_on_container_copy_construction(x.m_Alloc) }
    {
        Assign(x.Data(), x.Size());
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
        if constexpr (!TAllocatorTraits::is_always_equal::value)
            if (m_Alloc != x.m_Alloc)
            {
                m_Alloc.deallocate(m_pData, m_cCapacity);
                m_pData = nullptr;
                m_cSize = 0;
                m_cCapacity = 0;
            }
        if constexpr (TAllocatorTraits::propagate_on_container_copy_assignment::value)
            m_Alloc = x.m_Alloc;

        Assign(x.Data(), x.Size());
        return *this;
    }
    CTrivialBuffer& operator=(CTrivialBuffer&& x) noexcept
    {
        if (this == &x)
            return *this;
        if constexpr (TAllocatorTraits::propagate_on_container_move_assignment::value)
        {
            if constexpr (TAllocatorTraits::is_always_equal::value)
                m_Alloc = std::move(x.m_Alloc);
            else if (m_Alloc != x.m_Alloc)
            {
                m_Alloc.deallocate(m_pData, m_cCapacity);
                m_Alloc = std::move(x.m_Alloc);
                m_pData = nullptr;
                m_cSize = 0;
                m_cCapacity = 0;
            }
        }
        else if constexpr (!TAllocatorTraits::is_always_equal::value)
            if (m_Alloc != x.m_Alloc)
            {
                Assign(x.Data(), x.Size());
                return *this;
            }
        std::swap(m_pData, x.m_pData);
        std::swap(m_cSize, x.m_cSize);
        std::swap(m_cCapacity, x.m_cCapacity);
        return *this;
    }

    void Assign(_In_reads_(c) TConstPointer p, size_t c) noexcept
    {
        ReSizeExtra(c);
        memcpy(m_pData, p, c * sizeof(T));
    }

    void Reserve(size_t c) noexcept
    {
        if (m_cCapacity >= c)
            return;
        const auto pOld = m_pData;
        if constexpr (sizeof(T) & 1)
            c += (c & 1);
        m_pData = m_Alloc.allocate(c);
        if (pOld)
        {
            memcpy(m_pData, pOld, m_cSize * sizeof(T));
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
            Reserve(TAllocatorTraits::MakeCapacity(c));
        m_cSize = c;
    }

    EckInlineCe void Clear() noexcept { m_cSize = 0; }

    EckInlineNdCe TPointer Data() noexcept { return m_pData; }
    EckInlineNdCe TConstPointer Data() const noexcept { return m_pData; }

    EckInlineNdCe size_t Size() const noexcept { return m_cSize; }
    EckInlineNdCe size_t ByteSize() const noexcept { return m_cSize * sizeof(T); }

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
        memcpy(Data() + cOld, p, c * sizeof(T));
    }

    EckInline void PushBack(const T& e) noexcept
    {
        const auto cOld = Size();
        ReSizeExtra(Size() + 1);
        Data()[cOld] = e;
    }

    template<class U, size_t N>
    EckInline void PushBack(std::span<U, N> sp) noexcept { PushBack(sp.data(), sp.size()); }

    EckInline auto PushBackSize(size_t c) noexcept
    {
        const auto cOld = Size();
        ReSizeExtra(Size() + c);
        return Data() + cOld;
    }

    EckInline auto& PushBack() noexcept { return *PushBackSize(1); }

    void PushBackMultiple(const T& e, size_t c) noexcept
    {
        const auto p = PushBackSize(c);
        for (size_t i = 0; i < c; ++i)
            p[i] = e;
    }

    template<class... U>
    auto& EmplaceBack(U&&... Args) noexcept
    {
        const auto cOld = Size();
        ReSizeExtra(Size() + 1);
        new (Data() + cOld) T(std::forward<U>(Args)...);
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
            (Size() - pos - c) * sizeof(T));
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
            (cOld - posStart - cReplacing) * sizeof(T));
        memcpy(Data() + posStart, pNew, cNew * sizeof(T));
    }

    TPointer InsertSize(size_t pos, size_t c) noexcept
    {
        EckAssert(pos <= Size());
        const auto cOld = Size();
        ReSizeExtra(Size() + c);
        memmove(
            Data() + pos + c,
            Data() + pos,
            (cOld - pos) * sizeof(T));
        return Data() + pos;
    }

    void Insert(size_t pos, _In_reads_(c) TConstPointer p, size_t c) noexcept
    {
        const auto pNew = InsertSize(pos, c);
        memcpy(pNew, p, c * sizeof(T));
    }

    void Insert(size_t pos, const T& e) noexcept
    {
        EckAssert(pos <= Size());
        const auto cOld = Size();
        ReSizeExtra(Size() + 1);
        memmove(
            Data() + pos + 1,
            Data() + pos,
            (cOld - pos) * sizeof(T));
        Data()[pos] = e;
    }

    EckInline auto& Insert(size_t pos) noexcept { return *InsertSize(pos, 1); }

    EckInline void InsertMultiple(size_t pos, const T& e, size_t c) noexcept
    {
        const auto p = InsertSize(pos, c);
        for (size_t i = 0; i < c; ++i)
            p[i] = e;
    }

    EckInlineNdCe std::span<T> ToSpan() noexcept
    {
        return { Data(), Data() + Size() };
    }
    EckInlineNdCe std::span<const T> ToSpan() const noexcept
    {
        return { Data(), Data() + Size() };
    }

    EckInlineNdCe std::span<T> SubSpan(size_t pos, size_t c) noexcept
    {
        EckAssert(pos + c <= Size());
        return { Data() + pos, Data() + pos + c };
    }
    EckInlineNdCe std::span<const T> SubSpan(size_t pos, size_t c) const noexcept
    {
        EckAssert(pos + c <= Size());
        return { Data() + pos, Data() + pos + c };
    }

    void Attach(const OWNED_RAW_BUFFER& Buffer) noexcept
    {
        EckAssert(!(Buffer.cbCapacity & 1) && Buffer.cbValid <= Buffer.cbCapacity);
        m_Alloc.deallocate(m_pData, m_cCapacity);
        if (Buffer.pData)
        {
            m_pData = (T*)Buffer.pData;
            EckAssert(!(Buffer.cbValid % sizeof(T)) && !(Buffer.cbCapacity % sizeof(T)));
            m_cSize = Buffer.cbValid / sizeof(T);
            m_cCapacity = Buffer.cbCapacity / sizeof(T);
        }
        else
        {
            m_pData = nullptr;
            m_cSize = m_cCapacity = 0;
        }
    }
    void Detach(_Out_ OWNED_RAW_BUFFER& Buffer) noexcept
    {
        Buffer.pData = m_pData;
        Buffer.cbValid = m_cSize * sizeof(T);
        Buffer.cbCapacity = m_cCapacity * sizeof(T);
        EckAssert(!(Buffer.cbCapacity & 1));
        m_pData = nullptr;
        m_cSize = m_cCapacity = 0u;
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
ECK_NAMESPACE_END