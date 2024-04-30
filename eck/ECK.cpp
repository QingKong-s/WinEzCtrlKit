/*
* WinEzCtrlKit Library
*
* ECK.cpp ： MAIN
*
* Copyright(C) 2024 QingKong
*/
#include "ECK.h"
#include "Utility.h"
#include "CRefStr.h"
#include "CWnd.h"
#include "GraphicsHelper.h"

#include <Shlwapi.h>
#include <d3d11.h>

ECK_NAMESPACE_BEGIN
CRefStrW g_rsCurrDir{};
ULONG_PTR g_uGpToken = 0u;

HINSTANCE g_hInstance = NULL;
IWICImagingFactory* g_pWicFactory = NULL;
ID2D1Factory1* g_pD2dFactory = NULL;
IDWriteFactory* g_pDwFactory = NULL;
ID2D1Device* g_pD2dDevice = NULL;
IDXGIDevice1* g_pDxgiDevice = NULL;
IDXGIFactory2* g_pDxgiFactory = NULL;

DWORD g_dwTlsSlot = 0;

ECK_PRIV_NAMESPACE_BEGIN
FAllowDarkModeForWindow			pfnAllowDarkModeForWindow{};
FAllowDarkModeForApp			pfnAllowDarkModeForApp{};
FIsDarkModeAllowedForWindow		pfnIsDarkModeAllowedForWindow{};
FShouldAppsUseDarkMode			pfnShouldAppsUseDarkMode{};
FFlushMenuThemes				pfnFlushMenuThemes{};
FRefreshImmersiveColorPolicyState		pfnRefreshImmersiveColorPolicyState{};
FGetIsImmersiveColorUsingHighContrast	pfnGetIsImmersiveColorUsingHighContrast{};

FShouldSystemUseDarkMode		pfnShouldSystemUseDarkMode{};
FSetPreferredAppMode			pfnSetPreferredAppMode{};
FIsDarkModeAllowedForApp		pfnIsDarkModeAllowedForApp{};

FRtlGetNtVersionNumbers			pfnRtlGetNtVersionNumbers{};
FSetWindowCompositionAttribute	pfnSetWindowCompositionAttribute{};
FOpenNcThemeData				pfnOpenNcThemeData{};
ECK_PRIV_NAMESPACE_END

BOOL g_bWin10_1607 = FALSE;
BOOL g_bWin10_1809 = FALSE;
BOOL g_bWin10_1903 = FALSE;
BOOL g_bWin11_B22000 = FALSE;

static BOOL DefMsgFilter(const MSG&)
{
	return FALSE;
}
FMsgFilter g_pfnMsgFilter = DefMsgFilter;

#ifdef _DEBUG
static void WINAPI GdiplusDebug(Gdiplus::DebugEventLevel dwLevel, CHAR* pszMsg)
{
	DbgPrint(StrX2W(pszMsg).Data());
	if (dwLevel == Gdiplus::DebugEventLevelFatal)
		DebugBreak();
}
#endif

#pragma region(UxTheme Fixer)
static EckPriv___::FOpenNcThemeData s_pfnOpenNcThemeData{};
static EckPriv___::FOpenThemeData s_pfnOpenThemeData{ OpenThemeData };
static EckPriv___::FDrawThemeText s_pfnDrawThemeText{ DrawThemeText };
static EckPriv___::FOpenThemeDataForDpi s_pfnOpenThemeDataForDpi{ OpenThemeDataForDpi };
static EckPriv___::FDrawThemeBackgroundEx s_pfnDrawThemeBackgroundEx{ DrawThemeBackgroundEx };
static EckPriv___::FDrawThemeBackground s_pfnDrawThemeBackground{ DrawThemeBackground };
static EckPriv___::FGetThemeColor s_pfnGetThemeColor{ GetThemeColor };
static EckPriv___::FCloseThemeData s_pfnCloseThemeData{ CloseThemeData };

static HTHEME WINAPI NewOpenNcThemeData(HWND hWnd, PCWSTR pszClassList)
{
	if (_wcsicmp(pszClassList, L"ScrollBar") == 0)
		return s_pfnOpenNcThemeData(NULL, L"Explorer::ScrollBar");// for nc scrollbar
	else ECKLIKELY
		return s_pfnOpenNcThemeData(hWnd, pszClassList);
}

