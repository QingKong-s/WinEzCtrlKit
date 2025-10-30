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

template<CcpIsStdChar TChar>
EckInlineNd TChar TchToUpper(TChar c)
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
template<CcpIsStdChar TChar>
EckInlineNd TChar TchToLower(TChar c)
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

template<CcpIsStdChar TChar>
EckInlineNdCe bool TchEqual(TChar c1, TChar c2)
{
	return c1 == c2;
}
template<CcpIsStdChar TChar>
EckInlineNd bool TchEqualI(TChar c1, TChar c2)
{
	return TchToUpper(c1) == TchToUpper(c2);
}

template<CcpIsStdCharPtr TPtr>
EckInlineNd size_t TcsLen(_In_z_ TPtr Str)
{
	if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
		return strlen(Str);
	else
		return wcslen(Str);
}

template<CcpIsStdCharPtr TPtr>
EckInlineNd bool TcsEqual(_In_z_ TPtr Str1, _In_z_ ConstStdCharPtr_T<TPtr> Str2)
{
	if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
		return strcmp(Str1, Str2) == 0;
	else
		return wcscmp(Str1, Str2) == 0;
}
template<CcpIsStdCharPtr TPtr>
EckInlineNd bool TcsEqualI(_In_z_ TPtr Str1, _In_z_ ConstStdCharPtr_T<TPtr> Str2)
{
	if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
		return stricmp(Str1, Str2) == 0;
	else
		return wcsicmp(Str1, Str2) == 0;
}

template<CcpIsStdCharPtr TPtr>
EckInlineNd bool TcsEqualMaxLen(_In_reads_or_z_(Max) TPtr Str1,
	_In_reads_or_z_(Max) ConstStdCharPtr_T<TPtr> Str2, size_t Max)
{
	if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
		return strncmp(Str1, Str2, Max) == 0;
	else
		return wcsncmp(Str1, Str2, Max) == 0;
}
template<CcpIsStdCharPtr TPtr>
EckInlineNd bool TcsEqualMaxLenI(_In_reads_or_z_(Max) TPtr Str1,
	_In_reads_or_z_(Max) ConstStdCharPtr_T<TPtr> Str2, size_t Max)
{
	if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
		return strnicmp(Str1, Str2, Max) == 0;
	else
		return wcsnicmp(Str1, Str2, Max) == 0;
}

template<CcpIsStdCharPtr TPtr>
EckInlineNd bool TcsEqualLen(_In_reads_(Len) TPtr Str1,
	_In_reads_(Len) ConstStdCharPtr_T<TPtr> Str2, size_t Len)
{
	if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
		return memcmp(Str1, Str2, Len) == 0;
	else
		return wmemcmp(Str1, Str2, Len) == 0;
}
template<CcpIsStdCharPtr TPtr>
EckInlineNd bool TcsEqualLenI(_In_reads_(Len) TPtr Str1,
	_In_reads_(Len) ConstStdCharPtr_T<TPtr> Str2, size_t Len)
{
	EckCounter(Len, i)
	{
		if (!TchEqualI(Str1[i], Str2[i]))
			return false;
	}
	return true;
}

template<CcpIsStdCharPtr TPtr>
EckInlineNd bool TcsEqualLen2(_In_reads_(Len1) TPtr Str1, size_t Len1,
	_In_reads_(Len2) ConstStdCharPtr_T<TPtr> Str2, size_t Len2)
{
	if (Len1 != Len2)
		return false;
	return TcsEqualLen(Str1, Str2, Len1);
}
template<CcpIsStdCharPtr TPtr>
EckInlineNd bool TcsEqualLen2I(_In_reads_(Len1) TPtr Str1, size_t Len1,
	_In_reads_(Len2) ConstStdCharPtr_T<TPtr> Str2, size_t Len2)
{
	if (Len1 != Len2)
		return false;
	return TcsEqualLenI(Str1, Str2, Len1);
}

template<CcpIsStdCharPtr TPtr>
EckInlineNd bool TcsIsStartWithLen2(_In_reads_(Len1) TPtr Str1, size_t Len1,
	_In_reads_(Len2) ConstStdCharPtr_T<TPtr> Str2, size_t Len2)
{
	if (Len1 < Len2)
		return false;
	return TcsEqualLen(Str1, Str2, Len2);
}
template<CcpIsStdCharPtr TPtr>
EckInlineNd bool TcsIsStartWithLen2I(_In_reads_(Len1) TPtr Str1, size_t Len1,
	_In_reads_(Len2) ConstStdCharPtr_T<TPtr> Str2, size_t Len2)
{
	if (Len1 < Len2)
		return false;
	return TcsEqualLenI(Str1, Str2, Len2);
}

