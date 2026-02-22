#pragma once
#include "CWindow.h"

ECK_NAMESPACE_BEGIN
constexpr inline UINT DateTimePickerFormatMask = 0xC;
constexpr inline UINT DTS_TIMEFORMAT_NO_UPDOWN = 0x8;
class CDateTimePicker : public CWindow
{
public:
    ECK_RTTI(CDateTimePicker, CWindow);
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

    void CloseMonthCalendar() const noexcept
    {
        SendMessage(DTM_CLOSEMONTHCAL, 0, 0);
    }

    void GetDateTimePickerInfomation(_Inout_ DATETIMEPICKERINFO* pdtpi) const noexcept
    {
        SendMessage(DTM_GETDATETIMEPICKERINFO, 0, (LPARAM)pdtpi);
    }

    void GetIdealSize(_Out_ SIZE* psize) const noexcept
    {
        SendMessage(DTM_GETIDEALSIZE, 0, (LPARAM)psize);
    }

    COLORREF GetMonthCalendarColor(int iType) const noexcept
    {
        return (COLORREF)SendMessage(DTM_GETMCCOLOR, iType, 0);
    }

    HFONT GetMonthCalendarFont() const noexcept
    {
        return (HFONT)SendMessage(DTM_GETMCFONT, 0, 0);
    }

    DWORD GetMonthCalendarStyle() const noexcept
    {
        return (DWORD)SendMessage(DTM_GETMCSTYLE, 0, 0);
    }

    HWND GetMonthCalendar() const noexcept
    {
        return (HWND)SendMessage(DTM_GETMONTHCAL, 0, 0);
    }

    // 返回GDTR_*组合
    UINT GetRange(_Out_writes_(2) SYSTEMTIME* pst) const noexcept
    {
        return (UINT)SendMessage(DTM_GETRANGE, 0, (LPARAM)pst);
    }

    UINT GetSystemTime(_In_ SYSTEMTIME* pst) const noexcept
    {
        return (UINT)SendMessage(DTM_GETSYSTEMTIME, 0, (LPARAM)pst);
    }

    BOOL SetFormat(_In_z_ PCWSTR pszText) const noexcept
    {
        return (BOOL)SendMessage(DTM_SETFORMATW, 0, (LPARAM)pszText);
    }

    COLORREF GetMonthCalendarColor(int iType, COLORREF cr) const noexcept
    {
        return (COLORREF)SendMessage(DTM_SETMCCOLOR, iType, cr);
    }

    void SetMonthCalendarFont(HFONT hFont, BOOL bRedraw = TRUE) const noexcept
    {
        SendMessage(DTM_SETMCFONT, (WPARAM)hFont, bRedraw);
    }

    DWORD SetMonthCalendarStyle(DWORD dwStyle) const noexcept
    {
        return (DWORD)SendMessage(DTM_SETMCSTYLE, 0, dwStyle);
    }

    BOOL SetRange(UINT uFlags, _In_reads_(2) const SYSTEMTIME* pst) const noexcept
    {
        return (BOOL)SendMessage(DTM_SETRANGE, uFlags, (LPARAM)pst);
    }

    BOOL SetSystemTime(UINT uFlags, _In_ SYSTEMTIME* pst) const noexcept
    {
        return (BOOL)SendMessage(DTM_SETSYSTEMTIME, uFlags, (LPARAM)pst);
    }
};
ECK_NAMESPACE_END