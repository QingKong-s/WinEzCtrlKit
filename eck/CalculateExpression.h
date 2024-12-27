/*
* WinEzCtrlKit Library
*
* CalculateExpression.h ： 简单表达式计算
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
enum class CalcExpResult
{
	Ok,
	// 非法字符
	InvalidChar,
	// 相邻运算符
	AdjacentOp,
	// 括号不匹配
	UnmatchedParentheses,
	// 运算符错误
	OpError,
};

namespace Priv
{
	enum class CalcExpOp : char
	{
		None,
		Ln,
		Log,

		Sin,
		Cos,
		Tan,
		Cot,
		Sec,
		Csc,

		Asin,
		Acos,
		Atan,
		Acot,
		Asec,
		Acsc,

		Sqrt,
		Sinh,
		Cosh,

		Ceil,
		Floor,
		Round,
		Max,
	};

	template<class TChar>
	EckInline constexpr int CalcExpPriority(TChar ch)
	{
		switch (ch)
		{
		case '+':
		case '-':
			return 1;
		case '*':
		case '/':
		case '%':
			return 2;
		case '^':
			return 3;
		case '(':
			return INT_MAX;
		default:
			return 4;
		}
	}

	template<class TChar>
	BOOL CalcExpDoOp(std::vector<double>& vNum, std::vector<TChar>& vOp)
	{
		if (vOp.empty() || vNum.empty())
			return FALSE;
		const BOOL bIsBinary = (vOp.back() >= (TChar)CalcExpOp::Max);
		if (bIsBinary ? (vNum.size() < 2) : FALSE)
			return FALSE;
		const auto chOp = vOp.back();
		vOp.pop_back();
		const auto n2 = vNum.back();
		vNum.pop_back();
		double n1;
		if (bIsBinary)
		{
			n1 = vNum.back();
			vNum.pop_back();
		}
		else
			n1 = 0.;

		switch (chOp)
		{
		case '+':
			vNum.push_back(n1 + n2);
			break;
		case '-':
			vNum.push_back(n1 - n2);
			break;
		case '*':
			vNum.push_back(n1 * n2);
			break;
		case '/':
			vNum.push_back(n1 / n2);
			break;
		case '%':
			vNum.push_back(fmod(n1, n2));
			break;
		case '^':
			vNum.push_back(pow(n1, n2));
			break;
		case (TChar)CalcExpOp::Ln:
			vNum.push_back(log(n2));
			break;
		case (TChar)CalcExpOp::Log:
			vNum.push_back(log10(n2));
			break;
		case (TChar)CalcExpOp::Sin:
			vNum.push_back(sin(n2));
			break;
		case (TChar)CalcExpOp::Cos:
			vNum.push_back(cos(n2));
			break;
		case (TChar)CalcExpOp::Tan:
			vNum.push_back(tan(n2));
			break;
		case (TChar)CalcExpOp::Cot:
			vNum.push_back(1. / tan(n2));
			break;
		case (TChar)CalcExpOp::Sec:
			vNum.push_back(1. / cos(n2));
			break;
		case (TChar)CalcExpOp::Csc:
			vNum.push_back(1. / sin(n2));
			break;
		case (TChar)CalcExpOp::Asin:
			vNum.push_back(asin(n2));
			break;
		case (TChar)CalcExpOp::Acos:
			vNum.push_back(acos(n2));
			break;
		case (TChar)CalcExpOp::Atan:
			vNum.push_back(atan(n2));
			break;
		case (TChar)CalcExpOp::Acot:
			vNum.push_back(atan(1. / n2));
			break;
		case (TChar)CalcExpOp::Asec:
			vNum.push_back(acos(1. / n2));
			break;
		case (TChar)CalcExpOp::Acsc:
			vNum.push_back(asin(1. / n2));
			break;
		case (TChar)CalcExpOp::Sqrt:
			vNum.push_back(sqrt(n2));
			break;
		case (TChar)CalcExpOp::Sinh:
			vNum.push_back(sinh(n2));
			break;
		case (TChar)CalcExpOp::Cosh:
			vNum.push_back(cosh(n2));
			break;
		case (TChar)CalcExpOp::Ceil:
			vNum.push_back(ceil(n2));
			break;
		case (TChar)CalcExpOp::Floor:
			vNum.push_back(floor(n2));
			break;
		case (TChar)CalcExpOp::Round:
			vNum.push_back(round(n2));
			break;
		default:
			return FALSE;
		}
		return TRUE;
	}
}

inline CalcExpResult CalculateExpression(double& lfResult,
	PCWSTR pszExp, int cchExp = -1)
{
	using TChar = WCHAR;
	lfResult = 0.;
	std::vector<double> vNum{};
	std::vector<TChar> vOp{};
	BOOL bLastIsOp = TRUE;
	for (auto p = pszExp; p < pszExp + cchExp; ++p)
	{
		const auto ch = *p;
		if (ch == ' ' || ch == '\t')
			continue;
		if (iswdigit(ch))
		{
			bLastIsOp = FALSE;
			vNum.push_back(wcstod(p, (TChar**)&p));
			--p;
			continue;
		}
		else if (ch == '(')
		{
			bLastIsOp = TRUE;
			vOp.push_back(ch);
			continue;
		}
		else if (ch == ')')
		{
			bLastIsOp = FALSE;
			while (!vOp.empty() && vOp.back() != '(')
				if (!Priv::CalcExpDoOp(vNum, vOp))
					return CalcExpResult::OpError;
			if (vOp.empty())
				return CalcExpResult::UnmatchedParentheses;
			vOp.pop_back();
		}
		else if (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '%' || ch == '^')
		{
			if (bLastIsOp)
				if (ch == '+')
					continue;
				else if (ch == '-')
				{
					vNum.push_back(0.);
					bLastIsOp = FALSE;
				}
				else
					return CalcExpResult::AdjacentOp;
			if (vOp.empty() || Priv::CalcExpPriority(ch) > Priv::CalcExpPriority(vOp.back()))
			{
				vOp.push_back(ch);
				continue;
			}
			else
			{
				while (!vOp.empty() &&
					vOp.back() != '(' &&
					Priv::CalcExpPriority(ch) <= Priv::CalcExpPriority(vOp.back()))
				{
					if (!Priv::CalcExpDoOp(vNum, vOp))
						return CalcExpResult::OpError;
				}
				vOp.push_back(ch);
			}
		}
		else if (iswalpha(ch))
		{
			if (EckIsStartWithConstStringIW(p, L"Pi"))
			{
				vNum.push_back(3.14159265358979323846264338328);
				p += 1;
				bLastIsOp = FALSE;
			}
			else if (ch == 'E' || ch == 'e')
			{
				vNum.push_back(2.71828182845904523536028747135);
				bLastIsOp = FALSE;
			}
			else if (EckIsStartWithConstStringIW(p, L"Ln"))
			{
				vOp.push_back((TChar)Priv::CalcExpOp::Ln);
				p += 1;
				bLastIsOp = TRUE;
			}
			else if (EckIsStartWithConstStringIW(p, L"Log"))
			{
				vOp.push_back((TChar)Priv::CalcExpOp::Log);
				p += 2;
				bLastIsOp = TRUE;
			}
			else if (EckIsStartWithConstStringIW(p, L"Sin"))
			{
				vOp.push_back((TChar)Priv::CalcExpOp::Sin);
				p += 2;
				bLastIsOp = TRUE;
			}
			else if (EckIsStartWithConstStringIW(p, L"Cos"))
			{
				vOp.push_back((TChar)Priv::CalcExpOp::Cos);
				p += 2;
				bLastIsOp = TRUE;
			}
			else if (EckIsStartWithConstStringIW(p, L"Tan"))
			{
				vOp.push_back((TChar)Priv::CalcExpOp::Tan);
				p += 2;
				bLastIsOp = TRUE;
			}
			else if (EckIsStartWithConstStringIW(p, L"Cot"))
			{
				vOp.push_back((TChar)Priv::CalcExpOp::Cot);
				p += 2;
				bLastIsOp = TRUE;
			}
			else if (EckIsStartWithConstStringIW(p, L"Sec"))
			{
				vOp.push_back((TChar)Priv::CalcExpOp::Sec);
				p += 2;
				bLastIsOp = TRUE;
			}
			else if (EckIsStartWithConstStringIW(p, L"Csc"))
			{
				vOp.push_back((TChar)Priv::CalcExpOp::Csc);
				p += 2;
				bLastIsOp = TRUE;
			}
			else if (EckIsStartWithConstStringIW(p, L"Asin"))
			{
				vOp.push_back((TChar)Priv::CalcExpOp::Asin);
				p += 3;
				bLastIsOp = TRUE;
			}
			else if (EckIsStartWithConstStringIW(p, L"Acos"))
			{
				vOp.push_back((TChar)Priv::CalcExpOp::Acos);
				p += 3;
				bLastIsOp = TRUE;
			}
			else if (EckIsStartWithConstStringIW(p, L"Atan"))
			{
				vOp.push_back((TChar)Priv::CalcExpOp::Atan);
				p += 3;
				bLastIsOp = TRUE;
			}
			else if (EckIsStartWithConstStringIW(p, L"Acot"))
			{
				vOp.push_back((TChar)Priv::CalcExpOp::Acot);
				p += 3;
				bLastIsOp = TRUE;
			}
			else if (EckIsStartWithConstStringIW(p, L"Asec"))
			{
				vOp.push_back((TChar)Priv::CalcExpOp::Asec);
				p += 3;
				bLastIsOp = TRUE;
			}
			else if (EckIsStartWithConstStringIW(p, L"Acsc"))
			{
				vOp.push_back((TChar)Priv::CalcExpOp::Acsc);
				p += 3;
				bLastIsOp = TRUE;
			}
			else if (EckIsStartWithConstStringIW(p, L"Sqrt"))
			{
				vOp.push_back((TChar)Priv::CalcExpOp::Sqrt);
				p += 3;
				bLastIsOp = TRUE;
			}
			else if (EckIsStartWithConstStringIW(p, L"Sinh"))
			{
				vOp.push_back((TChar)Priv::CalcExpOp::Sinh);
				p += 3;
				bLastIsOp = TRUE;
			}
			else if (EckIsStartWithConstStringIW(p, L"Cosh"))
			{
				vOp.push_back((TChar)Priv::CalcExpOp::Cosh);
				p += 3;
				bLastIsOp = TRUE;
			}
			else if (EckIsStartWithConstStringIW(p, L"Ceil"))
			{
				vOp.push_back((TChar)Priv::CalcExpOp::Ceil);
				p += 3;
				bLastIsOp = TRUE;
			}
			else if (EckIsStartWithConstStringIW(p, L"Floor"))
			{
				vOp.push_back((TChar)Priv::CalcExpOp::Floor);
				p += 4;
				bLastIsOp = TRUE;
			}
			else if (EckIsStartWithConstStringIW(p, L"Round"))
			{
				vOp.push_back((TChar)Priv::CalcExpOp::Round);
				p += 4;
				bLastIsOp = TRUE;
			}
			else
				return CalcExpResult::InvalidChar;
		}
		else
			return CalcExpResult::InvalidChar;
	}
	while (!vOp.empty())
		if (!Priv::CalcExpDoOp(vNum, vOp))
			return CalcExpResult::OpError;
	if (vNum.size() != 1)
		return CalcExpResult::OpError;
	lfResult = vNum.back();
	return CalcExpResult::Ok;
}

inline CalcExpResult CalculateExpression(double& lfResult,
	PCSTR pszExp, int cchExp = -1)
{
	using TChar = CHAR;
	lfResult = 0.;
	std::vector<double> vNum{};
	std::vector<TChar> vOp{};
	BOOL bLastIsOp = TRUE;
	for (auto p = pszExp; p < pszExp + cchExp; ++p)
	{
		const auto ch = *p;
		if (ch == ' ' || ch == '\t')
			continue;
		if (iswdigit(ch))
		{
			bLastIsOp = FALSE;
			vNum.push_back(strtod(p, (TChar**)&p));
			--p;
			continue;
		}
		else if (ch == '(')
		{
			bLastIsOp = TRUE;
			vOp.push_back(ch);
			continue;
		}
		else if (ch == ')')
		{
			bLastIsOp = FALSE;
			while (!vOp.empty() && vOp.back() != '(')
				if (!Priv::CalcExpDoOp(vNum, vOp))
					return CalcExpResult::OpError;
			if (vOp.empty())
				return CalcExpResult::UnmatchedParentheses;
			vOp.pop_back();
		}
		else if (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '%' || ch == '^')
		{
			if (bLastIsOp)
				if (ch == '+')
					continue;
				else if (ch == '-')
				{
					vNum.push_back(0.);
					bLastIsOp = FALSE;
				}
				else
					return CalcExpResult::AdjacentOp;
			if (vOp.empty() || Priv::CalcExpPriority(ch) > Priv::CalcExpPriority(vOp.back()))
			{
				vOp.push_back(ch);
				continue;
			}
			else
			{
				while (!vOp.empty() &&
					vOp.back() != '(' &&
					Priv::CalcExpPriority(ch) <= Priv::CalcExpPriority(vOp.back()))
				{
					if (!Priv::CalcExpDoOp(vNum, vOp))
						return CalcExpResult::OpError;
				}
				vOp.push_back(ch);
			}
		}
		else if (iswalpha(ch))
		{
			if (EckIsStartWithConstStringIA(p, "Pi"))
			{
				vNum.push_back(3.14159265358979323846264338328);
				p += 1;
				bLastIsOp = FALSE;
			}
			else if (ch == 'E' || ch == 'e')
			{
				vNum.push_back(2.71828182845904523536028747135);
				bLastIsOp = FALSE;
			}
			else if (EckIsStartWithConstStringIA(p, "Ln"))
			{
				vOp.push_back((TChar)Priv::CalcExpOp::Ln);
				p += 1;
				bLastIsOp = TRUE;
			}
			else if (EckIsStartWithConstStringIA(p, "Log"))
			{
				vOp.push_back((TChar)Priv::CalcExpOp::Log);
				p += 2;
				bLastIsOp = TRUE;
			}
			else if (EckIsStartWithConstStringIA(p, "Sin"))
			{
				vOp.push_back((TChar)Priv::CalcExpOp::Sin);
				p += 2;
				bLastIsOp = TRUE;
			}
			else if (EckIsStartWithConstStringIA(p, "Cos"))
			{
				vOp.push_back((TChar)Priv::CalcExpOp::Cos);
				p += 2;
				bLastIsOp = TRUE;
			}
			else if (EckIsStartWithConstStringIA(p, "Tan"))
			{
				vOp.push_back((TChar)Priv::CalcExpOp::Tan);
				p += 2;
				bLastIsOp = TRUE;
			}
			else if (EckIsStartWithConstStringIA(p, "Cot"))
			{
				vOp.push_back((TChar)Priv::CalcExpOp::Cot);
				p += 2;
				bLastIsOp = TRUE;
			}
			else if (EckIsStartWithConstStringIA(p, "Sec"))
			{
				vOp.push_back((TChar)Priv::CalcExpOp::Sec);
				p += 2;
				bLastIsOp = TRUE;
			}
			else if (EckIsStartWithConstStringIA(p, "Csc"))
			{
				vOp.push_back((TChar)Priv::CalcExpOp::Csc);
				p += 2;
				bLastIsOp = TRUE;
			}
			else if (EckIsStartWithConstStringIA(p, "Asin"))
			{
				vOp.push_back((TChar)Priv::CalcExpOp::Asin);
				p += 3;
				bLastIsOp = TRUE;
			}
			else if (EckIsStartWithConstStringIA(p, "Acos"))
			{
				vOp.push_back((TChar)Priv::CalcExpOp::Acos);
				p += 3;
				bLastIsOp = TRUE;
			}
			else if (EckIsStartWithConstStringIA(p, "Atan"))
			{
				vOp.push_back((TChar)Priv::CalcExpOp::Atan);
				p += 3;
				bLastIsOp = TRUE;
			}
			else if (EckIsStartWithConstStringIA(p, "Acot"))
			{
				vOp.push_back((TChar)Priv::CalcExpOp::Acot);
				p += 3;
				bLastIsOp = TRUE;
			}
			else if (EckIsStartWithConstStringIA(p, "Asec"))
			{
				vOp.push_back((TChar)Priv::CalcExpOp::Asec);
				p += 3;
				bLastIsOp = TRUE;
			}
			else if (EckIsStartWithConstStringIA(p, "Acsc"))
			{
				vOp.push_back((TChar)Priv::CalcExpOp::Acsc);
				p += 3;
				bLastIsOp = TRUE;
			}
			else if (EckIsStartWithConstStringIA(p, "Sqrt"))
			{
				vOp.push_back((TChar)Priv::CalcExpOp::Sqrt);
				p += 3;
				bLastIsOp = TRUE;
			}
			else if (EckIsStartWithConstStringIA(p, "Sinh"))
			{
				vOp.push_back((TChar)Priv::CalcExpOp::Sinh);
				p += 3;
				bLastIsOp = TRUE;
			}
			else if (EckIsStartWithConstStringIA(p, "Cosh"))
			{
				vOp.push_back((TChar)Priv::CalcExpOp::Cosh);
				p += 3;
				bLastIsOp = TRUE;
			}
			else if (EckIsStartWithConstStringIA(p, "Ceil"))
			{
				vOp.push_back((TChar)Priv::CalcExpOp::Ceil);
				p += 3;
				bLastIsOp = TRUE;
			}
			else if (EckIsStartWithConstStringIA(p, "Floor"))
			{
				vOp.push_back((TChar)Priv::CalcExpOp::Floor);
				p += 4;
				bLastIsOp = TRUE;
			}
			else if (EckIsStartWithConstStringIA(p, "Round"))
			{
				vOp.push_back((TChar)Priv::CalcExpOp::Round);
				p += 4;
				bLastIsOp = TRUE;
			}
			else
				return CalcExpResult::InvalidChar;
		}
		else
			return CalcExpResult::InvalidChar;
	}
	while (!vOp.empty())
		if (!Priv::CalcExpDoOp(vNum, vOp))
			return CalcExpResult::OpError;
	if (vNum.size() != 1)
		return CalcExpResult::OpError;
	lfResult = vNum.back();
	return CalcExpResult::Ok;
}
ECK_NAMESPACE_END