template<CcpIsStdCharPtr TPtr>
EckInlineNd bool TcsIsEndWithLen2(_In_reads_(Len1) TPtr Str1, size_t Len1,
	_In_reads_(Len2) ConstStdCharPtr_T<TPtr> Str2, size_t Len2)
{
	if (Len1 < Len2)
		return false;
	return TcsEqualLen(Str1 + Len1 - Len2, Str2, Len2);
}
template<CcpIsStdCharPtr TPtr>
EckInlineNd bool TcsIsEndWithLen2I(_In_reads_(Len1) TPtr Str1, size_t Len1,
	_In_reads_(Len2) ConstStdCharPtr_T<TPtr> Str2, size_t Len2)
{
	if (Len1 < Len2)
		return false;
	return TcsEqualLenI(Str1 + Len1 - Len2, Str2, Len2);
}

template<CcpIsStdCharPtr TPtr>
_Ret_maybenull_ EckInlineNd TPtr TcsChar(_In_z_ TPtr Str, RemoveStdCharPtr_T<TPtr> c)
{
	if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
		return (TPtr)strchr((CHAR*)Str, (CHAR)c);
	else
		return (TPtr)wcschr((WCHAR*)Str, (WCHAR)c);
}
template<CcpIsStdCharPtr TPtr>
_Ret_maybenull_ EckInlineNd TPtr TcsCharLen(_In_reads_(Len) TPtr Str, size_t Len,
	RemoveStdCharPtr_T<TPtr> c)
{
	if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
		return (TPtr)memchr((CHAR*)Str, (CHAR)c, Len);
	else
		return (TPtr)wmemchr((WCHAR*)Str, (WCHAR)c, Len);
}

template<CcpIsStdCharPtr TPtr>
_Ret_maybenull_ EckInlineNd TPtr TcsRChar(_In_z_ TPtr Str, RemoveStdCharPtr_T<TPtr> c)
{
	if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
		return (TPtr)strrchr((CHAR*)Str, (CHAR)c);
	else
		return (TPtr)wcsrchr((WCHAR*)Str, (WCHAR)c);
}
template<CcpIsStdCharPtr TPtr>
_Ret_maybenull_ EckInlineNd TPtr TcsRCharLen(_In_reads_(Len) TPtr Str, size_t Len,
	RemoveStdCharPtr_T<TPtr> c)
{
	for (auto p = Str + Len - 1; p >= Str; --p)
	{
		if (*p == c)
			return p;
	}
	return nullptr;
}

template<CcpIsStdCharPtr TPtr>
_Ret_maybenull_ EckInlineNd TPtr TcsStr(_In_z_ TPtr Str, _In_z_ ConstStdCharPtr_T<TPtr> SubStr)
{
	if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
		return (TPtr)strstr((CHAR*)Str, SubStr);
	else
		return (TPtr)wcsstr((WCHAR*)Str, SubStr);
}
template<CcpIsStdCharPtr TPtr>
_Ret_maybenull_ EckInlineNd TPtr TcsStrI(_In_z_ TPtr Str, _In_z_ ConstStdCharPtr_T<TPtr> SubStr)
{
	if (!*SubStr)
		return Str;
	for (; *Str; ++Str)
	{
		if (TchEqualI(*Str, *SubStr) && TcsEqualI(Str, SubStr))
			return Str;
	}
	return nullptr;
}

