#pragma once
#include "CWnd.h"
#include "CComboBox.h"
#include "LunarDateLib.h"
#include "MathHelper.h"

#include <chrono>
#include <bitset>

ECK_NAMESPACE_BEGIN

struct CEDate
{
	WORD wYear;
	BYTE byMonth;
	BYTE byDay;
};

struct LunarDate
{
	WORD wYear;
	BYTE byMonth;
	BYTE byDay;
	BYTE bLeapMonth;
};

constexpr
int 
IDC_CB_Year = 101,
IDC_CB_Month = 102;


class CLunarCalendar :public CWnd
{
private:
	CComboBox m_CBYear{};
	CComboBox m_CBMonth{};

	static ATOM m_atomLunarCalendar;

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void PaintUnit(RECT rc)
	{

	}
public:
	ATOM RegisterWndClass(HINSTANCE hInstance);

	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = NULL) override
	{
		dwStyle |= WS_CHILD;
		m_hWnd = CreateWindowExW(dwExStyle, WCN_LUNARCALENDAR, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, g_hInstance, this);
		
		return m_hWnd;
	}
};

#ifdef _DEBUG
#define EckVerifyCeDate(x) EckAssert(x.byMonth >= 1 && x.byMonth <= 12 && \
									x.byDay >= 1 && x.byDay <= CeGetMonthDays(x.wYear, x.byMonth))
#define EckVerifyLnDate(x) EckAssert(x.wYear >= LunarDateLib::iStartYearReal && x.wYear < LunarDateLib::iEndYearReal && \
									x.byMonth >= 0 && (x.bLeapMonth ? (x.byMonth <= 13) : (x.byMonth <= 12)) && \
									x.bLeapMonth ? (LnGetLeapMonth(x.wYear) == x.byMonth) : TRUE && \
									x.byDay >= 1 && x.byDay <= 30)
#else
#define EckVerifyCeDate(x) ;
#define EckVerifyLnDate(x) ;
#endif // _DEBUG

inline constexpr BYTE c_CeMonthDays[12]{ 31,0,31,30,31,30,31,31,30,31,30,31 };
inline constexpr PCWSTR c_60JiaZi[60]
{
	L"甲子",L"乙丑",L"丙寅",L"丁卯",L"戊辰",L"己巳",L"庚午",L"辛未",L"壬申",L"癸酉",
	L"甲戌",L"乙亥",L"丙子",L"丁丑",L"戊寅",L"己卯",L"庚辰",L"辛巳",L"壬午",L"癸未",
	L"甲申",L"乙酉",L"丙戌",L"丁亥",L"戊子",L"己丑",L"庚寅",L"辛卯",L"壬辰",L"癸巳",
	L"甲午",L"乙未",L"丙申",L"丁酉",L"戊戌",L"己亥",L"庚子",L"辛丑",L"壬寅",L"癸卯",
	L"甲辰",L"乙巳",L"丙午",L"丁未",L"戊申",L"己酉",L"庚戌",L"辛亥",L"壬子",L"癸丑",
	L"甲寅",L"乙卯",L"丙辰",L"丁巳",L"戊午",L"己未",L"庚申",L"辛酉",L"壬戌",L"癸亥"
};
inline constexpr PCWSTR c_NaYin[30]
{
	L"海中金",L"炉中火",L"大林木",L"路旁土",L"剑锋金",
	L"山头火",L"涧下水",L"城头土",L"白蜡金",L"杨柳木",
	L"泉中水",L"屋上土",L"霹雳火",L"松柏木",L"长流水",
	L"沙中金",L"山下火",L"平地木",L"壁上土",L"金箔金",
	L"覆灯火",L"天河水",L"大驿土",L"钗钏金",L"桑柘木",
	L"大溪水",L"沙中土",L"天上火",L"石榴木",L"大海水"
};
inline constexpr PCWSTR c_JieQi[24]
{
	L"小寒",L"大寒",L"立春",L"雨水",L"惊蛰",L"春分",L"清明",L"谷雨",L"立夏",L"小满",L"芒种",L"夏至",
	L"小暑",L"大暑",L"立秋",L"处暑",L"白露",L"秋分",L"寒露",L"霜降",L"立冬",L"小雪",L"大雪",L"冬至"
};
inline constexpr PCWSTR c_ShuXiang[12]{ L"鼠",L"牛",L"虎",L"兔",L"龙",L"蛇",L"马",L"羊",L"猴",L"鸡",L"狗",L"猪" };
inline constexpr PCWSTR c_TianGan[10]{ L"甲",L"乙",L"丙",L"丁",L"戊",L"己",L"庚",L"辛",L"壬",L"癸" };
inline constexpr PCWSTR c_DiZhi[12]{ L"子",L"丑",L"寅",L"卯",L"辰",L"巳",L"午",L"未",L"申",L"酉",L"戌",L"亥" };