static HTHEME WINAPI NewOpenThemeData(HWND hWnd, PCWSTR pszClassList)
{
	HTHEME hTheme;
	if (ShouldAppUseDarkMode())
	{
		if (_wcsicmp(pszClassList, L"Combobox") == 0)
			hTheme = s_pfnOpenThemeData(hWnd, L"DarkMode_CFD::Combobox");
		else if (_wcsicmp(pszClassList, L"Edit") == 0)
			hTheme = s_pfnOpenThemeData(hWnd, L"DarkMode_CFD::Edit");
		else if (_wcsicmp(pszClassList, L"TaskDialog") == 0 || _wcsicmp(pszClassList, L"TaskDialogStyle") == 0)
			hTheme = s_pfnOpenThemeData(hWnd, L"DarkMode_Explorer::TaskDialog");
		else
			hTheme = s_pfnOpenThemeData(hWnd, pszClassList);
	}
	else
		hTheme = s_pfnOpenThemeData(hWnd, pszClassList);
	GetThreadCtx()->OnThemeOpen(hTheme, pszClassList);
	return hTheme;
}

static HTHEME WINAPI NewOpenThemeDataForDpi(HWND hWnd, PCWSTR pszClassList, UINT uDpi)
{
	HTHEME hTheme;
	if (ShouldAppUseDarkMode())
	{
		if (_wcsicmp(pszClassList, L"Combobox") == 0)
			hTheme = s_pfnOpenThemeDataForDpi(hWnd, L"DarkMode_CFD::Combobox", uDpi);
		else if (_wcsicmp(pszClassList, L"Edit") == 0)
			hTheme = s_pfnOpenThemeDataForDpi(hWnd, L"DarkMode_CFD::Edit", uDpi);
		else if (_wcsicmp(pszClassList, L"TaskDialog") == 0 || _wcsicmp(pszClassList, L"TaskDialogStyle") == 0)
			hTheme = s_pfnOpenThemeDataForDpi(hWnd, L"DarkMode_Explorer::TaskDialog", uDpi);
		else
			hTheme = s_pfnOpenThemeDataForDpi(hWnd, pszClassList, uDpi);
	}
	else
		hTheme = s_pfnOpenThemeDataForDpi(hWnd, pszClassList, uDpi);
	GetThreadCtx()->OnThemeOpen(hTheme, pszClassList);
	return hTheme;
}

static HRESULT WINAPI NewDrawThemeText(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText,
	int cchText, DWORD dwTextFlags, DWORD dwTextFlags2, LPCRECT pRect)
{
	const auto* const ptc = GetThreadCtx();
	if (ShouldAppUseDarkMode() && ptc->IsThemeButton(hTheme))
		switch (iPartId)
		{
		case BP_CHECKBOX:
		case BP_GROUPBOX:
		case BP_RADIOBUTTON:
		{
			if (ShouldAppUseDarkMode())
			{
				DTTOPTS dtt;
				dtt.dwSize = sizeof(dtt);
				dtt.dwFlags = DTT_TEXTCOLOR;
				dtt.crText = ptc->crDefText;
				dwTextFlags &= ~DT_CALCRECT;
				return DrawThemeTextEx(hTheme, hdc, iPartId, iStateId,
					pszText, cchText, dwTextFlags, (RECT*)pRect, &dtt);
			}
		}
		break;
		}
	return s_pfnDrawThemeText(hTheme, hdc, iPartId, iStateId, pszText, cchText,
		dwTextFlags, dwTextFlags2, pRect);
}

static HRESULT DtbAdjustLuma(HTHEME hTheme, HDC hDC, int iPartId, int iStateId,
	const RECT* prc, const DTBGOPTS& Opt, float fPercent = 0.8f)
{
	CEzCDC DC{};
	RECT rcReal{ 0,0,prc->right - prc->left, prc->bottom - prc->top };
	DC.Create32(NULL, rcReal.right, rcReal.bottom);

	auto RealOpt{ Opt };
	RealOpt.rcClip = {};
	RealOpt.dwFlags &= ~DTBG_CLIPRECT;
	const auto hr = s_pfnDrawThemeBackgroundEx(hTheme, DC.GetDC(), iPartId, iStateId, &rcReal, &RealOpt);
	if (FAILED(hr))
		return hr;
	BITMAP bmp;
	GetObjectW(DC.GetBitmap(), sizeof(bmp), &bmp);
	GpBitmap* pBitmap;
	GdipCreateBitmapFromScan0(rcReal.right, rcReal.bottom, bmp.bmWidthBytes,
		PixelFormat32bppARGB, (BYTE*)bmp.bmBits, &pBitmap);
	GpImageAttributes* pIA;
	GdipCreateImageAttributes(&pIA);

	const Gdiplus::ColorMatrix mat
	{
		1, 0, 0, 0, 0,
		0, 1, 0, 0, 0,
		0, 0, 1, 0, 0,
		0, 0, 0, 1, 0,
		fPercent, fPercent, fPercent, 0, 1
	};
	GdipSetImageAttributesColorMatrix(pIA, Gdiplus::ColorAdjustTypeDefault, TRUE, &mat,
		NULL, Gdiplus::ColorMatrixFlagsDefault);
	GpGraphics* pGraphics;
	GdipCreateFromHDC(hDC, &pGraphics);
	if (Opt.dwFlags & DTBG_CLIPRECT)
	{
		GdipSetClipRectI(pGraphics, Opt.rcClip.left, Opt.rcClip.top,
			Opt.rcClip.right - Opt.rcClip.left, Opt.rcClip.bottom - Opt.rcClip.top,
			Gdiplus::CombineModeReplace);
	}
	GdipDrawImageRectRectI(pGraphics, pBitmap, prc->left, prc->top, rcReal.right, rcReal.bottom,
		0, 0, rcReal.right, rcReal.bottom, Gdiplus::UnitPixel, pIA, NULL, NULL);
	GdipDeleteGraphics(pGraphics);
	GdipDisposeImage(pBitmap);
	GdipDisposeImageAttributes(pIA);
	return S_OK;
}

