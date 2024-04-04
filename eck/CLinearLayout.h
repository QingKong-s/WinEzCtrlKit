#pragma once
#include "CLayoutBase.h"

ECK_NAMESPACE_BEGIN
enum
{
	// 固定宽度
	LLF_FIXWIDTH = 1u << 0,
	// 固定高度
	LLF_FIXHEIGHT = 1u << 1,
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
	};

	std::vector<CTRL> m_vCtrl{};
	Align m_eAlign = Align::Near;
public:
	void Add(ILayout* pCtrl, const MARGINS& Margin = {}, UINT uFlags = 0u)
	{
		pCtrl->LoSetParent(this);
		const auto size = pCtrl->LoGetSize();
		m_vCtrl.emplace_back(pCtrl, Margin, uFlags, (short)size.first, (short)size.second);
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
				__assume(0);
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
			cx = std::max(cx, cxAppr + e.Margin.cxLeftWidth + e.Margin.cxRightWidth);
			cy += (e.Margin.cyTopHeight + cyAppr + e.Margin.cyBottomHeight);
		}
		cx_ = cx;
		cy_ = cy;
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
				__assume(0);
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
};
ECK_NAMESPACE_END