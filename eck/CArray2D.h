#pragma once
#include "CAllocator.h"

ECK_NAMESPACE_BEGIN
template<class TElem>
class Array2DProxy
{
    template<class TElem, class TAllocator>
    friend class CArray2D;
private:
    TElem* e;

    constexpr Array2DProxy(TElem* e) noexcept : e{ e } {}
public:
    EckInlineNdCe TElem& operator[](size_t idx) noexcept { return e[idx]; }
    EckInlineNdCe TElem* AddrOf() noexcept { return e; }
    EckInlineNdCe const TElem* AddrOf() const noexcept { return e; }
};


template<class TElem, class TAllocator = std::allocator<TElem>>
class CArray2D
{
public:
    using TAlloc = TAllocator;
    using TAllocTraits = CAllocatorTraits<TAlloc>;
    using TProxy = Array2DProxy<TElem>;

    using TPointer = TElem*;
    using TConstPointer = const TElem*;

    using TIterator = TPointer;
    using TConstIterator = TConstPointer;
    using TReverseIterator = std::reverse_iterator<TIterator>;
    using TConstReverseIterator = std::reverse_iterator<TConstIterator>;
private:
    TElem* m_pMem{};
    size_t m_cCount{};
    size_t m_cCapacity{};
    size_t m_c0{};
    size_t m_c1{};
    [[no_unique_address]] TAlloc m_Alloc{};
public:
    CArray2D() = default;

    CArray2D(size_t c0, size_t c1) noexcept :m_c0{ c0 }, m_c1{ c1 },
        m_cCount{ c0 * c1 }, m_cCapacity{ c0 * c1 }
    {
        m_pMem = m_Alloc.allocate(m_cCapacity);
        std::uninitialized_value_construct(begin(), end());
    }

    CArray2D(const CArray2D& x) noexcept
        :m_cCount{ x.m_cCount }, m_cCapacity{ x.m_cCapacity }, m_c0{ x.m_c0 }, m_c1{ x.m_c1 },
        m_Alloc{ TAllocTraits::select_on_container_copy_construction(x.m_Alloc) }
    {
        m_pMem = m_Alloc.allocate(m_cCapacity);
        std::uninitialized_copy(x.begin(), x.end(), begin());
    }

    CArray2D(CArray2D&& x) noexcept
        :m_pMem{ x.m_pMem }, m_cCount{ x.m_cCount }, m_cCapacity{ x.m_cCapacity },
        m_c0{ x.m_c0 }, m_c1{ x.m_c1 }, m_Alloc{ std::move(x.m_Alloc) }
    {
        x.m_pMem = nullptr;
        x.m_cCount = x.m_cCapacity = 0u;
    }

    CArray2D& operator=(const CArray2D& x) noexcept
    {
        if (this == &x)
            return *this;
        m_c0 = x.m_c0;
        m_c1 = x.m_c1;
        if constexpr (!TAllocTraits::is_always_equal::value)
        {
            if (m_Alloc != x.m_Alloc)
            {
                m_Alloc.deallocate(m_pMem, m_cCapacity);
                m_Alloc = x.m_Alloc;
                m_cCount = x.m_cCount;
                m_cCapacity = x.m_cCapacity;
                m_pMem = m_Alloc.allocate(m_cCapacity);
                std::uninitialized_copy(x.begin(), x.end(), begin());
                return *this;
            }
            else if constexpr (TAllocTraits::propagate_on_container_copy_assignment::value)
                m_Alloc = x.m_Alloc;
        }
        else if constexpr (TAllocTraits::propagate_on_container_copy_assignment::value)
            m_Alloc = x.m_Alloc;

        if (m_cCapacity < x.m_cCount)
        {
            m_Alloc.deallocate(m_pMem, m_cCapacity);
            m_cCount = x.m_cCount;
            m_cCapacity = TAllocTraits::MakeCapacity(m_cCount);
            m_pMem = m_Alloc.allocate(m_cCapacity);
            std::uninitialized_copy(x.begin(), x.end(), begin());
            return *this;
        }

        if (m_cCount > x.m_cCount)
        {
            std::destroy(begin() + x.m_cCount, end());
            m_cCount = x.m_cCount;
            std::copy(x.begin(), x.end(), begin());
        }
        else if (m_cCount < x.m_cCount)
        {
            std::copy(x.begin(), x.begin() + m_cCount, begin());
            std::uninitialized_copy(x.begin() + m_cCount, x.end(), begin() + m_cCount);
            m_cCount = x.m_cCount;
        }
        return *this;
    }

