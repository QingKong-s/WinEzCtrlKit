#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
inline constexpr int INVALID_STR_POS = -1;
inline constexpr int StrNPos = -1;

inline constexpr CHAR SpaceCharsA[]{ " \t\r\n" };
// U+00A0 non-breaking space
// U+2000-2006 EN space
// U+2007 figure space
// U+2008 punctuation space
// U+2009 thin space
// U+200A hair space
// U+200B zero width space
// U+202F narrow no-break space
// U+205F medium mathematical space
// U+3000 ideographic space
// U+FEFF zero width no-break space
inline constexpr WCHAR SpaceCharsW[]{ L" \t\r\n\u00A0\u2000\u2001\u2002\u2003\u2004\u2005\u2006\u2007\u2008\u2009\u200A\u200B\u202F\u205F\u3000\uFEFF" };

template<CcpStdChar TChar>
EckInlineNd TChar TchToUpper(TChar c) noexcept
{
#if ECK_OPT_NO_ASCII_UPPER_LOWER
    if constexpr (std::is_same_v<TChar, char>)
        return (TChar)toupper(c);
    else
        return (TChar)towupper(c);
#else
    if constexpr (std::is_same_v<TChar, char>)
        return (TChar)__ascii_toupper(c);
    else
        return (TChar)__ascii_towupper(c);
#endif
}
template<CcpStdChar TChar>
EckInlineNd TChar TchToLower(TChar c) noexcept
{
#if ECK_OPT_NO_ASCII_UPPER_LOWER
    if constexpr (std::is_same_v<TChar, char>)
        return (TChar)tolower(c);
    else
        return (TChar)towlower(c);
#else
    if constexpr (std::is_same_v<TChar, char>)
        return (TChar)__ascii_tolower(c);
    else
        return (TChar)__ascii_towlower(c);
#endif
}

template<CcpStdChar TChar>
EckInlineNdCe bool TchEqual(TChar c1, TChar c2) noexcept
{
    return c1 == c2;
}
template<CcpStdChar TChar>
EckInlineNd bool TchEqualI(TChar c1, TChar c2) noexcept
{
    return TchToUpper(c1) == TchToUpper(c2);
}

template<CcpStdCharPtr TPtr>
EckInlineNd size_t TcsLength(_In_z_ TPtr Str) noexcept
{
    if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
        return strlen(Str);
    else
        return wcslen(Str);
}

template<CcpStdCharPtr TPtr>
EckInlineNd bool TcsEqual(_In_z_ TPtr Str1, _In_z_ ConstStdCharPtr_T<TPtr> Str2) noexcept
{
    if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
        return strcmp(Str1, Str2) == 0;
    else
        return wcscmp(Str1, Str2) == 0;
}
template<CcpStdCharPtr TPtr>
EckInlineNd bool TcsEqualI(_In_z_ TPtr Str1, _In_z_ ConstStdCharPtr_T<TPtr> Str2) noexcept
{
    if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
        return stricmp(Str1, Str2) == 0;
    else
        return wcsicmp(Str1, Str2) == 0;
}

template<CcpStdCharPtr TPtr>
EckInlineNd bool TcsEqualMaxLength(_In_reads_or_z_(Max) TPtr Str1,
    _In_reads_or_z_(Max) ConstStdCharPtr_T<TPtr> Str2, size_t Max) noexcept
{
    if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
        return strncmp(Str1, Str2, Max) == 0;
    else
        return wcsncmp(Str1, Str2, Max) == 0;
}
template<CcpStdCharPtr TPtr>
EckInlineNd bool TcsEqualMaxLengthI(_In_reads_or_z_(Max) TPtr Str1,
    _In_reads_or_z_(Max) ConstStdCharPtr_T<TPtr> Str2, size_t Max) noexcept
{
    if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
        return strnicmp(Str1, Str2, Max) == 0;
    else
        return wcsnicmp(Str1, Str2, Max) == 0;
}

template<CcpStdCharPtr TPtr>
EckInlineNd bool TcsEqualLength(_In_reads_(Len) TPtr Str1,
    _In_reads_(Len) ConstStdCharPtr_T<TPtr> Str2, size_t Len) noexcept
{
    if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
        return memcmp(Str1, Str2, Len) == 0;
    else
        return wmemcmp(Str1, Str2, Len) == 0;
}
template<CcpStdCharPtr TPtr>
EckInlineNd bool TcsEqualLengthI(_In_reads_(Len) TPtr Str1,
    _In_reads_(Len) ConstStdCharPtr_T<TPtr> Str2, size_t Len) noexcept
{
    EckCounter(Len, i)
    {
        if (!TchEqualI(Str1[i], Str2[i]))
            return false;
    }
    return true;
}

