#pragma once
POINT PtAlign(POINT pt, int iUnit) noexcept;
void DrawGridPoint(HDC hDC, const RECT& rcPaint, int d, COLORREF crPoint) noexcept;