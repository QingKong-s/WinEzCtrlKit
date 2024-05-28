#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
class CDateTimePicker :public CWnd
{
public:
	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = NULL) override
	{
		return IntCreate(0, DATETIMEPICK_CLASSW, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, NULL, NULL);
	}

	void CloseMonthCal()
	{
		SendMsg(DTM_CLOSEMONTHCAL, 0, 0);
	}

	void GetDateTimePickerInfo(DATETIMEPICKERINFO* pdtpi)
	{
		SendMsg(DTM_GETDATETIMEPICKERINFO, 0, (LPARAM)pdtpi);
	}

	void GetIdealSize(SIZE* psize)
	{
		SendMsg(DTM_GETIDEALSIZE, 0, (LPARAM)psize);
	}

	COLORREF GetMonthCalColor(int iType)
	{
		return (COLORREF)SendMsg(DTM_GETMCCOLOR, iType, 0);
	}

	HFONT GetMonthCalFont()
	{
		return (HFONT)SendMsg(DTM_GETMCFONT, 0, 0);
	}

	DWORD GetMonthCalStyle()
	{
		return (DWORD)SendMsg(DTM_GETMCSTYLE, 0, 0);
	}

	HWND GetMonthCal()
	{
		return (HWND)SendMsg(DTM_GETMONTHCAL, 0, 0);
	}

	DWORD GetRange(_Out_writes_(2) SYSTEMTIME* pst)
	{
		return (DWORD)SendMsg(DTM_GETRANGE, 0, (LPARAM)pst);
	}

	DWORD GetSystemTime(SYSTEMTIME* pst)
	{
		return (DWORD)SendMsg(DTM_GETSYSTEMTIME, 0, (LPARAM)pst);
	}

	BOOL SetFormat(PCWSTR pszText)
	{
		return (BOOL)SendMsg(DTM_SETFORMATW, 0, (LPARAM)pszText);
	}

	COLORREF GetMonthCalColor(int iType, COLORREF cr)
	{
		return (COLORREF)SendMsg(DTM_SETMCCOLOR, iType, cr);
	}

	void SetMonthCalFont(HFONT hFont, BOOL bRedraw = TRUE)
	{
		SendMsg(DTM_SETMCFONT, (WPARAM)hFont, bRedraw);
	}

	DWORD SetMonthCalStyle(DWORD dwStyle)
	{
		return (DWORD)SendMsg(DTM_SETMCSTYLE, 0, dwStyle);
	}

	BOOL SetRange(DWORD dwFlags, _In_reads_(2) const SYSTEMTIME* pst)
	{
		return (BOOL)SendMsg(DTM_SETRANGE, dwFlags, (LPARAM)pst);
	}

	BOOL SetSystemTime(DWORD dwFlags, SYSTEMTIME* pst)
	{
		return (BOOL)SendMsg(DTM_SETSYSTEMTIME, 0, (LPARAM)pst);
	}
};
ECK_NAMESPACE_END