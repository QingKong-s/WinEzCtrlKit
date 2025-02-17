﻿#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
class CMonthCalendar : public CWnd
{
private:
	BOOL m_bAutoDarkMode{ TRUE };
public:
	ECK_RTTI(CMonthCalendar);
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

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		if (m_bAutoDarkMode)
			switch (uMsg)
			{
			case WM_THEMECHANGED:
			case WM_CREATE:
			{
				const auto lResult = __super::OnMsg(hWnd, uMsg, wParam, lParam);
				SetCorrectColor();
				return lResult;
			}
			break;
			}
		return __super::OnMsg(hWnd, uMsg, wParam, lParam);
	}

	EckInline int GetCalBorder() const
	{
		return (int)SendMsg(MCM_GETCALENDARBORDER, 0, 0);
	}

	EckInline int GetCalCount() const
	{
		return (int)SendMsg(MCM_GETCALENDARCOUNT, 0, 0);
	}

	EckInline BOOL GetGridInfo(_Inout_ MCGRIDINFO* pmcgi) const
	{
		return (BOOL)SendMsg(MCM_GETCALENDARGRIDINFO, 0, (LPARAM)pmcgi);
	}

	/// <summary>
	/// 取日历类型
	/// </summary>
	/// <returns>CAL_常量</returns>
	EckInline int GetCalId() const
	{
		return (int)SendMsg(MCM_GETCALID, 0, 0);
	}

	/// <summary>
	/// 取颜色
	/// </summary>
	/// <param name="iType">类型，MCSC_常量</param>
	/// <returns>成功返回颜色，失败返回CLR_INVALID</returns>
	EckInline COLORREF GetColor(int iType) const
	{
		return (COLORREF)SendMsg(MCM_GETCOLOR, iType, 0);
	}

	/// <summary>
	/// 取视图类型
	/// </summary>
	/// <returns>MCMV_常量</returns>
	EckInline int GetView() const
	{
		return (int)SendMsg(MCM_GETCURRENTVIEW, 0, 0);
	}

	EckInline BOOL GetCurrSel(_Out_ SYSTEMTIME* pst) const
	{
		return (BOOL)SendMsg(MCM_GETCURSEL, 0, (LPARAM)pst);
	}

	EckInline DWORD GetFirstDayOfWeek() const
	{
		return (DWORD)SendMsg(MCM_GETFIRSTDAYOFWEEK, 0, 0);
	}

	EckInline BOOL GetFirstDayOfWeek(_Out_ int* pnDay) const
	{
		const auto dwRet = GetFirstDayOfWeek();
		*pnDay = LOWORD(dwRet);
		return HIWORD(dwRet);
	}

	EckInline int GetMaxSelCount() const
	{
		return (int)SendMsg(MCM_GETMAXSELCOUNT, 0, 0);
	}

	EckInline int GetMaxTodayWidth() const
	{
		return (int)SendMsg(MCM_GETMAXTODAYWIDTH, 0, 0);
	}

	EckInline BOOL GetMinReqRect(_Out_ RECT* prc) const
	{
		return (BOOL)SendMsg(MCM_GETMINREQRECT, 0, (LPARAM)prc);
	}

	EckInline int GetMonthDelta() const
	{
		return (int)SendMsg(MCM_GETMONTHDELTA, 0, 0);
	}

	/// <summary>
	/// 取月历上下限
	/// </summary>
	/// <param name="iOption">GMR_常量</param>
	/// <param name="pst">依次接收最小值和最大值</param>
	/// <returns>范围跨越的月数</returns>
	EckInline int GetMonthRange(int iOption, _Out_writes_(2) SYSTEMTIME* pst) const
	{
		return (int)SendMsg(MCM_GETMONTHRANGE, iOption, (LPARAM)pst);
	}

	/// <summary>
	/// 取月历上下限。
	/// 检索已设置的最小和最大日期范围
	/// </summary>
	/// <param name="pst">依次接收最小值和最大值</param>
	/// <returns>GDTR_常量</returns>
	EckInline DWORD GetRange(_Out_writes_(2) SYSTEMTIME* pst) const
	{
		return (DWORD)SendMsg(MCM_GETRANGE, 0, (LPARAM)pst);
	}

	EckInline BOOL GetSelRange(_Out_writes_(2) SYSTEMTIME* pst) const
	{
		return (BOOL)SendMsg(MCM_GETSELRANGE, 0, (LPARAM)pst);
	}

	EckInline BOOL GetToday(_Out_ SYSTEMTIME* pst) const
	{
		return (BOOL)SendMsg(MCM_GETTODAY, 0, (LPARAM)pst);
	}

	EckInline UINT HitTest(_Inout_ MCHITTESTINFO* pMCHitTest) const
	{
		return (UINT)SendMsg(MCM_HITTEST, 0, (LPARAM)pMCHitTest);
	}

	EckInline void SetCalBorder(int i, BOOL bSetOrReset) const
	{
		SendMsg(MCM_SETCALENDARBORDER, bSetOrReset, i);
	}

	EckInline void SetCalId(int iCalID) const
	{
		SendMsg(MCM_SETCALID, iCalID, 0);
	}

	/// <summary>
	/// 设置颜色
	/// </summary>
	/// <param name="iType">类型，MCSC_常量</param>
	/// <param name="crColor">颜色</param>
	/// <returns>成功返回上一个颜色，失败返回CLR_INVALID</returns>
	EckInline COLORREF SetColor(int iType, COLORREF crColor) const
	{
		return (COLORREF)SendMsg(MCM_SETCOLOR, iType, (LPARAM)crColor);
	}

	/// <summary>
	/// 设置视图类型
	/// </summary>
	/// <param name="iView">MCMV_常量</param>
	/// <returns>成功返回TRUE，失败返回FALSE</returns>
	EckInline BOOL SetView(int iView) const
	{
		return (BOOL)SendMsg(MCM_SETCURRENTVIEW, iView, 0);
	}

	EckInline BOOL SetCurrSel(_In_ SYSTEMTIME* pst) const
	{
		return (BOOL)SendMsg(MCM_SETCURSEL, 0, (LPARAM)pst);
	}

	EckInline BOOL SetDayState(_In_ MONTHDAYSTATE* pmds, int c)
	{
		return (BOOL)SendMsg(MCM_SETDAYSTATE, c, (LPARAM)pmds);
	}

	EckInline DWORD SetFirstDayOfWeek(int nDay) const
	{
		return (DWORD)SendMsg(MCM_SETFIRSTDAYOFWEEK, 0, nDay);
	}

	EckInline BOOL SetMaxSelCount(int cMax) const
	{
		return (BOOL)SendMsg(MCM_SETMAXSELCOUNT, cMax, 0);
	}

	EckInline int SetMonthDelta(int nDelta) const
	{
		return (int)SendMsg(MCM_SETMONTHDELTA, nDelta, 0);
	}

	EckInline BOOL SetRange(UINT uType, _In_ SYSTEMTIME* pst) const
	{
		return (BOOL)SendMsg(MCM_SETRANGE, uType, (LPARAM)pst);
	}

	EckInline BOOL SetSelRange(_In_reads_(2) SYSTEMTIME* pst) const
	{
		return (BOOL)SendMsg(MCM_SETSELRANGE, 0, (LPARAM)pst);
	}

	EckInline void SetToday(_In_ SYSTEMTIME* pst) const
	{
		SendMsg(MCM_SETTODAY, 0, (LPARAM)pst);
	}

	EckInline constexpr void SetAutoDarkMode(BOOL b) { m_bAutoDarkMode = b; }
	EckInline constexpr BOOL GetAutoDarkMode() const { return m_bAutoDarkMode; }

	EckInline void SetCorrectColor() const
	{
		SetColor(MCSC_BACKGROUND, GetThreadCtx()->crDefBkg);
	}
};
ECK_RTTI_IMPL_BASE_INLINE(CMonthCalendar, CWnd);
ECK_NAMESPACE_END