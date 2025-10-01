#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
template<class T, class TSize = size_t>
struct CAllocatorHeap
{
private:
    HANDLE m_hHeap = RtlProcessHeap();
    DWORD m_dwSerialize = 0;

    template<class T1, class TSize1, class T2, class TSize2>
    friend bool operator==(const CAllocatorHeap<T1, TSize1>& a1, const CAllocatorHeap<T2, TSize2>& a2) noexcept;
public:
    using value_type = T;
    using size_type = TSize;
    using difference_type = std::make_signed_t<size_type>;

    [[nodiscard]] EckInline T* allocate(size_type c)
    {
        auto p = (T*)RtlAllocateHeap(m_hHeap, m_dwSerialize, c * sizeof(value_type));
        if (p)
            return p;
        else
            throw std::bad_alloc{};
    }

    EckInline void deallocate(T* p, size_type c)
    {
        EckAssert(RtlSizeHeap(m_hHeap, m_dwSerialize, p) / sizeof(value_type) == c);
        RtlFreeHeap(m_hHeap, m_dwSerialize, p);
    }

    EckInline void SetHeapSerialize(BOOL b) { m_dwSerialize = (b ? 0 : HEAP_NO_SERIALIZE); }

    EckInline void SetHHeap(HANDLE hHeap) { m_hHeap = hHeap; }

    EckInline HANDLE GetHHeap() const { return m_hHeap; }
};

template<class T1, class TSize1, class T2, class TSize2>
EckInline bool operator==(const CAllocatorHeap<T1, TSize1>& a1, const CAllocatorHeap<T2, TSize2>& a2) noexcept
{
    return a1.m_hHeap == a1.m_hHeap;
}



template<class T, class TSize = size_t>
struct CAllocatorProcHeap
{
    template<class T1, class TSize1, class T2, class TSize2>
    friend constexpr bool operator==(const CAllocatorProcHeap<T1, TSize1>& a1, const CAllocatorProcHeap<T2, TSize2>& a2) noexcept;

    using value_type = T;
    using size_type = TSize;
    using difference_type = std::make_signed_t<size_type>;

    [[nodiscard]] EckInline T* allocate(size_type c)
    {
        auto p = (T*)HeapAlloc(GetProcessHeap(), 0, c * sizeof(value_type));
        if (p)
            return p;
        else
            throw std::bad_alloc{};
    }

    EckInline void deallocate(T* p, size_type c)
    {
        EckAssert(p ? (HeapSize(GetProcessHeap(), 0, p) / sizeof(value_type) == c) : TRUE);
        HeapFree(GetProcessHeap(), 0, p);
    }
};

template<class T1, class TSize1, class T2, class TSize2>
EckInline constexpr bool operator==(const CAllocatorProcHeap<T1, TSize1>& a1, const CAllocatorProcHeap<T2, TSize2>& a2) noexcept
{
    return true;
}



/// <summary>
/// VirtualAlloc分配器
/// 可以对分配的内存调用VirtualProtect
/// </summary>
template<class T, class TSize = size_t>
struct CAllocatorVA
{
    using value_type = T;
    using size_type = TSize;
    using difference_type = std::make_signed_t<size_type>;

    static TSize s_cbPage;

    constexpr CAllocatorVA() noexcept {}

    constexpr CAllocatorVA(const CAllocatorVA&) noexcept = default;

    template <class U>
    constexpr CAllocatorVA(const CAllocatorVA<U>&) noexcept {}

    constexpr ~CAllocatorVA() = default;

    constexpr CAllocatorVA& operator=(const CAllocatorVA&) = default;

    [[nodiscard]] EckInline T* allocate(size_type c)
    {
        auto p = (T*)VAlloc(c * sizeof(value_type));
        if (p)
            return p;
        else
            throw std::bad_alloc{};
    }

    EckInline void deallocate(T* p, size_type c)
    {
        VFree(p);
    }

    EckInline static TSize MakeCapacity(TSize c)
    {
        return AlignMemSize(c * sizeof(value_type), s_cbPage) / sizeof(value_type);
    }
private:
    EckInline constexpr static TSize AlignMemSize(TSize cbSize, TSize cbAlign)
    {
        if (cbSize / cbAlign * cbAlign == cbSize)
            return cbSize;
        else [[likely]]
            return (cbSize / cbAlign + 1) * cbAlign;
    }
};

template<class T, class TSize>
inline TSize CAllocatorVA<T, TSize>::s_cbPage = []()
    {
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        return (TSize)si.dwPageSize;
    }
();

template<class T1, class TSize1, class T2, class TSize2>
EckInline constexpr bool operator==(const CAllocatorVA<T1, TSize1>& a1, const CAllocatorVA<T2, TSize2>& a2) noexcept
{
    return true;
}


template<class TAlloc, class TSize, class = void>
struct HasMakeCapacity : std::false_type {};

template<class TAlloc, class TSize>
struct HasMakeCapacity<TAlloc, TSize,
    std::void_t<decltype(TAlloc::MakeCapacity(std::declval<const TSize&>()))>> : std::true_type {};

template<class TAlloc>
struct CAllocatorTraits : std::allocator_traits<TAlloc>
{
private:
    using TBase = std::allocator_traits<TAlloc>;
public:
    EckInline static typename TBase::size_type MakeCapacity(typename TBase::size_type c)
    {
        if constexpr (HasMakeCapacity<TBase::allocator_type, TBase::size_type>::value)
            return TBase::allocator_type::MakeCapacity(c);
        else
            return c + c / 2;
    }
};

template<class T,class TSize = size_t>
struct CDefAllocator :public std::allocator<T>
{
    using size_type = TSize;

    constexpr CDefAllocator() noexcept = default;
    constexpr CDefAllocator(const CDefAllocator&) noexcept = default;
    template <class U>
    constexpr CDefAllocator(const CDefAllocator<U>&) noexcept {}
    constexpr ~CDefAllocator() = default;
    constexpr CDefAllocator& operator=(const CDefAllocator&) = default;

    EckInline constexpr void deallocate(T* const p, const TSize c)
    {
        __super::deallocate(p, (size_t)c);
    }

    EckInline [[nodiscard]] constexpr __declspec(allocator) T* allocate(const TSize c)
    {
        return __super::allocate((size_t)c);
    }
};

template<class T1, class TSize1, class T2, class TSize2>
EckInline constexpr bool operator==(const CDefAllocator<T1, TSize1>& a1, 
    const CDefAllocator<T2, TSize2>& a2) noexcept
{
    return true;
}
ECK_NAMESPACE_END