﻿#pragma once
#include "CLayoutBase.h"

ECK_NAMESPACE_BEGIN
class CSplitLayoutBase :public CLayoutBase
{
public:
	ECK_RTTI(CSplitLayoutBase);
protected:
	struct ITEM :public ITEMBASE
	{
		UINT uWeight;

		ITEM() = default;
		constexpr ITEM(ILayout* pCtrl, const MARGINS& Margin,
			UINT uFlags, short cx, short cy, UINT uWeight)
			: ITEMBASE{ pCtrl, Margin, uFlags, cx, cy }, uWeight{ uWeight } {
		}
	};

	std::vector<ITEM> m_vItem{};
	UINT m_uTotalWeight{};
	BOOL m_bScaleMode{ TRUE };
public:
	void LoOnDpiChanged(int iDpi) override
	{
		Refresh();
		for (auto& e : m_vItem)
		{
			ReCalcDpiSize(e, iDpi);
			e.pCtrl->LoOnDpiChanged(iDpi);
		}
		m_iDpi = iDpi;
	}

	void LoInitDpi(int iDpi) override
	{
		m_iDpi = iDpi;
		for (auto& e : m_vItem)
			e.pCtrl->LoInitDpi(iDpi);
	}

	EckInline constexpr void SetScaleMode(BOOL bScaleMode) { m_bScaleMode = bScaleMode; }

	EckInline constexpr BOOL GetScaleMode() const { return m_bScaleMode; }

	void Clear() override
	{
		CLayoutBase::Clear();
		m_vItem.clear();
		m_uTotalWeight = 0u;
	}

	EckInline constexpr auto& GetList() { return m_vItem; }

	void LoShow(BOOL bShow) override
	{
		for (const auto& e : GetList())
			e.pCtrl->LoShow(bShow);
	}
};
ECK_RTTI_IMPL_BASE_INLINE(CSplitLayoutBase, CLayoutBase);

class CSplitLayoutH final :public CSplitLayoutBase
{
public:
	ECK_RTTI(CSplitLayoutH);

	void LoCommit() override
	{
		HDWP hDwp = PreArrange(m_vItem.size());
		int x, y, cx, cy;
		int xStep{ m_x }, cxTemp;
		for (int i{}; const auto& e : m_vItem)
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
			(short)size.cx, (short)size.cy, uWeight);
		m_uTotalWeight += uWeight;
		m_cx += (size.cx + Margin.cxLeftWidth + Margin.cxRightWidth);
		m_cy = std::max(m_cy, (int)size.cy + Margin.cyTopHeight + Margin.cyBottomHeight);
	}

	void Refresh() override
	{
		m_uTotalWeight = 0u;
		m_cx = m_cy = 0;
		for (auto& e : m_vItem)
		{
			const auto size = e.pCtrl->LoGetSize();
			e.cx = (short)size.cx;
			e.cy = (short)size.cy;
			m_uTotalWeight += e.uWeight;
			m_cx += (size.cx + e.Margin.cxLeftWidth + e.Margin.cxRightWidth);
			m_cy = std::max(m_cy, (int)size.cy + e.Margin.cyTopHeight + e.Margin.cyBottomHeight);
		}
	}
};
ECK_RTTI_IMPL_BASE_INLINE(CSplitLayoutH, CSplitLayoutBase);

class CSplitLayoutV final :public CSplitLayoutBase
{
public:
	ECK_RTTI(CSplitLayoutV);

	void LoCommit() override
	{
		HDWP hDwp = PreArrange(m_vItem.size());
		int x, y, cx, cy;
		int yStep{ m_y }, cyTemp;
		for (int i{}; const auto& e : m_vItem)
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
			(short)size.cx, (short)size.cy, uWeight);
		m_uTotalWeight += uWeight;
		m_cx += (size.cx + Margin.cxLeftWidth + Margin.cxRightWidth);
		m_cy = std::max(m_cy, (int)size.cy);
	}

	void Refresh() override
	{
		m_uTotalWeight = 0u;
		m_cx = m_cy = 0;
		for (auto& e : m_vItem)
		{
			const auto size = e.pCtrl->LoGetSize();
			e.cx = (short)size.cx;
			e.cy = (short)size.cy;
			m_uTotalWeight += e.uWeight;
			m_cx += (size.cx + e.Margin.cxLeftWidth + e.Margin.cxRightWidth);
			m_cy = std::max(m_cy, (int)size.cy + e.Margin.cyTopHeight + e.Margin.cyBottomHeight);
		}
	}
};
ECK_RTTI_IMPL_BASE_INLINE(CSplitLayoutV, CSplitLayoutBase);
ECK_NAMESPACE_END