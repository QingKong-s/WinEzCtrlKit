#pragma once
#include "ECK.h"

#include "..\ThirdPartyLib\FastFloat\fast_float.h"

#include <charconv>

ECK_NAMESPACE_BEGIN
namespace Priv
{
    constexpr inline BYTE CharToDigitTable[]{ 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 255, 255, 255, 255, 255,
    255, 255, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
    255, 255, 255, 255, 255, 255, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
    31, 32, 33, 34, 35 };

    constexpr inline BYTE TableI32StringSize[]{ 0,32,32,21,16,14,13,12,11,11,10,10,9,9,9,9,8,8,8,8,8,8,8,8,7,7,7,7,7,7,7,7,7,7,7,7 };
    constexpr inline BYTE TableI64StringSize[]{ 0,64,64,41,32,28,25,23,22,21,20,19,18,18,17,17,16,16,16,16,15,15,15,15,14,14,14,14,14,14,14,13,13,13,13,13 };
}

enum class TcsCvtErr
{
    Ok,
    Overflow,		// 解析结果溢出
    Nothing,		// 输入字符串为空
    ErrorFormat,
    OnlySign,		// 解析符号完毕后已到结尾
    OnlyRadixPrefix,// 解析进制前缀完毕后已到结尾
    BufferTooSmall,
    Unknown,
};


// 根据目标整数类型、进制和填充，计算转换所需的最小缓冲区大小（包含结束符和可能的负号）
template<std::integral TInt>
EckInlineNdCe size_t TcsCvtCalcBufferSize(int iRadix = 10, int cchFillTo = 0) noexcept
{
    if constexpr (sizeof(TInt) == 8)
        return std::max((size_t)Priv::TableI64StringSize[iRadix], (size_t)cchFillTo) + 2;
    else
        return std::max((size_t)Priv::TableI32StringSize[iRadix], (size_t)cchFillTo) + 2;
}

/// <summary>
/// 文本到整数
/// </summary>
/// <param name="p">输入文本</param>
/// <param name="cch">文本长度</param>
/// <param name="i">接收转换结果</param>
/// <param name="iRadix">进制，在2到36之间，若为0则根据前缀自动判断进制，支持0x和0b</param>
/// <param name="ppEnd">接收指向扫描结束位置下一个字符的指针，若失败，返回p的值</param>
/// <returns>错误代码</returns>
template<CcpStdCharPtr TPtr, std::integral TInt>
inline TcsCvtErr TcsToInt(
    _In_reads_(cch) TPtr p,
    size_t cch,
    _Out_ TInt& i,
    int iRadix = 0,
    _Outptr_opt_ TPtr* ppEnd = nullptr) noexcept
{
    i = 0;
    if (!cch)
    {
        if (ppEnd) *ppEnd = p;
        return TcsCvtErr::Nothing;
    }
    const auto pEnd = p + cch;
    BOOL bNeg = FALSE;
    // 识别符号
    if (*p == '-')
        bNeg = TRUE, ++p;
    else if (*p == '+')
        ++p;
    if (p == pEnd)
    {
        if (ppEnd) *ppEnd = p;
        return TcsCvtErr::OnlySign;
    }
    // 若未指定进制，则尝试识别进制
    if (iRadix == 0)
        if (p + 2 <= pEnd &&
            (*p == '0' && (p[1] == 'x' || p[1] == 'X')))// 0x, 0X
        {
            p += 2;
            iRadix = 16;
        }
        else if (p + 2 <= pEnd &&
            *p == '0' && (p[1] == 'b' || p[1] == 'B'))// 0b, 0B
        {
            p += 2;
            iRadix = 2;
        }
        else
            iRadix = 10;
    if (p == pEnd)
    {
        if (ppEnd) *ppEnd = p;
        return TcsCvtErr::OnlyRadixPrefix;
    }

    using TUnsigned = std::make_unsigned_t<TInt>;
    TUnsigned Max;
    if constexpr (std::is_signed_v<TInt>)
        if (bNeg)
            Max = TUnsigned(TUnsigned{} - TUnsigned(std::numeric_limits<TInt>::min()));
        else
            Max = TUnsigned(std::numeric_limits<TInt>::max());
    else
        Max = TUnsigned(std::numeric_limits<TInt>::max());
    TUnsigned Result{};
    const auto pBegin = p;
    for (; p < pEnd; ++p)
    {
        const auto ch = *p;
        if (ch >= ARRAYSIZE(Priv::CharToDigitTable))
            break;
        const int Digit = Priv::CharToDigitTable[ch];
        if (Digit >= iRadix)
            break;
        if (Result > (Max - Digit) / iRadix)
        {
            if (ppEnd) *ppEnd = p;
            if constexpr (std::is_signed_v<TInt>)
                if (bNeg)
                    i = std::numeric_limits<TInt>::min();
                else
                    i = std::numeric_limits<TInt>::max();
            else
                i = std::numeric_limits<TInt>::max();
            return TcsCvtErr::Overflow;
        }
        else
            Result = Result * iRadix + Digit;
    }
    if constexpr (std::is_signed_v<TInt>)
        if (bNeg)
            Result = TUnsigned(TUnsigned{} - Result);
    i = TInt(Result);
    if (ppEnd) *ppEnd = p;
    return TcsCvtErr::Ok;
}

