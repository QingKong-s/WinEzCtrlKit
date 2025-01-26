#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
inline constexpr int INVALID_STR_POS = -1;
inline constexpr int StrNPos = -1;

inline constexpr CHAR SpaceCharsA[]{ " \t\r\n" };
// U+00A0 non-breaking space
// U+2000-2006 EN space
// U+2007 figure space
// U+2008 Punctuation space
// U+2009 Thin space
// U+200A Hair space
// U+200B zero width space
// U+202F Narrow no-break space
// U+205F medium mathematical space
// U+3000 ideographic space
// U+FEFF zero width no-break space
inline constexpr WCHAR SpaceCharsW[]{ L" \t\r\n\u00A0\u2000\u2001\u2002\u2003\u2004\u2005\u2006\u2007\u2008\u2009\u200A\u200B\u202F\u205F\u3000\uFEFF" };

template<class TChar>
concept ccpIsStdChar = std::is_same_v<TChar, CHAR> || std::is_same_v<TChar, WCHAR>;

template<class TCharPtr>
concept ccpIsStdCharPtr = std::is_pointer_v<TCharPtr> && ccpIsStdChar<std::remove_cvref_t<std::remove_pointer_t<TCharPtr>>>;

template<ccpIsStdCharPtr TPtr>
using RemoveStdCharPtr_T = std::remove_cvref_t<std::remove_pointer_t<TPtr>>;


template<ccpIsStdChar TChar>
EckInlineNd TChar TchToUpper(TChar c)
{
	if constexpr (std::is_same_v<TChar, char>)
		return (TChar)toupper(c);
	else
		return (TChar)towupper(c);
}
template<ccpIsStdChar TChar>
EckInlineNd TChar TchToLower(TChar c)
{
	if constexpr (std::is_same_v<TChar, char>)
		return (TChar)tolower(c);
	else
		return (TChar)towlower(c);
}

template<ccpIsStdChar TChar>
EckInlineNdCe bool TchEqual(TChar c1, TChar c2)
{
	return c1 == c2;
}
template<ccpIsStdChar TChar>
EckInlineNd bool TchEqualI(TChar c1, TChar c2)
{
	return TchToUpper(c1) == TchToUpper(c2);
}

template<ccpIsStdCharPtr TPtr>
EckInlineNd size_t TcsLen(_In_z_ TPtr Str)
{
	if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
		return strlen((CHAR*)Str);
	else
		return wcslen((WCHAR*)Str);
}

template<ccpIsStdCharPtr TPtr>
EckInlineNd bool TcsEqual(_In_z_ TPtr Str1, _In_z_ TPtr Str2)
{
	if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
		return strcmp((CHAR*)Str1, (CHAR*)Str2) == 0;
	else
		return wcscmp((WCHAR*)Str1, (WCHAR*)Str2) == 0;
}
template<ccpIsStdCharPtr TPtr>
EckInlineNd bool TcsEqualI(_In_z_ TPtr Str1, _In_z_ TPtr Str2)
{
	if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
		return stricmp((CHAR*)Str1, (CHAR*)Str2) == 0;
	else
		return wcsicmp((WCHAR*)Str1, (WCHAR*)Str2) == 0;
}

template<ccpIsStdCharPtr TPtr>
EckInlineNd bool TcsEqualMaxLen(_In_z_ TPtr Str1, _In_z_ TPtr Str2, size_t Max)
{
	if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
		return strncmp((CHAR*)Str1, (CHAR*)Str2, Max) == 0;
	else
		return wcsncmp((WCHAR*)Str1, (WCHAR*)Str2, Max) == 0;
}
template<ccpIsStdCharPtr TPtr>
EckInlineNd bool TcsEqualMaxLenI(_In_z_ TPtr Str1, _In_z_ TPtr Str2, size_t Max)
{
	if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
		return strnicmp((CHAR*)Str1, (CHAR*)Str2, Max) == 0;
	else
		return wcsnicmp((WCHAR*)Str1, (WCHAR*)Str2, Max) == 0;
}

template<ccpIsStdCharPtr TPtr>
EckInlineNd bool TcsEqualLen(_In_reads_(Len) TPtr Str1, _In_reads_(Len) TPtr Str2, size_t Len)
{
	if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
		return memcmp((CHAR*)Str1, (CHAR*)Str2, Len) == 0;
	else
		return wmemcmp((WCHAR*)Str1, (WCHAR*)Str2, Len) == 0;
}
template<ccpIsStdCharPtr TPtr>
EckInlineNd bool TcsEqualLenI(_In_reads_(Len) TPtr Str1, _In_reads_(Len) TPtr Str2, size_t Len)
{
	EckCounter(Len, i)
	{
		if (!TchEqualI(Str1[i], Str2[i]))
			return false;
	}
	return true;
}