    CArray2D& operator=(CArray2D&& x) noexcept
    {
        if (this == &x)
            return *this;

        if (this == &x)
            return *this;
        if constexpr (TAllocTraits::propagate_on_container_move_assignment::value)
            m_Alloc = std::move(x.m_Alloc);
        else if constexpr (!TAllocTraits::is_always_equal::value)
            if (m_Alloc != x.m_Alloc)
            {
                m_Alloc.deallocate(m_pMem, m_cCapacity);
                m_Alloc = std::move(x.m_Alloc);
                m_cCount = x.m_cCount;
                m_cCapacity = TAllocTraits::MakeCapacity(m_cCount);
                m_pMem = m_Alloc.allocate(m_cCapacity);
                std::uninitialized_copy(x.begin(), x.end(), begin());
                return *this;
            }
        std::swap(m_pMem, x.m_pMem);
        std::swap(m_cCount, x.m_cCount);
        std::swap(m_cCapacity, x.m_cCapacity);
        std::swap(m_c0, x.m_c0);
        std::swap(m_c1, x.m_c1);
        return *this;
    }

    ~CArray2D()
    {
        std::destroy(begin(), end());
        m_Alloc.deallocate(m_pMem, m_cCapacity);
    }

    TProxy operator[](size_t x) noexcept
    {
        return TProxy{ m_pMem + x * m_c1 };
    }

    CArray2D& ReDim(size_t c0, size_t c1) noexcept
    {
        if (m_c0 == c0 && m_c1 == c1)
            return *this;
        m_c0 = c0;
        m_c1 = c1;
        size_t cTotal = c0 * c1;

        if (m_cCapacity < cTotal)
        {
            CArray2D t(c0, c1);
            std::swap(*this, t);
            return *this;
        }

        if (m_cCount > cTotal)
            std::destroy(begin() + cTotal, end());
        else
            std::uninitialized_value_construct(begin() + m_cCount, begin() + cTotal);
        m_cCount = cTotal;
        return *this;
    }

    EckInlineNdCe size_t Size() const noexcept
    {
        return m_cCount;
    }
    EckInlineNdCe size_t Size(size_t idxDim) const noexcept
    {
        EckAssert(idxDim == 0 || idxDim == 1);// 数组维度超出范围
        return idxDim == 0 ? m_c0 : m_c1;
    }

    EckInlineCe void Clear() noexcept
    {
        std::destroy(begin(), end());
        m_cCount = 0u;
        m_c0 = m_c1 = 0u;
    }

    EckInlineNdCe TElem* Data() noexcept { return m_pMem; }
    EckInlineNdCe const TElem* Data() const noexcept { return m_pMem; }

    EckInlineNdCe TIterator begin() noexcept { return Data(); }
    EckInlineNdCe TIterator end() noexcept { return begin() + Size(); }
    EckInlineNdCe TConstIterator begin() const noexcept { return Data(); }
    EckInlineNdCe TConstIterator end() const noexcept { return begin() + Size(); }
    EckInlineNdCe TConstIterator cbegin() const noexcept { begin(); }
    EckInlineNdCe TConstIterator cend() const noexcept { end(); }
    EckInlineNdCe TReverseIterator rbegin() noexcept { return TReverseIterator(begin()); }
    EckInlineNdCe TReverseIterator rend() noexcept { return TReverseIterator(end()); }
    EckInlineNdCe TConstReverseIterator rbegin() const noexcept { return TConstReverseIterator(begin()); }
    EckInlineNdCe TConstReverseIterator rend() const noexcept { return TConstReverseIterator(end()); }
    EckInlineNdCe TConstReverseIterator crbegin() const noexcept { return rbegin(); }
    EckInlineNdCe TConstReverseIterator crend() const noexcept { return rend(); }
};
ECK_NAMESPACE_END