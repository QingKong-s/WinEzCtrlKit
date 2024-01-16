#pragma once
#include "CAllocator.h"
#include "Utility.h"

ECK_NAMESPACE_BEGIN

template<class T_, class TAlloc_ = CAllocatorVA<BYTE>>
class CFixedBlockCollection
{
public:
	using T = T_;
	using TAlloc = TAlloc_;
	using TSize = TAlloc::size_type;
	using TPointer = T*;
private:
	struct PAGE
	{
		TPointer p;
		TSize cbSize;
		TSize cAlloc;
	};

	std::vector<PAGE> m_vPage{};
	TSize m_cbPage = CAllocatorVA<T>::s_cbPage;
	TSize m_cbCurrFree = 0;
	TSize m_cbCurr = 0;

	ECKNOUNIQUEADDR TAlloc m_Alloc{};
public:
	template<class... TParam>
	TPointer Alloc(int cBlock, TParam&&... Args)
	{
		const TSize cb = cBlock * sizeof(T);
		if (m_cbCurrFree < cb)
		{
			m_cbCurrFree = m_cbCurr = AlignMemSize(cb, m_cbPage);
			m_vPage.emplace_back((TPointer)m_Alloc.allocate(m_cbCurr), m_cbCurr);
		}
		auto p = (BYTE*)m_vPage.back().p;
		++m_vPage.back().cAlloc;
		const auto pNew = (TPointer)(p + (m_cbCurr - m_cbCurrFree));
		for (auto p0 = pNew; p0 < pNew + cBlock; ++p0)
			std::construct_at(p0, Args...);
		m_cbCurrFree -= cb;
		return pNew;
	}

	void FreeAll()
	{
		for (auto e : m_vPage)
		{
			std::destroy(e.p, e.p + e.cAlloc);
			m_Alloc.deallocate((BYTE*)e.p, e.cbSize);
		}
		m_vPage.clear();
		m_cbCurrFree = m_cbCurr = 0;
	}
};

ECK_NAMESPACE_END