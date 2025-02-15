#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
EckInlineNd int WcslCompareLen2(_In_reads_z_(Len1) PCWSTR Str1, int Len1,
	_In_reads_z_(Len2) PCWSTR Str2, int Len2,
	UINT uFlags, PCWSTR pszLocaleName = LOCALE_NAME_USER_DEFAULT)
{
	const int r = CompareStringEx(pszLocaleName, uFlags,
		Str1, Len1, Str2, Len2, nullptr, nullptr, 0);
#ifdef _DEBUG
	if (!r)
	{
		EckDbgPrint("CompareStringEx failed.");
		EckDbgBreak();
	}
#endif
	return r - 2;
}
EckInlineNd int WcslCompareLen(_In_reads_z_(Len) PCWSTR Str1,
	_In_reads_z_(Len) PCWSTR Str2, size_t Len,
	UINT uFlags, PCWSTR pszLocaleName = LOCALE_NAME_USER_DEFAULT)
{
	return WcslCompareLen2(Str1, Len, Str2, Len, uFlags, pszLocaleName);
}

EckInlineNd int WcslFindWorker(_In_reads_(Len) PCWSTR Str, int Len,
	_In_reads_(SubLen) PCWSTR SubStr, int SubLen,
	UINT uFlags, int* pcchMatch = nullptr, PCWSTR pszLocaleName = LOCALE_NAME_USER_DEFAULT)
{
	const auto r = FindNLSStringEx(pszLocaleName, uFlags,
		Str, Len, SubStr, SubLen, pcchMatch, nullptr, nullptr, 0);
#ifdef _DEBUG
	if (r == -1)
	{
		const auto dwErr = NtCurrentTeb()->LastErrorValue;
		EckDbgPrint("FindNLSStringEx failed. Error code: %d", dwErr);
		EckDbgPrintFormatMessage(dwErr);
		EckDbgBreak();
	}
#endif
	return r;
}

EckInlineNd int WcslStrLen(_In_reads_(Len) PCWSTR Str, int Len,
	_In_reads_(SubLen) PCWSTR SubStr, int SubLen,
	UINT uFlags = 0, int* pcchMatch = nullptr, PCWSTR pszLocaleName = LOCALE_NAME_USER_DEFAULT)
{
	return WcslFindWorker(Str, Len, SubStr, SubLen,
		uFlags | FIND_FROMSTART, pcchMatch, pszLocaleName);
}
EckInlineNd int WcslStrLenI(_In_reads_(Len) PCWSTR Str, int Len,
	_In_reads_(SubLen) PCWSTR SubStr, int SubLen,
	UINT uFlags = 0, int* pcchMatch = nullptr, PCWSTR pszLocaleName = LOCALE_NAME_USER_DEFAULT)
{
	return WcslFindWorker(Str, Len, SubStr, SubLen,
		uFlags | FIND_FROMSTART | NORM_IGNORECASE, pcchMatch, pszLocaleName);
}
EckInlineNd int WcslStrRLen(_In_reads_(Len) PCWSTR Str, int Len,
	_In_reads_(SubLen) PCWSTR SubStr, int SubLen,
	UINT uFlags = 0, int* pcchMatch = nullptr, PCWSTR pszLocaleName = LOCALE_NAME_USER_DEFAULT)
{
	return WcslFindWorker(Str, Len, SubStr, SubLen,
		uFlags | FIND_FROMEND, pcchMatch, pszLocaleName);
}
EckInlineNd int WcslStrRLenI(_In_reads_(Len) PCWSTR Str, int Len,
	_In_reads_(SubLen) PCWSTR SubStr, int SubLen,
	UINT uFlags = 0, int* pcchMatch = nullptr, PCWSTR pszLocaleName = LOCALE_NAME_USER_DEFAULT)
{
	return WcslFindWorker(Str, Len, SubStr, SubLen,
		uFlags | FIND_FROMEND | NORM_IGNORECASE, pcchMatch, pszLocaleName);
}
EckInlineNd int WcslIsStartWith(_In_reads_z_(Len) PCWSTR Str, int Len,
	_In_reads_z_(SubLen) PCWSTR SubStr, int SubLen,
	UINT uFlags = 0, int* pcchMatch = nullptr, PCWSTR pszLocaleName = LOCALE_NAME_USER_DEFAULT)
{
	return WcslFindWorker(Str, Len, SubStr, SubLen,
		uFlags | FIND_STARTSWITH, pcchMatch, pszLocaleName);
}
EckInlineNd int WcslIsStartWithI(_In_reads_z_(Len) PCWSTR Str, int Len,
	_In_reads_z_(SubLen) PCWSTR SubStr, int SubLen,
	UINT uFlags = 0, int* pcchMatch = nullptr, PCWSTR pszLocaleName = LOCALE_NAME_USER_DEFAULT)
{
	return WcslFindWorker(Str, Len, SubStr, SubLen,
		uFlags | FIND_STARTSWITH | NORM_IGNORECASE, pcchMatch, pszLocaleName);
}
EckInlineNd int WcslIsEndWith(_In_reads_z_(Len) PCWSTR Str, int Len,
	_In_reads_z_(SubLen) PCWSTR SubStr, int SubLen,
	UINT uFlags = 0, int* pcchMatch = nullptr, PCWSTR pszLocaleName = LOCALE_NAME_USER_DEFAULT)
{
	return WcslFindWorker(Str, Len, SubStr, SubLen,
		uFlags | FIND_ENDSWITH, pcchMatch, pszLocaleName);
}
EckInlineNd int WcslIsEndWithI(_In_reads_z_(Len) PCWSTR Str, int Len,
	_In_reads_z_(SubLen) PCWSTR SubStr, int SubLen,
	UINT uFlags = 0, int* pcchMatch = nullptr, PCWSTR pszLocaleName = LOCALE_NAME_USER_DEFAULT)
{
	return WcslFindWorker(Str, Len, SubStr, SubLen,
		uFlags | FIND_ENDSWITH | NORM_IGNORECASE, pcchMatch, pszLocaleName);
}
ECK_NAMESPACE_END