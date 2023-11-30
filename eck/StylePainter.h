#pragma once
#include "Utility.h"

ECK_NAMESPACE_BEGIN



struct CWin8UxRes
{
	HBRUSH m_hbrHot = CreateSolidBrush(0xFBF3E5);
	HPEN m_hpenHot = CreatePen(PS_SOLID, 1, 0xE7C070);
	HBRUSH m_hbrSel = CreateSolidBrush(0xF6E8CB);
	HPEN m_hpenSel = CreatePen(PS_SOLID, 1, 0xDAA026);
	HBRUSH m_hbrHotSel = CreateSolidBrush(0xFFE8D1);
	HPEN m_hpenHotSel = CreatePen(PS_SOLID, 1, 0xE8A766);
};
class CWin8UxThemePainter
{
	static CWin8UxRes s_Res;
	CWin8UxRes* m_pRes = &s_Res;
public:
	EckInline CWin8UxRes* SetRes(CWin8UxRes* pRes)
	{
		std::swap(pRes, m_pRes);
		return pRes;
	}

	EckInline const CWin8UxRes* GetRes() const { return m_pRes; }

	HRESULT DrawThemeBackground(HDC hDC, int iPart, int iState, const RECT& rc, const RECT* prcClip)
	{
		if (prcClip)
			IntersectClipRect(hDC, prcClip->left, prcClip->top, prcClip->right, prcClip->bottom);
		HGDIOBJ hOld1, hOld2;
		BOOL b;
		switch (iPart)
		{
		case LVP_LISTITEM:
		{
			switch (iState)
			{
			case LISS_NORMAL:
				return HResultFromBool(FillRect(hDC, &rc, (HBRUSH)GetSysColorBrush(COLOR_WINDOW)));
			case LISS_HOT:
				hOld1 = SelectObject(hDC, m_pRes->m_hbrHot);
				hOld2 = SelectObject(hDC, m_pRes->m_hpenHot);
				b = Rectangle(hDC, rc.left, rc.top, rc.right, rc.bottom);
				SelectObject(hDC, hOld1);
				SelectObject(hDC, hOld2);
				return HResultFromBool(b);
			case LISS_SELECTED:
				hOld1 = SelectObject(hDC, m_pRes->m_hbrSel);
				hOld2 = SelectObject(hDC, m_pRes->m_hpenSel);
				b = Rectangle(hDC, rc.left, rc.top, rc.right, rc.bottom);
				SelectObject(hDC, hOld1);
				SelectObject(hDC, hOld2);
				return HResultFromBool(b);
			case LISS_HOTSELECTED:
				hOld1 = SelectObject(hDC, m_pRes->m_hbrHotSel);
				hOld2 = SelectObject(hDC, m_pRes->m_hpenHotSel);
				b = Rectangle(hDC, rc.left, rc.top, rc.right, rc.bottom);
				SelectObject(hDC, hOld1);
				SelectObject(hDC, hOld2);
				return HResultFromBool(b);
			}
		}
		break;
		}
		return E_NOTIMPL;
	}
};
inline CWin8UxRes CWin8UxThemePainter::s_Res{};
ECK_NAMESPACE_END