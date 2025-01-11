#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
EckInline BOOL DaAdjustWindowRectEx(_Inout_ RECT* prc, DWORD dwStyle, BOOL bMenu,
	DWORD dwExStyle, [[maybe_unused]] UINT uDpi)
{
#if ECKDPIAPI
	return AdjustWindowRectExForDpi(prc, dwStyle, bMenu, dwExStyle, uDpi);
#else
	return AdjustWindowRectEx(prc, dwStyle, bMenu, dwExStyle);
#endif
}

EckInline BOOL DaSystemParametersInfo(UINT uAction, UINT uParam, PVOID pParam,
	UINT fWinIni, [[maybe_unused]] UINT uDpi)
{
#if ECKDPIAPI
	return SystemParametersInfoForDpi(uAction, uParam, pParam, fWinIni, uDpi);
#else
	return SystemParametersInfoW(uAction, uParam, pParam, fWinIni);
#endif
}

EckInline int DaGetSystemMetrics(int nIndex, [[maybe_unused]] UINT uDpi)
{
#if ECKDPIAPI
	return GetSystemMetricsForDpi(nIndex, uDpi);
#else
	return GetSystemMetrics(nIndex);
#endif
}
ECK_NAMESPACE_END