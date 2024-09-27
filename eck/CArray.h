/*
* WinEzCtrlKit Library
*
* CArray.h ： 多维数组
* 内存空间连续的多维数组
*
* Copyright(C) 2023-2024 QingKong
*/
#pragma once
#include "CAllocator.h"

#include <vector>

ECK_NAMESPACE_BEGIN

template<class TElem>
class ArrayDim
{
private:
	TElem* e;
	size_t idxCurrDim;
	std::vector<size_t>* DimInfo;

	template<class TElem, class TAllocator>
	friend class CArray;
public:
	EckInline operator TElem& () { return *e; }

	EckInline operator const TElem& () const { return *e; }

	ArrayDim<TElem> operator[](size_t x)
	{
#ifdef _DEBUG
		if (idxCurrDim >= DimInfo->size())
		{
			EckDbgPrintWithPos(L"下标个数超出维数范围");
			EckDbgBreak();
		}

		if (x > DimInfo->at(idxCurrDim))
		{
			EckDbgPrintWithPos(L"下标超出范围");
			EckDbgBreak();
		}
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

	EckInline TElem* AddrOf() { return e; }

	EckInline const TElem* AddrOf() const { return e; }

	EckInline TElem& Data() { return *e; }

	EckInline const TElem& Data() const { return *e; }
};

template<class TElem, class TAllocator = std::allocator<TElem>>
class CArray
{
public:
	using TAlloc = TAllocator;
	using TAllocTraits = CAllocatorTraits<TAlloc>;
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
	TAlloc m_Alloc{};

	constexpr void UnpackSizeArgs(size_t& cTotal) {}

	template<class TSize0, class... TSize>
	constexpr void UnpackSizeArgs(size_t& cTotal, TSize0 c0, TSize... c)
	{
		cTotal *= c0;
		m_Dim.emplace_back(c0);
		UnpackSizeArgs(cTotal, c...);
	}
public:
	CArray() = default;

	template<class... TSize>
	CArray(TSize... c)
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

	CArray(const CArray& x)
		:m_cCount{ x.m_cCount }, m_cCapacity{ x.m_cCapacity }, m_Dim{ x.m_Dim },
		m_Alloc{ TAllocTraits::select_on_container_copy_construction(x.m_Alloc) }
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

	CArray& operator=(const CArray& x)
	{
		if (this == &x)
			return *this;
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

		m_Dim = x.m_Dim;
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

	CArray& operator=(CArray&& x) noexcept
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
		std::swap(m_Dim, x.m_Dim);
		return *this;
	}

	~CArray()
	{
		std::destroy(begin(), end());
		m_Alloc.deallocate(m_pMem, m_cCapacity);
	}

	TAryDim operator[](size_t x)
	{
#ifdef _DEBUG
		if (!m_Dim.size())
		{
			EckDbgPrintWithPos(L"下标个数超出维数范围");
			EckDbgBreak();
		}

		if (x > m_Dim[0])
		{
			EckDbgPrintWithPos(L"下标超出范围");
			EckDbgBreak();
		}
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
	CArray& ReDim(TSize... c)
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

	EckInline constexpr size_t Size() const noexcept { return m_cCount; }

	EckInline constexpr size_t Size(size_t idxDim) const noexcept { return m_Dim[idxDim]; }

	EckInline constexpr void Clear() noexcept
	{
		std::destroy(begin(), end());
		m_cCount = 0;
		m_Dim.clear();
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