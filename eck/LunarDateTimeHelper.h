#pragma once
#include "LunarDateLib.h"
#include "DateTimeHelper.h"

ECK_NAMESPACE_BEGIN
struct LunarDate
{
    WORD wYear;
    BYTE byMonth;
    BYTE byDay;
    BYTE bLeapMonth;
};

#ifdef _DEBUG
#define EckVerifyLnDate(x)          \
    EckAssert(x.wYear >= LunarDateLib::iStartYearReal &&    \
        x.wYear < LunarDateLib::iEndYearReal &&             \
        x.byMonth >= 0 &&   \
        (x.bLeapMonth ? (x.byMonth <= 13) : (x.byMonth <= 12)) &&       \
        x.bLeapMonth ? (LnGetLeapMonth(x.wYear) == x.byMonth) : TRUE && \
        x.byDay >= 1 &&     \
        x.byDay <= 30)
#else
#define EckVerifyLnDate(x) ;
#endif // _DEBUG

inline constexpr PCWSTR SixtyJiaZi[60]
{
    L"甲子",L"乙丑",L"丙寅",L"丁卯",L"戊辰",L"己巳",L"庚午",L"辛未",L"壬申",L"癸酉",
    L"甲戌",L"乙亥",L"丙子",L"丁丑",L"戊寅",L"己卯",L"庚辰",L"辛巳",L"壬午",L"癸未",
    L"甲申",L"乙酉",L"丙戌",L"丁亥",L"戊子",L"己丑",L"庚寅",L"辛卯",L"壬辰",L"癸巳",
    L"甲午",L"乙未",L"丙申",L"丁酉",L"戊戌",L"己亥",L"庚子",L"辛丑",L"壬寅",L"癸卯",
    L"甲辰",L"乙巳",L"丙午",L"丁未",L"戊申",L"己酉",L"庚戌",L"辛亥",L"壬子",L"癸丑",
    L"甲寅",L"乙卯",L"丙辰",L"丁巳",L"戊午",L"己未",L"庚申",L"辛酉",L"壬戌",L"癸亥"
};
inline constexpr PCWSTR NaYin[30]
{
    L"海中金",L"炉中火",L"大林木",L"路旁土",L"剑锋金",
    L"山头火",L"涧下水",L"城头土",L"白蜡金",L"杨柳木",
    L"泉中水",L"屋上土",L"霹雳火",L"松柏木",L"长流水",
    L"沙中金",L"山下火",L"平地木",L"壁上土",L"金箔金",
    L"覆灯火",L"天河水",L"大驿土",L"钗钏金",L"桑柘木",
    L"大溪水",L"沙中土",L"天上火",L"石榴木",L"大海水"
};
inline constexpr PCWSTR SolarTerm[24]
{
    L"小寒",L"大寒",L"立春",L"雨水",L"惊蛰",L"春分",L"清明",L"谷雨",L"立夏",L"小满",L"芒种",L"夏至",
    L"小暑",L"大暑",L"立秋",L"处暑",L"白露",L"秋分",L"寒露",L"霜降",L"立冬",L"小雪",L"大雪",L"冬至"
};
inline constexpr PCWSTR ChineseZodiac[12]{ L"鼠",L"牛",L"虎",L"兔",L"龙",L"蛇",L"马",L"羊",L"猴",L"鸡",L"狗",L"猪" };
inline constexpr PCWSTR HeavenlyStem[10]{ L"甲",L"乙",L"丙",L"丁",L"戊",L"己",L"庚",L"辛",L"壬",L"癸" };
inline constexpr PCWSTR EarthlyBranch[12]{ L"子",L"丑",L"寅",L"卯",L"辰",L"巳",L"午",L"未",L"申",L"酉",L"戌",L"亥" };

EckInlineNdCe int LnGetSpringFestivalNumber(int iYear) noexcept
{
    EckAssert(iYear >= LunarDateLib::iStartYearReal && iYear < LunarDateLib::iEndYearReal);
    return ((LunarDateLib::byMonthInfo[(iYear - LunarDateLib::iStartYear) * 3] >> 1) & 0b111111) + 1;
}

EckInlineNdCe int LnGetLeapMonth(int iYear) noexcept
{
    EckAssert(iYear >= LunarDateLib::iStartYearReal && iYear < LunarDateLib::iEndYearReal);
    const int idx = (iYear - LunarDateLib::iStartYear) * 3;
    return (LunarDateLib::byMonthInfo[idx + 1] >> 5) | ((LunarDateLib::byMonthInfo[idx] & 1) << 3);
}

EckInlineNdCe BOOL LnIsMonth30Days(LunarDate LunarDate) noexcept
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

EckInlineNdCe int LnGetMonthDays(LunarDate LunarDate) noexcept
{
    return LnIsMonth30Days(LunarDate) ? 30 : 29;
}

