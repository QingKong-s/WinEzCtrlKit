#pragma once
#include "CHeader.h"

ECK_NAMESPACE_BEGIN
EckInline HRESULT DrawListViewColumnDetail(HTHEME hTheme, HDC hDC, int x, int yTop, int yBottom)
{
	RECT rc{ x - 1,yTop,x,yBottom };
	return DrawThemeBackground(hTheme, hDC, LVP_COLUMNDETAIL, 0, &rc, NULL);
}

inline void DrawSelectionRect(HDC hDC, const RECT& rc)
{
	HDC hCDC = CreateCompatibleDC(hDC);
	HBITMAP hBitmap = CreateCompatibleBitmap(hDC, 1, 1);
	SelectObject(hCDC, hBitmap);

	constexpr RECT rcInt{ 0,0,1,1 };
	FillRect(hCDC, &rcInt, GetSysColorBrush(COLOR_MENUHILIGHT));

	BLENDFUNCTION bf{ .BlendOp = AC_SRC_OVER,.SourceConstantAlpha = 70 };
	AlphaBlend(hDC, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
		hCDC, 0, 0, 1, 1, bf);

	DeleteDC(hCDC);
	DeleteObject(hBitmap);

	FrameRect(hDC, &rc, GetSysColorBrush(COLOR_HIGHLIGHT));
}

inline void DrawPlusMinusGlyph(HDC hDC, BOOL bPlus, const RECT& rc, COLORREF crBorder, COLORREF crSign)
{
	SetDCBrushColor(hDC, crBorder);
	FrameRect(hDC, &rc, GetStockBrush(DC_BRUSH));
	SetDCPenColor(hDC, crSign);
	const HGDIOBJ hOld = SelectObject(hDC, GetStockBrush(DC_PEN));
	const int xMargin = (rc.right - rc.left) / 5;
	MoveToEx(hDC, rc.left + xMargin, (rc.top + rc.bottom) / 2, NULL);
	LineTo(hDC, rc.right - xMargin, (rc.top + rc.bottom) / 2);
	if (bPlus)
	{
		const int yMargin = (rc.bottom - rc.top) / 5;
		MoveToEx(hDC, (rc.left + rc.right) / 2, rc.top + yMargin, NULL);
		LineTo(hDC, (rc.left + rc.right) / 2, rc.bottom - yMargin);
	}
	SelectObject(hDC, hOld);
}
ECK_NAMESPACE_END