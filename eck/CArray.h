#pragma once
#include "CAllocator.h"

ECK_NAMESPACE_BEGIN
template<class TElem>
class ArrayDim
{
private:
    TElem* e;
    size_t idxCurrDim;
    std::vector<size_t>* DimInfo;

    template<class TElem, class TAllocatorator>
    friend class CArray;
public:
    EckInlineNdCe operator TElem& () noexcept { return *e; }
    EckInlineNdCe operator const TElem& () const noexcept { return *e; }

    [[nodiscard]] ArrayDim<TElem> operator[](size_t x) noexcept
    {
#ifdef _DEBUG
        if (idxCurrDim >= DimInfo->size())// 下标个数超出维数范围
            EckDbgBreak();
        if (x > DimInfo->at(idxCurrDim))// 下标超出范围
            EckDbgBreak();
#endif
        size_t c = 1u;
        for (size_t i = DimInfo->size() - 1; i > idxCurrDim; --i)
            c *= DimInfo->at(i);

        ArrayDim<TElem> d;
        d.e = e + c * x;
        d.idxCurrDim = idxCurrDim + 1;
        d.DimInfo = DimInfo;
        return d;
    }

    EckInline const TElem& operator=(const TElem& x)
    {
        *e = x;
        return x;
    }

    EckInlineNdCe TElem* AddrOf() { return e; }
    EckInlineNdCe const TElem* AddrOf() const { return e; }

    EckInlineNdCe TElem& Data() { return *e; }
    EckInlineNdCe const TElem& Data() const { return *e; }
};

template<class TElem, class TAllocatorator = std::allocator<TElem>>
class CArray
{
public:
    using TAllocator = TAllocatorator;
    using TAllocatorTraits = CAllocatorTraits<TAllocator>;
    using TAryDim = ArrayDim<TElem>;

    using TPointer = TElem*;
    using TConstPointer = const TElem*;

    using TIterator = TPointer;
    using TConstIterator = TConstPointer;
    using TReverseIterator = std::reverse_iterator<TIterator>;
    using TConstReverseIterator = std::reverse_iterator<TConstIterator>;
private:
    TElem* m_pMem = nullptr;
    size_t m_cCount = 0u;
    size_t m_cCapacity = 0u;
    std::vector<size_t> m_Dim{};
    TAllocator m_Alloc{};

    constexpr void UnpackSizeArgs(size_t& cTotal) noexcept {}

    template<class TSize0, class... TSize>
    constexpr void UnpackSizeArgs(size_t& cTotal, TSize0 c0, TSize... c) noexcept
    {
        cTotal *= c0;
        m_Dim.emplace_back(c0);
        UnpackSizeArgs(cTotal, c...);
    }
public:
    CArray() = default;

    template<class... TSize>
    CArray(TSize... c) noexcept
    {
        if constexpr (!sizeof...(c))
            throw std::bad_array_new_length{};
        size_t cTotal = 1u;
        UnpackSizeArgs(cTotal, c...);
        m_cCount = cTotal;
        m_cCapacity = m_cCount;
        m_pMem = m_Alloc.allocate(m_cCapacity);
        std::uninitialized_value_construct(begin(), end());
    }

    CArray(const CArray& x) noexcept
        :m_cCount{ x.m_cCount }, m_cCapacity{ x.m_cCapacity }, m_Dim{ x.m_Dim },
        m_Alloc{ TAllocatorTraits::select_on_container_copy_construction(x.m_Alloc) }
    {
        m_pMem = m_Alloc.allocate(m_cCapacity);
        std::uninitialized_copy(x.begin(), x.end(), begin());
    }

    CArray(CArray&& x) noexcept
        :m_pMem{ x.m_pMem }, m_cCount{ x.m_cCount }, m_cCapacity{ x.m_cCapacity },
        m_Dim{ std::move(x.m_Dim) }, m_Alloc{ std::move(x.m_Alloc) }
    {
        x.m_pMem = nullptr;
        x.m_cCount = x.m_cCapacity = 0u;
        x.m_Dim.clear();
    }

    CArray& operator=(const CArray& x) noexcept
    {
        if (this == &x)
            return *this;
        if constexpr (!TAllocatorTraits::is_always_equal::value)
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
            else if constexpr (TAllocatorTraits::propagate_on_container_copy_assignment::value)
                m_Alloc = x.m_Alloc;
        }
        else if constexpr (TAllocatorTraits::propagate_on_container_copy_assignment::value)
            m_Alloc = x.m_Alloc;

