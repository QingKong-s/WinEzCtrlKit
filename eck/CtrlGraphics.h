﻿#pragma once
#include "GraphicsHelper.h"

ECK_NAMESPACE_BEGIN
EckInline HRESULT DrawListViewColumnDetail(HTHEME hTheme, HDC hDC, int x, int yTop, int yBottom)
{
	const RECT rc{ x - 2,yTop,x,yBottom };
	return DrawThemeBackground(hTheme, hDC, LVP_COLUMNDETAIL, 0, &rc, nullptr);
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

inline void DrawSelectionRect(HDC hDC, const RECT& rc, int cxFrame)
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

	const auto hOldPen = SelectObject(hDC, CreatePen(PS_SOLID, cxFrame, GetSysColor(COLOR_HIGHLIGHT)));
	const auto hOldBrush = SelectObject(hDC, GetStockObject(NULL_BRUSH));
	Rectangle(hDC, rc.left, rc.top, rc.right, rc.bottom);
	SelectObject(hDC, hOldBrush);
	DeleteObject(SelectObject(hDC, hOldPen));
}

inline void DrawPlusMinusGlyph(HDC hDC, BOOL bPlus, const RECT& rc, COLORREF crBorder, COLORREF crSign)
{
	SetDCBrushColor(hDC, crBorder);
	FrameRect(hDC, &rc, GetStockBrush(DC_BRUSH));
	SetDCPenColor(hDC, crSign);
	const HGDIOBJ hOld = SelectObject(hDC, GetStockBrush(DC_PEN));
	const int xMargin = (rc.right - rc.left) / 5;
	MoveToEx(hDC, rc.left + xMargin, (rc.top + rc.bottom) / 2, nullptr);
	LineTo(hDC, rc.right - xMargin, (rc.top + rc.bottom) / 2);
	if (bPlus)
	{
		const int yMargin = (rc.bottom - rc.top) / 5;
		MoveToEx(hDC, (rc.left + rc.right) / 2, rc.top + yMargin, nullptr);
		LineTo(hDC, (rc.left + rc.right) / 2, rc.bottom - yMargin);
	}
	SelectObject(hDC, hOld);
}

inline BOOL AlphaBlendColor(HDC hDC, const RECT& rc, COLORREF cr, BYTE byAlpha = ColorFillAlpha)
{
	CEzCDC DC{};
	DC.CreateFromDC(hDC, 1, 1);
	constexpr RECT rcDst{ 0,0,1,1 };
	SetDCBrushColor(DC.GetDC(), cr);
	FillRect(DC.GetDC(), &rcDst, GetStockBrush(DC_BRUSH));
	return AlphaBlend(hDC, rc.left, rc.top, rc.right - rc.left,rc.bottom - rc.top,
		DC.GetDC(), 0, 0, 1, 1, { AC_SRC_OVER,0,byAlpha,0 });
}
ECK_NAMESPACE_END