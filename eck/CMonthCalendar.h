#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
class CMonthCalendar : public CWnd
{
private:
    BOOL m_bAutoDarkMode{ TRUE };
public:
    ECK_RTTI(CMonthCalendar, CWnd);
    ECK_CWND_NOSINGLEOWNER(CMonthCalendar);
    ECK_CWND_CREATE_CLS(MONTHCAL_CLASSW);

    ECK_CWNDPROP_STYLE(DayState, MCS_DAYSTATE);
    ECK_CWNDPROP_STYLE(MultiSelect, MCS_MULTISELECT);
    ECK_CWNDPROP_STYLE(WeekNumbers, MCS_WEEKNUMBERS);
    ECK_CWNDPROP_STYLE(NoTodayCircle, MCS_NOTODAYCIRCLE);
    ECK_CWNDPROP_STYLE(NoToday, MCS_NOTODAY);
    ECK_CWNDPROP_STYLE(NoTrailingDates, MCS_NOTRAILINGDATES);
    ECK_CWNDPROP_STYLE(ShortDaysOfWeek, MCS_SHORTDAYSOFWEEK);
    ECK_CWNDPROP_STYLE(NoSelChangeOnNav, MCS_NOSELCHANGEONNAV);

    LRESULT OnMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept override
    {
        if (m_bAutoDarkMode)
            switch (uMsg)
            {
            case WM_THEMECHANGED:
            case WM_CREATE:
            {
                const auto lResult = __super::OnMessage(hWnd, uMsg, wParam, lParam);
                SetCorrectColor();
                return lResult;
            }
            break;
            }
        return __super::OnMessage(hWnd, uMsg, wParam, lParam);
    }

    EckInline int GetCalendarBorder() const noexcept
    {
        return (int)SendMsg(MCM_GETCALENDARBORDER, 0, 0);
    }

    EckInline int GetCalendarCount() const noexcept
    {
        return (int)SendMsg(MCM_GETCALENDARCOUNT, 0, 0);
    }

    EckInline BOOL GetGridInfomation(_Inout_ MCGRIDINFO* pmcgi) const noexcept
    {
        return (BOOL)SendMsg(MCM_GETCALENDARGRIDINFO, 0, (LPARAM)pmcgi);
    }

    /// <summary>
    /// 取日历类型
    /// </summary>
    /// <returns>CAL_常量</returns>
    EckInline int GetCalendarId() const noexcept
    {
        return (int)SendMsg(MCM_GETCALID, 0, 0);
    }

    /// <summary>
    /// 取颜色
    /// </summary>
    /// <param name="iType">类型，MCSC_常量</param>
    /// <returns>成功返回颜色，失败返回CLR_INVALID</returns>
    EckInline COLORREF GetColor(int iType) const noexcept
    {
        return (COLORREF)SendMsg(MCM_GETCOLOR, iType, 0);
    }

    /// <summary>
    /// 取视图类型
    /// </summary>
    /// <returns>MCMV_常量</returns>
    EckInline int GetView() const noexcept
    {
        return (int)SendMsg(MCM_GETCURRENTVIEW, 0, 0);
    }

    EckInline BOOL GetCurrentSelection(_Out_ SYSTEMTIME* pst) const noexcept
    {
        return (BOOL)SendMsg(MCM_GETCURSEL, 0, (LPARAM)pst);
    }

    EckInline DWORD GetFirstDayOfWeek() const noexcept
    {
        return (DWORD)SendMsg(MCM_GETFIRSTDAYOFWEEK, 0, 0);
    }

    EckInline BOOL GetFirstDayOfWeek(_Out_ int* pnDay) const noexcept
    {
        const auto dwRet = GetFirstDayOfWeek();
        *pnDay = LOWORD(dwRet);
        return HIWORD(dwRet);
    }

    EckInline int GetMaxSelectionCount() const noexcept
    {
        return (int)SendMsg(MCM_GETMAXSELCOUNT, 0, 0);
    }

    EckInline int GetMaxTodayWidth() const noexcept
    {
        return (int)SendMsg(MCM_GETMAXTODAYWIDTH, 0, 0);
    }

    EckInline BOOL GetMinimumRect(_Out_ RECT* prc) const noexcept
    {
        return (BOOL)SendMsg(MCM_GETMINREQRECT, 0, (LPARAM)prc);
    }

    EckInline int GetMonthDelta() const noexcept
    {
        return (int)SendMsg(MCM_GETMONTHDELTA, 0, 0);
    }