EckInline static HRESULT DtbAdjustLuma(HTHEME hTheme, HDC hDC, int iPartId, int iStateId,
	const RECT* prc, const RECT* prcClip, float fPercent = 0.8f)
{
	DTBGOPTS opt;
	opt.dwSize = sizeof(DTBGOPTS);
	opt.dwFlags = 0u;
	if (prcClip)
	{
		opt.dwFlags |= DTBG_CLIPRECT;
		opt.rcClip = *prcClip;
	}
	return DtbAdjustLuma(hTheme, hDC, iPartId, iStateId, prc, opt, fPercent);
}

static HRESULT WINAPI NewDrawThemeBackgroundEx(HTHEME hTheme, HDC hdc, int iPartId, int iStateId,
	LPCRECT pRect, const DTBGOPTS* pOptions)
{
	if (ShouldAppUseDarkMode())
	{
		const auto* const ptc = GetThreadCtx();
		if (ptc->IsThemeTaskDialog(hTheme))
			switch (iPartId)
			{
			case TDLG_SECONDARYPANEL:
				iPartId = TDLG_FOOTNOTEPANE;
				break;
			case TDLG_FOOTNOTESEPARATOR:
				return DtbAdjustLuma(hTheme, hdc, iPartId, iStateId, pRect, *pOptions, -0.67f);
			}
		if (ptc->IsThemeButton(hTheme))
			switch (iPartId)
			{
			case BP_COMMANDLINKGLYPH:
				return DtbAdjustLuma(hTheme, hdc, iPartId, iStateId, pRect, *pOptions, 0.3f);
			case BP_COMMANDLINK:
				return DtbAdjustLuma(hTheme, hdc, iPartId, iStateId, pRect, *pOptions, 0.9f);
			}
	}
	return s_pfnDrawThemeBackgroundEx(hTheme, hdc, iPartId, iStateId, pRect, pOptions);
}

static HRESULT WINAPI NewDrawThemeBackground(HTHEME hTheme, HDC hdc, int iPartId, int iStateId,
	LPCRECT pRect, LPCRECT pClipRect)
{
	if (ShouldAppUseDarkMode())
	{
		const auto* const ptc = GetThreadCtx();
		if (ptc->IsThemeTaskDialog(hTheme))
			switch (iPartId)
			{
			case TDLG_SECONDARYPANEL:
				iPartId = TDLG_FOOTNOTEPANE;
				break;
			case TDLG_FOOTNOTESEPARATOR:
				return DtbAdjustLuma(hTheme, hdc, iPartId, iStateId, pRect, pClipRect, -0.67f);
			}
		if (ptc->IsThemeButton(hTheme))
			switch (iPartId)
			{
			case BP_COMMANDLINKGLYPH:
				return DtbAdjustLuma(hTheme, hdc, iPartId, iStateId, pRect, pClipRect, 0.3f);
			case BP_COMMANDLINK:
				return DtbAdjustLuma(hTheme, hdc, iPartId, iStateId, pRect, pClipRect, 0.9f);
			}
	}
	return s_pfnDrawThemeBackground(hTheme, hdc, iPartId, iStateId, pRect, pClipRect);
}

