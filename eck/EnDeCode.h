#pragma once
#include "CRefStr.h"

ECK_NAMESPACE_BEGIN
inline void UrlEncode(_In_reads_or_z_(cchText) PCCH pszText,
    int cchText, CRefStrA& rs, BOOL bPlusForSpace = TRUE) noexcept
{
    if (cchText < 0)
        cchText = (int)TcsLen(pszText);
    rs.Reserve(cchText + cchText / 2);
    CHAR ch;
    const auto pszEnd = pszText + cchText;
    for (ch = *pszText; pszText < pszEnd; ch = *++pszText)
    {
        if (isalnum((BYTE)ch) || (ch == '-') || (ch == '_') || (ch == '.') || (ch == '~'))
            rs.PushBackChar(ch);
        else if (ch == ' ' && bPlusForSpace)
            rs.PushBackChar('+');
        else
        {
            rs.PushBackChar('%');
            rs.PushBackChar(ByteToHex((BYTE)ch >> 4));
            rs.PushBackChar(ByteToHex((BYTE)ch & 0b1111));
        }
    }
}

EckInline void UrlEncode(_In_reads_or_z_(cchText) const char8_t* pszText,
    int cchText, CRefStrA& rs, BOOL bPlusForSpace = TRUE) noexcept
{
    return UrlEncode((PCCH)pszText, cchText, rs, bPlusForSpace);
}

inline void UrlDecode(_In_reads_or_z_(cchText) PCCH pszText,
    int cchText, CRefStrA& rs) noexcept
{
    if (cchText < 0)
        cchText = (int)TcsLen(pszText);
    rs.Reserve(cchText);
    CHAR ch;
    const auto pszEnd = pszText + cchText;
    for (ch = *pszText; pszText < pszEnd; ch = *++pszText)
    {
        if (ch == '+')
            rs.PushBackChar(' ');
        else if (ch == '%')
        {
            EckAssert(pszText + 2 < pszEnd);
            const BYTE h = ByteFromHex(*++pszText);
            const BYTE l = ByteFromHex(*++pszText);
            rs.PushBackChar((h << 4) + l);
        }
        else
            rs.PushBackChar(ch);
    }
}

EckInline void UrlDecode(_In_reads_or_z_(cchText) const char8_t* pszText,
    int cchText, CRefStrA& rs) noexcept
{
    UrlDecode((PCCH)pszText, cchText, rs);
}
ECK_NAMESPACE_END