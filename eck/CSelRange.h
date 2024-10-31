/*
* WinEzCtrlKit Library
*
* CSelRange.h ： 选择区间
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
// 提供一种ILVRange的实现。
// 可能有控件需要使用此基础设施，因此允许作为自动变量使用，
// 但此种情况下绝对不能调用COM生命周期方法。
class CSelRange :public ILVRange
{
private:
public:
	struct RANGE// 闭区间
	{
		LONG idxBegin;
		LONG idxEnd;
	};

	ULONG m_cRef{ 1 };
	std::vector<RANGE> m_vRange{};// 从小到大排列

	/// <summary>
	/// 查找所在区间
	/// </summary>
	/// <param name="idxItem">项目索引</param>
	/// <param name="bFound">是否在返回的区间内</param>
	/// <returns>搜寻结束时的迭代器，若未找到，此迭代器指向第一个大于idxItem的区间</returns>
	auto FindRange(LONG idxItem, _Out_ BOOL& bFound)
	{
		const auto it = std::lower_bound(m_vRange.begin(), m_vRange.end(), idxItem,
			[](RANGE r, LONG idx) { return r.idxEnd < idx; });
		bFound = (it != m_vRange.end() && idxItem >= it->idxBegin);
		return it;
	}
public:
	ULONG STDMETHODCALLTYPE AddRef() { return ++m_cRef; }

	ULONG STDMETHODCALLTYPE Release()
	{
		if (m_cRef == 1)
		{
			delete this;
			return 0;
		}
		return --m_cRef;
	}

	STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject)
	{
		static const QITAB qit[]
		{
			QITABENT(CSelRange, ILVRange),
			{ 0 },
		};
		return QISearch(this, qit, riid, ppvObject);
	}

	/// <summary>
	/// 并
	/// </summary>
	/// <param name="idxBegin">起始索引（含）</param>
	/// <param name="idxEnd">结束索引（含）</param>
	/// <returns>HRESULT</returns>
	STDMETHODIMP IncludeRange(LONG idxBegin, LONG idxEnd)
	{
		if (idxEnd < idxBegin || idxBegin < 0)
			return E_INVALIDARG;
		BOOL bFound0, bFound1;
		const auto it0 = FindRange(idxBegin, bFound0);
		if (it0 == m_vRange.end())// 左边界大于所有区间
		{
			m_vRange.emplace_back(idxBegin, idxEnd);
			return S_OK;
		}
		const auto it1 = FindRange(idxEnd, bFound1);
		if (it1 == m_vRange.begin() && !bFound1)// 右边界小于所有区间
		{
			m_vRange.emplace(it1, idxBegin, idxEnd);
			return S_OK;
		}

		if (!bFound0)// 需要修改左界
			it0->idxBegin = idxBegin;
		if (bFound1)
			it0->idxEnd = it1->idxEnd;
		else
			it0->idxEnd = std::max(it0->idxEnd, idxEnd);
		const auto itEraseBegin = it0 + 1;// 无论如何，留下左边界处的一个区间
		const auto itEraseEnd = it1 + (bFound1 ? 1 : 0);
		if (itEraseEnd > itEraseBegin)
			m_vRange.erase(itEraseBegin, itEraseEnd);
		return S_OK;
	}

	/// <summary>
	/// 差
	/// </summary>
	/// <param name="idxBegin">起始索引（含）</param>
	/// <param name="idxEnd">结束索引（含）</param>
	/// <returns>HRESULT</returns>
	STDMETHODIMP ExcludeRange(LONG idxBegin, LONG idxEnd)
	{
		if (idxEnd < idxBegin || idxBegin < 0)
			return E_INVALIDARG;
		BOOL bFound0, bFound1;
		const auto it0 = FindRange(idxBegin, bFound0);
		const auto it1 = FindRange(idxEnd, bFound1);
		if (bFound0 && bFound1 && it0 == it1)// 在同一个区间内
		{
			const auto t = it0->idxEnd;
			it0->idxEnd = idxBegin - 1;
			if (it0->idxEnd < it0->idxBegin)// 左半区间被完全切除
			{
				if (idxEnd + 1 <= t)// 右半区间应保留
				{
					it0->idxBegin = idxEnd + 1;
					it0->idxEnd = t;
				}
				else// 右半区间被完全切除
					m_vRange.erase(it0);
			}
			else// 左半区间应保留
			{
				if (idxEnd + 1 <= t)// 右半区间应保留
					m_vRange.emplace(it0 + 1, idxEnd + 1, t);
				// else// 右半区间被完全切除
			}
			return S_OK;
		}

		auto itEraseBegin = it0,
			itEraseEnd = it1;
		if (bFound0)
		{
			it0->idxEnd = idxBegin - 1;
			if (it0->idxEnd >= it0->idxBegin)
				++itEraseBegin;
		}
		if (bFound1)
		{
			it1->idxBegin = idxEnd + 1;
			if (it1->idxEnd >= it1->idxBegin)
			{
				if (itEraseEnd == m_vRange.begin())
					itEraseEnd = m_vRange.begin();
				else
					--itEraseEnd;
			}
		}
		if (itEraseEnd > itEraseBegin)
			m_vRange.erase(itEraseBegin, itEraseEnd);
		return S_OK;
	}

	/// <summary>
	/// 取反
	/// </summary>
	/// <param name="idxBegin">起始索引（含）</param>
	/// <param name="idxEnd">结束索引（含）</param>
	/// <returns>HRESULT</returns>
	STDMETHODIMP InvertRange(LONG idxBegin, LONG idxEnd)
	{
		if (idxEnd < idxBegin || idxBegin < 0)
			return E_INVALIDARG;
		BOOL bFound0, bFound1;
		auto it0 = FindRange(idxBegin, bFound0);
		auto it1 = FindRange(idxEnd, bFound1);
		const auto d0 = it0 - m_vRange.begin() +
			((it0 == it1) ? 1 : 0);// 若it0 == it1，保证it0与it1指向同一个区间
		it1 = m_vRange.emplace(it1) + 1;
		it0 = m_vRange.begin() + d0;
		const auto itHole = it1 - 1;// 新插入的空洞区间

		if (it0 == it1)// 给一个特判...
		{
			if (bFound0)
			{
				if (bFound1)
				{
					itHole->idxBegin = it0->idxBegin;
					itHole->idxEnd = idxBegin - 1;
					it0->idxBegin = idxEnd + 1;
					it0->idxEnd = it0->idxEnd;
					if (it0->idxEnd < it0->idxBegin)
						m_vRange.erase(it0);
					if (itHole->idxEnd < itHole->idxBegin)
						m_vRange.erase(itHole);
				}// else 不可能出现
			}
			else
			{
				if (bFound1)
				{
					itHole->idxBegin = idxBegin;
					itHole->idxEnd = it0->idxBegin - 1;
					it0->idxBegin = idxEnd + 1;
					if (it0->idxEnd < it0->idxBegin)
						m_vRange.erase(it0);
					if (itHole->idxEnd < itHole->idxBegin)
						m_vRange.erase(itHole);
				}
				else
				{
					itHole->idxBegin = idxBegin;
					itHole->idxEnd = idxEnd;
					// 不可能无效
				}
			}
			return S_OK;
		}
		// bFound0 == 1时应跳过it0，否则应给予特判填充，两者都需要+1
		const auto itInvertBegin = it0 + 1;
		const auto itInvertEnd = itHole - 1;
		itHole->idxBegin = (itHole - 1)->idxEnd + 1;
		itHole->idxEnd = (bFound1 ? it1->idxBegin - 1 : idxEnd);
		// 平移
		for (auto it = itInvertEnd; it >= itInvertBegin; --it)
		{
			it->idxEnd = it->idxBegin - 1;
			it->idxBegin = (it - 1)->idxEnd + 1;
		}

		if (bFound0)// 裁掉
			it0->idxEnd = idxBegin - 1;
		else// 填充为idxBegin到it0->idxBegin的区间
		{
			it0->idxEnd = it0->idxBegin - 1;
			it0->idxBegin = idxBegin;
		}

		if (bFound1)// 裁掉
			it1->idxBegin = idxEnd + 1;

		// 压实并移除0长度区间
		for (auto it = m_vRange.end() - 1; it > m_vRange.begin();)
		{
			if (it->idxEnd < it->idxBegin)
			{
				--it;
				m_vRange.erase(it + 1);
			}
			else if (it->idxBegin <= (it - 1)->idxEnd + 1)
			{
				(it - 1)->idxEnd = it->idxEnd;
				--it;
				m_vRange.erase(it + 1);
			}
			else
				--it;
		}
		return S_OK;
	}

	/// <summary>
	/// 插入项目。
	/// 在指定的位置增加一个未选择项目（即新项目）
	/// </summary>
	/// <param name="idxItem">项目索引</param>
	/// <returns>HRESULT</returns>
	STDMETHODIMP InsertItem(LONG idxItem)
	{
		BOOL bFound;
		const auto it = FindRange(idxItem, bFound);
		std::vector<RANGE>::iterator itIncBegin;
		if (bFound)
		{
			if (it->idxBegin == idxItem)
				itIncBegin = it;
			else
			{
				const auto t = it->idxEnd;
				it->idxEnd = idxItem - 1;
				// 无需+1，后续统一处理
				itIncBegin = m_vRange.emplace(it + 1, idxItem, t);
			}
		}
		else
			itIncBegin = it;
		for (auto it = itIncBegin; it != m_vRange.end(); ++it)
		{
			++it->idxBegin;
			++it->idxEnd;
		}
		return S_OK;
	}

	/// <summary>
	/// 删除项目
	/// </summary>
	/// <param name="idxItem"></param>
	/// <returns></returns>
	STDMETHODIMP RemoveItem(LONG idxItem)
	{
		BOOL bFound;
		const auto it = FindRange(idxItem, bFound);
		if (it == m_vRange.end())
			return S_OK;
		std::vector<RANGE>::iterator itDecBegin;
		BOOL bEraseIt;
		if (bFound)
		{
			--it->idxEnd;
			bEraseIt = (it->idxEnd < it->idxBegin);
			itDecBegin = it + 1;
		}
		else
		{
			itDecBegin = it;
			bEraseIt = FALSE;
			if (it != m_vRange.begin())
			{
				if (idxItem == (it - 1)->idxEnd + 1 &&
					idxItem == it->idxBegin - 1)
				{
					(it - 1)->idxEnd = it->idxEnd - 1;
					bEraseIt = TRUE;
					++itDecBegin;
				}
			}
		}
		for (auto it = itDecBegin; it != m_vRange.end(); ++it)
		{
			--it->idxBegin;
			--it->idxEnd;
		}
		if (bEraseIt)
			m_vRange.erase(it);
		return S_OK;
	}

	STDMETHODIMP Clear()
	{
		m_vRange.clear();
		return S_OK;
	}

	STDMETHODIMP IsSelected(LONG idxItem)
	{
		BOOL bFound;
		FindRange(idxItem, bFound);
		return bFound ? S_OK : S_FALSE;
	}

	STDMETHODIMP IsEmpty()
	{
		return (m_vRange.empty()) ? S_OK : S_FALSE;
	}

	/// <summary>
	/// 取下一选中项
	/// </summary>
	/// <param name="idxItem">当前项目</param>
	/// <param name="pidxItem">下一项目，若无则设为-1</param>
	/// <returns>HRESULT</returns>
	STDMETHODIMP NextSelected(LONG idxItem, LONG* pidxItem)
	{
		BOOL bFound;
		const auto it = FindRange(idxItem, bFound);
		if (bFound && idxItem != it->idxEnd)
			*pidxItem = idxItem + 1;
		else if (it != m_vRange.end())
			*pidxItem = (it + 1)->idxBegin;
		else
			*pidxItem = -1;
		return S_OK;
	}

	/// <summary>
	/// 取下一未选中项
	/// </summary>
	/// <param name="idxItem">当前项目</param>
	/// <param name="pidxItem">下一项目，若无则设为-1</param>
	/// <returns>HRESULT</returns>
	STDMETHODIMP NextUnSelected(LONG idxItem, LONG* pidxItem)
	{
		BOOL bFound;
		const auto it = FindRange(idxItem, bFound);
		if (it == m_vRange.end())
			*pidxItem = -1;
		else if (!bFound && idxItem != it->idxBegin - 1)
			*pidxItem = idxItem + 1;
		else
			*pidxItem = it->idxEnd + 1;
		return S_OK;
	}

	// 计算选中项数
	STDMETHODIMP CountIncluded(LONG* pcIncluded)
	{
		LONG c{};
		for (const auto& e : m_vRange)
			c += (e.idxEnd - e.idxBegin + 1);
		*pcIncluded = c;
		return S_OK;
	}

	EckInline constexpr const auto& GetList() const { return m_vRange; }
};
ECK_NAMESPACE_END