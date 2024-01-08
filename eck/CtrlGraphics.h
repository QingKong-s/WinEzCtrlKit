#pragma once
#include "CHeader.h"

ECK_NAMESPACE_BEGIN
inline BOOL DrawListViewItemBackground(HTHEME hTheme, int iLVType, CHeader* pHeader,
	const NMLVCUSTOMDRAW* plvnmcd, int cCol, RECT* prcCol)
{
	if (hTheme)
	{
		FillRect(plvnmcd->nmcd.hdc, &plvnmcd->nmcd.rc, GetSysColorBrush(COLOR_WINDOW));
		if (iLVType == LV_VIEW_DETAILS)
		{
			EckCounter(cCol, i)
				pHeader->GetItemRect(i, prcCol + i);
			if (plvnmcd->iStateId && cCol)
			{
				RECT rc
				{ 
					plvnmcd->nmcd.rc.left,
					plvnmcd->nmcd.rc.top,
					prcCol[cCol - 1].right,
					plvnmcd->nmcd.rc.bottom 
				};
				DrawThemeBackground(hTheme, plvnmcd->nmcd.hdc, plvnmcd->iPartId,
					plvnmcd->iStateId, &rc, NULL);
			}
			EckCounter(cCol, i)
			{
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
		else
			if (plvnmcd->iStateId)
				DrawThemeBackground(hTheme, plvnmcd->nmcd.hdc, plvnmcd->iPartId,
					plvnmcd->iStateId, &plvnmcd->nmcd.rc, NULL);
	}
	else
	{

	}
	return TRUE;
}
ECK_NAMESPACE_END