EckInline constexpr BOOL CeIsLeapYear(int iYear)
{
#if ECKCXX20
	return std::chrono::year(iYear).is_leap();
#else
	return iYear % 4 == 0 && (iYear % 100 != 0 || iYear % 400 == 0);
#endif
}

EckInline constexpr int CeGetMonthDays(int iYear, int iMonth)
{
	EckAssert(iMonth >= 1 && iMonth <= 12);
	if (iMonth == 2)
		if (CeIsLeapYear(iYear))
			return 29;
		else
			return 28;
	else ECKLIKELY
		return c_CeMonthDays[iMonth - 1];
}

EckInline constexpr int LnGetSpringFestivalNum(int iYear)
{
	EckAssert(iYear >= LunarDateLib::iStartYearReal && iYear < LunarDateLib::iEndYearReal);
	return ((LunarDateLib::byMonthInfo[(iYear - LunarDateLib::iStartYear) * 3] >> 1) & 0b111111) + 1;
}

EckInline constexpr int LnGetLeapMonth(int iYear)
{
	EckAssert(iYear >= LunarDateLib::iStartYearReal && iYear < LunarDateLib::iEndYearReal);
	const int idx = (iYear - LunarDateLib::iStartYear) * 3;
	return (LunarDateLib::byMonthInfo[idx + 1] >> 5) | ((LunarDateLib::byMonthInfo[idx] & 1) << 3);
}

EckInline constexpr BOOL LnIsMonth30Days(LunarDate LunarDate)
{
	EckVerifyLnDate(LunarDate);
	const int idx = (LunarDate.wYear - LunarDateLib::iStartYear) * 3;
	const int iLeapMonth = LnGetLeapMonth(LunarDate.wYear);
	if (LunarDate.bLeapMonth)
		++LunarDate.byMonth;
	else if (iLeapMonth && LunarDate.byMonth > iLeapMonth)
		++LunarDate.byMonth;

	if (LunarDate.byMonth > 8)
		return (LunarDateLib::byMonthInfo[idx + 1] >> (LunarDate.byMonth - 9)) & 1;
	else
		return (LunarDateLib::byMonthInfo[idx + 2] >> (LunarDate.byMonth - 1)) & 1;
}

EckInline constexpr int LnGetMonthDays(LunarDate LunarDate)
{
	return LnIsMonth30Days(LunarDate) ? 30 : 29;
}

EckInline constexpr int LnGetYearDays(int iYear)
{
	int c = 0;
	const int iLeapMonth = LnGetLeapMonth(iYear);
#if ECKCXX20
	LunarDate Date{ .wYear = (WORD)iYear,.bLeapMonth = FALSE };
#else
	LunarDate Date{ (WORD)iYear,0,0,FALSE };
#endif
	for (Date.byMonth = 1; Date.byMonth <= 12; ++Date.byMonth)
		c += LnGetMonthDays(Date);
	if (iLeapMonth)
	{
		Date.bLeapMonth = TRUE;
		Date.byMonth = iLeapMonth;
		c += LnGetMonthDays(Date);
	}
	return c;
}