template<ccpIsStdCharPtr TPtr>
_Ret_maybenull_ EckInlineNd TPtr TcsChar(_In_z_ TPtr Str, RemoveStdCharPtr_T<TPtr> c)
{
	if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
		return (TPtr)strchr((CHAR*)Str, (CHAR)c);
	else
		return (TPtr)wcschr((WCHAR*)Str, (WCHAR)c);
}
template<ccpIsStdCharPtr TPtr>
_Ret_maybenull_ EckInlineNd TPtr TcsCharLen(_In_reads_(Len) TPtr Str, size_t Len,
	RemoveStdCharPtr_T<TPtr> c)
{
	if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
		return (TPtr)memchr((CHAR*)Str, (CHAR)c, Len);
	else
		return (TPtr)wmemchr((WCHAR*)Str, (WCHAR)c, Len);
}

template<ccpIsStdCharPtr TPtr>
_Ret_maybenull_ EckInlineNd TPtr TcsRChar(_In_z_ TPtr Str, RemoveStdCharPtr_T<TPtr> c)
{
	if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
		return (TPtr)strrchr((CHAR*)Str, (CHAR)c);
	else
		return (TPtr)wcsrchr((WCHAR*)Str, (WCHAR)c);
}
template<ccpIsStdCharPtr TPtr>
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

template<ccpIsStdCharPtr TPtr>
_Ret_maybenull_ EckInlineNd TPtr TcsStr(_In_z_ TPtr Str, _In_z_ TPtr SubStr)
{
	if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
		return (TPtr)strstr((CHAR*)Str, (CHAR*)SubStr);
	else
		return (TPtr)wcsstr((WCHAR*)Str, (WCHAR*)SubStr);
}
template<ccpIsStdCharPtr TPtr>
_Ret_maybenull_ EckInlineNd TPtr TcsStrI(_In_z_ TPtr Str, _In_z_ TPtr SubStr)
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

template<ccpIsStdCharPtr TPtr>
_Ret_maybenull_ EckInlineNd TPtr TcsStrLen(_In_reads_(Len) TPtr Str, size_t Len,
	_In_reads_(SubLen) TPtr SubStr, size_t SubLen)
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
	}
	return nullptr;
}
template<ccpIsStdCharPtr TPtr>
_Ret_maybenull_ EckInlineNd TPtr TcsStrLenI(_In_reads_(Len) TPtr Str, size_t Len,
	_In_reads_(SubLen) TPtr SubStr, size_t SubLen)
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

template<ccpIsStdCharPtr TPtr>
_Ret_maybenull_ EckInlineNd TPtr TcsRStrLen(_In_reads_(Len) TPtr Str, size_t Len,
	_In_reads_(SubLen) TPtr SubStr, size_t SubLen, size_t posStart = 0)
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
template<ccpIsStdCharPtr TPtr>
_Ret_maybenull_ EckInlineNd TPtr TcsRStrLenI(_In_reads_(Len) TPtr Str, size_t Len,
	_In_reads_(SubLen) TPtr SubStr, size_t SubLen, size_t posStart = 0)
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

template<ccpIsStdCharPtr TPtr>
_Ret_maybenull_ EckInlineNd TPtr TcsChrFirstOf(_In_reads_(cchText) TPtr Str, size_t Len,
	_In_reads_(cchChars) TPtr Chars, size_t CharsLen)
{
#if defined(_MSC_VER) && _USE_STD_VECTOR_ALGORITHMS
	if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
		return (TPtr)__std_find_first_of_trivial_1(Str, Str + Len, Chars, Chars + CharsLen);
	else
		return (TPtr)__std_find_first_of_trivial_2(Str, Str + Len, Chars, Chars + CharsLen);
#else
	for (auto p = Str; p < Str + Len; ++p)
	{
		if (TcsCharLen(Chars, CharsLen, *p))
			return p;
	}
	return nullptr;
#endif
}
template<ccpIsStdCharPtr TPtr>
_Ret_maybenull_ EckInlineNd TPtr TcsChrFirstNotOf(_In_reads_(cchText) TPtr Str, size_t Len,
	_In_reads_(cchChars) TPtr Chars, size_t CharsLen)
{
	for (auto p = Str; p < Str + Len; ++p)
	{
		if (!TcsCharLen(Chars, CharsLen, *p))
			return p;
	}
	return nullptr;
}
template<ccpIsStdCharPtr TPtr>
_Ret_maybenull_ EckInlineNd TPtr TcsChrLastOf(_In_reads_(cchText) TPtr Str, size_t Len,
	_In_reads_(cchChars) TPtr Chars, size_t CharsLen, size_t posStart = 0)
{
	EckAssert(Len);
#if defined(_MSC_VER) && _USE_STD_VECTOR_ALGORITHMS
	if constexpr (std::is_same_v<RemoveStdCharPtr_T<TPtr>, char>)
		return (TPtr)__std_find_last_of_trivial_pos_1(
			Str, std::min(posStart + 1, Len), Chars, CharsLen);
	else
		return (TPtr)__std_find_last_of_trivial_pos_2(
			Str, std::min(posStart + 1, Len), Chars, CharsLen);
#else
	for (auto p = Str + std::min(posStart, Len - 1); p >= Str; --p)
	{
		if (TcsCharLen(Chars, CharsLen, *p))
			return p;
	}
	return nullptr;
#endif
}
template<ccpIsStdCharPtr TPtr>
_Ret_maybenull_ EckInlineNd TPtr TcsChrLastNotOf(_In_reads_(cchText) TPtr Str, size_t Len,
	_In_reads_(cchChars) TPtr Chars, size_t CharsLen, size_t posStart = 0)
{
	EckAssert(Len);
	for (auto p = Str + std::min(posStart, Len - 1); p >= Str; --p)
	{
		if (!TcsCharLen(Chars, CharsLen, *p))
			return p;
	}
	return nullptr;
}




