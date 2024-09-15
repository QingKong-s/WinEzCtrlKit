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
	// 不应使用这些标志，仅用作兼容
	
	// 固定宽度
	LLF_FIXWIDTH = LF_FIX_WIDTH,
	// 固定高度
	LLF_FIXHEIGHT = LF_FIX_HEIGHT,
	// 固定大小
	LLF_FIXSIZE = LF_FIX,

	// 宽度占据剩余部分
	LLF_FILLWIDTH = LF_FILL_WIDTH,
	// 高度占据剩余部分
	LLF_FILLHEIGHT = LF_FILL_HEIGHT,
	// 大小占据剩余部分
	LLF_FILLSIZE = LF_FILL,
};

class CLinearLayoutBase :public CLayoutBase
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
	BOOL m_bHasFillSizeCtrl = FALSE;

	static void GetCtrlSize(const ITEM& e, int& cx, int& cy)
	{
		cx = 0, cy = 0;
		if (IsBitSet(e.uFlags, LF_FIX))
			cx = e.cx, cy = e.cy;
		else
		{
			e.pCtrl->LoGetAppropriateSize(cx, cy);
			if (e.uFlags & LF_FIX_HEIGHT)
				cy = e.cy;
			else if (e.uFlags & LF_FIX_WIDTH)
				cx = e.cx;
		}
	}
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

	// 清除布局内容
	void Clear() override
	{
		CLayoutBase::Clear();
		m_vItem.clear();
		m_bHasFillSizeCtrl = FALSE;
	}

	// 取底层列表
	EckInline constexpr auto& GetList() { return m_vItem; }
};

class CLinearLayoutV final :public CLinearLayoutBase
{
public:
	void LoCommit() override
	{
		HDWP hDwp = PreArrange(m_vItem.size());

		int x, y = m_y, cxAppr, cyAppr;
		int cyLeave{};
		UINT uTotalWeight{};
		if (m_bHasFillSizeCtrl)
		{
			cyLeave = m_cy;
			for (const auto& e : m_vItem)
			{
				cyLeave -= (e.Margin.cyTopHeight + e.Margin.cyBottomHeight);
				if (!(e.uFlags & LF_FILL_HEIGHT))
				{
					GetCtrlSize(e, cxAppr, cyAppr);
					cyLeave -= cyAppr;
				}
				else
					uTotalWeight += e.uWeight;
			}
		}

		for (const auto& e : m_vItem)
		{
			if (!IsBitSet(e.uFlags, LF_FILL))
				GetCtrlSize(e, cxAppr, cyAppr);
			if (e.uFlags & LF_FILL_HEIGHT)
				cyAppr = cyLeave * e.uWeight / uTotalWeight;
			if (e.uFlags & LF_FILL_WIDTH)
				cxAppr = m_cx - e.Margin.cxLeftWidth - e.Margin.cxRightWidth;

			switch (GetSingleAlignFromFlags(e.uFlags))
			{
			case LF_ALIGN_NEAR:
				x = m_x + e.Margin.cxLeftWidth;
				break;
			case LF_ALIGN_CENTER:
				x = m_x + e.Margin.cxLeftWidth +
					(m_cx - (cxAppr + e.Margin.cxLeftWidth + e.Margin.cxRightWidth)) / 2;
				break;
			case LF_ALIGN_FAR:
				x = m_x + m_cx - cxAppr - e.Margin.cxRightWidth - e.Margin.cxLeftWidth;
				break;
			default:
				ECK_UNREACHABLE;
			}

			y += e.Margin.cyTopHeight;

			const auto hWnd = e.pCtrl->LoGetHWND();
			if (hWnd)
			{
				hDwp = DeferWindowPos(hDwp, hWnd, nullptr,
					x, y, cxAppr, cyAppr, SWP_NOZORDER | SWP_NOACTIVATE);
			}
			else
			{
				e.pCtrl->LoSetPosSize(x, y, cxAppr, cyAppr);
				e.pCtrl->LoCommit();
			}
			y += (cyAppr + e.Margin.cyBottomHeight);
		}

		PostArrange(hDwp);
	}

	void LoGetAppropriateSize(int& cx_, int& cy_) override
	{
		int cx{}, cy{}, cxAppr, cyAppr;

		for (const auto& e : m_vItem)
		{
			GetCtrlSize(e, cxAppr, cyAppr);
			cx = std::max(cx, cxAppr + e.Margin.cxLeftWidth + e.Margin.cxRightWidth);
			cy += (e.Margin.cyTopHeight + cyAppr + e.Margin.cyBottomHeight);
		}
		cx_ = cx;
		cy_ = cy;
	}

