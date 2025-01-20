#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
template<class T>
class CSelRangeT
{
public:
	struct RANGE// 闭区间
	{
		T idxBegin;
		T idxEnd;
	};
private:
	std::vector<RANGE> m_vRange{};// 从小到大排列

	/// <summary>
	/// 查找所在区间
	/// </summary>
	/// <param name="idxItem">项目索引</param>
	/// <param name="bFound">是否在返回的区间内</param>
	/// <returns>搜寻结束时的迭代器，若未找到，此迭代器指向第一个大于idxItem的区间</returns>
	constexpr auto FindRange(T idxItem, _Out_ BOOL& bFound)
	{
		const auto it = std::lower_bound(m_vRange.begin(), m_vRange.end(), idxItem,
			[](RANGE r, T idx) { return r.idxEnd < idx; });
		bFound = (it != m_vRange.end() && idxItem >= it->idxBegin);
		return it;
	}

	constexpr auto FindRange(T idxItem, _Out_ BOOL& bFound) const
	{
		const auto it = std::lower_bound(m_vRange.begin(), m_vRange.end(), idxItem,
			[](RANGE r, T idx) { return r.idxEnd < idx; });
		bFound = (it != m_vRange.end() && idxItem >= it->idxBegin);
		return it;
	}
public:
	CSelRangeT() = default;
	constexpr CSelRangeT(std::initializer_list<RANGE> il) : m_vRange{ il } {}

	/// <summary>
	/// 并
	/// </summary>
	/// <param name="idxBegin">起始索引（含）</param>
	/// <param name="idxEnd">结束索引（含）</param>
	constexpr void IncludeRange(T idxBegin, T idxEnd)
	{
		EckAssert(idxEnd >= idxBegin && idxBegin >= 0);
		BOOL bFound0, bFound1;
		auto it0 = FindRange(idxBegin, bFound0);
		if (it0 == m_vRange.end())// 左边界大于所有区间
		{
			if (!m_vRange.empty() && idxBegin == m_vRange.back().idxEnd + 1)
				m_vRange.back().idxEnd = idxEnd;
			else
				m_vRange.emplace_back(idxBegin, idxEnd);
			return;
		}
		const auto it1 = FindRange(idxEnd, bFound1);
		if (it1 == m_vRange.begin() && !bFound1)// 右边界小于所有区间
		{
			if (idxEnd == it1->idxBegin - 1)
				it1->idxBegin = idxBegin;
			else
				m_vRange.emplace(it1, idxBegin, idxEnd);
			return;
		}

		if (it0 == it1 && !bFound0 && !bFound1)
		{
			if (it0 != m_vRange.begin() && idxBegin <= (it0 - 1)->idxEnd + 1)
				if (idxEnd + 1 >= it0->idxBegin)
				{
					(it0 - 1)->idxEnd = it0->idxEnd;
					m_vRange.erase(it0);
				}
				else
					(it0 - 1)->idxEnd = idxEnd;
			else if (idxEnd + 1 >= it0->idxBegin)
				it0->idxBegin = idxBegin;
			else
				m_vRange.emplace(it0, idxBegin, idxEnd);
			return;
		}

		auto itEraseBegin = it0 + 1;// 无论如何，留下左边界处的一个区间
		auto itEraseEnd = it1 + (bFound1 ? 1 : 0);

		if (!bFound0)// 需要修改左界
		{
			if (it0 != m_vRange.begin() && idxBegin <= (it0 - 1)->idxEnd + 1)
			{
				(it0 - 1)->idxEnd = it0->idxEnd;
				--itEraseBegin;
				--it0;
			}
			else
				it0->idxBegin = idxBegin;
		}

		if (bFound1)
			it0->idxEnd = it1->idxEnd;
		else if (idxEnd > it0->idxEnd)
			it0->idxEnd = idxEnd;

		if (itEraseEnd != m_vRange.end() && itEraseEnd->idxBegin <= it0->idxEnd + 1)
		{
			if (itEraseEnd->idxEnd > it0->idxEnd)
				it0->idxEnd = itEraseEnd->idxEnd;
			++itEraseEnd;
		}

		if (itEraseEnd > itEraseBegin)
			m_vRange.erase(itEraseBegin, itEraseEnd);
	}

