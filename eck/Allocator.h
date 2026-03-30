#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
template<class TAllocator, class TSize>
concept CcpHasMakeCapacity = requires(const TSize & c)
{
    { TAllocator::MakeCapacity(c) } -> std::convertible_to<TSize>;
};

template<class TAllocator>
struct CAllocatorTraits : std::allocator_traits<TAllocator>
{
private:
    using TBase = std::allocator_traits<TAllocator>;
public:
    EckInline static typename TBase::size_type MakeCapacity(typename TBase::size_type c)
    {
        if constexpr (CcpHasMakeCapacity<TBase::allocator_type, TBase::size_type>)
            return TBase::allocator_type::MakeCapacity(c);
        else
            return c + c / 2;
    }
};

template<class T, class TSize = size_t>
struct CDefaultAllocator : public std::allocator<T>
{
    using size_type = TSize;

    constexpr CDefaultAllocator() noexcept = default;
    constexpr CDefaultAllocator(const CDefaultAllocator&) noexcept = default;
    template <class U>
    constexpr CDefaultAllocator(const CDefaultAllocator<U>&) noexcept {}
    constexpr ~CDefaultAllocator() = default;
    constexpr CDefaultAllocator& operator=(const CDefaultAllocator&) = default;

    EckInline constexpr void deallocate(T* const p, const TSize c)
    {
        __super::deallocate(p, (size_t)c);
    }

    EckInlineNdCe __declspec(allocator) T* allocate(const TSize c)
    {
        return __super::allocate((size_t)c);
    }
};

template<class T1, class TSize1, class T2, class TSize2>
EckInlineNdCe bool operator==(
    const CDefaultAllocator<T1, TSize1>& a1,
    const CDefaultAllocator<T2, TSize2>& a2) noexcept
{
    return true;
}
ECK_NAMESPACE_END