static HRESULT WINAPI NewGetThemeColor(HTHEME hTheme, int iPartId, int iStateId, int iPropId, COLORREF* pColor)
{
	const auto* const ptc = GetThreadCtx();
	if (ShouldAppUseDarkMode() && ptc->IsThemeTaskDialog(hTheme))
		switch (iPartId)
		{
		case TDLG_CONTENTPANE:
		case TDLG_EXPANDEDCONTENT:
		case TDLG_EXPANDOTEXT:
		case TDLG_VERIFICATIONTEXT:
		case TDLG_FOOTNOTEPANE:
		case TDLG_RADIOBUTTONPANE:
		{
			if (iPropId == TMT_TEXTCOLOR)
				*pColor = ptc->crDefText;
			else
				break;
		}
		return S_OK;

		case TDLG_MAININSTRUCTIONPANE:
		{
			const auto hr = s_pfnGetThemeColor(hTheme, iPartId, iStateId, iPropId, pColor);
			*pColor = AdjustColorrefLuma(*pColor, 300);
			return hr;
		}
		break;
		}
	return s_pfnGetThemeColor(hTheme, iPartId, iStateId, iPropId, pColor);
}

static HRESULT WINAPI NewCloseThemeData(HTHEME hTheme)
{
	GetThreadCtx()->OnThemeClose(hTheme);
	return s_pfnCloseThemeData(hTheme);
}
#pragma endregion

enum
{
	RWCT_EZREG,
	RWCT_CUSTOM,
};

struct EZREGWNDINFO
{
	PCWSTR pszClass = NULL;
	UINT uClassStyle = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
};

struct
{
	union
	{
		EZREGWNDINFO ez{};
		WNDCLASSW wc;
	};
	int iType = RWCT_EZREG;
}
g_WndClassInfo[]
{
	{ },
	{ WCN_LABEL },
	{ WCN_BK },
	{ WCN_SPLITBAR },
	{ WCN_DRAWPANEL },
	{ WCN_DRAWPANELD2D },
	{ WCN_LISTBOXNEW },
	{ WCN_ANIMATIONBOX },
	{ WCN_TREELIST },
	{ WCN_COMBOBOXNEW },
};