    /// <summary>
    /// 取月历上下限
    /// </summary>
    /// <param name="iOption">GMR_常量</param>
    /// <param name="pst">依次接收最小值和最大值</param>
    /// <returns>范围跨越的月数</returns>
    EckInline int GetMonthRange(int iOption, _Out_writes_(2) SYSTEMTIME* pst) const noexcept
    {
        return (int)SendMsg(MCM_GETMONTHRANGE, iOption, (LPARAM)pst);
    }

    /// <summary>
    /// 取月历上下限。
    /// 检索已设置的最小和最大日期范围
    /// </summary>
    /// <param name="pst">依次接收最小值和最大值</param>
    /// <returns>GDTR_常量</returns>
    EckInline DWORD GetRange(_Out_writes_(2) SYSTEMTIME* pst) const noexcept
    {
        return (DWORD)SendMsg(MCM_GETRANGE, 0, (LPARAM)pst);
    }

    EckInline BOOL GetSelectionRange(_Out_writes_(2) SYSTEMTIME* pst) const noexcept
    {
        return (BOOL)SendMsg(MCM_GETSELRANGE, 0, (LPARAM)pst);
    }

    EckInline BOOL GetToday(_Out_ SYSTEMTIME* pst) const noexcept
    {
        return (BOOL)SendMsg(MCM_GETTODAY, 0, (LPARAM)pst);
    }

    EckInline UINT HitTest(_Inout_ MCHITTESTINFO* pMCHitTest) const noexcept
    {
        return (UINT)SendMsg(MCM_HITTEST, 0, (LPARAM)pMCHitTest);
    }

    EckInline void SetCalendarBorder(int i, BOOL bSetOrReset) const noexcept
    {
        SendMsg(MCM_SETCALENDARBORDER, bSetOrReset, i);
    }

    EckInline void SetCalendarId(int iCalID) const noexcept
    {
        SendMsg(MCM_SETCALID, iCalID, 0);
    }

    /// <summary>
    /// 设置颜色
    /// </summary>
    /// <param name="iType">类型，MCSC_常量</param>
    /// <param name="crColor">颜色</param>
    /// <returns>成功返回上一个颜色，失败返回CLR_INVALID</returns>
    EckInline COLORREF SetColor(int iType, COLORREF crColor) const noexcept
    {
        return (COLORREF)SendMsg(MCM_SETCOLOR, iType, (LPARAM)crColor);
    }

    /// <summary>
    /// 设置视图类型
    /// </summary>
    /// <param name="iView">MCMV_常量</param>
    /// <returns>成功返回TRUE，失败返回FALSE</returns>
    EckInline BOOL SetView(int iView) const noexcept
    {
        return (BOOL)SendMsg(MCM_SETCURRENTVIEW, iView, 0);
    }

    EckInline BOOL SetCurrentSelection(_In_ SYSTEMTIME* pst) const noexcept
    {
        return (BOOL)SendMsg(MCM_SETCURSEL, 0, (LPARAM)pst);
    }

    EckInline BOOL SetDayState(_In_ MONTHDAYSTATE* pmds, int c)
    {
        return (BOOL)SendMsg(MCM_SETDAYSTATE, c, (LPARAM)pmds);
    }

    EckInline DWORD SetFirstDayOfWeek(int nDay) const noexcept
    {
        return (DWORD)SendMsg(MCM_SETFIRSTDAYOFWEEK, 0, nDay);
    }

    EckInline BOOL SetMaxSelectionCount(int cMax) const noexcept
    {
        return (BOOL)SendMsg(MCM_SETMAXSELCOUNT, cMax, 0);
    }

    EckInline int SetMonthDelta(int nDelta) const noexcept
    {
        return (int)SendMsg(MCM_SETMONTHDELTA, nDelta, 0);
    }

    EckInline BOOL SetRange(UINT uType, _In_ SYSTEMTIME* pst) const noexcept
    {
        return (BOOL)SendMsg(MCM_SETRANGE, uType, (LPARAM)pst);
    }

    EckInline BOOL SetSelectionRange(_In_reads_(2) SYSTEMTIME* pst) const noexcept
    {
        return (BOOL)SendMsg(MCM_SETSELRANGE, 0, (LPARAM)pst);
    }

    EckInline void SetToday(_In_ SYSTEMTIME* pst) const noexcept
    {
        SendMsg(MCM_SETTODAY, 0, (LPARAM)pst);
    }

    EckInline constexpr void SetAutoDarkMode(BOOL b) noexcept { m_bAutoDarkMode = b; }
    EckInline constexpr BOOL GetAutoDarkMode() const noexcept { return m_bAutoDarkMode; }

    EckInline void SetCorrectColor() const noexcept
    {
        SetColor(MCSC_BACKGROUND, PtcCurrent()->crDefBkg);
    }
};
ECK_NAMESPACE_END