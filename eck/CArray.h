/*
* WinEzCtrlKit Library
*
* CArray.h ： 多维数组
* 内存空间连续的多维数组
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "ECK.h"
#include "CAllocator.h"
#include "Utility.h"

#include <vector>

ECK_NAMESPACE_BEGIN

template<class TElem>
class ArrayDim
{
private:
	TElem* e;
	SIZE_T idxCurrDim;
	std::vector<SIZE_T>* DimInfo;

	template<class TElem, class TAllocator>
	friend class CArray;
public:
	operator TElem& ()
	{
		return *e;
	}

	ArrayDim<TElem> operator[](SIZE_T x)
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
		SIZE_T c = 1u;
		for (SIZE_T i = DimInfo->size() - 1; i > idxCurrDim; --i)
		{
			c *= DimInfo->at(i);
		}

		ArrayDim<TElem> d;
		d.e = e + c * x;
		d.idxCurrDim = idxCurrDim + 1;
		d.DimInfo = DimInfo;
		return d;
	}

	const TElem& operator=(const TElem& x)
	{
		*e = x;
		return x;
	}

	TElem* AddressOf()
	{
		return e;
	}
};

template<class TElem, class TAllocator = CAllocator<TElem>>
class CArray
{
private:
	TElem* m_pMem = NULL;
	SIZE_T m_cCount = 0u;
	SIZE_T m_cCapacity = 0u;

	std::vector<SIZE_T> m_Dim{};

	using TAlloc = TAllocator;
	using TAryDim = ArrayDim<TElem>;
public:
	CArray(SIZE_T cDimensions, ...)
	{
		va_list vl;
		va_start(vl, cDimensions);
		ReDim(TRUE, cDimensions, vl);
		va_end(vl);
	}

	CArray(const CArray& x)
	{
		operator=(x);
	}

	CArray(CArray&& x)
	{
		operator=(std::move(x));
	}

	CArray& operator=(const CArray& x)
	{
		m_cCount = x.m_cCount;
		m_cCapacity = TAlloc::MakeCapacity(m_cCount);
		m_pMem = TAlloc::Alloc(m_cCapacity);
		m_Dim = x.m_Dim;
		memcpy(m_pMem, x.m_pMem, m_cCount * sizeof(TElem));
	}

	CArray& operator=(CArray&& x)
	{
		m_cCount = x.m_cCount;
		m_cCapacity = x.m_cCapacity;
		m_pMem = x.m_pMem;
		m_Dim = x.m_Dim;
		x.m_pMem = NULL;
		x.m_cCount = x.m_cCapacity = 0u;
		x.m_Dim.clear();
	}

	~CArray()
	{
		TAlloc::Free(m_pMem);
	}

	TAryDim operator[](SIZE_T x)
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
		SIZE_T c = 1;
		for (SIZE_T i = m_Dim.size() - 1; i > 0; --i)
		{
			c *= m_Dim[i];
		}

		TAryDim d;
		d.e = m_pMem + c * x;
		d.idxCurrDim = 1;
		d.DimInfo = &m_Dim;
		return d;
	}

	void ReDim(BOOL bReservePrevData, SIZE_T cDimensions, va_list vlCounts)
	{
		SIZE_T cTotal = 1;
		SIZE_T c;
		m_Dim.clear();
		EckCounter(cDimensions, i)
		{
			c = va_arg(vlCounts, SIZE_T);
			cTotal *= c;
			m_Dim.push_back(c);
		}

		if (m_cCapacity < cTotal)
		{
			m_cCapacity = TAlloc::MakeCapacity(cTotal);
			if (m_pMem)
				m_pMem = TAlloc::ReAllocZ(m_pMem, m_cCapacity);
			else
				m_pMem = TAlloc::AllocZ(m_cCapacity);
		}

		if (!bReservePrevData)
			ZeroMemory(m_pMem, m_cCount * sizeof(TElem));

		m_cCount = cTotal;
	}

	void ReDim(BOOL bReservePrevData, SIZE_T cDimensions, ...)
	{
		va_list vl;
		va_start(vl, cDimensions);
		ReDim(bReservePrevData, cDimensions, vl);
		va_end(vl);
	}

	EckInline TElem* AddressOf()
	{
		return m_pMem;
	}

	EckInline SIZE_T GetElementsCount()
	{
		return m_cCount;
	}

	EckInline void Reset(BOOL bZeroMemory = TRUE)
	{
		if (bZeroMemory)
			ZeroMemory(m_pMem, m_cCount * sizeof(TElem));
		m_cCount = 0;
		m_Dim.clear();
	}
};

ECK_NAMESPACE_END