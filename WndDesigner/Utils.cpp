#include "pch.h"
#include "Utils.h"

POINT PtAlign(POINT pt, int iUnit) noexcept
{
    int ii = (pt.x / iUnit) * iUnit;
    int iDelta = pt.x - ii;
    pt.x = (iDelta > iUnit / 2 ? ii + iUnit : ii);
    ii = (pt.y / iUnit) * iUnit;
    iDelta = pt.y - ii;
    pt.y = (iDelta > iUnit / 2 ? ii + iUnit : ii);
    return pt;
}

void DrawGridPoint(HDC hDC, const RECT& rcPaint, int d, COLORREF crPoint) noexcept
{
    for (int x = (rcPaint.left / d) * d; x < rcPaint.right; x += d)
        for (int y = (rcPaint.top / d) * d; y < rcPaint.bottom; y += d)
            SetPixel(hDC, x, y, crPoint);
}