template<CcpStdCharPtr TPtr>
EckInlineNd bool TcsEqualLength2(_In_reads_(Len1) TPtr Str1, size_t Len1,
    _In_reads_(Len2) ConstStdCharPtr_T<TPtr> Str2, size_t Len2) noexcept
{
    if (Len1 != Len2)
        return false;
    return TcsEqualLength(Str1, Str2, Len1);
}
template<CcpStdCharPtr TPtr>
EckInlineNd bool TcsEqualLength2I(_In_reads_(Len1) TPtr Str1, size_t Len1,
    _In_reads_(Len2) ConstStdCharPtr_T<TPtr> Str2, size_t Len2) noexcept
{
    if (Len1 != Len2)
        return false;
    return TcsEqualLengthI(Str1, Str2, Len1);
}

template<CcpStdCharPtr TPtr>
EckInlineNd bool TcsIsStartWithLength2(_In_reads_(Len1) TPtr Str1, size_t Len1,
    _In_reads_(Len2) ConstStdCharPtr_T<TPtr> Str2, size_t Len2) noexcept
{
    if (Len1 < Len2)
        return false;
    return TcsEqualLength(Str1, Str2, Len2);
}
template<CcpStdCharPtr TPtr>
EckInlineNd bool TcsIsStartWithLength2I(_In_reads_(Len1) TPtr Str1, size_t Len1,
    _In_reads_(Len2) ConstStdCharPtr_T<TPtr> Str2, size_t Len2) noexcept
{
    if (Len1 < Len2)
        return false;
    return TcsEqualLengthI(Str1, Str2, Len2);
}

template<CcpStdCharPtr TPtr>
EckInlineNd bool TcsIsEndWithLength2(_In_reads_(Len1) TPtr Str1, size_t Len1,
    _In_reads_(Len2) ConstStdCharPtr_T<TPtr> Str2, size_t Len2) noexcept
{
    if (Len1 < Len2)
        return false;
    return TcsEqualLength(Str1 + Len1 - Len2, Str2, Len2);
}
template<CcpStdCharPtr TPtr>
EckInlineNd bool TcsIsEndWithLength2I(_In_reads_(Len1) TPtr Str1, size_t Len1,
    _In_reads_(Len2) ConstStdCharPtr_T<TPtr> Str2, size_t Len2) noexcept
{
    if (Len1 < Len2)
        return false;
    return TcsEqualLengthI(Str1 + Len1 - Len2, Str2, Len2);
}

template<CcpStdCharPtr TPtr>
_Ret_maybenull_ EckInlineNd TPtr TcsChar(_In_z_ TPtr Str, RemoveStdCharPtr_T<TPtr> c) noexcept
{
    if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
        return (TPtr)strchr((CHAR*)Str, (CHAR)c);
    else
        return (TPtr)wcschr((WCHAR*)Str, (WCHAR)c);
}
template<CcpStdCharPtr TPtr>
_Ret_maybenull_ EckInlineNd TPtr TcsCharLength(_In_reads_(Len) TPtr Str, size_t Len,
    RemoveStdCharPtr_T<TPtr> c) noexcept
{
    if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
        return (TPtr)memchr((CHAR*)Str, (CHAR)c, Len);
    else
        return (TPtr)wmemchr((WCHAR*)Str, (WCHAR)c, Len);
}

template<CcpStdCharPtr TPtr>
_Ret_maybenull_ EckInlineNd TPtr TcsRChar(_In_z_ TPtr Str, RemoveStdCharPtr_T<TPtr> c) noexcept
{
    if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
        return (TPtr)strrchr((CHAR*)Str, (CHAR)c);
    else
        return (TPtr)wcsrchr((WCHAR*)Str, (WCHAR)c);
}
template<CcpStdCharPtr TPtr>
_Ret_maybenull_ EckInlineNd TPtr TcsRCharLength(_In_reads_(Len) TPtr Str, size_t Len,
    RemoveStdCharPtr_T<TPtr> c) noexcept
{
    for (auto p = Str + Len - 1; p >= Str; --p)
    {
        if (*p == c)
            return p;
    }
    return nullptr;
}

