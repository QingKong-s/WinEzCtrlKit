/*
* WinEzCtrlKit Library
*
* CAllocator.h ： 内存分配器
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "ECK.h"

#include <iostream>

ECK_NAMESPACE_BEGIN

template<class T, class TSize = size_t>
struct CAllocatorHeap
{
private:
	HANDLE m_hHeap = GetProcessHeap();
	DWORD m_dwSerialize = 0;

	template<class T1, class TSize1, class T2, class TSize2>
	friend bool operator==(const CAllocatorHeap<T1, TSize1>& a1, const CAllocatorHeap<T2, TSize2>& a2) noexcept;
public:
	using value_type = T;
	using size_type = TSize;
	using difference_type = std::make_signed_t<size_type>;

	[[nodiscard]] EckInline T* allocate(size_type c)
	{
		auto p = (T*)HeapAlloc(m_hHeap, m_dwSerialize, c * sizeof(value_type));
		if (p)
			return p;
		else
			throw std::bad_alloc{};
	}

	EckInline void deallocate(T* p, size_type c)
	{
		EckAssert(HeapSize(m_hHeap, m_dwSerialize, p) / sizeof(value_type) == c);
		HeapFree(m_hHeap, m_dwSerialize, p);
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
/// 适用于分配大内存，可以对分配的内存调用VirtualProtect
/// </summary>
template<class T, class TSize = size_t>
struct CAllocatorVA
{
	using value_type = T;
	using size_type = TSize;
	using difference_type = std::make_signed_t<size_type>;

	static TSize s_cbPage;

	[[nodiscard]] EckInline T* allocate(size_type c)
	{
		auto p = (T*)VirtualAlloc(NULL, c * sizeof(value_type), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		if (p)
			return p;
		else
			throw std::bad_alloc{};
	}

	EckInline void deallocate(T* p, size_type c)
	{
		VirtualFree(p, 0, MEM_RELEASE);
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
	EckInline static TBase::size_type MakeCapacity(TBase::size_type c)
	{
		if constexpr (HasMakeCapacity<TBase::allocator_type, TBase::size_type>::value)
			return TBase::allocator_type::MakeCapacity(c);
		else
			return c + c / 2;
	}
};
ECK_NAMESPACE_END