#include "pch.h"

#include "../eck/DateTimeHelper.h"

using namespace eck;

TS_NS_BEGIN
TEST_CLASS(TsDateTimeHelper)
{
public:
    TEST_METHOD(TsLeapYear_DivisibleBy4)
    {
        Assert::IsTrue(IsLeapYear(2024));
        Assert::IsTrue(IsLeapYear(2020));
        Assert::IsTrue(IsLeapYear(2016));
    }

    TEST_METHOD(TsLeapYear_CenturyNotDivisibleBy400)
    {
        Assert::IsFalse(IsLeapYear(1900));
        Assert::IsFalse(IsLeapYear(2100));
        Assert::IsFalse(IsLeapYear(2200));
    }

    TEST_METHOD(TsLeapYear_DivisibleBy400)
    {
        Assert::IsTrue(IsLeapYear(2000));
        Assert::IsTrue(IsLeapYear(1600));
        Assert::IsTrue(IsLeapYear(2400));
    }

    TEST_METHOD(TsLeapYear_NotDivisibleBy4)
    {
        Assert::IsFalse(IsLeapYear(2023));
        Assert::IsFalse(IsLeapYear(2021));
        Assert::IsFalse(IsLeapYear(2019));
    }

    TEST_METHOD(TsMonthDays_31DayMonths)
    {
        Assert::AreEqual(31, GetMonthDays(2023, 1));  // January
        Assert::AreEqual(31, GetMonthDays(2023, 3));  // March
        Assert::AreEqual(31, GetMonthDays(2023, 5));  // May
        Assert::AreEqual(31, GetMonthDays(2023, 7));  // July
        Assert::AreEqual(31, GetMonthDays(2023, 8));  // August
        Assert::AreEqual(31, GetMonthDays(2023, 10)); // October
        Assert::AreEqual(31, GetMonthDays(2023, 12)); // December
    }

    TEST_METHOD(TsMonthDays_30DayMonths)
    {
        Assert::AreEqual(30, GetMonthDays(2023, 4));  // April
        Assert::AreEqual(30, GetMonthDays(2023, 6));  // June
        Assert::AreEqual(30, GetMonthDays(2023, 9));  // September
        Assert::AreEqual(30, GetMonthDays(2023, 11)); // November
    }

    TEST_METHOD(TsMonthDays_February_LeapYear)
    {
        Assert::AreEqual(29, GetMonthDays(2024, 2));
        Assert::AreEqual(29, GetMonthDays(2000, 2));
    }

    TEST_METHOD(TsMonthDays_February_NonLeapYear)
    {
        Assert::AreEqual(28, GetMonthDays(2023, 2));
        Assert::AreEqual(28, GetMonthDays(1900, 2));
    }

    TEST_METHOD(TsGetDateDays_JanuaryFirst)
    {
        CEDate date{ 2023, 1, 1 };
        Assert::AreEqual(1, GetDateDays(date));
    }

    TEST_METHOD(TsGetDateDays_LastDayOfYear_NonLeap)
    {
        CEDate date{ 2023, 12, 31 };
        Assert::AreEqual(365, GetDateDays(date));
    }

    TEST_METHOD(TsGetDateDays_LastDayOfYear_Leap)
    {
        CEDate date{ 2024, 12, 31 };
        Assert::AreEqual(366, GetDateDays(date));
    }

    TEST_METHOD(TsGetDateDays_February29_LeapYear)
    {
        CEDate date{ 2024, 2, 29 };
        Assert::AreEqual(60, GetDateDays(date));
    }

    TEST_METHOD(TsGetDateDays_MidYear)
    {
        CEDate date{ 2023, 7, 1 }; // July 1st
        Assert::AreEqual(182, GetDateDays(date));
    }

    TEST_METHOD(TsGetDateDays_SystemTime)
    {
        SYSTEMTIME st{ 2023, 6, 0, 15, 0, 0, 0, 0 }; // June 15
        int days = GetDateDays(st);
        Assert::IsTrue(days > 0 && days <= 366);
    }

    TEST_METHOD(TsGetNumOfDaysDate_DayOne)
    {
        SYSTEMTIME st{ 2023, 0, 0, 1, 0, 0, 0, 0 };
        GetNumOfDaysDate(st);
        Assert::AreEqual((WORD)1, st.wMonth);
        Assert::AreEqual((WORD)1, st.wDay);
    }

    TEST_METHOD(TsGetNumOfDaysDate_Day365)
    {
        SYSTEMTIME st{ 2023, 0, 0, 365, 0, 0, 0, 0 };
        GetNumOfDaysDate(st);
        Assert::AreEqual((WORD)12, st.wMonth);
        Assert::AreEqual((WORD)31, st.wDay);
    }

    TEST_METHOD(TsGetNumOfDaysDate_Day60_LeapYear)
    {
        SYSTEMTIME st{ 2024, 0, 0, 60, 0, 0, 0, 0 };
        GetNumOfDaysDate(st);
        Assert::AreEqual((WORD)2, st.wMonth);
        Assert::AreEqual((WORD)29, st.wDay);
    }

    TEST_METHOD(TsGetNumOfDaysDate_Day60_NonLeapYear)
    {
        SYSTEMTIME st{ 2023, 0, 0, 60, 0, 0, 0, 0 };
        GetNumOfDaysDate(st);
        Assert::AreEqual((WORD)3, st.wMonth);
        Assert::AreEqual((WORD)1, st.wDay);
    }

    TEST_METHOD(TsSystemTimeToNtTime_Epoch)
    {
        SYSTEMTIME st{ 1601, 1, 0, 1, 0, 0, 0, 0 };
        ULONGLONG nt = SystemTimeToNtTime(st);
        Assert::AreEqual(0ull, nt);
    }

    TEST_METHOD(TsNtTimeToSystemTime_Epoch)
    {
        SYSTEMTIME st = NtTimeToSystemTime(0ull);
        Assert::AreEqual((WORD)1601, st.wYear);
        Assert::AreEqual((WORD)1, st.wMonth);
        Assert::AreEqual((WORD)1, st.wDay);
    }

    TEST_METHOD(TsSystemTimeRoundTrip)
    {
        SYSTEMTIME original{ 2023, 6, 0, 15, 14, 30, 45, 123 };
        ULONGLONG nt = SystemTimeToNtTime(original);
        SYSTEMTIME result = NtTimeToSystemTime(nt);

        Assert::AreEqual(original.wYear, result.wYear);
        Assert::AreEqual(original.wMonth, result.wMonth);
        Assert::AreEqual(original.wDay, result.wDay);
        Assert::AreEqual(original.wHour, result.wHour);
        Assert::AreEqual(original.wMinute, result.wMinute);
        Assert::AreEqual(original.wSecond, result.wSecond);
        Assert::AreEqual(original.wMilliseconds, result.wMilliseconds);
    }

    TEST_METHOD(TsSystemTimeToNtTimeMs_Y2K)
    {
        SYSTEMTIME st{ 2000, 1, 0, 1, 0, 0, 0, 0 };
        ULONGLONG ms = SystemTimeToNtTimeMs(st);
        Assert::IsTrue(ms > 0);
    }

    TEST_METHOD(TsTimeDeltaYear_Forward)
    {
        SYSTEMTIME st{ 2023, 6, 0, 15, 12, 30, 45, 0 };
        TimeDeltaYear(st, 5);
        Assert::AreEqual((WORD)2028, st.wYear);
    }

    TEST_METHOD(TsTimeDeltaYear_Backward)
    {
        SYSTEMTIME st{ 2023, 6, 0, 15, 12, 30, 45, 0 };
        TimeDeltaYear(st, -10);
        Assert::AreEqual((WORD)2013, st.wYear);
    }

    TEST_METHOD(TsTimeDeltaMonth_SameYear)
    {
        SYSTEMTIME st{ 2023, 6, 0, 15, 12, 30, 45, 0 };
        TimeDeltaMonth(st, 3);
        Assert::AreEqual((WORD)2023, st.wYear);
        Assert::AreEqual((WORD)9, st.wMonth);
    }

    TEST_METHOD(TsTimeDeltaMonth_CrossYear)
    {
        SYSTEMTIME st{ 2023, 11, 0, 15, 12, 30, 45, 0 };
        TimeDeltaMonth(st, 3);
        Assert::AreEqual((WORD)2024, st.wYear);
        Assert::AreEqual((WORD)2, st.wMonth);
    }

    TEST_METHOD(TsTimeDeltaMonth_Backward)
    {
        SYSTEMTIME st{ 2023, 3, 0, 15, 12, 30, 45, 0 };
        TimeDeltaMonth(st, -5);
        Assert::AreEqual((WORD)2022, st.wYear);
        Assert::AreEqual((WORD)10, st.wMonth);
    }

    TEST_METHOD(TsTimeDeltaDay_SameMonth)
    {
        SYSTEMTIME st{ 2023, 6, 0, 15, 12, 30, 45, 0 };
        TimeDeltaDay(st, 5);
        Assert::AreEqual((WORD)6, st.wMonth);
        Assert::AreEqual((WORD)20, st.wDay);
    }

    TEST_METHOD(TsTimeDeltaDay_CrossMonth)
    {
        SYSTEMTIME st{ 2023, 1, 0, 30, 12, 30, 45, 0 };
        TimeDeltaDay(st, 5);
        Assert::AreEqual((WORD)2, st.wMonth);
        Assert::AreEqual((WORD)4, st.wDay);
    }

    TEST_METHOD(TsTimeDeltaDay_CrossYear)
    {
        SYSTEMTIME st{ 2023, 12, 0, 30, 12, 30, 45, 0 };
        TimeDeltaDay(st, 5);
        Assert::AreEqual((WORD)2024, st.wYear);
        Assert::AreEqual((WORD)1, st.wMonth);
        Assert::AreEqual((WORD)4, st.wDay);
    }

    TEST_METHOD(TsTimeDeltaHour_SameDay)
    {
        SYSTEMTIME st{ 2023, 6, 0, 15, 10, 30, 45, 0 };
        TimeDeltaHour(st, 5);
        Assert::AreEqual((WORD)15, st.wHour);
    }

    TEST_METHOD(TsTimeDeltaHour_CrossDay)
    {
        SYSTEMTIME st{ 2023, 6, 0, 15, 22, 30, 45, 0 };
        TimeDeltaHour(st, 5);
        Assert::AreEqual((WORD)16, st.wDay);
        Assert::AreEqual((WORD)3, st.wHour);
    }

    TEST_METHOD(TsTimeDeltaMinute_SameHour)
    {
        SYSTEMTIME st{ 2023, 6, 0, 15, 10, 30, 45, 0 };
        TimeDeltaMinute(st, 20);
        Assert::AreEqual((WORD)50, st.wMinute);
    }

    TEST_METHOD(TsTimeDeltaMinute_CrossHour)
    {
        SYSTEMTIME st{ 2023, 6, 0, 15, 10, 50, 45, 0 };
        TimeDeltaMinute(st, 20);
        Assert::AreEqual((WORD)11, st.wHour);
        Assert::AreEqual((WORD)10, st.wMinute);
    }

    TEST_METHOD(TsTimeDeltaSecond_SameMinute)
    {
        SYSTEMTIME st{ 2023, 6, 0, 15, 10, 30, 45, 0 };
        TimeDeltaSecond(st, 10);
        Assert::AreEqual((WORD)55, st.wSecond);
    }

    TEST_METHOD(TsTimeDeltaSecond_CrossMinute)
    {
        SYSTEMTIME st{ 2023, 6, 0, 15, 10, 30, 55, 0 };
        TimeDeltaSecond(st, 10);
        Assert::AreEqual((WORD)31, st.wMinute);
        Assert::AreEqual((WORD)5, st.wSecond);
    }

    TEST_METHOD(TsTimeDeltaMillisecond_SameSecond)
    {
        SYSTEMTIME st{ 2023, 6, 0, 15, 10, 30, 45, 100 };
        TimeDeltaMillisecond(st, 200);
        Assert::AreEqual((WORD)300, st.wMilliseconds);
    }

    TEST_METHOD(TsTimeDeltaMillisecond_CrossSecond)
    {
        SYSTEMTIME st{ 2023, 6, 0, 15, 10, 30, 45, 900 };
        TimeDeltaMillisecond(st, 200);
        Assert::AreEqual((WORD)46, st.wSecond);
        Assert::AreEqual((WORD)100, st.wMilliseconds);
    }

    TEST_METHOD(TsTimeElapsedYear)
    {
        SYSTEMTIME st1{ 2020, 1, 0, 1, 0, 0, 0, 0 };
        SYSTEMTIME st2{ 2025, 1, 0, 1, 0, 0, 0, 0 };
        Assert::AreEqual(5, TimeElapsedYear(st1, st2));
    }

    TEST_METHOD(TsTimeElapsedMonth_SameYear)
    {
        SYSTEMTIME st1{ 2023, 3, 0, 1, 0, 0, 0, 0 };
        SYSTEMTIME st2{ 2023, 8, 0, 1, 0, 0, 0, 0 };
        Assert::AreEqual(5, TimeElapsedMonth(st1, st2));
    }

    TEST_METHOD(TsTimeElapsedMonth_DifferentYears)
    {
        SYSTEMTIME st1{ 2022, 10, 0, 1, 0, 0, 0, 0 };
        SYSTEMTIME st2{ 2023, 3, 0, 1, 0, 0, 0, 0 };
        Assert::AreEqual(5, TimeElapsedMonth(st1, st2));
    }

    TEST_METHOD(TsTimeElapsedDay)
    {
        SYSTEMTIME st1{ 2023, 1, 0, 1, 0, 0, 0, 0 };
        SYSTEMTIME st2{ 2023, 1, 0, 8, 0, 0, 0, 0 };
        Assert::AreEqual(7, TimeElapsedDay(st1, st2));
    }

    TEST_METHOD(TsTimeElapsedHour)
    {
        SYSTEMTIME st1{ 2023, 6, 0, 15, 10, 0, 0, 0 };
        SYSTEMTIME st2{ 2023, 6, 0, 15, 15, 0, 0, 0 };
        Assert::AreEqual(5LL, TimeElapsedHour(st1, st2));
    }

    TEST_METHOD(TsTimeElapsedMinute)
    {
        SYSTEMTIME st1{ 2023, 6, 0, 15, 10, 30, 0, 0 };
        SYSTEMTIME st2{ 2023, 6, 0, 15, 11, 0, 0, 0 };
        Assert::AreEqual(30LL, TimeElapsedMinute(st1, st2));
    }

    TEST_METHOD(TsTimeElapsedSecond)
    {
        SYSTEMTIME st1{ 2023, 6, 0, 15, 10, 30, 0, 0 };
        SYSTEMTIME st2{ 2023, 6, 0, 15, 10, 30, 45, 0 };
        Assert::AreEqual(45LL, TimeElapsedSecond(st1, st2));
    }

    TEST_METHOD(TsTimeElapsedMillisecond)
    {
        SYSTEMTIME st1{ 2023, 6, 0, 15, 10, 30, 0, 0 };
        SYSTEMTIME st2{ 2023, 6, 0, 15, 10, 30, 0, 500 };
        Assert::AreEqual(500LL, TimeElapsedMillisecond(st1, st2));
    }

    TEST_METHOD(TsUnixTimestampToSystemTime_Epoch)
    {
        SYSTEMTIME st = UnixTimestampToSystemTimeMs(0);
        Assert::AreEqual((WORD)1970, st.wYear);
        Assert::AreEqual((WORD)1, st.wMonth);
        Assert::AreEqual((WORD)1, st.wDay);
    }

    TEST_METHOD(TsUnixTimestampToSystemTime_Y2K)
    {
        // Unix timestamp for 2000-01-01 00:00:00 UTC
        ULONGLONG timestamp = 946684800000ull;
        SYSTEMTIME st = UnixTimestampToSystemTimeMs(timestamp);
        Assert::AreEqual((WORD)2000, st.wYear);
        Assert::AreEqual((WORD)1, st.wMonth);
        Assert::AreEqual((WORD)1, st.wDay);
    }

    TEST_METHOD(TsGetUnixTimestamp_NotZero)
    {
        ULONGLONG ts = GetUnixTimestampMs();
        // Should be a reasonable timestamp (after year 2020)
        Assert::IsTrue(ts > 1577836800000ull); // 2020-01-01
    }

    TEST_METHOD(TsLeapYear_FebTransition)
    {
        SYSTEMTIME st{ 2024, 2, 0, 28, 23, 59, 59, 999 };
        TimeDeltaMillisecond(st, 1);
        Assert::AreEqual((WORD)29, st.wDay);
        Assert::AreEqual((WORD)2, st.wMonth);
    }

    TEST_METHOD(TsNonLeapYear_FebToMarch)
    {
        SYSTEMTIME st{ 2023, 2, 0, 28, 23, 59, 59, 999 };
        TimeDeltaMillisecond(st, 1);
        Assert::AreEqual((WORD)1, st.wDay);
        Assert::AreEqual((WORD)3, st.wMonth);
    }

    TEST_METHOD(TsYearBoundary)
    {
        SYSTEMTIME st{ 2023, 12, 0, 31, 23, 59, 59, 999 };
        TimeDeltaMillisecond(st, 1);
        Assert::AreEqual((WORD)2024, st.wYear);
        Assert::AreEqual((WORD)1, st.wMonth);
        Assert::AreEqual((WORD)1, st.wDay);
    }

    TEST_METHOD(TsNegativeTimeDelta)
    {
        SYSTEMTIME st{ 2023, 1, 0, 1, 0, 0, 0, 0 };
        TimeDeltaDay(st, -1);
        Assert::AreEqual((WORD)2022, st.wYear);
        Assert::AreEqual((WORD)12, st.wMonth);
        Assert::AreEqual((WORD)31, st.wDay);
    }
};
TS_NS_END