template<CcpStdCharPtr TPtr>
_Ret_maybenull_ EckInlineNd TPtr TcsString(_In_z_ TPtr Str, _In_z_ ConstStdCharPtr_T<TPtr> SubStr) noexcept
{
    if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
        return (TPtr)strstr((CHAR*)Str, SubStr);
    else
        return (TPtr)wcsstr((WCHAR*)Str, SubStr);
}
template<CcpStdCharPtr TPtr>
_Ret_maybenull_ EckInlineNd TPtr TcsStringI(_In_z_ TPtr Str, _In_z_ ConstStdCharPtr_T<TPtr> SubStr) noexcept
{
    if (!*SubStr)
        return Str;
    for (; *Str; ++Str)
    {
        if (TchEqualI(*Str, *SubStr))
        {
            for (auto p1 = Str, p2 = SubStr; *p2; ++p1, ++p2)
            {
                if (!*p1 || !TchEqualI(*p1, *p2))
                    goto Next;
            }
            return Str;
        }
    Next:;
    }
    return nullptr;
}

template<CcpStdCharPtr TPtr>
_Ret_maybenull_ EckInlineNd TPtr TcsStringLength(_In_reads_(Len) TPtr Str, size_t Len,
    _In_reads_(SubLen) ConstStdCharPtr_T<TPtr> SubStr, size_t SubLen) noexcept
{
    if (Len < SubLen)
        return nullptr;
    if (SubLen == 0 || *SubStr == '\0')
        return Str;
    const auto pEnd = Str + Len - SubLen + 1;
    for (auto p = Str; p < pEnd; ++p)
    {
        const auto pFind = TcsCharLength(p, pEnd - p, *SubStr);
        if (!pFind)
            return nullptr;
        if (TcsEqualLength(pFind, SubStr, SubLen))
            return pFind;
        p = pFind + 1;
    }
    return nullptr;
}
template<CcpStdCharPtr TPtr>
_Ret_maybenull_ EckInlineNd TPtr TcsStringLengthI(_In_reads_(Len) TPtr Str, size_t Len,
    _In_reads_(SubLen) ConstStdCharPtr_T<TPtr> SubStr, size_t SubLen) noexcept
{
    if (Len < SubLen)
        return nullptr;
    if (SubLen == 0 || *SubStr == '\0')
        return Str;
    for (auto p = Str; p < Str + Len - SubLen + 1; ++p)
    {
        if (TchEqualI(*p, *SubStr) && TcsEqualLengthI(p, SubStr, SubLen))
            return p;
    }
    return nullptr;
}

template<CcpStdCharPtr TPtr>
_Ret_maybenull_ EckInlineNd TPtr TcsRStringLength(_In_reads_(Len) TPtr Str, size_t Len,
    _In_reads_(SubLen) ConstStdCharPtr_T<TPtr> SubStr, size_t SubLen, size_t posStart = SizeTMax) noexcept
{
    if (Len < SubLen)
        return nullptr;
    if (SubLen == 0 || *SubStr == '\0')
        return Str + std::min(posStart, Len - SubLen);
    for (auto p = Str + std::min(posStart, Len - SubLen); p >= Str; --p)
    {
        if (TchEqual(*p, *SubStr) && TcsEqualLength(p, SubStr, SubLen))
            return p;
    }
    return nullptr;
}
template<CcpStdCharPtr TPtr>
_Ret_maybenull_ EckInlineNd TPtr TcsRStringLengthI(_In_reads_(Len) TPtr Str, size_t Len,
    _In_reads_(SubLen) ConstStdCharPtr_T<TPtr> SubStr, size_t SubLen, size_t posStart = SizeTMax) noexcept
{
    if (Len < SubLen)
        return nullptr;
    if (SubLen == 0 || *SubStr == '\0')
        return Str + std::min(posStart, Len - SubLen);
    for (auto p = Str + std::min(posStart, Len - SubLen); p >= Str; --p)
    {
        if (TchEqualI(*p, *SubStr) && TcsEqualLengthI(p, SubStr, SubLen))
            return p;
    }
    return nullptr;
}