/// <summary>
/// 整数到文本
/// </summary>
/// <param name="p">输出缓冲区，必须足够大以包括转换所需的工作空间，一般使用TcsCvtCalcBufferSize计算。若空间足够，则函数添加结尾NULL，否则，函数不执行截断操作且不报告错误</param>
/// <param name="cch">缓冲区大小</param>
/// <param name="i">要转换的整数值</param>
/// <param name="iRadix">目标进制，在2到36之间</param>
/// <param name="bUpperCase">是否使用大写字母表示大于9的数字字符</param>
/// <param name="ppEnd">接收写入数字序列结束位置的指针，若失败，返回p的值</param>
/// <param name="cchFillTo">最小数字宽度（不包括负号），当实际数字位数小于该值时执行填充</param>
/// <param name="chFill">用作填充的字符</param>
/// <returns>错误代码</returns>
template<CcpNonConstStdCharPtr TPtr, std::integral TInt>
inline TcsCvtErr TcsFromInt(
    _Out_writes_(cch) TPtr p,
    size_t cch,
    TInt i,
    int iRadix = 10,
    BOOL bUpperCase = TRUE,
    _Outptr_opt_ TPtr* ppEnd = nullptr,
    int cchFillTo = 0,
    RemoveStdCharPtr_T<TPtr> chFill = '0') noexcept
{
    EckAssert(iRadix >= 2 && iRadix <= 36);
    if (cch < TcsCvtCalcBufferSize<TInt>(iRadix, cchFillTo) - 1)
    {
        if (ppEnd) *ppEnd = p;
        return TcsCvtErr::BufferTooSmall;
    }
    const auto pEnd = p + cch;
    using TUnsigned = std::make_unsigned_t<TInt>;
    auto Val = (TUnsigned)i;
    BOOL bNeg{};
    if constexpr (std::is_signed_v<TInt>)
    {
        if (i < 0)
        {
            bNeg = TRUE;
            Val = TUnsigned(0 - i);
        }
    }
    using TChar = RemoveStdCharPtr_T<TPtr>;

    constexpr TChar DigU[]{ '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E',
    'F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z' };
    constexpr TChar DigL[]{ '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e',
    'f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z' };
    const auto DigTable = (bUpperCase ? DigU : DigL);

    auto pWrite = pEnd;
    do
    {
        *--pWrite = DigTable[Val % iRadix];
        Val /= iRadix;
    } while (Val);
    const auto cchNum = pEnd - pWrite;
    const auto cchExtraFill = (cchFillTo > cchNum) ? (cchFillTo - cchNum) : 0;
    if (bNeg)
        *p++ = '-';
    const auto pFillEnd = p + cchExtraFill;
    if (pFillEnd > pWrite)
    {
        if (ppEnd) *ppEnd = (bNeg ? p - 1 : p);
        return TcsCvtErr::BufferTooSmall;
    }
    for (; p < pFillEnd; ++p)
        *p = chFill;
    if (p != pWrite)
        if constexpr (sizeof(TChar) == 1)
            memmove(p, pWrite, cchNum);
        else
            wmemmove(p, pWrite, cchNum);
    p += cchNum;
    if (p < pEnd) *p = 0;
    if (ppEnd) *ppEnd = p;
    return TcsCvtErr::Ok;
}


