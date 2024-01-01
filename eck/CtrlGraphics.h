#pragma once
#include "CHeader.h"

ECK_NAMESPACE_BEGIN
inline BOOL DrawListViewItemBackground(HTHEME hTheme, int iLVType, CHeader* pHeader,
	const NMLVCUSTOMDRAW* plvnmcd, int cCol, RECT* prcCol)
{
	if (hTheme)
	{
		if (plvnmcd->iStateId)
			DrawThemeBackground(hTheme, plvnmcd->nmcd.hdc, plvnmcd->iPartId,
				plvnmcd->iStateId, &plvnmcd->nmcd.rc, NULL);
		else
			FillRect(plvnmcd->nmcd.hdc, &plvnmcd->nmcd.rc, GetSysColorBrush(COLOR_WINDOW));
		if (iLVType == LV_VIEW_DETAILS)
		{
			EckCounter(cCol, i)
			{
				pHeader->GetItemRect(i, prcCol + i);
				if (prcCol[i].left >= 0)
				{
					RECT rc
					{ 
						prcCol[i].left - 1,
						plvnmcd->nmcd.rc.top,
						prcCol[i].left,
						plvnmcd->nmcd.rc.bottom 
					};
					DrawThemeBackground(hTheme, plvnmcd->nmcd.hdc, LVP_COLUMNDETAIL, 0, &rc, NULL);
				}
			}
		}
	}
	else
	{

	}
	return TRUE;
}
ECK_NAMESPACE_END