template<ccpIsStdCharPtr TPtr>
EckInlineNd int FindStr(_In_z_ TPtr pszText, _In_z_ TPtr pszSub, int posStart = 0)
{
	const auto pszFind = TcsStr(pszText + posStart, pszSub);
	return pszFind ? int(pszFind - pszText) : StrNPos;
}
template<ccpIsStdCharPtr TPtr>
EckInlineNd int FindStrI(_In_z_ TPtr pszText, _In_z_ TPtr pszSub, int posStart = 0)
{
	const auto pszFind = TcsStrI(pszText + posStart, pszSub);
	return pszFind ? int(pszFind - pszText) : StrNPos;
}

template<ccpIsStdCharPtr TPtr>
EckInlineNd int FindStrLen(_In_reads_(cchText) TPtr pszText, int cchText,
	_In_reads_(cchSub) TPtr pszSub, int cchSub, int posStart = 0)
{
	const auto pFind = TcsStrLen(pszText + posStart, cchText - posStart, pszSub, cchSub);
	return pFind ? int(pFind - pszText) : StrNPos;
}
template<ccpIsStdCharPtr TPtr>
EckInlineNd int FindStrLenI(_In_reads_(cchText) TPtr pszText, int cchText,
	_In_reads_(cchSub) TPtr pszSub, int cchSub, int posStart = 0)
{
	const auto pFind = TcsStrLenI(pszText + posStart, cchText - posStart, pszSub, cchSub);
	return pFind ? int(pFind - pszText) : StrNPos;
}

template<ccpIsStdCharPtr TPtr>
EckInlineNd int FindStrRev(_In_reads_(cchText) TPtr pszText, int cchText,
	_In_reads_(cchSub) TPtr pszSub, int cchSub, int posStart = -1)
{
	const auto pFind = TcsRStrLen(pszText, cchText, pszSub, cchSub, posStart);
	return pFind ? int(pFind - pszText) : StrNPos;
}
template<ccpIsStdCharPtr TPtr>
EckInlineNd int FindStrRevI(_In_reads_(cchText) TPtr pszText, int cchText,
	_In_reads_(cchSub) TPtr pszSub, int cchSub, int posStart = -1)
{
	const auto pFind = TcsRStrLenI(pszText, cchText, pszSub, cchSub, posStart);
	return pFind ? int(pFind - pszText) : StrNPos;
}

template<ccpIsStdCharPtr TPtr>
EckInlineNd int FindChar(_In_z_ TPtr pszText, RemoveStdCharPtr_T<TPtr> ch, int posStart = 0)
{
	const auto pszFind = TcsChar(pszText + posStart, ch);
	return pszFind ? int(pszFind - pszText) : StrNPos;
}
template<ccpIsStdCharPtr TPtr>
EckInlineNd int FindCharLen(_In_reads_(cchText) TPtr pszText, int cchText,
	RemoveStdCharPtr_T<TPtr> ch, int posStart = 0)
{
	EckAssert(posStart >= 0 && posStart <= cchText);
	const auto pFind = TcsCharLen(pszText + posStart, cchText - posStart, ch);
	return pFind ? int(pFind - pszText) : StrNPos;
}

template<ccpIsStdCharPtr TPtr>

ECK_NAMESPACE_END