EckInline constexpr int CeGetDateDays(CEDate CeDate)
{
	int c = CeDate.byDay;
	EckCounter(CeDate.byMonth - 1, i)
		c += CeGetMonthDays(CeDate.wYear, i + 1);
	return c;
}

inline constexpr LunarDate CeToLunar(CEDate CeDate)
{
	EckVerifyCeDate(CeDate);
#if ECKCXX20
	LunarDate Date{ .byDay = 1 };
	int iDays;
#else
	LunarDate Date{ 0,0,1,FALSE };
	int iDays = 0;
#endif
	const int iChuYi = LnGetSpringFestivalNum(CeDate.wYear);// 取初一
	const int iCeDays = CeGetDateDays(CeDate);// 取给定日期的年内序数
	int c = iChuYi;
	if (iCeDays < iChuYi)// 需要到上一年寻找
	{
		Date.wYear = CeDate.wYear - 1;
		Date.bLeapMonth = FALSE;
		const int iLeapMonth = LnGetLeapMonth(CeDate.wYear);
		for (Date.byMonth = 12; Date.byMonth >= 1; --Date.byMonth)
		{
			if (Date.byMonth == iLeapMonth)
			{
				Date.bLeapMonth = TRUE;
				iDays = LnGetMonthDays(Date);
				if (c - iDays > iCeDays)
					c -= iDays;
				else
					break;
				Date.bLeapMonth = FALSE;
			}
			iDays = LnGetMonthDays(Date);
			if (c - iDays > iCeDays)
				c -= iDays;
			else
				break;
		}

		Date.byDay = iDays - (c - iCeDays) + 1;
		return Date;
	}
	else// 需要在本年寻找
	{
		Date.wYear = CeDate.wYear;
		Date.bLeapMonth = FALSE;
		const int iLeapMonth = LnGetLeapMonth(CeDate.wYear);
		for (Date.byMonth = 1; Date.byMonth <= 12; ++Date.byMonth)
		{
			if (Date.byMonth == iLeapMonth)
			{
				Date.bLeapMonth = TRUE;
				iDays = LnGetMonthDays(Date);
				if (c + iDays < iCeDays)
					c += iDays;
				else
					break;
				Date.bLeapMonth = FALSE;
			}
			iDays = LnGetMonthDays(Date);
			if (c + iDays < iCeDays)
				c += iDays;
			else
				break;
		}

		Date.byDay = iCeDays - c;
		return Date;
	}
}
///////////////////////天干地支
EckInline constexpr PCWSTR NumToTianGan(int iNum)
{
	EckAssert(iNum >= 0 && iNum < ARRAYSIZE(c_TianGan));
	return c_TianGan[iNum];
}

EckInline constexpr int TianGanToNum(PCWSTR pszTianGan)
{
	EckCounter(ARRAYSIZE(c_TianGan), i)
	{
		if (wcscmp(pszTianGan, c_TianGan[i]) == 0)
			return (int)i;
	}
	return 0;
}

EckInline constexpr PCWSTR NumToDiZhi(int iNum)
{
	EckAssert(iNum >= 0 && iNum < ARRAYSIZE(c_DiZhi));
	return c_DiZhi[iNum];
}

EckInline constexpr int DiZhiToNum(PCWSTR pszDiZhi)
{
	EckCounter(ARRAYSIZE(c_DiZhi), i)
	{
		if (wcscmp(pszDiZhi, c_DiZhi[i]) == 0)
			return (int)i;
	}
	return 0;
}

EckInline constexpr PCWSTR NumToJiaZi(int i)
{
	EckAssert(i >= 0 && i < ARRAYSIZE(c_60JiaZi));
	return c_60JiaZi[i];
}