template<CcpStdCharPtr TPtr>
_Ret_maybenull_ EckInlineNd TPtr TcsCharFirstOf(_In_reads_(Len) TPtr Str, size_t Len,
    _In_reads_(CharsLen) ConstStdCharPtr_T<TPtr> Chars, size_t CharsLen) noexcept
{
#if defined(_MSC_VER) && _USE_STD_VECTOR_ALGORITHMS && 0
    TPtr r;
    if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
        r = (TPtr)__std_find_first_of_trivial_1(Str, Str + Len, Chars, Chars + CharsLen);
    else
        r = (TPtr)__std_find_first_of_trivial_2(Str, Str + Len, Chars, Chars + CharsLen);
    return (r == Str + Len) ? nullptr : r;
#else
    for (auto p = Str; p < Str + Len; ++p)
    {
        if (TcsCharLength(Chars, CharsLen, *p))
            return p;
    }
    return nullptr;
#endif
}
template<CcpStdCharPtr TPtr>
_Ret_maybenull_ EckInlineNd TPtr TcsCharFirstNotOf(_In_reads_(Len) TPtr Str, size_t Len,
    _In_reads_(CharsLen) ConstStdCharPtr_T<TPtr> Chars, size_t CharsLen) noexcept
{
    for (auto p = Str; p < Str + Len; ++p)
    {
        if (!TcsCharLength(Chars, CharsLen, *p))
            return p;
    }
    return nullptr;
}
template<CcpStdCharPtr TPtr>
_Ret_maybenull_ EckInlineNd TPtr TcsCharLastOf(_In_reads_(Len) TPtr Str, size_t Len,
    _In_reads_(CharsLen) ConstStdCharPtr_T<TPtr> Chars, size_t CharsLen, size_t posStart = SizeTMax) noexcept
{
    EckAssert(Len);
#if defined(_MSC_VER) && _USE_STD_VECTOR_ALGORITHMS && 0
    TPtr r;
    if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
        r = (TPtr)__std_find_last_of_trivial_pos_1(
            Str, std::min(posStart + 1, Len), Chars, CharsLen);
    else
        r = (TPtr)__std_find_last_of_trivial_pos_2(
            Str, std::min(posStart + 1, Len), Chars, CharsLen);
    return (r == Str + Len) ? nullptr : r;
#else
    for (auto p = Str + std::min(posStart, Len - 1); p >= Str; --p)
    {
        if (TcsCharLength(Chars, CharsLen, *p))
            return p;
    }
    return nullptr;
#endif
}
template<CcpStdCharPtr TPtr>
_Ret_maybenull_ EckInlineNd TPtr TcsCharLastNotOf(_In_reads_(Len) TPtr Str, size_t Len,
    _In_reads_(CharsLen) ConstStdCharPtr_T<TPtr> Chars, size_t CharsLen, size_t posStart = SizeTMax) noexcept
{
    EckAssert(Len);
    for (auto p = Str + std::min(posStart, Len - 1); p >= Str; --p)
    {
        if (!TcsCharLength(Chars, CharsLen, *p))
            return p;
    }
    return nullptr;
}

template<CcpStdCharPtr TPtr>
_Ret_maybenull_ EckInlineNd TPtr TcsCharFirstOf(_In_z_ TPtr Str, _In_z_ ConstStdCharPtr_T<TPtr> Chars) noexcept
{
    if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
        return (TPtr)strpbrk(Str, Chars);
    else
        return (TPtr)wcspbrk(Str, Chars);
}

template<CcpNonConstStdCharPtr TPtr>
_Post_equal_to_(Dst) EckInlineNd TPtr TcsCopy(_Out_writes_z_(_String_length_(Src) + 1) TPtr Dst,
    _In_z_ ConstStdCharPtr_T<TPtr> Src) noexcept
{
    if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
        return strcpy(Dst, Src);
    else
        return wcscpy(Dst, Src);
}
template<CcpNonConstStdCharPtr TPtr>
_Post_equal_to_(Dst) EckInlineNd TPtr TcsCopyLength(_Out_writes_(Len) TPtr Dst,
    _In_z_ ConstStdCharPtr_T<TPtr> Src, size_t Len) noexcept
{
    if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
        return (TPtr)memcpy(Dst, Src, Len);
    else
        return (TPtr)wmemcpy(Dst, Src, Len);
}
template<CcpNonConstStdCharPtr TPtr>
_Post_equal_to_(Dst) EckInlineNd TPtr TcsCopyLengthEnd(_Out_writes_(Len) TPtr Dst,
    _In_z_ ConstStdCharPtr_T<TPtr> Src, size_t Len) noexcept
{
    *(TcsCopyLength(Dst, Src, Len) + Len) = 0;
    return Dst;
}

