/*
* WinEzCtrlKit Library
*
* CFrameLayout.h ： 帧布局
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "CLayoutBase.h"
#include "Utility.h"

enum
{
	// 对齐选项，低4位
	FLF_ALIGN_LT,	// 左上
	FLF_ALIGN_T,	// 上
	FLF_ALIGN_RT,	// 右上
	FLF_ALIGN_L,	// 左
	FLF_ALIGN_C,	// 中
	FLF_ALIGN_R,	// 右
	FLF_ALIGN_LB,	// 左下
	FLF_ALIGN_B,	// 下
	FLF_ALIGN_RB,	// 右下

	FLF_FILL_WIDTH = 1u << 4,	// 水平充满
	FLF_FILL_HEIGHT = 1u << 5,	// 垂直充满
	FLF_FILL = FLF_FILL_WIDTH | FLF_FILL_HEIGHT,	// 充满
	FLF_FIT = 1u << 6,			// 等比例缩放以适应
};

ECK_NAMESPACE_BEGIN
class CFrameLayout : public CLayoutBase
{
private:
	struct ITEM
	{
		ILayout* pCtrl;
		MARGINS Margin;
		UINT uFlags;
		RECT rcOrg;// 左顶宽高
	};

	std::vector<ITEM> m_vCtrl{};
public:
	void Add(ILayout* pCtrl, const MARGINS& Margin = {}, UINT uFlags = 0u)
	{
		const auto pt = pCtrl->LoGetPos();
		const auto size = pCtrl->LoGetSize();
		const RECT rcOrg{ pt.first, pt.second, size.first, size.second };
		m_vCtrl.emplace_back(pCtrl, Margin, uFlags, rcOrg);
	}

	void LoCommit() override
	{
		HDWP hDwp = PreArrange((int)m_vCtrl.size());
		for (const auto& e : m_vCtrl)
		{
			const auto Align = GetLowNBits(e.uFlags, 4);
			int x, y;
			switch (Align)
			{
			case FLF_ALIGN_LT:
				x = m_x + e.Margin.cxLeftWidth;
				y = m_y + e.Margin.cyTopHeight;
				break;
			case FLF_ALIGN_T:
				x = m_x + (m_cx - e.rcOrg.right) / 2;
				y = m_y + e.Margin.cyTopHeight;
				break;
			case FLF_ALIGN_RT:
				x = m_x + m_cx - e.rcOrg.right - e.Margin.cxRightWidth;
				y = m_y + e.Margin.cyTopHeight;
				break;
			case FLF_ALIGN_L:
				x = m_x + e.Margin.cxLeftWidth;
				y = m_y + (m_cy - e.rcOrg.bottom) / 2;
				break;
			case FLF_ALIGN_C:
				x = m_x + (m_cx - e.rcOrg.right) / 2;
				y = m_y + (m_cy - e.rcOrg.bottom) / 2;
				break;
			case FLF_ALIGN_R:
				x = m_x + m_cx - e.rcOrg.right - e.Margin.cxRightWidth;
				y = m_y + (m_cy - e.rcOrg.bottom) / 2;
				break;
			case FLF_ALIGN_LB:
				x = m_x + e.Margin.cxLeftWidth;
				y = m_y + m_cy - e.rcOrg.bottom - e.Margin.cyBottomHeight;
				break;
			case FLF_ALIGN_B:
				x = m_x + (m_cx - e.rcOrg.right) / 2;
				y = m_y + m_cy - e.rcOrg.bottom - e.Margin.cyBottomHeight;
				break;
			case FLF_ALIGN_RB:
				x = m_x + m_cx - e.rcOrg.right - e.Margin.cxRightWidth;
				y = m_y + m_cy - e.rcOrg.bottom - e.Margin.cyBottomHeight;
				break;
			default:
				x = m_x + e.Margin.cxLeftWidth;
				y = m_y + e.Margin.cyTopHeight;
				break;
			}

			int cx{ INT_MIN }, cy{ INT_MIN };
			if (e.uFlags & FLF_FIT)
			{
				RECT rc{ 0,0,e.rcOrg.right,e.rcOrg.bottom };
				const RECT rcRef
				{
					m_x + e.Margin.cxLeftWidth,
					m_y + e.Margin.cyTopHeight,
					m_x + m_cx - e.Margin.cxRightWidth,
					m_y + m_cy - e.Margin.cyBottomHeight
				};
				AdjustRectToFitAnother(rc, rcRef);
				x = rc.left;
				y = rc.top;
				cx = rc.right - rc.left;
				cy = rc.bottom - rc.top;
			}
			else
			{
				if (e.uFlags & FLF_FILL_WIDTH)
					cx = m_cx - e.Margin.cxLeftWidth - e.Margin.cxRightWidth;
				if (e.uFlags & FLF_FILL_HEIGHT)
					cy = m_cy - e.Margin.cyTopHeight - e.Margin.cyBottomHeight;
			}

			if (cx != INT_MIN && cy == INT_MIN)
				cy = e.rcOrg.bottom;
			if (cx == INT_MIN && cy != INT_MIN)
				cx = e.rcOrg.right;

			if (const auto hWnd = e.pCtrl->LoGetHWND())
			{
				if (cx == INT_MIN)
					hDwp = DeferWindowPos(hDwp, hWnd, NULL,
						x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
				else
					hDwp = DeferWindowPos(hDwp, hWnd, NULL,
						x, y, cx, cy, SWP_NOZORDER | SWP_NOACTIVATE);
			}
			else
			{
				e.pCtrl->LoSetPos(x, y);
				if (cx != INT_MIN)
					e.pCtrl->LoSetSize(cx, cy);
				e.pCtrl->LoCommit();
			}
		}
		PostArrange(hDwp);
	}

	void ShowFrame(int idx)
	{
		EckAssert(idx >= 0 && idx < (int)m_vCtrl.size());
		int i{};
		m_vCtrl[idx].pCtrl->LoShow(TRUE);
		for (; i < idx; ++i)
			m_vCtrl[i].pCtrl->LoShow(FALSE);
		for (i = idx + 1; i < (int)m_vCtrl.size(); ++i)
			m_vCtrl[i].pCtrl->LoShow(FALSE);
	}
};
ECK_NAMESPACE_END