	constexpr void IncludeItem(T idxItem)
	{
		IncludeRange(idxItem, idxItem);
	}

	/// <summary>
	/// 差
	/// </summary>
	/// <param name="idxBegin">起始索引（含）</param>
	/// <param name="idxEnd">结束索引（含）</param>
	constexpr void ExcludeRange(T idxBegin, T idxEnd)
	{
		EckAssert(idxEnd >= idxBegin && idxBegin >= 0);
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
			return;
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
	}

	constexpr void ExcludeItem(T idxItem)
	{
		ExcludeRange(idxItem, idxItem);
	}

	/// <summary>
	/// 取反
	/// </summary>
	/// <param name="idxBegin">起始索引（含）</param>
	/// <param name="idxEnd">结束索引（含）</param>
	constexpr void InvertRange(T idxBegin, T idxEnd)
	{
		EckAssert(idxEnd >= idxBegin && idxBegin >= 0);
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
			return;
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
	}

	constexpr void InvertItem(T idxItem)
	{
		InvertRange(idxItem, idxItem);
	}

	/// <summary>
	/// 插入项目。
	/// 在指定的位置增加一个未选择项目（即新项目），此操作会使目标索引其后的所有项目索引+1。
	/// </summary>
	/// <param name="idxItem">项目索引</param>
	constexpr void InsertItem(T idxItem)
	{
		EckAssert(idxItem >= 0);
		BOOL bFound;
		const auto it = FindRange(idxItem, bFound);
		typename std::vector<RANGE>::iterator itIncBegin;
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
	}

	/// <summary>
	/// 删除项目。
	/// 删除指定的项目，此操作会使目标索引其后的所有项目索引-1。
	/// </summary>
	/// <param name="idxItem">项目索引</param>
	constexpr void RemoveItem(T idxItem)
	{
		EckAssert(idxItem >= 0);
		BOOL bFound;
		const auto it = FindRange(idxItem, bFound);
		if (it == m_vRange.end())
			return;
		typename std::vector<RANGE>::iterator itDecBegin;
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
	}

	EckInline constexpr void Clear()
	{
		m_vRange.clear();
	}

	/// <summary>
	/// 清除。
	/// 取消所有项目的选中，并返回状态更改的区间，以便调用方执行更新操作
	/// </summary>
	/// <param name="idxVisibleBegin">可视区间起始索引</param>
	/// <param name="idxVisibleEnd">可视区间结束索引</param>
	/// <param name="idxChangedBegin">被更改区间起始索引，若无需更新则为-1</param>
	/// <param name="idxChangedEnd">被更改区间结束索引，若无需更新则为-1</param>
	EckInline constexpr void Clear(T idxVisibleBegin, T idxVisibleEnd,
		_Out_ T& idxChangedBegin, _Out_ T& idxChangedEnd)
	{
		BOOL bFound;
		auto it = FindRange(idxVisibleBegin, bFound);
		if (it == m_vRange.end())
			idxChangedBegin = idxChangedEnd = -1;
		else
		{
			idxChangedBegin = it->idxBegin;
			it = FindRange(idxVisibleEnd, bFound);
			if (it == m_vRange.end() || it == m_vRange.begin())
				idxChangedBegin = idxChangedEnd = -1;
			else
				idxChangedEnd = it->idxEnd;
		}
		Clear();
	}

	EckInline constexpr BOOL IsSelected(T idxItem) const
	{
		BOOL bFound;
		FindRange(idxItem, bFound);
		return bFound;
	}