template<CcpIsStdCharPtr TPtr>
_Ret_maybenull_ EckInlineNd TPtr TcsStrLen(_In_reads_(Len) TPtr Str, size_t Len,
	_In_reads_(SubLen) ConstStdCharPtr_T<TPtr> SubStr, size_t SubLen)
{
	if (Len < SubLen)
		return nullptr;
	if (SubLen == 0 || *SubStr == '\0')
		return Str;
	const auto pEnd = Str + Len - SubLen + 1;
	for (auto p = Str; p < pEnd; ++p)
	{
		const auto pFind = TcsCharLen(p, pEnd - p, *SubStr);
		if (!pFind)
			return nullptr;
		if (TcsEqualLen(pFind, SubStr, SubLen))
			return pFind;
		p = pFind + 1;
	}
	return nullptr;
}
template<CcpIsStdCharPtr TPtr>
_Ret_maybenull_ EckInlineNd TPtr TcsStrLenI(_In_reads_(Len) TPtr Str, size_t Len,
	_In_reads_(SubLen) ConstStdCharPtr_T<TPtr> SubStr, size_t SubLen)
{
	if (Len < SubLen)
		return nullptr;
	if (SubLen == 0 || *SubStr == '\0')
		return Str;
	for (auto p = Str; p < Str + Len - SubLen + 1; ++p)
	{
		if (TchEqualI(*p, *SubStr) && TcsEqualLenI(p, SubStr, SubLen))
			return p;
	}
	return nullptr;
}

template<CcpIsStdCharPtr TPtr>
_Ret_maybenull_ EckInlineNd TPtr TcsRStrLen(_In_reads_(Len) TPtr Str, size_t Len,
	_In_reads_(SubLen) ConstStdCharPtr_T<TPtr> SubStr, size_t SubLen, size_t posStart = SizeTMax)
{
	if (Len < SubLen)
		return nullptr;
	if (SubLen == 0 || *SubStr == '\0')
		return Str + std::min(posStart, Len - SubLen);
	for (auto p = Str + std::min(posStart, Len - SubLen); p >= Str; --p)
	{
		if (TchEqual(*p, *SubStr) && TcsEqualLen(p, SubStr, SubLen))
			return p;
	}
	return nullptr;
}
template<CcpIsStdCharPtr TPtr>
_Ret_maybenull_ EckInlineNd TPtr TcsRStrLenI(_In_reads_(Len) TPtr Str, size_t Len,
	_In_reads_(SubLen) ConstStdCharPtr_T<TPtr> SubStr, size_t SubLen, size_t posStart = SizeTMax)
{
	if (Len < SubLen)
		return nullptr;
	if (SubLen == 0 || *SubStr == '\0')
		return Str + std::min(posStart, Len - SubLen);
	for (auto p = Str + std::min(posStart, Len - SubLen); p >= Str; --p)
	{
		if (TchEqualI(*p, *SubStr) && TcsEqualLenI(p, SubStr, SubLen))
			return p;
	}
	return nullptr;
}

template<CcpIsStdCharPtr TPtr>
_Ret_maybenull_ EckInlineNd TPtr TcsChrFirstOf(_In_reads_(Len) TPtr Str, size_t Len,
	_In_reads_(CharsLen) ConstStdCharPtr_T<TPtr> Chars, size_t CharsLen)
{
#if defined(_MSC_VER) && _USE_STD_VECTOR_ALGORITHMS
	TPtr r;
	if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
		r = (TPtr)__std_find_first_of_trivial_1(Str, Str + Len, Chars, Chars + CharsLen);
	else
		r = (TPtr)__std_find_first_of_trivial_2(Str, Str + Len, Chars, Chars + CharsLen);
	return (r == Str + Len) ? nullptr : r;
#else
	for (auto p = Str; p < Str + Len; ++p)
	{
		if (TcsCharLen(Chars, CharsLen, *p))
			return p;
	}
	return nullptr;
#endif
}
template<CcpIsStdCharPtr TPtr>
_Ret_maybenull_ EckInlineNd TPtr TcsChrFirstNotOf(_In_reads_(Len) TPtr Str, size_t Len,
	_In_reads_(CharsLen) ConstStdCharPtr_T<TPtr> Chars, size_t CharsLen)
{
	for (auto p = Str; p < Str + Len; ++p)
	{
		if (!TcsCharLen(Chars, CharsLen, *p))
			return p;
	}
	return nullptr;
}
template<CcpIsStdCharPtr TPtr>
_Ret_maybenull_ EckInlineNd TPtr TcsChrLastOf(_In_reads_(Len) TPtr Str, size_t Len,
	_In_reads_(CharsLen) ConstStdCharPtr_T<TPtr> Chars, size_t CharsLen, size_t posStart = SizeTMax)
{
	EckAssert(Len);
#if defined(_MSC_VER) && _USE_STD_VECTOR_ALGORITHMS
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
		if (TcsCharLen(Chars, CharsLen, *p))
			return p;
	}
	return nullptr;
