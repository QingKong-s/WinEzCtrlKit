#pragma once
#include "ECK.h"

#include <Richedit.h>
#include <TextServ.h>

ECK_NAMESPACE_BEGIN
extern void* g_pfnCreateTextServices;
extern void* g_pfnShutdownTextServices;
extern void* g_pfnCreateTextServices20;
extern void* g_pfnShutdownTextServices20;

constexpr inline IID
IID_ITextServices{ 0x8D33F740,0x0CF58,0x11CE,{ 0x0A8,0x9D,0,0x0AA,0,0x6C, 0x0AD,0x0C5 } },
IID_ITextServices2{ 0x8D33F741,0x0CF58,0x11CE,{ 0x0A8,0x9D,0,0x0AA,0,0x6C, 0x0AD,0x0C5 } },
IID_ITextHost{ 0x13e670f4,0x1a5a,0x11cf,{0xab,0xeb,0x00,0xaa,0x00,0xb6,0x5e,0xa1} },
IID_ITextHost2{ 0x13e670f5,0x1a5a,0x11cf,{0xab,0xeb,0x00,0xaa,0x00,0xb6,0x5e,0xa1} };


EckInline BOOL TsiInit()
{
	const auto hMod = LoadLibraryW(L"Msftedit.dll");
	if (!hMod)
		return FALSE;
	g_pfnCreateTextServices = GetProcAddress(hMod, "CreateTextServices");
	g_pfnShutdownTextServices = GetProcAddress(hMod, "ShutdownTextServices");
	return TRUE;
}

EckInlineNdCe BOOL TsiIsAvailable()
{
	return g_pfnCreateTextServices && g_pfnShutdownTextServices;
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

inline BOOL Tsi20Init()
{
	const auto hMod = LoadLibraryW(L"Riched20.dll");
	if (!hMod)
		return FALSE;
	g_pfnCreateTextServices20 = GetProcAddress(hMod, "CreateTextServices");
	g_pfnShutdownTextServices20 = GetProcAddress(hMod, "ShutdownTextServices");
	return TRUE;
}

EckInlineNdCe BOOL Tsi20IsAvailable()
{
	return g_pfnCreateTextServices20 && g_pfnShutdownTextServices20;
}

EckInline HRESULT Tsi20CreateTextServices(IUnknown* punkOuter,
	ITextHost* pHost, IUnknown** ppSrv)
{
	return PCreateTextServices(g_pfnCreateTextServices20)(punkOuter, pHost, ppSrv);
}

EckInline HRESULT Tsi20ShutdownTextServices(IUnknown* pSrv)
{
	return PShutdownTextServices(g_pfnShutdownTextServices20)(pSrv);
}
ECK_NAMESPACE_END