EckInline constexpr int JiaZiToNum(PCWSTR pszJiaZi)
{
	EckCounter(ARRAYSIZE(c_60JiaZi), i)
	{
		if (wcscmp(pszJiaZi, c_60JiaZi[i]) == 0)
			return (int)i;
	}
	return -1;
}

EckInline constexpr int Get60JiaZi(int iTianGan, int iDiZhi)
{
	EckAssert(iTianGan >= 0 && iTianGan < ARRAYSIZE(c_TianGan));
	EckAssert(iDiZhi >= 0 && iDiZhi < ARRAYSIZE(c_DiZhi));
	EckAssert((iTianGan % 2) ? (iDiZhi % 2) : (iDiZhi % 2 == 0));
	return (iDiZhi / 2)/*行索引*/ * ARRAYSIZE(c_TianGan) + iTianGan/*列索引*/;
}
///////////////////////纳音
EckInline constexpr int GetNaYin(int iJiaZi)
{
	if (iJiaZi % 2)
		++iJiaZi;
	return iJiaZi / 2;
}

EckInline constexpr int GetNaYin(PCWSTR pszJiaZi)
{
	return GetNaYin(JiaZiToNum(pszJiaZi));
}

EckInline constexpr PCWSTR NumToNaYin(int iNum)
{
	EckAssert(iNum >= 0 && iNum < ARRAYSIZE(c_NaYin));
	return c_NaYin[iNum];
}

EckInline constexpr int NaYinToNum(PCWSTR pszNaYin)
{
	EckCounter(ARRAYSIZE(c_NaYin), i)
	{
		if (wcscmp(pszNaYin, c_NaYin[i]) == 0)
			return (int)i;
	}
	return -1;
}
///////////////////////节气
EckInline constexpr int GetJieQi(CEDate Date)
{
	EckVerifyCeDate(Date);
	const int idx = (Date.wYear - LunarDateLib::iStartYear) * 3;
	ULONGLONG ullJieQi =
		(ULONGLONG)LunarDateLib::wSTSource[LunarDateLib::bySTIndex[idx]] |
		(ULONGLONG)LunarDateLib::wSTSource[LunarDateLib::bySTIndex[idx + 1]] << 16 |
		(ULONGLONG)LunarDateLib::wSTSource[LunarDateLib::bySTIndex[idx + 2]] << 32;
	int nCurrJieQi = (ullJieQi & 0b11ULL) + 4;
	const int nDate = CeGetDateDays(Date);
	if (nDate == nCurrJieQi)// 判断小寒
		return 0;
	EckCounter(23, i)// 判断小寒之后的节气
	{
		if (nDate == nCurrJieQi)
			return i + 1;
		nCurrJieQi += ((ullJieQi >> ((i + 1) * 2) & 0b11ULL) + 14);
	}
	return -1;
}

EckInline constexpr PCWSTR NumToJieQi(int iNum)
{
	EckAssert(iNum >= 0 && iNum < ARRAYSIZE(c_JieQi));
	return c_JieQi[iNum];
}

EckInline constexpr int JieQiToNum(PCWSTR pszJieQi)
{
	EckCounter(ARRAYSIZE(c_JieQi), i)
	{
		if (wcscmp(pszJieQi, c_JieQi[i]) == 0)
			return (int)i;
	}
	return -1;
}
///////////////////////属相
EckInline constexpr int GetShuXiang(int iYear)
{
	return LunarDateLib::iStartYearShuXiang + (iYear - LunarDateLib::iStartYear) % 12;
}

EckInline constexpr PCWSTR NumToShuXiang(int iNum)
{
	EckAssert(iNum >= 0 && iNum < ARRAYSIZE(c_ShuXiang));
	return c_ShuXiang[iNum];
}

EckInline constexpr int ShuXiangToNum(PCWSTR pszShuXiang)
{
	EckCounter(ARRAYSIZE(c_ShuXiang), i)
	{
		if (wcscmp(pszShuXiang, c_ShuXiang[i]) == 0)
			return (int)i;
	}
	return -1;
}
ECK_NAMESPACE_END