#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
enum class CalcExpResult
{
	Ok,
	InvalidChar,// 非法字符
	AdjacentOp,	// 相邻运算符
	UnmatchedParentheses,// 括号不匹配
	OpError,	// 运算符错误
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

	struct CalcExpFuncSym
	{
		WCHAR szNameW[6];
		char szNameA[6];
		CalcExpOp eOp;
		SCHAR cchName;
	};
#define ECKPRIV_CALCEXP_FUNCNAME(s) L#s, #s, CalcExpOp::##s, ARRAYSIZE(#s) - 1
	constexpr inline CalcExpFuncSym CalcExpFuncList[]
	{
		{ ECKPRIV_CALCEXP_FUNCNAME(Ln) },
		{ ECKPRIV_CALCEXP_FUNCNAME(Log) },
		{ ECKPRIV_CALCEXP_FUNCNAME(Sin) },
		{ ECKPRIV_CALCEXP_FUNCNAME(Cos) },
		{ ECKPRIV_CALCEXP_FUNCNAME(Tan) },
		{ ECKPRIV_CALCEXP_FUNCNAME(Cot) },
		{ ECKPRIV_CALCEXP_FUNCNAME(Sec) },
		{ ECKPRIV_CALCEXP_FUNCNAME(Csc) },
		{ ECKPRIV_CALCEXP_FUNCNAME(Asin) },
		{ ECKPRIV_CALCEXP_FUNCNAME(Acos) },
		{ ECKPRIV_CALCEXP_FUNCNAME(Atan) },
		{ ECKPRIV_CALCEXP_FUNCNAME(Acot) },
		{ ECKPRIV_CALCEXP_FUNCNAME(Asec) },
		{ ECKPRIV_CALCEXP_FUNCNAME(Acsc) },
		{ ECKPRIV_CALCEXP_FUNCNAME(Sqrt) },
		{ ECKPRIV_CALCEXP_FUNCNAME(Sinh) },
		{ ECKPRIV_CALCEXP_FUNCNAME(Cosh) },
		{ ECKPRIV_CALCEXP_FUNCNAME(Ceil) },
		{ ECKPRIV_CALCEXP_FUNCNAME(Floor) },
		{ ECKPRIV_CALCEXP_FUNCNAME(Round) },
	};
#undef ECKPRIV_CALCEXP_FUNCNAME

	struct CalcExpConstSym
	{
		WCHAR szNameW[3];
		char szNameA[3];
		SCHAR cchName;
		double lfVal;
	};
#define ECKPRIV_CALCEXP_CONSTNAME(s) L#s, #s, ARRAYSIZE(#s) - 1
	constexpr inline CalcExpConstSym CalcExpConstList[]
	{
		{ ECKPRIV_CALCEXP_CONSTNAME(Pi), 3.141592653589793 },
		{ ECKPRIV_CALCEXP_CONSTNAME(E), 2.718281828459045 },
	};
#undef ECKPRIV_CALCEXP_CONSTNAME

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
			return 5;
		default:
			return 4;
		}
	}

	template<class TChar>
	BOOL CalcExpDoOp(std::vector<double>& vNum, std::vector<TChar>& vOp)
	{
		if (vOp.empty() || vNum.empty())
			return FALSE;
		const auto bIsBinary = (vOp.back() >= (TChar)CalcExpOp::Max);
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

inline CalcExpResult CalculateExpression(_Out_ double& lfResult,
	_In_ PCWSTR pszExp, int cchExp = -1)
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
			for (const auto& e : Priv::CalcExpConstList)
				if (EckIsStartWithConstStringIW(p, e.szNameW))
				{
					vNum.push_back(e.lfVal);
					p += (e.cchName - 1);
					bLastIsOp = FALSE;
					goto ExitSearchSym;
				}
			for (const auto& e : Priv::CalcExpFuncList)
				if (EckIsStartWithConstStringIW(p, e.szNameW))
				{
					vOp.push_back((TChar)e.eOp);
					p += (e.cchName - 1);
					bLastIsOp = TRUE;
					goto ExitSearchSym;
				}
			return CalcExpResult::InvalidChar;
		ExitSearchSym:;
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

inline CalcExpResult CalculateExpression(_Out_ double& lfResult,
	_In_ PCSTR pszExp, int cchExp = -1)
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
		if (isdigit(ch))
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
		else if (isalpha(ch))
		{
			for (const auto& e : Priv::CalcExpConstList)
				if (EckIsStartWithConstStringIA(p, e.szNameA))
				{
					vNum.push_back(e.lfVal);
					p += (e.cchName - 1);
					bLastIsOp = FALSE;
					goto ExitSearchSym;
				}
			for (const auto& e : Priv::CalcExpFuncList)
				if (EckIsStartWithConstStringIA(p, e.szNameA))
				{
					vOp.push_back((TChar)e.eOp);
					p += (e.cchName - 1);
					bLastIsOp = TRUE;
					goto ExitSearchSym;
				}
			return CalcExpResult::InvalidChar;
		ExitSearchSym:;
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