enum class TcsFloatFmt
{
    General = std::chars_format::general,
    Hex = std::chars_format::hex,
    Fixed = std::chars_format::fixed,
    Scientific = std::chars_format::scientific,
};

namespace Priv
{
    EckInlineNdCe TcsCvtErr CharConvEcToTcsCvtErr(std::errc ec) noexcept
    {
        switch (ec)
        {
        case std::errc{}:                       return TcsCvtErr::Ok;
        case std::errc::result_out_of_range:
        case std::errc::value_too_large:
            return TcsCvtErr::BufferTooSmall;
        case std::errc::invalid_argument:       return TcsCvtErr::ErrorFormat;
        default: return TcsCvtErr::Unknown;
        }
    }
    EckInlineNdCe auto TcsFloatFmtToFastFloatFmt(TcsFloatFmt e) noexcept
    {
        switch (e)
        {
        case TcsFloatFmt::General:      return fast_float::chars_format::general;
        case TcsFloatFmt::Hex:          return fast_float::chars_format::hex;
        case TcsFloatFmt::Fixed:        return fast_float::chars_format::fixed;
        case TcsFloatFmt::Scientific:   return fast_float::chars_format::scientific;
        default:                        return fast_float::chars_format::general;
        }
    }
}

template<CcpStdCharPtr TPtr, std::floating_point TFloat>
inline TcsCvtErr TcsToFloat(
    _In_reads_(cch) TPtr p,
    size_t cch,
    _Out_ TFloat& f,
    _Outptr_opt_ TPtr* ppEnd = nullptr,
    TcsFloatFmt eFmt = TcsFloatFmt::General,
    int iRadix = 10) noexcept
{
    using TChar = RemoveStdCharPtr_T<TPtr>;

    if (!cch)
    {
        f = 0;
        if (ppEnd) *ppEnd = p;
        return TcsCvtErr::Nothing;
    }
    if (*p == '+')
        ++p, --cch;
    const fast_float::parse_options_t<TChar> Opt
    {
        Priv::TcsFloatFmtToFastFloatFmt(eFmt),
        '.',
        iRadix
    };
    const auto r = fast_float::from_chars_float_advanced(p, p + cch, f, Opt);
    if (ppEnd) *ppEnd = (TPtr)r.ptr;
    return Priv::CharConvEcToTcsCvtErr(r.ec);
}

template<CcpNonConstStdCharPtr TPtr, std::floating_point TFloat>
inline TcsCvtErr TcsFromFloat(
    _Out_writes_(cch) TPtr p,
    size_t cch,
    TFloat f,
    _Outptr_opt_ TPtr* ppEnd = nullptr,
    TcsFloatFmt eFmt = TcsFloatFmt::General,
    int iPrecision = 6) noexcept
{
    using TChar = RemoveStdCharPtr_T<TPtr>;

    const auto pBufA = (PCH)p;
    const auto cchBufA = cch * sizeof(TChar);
    const auto r = std::to_chars(pBufA, pBufA + cchBufA,
        f, (std::chars_format)eFmt, iPrecision);
    if constexpr (sizeof(TChar) != 1)
        if (r.ec == std::errc{})
        {
            auto pA = pBufA + (r.ptr - pBufA) - 1;
            auto pW = p + (r.ptr - pBufA) - 1;
            const auto pLastW = pW;
            if (pW >= p + cch)
            {
                if (ppEnd) *ppEnd = p;
                return TcsCvtErr::BufferTooSmall;
            }
            while (pW >= p)
                *pW-- = *pA--;
            if (pLastW + 1 < p + cch)
                *(pLastW + 1) = 0;
            if (ppEnd) *ppEnd = pLastW;
            return TcsCvtErr::Ok;
        }
        else
        {
            if (ppEnd) *ppEnd = p;
            return Priv::CharConvEcToTcsCvtErr(r.ec);
        }
    else
        if (r.ec == std::errc{})
        {
            if ((TPtr)r.ptr < p + cch)
                *(TPtr)r.ptr = 0;
            if (ppEnd) *ppEnd = (TPtr)r.ptr;
            return TcsCvtErr::Ok;
        }
        else
        {
            if (ppEnd) *ppEnd = p;
            return Priv::CharConvEcToTcsCvtErr(r.ec);
        }
}
ECK_NAMESPACE_END