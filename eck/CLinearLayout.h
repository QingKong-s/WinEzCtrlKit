/*
* WinEzCtrlKit Library
*
* CLinearLayout.h ： 线性布局
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "CLayoutBase.h"

ECK_NAMESPACE_BEGIN
enum
{
	// 固定宽度
	LLF_FIXWIDTH = 1u << 0,
	// 固定高度
	LLF_FIXHEIGHT = 1u << 1,
	// 宽度占据剩余部分
	LLF_FILLWIDTH = 1u << 2,
	// 高度占据剩余部分
	LLF_FILLHEIGHT = 1u << 3,
};

class CLinearLayout :public CLayoutBase
{
protected:
	struct CTRL
	{
		ILayout* pCtrl;
		MARGINS Margin;
		UINT uFlags;
		short cx;
		short cy;
		UINT uWeight;
	};

	std::vector<CTRL> m_vCtrl{};
	Align m_eAlign = Align::Near;
	BOOL m_bHasFillSizeCtrl = FALSE;

	static void GetCtrlSize(const CTRL& e, int& cx, int& cy)
	{
		cx = 0, cy = 0;
		if (e.uFlags & (LLF_FIXHEIGHT | LLF_FIXWIDTH))
			cx = e.cx, cy = e.cy;
		else
		{
			e.pCtrl->LoGetAppropriateSize(cx, cy);
			if (e.uFlags & LLF_FIXHEIGHT)
				cy = e.cy;
			else if (e.uFlags & LLF_FIXWIDTH)
				cx = e.cx;
		}
	}
};

class CLinearLayoutV :public CLinearLayout
{
public:
	void LoCommit() override
	{
		const HDWP hDwpParent = (m_pParent ? m_pParent->LoGetCurrHDWP() : NULL);
		HDWP hDwp = (hDwpParent ? hDwpParent : BeginDeferWindowPos((int)m_vCtrl.size()));

		int x, y = m_y, cxAppr, cyAppr;
		int cyLeave{};
		UINT uTotalWeight{};
		if (m_bHasFillSizeCtrl)
		{
			cyLeave = m_cy;
			for (const auto& e : m_vCtrl)
			{
				cyLeave -= (e.Margin.cyTopHeight + e.Margin.cyBottomHeight);
				if (!(e.uFlags & LLF_FILLHEIGHT))
				{
					GetCtrlSize(e, cxAppr, cyAppr);
					cyLeave -= cyAppr;
				}
				else
					uTotalWeight += e.uWeight;
			}
		}

		for (const auto& e : m_vCtrl)
		{
			if (!IsBitSet(e.uFlags, (LLF_FILLWIDTH | LLF_FILLHEIGHT)))
				GetCtrlSize(e, cxAppr, cyAppr);
			if (e.uFlags & LLF_FILLHEIGHT)
				cyAppr = cyLeave * e.uWeight / uTotalWeight;
			if (e.uFlags & LLF_FILLWIDTH)
				cxAppr = m_cx - e.Margin.cxLeftWidth - e.Margin.cxRightWidth;

			switch (m_eAlign)
			{
			case Align::Near:
				x = e.Margin.cxLeftWidth;
				break;
			case Align::Center:
				x = e.Margin.cxLeftWidth +
					m_x +
					(m_cx - cxAppr +
						e.Margin.cxLeftWidth + e.Margin.cxRightWidth) / 2;
				break;
			case Align::Far:
				x = m_x + m_cx - cxAppr - e.Margin.cxRightWidth - e.Margin.cxLeftWidth;
				break;
			default:
				ECK_UNREACHABLE;
			}

			x += m_x;
			y += e.Margin.cyTopHeight;

			const auto hWnd = e.pCtrl->LoGetHWND();
			if (hWnd)
			{
				hDwp = DeferWindowPos(hDwp, hWnd, NULL,
					x, y, cxAppr, cyAppr, SWP_NOZORDER | SWP_NOACTIVATE);
			}
			else
			{
				e.pCtrl->LoSetPosSize(x, y, cxAppr, cyAppr);
				e.pCtrl->LoCommit();
			}
			y += (cyAppr + e.Margin.cyBottomHeight);
		}
		if (!hDwpParent)
			EndDeferWindowPos(hDwp);
	}

	void LoGetAppropriateSize(int& cx_, int& cy_) override
	{
		int cx{}, cy{}, cxAppr, cyAppr;

		for (const auto& e : m_vCtrl)
		{
			GetCtrlSize(e, cxAppr, cyAppr);
			cx = std::max(cx, cxAppr + e.Margin.cxLeftWidth + e.Margin.cxRightWidth);
			cy += (e.Margin.cyTopHeight + cyAppr + e.Margin.cyBottomHeight);
		}
		cx_ = cx;
		cy_ = cy;
	}

	void Add(ILayout* pCtrl, const MARGINS& Margin = {}, UINT uFlags = 0u, UINT uWeight = 0u)
	{
		pCtrl->LoSetParent(this);
		const auto size = pCtrl->LoGetSize();
		m_vCtrl.emplace_back(pCtrl, Margin, uFlags, (short)size.first, (short)size.second, uWeight);
		if (uFlags & (LLF_FILLWIDTH | LLF_FILLWIDTH))
			m_bHasFillSizeCtrl = TRUE;
		m_cy += (size.second + Margin.cyTopHeight + Margin.cyBottomHeight);
		m_cx = std::max(m_cx, size.first);
	}
};

class CLinearLayoutH :public CLinearLayout
{
public:
	void LoCommit() override
	{
		const HDWP hDwpParent = (m_pParent ? m_pParent->LoGetCurrHDWP() : NULL);
		HDWP hDwp = (hDwpParent ? hDwpParent : BeginDeferWindowPos((int)m_vCtrl.size()));

		int x = m_x, y, cxAppr, cyAppr;
		int cxLeave{};
		UINT uTotalWeight{};
		if (m_bHasFillSizeCtrl)
		{
			cxLeave = m_cx;
			for (const auto& e : m_vCtrl)
			{
				cxLeave -= (e.Margin.cxLeftWidth + e.Margin.cxRightWidth);
				if (!(e.uFlags & LLF_FILLWIDTH))
				{
					GetCtrlSize(e, cxAppr, cyAppr);
					cxLeave -= cxAppr;
				}
				else
					uTotalWeight += e.uWeight;
			}
		}
		for (const auto& e : m_vCtrl)
		{
			if (!IsBitSet(e.uFlags, (LLF_FILLWIDTH | LLF_FILLHEIGHT)))
				GetCtrlSize(e, cxAppr, cyAppr);
			if (e.uFlags & LLF_FILLHEIGHT)
				cyAppr = m_cy - e.Margin.cyTopHeight - e.Margin.cyBottomHeight;
			if (e.uFlags & LLF_FILLWIDTH)
				cxAppr = cxLeave * e.uWeight / uTotalWeight;

			switch (m_eAlign)
			{
			case Align::Near:
				y = e.Margin.cyTopHeight;
				break;
			case Align::Center:
				y = e.Margin.cyTopHeight +
					m_y +
					(m_cy - cyAppr +
						e.Margin.cyTopHeight + e.Margin.cyBottomHeight) / 2;
				break;
			case Align::Far:
				y = m_y + m_cy - cyAppr - e.Margin.cyTopHeight - e.Margin.cyBottomHeight;
				break;
			default:
				ECK_UNREACHABLE;
			}

			x += e.Margin.cxLeftWidth;
			y += m_y;

			const auto hWnd = e.pCtrl->LoGetHWND();
			if (hWnd)
			{
				hDwp = DeferWindowPos(hDwp, hWnd, NULL,
					x, y, cxAppr, cyAppr, SWP_NOZORDER | SWP_NOACTIVATE);
			}
			else
			{
				e.pCtrl->LoSetPosSize(x, y, cxAppr, cyAppr);
				e.pCtrl->LoCommit();
			}
			x += (cxAppr + e.Margin.cxRightWidth);
		}
		if (!hDwpParent)
			EndDeferWindowPos(hDwp);
	}

	void LoGetAppropriateSize(int& cx_, int& cy_) override
	{
		int cx{}, cy{}, cxAppr, cyAppr;

		EckCounter(m_vCtrl.size(), i)
		{
			const auto& e = m_vCtrl[i];
			if (e.uFlags & (LLF_FIXHEIGHT | LLF_FIXWIDTH))
				cxAppr = e.cx, cyAppr = e.cy;
			else
			{
				e.pCtrl->LoGetAppropriateSize(cxAppr, cyAppr);
				if (e.uFlags & LLF_FIXHEIGHT)
					cyAppr = e.cy;
				else if (e.uFlags & LLF_FIXWIDTH)
					cxAppr = e.cx;
			}

			cy = std::max(cy, cyAppr + e.Margin.cyTopHeight + e.Margin.cyBottomHeight);
			cx += (e.Margin.cxLeftWidth + cxAppr + e.Margin.cxRightWidth);
		}
		cx_ = cx;
		cy_ = cy;
	}

	void Add(ILayout* pCtrl, const MARGINS& Margin = {}, UINT uFlags = 0u, UINT uWeight = 0u)
	{
		pCtrl->LoSetParent(this);
		const auto size = pCtrl->LoGetSize();
		m_vCtrl.emplace_back(pCtrl, Margin, uFlags, (short)size.first, (short)size.second, uWeight);
		if (uFlags & (LLF_FILLWIDTH | LLF_FILLWIDTH))
			m_bHasFillSizeCtrl = TRUE;
		m_cx += (size.first + Margin.cxLeftWidth + Margin.cxRightWidth);
		m_cy = std::max(m_cy, size.second);
	}
};
ECK_NAMESPACE_END