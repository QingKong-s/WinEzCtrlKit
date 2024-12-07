/*
* WinEzCtrlKit Library
*
* CDateTimePicker.h ： 标准日期时间选择器
*
* Copyright(C) 2023-2024 QingKong
*/
#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
constexpr inline UINT DateTimePickerFormatMask = 0xC;
constexpr inline UINT DTS_TIMEFORMAT_NO_UPDOWN = 0x8;
class CDateTimePicker :public CWnd
{
public:
	ECK_RTTI(CDateTimePicker);
	ECK_CWND_NOSINGLEOWNER(CDateTimePicker);
	ECK_CWND_CREATE_CLS(DATETIMEPICK_CLASSW);

	ECK_CWNDPROP_STYLE(AppCanParse, DTS_APPCANPARSE);
	ECK_CWNDPROP_STYLE(LongDateFormat, DTS_LONGDATEFORMAT);
	ECK_CWNDPROP_STYLE(RightAlign, DTS_RIGHTALIGN);
	ECK_CWNDPROP_STYLE(ShowNone, DTS_SHOWNONE);
	ECK_CWNDPROP_STYLE(ShortDateFormat, DTS_SHORTDATEFORMAT);
	ECK_CWNDPROP_STYLE_MASK(ShortDateCenturyFormat, DTS_SHORTDATECENTURYFORMAT, DateTimePickerFormatMask);
	ECK_CWNDPROP_STYLE_MASK(TimeFormat, DTS_TIMEFORMAT, DateTimePickerFormatMask);
	ECK_CWNDPROP_STYLE(UpDown, DTS_UPDOWN);

	void CloseMonthCal() const
	{
		SendMsg(DTM_CLOSEMONTHCAL, 0, 0);
	}

	void GetDateTimePickerInfo(_Inout_ DATETIMEPICKERINFO* pdtpi) const
	{
		SendMsg(DTM_GETDATETIMEPICKERINFO, 0, (LPARAM)pdtpi);
	}

	void GetIdealSize(_Out_ SIZE* psize) const
	{
		SendMsg(DTM_GETIDEALSIZE, 0, (LPARAM)psize);
	}

	COLORREF GetMonthCalColor(int iType) const
	{
		return (COLORREF)SendMsg(DTM_GETMCCOLOR, iType, 0);
	}

	HFONT GetMonthCalFont() const
	{
		return (HFONT)SendMsg(DTM_GETMCFONT, 0, 0);
	}

	DWORD GetMonthCalStyle() const
	{
		return (DWORD)SendMsg(DTM_GETMCSTYLE, 0, 0);
	}

	HWND GetMonthCal() const
	{
		return (HWND)SendMsg(DTM_GETMONTHCAL, 0, 0);
	}

	DWORD GetRange(_Out_writes_(2) SYSTEMTIME* pst) const
	{
		return (DWORD)SendMsg(DTM_GETRANGE, 0, (LPARAM)pst);
	}

	DWORD GetSystemTime(_In_ SYSTEMTIME* pst) const
	{
		return (DWORD)SendMsg(DTM_GETSYSTEMTIME, 0, (LPARAM)pst);
	}

	BOOL SetFormat(_In_z_ PCWSTR pszText) const
	{
		return (BOOL)SendMsg(DTM_SETFORMATW, 0, (LPARAM)pszText);
	}

	COLORREF GetMonthCalColor(int iType, COLORREF cr) const
	{
		return (COLORREF)SendMsg(DTM_SETMCCOLOR, iType, cr);
	}

	void SetMonthCalFont(HFONT hFont, BOOL bRedraw = TRUE) const
	{
		SendMsg(DTM_SETMCFONT, (WPARAM)hFont, bRedraw);
	}

	DWORD SetMonthCalStyle(DWORD dwStyle) const
	{
		return (DWORD)SendMsg(DTM_SETMCSTYLE, 0, dwStyle);
	}

	BOOL SetRange(DWORD dwFlags, _In_reads_(2) const SYSTEMTIME* pst) const
	{
		return (BOOL)SendMsg(DTM_SETRANGE, dwFlags, (LPARAM)pst);
	}

	BOOL SetSystemTime(DWORD dwFlags, _In_ SYSTEMTIME* pst) const
	{
		return (BOOL)SendMsg(DTM_SETSYSTEMTIME, 0, (LPARAM)pst);
	}
};
ECK_RTTI_IMPL_BASE_INLINE(CDateTimePicker, CWnd);
ECK_NAMESPACE_END