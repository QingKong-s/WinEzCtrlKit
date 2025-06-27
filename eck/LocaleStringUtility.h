#pragma once
#include "NativeWrapper.h"

ECK_NAMESPACE_BEGIN
EckInlineNd int WcslCompareLen2(_In_reads_(Len1) PCWCH Str1, int Len1,
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
EckInlineNd int WcslCompareLen(_In_reads_(Len) PCWCH Str1,
	_In_reads_(Len) PCWCH Str2, int Len,
	UINT uFlags, _In_opt_z_ PCWSTR pszLocaleName = LOCALE_NAME_USER_DEFAULT)
{
	return WcslCompareLen2(Str1, Len, Str2, Len, uFlags, pszLocaleName);
}

EckInlineNd int WcslFindWorker(_In_reads_(Len) PCWCH Str, int Len,
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

EckInlineNd int WcslStrLen(_In_reads_(Len) PCWCH Str, int Len,
	_In_reads_(SubLen) PCWCH SubStr, int SubLen,
	UINT uFlags = 0, int* pcchMatch = nullptr, _In_opt_z_ PCWSTR pszLocaleName = LOCALE_NAME_USER_DEFAULT)
{
	return WcslFindWorker(Str, Len, SubStr, SubLen,
		uFlags | FIND_FROMSTART, pcchMatch, pszLocaleName);
}
EckInlineNd int WcslStrLenI(_In_reads_(Len) PCWCH Str, int Len,
	_In_reads_(SubLen) PCWCH SubStr, int SubLen,
	UINT uFlags = 0, int* pcchMatch = nullptr, _In_opt_z_ PCWSTR pszLocaleName = LOCALE_NAME_USER_DEFAULT)
{
	return WcslFindWorker(Str, Len, SubStr, SubLen,
		uFlags | FIND_FROMSTART | NORM_IGNORECASE, pcchMatch, pszLocaleName);
}
EckInlineNd int WcslStrRLen(_In_reads_(Len) PCWCH Str, int Len,
	_In_reads_(SubLen) PCWCH SubStr, int SubLen,
	UINT uFlags = 0, int* pcchMatch = nullptr, _In_opt_z_ PCWSTR pszLocaleName = LOCALE_NAME_USER_DEFAULT)
{
	return WcslFindWorker(Str, Len, SubStr, SubLen,
		uFlags | FIND_FROMEND, pcchMatch, pszLocaleName);
}
EckInlineNd int WcslStrRLenI(_In_reads_(Len) PCWCH Str, int Len,
	_In_reads_(SubLen) PCWCH SubStr, int SubLen,
	UINT uFlags = 0, int* pcchMatch = nullptr, _In_opt_z_ PCWSTR pszLocaleName = LOCALE_NAME_USER_DEFAULT)
{
	return WcslFindWorker(Str, Len, SubStr, SubLen,
		uFlags | FIND_FROMEND | NORM_IGNORECASE, pcchMatch, pszLocaleName);
}
EckInlineNd int WcslIsStartWith(_In_reads_(Len) PCWCH Str, int Len,
	_In_reads_(SubLen) PCWCH SubStr, int SubLen,
	UINT uFlags = 0, int* pcchMatch = nullptr, _In_opt_z_ PCWSTR pszLocaleName = LOCALE_NAME_USER_DEFAULT)
{
	return WcslFindWorker(Str, Len, SubStr, SubLen,
		uFlags | FIND_STARTSWITH, pcchMatch, pszLocaleName);
}
EckInlineNd int WcslIsStartWithI(_In_reads_(Len) PCWCH Str, int Len,
	_In_reads_(SubLen) PCWCH SubStr, int SubLen,
	UINT uFlags = 0, int* pcchMatch = nullptr, _In_opt_z_ PCWSTR pszLocaleName = LOCALE_NAME_USER_DEFAULT)
{
	return WcslFindWorker(Str, Len, SubStr, SubLen,
		uFlags | FIND_STARTSWITH | NORM_IGNORECASE, pcchMatch, pszLocaleName);
}
EckInlineNd int WcslIsEndWith(_In_reads_(Len) PCWCH Str, int Len,
	_In_reads_(SubLen) PCWCH SubStr, int SubLen,
	UINT uFlags = 0, int* pcchMatch = nullptr, _In_opt_z_ PCWSTR pszLocaleName = LOCALE_NAME_USER_DEFAULT)
{
	return WcslFindWorker(Str, Len, SubStr, SubLen,
		uFlags | FIND_ENDSWITH, pcchMatch, pszLocaleName);
}
EckInlineNd int WcslIsEndWithI(_In_reads_(Len) PCWCH Str, int Len,
	_In_reads_(SubLen) PCWCH SubStr, int SubLen,
	UINT uFlags = 0, int* pcchMatch = nullptr, _In_opt_z_ PCWSTR pszLocaleName = LOCALE_NAME_USER_DEFAULT)
{
	return WcslFindWorker(Str, Len, SubStr, SubLen,
		uFlags | FIND_ENDSWITH | NORM_IGNORECASE, pcchMatch, pszLocaleName);
}
ECK_NAMESPACE_END