#endif
}
template<CcpIsStdCharPtr TPtr>
_Ret_maybenull_ EckInlineNd TPtr TcsChrLastNotOf(_In_reads_(Len) TPtr Str, size_t Len,
	_In_reads_(CharsLen) ConstStdCharPtr_T<TPtr> Chars, size_t CharsLen, size_t posStart = SizeTMax)
{
	EckAssert(Len);
	for (auto p = Str + std::min(posStart, Len - 1); p >= Str; --p)
	{
		if (!TcsCharLen(Chars, CharsLen, *p))
			return p;
	}
	return nullptr;
}

template<CcpIsStdCharPtr TPtr>
_Ret_maybenull_ EckInlineNd TPtr TcsChrFirstOf(_In_z_ TPtr Str, _In_z_ ConstStdCharPtr_T<TPtr> Chars)
{
	if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
		return (TPtr)strpbrk(Str, Chars);
	else
		return (TPtr)wcspbrk(Str, Chars);
}

template<CcpIsNonConstStdCharPtr TPtr>
_Post_equal_to_(Dst) EckInlineNd TPtr TcsCopy(_Out_writes_z_(_String_length_(Src) + 1) TPtr Dst,
	_In_z_ ConstStdCharPtr_T<TPtr> Src)
{
	if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
		return strcpy(Dst, Src);
	else
		return wcscpy(Dst, Src);
}
template<CcpIsNonConstStdCharPtr TPtr>
_Post_equal_to_(Dst) EckInlineNd TPtr TcsCopyLen(_Out_writes_(Len) TPtr Dst,
	_In_z_ ConstStdCharPtr_T<TPtr> Src, size_t Len)
{
	if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
		return (TPtr)memcpy(Dst, Src, Len);
	else
		return (TPtr)wmemcpy(Dst, Src, Len);
}
template<CcpIsNonConstStdCharPtr TPtr>
_Post_equal_to_(Dst) EckInlineNd TPtr TcsCopyLenEnd(_Out_writes_(Len) TPtr Dst,
	_In_z_ ConstStdCharPtr_T<TPtr> Src, size_t Len)
{
	*(TcsCopyLen(Dst, Src, Len) + Len) = 0;
	return Dst;
}

template<CcpIsNonConstStdCharPtr TPtr>
_Post_equal_to_(Dst) EckInlineNd TPtr TcsMoveLen(_Out_writes_(Len) TPtr Dst,
	_In_z_ ConstStdCharPtr_T<TPtr> Src, size_t Len)
{
	if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
		return (TPtr)memmove(Dst, Src, Len);
	else
		return (TPtr)wmemmove(Dst, Src, Len);
}
template<CcpIsNonConstStdCharPtr TPtr>
_Post_equal_to_(Dst) EckInlineNd TPtr TcsMoveLenEnd(_Out_writes_(Len) TPtr Dst,
	_In_z_ ConstStdCharPtr_T<TPtr> Src, size_t Len)
{
	*(TcsMoveLen(Dst, Src, Len) + Len) = 0;
	return Dst;
}

template<CcpIsStdCharPtr TPtr>
EckInlineNd int TcsCompare(_In_z_ TPtr Str1, _In_z_ ConstStdCharPtr_T<TPtr> Str2)
{
	if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
		return strcmp(Str1, Str2);
	else
		return wcscmp(Str1, Str2);
}
template<CcpIsStdCharPtr TPtr>
EckInlineNd int TcsCompareMaxLen(_In_reads_or_z_(Max) TPtr Str1,
	_In_reads_or_z_(Max) ConstStdCharPtr_T<TPtr> Str2, size_t Max)
{
	if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
		return strncmp(Str1, Str2, Max);
	else
		return wcsncmp(Str1, Str2, Max);
}
template<CcpIsStdCharPtr TPtr>
EckInlineNd int TcsCompareLen(_In_reads_(Len) TPtr Str1,
	_In_reads_(Len) ConstStdCharPtr_T<TPtr> Str2, size_t Len)
{
	if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
		return memcmp(Str1, Str2, Len);
	else
		return wmemcmp(Str1, Str2, Len);
}
template<CcpIsStdCharPtr TPtr>
EckInlineNd int TcsCompareLen2(_In_reads_(Len1) TPtr Str1, size_t Len1,
	_In_reads_(Len2) ConstStdCharPtr_T<TPtr> Str2, size_t Len2)
{
	const auto r = TcsCompareLen(Str1, Str2, std::min(Len1, Len2));
	if (r)
		return r;
	if (Len1 < Len2)
		return -1;
	else if (Len1 > Len2)
		return 1;
	else
		return 0;
}