InitStatus Init(HINSTANCE hInstance, const INITPARAM* pInitParam, DWORD* pdwErrCode)
{
	EckAssert(!g_hInstance && !g_dwTlsSlot);
	DWORD dwTemp;
	if (!pdwErrCode)
		pdwErrCode = &dwTemp;
	INITPARAM ip{};
	if (!pInitParam)
		pInitParam = &ip;
	*pdwErrCode = 0;

	g_dwTlsSlot = TlsAlloc();
	g_hInstance = hInstance;

	if (!IsBitSet(pInitParam->uFlags, EIF_NOINITTHREAD))
		ThreadInit();

	if (!IsBitSet(pInitParam->uFlags, EIF_NOINITGDIPLUS))
	{
		GdiplusStartupInput gpsi{};
		gpsi.GdiplusVersion = 1;
#ifdef _DEBUG
		gpsi.DebugEventCallback = GdiplusDebug;
#endif
		Gdiplus::GpStatus uGpRet = GdiplusStartup(&g_uGpToken, &gpsi, NULL);
		if (uGpRet != Gdiplus::Ok)
		{
			*pdwErrCode = uGpRet;
			return InitStatus::GdiplusInitError;
		}
	}

	g_WndClassInfo[0].iType = RWCT_CUSTOM;
	g_WndClassInfo[0].wc = { CS_STDWND,DefDlgProcW,0,DLGWINDOWEXTRA + sizeof(void*) * 2,
	g_hInstance,NULL,LoadCursorW(NULL,IDC_ARROW),NULL,NULL,WCN_DLG };

	for (const auto& e : g_WndClassInfo)
	{
		switch (e.iType)
		{
		case RWCT_EZREG:
			if (!EzRegisterWndClass(e.ez.pszClass, e.ez.uClassStyle))
			{
				*pdwErrCode = GetLastError();
				EckDbgPrintFormatMessage(*pdwErrCode);
				EckDbgBreak();
				return InitStatus::RegWndClassError;
			}
			break;
		case RWCT_CUSTOM:
			if (!RegisterClassW(&e.wc))
			{
				*pdwErrCode = GetLastError();
				EckDbgPrintFormatMessage(*pdwErrCode);
				EckDbgBreak();
				return InitStatus::RegWndClassError;
			}
			break;
		default:
			ECK_UNREACHABLE;
		}
	}

	g_rsCurrDir.ReSize(32768);
	GetModuleFileNameW(NULL, g_rsCurrDir.Data(), g_rsCurrDir.Size());
	PathRemoveFileSpecW(g_rsCurrDir.Data());
	g_rsCurrDir.ReCalcLen();
	g_rsCurrDir.ShrinkToFit();

	HRESULT hr;
	if (!IsBitSet(pInitParam->uFlags, EIF_NOINITD2D))
	{
#ifndef NDEBUG
		D2D1_FACTORY_OPTIONS D2DFactoryOptions;
		D2DFactoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
		hr = D2D1CreateFactory(pInitParam->uD2dFactoryType, __uuidof(ID2D1Factory1), &D2DFactoryOptions, (void**)&g_pD2dFactory);
#else
		hr = D2D1CreateFactory(pInitParam->uD2dFactoryType, IID_PPV_ARGS(&g_pD2dFactory));
#endif // !NDEBUG
		if (FAILED(hr))
		{
			*pdwErrCode = hr;
			EckDbgPrintFormatMessage(hr);
			return InitStatus::D2dFactoryError;
		}
		//////////////创建DWrite工厂
		hr = DWriteCreateFactory(pInitParam->uDWriteFactoryType, __uuidof(IDWriteFactory), (IUnknown**)&g_pDwFactory);
		if (FAILED(hr))
		{
			*pdwErrCode = hr;
			EckDbgPrintFormatMessage(hr);
			return InitStatus::DWriteFactoryError;
		}
		//////////////创建DXGI工厂
		ID3D11Device* pD3DDevice;
		hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_BGRA_SUPPORT
#ifndef NDEBUG
			| D3D11_CREATE_DEVICE_DEBUG
#endif // !NDEBUG
			, pInitParam->pD3dFeatureLevel, pInitParam->cD3dFeatureLevel,
			D3D11_SDK_VERSION, &pD3DDevice, NULL, NULL);
		if (FAILED(hr))
		{
			*pdwErrCode = hr;
			EckDbgPrintFormatMessage(hr);
			return InitStatus::D3dDeviceError;
		}
		pD3DDevice->QueryInterface(IID_PPV_ARGS(&g_pDxgiDevice));
		pD3DDevice->Release();

		IDXGIAdapter* pDXGIAdapter;
		g_pDxgiDevice->GetAdapter(&pDXGIAdapter);
		pDXGIAdapter->GetParent(IID_PPV_ARGS(&g_pDxgiFactory));
		pDXGIAdapter->Release();
		//////////////创建DXGI设备
		hr = g_pD2dFactory->CreateDevice(g_pDxgiDevice, &g_pD2dDevice);
		if (FAILED(hr))
		{
			*pdwErrCode = hr;
			EckDbgPrintFormatMessage(hr);
			return InitStatus::DxgiDeviceError;
		}
	}
	//////////////创建WIC工厂
	if (!IsBitSet(pInitParam->uFlags, EIF_NOINITWIC))
	{
		hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&g_pWicFactory));
		if (FAILED(hr))
		{
			*pdwErrCode = hr;
			EckDbgPrintFormatMessage(hr);
			return InitStatus::WicFactoryError;
		}
	}

	using namespace EckPriv___;
	//////////////暗色模式支持、OS版本检测
	const HMODULE hModUx = LoadLibraryW(L"UxTheme.dll");
	*pdwErrCode = GetLastError();
	if (!hModUx)
		return InitStatus::UxThemeError;

	const HMODULE hModNtdll = LoadLibraryW(L"ntdll.dll");
	EckAssert(hModNtdll);
	pfnRtlGetNtVersionNumbers = (FRtlGetNtVersionNumbers)
		GetProcAddress(hModNtdll, "RtlGetNtVersionNumbers");
	FreeLibrary(hModNtdll);

	DWORD dwMajorVer, dwMinorVer, dwBuildNumber;
	RtlGetNtVersionNumbers(&dwMajorVer, &dwMinorVer, &dwBuildNumber);
	if (dwMajorVer >= 10 && dwBuildNumber >= 14393)
	{
		g_bWin10_1607 = TRUE;
		if (dwBuildNumber >= 17763)
		{
			g_bWin10_1809 = TRUE;
			if (dwBuildNumber > 18362)
			{
				g_bWin10_1903 = TRUE;
				if (dwBuildNumber > 22000)
					g_bWin11_B22000 = TRUE;
			}

			if (g_bWin10_1903)
			{
				pfnShouldSystemUseDarkMode = (FShouldSystemUseDarkMode)
					GetProcAddress(hModUx, MAKEINTRESOURCEA(138));
				pfnIsDarkModeAllowedForApp = (FIsDarkModeAllowedForApp)
					GetProcAddress(hModUx, MAKEINTRESOURCEA(139));
				pfnSetPreferredAppMode = (FSetPreferredAppMode)
					GetProcAddress(hModUx, MAKEINTRESOURCEA(135));
			}
			else
			{
				pfnAllowDarkModeForApp = (FAllowDarkModeForApp)
					GetProcAddress(hModUx, MAKEINTRESOURCEA(135));
			}
			pfnAllowDarkModeForWindow = (FAllowDarkModeForWindow)
				GetProcAddress(hModUx, MAKEINTRESOURCEA(133));
			pfnIsDarkModeAllowedForWindow = (FIsDarkModeAllowedForWindow)
				GetProcAddress(hModUx, MAKEINTRESOURCEA(137));
			pfnShouldAppsUseDarkMode = (FShouldAppsUseDarkMode)
				GetProcAddress(hModUx, MAKEINTRESOURCEA(132));
			pfnFlushMenuThemes = (FFlushMenuThemes)
				GetProcAddress(hModUx, MAKEINTRESOURCEA(136));
			pfnRefreshImmersiveColorPolicyState = (FRefreshImmersiveColorPolicyState)
				GetProcAddress(hModUx, MAKEINTRESOURCEA(104));
			pfnGetIsImmersiveColorUsingHighContrast = (FGetIsImmersiveColorUsingHighContrast)
				GetProcAddress(hModUx, MAKEINTRESOURCEA(106));
			pfnOpenNcThemeData = (FOpenNcThemeData)
				GetProcAddress(hModUx, MAKEINTRESOURCEA(49));
			s_pfnOpenNcThemeData = pfnOpenNcThemeData;
		}
	}
	FreeLibrary(hModUx);

	const auto hModUser32 = LoadLibraryW(L"User32.dll");
	EckAssert(hModUser32);
	pfnSetWindowCompositionAttribute = (FSetWindowCompositionAttribute)
		GetProcAddress(hModUser32, "SetWindowCompositionAttribute");
	FreeLibrary(hModUser32);

	if (!IsBitSet(pInitParam->uFlags, EIF_NODARKMODE))
	{
		SetPreferredAppMode(PreferredAppMode::AllowDark);
		RefreshImmersiveColorStuff();
		FlushMenuTheme();
	}

	DetourTransactionBegin();
	if (s_pfnOpenNcThemeData)
		DetourAttach(&s_pfnOpenNcThemeData, NewOpenNcThemeData);
	s_pfnOpenThemeData = OpenThemeData;
	DetourAttach(&s_pfnOpenThemeData, NewOpenThemeData);
	DetourAttach(&s_pfnDrawThemeText, NewDrawThemeText);
	DetourAttach(&s_pfnOpenThemeDataForDpi, NewOpenThemeDataForDpi);
	DetourAttach(&s_pfnDrawThemeBackgroundEx, NewDrawThemeBackgroundEx);
	DetourAttach(&s_pfnDrawThemeBackground, NewDrawThemeBackground);
	DetourAttach(&s_pfnGetThemeColor, NewGetThemeColor);
	DetourAttach(&s_pfnCloseThemeData, NewCloseThemeData);
	DetourTransactionCommit();
	return InitStatus::Ok;
}

