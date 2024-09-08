/*
* WinEzCtrlKit Library
*
* CFlowLayout.h ： 流式布局
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "CLayoutBase.h"

ECK_NAMESPACE_BEGIN
enum :UINT
{
	// 在前方插入换行
	FLF_BREAKLINE = 1u << 15,
};

class CFlowLayoutBase :public CLayoutBase
{
protected:
	struct ITEM :public ITEMBASE
	{
		RCWH rcPos;

		ITEM() = default;
		constexpr ITEM(ILayout* pCtrl, const MARGINS& Margin, UINT uFlags, short cx, short cy)
			: ITEMBASE{ pCtrl, Margin, uFlags, cx, cy } {}
	};

	std::vector<ITEM> m_vItem{};
public:
	EckInline size_t Add(ILayout* pCtrl, const MARGINS& Margin = {}, const UINT uFlags = 0)
	{
		const auto size = pCtrl->LoGetSize();
		m_vItem.emplace_back(pCtrl, Margin, uFlags, (short)size.first, (short)size.second);
		return m_vItem.size() - 1;
	}

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

	void Refresh() override
	{
		for (auto& e : m_vItem)
		{
			const auto size = e.pCtrl->LoGetSize();
			e.cx = (short)size.first;
			e.cy = (short)size.second;
		}
	}

	void Clear() override
	{
		CLayoutBase::Clear();
		m_vItem.clear();
	}

	EckInline constexpr auto& GetList() { return m_vItem; }
};

class CFlowLayoutH final :public CFlowLayoutBase
{
private:
	void MoveCtrl(ITEM& e, HDWP& hDwp, int cyLineMax, int y)
	{
		if (e.uFlags & LF_FIX_HEIGHT)
			switch (GetSingleAlignFromFlags(e.uFlags))
			{
			case LF_ALIGN_NEAR:
				e.rcPos.y = y;
				break;
			case LF_ALIGN_CENTER:
				e.rcPos.y = y + (cyLineMax - (e.rcPos.cy + e.Margin.cyTopHeight + e.Margin.cyBottomHeight)) / 2;
				break;
			case LF_ALIGN_FAR:
				e.rcPos.y = y + cyLineMax - e.rcPos.cy - e.Margin.cyBottomHeight;
				break;
			default:
				ECK_UNREACHABLE;
			}
		else if (e.uFlags & LF_FILL_HEIGHT)
		{
			e.rcPos.y = y + e.Margin.cyTopHeight;
			e.rcPos.cy = cyLineMax - e.Margin.cyBottomHeight - e.Margin.cyTopHeight;
		}

		if (const auto hWnd = e.pCtrl->LoGetHWND())
		{
			if (e.uFlags & LF_FIX_HEIGHT)
				hDwp = DeferWindowPos(hDwp, hWnd, NULL, e.rcPos.x, e.rcPos.y, 0, 0,
					SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
			else
				hDwp = DeferWindowPos(hDwp, hWnd, NULL, e.rcPos.x, e.rcPos.y, e.rcPos.cx, e.rcPos.cy,
					SWP_NOZORDER | SWP_NOACTIVATE);
		}
		else
		{
			if (e.uFlags & LF_FIX_HEIGHT)
				e.pCtrl->LoSetPos(e.rcPos.x, e.rcPos.y);
			else
				e.pCtrl->LoSetPosSize(e.rcPos.x, e.rcPos.y, e.rcPos.cx, e.rcPos.cy);
			e.pCtrl->LoCommit();
		}
	}
public:
	void LoCommit() override
	{
		HDWP hDwp = PreArrange(m_vItem.size());
		int x = m_x;
		int y = m_y;
		int cxAppr, cyAppr;
		int cyLineMax = 0;
		int idxInLine = 0;
		EckCounter(m_vItem.size(), i)
		{
			auto& e = m_vItem[i];
			if (IsBitSet(e.uFlags, LF_FIX))
			{
				cxAppr = e.cx;
				cyAppr = e.cy;
			}
			else
			{
				e.pCtrl->LoGetAppropriateSize(cxAppr, cyAppr);
				if (e.uFlags & LF_FIX_WIDTH)
					cxAppr = e.cx;
				if (e.uFlags & LF_FIX_HEIGHT)
					cyAppr = e.cy;
			}

			const auto xt = x + e.Margin.cxLeftWidth + e.Margin.cxRightWidth + cxAppr;
			if (xt <= m_x + m_cx || (e.uFlags & FLF_BREAKLINE) ||
				idxInLine == 0/*无论有没有空间，至少要放下一个控件*/)
			{
				const int cyReal = cyAppr + e.Margin.cyTopHeight + e.Margin.cyBottomHeight;
				if (cyReal > cyLineMax)
					cyLineMax = cyReal;
				e.rcPos.x = x + e.Margin.cxLeftWidth;
				e.rcPos.cx = cxAppr;
				e.rcPos.cy = cyAppr;
				++idxInLine;
				x = xt;
			}
			else// 开始新行
			{
				// 归位上一行
				for (size_t j = i - idxInLine; j < i; ++j)
					MoveCtrl(m_vItem[j], hDwp, cyLineMax, y);

				// 重置行参数
				x = m_x;
				y += cyLineMax;
				cyLineMax = 0;
				idxInLine = 0;

				// 本行第一个控件
				const int cyReal = cyAppr + e.Margin.cyTopHeight + e.Margin.cyBottomHeight;
				if (cyReal > cyLineMax)
					cyLineMax = cyReal;
				e.rcPos.x = x + e.Margin.cxLeftWidth;
				e.rcPos.cx = cxAppr;
				e.rcPos.cy = cyAppr;
				++idxInLine;
				x += (e.Margin.cxLeftWidth + e.Margin.cxRightWidth + cxAppr);
			}
		}
		for (size_t j = m_vItem.size() - idxInLine; j < m_vItem.size(); ++j)
			MoveCtrl(m_vItem[j], hDwp, cyLineMax, y);
		PostArrange(hDwp);
	}

	void LoGetAppropriateSize(int& cx, int& cy) override
	{
		cx = 0;
		cy = 0;
		for (const auto& e : m_vItem)
		{
			cx += e.cx + e.Margin.cxLeftWidth + e.Margin.cxRightWidth;
			cy = std::max(cy, e.cy + e.Margin.cyTopHeight + e.Margin.cyBottomHeight);
		}
	}
};

