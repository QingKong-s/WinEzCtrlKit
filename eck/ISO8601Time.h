#pragma once
#include "DateTimeHelper.h"
#include "CRefStr.h"

ECK_NAMESPACE_BEGIN
inline BOOL Iso8601ToSystemTime(PCSTR pszIso8601, SYSTEMTIME& st)
{
	// 自动机状态
	enum class State
	{
		Year,
		AfterYear,
		Month,
		Day,
		Hour,
		Minute,
		Second,
		Millisecond,
		Accept,
	};

	// 时区调节
	enum class Timezone
	{
		None,
		UTC,
		Plus,
		Minus,
	};

	// 跟在年份之后的字段
	enum class AfterYear
	{
		None,
		Month,
		Week,
		NumOfDays,
	};

	AfterYear eAfterYear = AfterYear::None;
	Timezone eTimezone = Timezone::None;

	State eState = State::Year;
	auto p = pszIso8601;
	auto pStateBegin = pszIso8601;
	if (*p == '+' || *p == '-')
		++p;

	EckLoop()
	{
		const auto ch = *p;
		switch (eState)
		{
		case State::Year:
		{
			if (ch >= '0' && ch <= '9')
				++p;
			else if (ch == '-')
			{
				eState = State::AfterYear;
				++p;
				if (*p == 'W')
				{
					++p;
					eAfterYear = AfterYear::Week;
				}
			}
			else if (ch == 'Z' || ch == '+' || ch == '-')
			{
				if (eTimezone != Timezone::None)
					return FALSE;
				eState = State::Hour;
				switch (ch)
				{
				case 'Z':eTimezone = Timezone::UTC; break;
				case '+':eTimezone = Timezone::Plus; break;
				case '-':eTimezone = Timezone::Minus; break;
				default:ECK_UNREACHABLE;
				}
				++p;
			}
			else if (ch == '\0')
			{
				eState = State::Accept;
				++p;
			}
			else
				return FALSE;
			if (eState != State::Year)
			{
				st.wYear = (WORD)atoi(pStateBegin);
				pStateBegin = p;
			}
		}
		break;
		case State::AfterYear:// 可接月、周、顺序日
		{
			if (ch >= '0' && ch <= '9')
				++p;
			else if (ch == '-')
			{
				if (eAfterYear != AfterYear::Week)
				{
					if (eAfterYear != AfterYear::None)
						return FALSE;
					eAfterYear = AfterYear::Month;
				}
				eState = State::Day;
				++p;
			}
			else if (ch == 'Z' || ch == '+' || ch == '-')
			{
				if (eTimezone != Timezone::None)
					return FALSE;
				eState = State::Hour;
				switch (ch)
				{
				case 'Z':eTimezone = Timezone::UTC; break;
				case '+':eTimezone = Timezone::Plus; break;
				case '-':eTimezone = Timezone::Minus; break;
				default:ECK_UNREACHABLE;
				}
				++p;
			}
			else if (ch == '\0')
			{
				eState = State::Accept;
				++p;
			}
			else
				return FALSE;
			if (eState != State::AfterYear)
			{
				const int cch = (int)(p - pStateBegin);
				if (cch == 4)// 顺序日
				{
					const auto t = atoi(pStateBegin);
					if (t < 1 || t > 366)
						return FALSE;
					st.wDay = (WORD)t;
					GetNumOfDaysDate(st);
				}
				else if (cch == 3)// 周、月
				{
					if (eAfterYear == AfterYear::Week)
					{
						if (eState != State::Day)
						{
							st.wDay = ((WORD)atoi(pStateBegin) - 1) * 7;
							if (st.wDay < 1 || st.wDay > 366)
								return FALSE;
							GetNumOfDaysDate(st);
						}
						else
							st.wDayOfWeek = (WORD)atoi(pStateBegin);
					}
					else
						st.wMonth = (WORD)atoi(pStateBegin);
					pStateBegin = p;
				}
				else
					return FALSE;
			}
		}
		break;
		case State::Day:
		{
			if (ch >= '0' && ch <= '9')
				++p;
			else if (ch == 'T')
			{
				eState = State::Hour;
				++p;
			}
			else if (ch == 'Z' || ch == '+' || ch == '-')
			{
				if (eTimezone != Timezone::None)
					return FALSE;
				eState = State::Hour;
				switch (ch)
				{
				case 'Z':eTimezone = Timezone::UTC; break;
				case '+':eTimezone = Timezone::Plus; break;
				case '-':eTimezone = Timezone::Minus; break;
				default:ECK_UNREACHABLE;
				}
				++p;
			}
			else if (ch == '\0')
			{
				eState = State::Accept;
				++p;
			}
			else
				return FALSE;
			if (eState != State::Day)
			{
				if (st.wDayOfWeek)
				{
					st.wDay = (st.wDayOfWeek - 1) * 7 + (WORD)atoi(pStateBegin);
					if (st.wDay < 1 || st.wDay > 366)
						return FALSE;
					st.wDayOfWeek = 0;
					GetNumOfDaysDate(st);
				}
				else
					st.wDay = (WORD)atoi(pStateBegin);
				pStateBegin = p;
			}
		}
		break;
		case State::Hour:
		{
			BOOL bConvertToTimezones = FALSE;
			if (ch >= '0' && ch <= '9')
				++p;
			else if (ch == ':')
			{
				eState = State::Minute;
				++p;
			}
			else if (ch == 'Z' || ch == '+' || ch == '-')
			{
				if (eTimezone != Timezone::None)
					return FALSE;
				eState = State::Hour;
				bConvertToTimezones = TRUE;
				switch (ch)
				{
				case 'Z':eTimezone = Timezone::UTC; break;
				case '+':eTimezone = Timezone::Plus; break;
				case '-':eTimezone = Timezone::Minus; break;
				default:ECK_UNREACHABLE;
				}
				++p;
			}
			else if (ch == '\0')
			{
				eState = State::Accept;
				++p;
			}
			else
				return FALSE;
			if (eState != State::Hour || bConvertToTimezones)
			{
				if (eTimezone == Timezone::Plus)
					TimeDeltaHour(st, atoi(pStateBegin));
				else if (eTimezone == Timezone::Minus)
					TimeDeltaHour(st, -atoi(pStateBegin));
				else
					st.wHour = (WORD)atoi(pStateBegin);
				pStateBegin = p;
			}
		}
		break;
		case State::Minute:
		{
			if (ch >= '0' && ch <= '9')
				++p;
			else if (ch == ':')
			{
				eState = State::Second;
				++p;
			}
			else if (ch == 'Z' || ch == '+' || ch == '-')
			{
				if (eTimezone != Timezone::None)
					return FALSE;
				eState = State::Hour;
				switch (ch)
				{
				case 'Z':eTimezone = Timezone::UTC; break;
				case '+':eTimezone = Timezone::Plus; break;
				case '-':eTimezone = Timezone::Minus; break;
				default:ECK_UNREACHABLE;
				}
				++p;
			}
			else if (ch == '\0')
			{
				eState = State::Accept;
				++p;
			}
			else
				return FALSE;
			if (eState != State::Minute)
			{
				if (eTimezone != Timezone::None && eState != State::Accept)
					return FALSE;
				if (eTimezone == Timezone::Plus)
					TimeDeltaMinute(st, atoi(pStateBegin));
				else if (eTimezone == Timezone::Minus)
					TimeDeltaMinute(st, -atoi(pStateBegin));
				else
					st.wMinute = (WORD)atoi(pStateBegin);
				pStateBegin = p;
			}
		}
		break;
		case State::Second:
		{
			if (ch >= '0' && ch <= '9')
				++p;
			else if (ch == '.')
			{
				eState = State::Millisecond;
				++p;
			}
			else if (ch == 'Z' || ch == '+' || ch == '-')
			{
				if (eTimezone != Timezone::None)
					return FALSE;
				eState = State::Hour;
				switch (ch)
				{
				case 'Z':eTimezone = Timezone::UTC; break;
				case '+':eTimezone = Timezone::Plus; break;
				case '-':eTimezone = Timezone::Minus; break;
				default:ECK_UNREACHABLE;
				}
				++p;
			}
			else if (ch == '\0')
			{
				eState = State::Accept;
				++p;
			}
			else
				return FALSE;
			if (eState != State::Second)
			{
				st.wSecond = (WORD)atoi(pStateBegin);
				pStateBegin = p;
			}
		}
		break;
		case State::Millisecond:
		{
			if (ch >= '0' && ch <= '9')
				++p;
			else if (ch == 'Z' || ch == '+' || ch == '-')
			{
				if (eTimezone != Timezone::None)
					return FALSE;
				eState = State::Hour;
				switch (ch)
				{
				case 'Z':eState = State::Accept; break;
				case '+':eTimezone = Timezone::Plus; break;
				case '-':eTimezone = Timezone::Minus; break;
				default:ECK_UNREACHABLE;
				}
				++p;
			}
			else if (ch == '\0')
			{
				eState = State::Accept;
				++p;
			}
			else
				return FALSE;
			if (eState != State::Millisecond)
			{
				st.wMilliseconds = (WORD)atoi(pStateBegin);
				pStateBegin = p;
			}
		}
		break;
		case State::Accept: return TRUE;
		default: ECK_UNREACHABLE;
		}
	}
	return FALSE;
}

inline void SystemTimeToIso8601(const SYSTEMTIME& st, CRefStrA& rs, int iTzHour = 0, int iTzMinute = 0)
{
	rs.AppendFormat("%04d", st.wYear);
	if (st.wMonth)
		rs.AppendFormat("-%02d", st.wMonth);
	if (st.wDay)
		rs.AppendFormat("-%02d", st.wDay);
	if (st.wHour || st.wMinute || st.wSecond || st.wMilliseconds)
	{
		rs.AppendFormat("T%02d", st.wHour);
		if (st.wMinute)
			rs.AppendFormat(":%02d", st.wMinute);
		if (st.wSecond)
			rs.AppendFormat(":%02d", st.wSecond);
		if (st.wMilliseconds)
			rs.AppendFormat(".%03d", st.wMilliseconds);
	}
	if (iTzHour == INT_MIN)
		rs.PushBackChar('Z');
	else if (iTzHour)
	{
		if (iTzHour > 0)
			rs.AppendFormat("+%02d", iTzHour);
		else
			rs.AppendFormat("-%02d", -iTzHour);
		if (iTzMinute)
			rs.AppendFormat(":%02d", iTzMinute);
	}
}
ECK_NAMESPACE_END