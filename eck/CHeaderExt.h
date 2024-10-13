#pragma once
#include "CHeader.h"

ECK_NAMESPACE_BEGIN
class CHeaderExt : public CHeader
{
private:

public:
	LRESULT OnNotifyMsg(HWND hParent, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bProcessed) override
	{
		if (ShouldAppsUseDarkMode())
			switch (uMsg)
			{
			case WM_NOTIFY:
			{
				switch (((NMHDR*)lParam)->code)
				{
				case NM_CUSTOMDRAW:
				{
					bProcessed = TRUE;
					const auto pnmcd = (NMCUSTOMDRAW*)lParam;
					switch (pnmcd->dwDrawStage)
					{
					case CDDS_PREPAINT:
						return CDRF_NOTIFYITEMDRAW;
					case CDDS_ITEMPREPAINT:
						SetTextColor(pnmcd->hdc, GetThreadCtx()->crDefText);
						return CDRF_DODEFAULT;
					}
				}
				return CDRF_DODEFAULT;
				}
			}
			break;
			}
		return __super::OnNotifyMsg(hParent, uMsg, wParam, lParam, bProcessed);
	}
};
ECK_NAMESPACE_END