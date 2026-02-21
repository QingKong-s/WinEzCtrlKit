#pragma once
#include "CUnknown.h"

ECK_NAMESPACE_BEGIN
template<class T>
    requires std::is_signed_v<T>
class CSelectionRangeT
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
    constexpr auto FindRange(T idxItem, _Out_ BOOL& bFound) noexcept
    {
        const auto it = std::lower_bound(m_vRange.begin(), m_vRange.end(), idxItem,
            [](RANGE r, T idx) { return r.idxEnd < idx; });
        bFound = (it != m_vRange.end() && idxItem >= it->idxBegin);
        return it;
    }

    constexpr auto FindRange(T idxItem, _Out_ BOOL& bFound) const noexcept
    {
        const auto it = std::lower_bound(m_vRange.begin(), m_vRange.end(), idxItem,
            [](RANGE r, T idx) { return r.idxEnd < idx; });
        bFound = (it != m_vRange.end() && idxItem >= it->idxBegin);
        return it;
    }
public:
    CSelectionRangeT() = default;
    constexpr CSelectionRangeT(std::initializer_list<RANGE> il) noexcept : m_vRange{ il } {}

    // 并闭区间
    void IncludeRange(T idxBegin, T idxEnd) noexcept
    {
        EckAssert(idxEnd >= idxBegin);
        // 第一个End >= idxBegin - 1的区间
        const auto itLeft = std::lower_bound(m_vRange.begin(), m_vRange.end(), idxBegin - 1,
            [](const RANGE& r, T val) { return r.idxEnd < val; });
        // 第一个Begin > idxEnd + 1的区间
        const auto itRight = std::upper_bound(itLeft, m_vRange.end(), idxEnd + 1,
            [](T val, const RANGE& r) { return val < r.idxBegin; });
        // 此时[itLeft, itRight)是所有需要被合并或处理的区间
        if (itLeft == itRight)// 没有重叠
            m_vRange.insert(itLeft, { idxBegin, idxEnd });
        else
        {
            itLeft->idxBegin = std::min(idxBegin, itLeft->idxBegin);
            itLeft->idxEnd = std::max(idxEnd, (itRight - 1)->idxEnd);// (itRight - 1)最后一个重叠区间
            if (itLeft + 1 < itRight)
                m_vRange.erase(itLeft + 1, itRight);
        }
    }

    constexpr void IncludeItem(T idxItem) noexcept
    {
        IncludeRange(idxItem, idxItem);
    }

    // 差闭区间
    void ExcludeRange(T idxBegin, T idxEnd) noexcept
    {
        EckAssert(idxEnd >= idxBegin);
        // 可能被切除的第一个区间(End >= Begin)
        auto it = std::lower_bound(m_vRange.begin(), m_vRange.end(), idxBegin,
            [](const RANGE& r, T val) { return r.idxEnd < val; });
        if (it == m_vRange.end())
            return;

        while (it != m_vRange.end() && it->idxBegin <= idxEnd)
        {
            // 区间一分为二
            if (it->idxBegin < idxBegin && it->idxEnd > idxEnd)
            {
                const RANGE rightPart{ idxEnd + 1, it->idxEnd };
                it->idxEnd = idxBegin - 1;
                it = m_vRange.insert(it + 1, rightPart);
                break;
            }

            // 区间被完全删除
            if (it->idxBegin >= idxBegin && it->idxEnd <= idxEnd)
            {
                // 记录下一个位置，即批量删除的起点
                auto itNext = it + 1;
                // 查找能否批量删除
                while (itNext != m_vRange.end() && itNext->idxEnd <= idxEnd)
                    ++itNext;
                it = m_vRange.erase(it, itNext);
                continue;
            }

            if (it->idxBegin < idxBegin)// 删除右侧
            {
                it->idxEnd = idxBegin - 1;
                ++it;
            }
            else// it->idxEnd > idxEnd，删除左侧
            {
                it->idxBegin = idxEnd + 1;
                break;
            }
        }
    }

    constexpr void ExcludeItem(T idxItem) noexcept
    {
        ExcludeRange(idxItem, idxItem);
    }

    // 取反闭区间
    constexpr void InvertRange(T idxBegin, T idxEnd) noexcept
    {
        // 第一个结束位置 >= idxBegin的区间，可能与反转区左侧重叠
        auto itFirst = std::lower_bound(m_vRange.begin(), m_vRange.end(), idxBegin,
            [](const RANGE& r, T val) { return r.idxEnd < val; });
        // 第一个开始位置 > idxEnd的区间，完全在反转区右侧
        auto itLast = std::upper_bound(itFirst, m_vRange.end(), idxEnd,
            [](T val, const RANGE& r) { return val < r.idxBegin; });

        if (itFirst == itLast)
        {
            auto idxNewBegin = idxBegin;
            auto idxNewEnd = idxEnd;
            auto itInsertPos = itFirst;
            // 左侧合并
            if (itFirst != m_vRange.begin())
            {
                const auto itPrev = itFirst - 1;
                if (itPrev->idxEnd + 1 == idxBegin)
                {
                    idxNewBegin = itPrev->idxBegin;
                    itInsertPos = itPrev; // 从左邻居位置开始处理
                }
            }
            // 右侧合并
            if (itFirst != m_vRange.end())
            {
                if (idxEnd + 1 == itFirst->idxBegin)
                {
                    idxNewEnd = itFirst->idxEnd;
                    itLast = itFirst + 1;
                }
            }
            auto itPos = m_vRange.erase(itInsertPos, itLast);
            m_vRange.insert(itPos, { idxNewBegin, idxNewEnd });
            return;
        }

        std::vector<RANGE> vNewItems;
        vNewItems.reserve((itLast - itFirst) + 2);
        if (itFirst->idxBegin < idxBegin)// 左侧剩余
            vNewItems.push_back({ itFirst->idxBegin, idxBegin - 1 });
        auto idxCurr = idxBegin;
        for (auto it = itFirst; it != itLast; ++it)
        {
            const auto idxOverlapBegin = std::max(idxCurr, it->idxBegin);
            if (idxCurr < idxOverlapBegin)
                vNewItems.push_back({ idxCurr, idxOverlapBegin - 1 });
            idxCurr = std::min(it->idxEnd, idxEnd) + 1;
        }
        if (idxCurr <= idxEnd)// 右侧剩余
            vNewItems.push_back({ idxCurr, idxEnd });

        const auto itOverlappedLast = itLast - 1;
        if (itOverlappedLast->idxEnd > idxEnd)
            vNewItems.push_back({ idxEnd + 1, itOverlappedLast->idxEnd });

        const auto itInsertPos = m_vRange.erase(itFirst, itLast);
        m_vRange.insert(itInsertPos, vNewItems.begin(), vNewItems.end());
    }

    constexpr void InvertItem(T idxItem) noexcept
    {
        InvertRange(idxItem, idxItem);
    }

    // 插入项目。
    // 在指定的位置增加一个未选择项目（即新项目），此操作会使目标索引其后的所有项目索引+1
    constexpr void InsertItem(T idxItem) noexcept
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

    // 删除项目
    // 删除指定的项目，此操作会使目标索引其后的所有项目索引-1
    constexpr void RemoveItem(T idxItem) noexcept
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

    EckInlineCe void Clear() noexcept
    {
        m_vRange.clear();
    }

    /// <summary>
    /// 清除。
    /// 取消所有项目的选中，并返回状态更改的区间，以便调用方执行更新操作
    /// </summary>
    /// <param name="idxVisibleBegin、idxVisibleEnd">可视区间</param>
    /// <param name="idxChangedBegin、idxChangedEnd">被更改区间，若无需更新则为-1</param>
    EckInlineCe void Clear(T idxVisibleBegin, T idxVisibleEnd,
        _Out_ T& idxChangedBegin, _Out_ T& idxChangedEnd) noexcept
    {
        BOOL bFound;
        auto it = FindRange(idxVisibleBegin, bFound);
        if (it == m_vRange.end())
            idxChangedBegin = idxChangedEnd = -1;
        else
        {
            idxChangedBegin = it->idxBegin;
            if (idxVisibleEnd >= it->idxEnd)
                idxChangedEnd = it->idxEnd;
            else
            {
                it = FindRange(idxVisibleEnd, bFound);
                if (it == m_vRange.end() || it == m_vRange.begin())
                    idxChangedBegin = idxChangedEnd = -1;
                else if (bFound)
                    idxChangedEnd = idxVisibleEnd;
                else
                    idxChangedEnd = it->idxBegin;
            }
        }
        Clear();
    }

    EckInlineCe BOOL IsSelected(T idxItem) const noexcept
    {
        BOOL bFound;
        FindRange(idxItem, bFound);
        return bFound;
    }

    EckInlineCe BOOL IsEmpty() const noexcept
    {
        return m_vRange.empty();
    }

    // 取下一选中项。若idxItem选中，则返回idxItem
    constexpr T NextSelected(T idxItem) const noexcept
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
    constexpr T NextUnSelected(T idxItem) const noexcept
    {
        BOOL bFound;
        const auto it = FindRange(idxItem, bFound);
        if (bFound)
            return it->idxEnd + 1;
        else
            return idxItem;
    }

    // 计算选中项数
    EckInlineCe T CountIncluded() const noexcept
    {
        T c{};
        for (const auto& e : m_vRange)
            c += (e.idxEnd - e.idxBegin + 1);
        return c;
    }

    EckInlineCe const auto& GetList() const noexcept { return m_vRange; }

    EckInlineCe void OnSetItemCount(T cItem) noexcept
    {
        ExcludeRange(cItem, std::numeric_limits<T>::max());
    }

    EckInlineCe T GetFirstSelected() const noexcept
    {
        if (m_vRange.empty())
            return -1;
        return m_vRange.front().idxBegin;
    }

    EckInlineCe T GetLastSelected() const noexcept
    {
        if (m_vRange.empty())
            return -1;
        return m_vRange.back().idxEnd;
    }
};

using CSelRange = CSelectionRangeT<int>;

class CLVRange final : public CUnknown<CLVRange, ILVRange>
{
private:
    CSelRange m_SelRange{};
public:
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

    EckInlineCe auto& GetSelectionRange() const { return m_SelRange; }
    EckInlineCe auto& GetSelectionRange() { return m_SelRange; }
};
ECK_NAMESPACE_END