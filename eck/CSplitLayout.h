/*
* WinEzCtrlKit Library
*
* CSplitLayout.h ： 分割布局
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "CLayoutBase.h"

enum :UINT
{
	// 固定宽度或高度，若未设置该标志，则将其宽度或高度充满到布局尺寸
	SLF_FIXWH = 1u << 0,
};

ECK_NAMESPACE_BEGIN
class CSplitLayout :public CLayoutBase
{
protected:
	struct CTRL
	{
		ILayout* pCtrl;
		MARGINS Margin;
		UINT uFlags;
		Align eAlign;
		short cx;
		short cy;
		UINT uWeight;
	};

	std::vector<CTRL> m_vCtrl{};
	UINT m_uTotalWeight{};
	BOOL m_bScaleMode{ TRUE };
public:
	EckInline void SetScaleMode(BOOL bScaleMode)
	{
		EckAssert(m_vCtrl.empty());
		m_bScaleMode = bScaleMode;
	}

	void LoGetAppropriateSize(int& cx, int& cy) override
	{
		cx = m_cx;
		cy = m_cy;
	}

	void Clear() override
	{
		CLayoutBase::Clear();
		m_vCtrl.clear();
		m_uTotalWeight = 0u;
	}
};

class CSplitLayoutH :public CSplitLayout
{
public:
	void LoCommit() override
	{
		const HDWP hDwpParent = (m_pParent ? m_pParent->LoGetCurrHDWP() : NULL);
		HDWP hDwp = (hDwpParent ? hDwpParent : BeginDeferWindowPos((int)m_vCtrl.size()));
		int x = m_x, y, cx, cy;
		int cxLeave{};
		UINT uTotalWeight{};

		for (int i{}; const auto& e : m_vCtrl)
		{
			if (m_bScaleMode)
				cx = e.uWeight * m_cx / m_uTotalWeight;
			else
				cx = ((i == (int)m_vCtrl.size() - 1) ? (m_cx - x) : (m_vCtrl[i + 1].uWeight - x));
			cx -= (e.Margin.cxLeftWidth + e.Margin.cxRightWidth);
			if (e.uFlags & SLF_FIXWH)
			{
				cy = e.cy;
				switch (e.eAlign)
				{
				case Align::Near:
					y = m_y + e.Margin.cyTopHeight;
					break;
				case Align::Center:
					y = m_y + e.Margin.cyTopHeight +
						(m_cy - (cy + e.Margin.cyTopHeight + e.Margin.cyBottomHeight)) / 2;
					break;
				case Align::Far:
					y = m_y + m_cy - cy - e.Margin.cyTopHeight - e.Margin.cyBottomHeight;
					break;
				default:
					ECK_UNREACHABLE;
				}
			}
			else
			{
				cy = m_cy;
				y = m_y + e.Margin.cyTopHeight;
			}

			x += e.Margin.cxLeftWidth;

			const auto hWnd = e.pCtrl->LoGetHWND();
			if (hWnd)
			{
				hDwp = DeferWindowPos(hDwp, hWnd, NULL,
					x, y, cx, cy, SWP_NOZORDER | SWP_NOACTIVATE);
			}
			else
			{
				e.pCtrl->LoSetPosSize(x, y, cx, cy);
				e.pCtrl->LoCommit();
			}
			x += (cx + e.Margin.cxRightWidth);
		}
		if (!hDwpParent)
			EndDeferWindowPos(hDwp);
	}

	void Add(ILayout* pCtrl, const MARGINS& Margin = {}, UINT uFlags = 0u, 
		UINT uWeight = 0u, Align eAlign = Align::Near)
	{
		pCtrl->LoSetParent(this);
		const auto size = pCtrl->LoGetSize();
		m_vCtrl.emplace_back(pCtrl, Margin, uFlags, eAlign, 
			(short)size.first, (short)size.second, uWeight);
		m_uTotalWeight += uWeight;
		m_cx += (size.first + Margin.cxLeftWidth + Margin.cxRightWidth);
		m_cy = std::max(m_cy, size.second);
	}
};

class CSplitLayoutV :public CSplitLayout
{
public:
	void LoCommit() override
	{
		const HDWP hDwpParent = (m_pParent ? m_pParent->LoGetCurrHDWP() : NULL);
		HDWP hDwp = (hDwpParent ? hDwpParent : BeginDeferWindowPos((int)m_vCtrl.size()));
		int x, y = m_y, cx, cy;
		int cxLeave{};
		UINT uTotalWeight{};

		for (int i{}; const auto & e : m_vCtrl)
		{
			if (m_bScaleMode)
				cy = e.uWeight * m_cy / m_uTotalWeight;
			else
				cy = ((i == (int)m_vCtrl.size() - 1) ? (m_cy - y) : (m_vCtrl[i + 1].uWeight - y));
			cy -= (e.Margin.cyTopHeight + e.Margin.cyBottomHeight);
			if (e.uFlags & SLF_FIXWH)
			{
				cx = e.cx;
				switch (e.eAlign)
				{
				case Align::Near:
					x = m_x + e.Margin.cxLeftWidth;
					break;
				case Align::Center:
					x = m_x + e.Margin.cxLeftWidth +
						(m_cx - (cx + e.Margin.cxLeftWidth + e.Margin.cxRightWidth)) / 2;
					break;
				case Align::Far:
					x = m_x + m_cx - cx - e.Margin.cxRightWidth - e.Margin.cxLeftWidth;
					break;
				default:
					ECK_UNREACHABLE;
				}
			}
			else
			{
				cx = m_cx;
				x = m_x + e.Margin.cxLeftWidth;
			}

			y += e.Margin.cyTopHeight;

			const auto hWnd = e.pCtrl->LoGetHWND();
			if (hWnd)
			{
				hDwp = DeferWindowPos(hDwp, hWnd, NULL,
					x, y, cx, cy, SWP_NOZORDER | SWP_NOACTIVATE);
			}
			else
			{
				e.pCtrl->LoSetPosSize(x, y, cx, cy);
				e.pCtrl->LoCommit();
			}
			y += (cy + e.Margin.cyBottomHeight);
		}
		if (!hDwpParent)
			EndDeferWindowPos(hDwp);
	}

	void Add(ILayout* pCtrl, const MARGINS& Margin = {}, UINT uFlags = 0u,
		UINT uWeight = 0u, Align eAlign = Align::Near)
	{
		pCtrl->LoSetParent(this);
		const auto size = pCtrl->LoGetSize();
		m_vCtrl.emplace_back(pCtrl, Margin, uFlags, eAlign,
			(short)size.first, (short)size.second, uWeight);
		m_uTotalWeight += uWeight;
		m_cx += (size.first + Margin.cxLeftWidth + Margin.cxRightWidth);
		m_cy = std::max(m_cy, size.second);
	}
};
ECK_NAMESPACE_END