template<CcpIsStdCharPtr TPtr>
EckInlineNd int TcsCompareI(_In_z_ TPtr Str1, _In_z_ ConstStdCharPtr_T<TPtr> Str2)
{
	if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
		return stricmp(Str1, Str2);
	else
		return wcsicmp(Str1, Str2);
}
template<CcpIsStdCharPtr TPtr>
EckInlineNd int TcsCompareMaxLenI(_In_reads_or_z_(Max) TPtr Str1,
	_In_reads_or_z_(Max) ConstStdCharPtr_T<TPtr> Str2, size_t Max)
{
	if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
		return strnicmp(Str1, Str2, Max);
	else
		return wcsnicmp(Str1, Str2, Max);
}
template<CcpIsStdCharPtr TPtr>
EckInlineNd int TcsCompareLenI(_In_reads_(Len) TPtr Str1,
	_In_reads_(Len) ConstStdCharPtr_T<TPtr> Str2, size_t Len)
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
template<CcpIsStdCharPtr TPtr>
EckInlineNd int TcsCompareLen2I(_In_reads_(Len1) TPtr Str1, size_t Len1,
	_In_reads_(Len2) ConstStdCharPtr_T<TPtr> Str2, size_t Len2)
{
	const auto r = TcsCompareLenI(Str1, Str2, std::min(Len1, Len2));
	if (r)
		return r;
	if (Len1 < Len2)
		return -1;
	else if (Len1 > Len2)
		return 1;
	else
		return 0;
}

template<CcpIsNonConstStdCharPtr TPtr>
EckInlineNd TPtr TcsSet(_Out_writes_z_(cchDst) TPtr Dst,
	RemoveStdCharPtr_T<TPtr> ch, size_t cchDst)
{
	if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
		return (TPtr)memset(Dst, ch, cchDst);
	else
		return (TPtr)wmemset(Dst, ch, cchDst);
}

namespace Priv
{
	inline constexpr unsigned char CharToDigitTable[]{ 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 255, 255, 255, 255, 255,
	255, 255, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
	255, 255, 255, 255, 255, 255, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
	31, 32, 33, 34, 35, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 };
}

enum class TcsCvtErr
{
	Ok,
	Overflow,
	Nothing,
};

