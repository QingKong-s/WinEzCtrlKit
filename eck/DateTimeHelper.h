﻿/*
* WinEzCtrlKit Library
*
* DateTimeHelper.h ： 日期时间
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
struct CEDate
{
	WORD wYear;
	BYTE byMonth;
	BYTE byDay;
};

#ifdef _DEBUG
#define EckVerifyCeDate(x)														\
			EckAssert(x.byMonth >= 1 &&											\
				x.byMonth <= 12 &&												\
				x.byDay >= 1 &&													\
				x.byDay <= GetMonthDays(x.wYear, x.byMonth))
#else
#define EckVerifyCeDate(x) ;
#endif // _DEBUG

inline constexpr BYTE CeMonthDays[12]{ 31,0,31,30,31,30,31,31,30,31,30,31 };

inline constexpr USHORT MonthDaysSumLeapYear[12]{ 0,31,60,91,121,152,182,213,244,274,305,335 };

inline constexpr USHORT MonthDaysSum[12]{ 0,31,59,90,120,151,181,212,243,273,304,334 };

inline constexpr BYTE DayInMonthLeapYear[]
{
	0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2,  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	3,  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	4,  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	5,  5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	6,  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	7,  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	8,  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	9,  9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
	10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,
	11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11
};

inline constexpr BYTE DayInMonth[]
{
	0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2,  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	3,  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	4,  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	5,  5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	6,  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	7,  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	8,  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	9,  9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
	10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,
	11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11
};

// 是否闰年
EckInline [[nodiscard]] constexpr BOOL IsLeapYear(int iYear)
{
	return iYear % 4 == 0 && (iYear % 100 != 0 || iYear % 400 == 0);
}

// 取某月天数
EckInline [[nodiscard]] constexpr int GetMonthDays(int iYear, int iMonth)
{
	EckAssert(iMonth >= 1 && iMonth <= 12);
	if (iMonth == 2)
		if (IsLeapYear(iYear))
			return 29;
		else
			return 28;
	else
		return CeMonthDays[iMonth - 1];
}

// 取给定日期年内序数，即该年的第几天
EckInline [[nodiscard]] constexpr int GetDateDays(CEDate CeDate)
{
	int c = CeDate.byDay;
	if (IsLeapYear(CeDate.wYear))
		c += MonthDaysSumLeapYear[CeDate.byMonth - 1];
	else
		c += MonthDaysSum[CeDate.byMonth - 1];
	return c;
}

// 取给定日期年内序数，即该年的第几天
EckInline [[nodiscard]] constexpr int GetDateDays(const SYSTEMTIME& st)
{
	return GetDateDays(CEDate{ st.wYear, (BYTE)st.wMonth, (BYTE)st.wDay });
}

/// <summary>
/// 年内序数转日期
/// </summary>
/// <param name="st">wYear指定年份，wDay指定年内序数，函数返回后wMonth和wDay会被修改为常规日期</param>
EckInline constexpr void GetNumOfDaysDate(SYSTEMTIME& st)
{
	int t = st.wDay;
	for (st.wMonth = 1; st.wMonth <= 12; ++st.wMonth)
	{
		const int cDays = GetMonthDays(st.wYear, st.wMonth);
		if (t > cDays)
			t -= cDays;
		else
			break;
	}
	st.wDay = (WORD)t;
}

// 到NT时间，即从1601年1月1日0时0分0秒起到某时间经过的毫秒数
[[nodiscard]] inline constexpr ULONGLONG ToFileTimeMs(const SYSTEMTIME& st)
{
	const int dYear = st.wYear - 1601;
	const int cLeapYear = dYear / 4 - dYear / 100 + dYear / 400;
	int cDays = dYear * 365 + cLeapYear;// 年
	// 月
	if (IsLeapYear(st.wYear - 1600))
		cDays += MonthDaysSumLeapYear[st.wMonth - 1];
	else
		cDays += MonthDaysSum[st.wMonth - 1];
	// 日
	cDays += (st.wDay - 1);
	// 时分秒毫秒
	return cDays * 86400000ull +
		(st.wHour * 3600ull + st.wMinute * 60ull + st.wSecond) * 1000ull +
		st.wMilliseconds;
}

// 到NT时间，即从1601年1月1日0时0分0秒起到某时间经过的100纳秒数
EckInline [[nodiscard]] constexpr ULONGLONG ToFileTime(const SYSTEMTIME& st)
{
	return ToFileTimeMs(st) * 10000ull;
}

// 以毫秒计的NT时间到系统时间
inline [[nodiscard]] constexpr SYSTEMTIME ToSystemTimeMs(ULONGLONG ft)
{
	SYSTEMTIME st{};
	auto cDays = ft / 86400000ull;
	const auto ms = ft - cDays * 86400000ull;
	// 周
	st.wDayOfWeek = WORD((cDays + 1) % 7);
	// 年
	constexpr ULONGLONG cDays400 = 365 * 400 + 97;
	constexpr ULONGLONG cDays100 = 365 * 100 + 24;
	constexpr ULONGLONG cDays4 = 365 * 4 + 1;
	auto t = cDays;
	const auto c400 = t / cDays400;
	t -= (c400 * cDays400);
	const auto c100 = (t * 100 + 75) / 3652425;
	t -= (c100 * cDays100);
	const auto c4 = t / cDays4;
	t -= (c4 * cDays4);
	const auto dYear = c400 * 400 + c100 * 100 + c4 * 4 + (t * 100 + 75) / 36525;
	st.wYear = (WORD)(1601 + dYear);
	cDays -= (dYear * 365 + (dYear / 4 - dYear / 100 + dYear / 400));
	// 月
	if (IsLeapYear(st.wYear - 1600))
	{
		st.wMonth = DayInMonth[cDays] + 1;
		cDays -= MonthDaysSumLeapYear[st.wMonth - 1];
	}
	else
	{
		st.wMonth = DayInMonth[cDays] + 1;
		cDays -= MonthDaysSum[st.wMonth - 1];
	}
	// 日
	st.wDay = WORD(cDays + 1);
	// 时分秒毫秒
	const auto cSeconds = ms / 1000ull;
	st.wHour = WORD(cSeconds / 3600);
	st.wMinute = WORD((cSeconds % 3600) / 60);
	st.wSecond = WORD(cSeconds % 60);
	st.wMilliseconds = WORD(ms % 1000);
	return st;
}

// NT时间到系统时间
EckInline [[nodiscard]] constexpr SYSTEMTIME ToSystemTime(ULONGLONG ft)
{
	return ToSystemTimeMs(ft / 10000ull);
}

EckInline constexpr void TimeDeltaYear(SYSTEMTIME& st, int d)
{
	st.wYear += d;
}

EckInline constexpr void TimeDeltaMonth(SYSTEMTIME& st, int d)
{
	int t = st.wMonth + d;
	while (t > 12)
	{
		t -= 12;
		++st.wYear;
	}
	while (t < 1)
	{
		t += 12;
		--st.wYear;
	}
	st.wMonth = (WORD)t;
}

EckInline constexpr void TimeDeltaDay(SYSTEMTIME& st, int d)
{
	int t = st.wDay + d;
	while (t > GetMonthDays(st.wYear, st.wMonth))
	{
		t -= GetMonthDays(st.wYear, st.wMonth);
		TimeDeltaMonth(st, 1);
	}
	while (t < 1)
	{
		TimeDeltaMonth(st, -1);
		t += GetMonthDays(st.wYear, st.wMonth);
	}
	st.wDay = (WORD)t;
}

EckInline constexpr void TimeDeltaHour(SYSTEMTIME& st, LONGLONG d)
{
	LONGLONG t = st.wHour + d;
	while (t > 23)
	{
		TimeDeltaDay(st, 1);
		t -= 24;
	}
	while (t < 0)
	{
		TimeDeltaDay(st, -1);
		t += 24;
	}
	st.wHour = (WORD)t;
}

EckInline constexpr void TimeDeltaMinute(SYSTEMTIME& st, LONGLONG d)
{
	LONGLONG t = st.wMinute + d;
	while (t > 59)
	{
		TimeDeltaHour(st, 1);
		t -= 60;
	}
	while (t < 0)
	{
		TimeDeltaHour(st, -1);
		t += 60;
	}
	st.wMinute = (WORD)t;
}

EckInline constexpr void TimeDeltaSecond(SYSTEMTIME& st, LONGLONG d)
{
	LONGLONG t = st.wSecond + d;
	while (t > 59)
	{
		TimeDeltaMinute(st, 1);
		t -= 60;
	}
	while (t < 0)
	{
		TimeDeltaMinute(st, -1);
		t += 60;
	}
	st.wSecond = (WORD)t;
}

EckInline constexpr void TimeDeltaMillisecond(SYSTEMTIME& st, LONGLONG d)
{
	LONGLONG t = st.wMilliseconds + d;
	while (t > 999)
	{
		TimeDeltaSecond(st, 1);
		t -= 1000;
	}
	while (t < 0)
	{
		TimeDeltaSecond(st, -1);
		t += 1000;
	}
	st.wMilliseconds = (WORD)t;
}

EckInline [[nodiscard]] constexpr int TimeElapsedYear(const SYSTEMTIME& st1, const SYSTEMTIME& st2)
{
	return int(st2.wYear) - int(st1.wYear);
}

EckInline [[nodiscard]] constexpr int TimeElapsedMonth(const SYSTEMTIME& st1, const SYSTEMTIME& st2)
{
	return (int(st2.wYear) - int(st1.wYear)) * 12 + int(st2.wMonth) - int(st1.wMonth);
}

EckInline [[nodiscard]] constexpr int TimeElapsedDay(const SYSTEMTIME& st1, const SYSTEMTIME& st2)
{
	return (int)(ToFileTimeMs(st2) - ToFileTimeMs(st1)) / 86400000;
}

EckInline [[nodiscard]] constexpr LONGLONG TimeElapsedHour(const SYSTEMTIME& st1, const SYSTEMTIME& st2)
{
	return (ToFileTimeMs(st2) - ToFileTimeMs(st1)) / 3600000;
}

EckInline [[nodiscard]] constexpr LONGLONG TimeElapsedMinute(const SYSTEMTIME& st1, const SYSTEMTIME& st2)
{
	return (ToFileTimeMs(st2) - ToFileTimeMs(st1)) / 60000;
}

EckInline [[nodiscard]] constexpr  LONGLONG TimeElapsedSecond(const SYSTEMTIME& st1, const SYSTEMTIME& st2)
{
	return (ToFileTimeMs(st2) - ToFileTimeMs(st1)) / 1000;
}

EckInline [[nodiscard]] constexpr  LONGLONG TimeElapsedMillisecond(const SYSTEMTIME& st1, const SYSTEMTIME& st2)
{
	return (ToFileTimeMs(st2) - ToFileTimeMs(st1)) / 1;
}

// 取Unix时间戳，以毫秒计
inline [[nodiscard]] ULONGLONG GetUnixTimestamp()
{
	ULONGLONG ull;
	GetSystemTimeAsFileTime((FILETIME*)&ull);
	ull -= 116444736000000000ull;
	ull /= 10000ull;
	return ull;
}

// 以毫秒计的Unix时间戳转系统时间
inline SYSTEMTIME UnixTimestampToSystemTime(ULONGLONG ull)
{
	ull *= 10000ull;
	ull += 116444736000000000ull;
	return ToSystemTime(ull);
}
ECK_NAMESPACE_END