template<CcpNonConstStdCharPtr TPtr>
_Post_equal_to_(Dst) EckInlineNd TPtr TcsMoveLength(_Out_writes_(Len) TPtr Dst,
    _In_z_ ConstStdCharPtr_T<TPtr> Src, size_t Len) noexcept
{
    if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
        return (TPtr)memmove(Dst, Src, Len);
    else
        return (TPtr)wmemmove(Dst, Src, Len);
}
template<CcpNonConstStdCharPtr TPtr>
_Post_equal_to_(Dst) EckInlineNd TPtr TcsMoveLengthEnd(_Out_writes_(Len) TPtr Dst,
    _In_z_ ConstStdCharPtr_T<TPtr> Src, size_t Len) noexcept
{
    *(TcsMoveLength(Dst, Src, Len) + Len) = 0;
    return Dst;
}

template<CcpStdCharPtr TPtr>
EckInlineNd int TcsCompare(_In_z_ TPtr Str1, _In_z_ ConstStdCharPtr_T<TPtr> Str2) noexcept
{
    if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
        return strcmp(Str1, Str2);
    else
        return wcscmp(Str1, Str2);
}
template<CcpStdCharPtr TPtr>
EckInlineNd int TcsCompareMaxLength(_In_reads_or_z_(Max) TPtr Str1,
    _In_reads_or_z_(Max) ConstStdCharPtr_T<TPtr> Str2, size_t Max) noexcept
{
    if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
        return strncmp(Str1, Str2, Max);
    else
        return wcsncmp(Str1, Str2, Max);
}
template<CcpStdCharPtr TPtr>
EckInlineNd int TcsCompareLength(_In_reads_(Len) TPtr Str1,
    _In_reads_(Len) ConstStdCharPtr_T<TPtr> Str2, size_t Len) noexcept
{
    if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
        return memcmp(Str1, Str2, Len);
    else
        return wmemcmp(Str1, Str2, Len);
}
template<CcpStdCharPtr TPtr>
EckInlineNd int TcsCompareLength2(_In_reads_(Len1) TPtr Str1, size_t Len1,
    _In_reads_(Len2) ConstStdCharPtr_T<TPtr> Str2, size_t Len2) noexcept
{
    const auto r = TcsCompareLength(Str1, Str2, std::min(Len1, Len2));
    if (r)
        return r;
    if (Len1 < Len2)
        return -1;
    else if (Len1 > Len2)
        return 1;
    else
        return 0;
}

template<CcpStdCharPtr TPtr>
EckInlineNd int TcsCompareI(_In_z_ TPtr Str1, _In_z_ ConstStdCharPtr_T<TPtr> Str2) noexcept
{
    if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
        return stricmp(Str1, Str2);
    else
        return wcsicmp(Str1, Str2);
}
template<CcpStdCharPtr TPtr>
EckInlineNd int TcsCompareMaxLengthI(_In_reads_or_z_(Max) TPtr Str1,
    _In_reads_or_z_(Max) ConstStdCharPtr_T<TPtr> Str2, size_t Max) noexcept
{
    if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
        return _strnicmp(Str1, Str2, Max);
    else
        return _wcsnicmp(Str1, Str2, Max);
}
template<CcpStdCharPtr TPtr>
EckInlineNd int TcsCompareLengthI(_In_reads_(Len) TPtr Str1,
    _In_reads_(Len) ConstStdCharPtr_T<TPtr> Str2, size_t Len) noexcept
{
    using TChar = RemoveStdCharPtr_T<TPtr>;
    if constexpr (std::is_same_v<TChar, char>)
        return memicmp(Str1, Str2, Len);
    else
    {
        const auto pEnd1 = Str1 + Len;
        for (auto p1 = Str1, p2 = Str2; p1 < pEnd1; ++p1, ++p2)
        {
            const auto ch1 = (std::make_signed_t<TChar>)TchToLower(*p1);
            const auto ch2 = (std::make_signed_t<TChar>)TchToLower(*p2);
            if (ch1 != ch2)
                return ch1 - ch2;
        }
        return 0;
    }
}
template<CcpStdCharPtr TPtr>
EckInlineNd int TcsCompareLength2I(_In_reads_(Len1) TPtr Str1, size_t Len1,
    _In_reads_(Len2) ConstStdCharPtr_T<TPtr> Str2, size_t Len2) noexcept
{
    const auto r = TcsCompareLengthI(Str1, Str2, std::min(Len1, Len2));
    if (r)
        return r;
    if (Len1 < Len2)
        return -1;
    else if (Len1 > Len2)
        return 1;
    else
        return 0;
}

