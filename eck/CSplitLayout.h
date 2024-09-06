/*
* WinEzCtrlKit Library
*
* CSplitLayout.h ： 分割布局
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "CLayoutBase.h"

ECK_NAMESPACE_BEGIN
class CSplitLayout :public CLayoutBase
{
protected:
	struct ITEM :public ITEMBASE
	{
		UINT uWeight;

		ITEM() = default;
		constexpr ITEM(ILayout* pCtrl, const MARGINS& Margin, UINT uFlags, short cx, short cy,
			UINT uWeight)
			: ITEMBASE{ pCtrl, Margin, uFlags, cx, cy }, uWeight{ uWeight } {}
	};

	std::vector<ITEM> m_vItem{};
	UINT m_uTotalWeight{};
	BOOL m_bScaleMode{ TRUE };
public:
	EckInline constexpr void SetScaleMode(BOOL bScaleMode) { m_bScaleMode = bScaleMode; }

	EckInline constexpr BOOL GetScaleMode() const { return m_bScaleMode; }

	void Clear() override
	{
		CLayoutBase::Clear();
		m_vItem.clear();
		m_uTotalWeight = 0u;
	}

	EckInline constexpr auto& GetList() { return m_vItem; }
};

class CSplitLayoutH final :public CSplitLayout
{
public:
	void LoCommit() override
	{
		HDWP hDwp = PreArrange(m_vItem.size());
		int x, y, cx, cy;
		int xStep{ m_x }, cxTemp;
		for (int i{}; const auto & e : m_vItem)
		{
			if (m_bScaleMode)
				cxTemp = e.uWeight * m_cx / m_uTotalWeight;
			else
				cxTemp = ((i == (int)m_vItem.size() - 1) ?
					(m_cx - xStep) : (m_vItem[i + 1].uWeight - xStep));
			CalcCtrlPosSize(e, { xStep,m_y,cxTemp,m_cy }, x, y, cx, cy);
			MoveCtrlPosSize(e, hDwp, x, y, cx, cy);
			xStep += cxTemp;
		}
		PostArrange(hDwp);
	}

	void Add(ILayout* pCtrl, const MARGINS& Margin = {}, UINT uFlags = 0u, UINT uWeight = 0u)
	{
		const auto size = pCtrl->LoGetSize();
		m_vItem.emplace_back(pCtrl, Margin, uFlags,
			(short)size.first, (short)size.second, uWeight);
		m_uTotalWeight += uWeight;
		m_cx += (size.first + Margin.cxLeftWidth + Margin.cxRightWidth);
		m_cy = std::max(m_cy, size.second + Margin.cyTopHeight + Margin.cyBottomHeight);
	}

	void Refresh() override
	{
		m_uTotalWeight = 0u;
		m_cx = m_cy = 0;
		for (auto& e : m_vItem)
		{
			const auto size = e.pCtrl->LoGetSize();
			e.cx = (short)size.first;
			e.cy = (short)size.second;
			m_uTotalWeight += e.uWeight;
			m_cx += (size.first + e.Margin.cxLeftWidth + e.Margin.cxRightWidth);
			m_cy = std::max(m_cy, size.second + e.Margin.cyTopHeight + e.Margin.cyBottomHeight);
		}
	}
};

class CSplitLayoutV final :public CSplitLayout
{
public:
	void LoCommit() override
	{
		HDWP hDwp = PreArrange(m_vItem.size());
		int x, y, cx, cy;
		int yStep{ m_y }, cyTemp;
		for (int i{}; const auto & e : m_vItem)
		{
			if (m_bScaleMode)
				cyTemp = e.uWeight * m_cy / m_uTotalWeight;
			else
				cyTemp = ((i == (int)m_vItem.size() - 1) ?
					(m_cy - yStep) : (m_vItem[i + 1].uWeight - yStep));
			CalcCtrlPosSize(e, { m_x,yStep,m_cx,cyTemp }, x, y, cx, cy);
			MoveCtrlPosSize(e, hDwp, x, y, cx, cy);
			yStep += cyTemp;
		}
		PostArrange(hDwp);
	}

	void Add(ILayout* pCtrl, const MARGINS& Margin = {}, UINT uFlags = 0u, UINT uWeight = 0u)
	{
		const auto size = pCtrl->LoGetSize();
		m_vItem.emplace_back(pCtrl, Margin, uFlags,
			(short)size.first, (short)size.second, uWeight);
		m_uTotalWeight += uWeight;
		m_cx += (size.first + Margin.cxLeftWidth + Margin.cxRightWidth);
		m_cy = std::max(m_cy, size.second);
	}

	void Refresh() override
	{
		m_uTotalWeight = 0u;
		m_cx = m_cy = 0;
		for (auto& e : m_vItem)
		{
			const auto size = e.pCtrl->LoGetSize();
			e.cx = (short)size.first;
			e.cy = (short)size.second;
			m_uTotalWeight += e.uWeight;
			m_cx += (size.first + e.Margin.cxLeftWidth + e.Margin.cxRightWidth);
			m_cy = std::max(m_cy, size.second + e.Margin.cyTopHeight + e.Margin.cyBottomHeight);
		}
	}
};
ECK_NAMESPACE_END