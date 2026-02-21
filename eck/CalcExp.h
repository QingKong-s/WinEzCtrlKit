#pragma once
#include "StringUtility.h"

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
    enum class CepOp : char
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

    struct CepSymFunc
    {
        WCHAR szNameW[6];
        char szNameA[6];
        CepOp eOp;
        SCHAR cchName;
    };
#define ECKPRIV_CALCEXP_FUNCNAME(s) L## #s, #s, CepOp::s, CHAR(sizeof(#s) - 1)
    constexpr inline CepSymFunc CalcExpFuncList[]
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

    struct CepSymConst
    {
        WCHAR szNameW[3];
        char szNameA[3];
        CHAR cchName;
        double lfVal;
    };
#define ECKPRIV_CALCEXP_CONSTNAME(s) L## #s, #s, CHAR(sizeof(#s) - 1)
    constexpr inline CepSymConst CalcExpConstList[]
    {
        { ECKPRIV_CALCEXP_CONSTNAME(Pi), 3.141592653589793 },
        { ECKPRIV_CALCEXP_CONSTNAME(E), 2.718281828459045 },
    };
#undef ECKPRIV_CALCEXP_CONSTNAME

    EckInlineNdCe int CepPriority(auto ch) noexcept
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
    BOOL CepDoOperation(std::vector<double>& vNum, std::vector<TChar>& vOp) noexcept
    {
        if (vOp.empty() || vNum.empty())
            return FALSE;
        const auto bBinary = (vOp.back() >= (TChar)CepOp::Max);
        if (bBinary ? (vNum.size() < 2) : FALSE)
            return FALSE;
        const auto chOp = vOp.back();
        vOp.pop_back();
        const auto n2 = vNum.back();
        vNum.pop_back();
        double n1;
        if (bBinary)
        {
            n1 = vNum.back();
            vNum.pop_back();
        }
        else
            n1 = 0.;

        switch (chOp)
        {
        case '+': vNum.push_back(n1 + n2);      break;
        case '-': vNum.push_back(n1 - n2);      break;
        case '*': vNum.push_back(n1 * n2);      break;
        case '/': vNum.push_back(n1 / n2);      break;
        case '%': vNum.push_back(fmod(n1, n2)); break;
        case '^': vNum.push_back(pow(n1, n2));  break;
        case (TChar)CepOp::Ln:      vNum.push_back(log(n2));        break;
        case (TChar)CepOp::Log:     vNum.push_back(log10(n2));      break;
        case (TChar)CepOp::Sin:     vNum.push_back(sin(n2));        break;
        case (TChar)CepOp::Cos:     vNum.push_back(cos(n2));        break;
        case (TChar)CepOp::Tan:     vNum.push_back(tan(n2));        break;
        case (TChar)CepOp::Cot:     vNum.push_back(1. / tan(n2));   break;
        case (TChar)CepOp::Sec:     vNum.push_back(1. / cos(n2));   break;
        case (TChar)CepOp::Csc:     vNum.push_back(1. / sin(n2));   break;
        case (TChar)CepOp::Asin:    vNum.push_back(asin(n2));       break;
        case (TChar)CepOp::Acos:    vNum.push_back(acos(n2));       break;
        case (TChar)CepOp::Atan:    vNum.push_back(atan(n2));       break;
        case (TChar)CepOp::Acot:    vNum.push_back(atan(1. / n2));  break;
        case (TChar)CepOp::Asec:    vNum.push_back(acos(1. / n2));  break;
        case (TChar)CepOp::Acsc:    vNum.push_back(asin(1. / n2));  break;
        case (TChar)CepOp::Sqrt:    vNum.push_back(sqrt(n2));       break;
        case (TChar)CepOp::Sinh:    vNum.push_back(sinh(n2));       break;
        case (TChar)CepOp::Cosh:    vNum.push_back(cosh(n2));       break;
        case (TChar)CepOp::Ceil:    vNum.push_back(ceil(n2));       break;
        case (TChar)CepOp::Floor:   vNum.push_back(floor(n2));      break;
        case (TChar)CepOp::Round:   vNum.push_back(round(n2));      break;
        default:return FALSE;
        }
        return TRUE;
    }

    template<class TChar>
    struct CeCharTraits {};

    template<>
    struct CeCharTraits<CHAR>
    {
        static BOOL IsDigit(char ch) noexcept { return isdigit(ch); }
        static BOOL IsAlpha(char ch) noexcept { return isalpha(ch); }
        static double ToDouble(const char* p, const char** ppEnd) noexcept { return strtod(p, (char**)ppEnd); }
        static PCSTR GetFuntionName(const CepSymFunc& e) noexcept { return e.szNameA; }
        static PCSTR GetConstName(const CepSymConst& e) noexcept { return e.szNameA; }
    };

    template<>
    struct CeCharTraits<WCHAR>
    {
        static BOOL IsDigit(WCHAR ch) noexcept { return iswdigit(ch); }
        static BOOL IsAlpha(WCHAR ch) noexcept { return iswalpha(ch); }
        static double ToDouble(const WCHAR* p, const WCHAR** ppEnd) noexcept { return wcstod(p, (WCHAR**)ppEnd); }
        static PCWSTR GetFuntionName(const CepSymFunc& e) noexcept { return e.szNameW; }
        static PCWSTR GetConstName(const CepSymConst& e) noexcept { return e.szNameW; }
    };
}