template<CcpNonConstStdCharPtr TPtr>
EckInlineNd TPtr TcsSet(_Out_writes_z_(cchDst) TPtr Dst,
    RemoveStdCharPtr_T<TPtr> ch, size_t cchDst) noexcept
{
    if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
        return (TPtr)memset(Dst, ch, cchDst);
    else
        return (TPtr)wmemset(Dst, ch, cchDst);
}


template<CcpStdCharPtr TPtr>
EckInlineNd int FindString(_In_z_ TPtr pszText,
    _In_z_ ConstStdCharPtr_T<TPtr> pszSub, int posStart = 0) noexcept
{
    const auto pszFind = TcsString(pszText + posStart, pszSub);
    return pszFind ? int(pszFind - pszText) : StrNPos;
}
template<CcpStdCharPtr TPtr>
EckInlineNd int FindStringI(_In_z_ TPtr pszText,
    _In_z_ ConstStdCharPtr_T<TPtr> pszSub, int posStart = 0) noexcept
{
    const auto pszFind = TcsStringI(pszText + posStart, pszSub);
    return pszFind ? int(pszFind - pszText) : StrNPos;
}

template<CcpStdCharPtr TPtr>
EckInlineNd int FindStringLength(_In_reads_(cchText) TPtr pszText, int cchText,
    _In_reads_(cchSub) ConstStdCharPtr_T<TPtr> pszSub, int cchSub, int posStart = 0) noexcept
{
    const auto pFind = TcsStringLength(pszText + posStart, cchText - posStart, pszSub, cchSub);
    return pFind ? int(pFind - pszText) : StrNPos;
}
template<CcpStdCharPtr TPtr>
EckInlineNd int FindStringLengthI(_In_reads_(cchText) TPtr pszText, int cchText,
    _In_reads_(cchSub) ConstStdCharPtr_T<TPtr> pszSub, int cchSub, int posStart = 0) noexcept
{
    const auto pFind = TcsStringLengthI(pszText + posStart, cchText - posStart, pszSub, cchSub);
    return pFind ? int(pFind - pszText) : StrNPos;
}

template<CcpStdCharPtr TPtr>
EckInlineNd int RFindString(_In_reads_(cchText) TPtr pszText, int cchText,
    _In_reads_(cchSub) ConstStdCharPtr_T<TPtr> pszSub, int cchSub, int posStart = -1) noexcept
{
    const auto pFind = TcsRStringLength(pszText, cchText, pszSub, cchSub, posStart);
    return pFind ? int(pFind - pszText) : StrNPos;
}
template<CcpStdCharPtr TPtr>
EckInlineNd int RFindStringI(_In_reads_(cchText) TPtr pszText, int cchText,
    _In_reads_(cchSub) ConstStdCharPtr_T<TPtr> pszSub, int cchSub, int posStart = -1) noexcept
{
    const auto pFind = TcsRStringLengthI(pszText, cchText, pszSub, cchSub, posStart);
    return pFind ? int(pFind - pszText) : StrNPos;
}

template<CcpStdCharPtr TPtr>
EckInlineNd int FindChar(_In_z_ TPtr pszText, RemoveStdCharPtr_T<TPtr> ch, int posStart = 0) noexcept
{
    const auto pszFind = TcsChar(pszText + posStart, ch);
    return pszFind ? int(pszFind - pszText) : StrNPos;
}
template<CcpStdCharPtr TPtr>
EckInlineNd int FindCharLength(_In_reads_(cchText) TPtr pszText, int cchText,
    RemoveStdCharPtr_T<TPtr> ch, int posStart = 0) noexcept
{
    EckAssert(posStart >= 0 && posStart <= cchText);
    const auto pFind = TcsCharLength(pszText + posStart, cchText - posStart, ch);
    return pFind ? int(pFind - pszText) : StrNPos;
}
template<CcpStdCharPtr TPtr>
EckInlineNd int RFindCharLength(_In_reads_(cchText) TPtr pszText, int cchText,
    RemoveStdCharPtr_T<TPtr> ch, int posStart = -1) noexcept
{
    if (posStart < 0)
        posStart = cchText - 1;
    EckAssert(posStart >= 0 && posStart <= cchText);
    const auto pFind = TcsRCharLength(pszText, std::min(posStart + 1, cchText), ch);
    return pFind ? int(pFind - pszText) : StrNPos;
}

