#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
EckInline BOOL DaAdjustWindowRectEx(_Inout_ RECT* prc, DWORD dwStyle, BOOL bMenu,
	DWORD dwExStyle, [[maybe_unused]] UINT uDpi)
{
#if ECK_OPT_DYN_NF
	if (g_pfnAdjustWindowRectExForDpi)
		return g_pfnAdjustWindowRectExForDpi(prc, dwStyle, bMenu, dwExStyle, uDpi);
	else
		return AdjustWindowRectEx(prc, dwStyle, bMenu, dwExStyle);
#else
#if ECKDPIAPI
	return AdjustWindowRectExForDpi(prc, dwStyle, bMenu, dwExStyle, uDpi);
#else
	return AdjustWindowRectEx(prc, dwStyle, bMenu, dwExStyle);
#endif// ECKDPIAPI
#endif// ECK_OPT_DYN_NF
}

EckInline BOOL DaSystemParametersInfo(UINT uAction, UINT uParam, PVOID pParam,
	UINT fWinIni, [[maybe_unused]] UINT uDpi)
{
#if ECK_OPT_DYN_NF
	if (g_pfnSystemParametersInfoForDpi)
		return g_pfnSystemParametersInfoForDpi(uAction, uParam, pParam, fWinIni, uDpi);
	else
		return SystemParametersInfoW(uAction, uParam, pParam, fWinIni);
#else
#if ECKDPIAPI
	return SystemParametersInfoForDpi(uAction, uParam, pParam, fWinIni, uDpi);
#else
	return SystemParametersInfoW(uAction, uParam, pParam, fWinIni);
#endif// ECKDPIAPI
#endif// ECK_OPT_DYN_NF
}

EckInline int DaGetSystemMetrics(int nIndex, [[maybe_unused]] UINT uDpi)
{
#if ECK_OPT_DYN_NF
	if (g_pfnGetSystemMetricsForDpi)
		return g_pfnGetSystemMetricsForDpi(nIndex, uDpi);
	else
		return GetSystemMetrics(nIndex);
#else
#if ECKDPIAPI
	return GetSystemMetricsForDpi(nIndex, uDpi);
#else
	return GetSystemMetrics(nIndex);
#endif// ECKDPIAPI
#endif// ECK_OPT_DYN_NF
}
ECK_NAMESPACE_END