template<CcpStdCharPtr TPtr>
inline CalcExpResult CalculateExpression(
    _Out_ double& lfResult,
    _In_reads_or_z_(cchExp) TPtr pszExp,
    int cchExp = -1) noexcept
{
    using TChar = RemoveStdCharPtr_T<TPtr>;
    using TTraits = Priv::CeCharTraits<TChar>;

    lfResult = 0.;
    if (cchExp < 0)
        cchExp = (int)TcsLength(pszExp);
    std::vector<double> vNum{};
    std::vector<TChar> vOp{};
    BOOL bNumberBegin = TRUE;// TRUE = 期待数字
    for (auto p = pszExp; p < pszExp + cchExp; ++p)
    {
        const auto ch = *p;
        if (ch == ' ' || ch == '\t')
            continue;
        if (TTraits::IsDigit(ch))
        {
            bNumberBegin = FALSE;
            vNum.push_back(TTraits::ToDouble(p, &p));
            --p;
            continue;
        }
        else if (ch == '(')
        {
            bNumberBegin = TRUE;
            vOp.push_back(ch);
            continue;
        }
        else if (ch == ')')
        {
            bNumberBegin = FALSE;
            while (!vOp.empty() && vOp.back() != '(')
                if (!Priv::CepDoOperation(vNum, vOp))
                    return CalcExpResult::OpError;
            if (vOp.empty())
                return CalcExpResult::UnmatchedParentheses;
            vOp.pop_back();
        }
        else if (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '%' || ch == '^')
        {
            if (bNumberBegin)
                if (ch == '+')
                    continue;
                else if (ch == '-')
                {
                    vNum.push_back(0.);
                    bNumberBegin = FALSE;
                }
                else
                    return CalcExpResult::AdjacentOp;
            if (vOp.empty() || Priv::CepPriority(ch) > Priv::CepPriority(vOp.back()))
            {
                vOp.push_back(ch);
                continue;
            }
            else
            {
                while (!vOp.empty() &&
                    vOp.back() != '(' &&
                    Priv::CepPriority(ch) <= Priv::CepPriority(vOp.back()))
                {
                    if (!Priv::CepDoOperation(vNum, vOp))
                        return CalcExpResult::OpError;
                }
                vOp.push_back(ch);
            }
        }
        else if (TTraits::IsAlpha(ch))
        {
            for (const auto& e : Priv::CalcExpConstList)
                if (TcsCompareMaxLengthI(p, TTraits::GetConstName(e), e.cchName) == 0)
                {
                    vNum.push_back(e.lfVal);
                    p += (e.cchName - 1);
                    bNumberBegin = FALSE;
                    goto ExitSearchSym;
                }
            for (const auto& e : Priv::CalcExpFuncList)
                if (TcsCompareMaxLengthI(p, TTraits::GetFuntionName(e), e.cchName) == 0)
                {
                    vOp.push_back((TChar)e.eOp);
                    p += (e.cchName - 1);
                    bNumberBegin = TRUE;
                    goto ExitSearchSym;
                }
            return CalcExpResult::InvalidChar;
        ExitSearchSym:;
        }
        else
            return CalcExpResult::InvalidChar;
    }
    while (!vOp.empty())
        if (!Priv::CepDoOperation(vNum, vOp))
            return CalcExpResult::OpError;
    if (vNum.size() != 1)
        return CalcExpResult::OpError;
    lfResult = vNum.back();
    return CalcExpResult::Ok;
}
ECK_NAMESPACE_END