void UnInit()
{
	TlsFree(g_dwTlsSlot);
	g_dwTlsSlot = 0;
	if (g_uGpToken)
		GdiplusShutdown(g_uGpToken);
	g_uGpToken = 0u;
	g_hInstance = NULL;
	g_rsCurrDir.Clear();
	SafeRelease(g_pWicFactory);
	SafeRelease(g_pD2dFactory);
	SafeRelease(g_pDwFactory);
	SafeRelease(g_pD2dDevice);
	SafeRelease(g_pDxgiDevice);
	SafeRelease(g_pDxgiFactory);
}

DWORD GetThreadCtxTlsSlot()
{
	return g_dwTlsSlot;
}

void ThreadInit()
{
	EckAssert(!TlsGetValue(GetThreadCtxTlsSlot()));
	const auto p = new ECKTHREADCTX{};
	TlsSetValue(GetThreadCtxTlsSlot(), p);
	p->UpdateDefColor();
	p->hhkCbtDarkMode = SetWindowsHookExW(WH_CBT, [](int iCode, WPARAM wParam, LPARAM lParam)->LRESULT
		{
			constexpr static PCWSTR c_pszCommCtrlCls[]// 需要被设置主题的通用控件
			{
				WC_BUTTONW,
				WC_HEADERW,
				WC_LISTVIEWW,
				TOOLBARCLASSNAMEW,
				TOOLTIPS_CLASSW,
				WC_TREEVIEWW,
				UPDOWN_CLASSW,
				WC_SCROLLBARW
			};

			const auto* const p = GetThreadCtx();
			if (iCode == HCBT_CREATEWND && p->bEnableDarkModeHook)
			{
				const auto hWnd = (HWND)wParam;
				AllowDarkModeForWindow(hWnd, TRUE);

				const auto lResult = CallNextHookEx(p->hhkCbtDarkMode, iCode, wParam, lParam);
				if (IsWindow(hWnd))
				{
					const auto itEnd = c_pszCommCtrlCls + ARRAYSIZE(c_pszCommCtrlCls);
					const auto it = std::find_if(c_pszCommCtrlCls, itEnd,
						[lParam, hWnd](PCWSTR psz)
						{
							auto p = (CBT_CREATEWNDW*)lParam;
							if (IsPszId(p->lpcs->lpszClass))
							{
								WCHAR szCls[256];
								GetClassNameW(hWnd, szCls, ARRAYSIZE(szCls));
								return _wcsicmp(szCls, psz) == 0;
							}
							else
								return _wcsicmp(p->lpcs->lpszClass, psz) == 0;
						});

					if (it != itEnd)
					{
						if (it == c_pszCommCtrlCls + 1 || it == c_pszCommCtrlCls + 2)// HEADER/LISTVIEW
							SetWindowTheme(hWnd, L"ItemsView", NULL);
						else ECKLIKELY
							SetWindowTheme(hWnd, L"Explorer", NULL);
					}
				}
				return lResult;
			}

			return CallNextHookEx(p->hhkCbtDarkMode, iCode, wParam, lParam);
		}, NULL, GetCurrentThreadId());
}

