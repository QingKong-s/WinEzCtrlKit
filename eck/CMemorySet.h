/*
* WinEzCtrlKit Library
*
* CMemorySet.h ： 内存集
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "CSrwLock.h"

ECK_NAMESPACE_BEGIN
class CMemorySet
{
	template<class T>
	friend class CFixedMemorySet;
private:
	struct BLOCK
	{
		BLOCK* pNext;
		BYTE* pFreeBegin;
		SIZE_T cbBlock;
	};

	BLOCK* m_pHead{};
	SIZE_T m_cbPage{ 4096 };
	CSrwLock m_Lk{};
public:
	[[nodiscard]] void* Allocate(SIZE_T cbSize,
		SIZE_T cbAlign = sizeof(void*), BOOL bOnlySearchTop = TRUE)
	{
		if (cbSize == 0)
			return nullptr;
		CSrwWriteGuard _{ m_Lk };
		auto p{ m_pHead };
		while (p)
		{
			const auto pNew = StepToNextAlignBoundary((BYTE*)p, p->pFreeBegin, cbAlign);
			if (pNew + cbSize <= (BYTE*)p + p->cbBlock)
			{
				p->pFreeBegin = pNew + cbSize;
				return pNew;
			}
			if (bOnlySearchTop)
				break;
			p = p->pNext;
		}

		const SIZE_T cbBlock = AlignMemSize(
			std::max(m_cbPage, cbSize + sizeof(BLOCK) + cbAlign), 4096);
		const auto pBlock = (BLOCK*)VAlloc(cbBlock);
		if (!pBlock)
			return nullptr;
		pBlock->pNext = m_pHead;
		pBlock->pFreeBegin = StepToNextAlignBoundary((BYTE*)pBlock,
			(BYTE*)pBlock + sizeof(BLOCK), cbAlign) + cbSize;
		pBlock->cbBlock = cbBlock;
		m_pHead = pBlock;
		return pBlock->pFreeBegin - cbSize;
	}

	void Clear()
	{
		CSrwWriteGuard _{ m_Lk };
		auto p{ m_pHead };
		while (p)
		{
			auto pNext{ p->pNext };
			VFree(p);
			p = pNext;
		}
		m_pHead = nullptr;
	}

	void ClearRecord()
	{
		CSrwWriteGuard _{ m_Lk };
		auto p{ m_pHead };
		while (p)
		{
			p->pFreeBegin = (BYTE*)p + sizeof(BLOCK);
			p = p->pNext;
		}
	}

	EckInline void SetPageSize(SIZE_T cbPage)
	{
		CSrwWriteGuard _{ m_Lk };
		m_cbPage = cbPage;
	}

	EckInline SIZE_T GetPageSize()
	{
		CSrwReadGuard _{ m_Lk };
		return m_cbPage;
	}
};

template<class T_>
class CFixedMemorySet
{
public:
	using T = T_;
	using TPointer = T*;
private:
	struct PAGE
	{
		TPointer p;
		SIZE_T cbSize;
		SIZE_T cAlloc;
	};

	CMemorySet m_MemSet{};
public:
	template<class... TParam>
	EckInline TPointer New(TParam&&... Args)
	{
		const auto p = m_MemSet.Allocate(sizeof(T), alignof(T));
		if (!p)
			return nullptr;
		return std::construct_at((T*)p, std::forward<TParam>(Args)...);
	}

	template<class... TParam>
	EckInline TPointer NewN(size_t cBlock, TParam&&... Args)
	{
		const auto p = m_MemSet.Allocate(cBlock * sizeof(T), alignof(T));
		if (!p)
			return nullptr;
		for (auto p0 = (TPointer)p; p0 < (TPointer)p + cBlock; ++p0)
			std::construct_at(p0, Args...);
		return p;
	}

	void Delete()
	{
		auto pBlock{ m_MemSet.m_pHead };
		while (pBlock)
		{
			BYTE* p = StepToNextAlignBoundary((BYTE*)pBlock,
				(BYTE*)pBlock + sizeof(CMemorySet::BLOCK), alignof(T));
			while (p + sizeof(T) <= pBlock->pFreeBegin)
			{
				std::destroy_at((T*)p);
				p += sizeof(T);
				p = StepToNextAlignBoundary((BYTE*)pBlock, p, alignof(T));
			}
			pBlock = pBlock->pNext;
		}
		m_MemSet.ClearRecord();
	}

	EckInline constexpr auto& GetMemorySet() const { return m_MemSet; }
};
ECK_NAMESPACE_END