template<CcpStdCharPtr TPtr>
EckInlineNd int FindCharFirstOf(_In_z_ TPtr pszText, int cchText,
    _In_reads_(cchChars) ConstStdCharPtr_T<TPtr> pszChars, int cchChars, int posStart = 0) noexcept
{
    const auto pszFind = TcsCharFirstOf(pszText + posStart, pszChars, cchChars);
    return pszFind ? int(pszFind - pszText) : StrNPos;
}
template<CcpStdCharPtr TPtr>
EckInlineNd int FindCharFirstNotOf(_In_z_ TPtr pszText, int cchText,
    _In_reads_(cchChars) ConstStdCharPtr_T<TPtr> pszChars, int cchChars, int posStart = 0) noexcept
{
    const auto pszFind = TcsCharFirstNotOf(pszText + posStart, cchText - posStart, pszChars, cchChars);
    return pszFind ? int(pszFind - pszText) : StrNPos;
}
template<CcpStdCharPtr TPtr>
EckInlineNd int FindCharLastOf(_In_z_ TPtr pszText, int cchText,
    _In_reads_(cchChars) ConstStdCharPtr_T<TPtr> pszChars, int cchChars, int posStart = -1) noexcept
{
    const auto pszFind = TcsCharLastOf(pszText, cchText, pszChars, cchChars, posStart);
    return pszFind ? int(pszFind - pszText) : StrNPos;
}
template<CcpStdCharPtr TPtr>
EckInlineNd int FindCharLastNotOf(_In_z_ TPtr pszText, int cchText,
    _In_reads_(cchChars) ConstStdCharPtr_T<TPtr> pszChars, int cchChars, int posStart = -1) noexcept
{
    const auto pszFind = TcsCharLastNotOf(pszText, cchText, pszChars, cchChars, posStart);
    return pszFind ? int(pszFind - pszText) : StrNPos;
}

// Deprecated. For compatibility.
template<CcpStdCharPtr TPtr>
EckInlineNd TPtr TrimStringLeft(_In_z_ TPtr pszText) noexcept
{
    auto ch = *pszText;
    while ((ch == L' ' || ch == L'　') && ch != L'\0')
        ch = *++pszText;
    return pszText;
}

template<CcpStdCharPtr TPtr>
EckInlineNd TPtr TrimStringLeft(_In_reads_(cchText) TPtr pszText, int cchText) noexcept
{
    if (cchText < 0)
        cchText = (int)TcsLength(pszText);
    if (!cchText)
        return pszText;
    TPtr pFind;
    if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
        pFind = (TPtr)TcsCharFirstNotOf(pszText, cchText, EckStrAndLen(SpaceCharsA));
    else
        pFind = (TPtr)TcsCharFirstNotOf(pszText, cchText, EckStrAndLen(SpaceCharsW));
    return pFind ? pFind : pszText + cchText;
}

template<CcpStdCharPtr TPtr>
EckInlineNd TPtr TrimStringRight(_In_reads_(cchText) TPtr pszText, int cchText) noexcept
{
    if (cchText < 0)
        cchText = (int)TcsLength(pszText);
    if (!cchText)
        return pszText;
    TPtr pFind;
    if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
        pFind = (TPtr)TcsCharLastNotOf(pszText, cchText, EckStrAndLen(SpaceCharsA));
    else
        pFind = (TPtr)TcsCharLastNotOf(pszText, cchText, EckStrAndLen(SpaceCharsW));
    return pFind ? pFind + 1 : pszText;
}

template<CcpStdCharPtr TPtr>
EckInlineNdCe std::basic_string_view<RemoveStdCharPtr_T<TPtr>> TrimString(
    _In_reads_(cchText) TPtr pszText, int cchText) noexcept
{
    auto pStart = TrimStringLeft(pszText, cchText);
    auto pEnd = TrimStringRight(pStart, cchText);
    if (pStart >= pEnd)
        return {};
    else
        return { pStart, size_t(pEnd - pStart) };
}

