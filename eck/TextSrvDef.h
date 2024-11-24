/*
* WinEzCtrlKit Library
*
* TextSrvDef.h ： 文本服务接口定义
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "ECK.h"

#include <Richedit.h>
#include <TextServ.h>

ECK_NAMESPACE_BEGIN
extern void* g_pfnCreateTextServices;
extern void* g_pfnShutdownTextServices;
extern IID* g_pIID_ITextHost;
extern IID* g_pIID_ITextHost2;
extern IID* g_pIID_ITextServices;
extern IID* g_pIID_ITextServices2;

namespace Priv
{
	inline void TsipInit(HMODULE hMod)
	{
		g_pfnCreateTextServices = GetProcAddress(hMod, "CreateTextServices");
		g_pfnShutdownTextServices = GetProcAddress(hMod, "ShutdownTextServices");
		g_pIID_ITextHost = (IID*)GetProcAddress(hMod, "IID_ITextHost");
		g_pIID_ITextHost2 = (IID*)GetProcAddress(hMod, "IID_ITextHost2");
		g_pIID_ITextServices = (IID*)GetProcAddress(hMod, "IID_ITextServices");
		g_pIID_ITextServices2 = (IID*)GetProcAddress(hMod, "IID_ITextServices2");
	}
}

inline BOOL TsiInit41()
{
	const auto hMod = LoadLibraryW(L"Msftedit.dll");
	if (!hMod)
		return FALSE;
	Priv::TsipInit(hMod);
	return TRUE;
}

inline BOOL TsiInit20()
{
	const auto hMod = LoadLibraryW(L"Riched20.dll");
	if (!hMod)
		return FALSE;
	Priv::TsipInit(hMod);
	return TRUE;
}

enum class TsiState
{
	NotInitialized,
	Ver20,
	Ver41
};

EckInline TsiState TsiGetState()
{
	if (!g_pfnCreateTextServices)
		return TsiState::NotInitialized;
	if (g_pIID_ITextHost2 && g_pIID_ITextServices2)
		return TsiState::Ver41;
	else
		return TsiState::Ver20;
}

EckInline HRESULT TsiCreateTextServices(IUnknown* punkOuter,
	ITextHost* pHost, IUnknown** ppSrv)
{
	return PCreateTextServices(g_pfnCreateTextServices)(punkOuter, pHost, ppSrv);
}

EckInline HRESULT TsiShutdownTextServices(IUnknown* pSrv)
{
	return PShutdownTextServices(g_pfnShutdownTextServices)(pSrv);
}
ECK_NAMESPACE_END