/*
* WinEzCtrlKit Library
*
* CArray2D.h ： 二维数组
* 内存空间连续的二维数组
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "CAllocator.h"

ECK_NAMESPACE_BEGIN
template<class TElem>
class Array2DProxy
{
private:
	TElem* e;

	template<class TElem, class TAllocator>
	friend class CArray2D;

	Array2DProxy(TElem* e) :e{ e } {}
public:
	TElem& operator[](size_t idx)
	{
		return e[idx];
	}

	EckInline TElem* AddrOf() { return e; }

	EckInline const TElem* AddrOf() const { return e; }
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
	ECKNOUNIQUEADDR TAlloc m_Alloc{};
public:
	CArray2D() = default;

	CArray2D(size_t c0, size_t c1) :m_c0{ c0 }, m_c1{ c1 },
		m_cCount{ c0 * c1 }, m_cCapacity{ c0 * c1 }
	{
		m_pMem = m_Alloc.allocate(m_cCapacity);
		std::uninitialized_value_construct(begin(), end());
	}

	CArray2D(const CArray2D& x)
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

	CArray2D& operator=(const CArray2D& x)
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

	TProxy operator[](size_t x)
	{
		return TProxy{ m_pMem + x * m_c1 };
	}

	CArray2D& ReDim(size_t c0, size_t c1)
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

	EckInline constexpr size_t Size() const noexcept
	{
		return m_cCount;
	}

	EckInline constexpr size_t Size(size_t idxDim) const noexcept
	{
		EckAssert(idxDim == 0 || idxDim == 1 && "数组维度超出范围");
		return idxDim == 0 ? m_c0 : m_c1;
	}

	EckInline constexpr void Clear() noexcept
	{
		std::destroy(begin(), end());
		m_cCount = 0u;
		m_c0 = m_c1 = 0u;
	}

	EckInline constexpr TElem* Data() noexcept { return m_pMem; }

	EckInline constexpr const TElem* Data() const noexcept { return m_pMem; }

	EckInline constexpr TIterator begin() { return Data(); }
	EckInline constexpr TIterator end() { return begin() + Size(); }
	EckInline constexpr TConstIterator begin() const { return Data(); }
	EckInline constexpr TConstIterator end() const { return begin() + Size(); }
	EckInline constexpr TConstIterator cbegin() const { begin(); }
	EckInline constexpr TConstIterator cend() const { end(); }
	EckInline constexpr TReverseIterator rbegin() { return TReverseIterator(begin()); }
	EckInline constexpr TReverseIterator rend() { return TReverseIterator(end()); }
	EckInline constexpr TConstReverseIterator rbegin() const { return TConstReverseIterator(begin()); }
	EckInline constexpr TConstReverseIterator rend() const { return TConstReverseIterator(end()); }
	EckInline constexpr TConstReverseIterator crbegin() const { return rbegin(); }
	EckInline constexpr TConstReverseIterator crend() const { return rend(); }
};
ECK_NAMESPACE_END