EckInlineNdCe int LnGetYearDays(int iYear) noexcept
{
    int c = 0;
    const int iLeapMonth = LnGetLeapMonth(iYear);
    LunarDate Date{ .wYear = (WORD)iYear,.bLeapMonth = FALSE };
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

EckNfInlineNdCe LunarDate CeToLunar(CEDate CeDate) noexcept
{
    EckVerifyCeDate(CeDate);
    LunarDate Date{ .byDay = 1 };
    int iDays;
    const int iChuYi = LnGetSpringFestivalNumber(CeDate.wYear);// 取初一
    const int iCeDays = GetDateDays(CeDate);// 取给定日期的年内序数
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
EckInlineNdCe PCWSTR NumberToHeavenlyStem(int iNum) noexcept
{
    EckAssert(iNum >= 0 && iNum < ARRAYSIZE(HeavenlyStem));
    return HeavenlyStem[iNum];
}
EckInlineNdCe int HeavenlyStemToNumber(PCWSTR pszHeavenlyStem) noexcept
{
    EckCounter(ARRAYSIZE(HeavenlyStem), i)
    {
        if (wcscmp(pszHeavenlyStem, HeavenlyStem[i]) == 0)
            return (int)i;
    }
    return 0;
}

EckInlineNdCe PCWSTR NumberToEarthlyBranch(int iNum) noexcept
{
    EckAssert(iNum >= 0 && iNum < ARRAYSIZE(EarthlyBranch));
    return EarthlyBranch[iNum];
}
EckInlineNdCe int EarthlyBranchToNumber(PCWSTR pszEarthlyBranch) noexcept
{
    EckCounter(ARRAYSIZE(EarthlyBranch), i)
    {
        if (wcscmp(pszEarthlyBranch, EarthlyBranch[i]) == 0)
            return (int)i;
    }
    return 0;
}

EckInlineNdCe PCWSTR NumberToJiaZi(int i) noexcept
{
    EckAssert(i >= 0 && i < ARRAYSIZE(SixtyJiaZi));
    return SixtyJiaZi[i];
}
EckInlineNdCe int JiaZiToNumber(PCWSTR pszJiaZi) noexcept
{
    EckCounter(ARRAYSIZE(SixtyJiaZi), i)
    {
        if (wcscmp(pszJiaZi, SixtyJiaZi[i]) == 0)
            return (int)i;
    }
    return -1;
}

EckInlineNdCe int Get60JiaZi(int iHeavenlyStem, int iEarthlyBranch) noexcept
{
    EckAssert(iHeavenlyStem >= 0 && iHeavenlyStem < ARRAYSIZE(HeavenlyStem));
    EckAssert(iEarthlyBranch >= 0 && iEarthlyBranch < ARRAYSIZE(EarthlyBranch));
    EckAssert((iHeavenlyStem % 2) ? (iEarthlyBranch % 2) : (iEarthlyBranch % 2 == 0));
    return (iEarthlyBranch / 2)/*行索引*/ * ARRAYSIZE(HeavenlyStem) + iHeavenlyStem/*列索引*/;
}
///////////////////////纳音
EckInlineNdCe int GetNaYin(int iJiaZi) noexcept
{
    if (iJiaZi % 2)
        ++iJiaZi;
    return iJiaZi / 2;
}
EckInlineNdCe int GetNaYin(PCWSTR pszJiaZi) noexcept
{
    return GetNaYin(JiaZiToNumber(pszJiaZi));
}
EckInlineNdCe PCWSTR NumberToNaYin(int iNum) noexcept
{
    EckAssert(iNum >= 0 && iNum < ARRAYSIZE(NaYin));
    return NaYin[iNum];
}
EckInlineNdCe int NaYinToNumber(PCWSTR pszNaYin) noexcept
{
    EckCounter(ARRAYSIZE(NaYin), i)
    {
        if (wcscmp(pszNaYin, NaYin[i]) == 0)
            return (int)i;
    }
    return -1;
}
///////////////////////节气
EckInlineNdCe int GetSolarTerm(CEDate Date) noexcept
{
    EckVerifyCeDate(Date);
    const int idx = (Date.wYear - LunarDateLib::iStartYear) * 3;
    const ULONGLONG ullSolarTerm =
        (ULONGLONG)LunarDateLib::wSTSource[LunarDateLib::bySTIndex[idx]] |
        (ULONGLONG)LunarDateLib::wSTSource[LunarDateLib::bySTIndex[idx + 1]] << 16 |
        (ULONGLONG)LunarDateLib::wSTSource[LunarDateLib::bySTIndex[idx + 2]] << 32;
    int nCurrSolarTerm = (ullSolarTerm & 0b11ULL) + 4;
    const int nDate = GetDateDays(Date);
    if (nDate == nCurrSolarTerm)// 判断小寒
        return 0;
    EckCounter(23, i)// 判断小寒之后的节气
    {
        if (nDate == nCurrSolarTerm)
            return i + 1;
        nCurrSolarTerm += ((ullSolarTerm >> ((i + 1) * 2) & 0b11ULL) + 14);
    }
    return -1;
}

EckInlineNdCe PCWSTR NumberToSolarTerm(int iNum) noexcept
{
    EckAssert(iNum >= 0 && iNum < ARRAYSIZE(SolarTerm));
    return SolarTerm[iNum];
}

EckInlineNdCe int SolarTermToNumber(PCWSTR pszSolarTerm) noexcept
{
    EckCounter(ARRAYSIZE(SolarTerm), i)
    {
        if (wcscmp(pszSolarTerm, SolarTerm[i]) == 0)
            return (int)i;
    }
    return -1;
}
///////////////////////属相
EckInlineNdCe int GetChineseZodiac(int iYear) noexcept
{
    return LunarDateLib::iStartYearChineseZodiac + (iYear - LunarDateLib::iStartYear) % 12;
}

EckInlineNdCe PCWSTR NumberToChineseZodiac(int iNum) noexcept
{
    EckAssert(iNum >= 0 && iNum < ARRAYSIZE(ChineseZodiac));
    return ChineseZodiac[iNum];
}

EckInlineNdCe int ChineseZodiacToNumber(PCWSTR pszChineseZodiac) noexcept
{
    EckCounter(ARRAYSIZE(ChineseZodiac), i)
    {
        if (wcscmp(pszChineseZodiac, ChineseZodiac[i]) == 0)
            return (int)i;
    }
    return -1;
}
ECK_NAMESPACE_END