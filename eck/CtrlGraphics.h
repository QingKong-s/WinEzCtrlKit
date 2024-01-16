#pragma once
#include "CHeader.h"

ECK_NAMESPACE_BEGIN
EckInline HRESULT DrawListViewItemBackground(HTHEME hTheme, HDC hDC, int iPart, int iState, 
	RECT& rcRow, long xActualRight)
{

	return S_OK;
}

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
ECK_NAMESPACE_END