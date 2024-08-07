﻿/*
* WinEzCtrlKit Library
*
* CFlowLayout.h ： 流式布局
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "CLayoutBase.h"

ECK_NAMESPACE_BEGIN
enum
{
	// 固定宽度
	FLF_FIXWIDTH = 1u << 0,
	// 固定高度
	FLF_FIXHEIGHT = 1u << 1,
	// 在前方插入换行
	FLF_BREAKLINE = 1u << 2,
	// 对齐到所在行的顶边
	FLF_ALIGNTOP = 1u << 3,
	// 对齐到所在行的中间，这是默认值
	FLF_ALIGNCENTER = 0,
	// 对齐到所在行的底边
	FLF_ALIGNBOTTOM = 1u << 4,
};

class CFlowLayout :public CLayoutBase
{
private:
	struct ITEM
	{
		ILayout* pCtrl;
		UINT uFlags;
		RECT rcPos;// 左顶宽高
	};

	std::vector<ITEM> m_vCtrl{};

	MARGINS m_LineMargin{};
	int m_cxCtrlPadding{};
	int m_cyCtrlPadding{};
public:
	EckInline int Add(ILayout* pCtrl, UINT uFlags = 0)
	{
		m_vCtrl.emplace_back(pCtrl, uFlags, RECT{});
		return (int)m_vCtrl.size() - 1;
	}

	void LoCommit() override
	{
		const int cxMax = m_cx - m_LineMargin.cxLeftWidth - m_LineMargin.cxRightWidth;
		HDWP hDwp = PreArrange(m_vCtrl.size());
		int x = m_y + m_LineMargin.cxLeftWidth;
		int y = m_x + m_LineMargin.cyTopHeight;
		int cxAppr, cyAppr;
		int cyLineMax = 0;
		int idxInLine = 0;
		EckCounter(m_vCtrl.size(), i)
		{
			auto& e = m_vCtrl[i];
			e.pCtrl->LoGetAppropriateSize(cxAppr, cyAppr);
			if ((e.uFlags & FLF_FIXHEIGHT) || (e.uFlags & FLF_FIXWIDTH))
			{
				const auto size = e.pCtrl->LoGetSize();
				if (e.uFlags & FLF_FIXWIDTH)
					cxAppr = size.first;
				if (e.uFlags & FLF_FIXHEIGHT)
					cyAppr = size.second;
			}

			if (x + cxAppr <= cxMax || idxInLine == 0)
			{
				if (cyAppr > cyLineMax)
					cyLineMax = cyAppr;
				e.rcPos.left = x;
				e.rcPos.right = cxAppr;
				e.rcPos.bottom = cyAppr;
				++idxInLine;
				x += (cxAppr + m_cxCtrlPadding);
			}
			else// 开始新行
			{
				for (size_t j = i - idxInLine; j < i; ++j)
				{
					auto& f = m_vCtrl[j];
					f.rcPos.top = y + (cyLineMax - f.rcPos.bottom) / 2;
					if (const auto hWnd = f.pCtrl->LoGetHWND())
						hDwp = DeferWindowPos(hDwp, hWnd, NULL,
							f.rcPos.left, f.rcPos.top, f.rcPos.right, f.rcPos.bottom,
							SWP_NOZORDER | SWP_NOACTIVATE);
					else
					{
						f.pCtrl->LoSetPosSize(f.rcPos.left, f.rcPos.top, f.rcPos.right, f.rcPos.bottom);
						f.pCtrl->LoCommit();
					}
				}
				y += (m_LineMargin.cyTopHeight + cyLineMax);
				cyLineMax = 0;
				x = m_x + m_LineMargin.cxLeftWidth;
				idxInLine = 0;

				if (cyAppr > cyLineMax)
					cyLineMax = cyAppr;
				e.rcPos.left = x;
				e.rcPos.right = cxAppr;
				e.rcPos.bottom = cyAppr;
				++idxInLine;
				x += (cxAppr + m_cxCtrlPadding);
			}
		}
		if (idxInLine)
			for (size_t j = m_vCtrl.size() - idxInLine; j < m_vCtrl.size(); ++j)
			{
				auto& f = m_vCtrl[j];
				f.rcPos.top = y + (cyLineMax - f.rcPos.bottom) / 2;
				if (const auto hWnd = f.pCtrl->LoGetHWND())
					hDwp = DeferWindowPos(hDwp, hWnd, NULL,
						f.rcPos.left, f.rcPos.top, f.rcPos.right, f.rcPos.bottom,
						SWP_NOZORDER | SWP_NOACTIVATE);
				else
				{
					f.pCtrl->LoSetPosSize(f.rcPos.left, f.rcPos.top, f.rcPos.right, f.rcPos.bottom);
					f.pCtrl->LoCommit();
				}
			}
		PostArrange(hDwp);
	}
};

ECK_NAMESPACE_END