constexpr PCWSTR c_szErrInitStatus[]
{
	L"操作成功完成。",
	L"注册窗口类失败",
	L"GDI+初始化失败",
	L"WIC初始化失败",
	L"D2D初始化失败",
	L"DXGI设备创建失败",
	L"DWrite初始化失败",
	L"D3D设备创建失败",
};

PCWSTR InitStatusToString(InitStatus iStatus)
{
	return c_szErrInitStatus[(int)iStatus];
}

void ThreadUnInit()
{
	EckAssert(TlsGetValue(GetThreadCtxTlsSlot()));
	const auto p = (ECKTHREADCTX*)TlsGetValue(GetThreadCtxTlsSlot());
#ifdef _DEBUG
	if (!p->hmWnd.empty())
	{
		EckDbgPrintWithPos(L"窗口映射表不为空");
		EckDbgBreak();
	}
#endif // _DEBUG
	UnhookWindowsHookEx(p->hhkTempCBT);
	UnhookWindowsHookEx(p->hhkCbtDarkMode);
	delete p;
	TlsSetValue(GetThreadCtxTlsSlot(), NULL);
}

HHOOK BeginCbtHook(CWnd* pCurrWnd, FWndCreating pfnCreatingProc)
{
	EckAssert(pCurrWnd);
	const auto pCtx = GetThreadCtx();
	pCtx->pCurrWnd = pCurrWnd;
	pCtx->pfnWndCreatingProc = pfnCreatingProc;
	EckAssert(!pCtx->hhkTempCBT);
	HHOOK hHook = SetWindowsHookExW(WH_CBT, [](int iCode, WPARAM wParam, LPARAM lParam)->LRESULT
		{
			const auto pCtx = GetThreadCtx();
			if (iCode == HCBT_CREATEWND)
			{
				const auto pcbtcw = (CBT_CREATEWNDW*)lParam;
				pCtx->pCurrWnd->m_pfnRealProc =
					(WNDPROC)SetWindowLongPtrW((HWND)wParam, GWLP_WNDPROC,
						(LONG_PTR)CWnd::EckWndProc);
				EckAssert(!pCtx->pCurrWnd->m_hWnd);
				pCtx->pCurrWnd->m_hWnd = (HWND)wParam;
				pCtx->WmAdd((HWND)wParam, pCtx->pCurrWnd);
				if (!IsBitSet(pcbtcw->lpcs->style, WS_CHILD))
					pCtx->TwmAdd((HWND)wParam, pCtx->pCurrWnd);
				if (pCtx->pfnWndCreatingProc)
					pCtx->pfnWndCreatingProc((HWND)wParam, pcbtcw, pCtx);
				EndCbtHook();
			}
			return CallNextHookEx(pCtx->hhkTempCBT, iCode, wParam, lParam);
		}, NULL, GetCurrentThreadId());
	pCtx->hhkTempCBT = hHook;
	return pCtx->hhkTempCBT;
}