template<CcpIsStdCharPtr TPtr, std::integral TInt>
inline TcsCvtErr TcsToInt(_In_reads_(Len) TPtr Str, size_t Len,
	_Out_ TInt& i, int iRadix = 0, _Outptr_opt_ TPtr* EndPtr = nullptr)
{
	const auto pEnd = Str + Len;
	BOOL bNeg = FALSE;
	// 跳过前导空白
	for (; *Str == ' ' || *Str == '\t' || *Str == '\r' || *Str == '\n'; ++Str)
	{
		if (Str == pEnd)
		{
			if (EndPtr) *EndPtr = Str;
			i = {};
			return TcsCvtErr::Ok;
		}
	}
	// 识别符号
	if (*Str == '-')
		bNeg = TRUE, ++Str;
	else if (*Str == '+')
		++Str;
	// 若未指定进制，则尝试识别进制
	if (iRadix == 0)
	{
		if ((*Str == '0' && (Str[1] == 'x' || Str[1] == 'X')))// 0x, 0X
		{
			Str += 2;
			iRadix = 16;
		}
		else if (*Str == '0' && (Str[1] == 'b' || Str[1] == 'B'))// 0b, 0B
		{
			Str += 2;
			iRadix = 2;
		}
		else
			iRadix = 10;
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
	TUnsigned uResult = 0;
	const auto pBegin = Str;
	for (; Str < pEnd; ++Str)
	{
		const auto ch = *Str;
		if constexpr (sizeof(RemoveStdCharPtr_T<TPtr>) > 1)
			if (ch >= ARRAYSIZE(Priv::CharToDigitTable))
				break;
		const int Digit = Priv::CharToDigitTable[ch];
		if (Digit >= iRadix)
			break;
		if (uResult > (Max - Digit) / iRadix)
		{
			if (EndPtr) *EndPtr = Str;
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
			uResult = uResult * iRadix + Digit;
	}
	if (Str == pBegin)
	{
		if (EndPtr) *EndPtr = Str;
		i = {};
		return TcsCvtErr::Nothing;
	}
	if constexpr (std::is_signed_v<TInt>)
		if (bNeg)
			uResult = TUnsigned(TUnsigned{} - uResult);
	i = TInt(uResult);
	if (EndPtr) *EndPtr = Str;
	return TcsCvtErr::Ok;
}



template<CcpIsStdCharPtr TPtr>
EckInlineNd int FindStr(_In_z_ TPtr pszText,
	_In_z_ ConstStdCharPtr_T<TPtr> pszSub, int posStart = 0)
{
	const auto pszFind = TcsStr(pszText + posStart, pszSub);
	return pszFind ? int(pszFind - pszText) : StrNPos;
}
template<CcpIsStdCharPtr TPtr>
EckInlineNd int FindStrI(_In_z_ TPtr pszText,
	_In_z_ ConstStdCharPtr_T<TPtr> pszSub, int posStart = 0)
{
	const auto pszFind = TcsStrI(pszText + posStart, pszSub);
	return pszFind ? int(pszFind - pszText) : StrNPos;
}

template<CcpIsStdCharPtr TPtr>
EckInlineNd int FindStrLen(_In_reads_(cchText) TPtr pszText, int cchText,
	_In_reads_(cchSub) ConstStdCharPtr_T<TPtr> pszSub, int cchSub, int posStart = 0)
{
	const auto pFind = TcsStrLen(pszText + posStart, cchText - posStart, pszSub, cchSub);
	return pFind ? int(pFind - pszText) : StrNPos;
}
template<CcpIsStdCharPtr TPtr>
EckInlineNd int FindStrLenI(_In_reads_(cchText) TPtr pszText, int cchText,
	_In_reads_(cchSub) ConstStdCharPtr_T<TPtr> pszSub, int cchSub, int posStart = 0)
{
	const auto pFind = TcsStrLenI(pszText + posStart, cchText - posStart, pszSub, cchSub);
	return pFind ? int(pFind - pszText) : StrNPos;
}

template<CcpIsStdCharPtr TPtr>
EckInlineNd int FindStrRev(_In_reads_(cchText) TPtr pszText, int cchText,
	_In_reads_(cchSub) ConstStdCharPtr_T<TPtr> pszSub, int cchSub, int posStart = -1)
{
	const auto pFind = TcsRStrLen(pszText, cchText, pszSub, cchSub, posStart);
	return pFind ? int(pFind - pszText) : StrNPos;
}
template<CcpIsStdCharPtr TPtr>
EckInlineNd int FindStrRevI(_In_reads_(cchText) TPtr pszText, int cchText,
	_In_reads_(cchSub) ConstStdCharPtr_T<TPtr> pszSub, int cchSub, int posStart = -1)
{
	const auto pFind = TcsRStrLenI(pszText, cchText, pszSub, cchSub, posStart);
	return pFind ? int(pFind - pszText) : StrNPos;
}

template<CcpIsStdCharPtr TPtr>
EckInlineNd int FindChar(_In_z_ TPtr pszText, RemoveStdCharPtr_T<TPtr> ch, int posStart = 0)
{
	const auto pszFind = TcsChar(pszText + posStart, ch);
	return pszFind ? int(pszFind - pszText) : StrNPos;
}
template<CcpIsStdCharPtr TPtr>
EckInlineNd int FindCharLen(_In_reads_(cchText) TPtr pszText, int cchText,
	RemoveStdCharPtr_T<TPtr> ch, int posStart = 0)
{
	EckAssert(posStart >= 0 && posStart <= cchText);
	const auto pFind = TcsCharLen(pszText + posStart, cchText - posStart, ch);
	return pFind ? int(pFind - pszText) : StrNPos;
}
template<CcpIsStdCharPtr TPtr>
EckInlineNd int FindCharRevLen(_In_reads_(cchText) TPtr pszText, int cchText,
	RemoveStdCharPtr_T<TPtr> ch, int posStart = -1)
{
	if (posStart < 0)
		posStart = cchText - 1;
	EckAssert(posStart >= 0 && posStart <= cchText);
	const auto pFind = TcsRCharLen(pszText, std::min(posStart + 1, cchText), ch);
	return pFind ? int(pFind - pszText) : StrNPos;
}

template<CcpIsStdCharPtr TPtr>
EckInlineNd int FindCharFirstOf(_In_z_ TPtr pszText, int cchText,
	_In_reads_(cchChars) ConstStdCharPtr_T<TPtr> pszChars, int cchChars, int posStart = 0)
{
	const auto pszFind = TcsChrFirstOf(pszText + posStart, pszChars, cchChars);
	return pszFind ? int(pszFind - pszText) : StrNPos;
}
template<CcpIsStdCharPtr TPtr>
EckInlineNd int FindCharFirstNotOf(_In_z_ TPtr pszText, int cchText,
	_In_reads_(cchChars) ConstStdCharPtr_T<TPtr> pszChars, int cchChars, int posStart = 0)
{
	const auto pszFind = TcsChrFirstNotOf(pszText + posStart, cchText - posStart, pszChars, cchChars);
	return pszFind ? int(pszFind - pszText) : StrNPos;
}
template<CcpIsStdCharPtr TPtr>
EckInlineNd int FindCharLastOf(_In_z_ TPtr pszText, int cchText,
	_In_reads_(cchChars) ConstStdCharPtr_T<TPtr> pszChars, int cchChars, int posStart = -1)
{
	const auto pszFind = TcsChrLastOf(pszText, cchText, pszChars, cchChars, posStart);
	return pszFind ? int(pszFind - pszText) : StrNPos;
}
template<CcpIsStdCharPtr TPtr>
EckInlineNd int FindCharLastNotOf(_In_z_ TPtr pszText, int cchText,
	_In_reads_(cchChars) ConstStdCharPtr_T<TPtr> pszChars, int cchChars, int posStart = -1)
{
	const auto pszFind = TcsChrLastNotOf(pszText, cchText, pszChars, cchChars, posStart);
	return pszFind ? int(pszFind - pszText) : StrNPos;
}

// Deprecated. For compatibility.
template<CcpIsStdCharPtr TPtr>
EckInlineNd TPtr LTrimStr(_In_z_ TPtr pszText)
{
	auto ch = *pszText;
	while ((ch == L' ' || ch == L'　') && ch != L'\0')
		ch = *++pszText;
	return pszText;
}

template<CcpIsStdCharPtr TPtr>
EckInlineNd TPtr LTrimStr(_In_reads_(cchText) TPtr pszText, int cchText)
{
	if (cchText < 0)
		cchText = (int)TcsLen(pszText);
	if (!cchText)
		return pszText;
	TPtr pFind;
	if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
		pFind = (TPtr)TcsChrFirstNotOf(pszText, cchText, EckStrAndLen(SpaceCharsA));
	else
		pFind = (TPtr)TcsChrFirstNotOf(pszText, cchText, EckStrAndLen(SpaceCharsW));
	return pFind ? pFind : pszText + cchText;
}

template<CcpIsStdCharPtr TPtr>
EckInlineNd TPtr RTrimStr(_In_reads_(cchText) TPtr pszText, int cchText)
{
	if (cchText < 0)
		cchText = (int)TcsLen(pszText);
	if (!cchText)
		return pszText;
	TPtr pFind;
	if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
		pFind = (TPtr)TcsChrLastNotOf(pszText, cchText, EckStrAndLen(SpaceCharsA));
	else
		pFind = (TPtr)TcsChrLastNotOf(pszText, cchText, EckStrAndLen(SpaceCharsW));
	return pFind ? pFind + 1 : pszText;
}

template<CcpIsStdCharPtr TPtr>
EckInlineNdCe std::basic_string_view<RemoveStdCharPtr_T<TPtr>> LRTrimStr(
	_In_reads_(cchText) TPtr pszText, int cchText)
{
	auto pStart = LTrimStr(pszText, cchText);
	auto pEnd = RTrimStr(pStart, cchText);
	if (pStart >= pEnd)
		return {};
	else
		return { pStart, size_t(pEnd - pStart) };
}

template<CcpIsStdCharPtr TPtr, class TProcessor>
inline void SplitStr(TPtr pszText, int cchText,
	ConstStdCharPtr_T<TPtr> pszDiv, int cchDiv,
	int cSubTextExpected, TProcessor&& Processor)
{
	using TChar = RemoveStdCharPtr_T<TPtr>;
	if (cchText < 0)
		cchText = (int)TcsLen(pszText);
	if (cchDiv < 0)
		cchDiv = (int)TcsLen(pszDiv);

	auto pszFind = TcsStrLen(pszText, cchText, pszDiv, cchDiv);
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
		pszFind = TcsStrLen(pszPrevFirst, cchText - (pszPrevFirst - pszText), pszDiv, cchDiv);
	}
	Processor((TChar*)pszPrevFirst, int(pszText + cchText - pszPrevFirst));
}
// For compatibility.
template<CcpIsStdCharPtr TPtr, class TProcessor>
EckInline void SplitStr(TPtr pszText, ConstStdCharPtr_T<TPtr> pszDiv,
	int cSubTextExpected, int cchText, int cchDiv, TProcessor&& Processor)
{
	SplitStr(pszText, cchText, pszDiv, cchDiv, cSubTextExpected,
		std::forward<TProcessor>(Processor));
}


template<CcpIsStdCharPtr TPtr, class TProcessor>
inline void SplitStrWithMultiChar(TPtr pszText, int cchText,
	ConstStdCharPtr_T<TPtr> pszDiv, int cchDiv,
	int cSubTextExpected, TProcessor&& Processor)
{
	using TChar = RemoveStdCharPtr_T<TPtr>;
	if (cchText < 0)
		cchText = (int)TcsLen(pszText);
	if (cchDiv < 0)
		cchDiv = (int)TcsLen(pszDiv);

	auto pszFind = TcsChrFirstOf(pszText, cchText, pszDiv, cchDiv);
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
		pszFind = TcsChrFirstOf(pszPrevFirst, cchText - (pszPrevFirst - pszText), pszDiv, cchDiv);
	}

	Processor((TChar*)pszPrevFirst, int(pszText + cchText - pszPrevFirst));
}
// For compatibility.
template<CcpIsStdCharPtr TPtr, class TProcessor>
EckInline void SplitStrWithMultiChar(TPtr pszText, ConstStdCharPtr_T<TPtr> pszDiv,
	int cSubTextExpected, int cchText, int cchDiv, TProcessor&& Processor)
{
	SplitStrWithMultiChar(pszText, cchText, pszDiv, cchDiv,
		cSubTextExpected, std::forward<TProcessor>(Processor));
}