template<CcpStdCharPtr TPtr, class TProcessor>
inline void SplitString(TPtr pszText, int cchText,
    ConstStdCharPtr_T<TPtr> pszDiv, int cchDiv,
    int cSubTextExpected, TProcessor&& Processor) noexcept
{
    using TChar = RemoveStdCharPtr_T<TPtr>;
    if (cchText < 0)
        cchText = (int)TcsLength(pszText);
    if (cchDiv < 0)
        cchDiv = (int)TcsLength(pszDiv);

    auto pszFind = TcsStringLength(pszText, cchText, pszDiv, cchDiv);
    auto pszPrevFirst = pszText;
    int c{};
    while (pszFind)
    {
        EckCanCallbackContinue(Processor(
            (TChar*)pszPrevFirst, int(pszFind - pszPrevFirst)))
            return;
        ++c;
        if (c == cSubTextExpected)
            return;
        pszPrevFirst = pszFind + cchDiv;
        pszFind = TcsStringLength(pszPrevFirst, cchText - (pszPrevFirst - pszText), pszDiv, cchDiv);
    }
    Processor((TChar*)pszPrevFirst, int(pszText + cchText - pszPrevFirst));
}
// For compatibility.
template<CcpStdCharPtr TPtr, class TProcessor>
EckInline void SplitString(TPtr pszText, ConstStdCharPtr_T<TPtr> pszDiv,
    int cSubTextExpected, int cchText, int cchDiv, TProcessor&& Processor) noexcept
{
    SplitString(pszText, cchText, pszDiv, cchDiv, cSubTextExpected,
        std::forward<TProcessor>(Processor));
}


template<CcpStdCharPtr TPtr, class TProcessor>
inline void SplitStringMultipleChar(TPtr pszText, int cchText,
    ConstStdCharPtr_T<TPtr> pszDiv, int cchDiv,
    int cSubTextExpected, TProcessor&& Processor) noexcept
{
    using TChar = RemoveStdCharPtr_T<TPtr>;
    if (cchText < 0)
        cchText = (int)TcsLength(pszText);
    if (cchDiv < 0)
        cchDiv = (int)TcsLength(pszDiv);

    auto pszFind = TcsCharFirstOf(pszText, cchText, pszDiv, cchDiv);
    auto pszPrevFirst = pszText;
    int c{};
    while (pszFind)
    {
        EckCanCallbackContinue(Processor(
            (TChar*)pszPrevFirst, int(pszFind - pszPrevFirst)))
            return;
        ++c;
        if (c == cSubTextExpected)
            return;
        pszPrevFirst = pszFind + 1;
        pszFind = TcsCharFirstOf(pszPrevFirst, cchText - (pszPrevFirst - pszText), pszDiv, cchDiv);
    }

    Processor((TChar*)pszPrevFirst, int(pszText + cchText - pszPrevFirst));
}
// For compatibility.
template<CcpStdCharPtr TPtr, class TProcessor>
EckInline void SplitStringMultipleChar(TPtr pszText, ConstStdCharPtr_T<TPtr> pszDiv,
    int cSubTextExpected, int cchText, int cchDiv, TProcessor&& Processor) noexcept
{
    SplitStringMultipleChar(pszText, cchText, pszDiv, cchDiv,
        cSubTextExpected, std::forward<TProcessor>(Processor));
}

template<CcpNonConstStdCharPtr TPtr, CcpStdCharPtr TPtr2>
    requires IsSameStdCharPtr_V<TPtr, TPtr2>
EckInline void SplitStringAndCut(TPtr pszText, int cchText, ConstStdCharPtr_T<TPtr> pszDiv, int cchDiv,
    std::vector<TPtr2>& vResult, int cSubTextExpected = 0) noexcept
{
    SplitString(pszText, cchText, pszDiv, cchDiv, cSubTextExpected,
        [&](TPtr pszStart, int cchSub)
        {
            *(pszStart + cchSub) = 0;
            vResult.push_back(pszStart);
        });
}

template<CcpNonConstStdCharPtr TPtr, CcpStdCharPtr TPtr2>
    requires IsSameStdCharPtr_V<TPtr, TPtr2>
EckInline void SplitStringMultipleCharAndCut(TPtr pszText, int cchText,
    ConstStdCharPtr_T<TPtr> pszDiv, int cchDiv,
    std::vector<TPtr2>& vResult, int cSubTextExpected = 0) noexcept
{
    SplitStringMultipleChar(pszText, pszDiv, cSubTextExpected, cchText, cchDiv,
        [&](TPtr pszStart, int cchSub)
        {
            *(pszStart + cchSub) = 0;
            vResult.push_back(pszStart);
        });
}
ECK_NAMESPACE_END