	EckInline constexpr BOOL IsEmpty() const
	{
		return m_vRange.empty();
	}

	// 取下一选中项。若idxItem选中，则返回idxItem
	constexpr T NextSelected(T idxItem) const
	{
		BOOL bFound;
		const auto it = FindRange(idxItem, bFound);
		if (bFound)
			return idxItem;
		else if (it != m_vRange.end())
			return it->idxBegin;
		else
			return -1;
	}

	// 取下一未选中项。若idxItem未选中，则返回idxItem
	constexpr T NextUnSelected(T idxItem) const
	{
		BOOL bFound;
		const auto it = FindRange(idxItem, bFound);
		if (bFound)
			return it->idxEnd + 1;
		else
			return idxItem;
	}

	// 计算选中项数
	EckInline constexpr T CountIncluded() const
	{
		T c{};
		for (const auto& e : m_vRange)
			c += (e.idxEnd - e.idxBegin + 1);
		return c;
	}

	EckInline constexpr const auto& GetList() const { return m_vRange; }

	EckInline constexpr void OnSetItemCount(T cItem)
	{
		ExcludeRange(cItem - 1, std::numeric_limits<T>::max());
	}

	EckInline constexpr T GetFirstSelected() const
	{
		if (m_vRange.empty())
			return -1;
		return m_vRange.front().idxBegin;
	}

	EckInline constexpr T GetLastSelected() const
	{
		if (m_vRange.empty())
			return -1;
		return m_vRange.back().idxEnd;
	}
};

using CSelRange = CSelRangeT<int>;

class CLVRange :public ILVRange
{
private:
	ULONG m_cRef{ 1 };
	CSelRange m_SelRange{};
public:
	STDMETHODIMP_(ULONG) AddRef() { return ++m_cRef; }

	STDMETHODIMP_(ULONG) Release()
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

	STDMETHODIMP IncludeRange(LONG idxBegin, LONG idxEnd)
	{
		m_SelRange.IncludeRange(idxBegin, idxEnd);
		return S_OK;
	}

	STDMETHODIMP ExcludeRange(LONG idxBegin, LONG idxEnd)
	{
		m_SelRange.ExcludeRange(idxBegin, idxEnd);
		return S_OK;
	}

	STDMETHODIMP InvertRange(LONG idxBegin, LONG idxEnd)
	{
		m_SelRange.InvertRange(idxBegin, idxEnd);
		return S_OK;
	}

	STDMETHODIMP InsertItem(LONG idxItem)
	{
		m_SelRange.InsertItem(idxItem);
		return S_OK;
	}

	STDMETHODIMP RemoveItem(LONG idxItem)
	{
		m_SelRange.RemoveItem(idxItem);
		return S_OK;
	}

	STDMETHODIMP Clear()
	{
		m_SelRange.Clear();
		return S_OK;
	}

	STDMETHODIMP IsSelected(LONG idxItem)
	{
		return m_SelRange.IsSelected(idxItem) ? S_OK : S_FALSE;
	}

	STDMETHODIMP IsEmpty()
	{
		return m_SelRange.IsEmpty() ? S_OK : S_FALSE;
	}

	STDMETHODIMP NextSelected(LONG idxItem, LONG* pidxItem)
	{
		*pidxItem = m_SelRange.NextSelected(idxItem);
		return S_OK;
	}

	STDMETHODIMP NextUnSelected(LONG idxItem, LONG* pidxItem)
	{
		*pidxItem = m_SelRange.NextUnSelected(idxItem);
		return S_OK;
	}

	STDMETHODIMP CountIncluded(LONG* pcIncluded)
	{
		*pcIncluded = m_SelRange.CountIncluded();
		return S_OK;
	}

	EckInlineCe auto& GetSelRange() const { return m_SelRange; }
	EckInlineCe auto& GetSelRange() { return m_SelRange; }
};
ECK_NAMESPACE_END