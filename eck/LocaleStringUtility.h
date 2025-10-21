#pragma once
#include "NativeWrapper.h"

ECK_NAMESPACE_BEGIN
EckInlineNd int LcswCompareLen2(_In_reads_(Len1) PCWCH Str1, int Len1,
    _In_reads_(Len2) PCWCH Str2, int Len2,
    UINT uFlags, _In_opt_z_ PCWSTR pszLocaleName = LOCALE_NAME_USER_DEFAULT)
{
    const int r = CompareStringEx(pszLocaleName, uFlags,
        Str1, Len1, Str2, Len2, nullptr, nullptr, 0);
#ifdef _DEBUG
    if (!r)
    {
        EckDbgPrint(L"CompareStringEx failed.");
        EckDbgBreak();
    }
#endif
    return r - 2;
}
EckInlineNd int LcswCompareLen(_In_reads_(Len) PCWCH Str1,
    _In_reads_(Len) PCWCH Str2, int Len,
    UINT uFlags, _In_opt_z_ PCWSTR pszLocaleName = LOCALE_NAME_USER_DEFAULT)
{
    return LcswCompareLen2(Str1, Len, Str2, Len, uFlags, pszLocaleName);
}

EckInlineNd int LcswFindWorker(_In_reads_(Len) PCWCH Str, int Len,
    _In_reads_(SubLen) PCWCH SubStr, int SubLen,
    UINT uFlags, int* pcchMatch = nullptr, _In_opt_z_ PCWSTR pszLocaleName = LOCALE_NAME_USER_DEFAULT)
{
    const auto r = FindNLSStringEx(pszLocaleName, uFlags,
        Str, Len, SubStr, SubLen, pcchMatch, nullptr, nullptr, 0);
#ifdef _DEBUG
    if (r == -1)
    {
        const auto dwErr = NaGetLastError();
        EckDbgPrintFmt(L"FindNLSStringEx failed. Error code: %d", dwErr);
        EckDbgPrintFormatMessage(dwErr);
        EckDbgBreak();
    }
#endif
    return r;
}

EckInlineNd int LcswStrLen(_In_reads_(Len) PCWCH Str, int Len,
    _In_reads_(SubLen) PCWCH SubStr, int SubLen,
    UINT uFlags = 0, int* pcchMatch = nullptr, _In_opt_z_ PCWSTR pszLocaleName = LOCALE_NAME_USER_DEFAULT)
{
    return LcswFindWorker(Str, Len, SubStr, SubLen,
        uFlags | FIND_FROMSTART, pcchMatch, pszLocaleName);
}
EckInlineNd int LcswStrLenI(_In_reads_(Len) PCWCH Str, int Len,
    _In_reads_(SubLen) PCWCH SubStr, int SubLen,
    UINT uFlags = 0, int* pcchMatch = nullptr, _In_opt_z_ PCWSTR pszLocaleName = LOCALE_NAME_USER_DEFAULT)
{
    return LcswFindWorker(Str, Len, SubStr, SubLen,
        uFlags | FIND_FROMSTART | NORM_IGNORECASE, pcchMatch, pszLocaleName);
}
EckInlineNd int LcswStrRLen(_In_reads_(Len) PCWCH Str, int Len,
    _In_reads_(SubLen) PCWCH SubStr, int SubLen,
    UINT uFlags = 0, int* pcchMatch = nullptr, _In_opt_z_ PCWSTR pszLocaleName = LOCALE_NAME_USER_DEFAULT)
{
    return LcswFindWorker(Str, Len, SubStr, SubLen,
        uFlags | FIND_FROMEND, pcchMatch, pszLocaleName);
}
EckInlineNd int LcswStrRLenI(_In_reads_(Len) PCWCH Str, int Len,
    _In_reads_(SubLen) PCWCH SubStr, int SubLen,
    UINT uFlags = 0, int* pcchMatch = nullptr, _In_opt_z_ PCWSTR pszLocaleName = LOCALE_NAME_USER_DEFAULT)
{
    return LcswFindWorker(Str, Len, SubStr, SubLen,
        uFlags | FIND_FROMEND | NORM_IGNORECASE, pcchMatch, pszLocaleName);
}
EckInlineNd int LcswIsStartWith(_In_reads_(Len) PCWCH Str, int Len,
    _In_reads_(SubLen) PCWCH SubStr, int SubLen,
    UINT uFlags = 0, int* pcchMatch = nullptr, _In_opt_z_ PCWSTR pszLocaleName = LOCALE_NAME_USER_DEFAULT)
{
    return LcswFindWorker(Str, Len, SubStr, SubLen,
        uFlags | FIND_STARTSWITH, pcchMatch, pszLocaleName);
}
EckInlineNd int LcswIsStartWithI(_In_reads_(Len) PCWCH Str, int Len,
    _In_reads_(SubLen) PCWCH SubStr, int SubLen,
    UINT uFlags = 0, int* pcchMatch = nullptr, _In_opt_z_ PCWSTR pszLocaleName = LOCALE_NAME_USER_DEFAULT)
{
    return LcswFindWorker(Str, Len, SubStr, SubLen,
        uFlags | FIND_STARTSWITH | NORM_IGNORECASE, pcchMatch, pszLocaleName);
}
EckInlineNd int LcswIsEndWith(_In_reads_(Len) PCWCH Str, int Len,
    _In_reads_(SubLen) PCWCH SubStr, int SubLen,
    UINT uFlags = 0, int* pcchMatch = nullptr, _In_opt_z_ PCWSTR pszLocaleName = LOCALE_NAME_USER_DEFAULT)
{
    return LcswFindWorker(Str, Len, SubStr, SubLen,
        uFlags | FIND_ENDSWITH, pcchMatch, pszLocaleName);
}
EckInlineNd int LcswIsEndWithI(_In_reads_(Len) PCWCH Str, int Len,
    _In_reads_(SubLen) PCWCH SubStr, int SubLen,
    UINT uFlags = 0, int* pcchMatch = nullptr, _In_opt_z_ PCWSTR pszLocaleName = LOCALE_NAME_USER_DEFAULT)
{
    return LcswFindWorker(Str, Len, SubStr, SubLen,
        uFlags | FIND_ENDSWITH | NORM_IGNORECASE, pcchMatch, pszLocaleName);
}

EckNfInlineNdCe int LcsCountUtf8Char(_In_reads_or_z_(cch) PCCH psz, int cch)
{
    if (cch < 0)
        cch = (int)strlen(psz);
    int cchChars{};
    for (int i = 0; i < cch; )
    {
        const unsigned char ch = (unsigned char)psz[i];
        if (ch < 0x80)
            i += 1;
        else if ((ch & 0xE0) == 0xC0)
            i += 2;
        else if ((ch & 0xF0) == 0xE0)
            i += 3;
        else if ((ch & 0xF8) == 0xF0)
            i += 4;
        else
            i += 1;
        ++cchChars;
    }
    return cchChars;
}

EckNfInlineNd int LcsCountDbcsChar(_In_reads_or_z_(cch) PCCH psz, int cch, UINT uCp = CP_ACP)
{
    if (cch < 0)
        cch = (int)strlen(psz);
    int c = 0;
    for (auto p = psz; p < psz + cch; ++c)
    {
        if (IsDBCSLeadByteEx(uCp, *p))
            p += 2;
        else
            p += 1;
    }
    return c;
}

inline void LcsReverseUtf16ByteOrder(_Inout_updates_(cch) PWCH psz, int cch)
{
    const auto pEnd = psz + cch;
    for (; psz < pEnd; ++psz)
        *psz = _byteswap_ushort(*psz);
}
ECK_NAMESPACE_END