template<CcpIsNonConstStdCharPtr TPtr, CcpIsStdCharPtr TPtr2>
	requires IsSameStdCharPtr_V<TPtr, TPtr2>
EckInline void SplitStrAndCut(TPtr pszText, int cchText, ConstStdCharPtr_T<TPtr> pszDiv, int cchDiv,
	std::vector<TPtr2>& vResult, int cSubTextExpected = 0)
{
	SplitStr(pszText, cchText, pszDiv, cchDiv, cSubTextExpected,
		[&](TPtr pszStart, int cchSub)
		{
			*(pszStart + cchSub) = 0;
			vResult.push_back(pszStart);
		});
}

template<CcpIsNonConstStdCharPtr TPtr, CcpIsStdCharPtr TPtr2>
	requires IsSameStdCharPtr_V<TPtr, TPtr2>
EckInline void SplitStrWithMultiCharAndCut(TPtr pszText, int cchText,
	ConstStdCharPtr_T<TPtr> pszDiv, int cchDiv,
	std::vector<TPtr2>& vResult, int cSubTextExpected = 0)
{
	SplitStrWithMultiChar(pszText, pszDiv, cSubTextExpected, cchText, cchDiv,
		[&](TPtr pszStart, int cchSub)
		{
			*(pszStart + cchSub) = 0;
			vResult.push_back(pszStart);
		});
}

// For compatibility.
template<CcpIsStdCharPtr TPtr>
EckInlineNd int FindStrNcs(TPtr pszText, int cchText,
	ConstStdCharPtr_T<TPtr> pszSub, int cchSub, int posStart = 0)
{
	return FindStrLenI(pszText, cchText, pszSub, cchSub, posStart);
}
// For compatibility.
template<CcpIsStdCharPtr TPtr>
EckInline int FindStrRevNcs(TPtr pszText, int cchText,
	ConstStdCharPtr_T<TPtr> pszSub, int cchSub, int posStart = -1)
{
	return FindStrRevI(pszText, cchText, pszSub, cchSub, posStart);
}
ECK_NAMESPACE_END