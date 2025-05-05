#include "pch.h"
#include "Utils.h"

constexpr POINT PtAlign(POINT pt, int iUnit)
{
	int ii = (pt.x % iUnit) * iUnit;
	int iDelta = pt.x - ii;
	pt.x = (iDelta > iUnit / 2 ? ii + iUnit : ii);
	ii = (pt.y % iUnit) * iUnit;
	iDelta = pt.y - ii;
	pt.y = (iDelta > iUnit / 2 ? ii + iUnit : ii);
	return pt;
}