void EndCbtHook()
{
	const auto pCtx = GetThreadCtx();
	EckAssert(pCtx->hhkTempCBT);
	UnhookWindowsHookEx(pCtx->hhkTempCBT);
	pCtx->hhkTempCBT = NULL;
}

BOOL PreTranslateMessage(const MSG& Msg)
{
	if (g_pfnMsgFilter(Msg))
		return TRUE;
	HWND hWnd = Msg.hwnd;
	CWnd* pWnd;
	const ECKTHREADCTX* const pCtx = GetThreadCtx();
	while (hWnd)
	{
		pWnd = pCtx->WmAt(hWnd);
		if (pWnd && pWnd->PreTranslateMessage(Msg))
			return TRUE;
		hWnd = GetParent(hWnd);
	}
	return FALSE;
}

void SetMsgFilter(FMsgFilter pfnFilter)
{
	if (!pfnFilter)
		g_pfnMsgFilter = DefMsgFilter;
	else
		g_pfnMsgFilter = pfnFilter;
}

void DbgPrintWndMap()
{
	auto pCtx = GetThreadCtx();
	auto s = Format(L"当前线程（TID = %u）窗口映射表内容：\n", GetCurrentThreadId());
	CRefStrW rs{};
	for (const auto& e : pCtx->hmWnd)
	{
		auto rsText = e.second->GetText();
		if (!rsText.Data())
			rsText = L" ";
		auto rsCls = e.second->GetClsName();
		if (!rsCls.Data())
			rsCls = L" ";
		rs.Format(L"\tCWnd指针 = 0x%0p，HWND = 0x%0p，标题 = %s，类名 = %s\n",
			e.second,
			e.first,
			rsText.Data(),
			rsCls.Data());
		s += rs.Data();
	}
	s += Format(L"共有%u个窗口\n", pCtx->hmWnd.size());
	OutputDebugStringW(s.Data());
}

void DbgPrintFmt(_Printf_format_string_ PCWSTR pszFormat, ...)
{
	va_list vl;
	va_start(vl, pszFormat);
	CRefStrW rs{};
	rs.FormatV(pszFormat, vl);
	DbgPrint(rs);
	va_end(vl);
}

void DbgPrintWithPos(PCWSTR pszFile, PCWSTR pszFunc, int iLine, PCWSTR pszMsg)
{
	DbgPrint(Format(L"%s(%u) -> %s\n%s\n", pszFile, iLine, pszFunc, pszMsg));
}

const CRefStrW& GetRunningPath()
{
	return g_rsCurrDir;
}

void Assert(PCWSTR pszMsg, PCWSTR pszFile, PCWSTR pszLine)
{
	TASKDIALOGCONFIG tdc{ sizeof(TASKDIALOGCONFIG) };
	tdc.pszMainInstruction = L"断言失败！";
	tdc.pszMainIcon = TD_ERROR_ICON;

	constexpr TASKDIALOG_BUTTON Btns[]
	{
		{ 100,L"终止程序" },
		{ 101,L"调试程序" },
		{ 102,L"继续运行" },
	};
	tdc.pButtons = Btns;
	tdc.cButtons = ARRAYSIZE(Btns);
	tdc.nDefaultButton = 101;
	WCHAR szPath[MAX_PATH]{};
	GetModuleFileNameW(NULL, szPath, MAX_PATH);
	const auto rsContent = Format(L"程序位置：%s\n\n源文件：%s\n\n行号：%s\n\n测试表达式：%s",
		szPath, pszFile, pszLine, pszMsg);
	tdc.pszContent = rsContent.Data();

	tdc.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_USE_COMMAND_LINKS;

	BOOL t;
	int iBtn;
	TaskDialogIndirect(&tdc, &iBtn, &t, &t);
	switch (iBtn)
	{
	case 100:
		ExitProcess(0);
		return;
	case 101:
		DebugBreak();
		return;
	case 102:
		return;
	}
}

void ECKTHREADCTX::SetNcDarkModeForAllTopWnd(BOOL bDark)
{
	for (const auto& e : hmTopWnd)
		EnableWindowNcDarkMode(e.first, bDark);
}

void ECKTHREADCTX::UpdateDefColor()
{
	GetItemsViewForeBackColor(crDefText, crDefBkg);
	crDefBtnFace = (ShouldAppUseDarkMode() ? 0x303030 : GetSysColor(COLOR_BTNFACE));
}
ECK_NAMESPACE_END