	size_t Add(ILayout* pCtrl, const MARGINS& Margin = {}, UINT uFlags = 0u, UINT uWeight = 0u)
	{
		const auto size = pCtrl->LoGetSize();
		m_vItem.emplace_back(pCtrl, Margin, uFlags, (short)size.first, (short)size.second, uWeight);
		if (uFlags & (LF_FILL_WIDTH | LF_FILL_HEIGHT))
			m_bHasFillSizeCtrl = TRUE;
		m_cx = std::max(m_cx, size.first + Margin.cxLeftWidth + Margin.cxRightWidth);
		m_cy += (size.second + Margin.cyTopHeight + Margin.cyBottomHeight);
		return m_vItem.size() - 1;
	}

	// 若使用底层列表修改布局，或固定控件大小已变化，调用此函数刷新
	void Refresh() override
	{
		m_cx = m_cy = 0;
		for (auto& e : m_vItem)
		{
			const auto size = e.pCtrl->LoGetSize();
			e.cx = (short)size.first;
			e.cy = (short)size.second;
			m_cx = std::max(m_cx, size.first + e.Margin.cxLeftWidth + e.Margin.cxRightWidth);
			m_cy += (size.second + e.Margin.cyTopHeight + e.Margin.cyBottomHeight);
		}
	}
};

class CLinearLayoutH final :public CLinearLayoutBase
{
public:
	void LoCommit() override
	{
		HDWP hDwp = PreArrange(m_vItem.size());

		int x = m_x, y, cxAppr, cyAppr;
		int cxLeave{};
		UINT uTotalWeight{};
		if (m_bHasFillSizeCtrl)
		{
			cxLeave = m_cx;
			for (const auto& e : m_vItem)
			{
				cxLeave -= (e.Margin.cxLeftWidth + e.Margin.cxRightWidth);
				if (!(e.uFlags & LF_FILL_WIDTH))
				{
					GetCtrlSize(e, cxAppr, cyAppr);
					cxLeave -= cxAppr;
				}
				else
					uTotalWeight += e.uWeight;
			}
		}

		for (const auto& e : m_vItem)
		{
			if (!IsBitSet(e.uFlags, LF_FILL))
				GetCtrlSize(e, cxAppr, cyAppr);
			if (e.uFlags & LF_FILL_HEIGHT)
				cyAppr = m_cy - e.Margin.cyTopHeight - e.Margin.cyBottomHeight;
			if (e.uFlags & LF_FILL_WIDTH)
				cxAppr = cxLeave * e.uWeight / uTotalWeight;

			switch (GetSingleAlignFromFlags(e.uFlags))
			{
			case LF_ALIGN_NEAR:
				y = m_y + e.Margin.cyTopHeight;
				break;
			case LF_ALIGN_CENTER:
				y = m_y + e.Margin.cyTopHeight +
					(m_cy - (cyAppr + e.Margin.cyTopHeight + e.Margin.cyBottomHeight)) / 2;
				break;
			case LF_ALIGN_FAR:
				y = m_y + m_cy - cyAppr - e.Margin.cyTopHeight - e.Margin.cyBottomHeight;
				break;
			default:
				ECK_UNREACHABLE;
			}

			x += e.Margin.cxLeftWidth;

			const auto hWnd = e.pCtrl->LoGetHWND();
			if (hWnd)
			{
				hDwp = DeferWindowPos(hDwp, hWnd, nullptr,
					x, y, cxAppr, cyAppr, SWP_NOZORDER | SWP_NOACTIVATE);
			}
			else
			{
				e.pCtrl->LoSetPosSize(x, y, cxAppr, cyAppr);
				e.pCtrl->LoCommit();
			}
			x += (cxAppr + e.Margin.cxRightWidth);
		}

		PostArrange(hDwp);
	}

	void LoGetAppropriateSize(int& cx_, int& cy_) override
	{
		int cx{}, cy{}, cxAppr, cyAppr;

		for (const auto& e : m_vItem)
		{
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

	size_t Add(ILayout* pCtrl, const MARGINS& Margin = {}, UINT uFlags = 0u, UINT uWeight = 0u)
	{
		const auto size = pCtrl->LoGetSize();
		m_vItem.emplace_back(pCtrl, Margin, uFlags, (short)size.first, (short)size.second, uWeight);
		if (uFlags & (LF_FILL_WIDTH | LF_FILL_HEIGHT))
			m_bHasFillSizeCtrl = TRUE;
		m_cx += (size.first + Margin.cxLeftWidth + Margin.cxRightWidth);
		m_cy = std::max(m_cy, size.second + Margin.cyTopHeight + Margin.cyBottomHeight);
		return m_vItem.size() - 1;
	}

	// 若使用底层列表修改布局，或固定控件大小已变化，调用此函数刷新
	void Refresh() override
	{
		m_cx = m_cy = 0;
		for (auto& e : m_vItem)
		{
			const auto size = e.pCtrl->LoGetSize();
			e.cx = (short)size.first;
			e.cy = (short)size.second;
			m_cx += (size.first + e.Margin.cxLeftWidth + e.Margin.cxRightWidth);
			m_cy = std::max(m_cy, size.second + e.Margin.cyTopHeight + e.Margin.cyBottomHeight);
		}
	}
};
ECK_NAMESPACE_END