        m_Dim = x.m_Dim;
        if (m_cCapacity < x.m_cCount)
        {
            m_Alloc.deallocate(m_pMem, m_cCapacity);
            m_cCount = x.m_cCount;
            m_cCapacity = TAllocatorTraits::MakeCapacity(m_cCount);
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

    CArray& operator=(CArray&& x) noexcept
    {
        if (this == &x)
            return *this;

        if (this == &x)
            return *this;
        if constexpr (TAllocatorTraits::propagate_on_container_move_assignment::value)
            m_Alloc = std::move(x.m_Alloc);
        else if constexpr (!TAllocatorTraits::is_always_equal::value)
            if (m_Alloc != x.m_Alloc)
            {
                m_Alloc.deallocate(m_pMem, m_cCapacity);
                m_Alloc = std::move(x.m_Alloc);
                m_cCount = x.m_cCount;
                m_cCapacity = TAllocatorTraits::MakeCapacity(m_cCount);
                m_pMem = m_Alloc.allocate(m_cCapacity);
                std::uninitialized_copy(x.begin(), x.end(), begin());
                return *this;
            }
        std::swap(m_pMem, x.m_pMem);
        std::swap(m_cCount, x.m_cCount);
        std::swap(m_cCapacity, x.m_cCapacity);
        std::swap(m_Dim, x.m_Dim);
        return *this;
    }

    ~CArray()
    {
        std::destroy(begin(), end());
        m_Alloc.deallocate(m_pMem, m_cCapacity);
    }

    [[nodiscard]] TAryDim operator[](size_t x) noexcept
    {
#ifdef _DEBUG
        if (!m_Dim.size())// 下标个数超出维数范围
            EckDbgBreak();
        if (x > m_Dim[0])// 下标超出范围
            EckDbgBreak();
#endif
        size_t c = 1;
        for (size_t i = m_Dim.size() - 1; i > 0; --i)
            c *= m_Dim[i];

        TAryDim d;
        d.e = m_pMem + c * x;
        d.idxCurrDim = 1;
        d.DimInfo = &m_Dim;
        return d;
    }

    template<class... TSize>
    CArray& ReDim(TSize... c) noexcept
    {
        if constexpr (!sizeof...(c))
            throw std::bad_array_new_length{};

        size_t cTotal = 1u;
        m_Dim.clear();
        UnpackSizeArgs(cTotal, c...);

        if (m_cCapacity < cTotal)
        {
            CArray t(c...);
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

    EckInlineNdCe size_t Size() const noexcept { return m_cCount; }
    EckInlineNdCe size_t Size(size_t idxDim) const noexcept { return m_Dim[idxDim]; }

    EckInlineCe void Clear() noexcept
    {
        std::destroy(begin(), end());
        m_cCount = 0;
        m_Dim.clear();
    }

    EckInlineNdCe TElem* Data() noexcept { return m_pMem; }
    EckInlineNdCe const TElem* Data() const noexcept { return m_pMem; }

    EckInlineNdCe TIterator begin() noexcept { return Data(); }
    EckInlineNdCe TIterator end() noexcept { return begin() + Size(); }
    EckInlineNdCe TConstIterator begin() const noexcept { return Data(); }
    EckInlineNdCe TConstIterator end() const noexcept { return begin() + Size(); }
    EckInlineNdCe TConstIterator cbegin() const noexcept { begin(); }
    EckInlineNdCe TConstIterator cend() const noexcept { end(); }
    EckInlineNdCe TReverseIterator rbegin() noexcept { return TReverseIterator(end()); }
    EckInlineNdCe TReverseIterator rend() noexcept { return TReverseIterator(begin()); }
    EckInlineNdCe TConstReverseIterator rbegin() const noexcept { return TConstReverseIterator(end()); }
    EckInlineNdCe TConstReverseIterator rend() const noexcept { return TConstReverseIterator(begin()); }
    EckInlineNdCe TConstReverseIterator crbegin() const noexcept { return rbegin(); }
    EckInlineNdCe TConstReverseIterator crend() const noexcept { return rend(); }
};
ECK_NAMESPACE_END