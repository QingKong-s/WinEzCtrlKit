/*
* WinEzCtrlKit Library
*
* CLayoutBase.h ： 布局基类
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "ILayout.h"

ECK_NAMESPACE_BEGIN
enum :UINT
{
	// 对齐选项，低4位
	LF_ALIGN_LT,// 左上
	LF_ALIGN_T,	// 上
	LF_ALIGN_RT,// 右上
	LF_ALIGN_L,	// 左
	LF_ALIGN_C,	// 中
	LF_ALIGN_R,	// 右
	LF_ALIGN_LB,// 左下
	LF_ALIGN_B,	// 下
	LF_ALIGN_RB,// 右下

	// 下面三个标志供单向对齐使用
	LF_ALIGN_NEAR = 0,	// 近
	LF_ALIGN_CENTER,	// 中
	LF_ALIGN_FAR,		// 远

	LF_FILL_WIDTH = 1u << 4,	// 水平充满
	LF_FILL_HEIGHT = 1u << 5,	// 垂直充满
	LF_FILL = LF_FILL_WIDTH | LF_FILL_HEIGHT,	// 【结合标志】充满
	LF_FIT = 1u << 6,			// 等比例缩放以适应
	LF_FIX_WIDTH = 1u << 7,		// 固定宽度
	LF_FIX_HEIGHT = 1u << 8,	// 固定高度
	LF_FIX = LF_FIX_WIDTH | LF_FIX_HEIGHT,		// 【结合标志】固定尺寸
};

// 低15位为通用标志
constexpr inline int LayoutFlagsBitsBegin{ 15 };

class CLayoutBase :public ILayout
{
protected:
	struct ITEMBASE
	{
		ILayout* pCtrl;
		MARGINS Margin;
		UINT uFlags;
		short cx;
		short cy;
	};

	int m_x{},
		m_y{},
		m_cx{},
		m_cy{};
	int m_iDpi{ USER_DEFAULT_SCREEN_DPI };
	BOOL m_bUseDwp{ TRUE };
#ifdef _DEBUG
	BOOL m_bPrePostBalanced{};
#endif

	static void CalcCtrlPosSize(const ITEMBASE& e, const RCWH& cell, 
		int& x, int& y, int& cx, int& cy)
	{
		if (e.uFlags & LF_FIT)
		{
			RECT rc{ 0,0,e.cx,e.cy };
			const RECT rcRef
			{
				cell.x + e.Margin.cxLeftWidth,
				cell.y + e.Margin.cyTopHeight,
				cell.x + cell.cx - e.Margin.cxRightWidth,
				cell.y + cell.cy - e.Margin.cyBottomHeight
			};
			AdjustRectToFitAnother(rc, rcRef);
			x = rc.left;
			y = rc.top;
			cx = rc.right - rc.left;
			cy = rc.bottom - rc.top;
		}
		else if (IsBitSet(e.uFlags, LF_FILL))
		{
			x = cell.x + e.Margin.cxLeftWidth;
			y = cell.y + e.Margin.cyTopHeight;
			cx = cell.cx - e.Margin.cxLeftWidth - e.Margin.cxRightWidth;
			cy = cell.cy - e.Margin.cyTopHeight - e.Margin.cyBottomHeight;
		}
		else
		{
			if (IsBitSet(e.uFlags, LF_FIX))
			{
				cx = e.cx;
				cy = e.cy;
			}
			else
			{
				e.pCtrl->LoGetAppropriateSize(cx, cy);
				if (e.uFlags & LF_FIX_WIDTH)
					cx = e.cx;
				if (e.uFlags & LF_FIX_HEIGHT)
					cy = e.cy;
			}

			const int cx1 = cx + e.Margin.cxLeftWidth + e.Margin.cxRightWidth;
			const int cy1 = cy + e.Margin.cyTopHeight + e.Margin.cyBottomHeight;
			switch (GetAlignFromFlags(e.uFlags))
			{
			case LF_ALIGN_LT:
				x = cell.x + e.Margin.cxLeftWidth;
				y = cell.y + e.Margin.cyTopHeight;
				break;
			case LF_ALIGN_T:
				x = cell.x + (cell.cx - cx1) / 2 + e.Margin.cxLeftWidth;
				y = cell.y + e.Margin.cyTopHeight;
				break;
			case LF_ALIGN_RT:
				x = cell.x + cell.cx - cx - e.Margin.cxRightWidth;
				y = cell.y + e.Margin.cyTopHeight;
				break;
			case LF_ALIGN_L:
				x = cell.x + e.Margin.cxLeftWidth;
				y = cell.y + (cell.cy - (cy + e.Margin.cyTopHeight + e.Margin.cyBottomHeight)) / 2 + e.Margin.cyTopHeight;
				break;
			case LF_ALIGN_C:
				x = cell.x + (cell.cx - cx1) / 2 + e.Margin.cxLeftWidth;
				y = cell.y + (cell.cy - (cy + e.Margin.cyTopHeight + e.Margin.cyBottomHeight)) / 2 + e.Margin.cyTopHeight;
				break;
			case LF_ALIGN_R:
				x = cell.x + cell.cx - cx - e.Margin.cxRightWidth;
				y = cell.y + (cell.cy - (cy + e.Margin.cyTopHeight + e.Margin.cyBottomHeight)) / 2 + e.Margin.cyTopHeight;
				break;
			case LF_ALIGN_LB:
				x = cell.x + e.Margin.cxLeftWidth;
				y = cell.y + cell.cy - cy - e.Margin.cyBottomHeight;
				break;
			case LF_ALIGN_B:
				x = cell.x + (cell.cx - cx1) / 2 + e.Margin.cxLeftWidth;
				y = cell.y + cell.cy - cy - e.Margin.cyBottomHeight;
				break;
			case LF_ALIGN_RB:
				x = cell.x + cell.cx - cx - e.Margin.cxRightWidth;
				y = cell.y + cell.cy - cy - e.Margin.cyBottomHeight;
				break;
			default:
				ECK_UNREACHABLE;
			}

			if (e.uFlags & LF_FILL_WIDTH)
			{
				x = cell.x + e.Margin.cxLeftWidth;
				cx = cell.cx - e.Margin.cxLeftWidth - e.Margin.cxRightWidth;
			}
			if (e.uFlags & LF_FILL_HEIGHT)
			{
				y = cell.y + e.Margin.cyTopHeight;
				cy = cell.cy - e.Margin.cyTopHeight - e.Margin.cyBottomHeight;
			}
		}
	}

	static void MoveCtrlPosSize(const ITEMBASE& e, HDWP& hDwp, int x, int y, int cx, int cy)
	{
		if (const auto hWnd = e.pCtrl->LoGetHWND())
			if (IsBitSet(e.uFlags, LF_FIX))
				hDwp = DeferWindowPos(hDwp, hWnd, NULL, x, y, 0, 0,
					SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
			else
				hDwp = DeferWindowPos(hDwp, hWnd, NULL, x, y, cx, cy, SWP_NOACTIVATE | SWP_NOZORDER);
		else
		{
			if (IsBitSet(e.uFlags, LF_FIX))
				e.pCtrl->LoSetPos(x, y);
			else
				e.pCtrl->LoSetPosSize(x, y, cx, cy);
			e.pCtrl->LoCommit();
		}
	}

	EckInline constexpr void ReCalcDpiSize(ITEMBASE& e, int iDpiNew)
	{
		e.cx = DpiScale(e.cx, iDpiNew, m_iDpi);
		e.cy = DpiScale(e.cy, iDpiNew, m_iDpi);
		e.Margin.cxLeftWidth = DpiScale(e.Margin.cxLeftWidth, iDpiNew, m_iDpi);
		e.Margin.cyTopHeight = DpiScale(e.Margin.cyTopHeight, iDpiNew, m_iDpi);
		e.Margin.cxRightWidth = DpiScale(e.Margin.cxRightWidth, iDpiNew, m_iDpi);
		e.Margin.cyBottomHeight = DpiScale(e.Margin.cyBottomHeight, iDpiNew, m_iDpi);
	}
public:
	EckInline static constexpr UINT GetAlignFromFlags(UINT u)
	{
		return GetLowNBits(u, 4);
	}

	EckInline static constexpr UINT GetSingleAlignFromFlags(UINT u)
	{
		return GetLowNBits(u, 2);
	}

	EckInline static constexpr UINT GetCommFlagsFromFlags(UINT u)
	{
		return GetLowNBits(u, 9);
	}

	void LoSetPos(int x, int y) override
	{
		m_x = x;
		m_y = y;
	}

	void LoSetSize(int cx, int cy) override
	{
		m_cx = cx;
		m_cy = cy;
	}

	void LoSetPosSize(int x, int y, int cx, int cy) override
	{
		m_x = x;
		m_y = y;
		m_cx = cx;
		m_cy = cy;
	}

	std::pair<int, int> LoGetPos() override
	{
		return { m_x,m_y };
	}

	std::pair<int, int> LoGetSize() override
	{
		return { m_cx,m_cy };
	}

	void LoShow(BOOL bShow) override {}

	void LoGetAppropriateSize(int& cx, int& cy) override
	{
		cx = m_cx;
		cy = m_cy;
	}

	/// <summary>
	/// 重置布局对象的状态
	/// </summary>
	virtual void Clear()
	{
		m_x = m_y = m_cx = m_cy = 0;
		m_bUseDwp = TRUE;
	}

	/// <summary>
	/// 强制刷新布局对象信息
	/// </summary>
	virtual void Refresh() {}

	EckInline void Arrange(int cx, int cy)
	{
		CLayoutBase::LoSetPosSize(0, 0, cx, cy);
		LoCommit();
	}

	EckInline void Arrange(int x, int y, int cx, int cy)
	{
		CLayoutBase::LoSetPosSize(x, y, cx, cy);
		LoCommit();
	}

	EckInline constexpr void SetUseDwp(BOOL b) { m_bUseDwp = b; }

	EckInline constexpr BOOL GetUseDwp() const { return m_bUseDwp; }

	HDWP PreArrange(size_t cWindows = 2u)
	{
#ifdef _DEBUG
		EckAssert(!m_bPrePostBalanced);
		m_bPrePostBalanced = TRUE;
#endif
		if (m_bUseDwp)
			return BeginDeferWindowPos((int)cWindows);
		else
			return NULL;
	}

	void PostArrange(HDWP hDwp)
	{
#ifdef _DEBUG
		EckAssert(m_bPrePostBalanced);
		m_bPrePostBalanced = FALSE;
#endif
		EndDeferWindowPos(hDwp);
	}

	EckInline constexpr int GetDpi() const { return m_iDpi; }
};
ECK_NAMESPACE_END