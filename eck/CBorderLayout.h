/*
* WinEzCtrlKit Library
*
* CBorderLayout.h : 边框布局
*
* Copyright(C) 2023-2024 QingKong
*/
#pragma once
#include "CLayoutBase.h"

ECK_NAMESPACE_BEGIN
enum
{
	BLIDX_TOP,
	BLIDX_LEFT,
	BLIDX_RIGHT,
	BLIDX_BOTTOM,
	BLIDX_CENTER,
};

class CBorderLayoutBase : public CLayoutBase
{
protected:
	enum
	{
		FSIDX_TOP = 0,
		FSIDX_LEFT = 0,
		FSIDX_RIGHT = 1,
		FSIDX_BOTTOM = 1,
	};

	struct ITEM :ITEMBASE {};

	ITEM m_vItem[5];
public:
	void Add(ILayout* pCtrl, int nType, const MARGINS& Margin = {}, UINT uFlags = 0u)
	{
		auto& e = m_vItem[nType];
		if (pCtrl)
		{
			e.pCtrl = pCtrl;
			if (nType == BLIDX_TOP || nType == BLIDX_BOTTOM)
			{
				uFlags &= ~(LF_FILL_HEIGHT | LF_FIT);
				uFlags |= LF_FIX_HEIGHT;
			}
			else if (nType == BLIDX_LEFT || nType == BLIDX_RIGHT)
			{
				uFlags &= ~(LF_FILL_WIDTH | LF_FIT);
				uFlags |= LF_FIX_WIDTH;
			}
			e.uFlags = uFlags;
			e.Margin = Margin;
			const auto size = pCtrl->LoGetSize();
			e.cx = size.first;
			e.cy = size.second;
		}
		else
			e.pCtrl = nullptr;
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
};

class CBorderLayoutV : public CBorderLayoutBase
{
public:
	void LoCommit() override
	{
		int x, y, cx, cy;
		HDWP hDwp = PreArrange(5);
		const auto& T = m_vItem[BLIDX_TOP];
		const auto& L = m_vItem[BLIDX_LEFT];
		const auto& C = m_vItem[BLIDX_CENTER];
		const auto& R = m_vItem[BLIDX_RIGHT];
		const auto& B = m_vItem[BLIDX_BOTTOM];
		// 上
		if (T.pCtrl)
		{
			CalcCtrlPosSize(T, { m_x,m_y,m_cx,T.cy }, x, y, cx, cy);
			MoveCtrlPosSize(T, hDwp, x, y, cx, cy);
		}
		// 左
		if (L.pCtrl)
		{
			CalcCtrlPosSize(L, { m_x,m_y + T.cy,L.cx,m_cy - T.cy - B.cy }, x, y, cx, cy);
			MoveCtrlPosSize(L, hDwp, x, y, cx, cy);
		}
		// 中
		if (C.pCtrl)
		{
			CalcCtrlPosSize(C, { m_x + L.cx,m_y + T.cy,m_cx - L.cx - R.cx,m_cy - T.cy - B.cy }, x, y, cx, cy);
			MoveCtrlPosSize(C, hDwp, x, y, cx, cy);
		}
		// 右
		if (R.pCtrl)
		{
			CalcCtrlPosSize(R, { m_x + m_cx - R.cx,m_y + T.cy,R.cx,m_cy - T.cy - B.cy }, x, y, cx, cy);
			MoveCtrlPosSize(R, hDwp, x, y, cx, cy);
		}
		// 下
		if (B.pCtrl)
		{
			CalcCtrlPosSize(B, { m_x,m_y + m_cy - B.cy,m_cx,B.cy }, x, y, cx, cy);
			MoveCtrlPosSize(B, hDwp, x, y, cx, cy);
		}
		PostArrange(hDwp);
	}
};

class CBorderLayoutH : public CBorderLayoutBase
{
public:
	void LoCommit() override
	{
		int x, y, cx, cy;
		HDWP hDwp = PreArrange(5);
		const auto& T = m_vItem[BLIDX_TOP];
		const auto& L = m_vItem[BLIDX_LEFT];
		const auto& C = m_vItem[BLIDX_CENTER];
		const auto& R = m_vItem[BLIDX_RIGHT];
		const auto& B = m_vItem[BLIDX_BOTTOM];
		// 上
		if (T.pCtrl)
		{
			CalcCtrlPosSize(T, { m_x + L.cx,m_y,m_cx - L.cx - R.cx,T.cy }, x, y, cx, cy);
			MoveCtrlPosSize(T, hDwp, x, y, cx, cy);
		}
		// 左
		if (L.pCtrl)
		{
			CalcCtrlPosSize(L, { m_x,m_y,L.cx,m_cy }, x, y, cx, cy);
			MoveCtrlPosSize(L, hDwp, x, y, cx, cy);
		}
		// 中
		if (C.pCtrl)
		{
			CalcCtrlPosSize(C, { m_x + L.cx,m_y + T.cy,m_cx - L.cx - R.cx,m_cy - T.cy - B.cy }, x, y, cx, cy);
			MoveCtrlPosSize(C, hDwp, x, y, cx, cy);
		}
		// 右
		if (R.pCtrl)
		{
			CalcCtrlPosSize(R, { m_x + m_cx - R.cx,m_y,R.cx,m_cy }, x, y, cx, cy);
			MoveCtrlPosSize(R, hDwp, x, y, cx, cy);
		}
		// 下
		if (B.pCtrl)
		{
			CalcCtrlPosSize(B, { m_x + L.cx,m_y + m_cy - B.cy,m_cx - L.cx - R.cx,B.cy }, x, y, cx, cy);
			MoveCtrlPosSize(B, hDwp, x, y, cx, cy);
		}
		PostArrange(hDwp);
	}
};
ECK_NAMESPACE_END