class CFlowLayoutV final :public CFlowLayoutBase
{
private:
	void MoveCtrl(ITEM& e, HDWP& hDwp, int cxLineMax, int x)
	{
		if (e.uFlags & LF_FIX_WIDTH)
			switch (GetSingleAlignFromFlags(e.uFlags))
			{
			case LF_ALIGN_NEAR:
				e.rcPos.x = x;
				break;
			case LF_ALIGN_CENTER:
				e.rcPos.x = x + (cxLineMax - (e.rcPos.cx + e.Margin.cxLeftWidth + e.Margin.cxRightWidth)) / 2;
				break;
			case LF_ALIGN_FAR:
				e.rcPos.x = x + cxLineMax - e.rcPos.cx - e.Margin.cxRightWidth;
				break;
			default:
				ECK_UNREACHABLE;
			}
		else if (e.uFlags & LF_FILL_WIDTH)
		{
			e.rcPos.x = x + e.Margin.cxLeftWidth;
			e.rcPos.cx = cxLineMax - e.Margin.cxLeftWidth - e.Margin.cxRightWidth;
		}

		if (const auto hWnd = e.pCtrl->LoGetHWND())
		{
			if (e.uFlags & LF_FIX_WIDTH)
				hDwp = DeferWindowPos(hDwp, hWnd, NULL, e.rcPos.x, e.rcPos.y, 0, 0,
					SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
			else
				hDwp = DeferWindowPos(hDwp, hWnd, NULL, e.rcPos.x, e.rcPos.y, e.rcPos.cx, e.rcPos.cy,
					SWP_NOZORDER | SWP_NOACTIVATE);
		}
		else
		{
			if (e.uFlags & LF_FIX_WIDTH)
				e.pCtrl->LoSetPos(e.rcPos.x, e.rcPos.y);
			else
				e.pCtrl->LoSetPosSize(e.rcPos.x, e.rcPos.y, e.rcPos.cx, e.rcPos.cy);
			e.pCtrl->LoCommit();
		}
	}
public:
	void LoCommit() override
	{
		HDWP hDwp = PreArrange(m_vItem.size());
		int x = m_x;
		int y = m_y;
		int cxAppr, cyAppr;
		int cxLineMax = 0;
		int idxInLine = 0;
		EckCounter(m_vItem.size(), i)
		{
			auto& e = m_vItem[i];
			if (IsBitSet(e.uFlags, LF_FIX))
			{
				cxAppr = e.cx;
				cyAppr = e.cy;
			}
			else
			{
				e.pCtrl->LoGetAppropriateSize(cxAppr, cyAppr);
				if (e.uFlags & LF_FIX_WIDTH)
					cxAppr = e.cx;
				if (e.uFlags & LF_FIX_HEIGHT)
					cyAppr = e.cy;
			}

			const auto yt = y + e.Margin.cyTopHeight + e.Margin.cyBottomHeight + cyAppr;
			if (yt <= m_y + m_cy || (e.uFlags & FLF_BREAKLINE) ||
				idxInLine == 0/*无论有没有空间，至少要放下一个控件*/)
			{
				const int cxReal = cxAppr + e.Margin.cxLeftWidth + e.Margin.cxRightWidth;
				if (cxReal > cxLineMax)
					cxLineMax = cxReal;
				e.rcPos.y = y + e.Margin.cyTopHeight;
				e.rcPos.cy = cyAppr;
				e.rcPos.cx = cxAppr;
				++idxInLine;
				y = yt;
			}
			else// 开始新行
			{
				// 归位上一行
				for (size_t j = i - idxInLine; j < i; ++j)
					MoveCtrl(m_vItem[j], hDwp, cxLineMax, x);

				// 重置行参数
				y = m_y;
				x += cxLineMax;
				cxLineMax = 0;
				idxInLine = 0;

				// 本行第一个控件
				const int cxReal = cxAppr + e.Margin.cxLeftWidth + e.Margin.cxRightWidth;
				if (cxReal > cxLineMax)
					cxLineMax = cxReal;
				e.rcPos.y = y + e.Margin.cyTopHeight;
				e.rcPos.cy = cyAppr;
				e.rcPos.cx = cxAppr;
				++idxInLine;
				y += (e.Margin.cyTopHeight + e.Margin.cyBottomHeight + cyAppr);
			}
		}
		for (size_t j = m_vItem.size() - idxInLine; j < m_vItem.size(); ++j)
			MoveCtrl(m_vItem[j], hDwp, cxLineMax, x);
		PostArrange(hDwp);
	}

	void LoGetAppropriateSize(int& cx, int& cy) override
	{
		cx = 0;
		cy = 0;
		for (const auto& e : m_vItem)
		{
			cx = std::max(cx, e.cx + e.Margin.cxLeftWidth + e.Margin.cxRightWidth);
			cy += e.cy + e.Margin.cyTopHeight + e.Margin.cyBottomHeight;
		}
	}
};
ECK_NAMESPACE_END