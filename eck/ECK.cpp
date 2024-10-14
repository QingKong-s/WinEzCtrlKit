/*
* WinEzCtrlKit Library
*
* ECK.cpp ： MAIN
*
* Copyright(C) 2023-2024 QingKong
*/
#ifdef ECK_MACRO_MYDBG
#include "PchInc.h"
#endif// ECK_MACRO_MYDBG

// For Private API

FSetWindowCompositionAttribute	pfnSetWindowCompositionAttribute{};

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

FOpenNcThemeData				pfnOpenNcThemeData{};

ECK_NAMESPACE_BEGIN
CRefStrW g_rsCurrDir{};
ULONG_PTR g_uGpToken{};

HINSTANCE g_hInstance{};
IWICImagingFactory* g_pWicFactory{};
ID2D1Factory1* g_pD2dFactory{};
IDWriteFactory* g_pDwFactory{};
ID2D1Device* g_pD2dDevice{};
IDXGIDevice1* g_pDxgiDevice{};
IDXGIFactory2* g_pDxgiFactory{};

DWORD g_dwTlsSlot{};

NTVER g_NtVer{};

ClassInfo* g_pClassInfo{};


void EckInitPrivateApi()
{
	const auto hModUser32 = LoadLibraryW(L"User32.dll");
	EckAssert(hModUser32);
	pfnSetWindowCompositionAttribute = (FSetWindowCompositionAttribute)
		GetProcAddress(hModUser32, "SetWindowCompositionAttribute");
	FreeLibrary(hModUser32);

	const HMODULE hModUx = LoadLibraryW(L"UxTheme.dll");
	EckAssert(hModUx);
	pfnOpenNcThemeData = (FOpenNcThemeData)
		GetProcAddress(hModUx, MAKEINTRESOURCEA(49));

	if (g_NtVer.uMajor >= 10 && g_NtVer.uBuild >= WINVER_1607)
	{
		if (g_NtVer.uBuild >= WINVER_1809)
		{
			if (g_NtVer.uBuild > WINVER_1903)
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
		}
	}
	FreeLibrary(hModUx);
}

static BOOL DefMsgFilter(const MSG&)
{
	return FALSE;
}
FMsgFilter g_pfnMsgFilter = DefMsgFilter;

#pragma region(UxTheme DarkMode Fixer)
enum
{
	//===部件===
	// Link
	TP_LINK_HYPERLINK = 1,// 超链接
	//===状态===
	// Link
	TS_LINK_HYPERLINK_COMMON = 0,
	TS_LINK_HYPERLINK_NORMAL = 1,// invalid
	TS_LINK_HYPERLINK_LINK = 2,
};

using FOpenThemeData = HTHEME(WINAPI*)(HWND, LPCWSTR);
using FDrawThemeText = HRESULT(WINAPI*)(HTHEME, HDC, int, int, LPCWSTR, int, DWORD, DWORD, LPCRECT);
using FDrawThemeTextEx = HRESULT(WINAPI*)(HTHEME, HDC, int, int, LPCWSTR, int, DWORD, LPRECT, const DTTOPTS*);
using FOpenThemeDataForDpi = HTHEME(WINAPI*)(HWND, LPCWSTR, UINT);
using FDrawThemeBackgroundEx = HRESULT(WINAPI*)(HTHEME, HDC, int, int, LPCRECT, const DTBGOPTS*);
using FDrawThemeBackground = HRESULT(WINAPI*)(HTHEME, HDC, int, int, LPCRECT, LPCRECT);
using FGetThemeColor = HRESULT(WINAPI*)(HTHEME, int, int, int, COLORREF*);
using FCloseThemeData = HRESULT(WINAPI*)(HTHEME);
using FDrawThemeParentBackground = HRESULT(WINAPI*)(HWND, HDC, const RECT*);

static FOpenNcThemeData		s_pfnOpenNcThemeData{};
static FOpenThemeData		s_pfnOpenThemeData{ OpenThemeData };
static FDrawThemeText		s_pfnDrawThemeText{ DrawThemeText };
static FDrawThemeTextEx		s_pfnDrawThemeTextEx{ DrawThemeTextEx };
static FOpenThemeDataForDpi	s_pfnOpenThemeDataForDpi{};// DPI API引入较晚，动态加载之
static FDrawThemeBackgroundEx		s_pfnDrawThemeBackgroundEx{ DrawThemeBackgroundEx };
static FDrawThemeBackground	s_pfnDrawThemeBackground{ DrawThemeBackground };
static FGetThemeColor		s_pfnGetThemeColor{ GetThemeColor };
static FCloseThemeData		s_pfnCloseThemeData{ CloseThemeData };
static FDrawThemeParentBackground	s_pfnDrawThemeParentBackground{ DrawThemeParentBackground };

//
std::unordered_map<HTHEME, int> g_hsButtonTheme{};
CSrwLock g_LkThemeMapButton{};
// +TaskDialogStyle
std::unordered_map<HTHEME, int> g_hsTaskDialogTheme{};
CSrwLock g_LkThemeMapTaskDialog{};
//
std::unordered_map<HTHEME, int> g_hsTabTheme{};
CSrwLock g_LkThemeMapTab{};
// 附带一个ItemsView::ListView主题，用于绘制背景
std::unordered_map<HTHEME, std::pair<int, HTHEME>> g_hsToolBarTheme{};
CSrwLock g_LkThemeMapToolBar{};
// +AeroWizardStyle
std::unordered_map<HTHEME, int> g_hsAeroWizardTheme{};
CSrwLock g_LkThemeMapAeroWizard{};
// 附带一个DarkMode_CFD::ComboBox主题，用于绘制背景和下拉按钮
std::unordered_map<HTHEME, std::pair<int, HTHEME>> g_hsDateTimePickerTheme{};
CSrwLock g_LkThemeMapDateTimePicker{};
// +ItemsView
std::unordered_map<HTHEME, int> g_hsListViewTheme{};
CSrwLock g_LkThemeMapListView{};
//
std::unordered_map<HTHEME, int> g_hsLink{};
CSrwLock g_LkThemeMapLink{};

static BOOL IsThemeButton(HTHEME hTheme)
{
	CSrwReadGuard _{ g_LkThemeMapButton };
	return g_hsButtonTheme.find(hTheme) != g_hsButtonTheme.end();
}

static BOOL IsThemeTaskDialog(HTHEME hTheme)
{
	CSrwReadGuard _{ g_LkThemeMapTaskDialog };
	return g_hsTaskDialogTheme.find(hTheme) != g_hsTaskDialogTheme.end();
}

static BOOL IsThemeTab(HTHEME hTheme)
{
	CSrwReadGuard _{ g_LkThemeMapTab };
	return g_hsTabTheme.find(hTheme) != g_hsTabTheme.end();
}

static BOOL IsThemeToolBar(HTHEME hTheme, HTHEME* phThemeLV = nullptr)
{
	CSrwReadGuard _{ g_LkThemeMapToolBar };
	if (phThemeLV)
	{
		const auto it = g_hsToolBarTheme.find(hTheme);
		if (it != g_hsToolBarTheme.end())
		{
			*phThemeLV = it->second.second;
			return TRUE;
		}
		else
			return FALSE;
	}
	else
		return g_hsToolBarTheme.find(hTheme) != g_hsToolBarTheme.end();
}

static BOOL IsThemeAeroWizard(HTHEME hTheme)
{
	CSrwReadGuard _{ g_LkThemeMapAeroWizard };
	return g_hsAeroWizardTheme.find(hTheme) != g_hsAeroWizardTheme.end();
}

static BOOL IsThemeDateTimePicker(HTHEME hTheme, HTHEME* phThemeCBB = nullptr)
{
	CSrwReadGuard _{ g_LkThemeMapDateTimePicker };
	if (phThemeCBB)
	{
		const auto it = g_hsDateTimePickerTheme.find(hTheme);
		if (it != g_hsDateTimePickerTheme.end())
		{
			*phThemeCBB = it->second.second;
			return TRUE;
		}
		else
			return FALSE;
	}
	else
		return g_hsDateTimePickerTheme.find(hTheme) != g_hsDateTimePickerTheme.end();
}

static BOOL IsThemeListView(HTHEME hTheme)
{
	CSrwReadGuard _{ g_LkThemeMapListView };
	return g_hsListViewTheme.find(hTheme) != g_hsListViewTheme.end();
}

static BOOL IsThemeLink(HTHEME hTheme)
{
	CSrwReadGuard _{ g_LkThemeMapLink };
	return g_hsLink.find(hTheme) != g_hsLink.end();
}

static void OnThemeOpen(HTHEME hTheme, PCWSTR pszClassList)
{
	if (!hTheme)
		return;
	if (wcsstr(pszClassList, L"Button"))// maybe ~";Ok"
	{
		CSrwWriteGuard _{ g_LkThemeMapButton };
		++g_hsButtonTheme[hTheme];
	}
	else if (_wcsicmp(pszClassList, L"TaskDialog") == 0 ||
		_wcsicmp(pszClassList, L"TaskDialogStyle") == 0)
	{
		CSrwWriteGuard _{ g_LkThemeMapTaskDialog };
		++g_hsTaskDialogTheme[hTheme];
	}
	else if (_wcsicmp(pszClassList, L"Tab") == 0)
	{
		CSrwWriteGuard _{ g_LkThemeMapTab };
		++g_hsTabTheme[hTheme];
	}
	else if (_wcsicmp(pszClassList, L"ToolBar") == 0)
	{
		CSrwWriteGuard _{ g_LkThemeMapToolBar };
		auto& e = g_hsToolBarTheme[hTheme];
		if (!e.first && ShouldAppsUseDarkMode())
			e.second = s_pfnOpenThemeData(nullptr, L"ItemsView::ListView");
		++e.first;
	}
	else if (_wcsicmp(pszClassList, L"AeroWizard") == 0 ||
		_wcsicmp(pszClassList, L"AeroWizardStyle") == 0)
	{
		CSrwWriteGuard _{ g_LkThemeMapAeroWizard };
		++g_hsAeroWizardTheme[hTheme];
	}
	else if (_wcsicmp(pszClassList, L"DatePicker") == 0)
	{
		CSrwWriteGuard _{ g_LkThemeMapDateTimePicker };
		auto& e = g_hsDateTimePickerTheme[hTheme];
		if (!e.first && ShouldAppsUseDarkMode())
			e.second = s_pfnOpenThemeData(nullptr, L"DarkMode_CFD::ComboBox");
		++e.first;
	}
	else if (wcsstr(pszClassList, L"ListView") || wcscmp(pszClassList, L"ItemsView") == 0)
	{
		CSrwWriteGuard _{ g_LkThemeMapListView };
		++g_hsListViewTheme[hTheme];
	}
	else if (wcscmp(pszClassList, L"Link") == 0)
	{
		CSrwWriteGuard _{ g_LkThemeMapLink };
		++g_hsLink[hTheme];
	}
}

static void OnThemeClose(HTHEME hTheme)
{
	{
		CSrwWriteGuard _{ g_LkThemeMapButton };
		const auto it = g_hsButtonTheme.find(hTheme);
		if (it != g_hsButtonTheme.end())
		{
			EckAssert(it->second > 0);
			if (!--it->second)
				g_hsButtonTheme.erase(it);
		}
	}
	{
		CSrwWriteGuard _{ g_LkThemeMapTaskDialog };
		const auto it = g_hsTaskDialogTheme.find(hTheme);
		if (it != g_hsTaskDialogTheme.end())
		{
			EckAssert(it->second > 0);
			if (!--it->second)
				g_hsTaskDialogTheme.erase(it);
		}
	}
	{
		CSrwWriteGuard _{ g_LkThemeMapTab };
		const auto it = g_hsTabTheme.find(hTheme);
		if (it != g_hsTabTheme.end())
		{
			EckAssert(it->second > 0);
			if (!--it->second)
				g_hsTabTheme.erase(it);
		}
	}
	{
		CSrwWriteGuard _{ g_LkThemeMapToolBar };
		const auto it = g_hsToolBarTheme.find(hTheme);
		if (it != g_hsToolBarTheme.end())
		{
			EckAssert(it->second.first > 0);
			if (!--it->second.first)
			{
				s_pfnCloseThemeData(it->second.second);
				g_hsToolBarTheme.erase(it);
			}
		}
	}
	{
		CSrwWriteGuard _{ g_LkThemeMapAeroWizard };
		const auto it = g_hsAeroWizardTheme.find(hTheme);
		if (it != g_hsAeroWizardTheme.end())
		{
			EckAssert(it->second > 0);
			if (!--it->second)
				g_hsAeroWizardTheme.erase(it);
		}
	}
	{
		CSrwWriteGuard _{ g_LkThemeMapDateTimePicker };
		const auto it = g_hsDateTimePickerTheme.find(hTheme);
		if (it != g_hsDateTimePickerTheme.end())
		{
			EckAssert(it->second.first > 0);
			if (!--it->second.first)
			{
				s_pfnCloseThemeData(it->second.second);
				g_hsToolBarTheme.erase(it);
			}
		}
	}
	{
		CSrwWriteGuard _{ g_LkThemeMapListView };
		const auto it = g_hsListViewTheme.find(hTheme);
		if (it != g_hsListViewTheme.end())
		{
			EckAssert(it->second > 0);
			if (!--it->second)
				g_hsListViewTheme.erase(it);
		}
	}
	{
		CSrwWriteGuard _{ g_LkThemeMapLink };
		const auto it = g_hsLink.find(hTheme);
		if (it != g_hsLink.end())
		{
			EckAssert(it->second > 0);
			if (!--it->second)
				g_hsLink.erase(it);
		}
	}
}

static HRESULT WINAPI NewGetThemeColor(HTHEME hTheme, int iPartId, int iStateId, int iPropId, COLORREF* pColor);

// 某些控件使用此函数填充背景，仅父窗口实现WM_ERASETBKGND时成功，这里追加未实现的处理
static HRESULT WINAPI NewDrawThemeParentBackground(HWND hWnd, HDC hDC, const RECT* prc)
{
	const auto hr = s_pfnDrawThemeParentBackground(hWnd, hDC, prc);
	if (hr != S_OK)// include S_FALSE
	{
		if (!ShouldAppsUseDarkMode())
			return hr;
		RECT rc;
		if (!prc)
		{
			GetClipBox(hDC, &rc);
			prc = &rc;
		}
		SetDCBrushColor(hDC, GetThreadCtx()->crDefBkg);
		FillRect(hDC, prc, GetStockBrush(DC_BRUSH));
	}
	return S_OK;
}

// 非客户区滚动条
static HTHEME WINAPI NewOpenNcThemeData(HWND hWnd, PCWSTR pszClassList)
{
	if (_wcsicmp(pszClassList, L"ScrollBar") == 0)
		return s_pfnOpenNcThemeData(nullptr, L"Explorer::ScrollBar");// for nc scrollbar
	else ECKLIKELY
		return s_pfnOpenNcThemeData(hWnd, pszClassList);
}

// 多种主题修复
static HTHEME WINAPI NewOpenThemeData(HWND hWnd, PCWSTR pszClassList)
{
	HTHEME hTheme;
	if (ShouldAppsUseDarkMode())
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
	OnThemeOpen(hTheme, pszClassList);
	return hTheme;
}

// 多种主题修复
static HTHEME WINAPI NewOpenThemeDataForDpi(HWND hWnd, PCWSTR pszClassList, UINT uDpi)
{
	HTHEME hTheme;
	if (ShouldAppsUseDarkMode())
	{
		if (_wcsicmp(pszClassList, L"Combobox") == 0)
			hTheme = s_pfnOpenThemeDataForDpi(hWnd, L"DarkMode_CFD::Combobox", uDpi);
		else if (_wcsicmp(pszClassList, L"Edit") == 0)
			hTheme = s_pfnOpenThemeDataForDpi(hWnd, L"DarkMode_CFD::Edit", uDpi);
		else if (_wcsicmp(pszClassList, L"TaskDialog") == 0 ||
			_wcsicmp(pszClassList, L"TaskDialogStyle") == 0)
			hTheme = s_pfnOpenThemeDataForDpi(hWnd, L"DarkMode_Explorer::TaskDialog", uDpi);
		else
			hTheme = s_pfnOpenThemeDataForDpi(hWnd, pszClassList, uDpi);
	}
	else
		hTheme = s_pfnOpenThemeDataForDpi(hWnd, pszClassList, uDpi);
	OnThemeOpen(hTheme, pszClassList);
	return hTheme;
}

// 按钮、选择夹、工具条、时间日期选择器文本修复
static HRESULT WINAPI NewDrawThemeText(HTHEME hTheme, HDC hdc, int iPartId, int iStateId,
	LPCWSTR pszText, int cchText, DWORD dwTextFlags, DWORD dwTextFlags2, LPCRECT pRect)
{
	if (ShouldAppsUseDarkMode() && !IsBitSet(dwTextFlags, DT_CALCRECT))
	{
		const auto* const ptc = GetThreadCtx();
		if (IsThemeButton(hTheme))
			switch (iPartId)
			{
			case BP_CHECKBOX:
			case BP_GROUPBOX:
			case BP_RADIOBUTTON:
			{
				DTTOPTS dtt;
				dtt.dwSize = sizeof(dtt);
				dtt.dwFlags = DTT_TEXTCOLOR;
				dtt.crText = ptc->crDefText;
				return s_pfnDrawThemeTextEx(hTheme, hdc, iPartId, iStateId,
					pszText, cchText, dwTextFlags, (RECT*)pRect, &dtt);
			}
			break;
			}
		if (IsThemeTab(hTheme))
		{
			DTTOPTS dtt;
			dtt.dwSize = sizeof(dtt);
			dtt.dwFlags = DTT_TEXTCOLOR;
			dtt.crText = ptc->crDefText;
			return s_pfnDrawThemeTextEx(hTheme, hdc, iPartId, iStateId,
				pszText, cchText, dwTextFlags, (RECT*)pRect, &dtt);
		}
		if (IsThemeToolBar(hTheme) && iStateId != TS_DISABLED)
		{
			DTTOPTS dtt;
			dtt.dwSize = sizeof(dtt);
			dtt.dwFlags = DTT_TEXTCOLOR;
			dtt.crText = ptc->crDefText;
			return s_pfnDrawThemeTextEx(hTheme, hdc, iPartId, iStateId,
				pszText, cchText, dwTextFlags, (RECT*)pRect, &dtt);
		}
		if (IsThemeDateTimePicker(hTheme))
			switch (iPartId)
			{
			case DP_DATETEXT:
			{
				DTTOPTS dtt;
				dtt.dwSize = sizeof(dtt);
				dtt.dwFlags = DTT_TEXTCOLOR;

				switch (iStateId)
				{
				case DPDB_DISABLED:
					dtt.crText = 0x3f3f3f;
					break;
				default:
					dtt.crText = ptc->crDefText;
					break;
				}

				return s_pfnDrawThemeTextEx(hTheme, hdc, iPartId, iStateId,
					pszText, cchText, dwTextFlags, (RECT*)pRect, &dtt);
			}
			return S_OK;
			}
	}
	return s_pfnDrawThemeText(hTheme, hdc, iPartId, iStateId, pszText, cchText,
		dwTextFlags, dwTextFlags2, pRect);
}

// comctl32!SHThemeDrawText(未导出的内部函数)将调用DrawThemeTextEx，若失败，回落到DrawTextW
// 列表视图组标题修复
static HRESULT WINAPI NewDrawThemeTextEx(HTHEME hTheme, HDC hdc, int iPartId, int iStateId,
	LPCWSTR pszText, int cchText, DWORD dwTextFlags, LPRECT pRect, const DTTOPTS* pOptions)
{
	DTTOPTS NewOpt;
	if (ShouldAppsUseDarkMode() && !IsBitSet(dwTextFlags, DT_CALCRECT))
	{
		if (IsThemeListView(hTheme))
			if (!pOptions || (pOptions->dwFlags & DTT_COLORPROP))
			{
				NewOpt.dwSize = sizeof(DTTOPTS);
				NewOpt.dwFlags = DTT_TEXTCOLOR;
				NewGetThemeColor(hTheme, iPartId, iStateId,
					(pOptions && (pOptions->dwFlags & DTT_COLORPROP)) ?
					pOptions->iColorPropId : TMT_TEXTCOLOR, &NewOpt.crText);
				pOptions = &NewOpt;
			}
		if (IsThemeLink(hTheme))
			if (!pOptions || (pOptions->dwFlags & DTT_COLORPROP))
			{
				NewOpt.dwSize = sizeof(DTTOPTS);
				NewOpt.dwFlags = DTT_TEXTCOLOR;
				NewGetThemeColor(hTheme, iPartId, iStateId, TMT_TEXTCOLOR, &NewOpt.crText);
				pOptions = &NewOpt;
			}
	}
	return s_pfnDrawThemeTextEx(hTheme, hdc, iPartId, iStateId, pszText, cchText,
		dwTextFlags, pRect, pOptions);
}

static HRESULT DtbAdjustLuma(HTHEME hTheme, HDC hDC, int iPartId, int iStateId,
	const RECT* prc, const DTBGOPTS& Opt, float fPercent = 0.8f)
{
	RECT rcReal{ 0,0,prc->right - prc->left, prc->bottom - prc->top };
	CEzCDC DC{};
	DC.Create32(nullptr, rcReal.right, rcReal.bottom);

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
		nullptr, Gdiplus::ColorMatrixFlagsDefault);
	GpGraphics* pGraphics;
	GdipCreateFromHDC(hDC, &pGraphics);
	if (Opt.dwFlags & DTBG_CLIPRECT)
	{
		GdipSetClipRectI(pGraphics, Opt.rcClip.left, Opt.rcClip.top,
			Opt.rcClip.right - Opt.rcClip.left, Opt.rcClip.bottom - Opt.rcClip.top,
			Gdiplus::CombineModeReplace);
	}
	GdipDrawImageRectRectI(pGraphics, pBitmap, prc->left, prc->top, rcReal.right, rcReal.bottom,
		0, 0, rcReal.right, rcReal.bottom, Gdiplus::UnitPixel, pIA, nullptr, nullptr);
	GdipDeleteGraphics(pGraphics);
	GdipDisposeImage(pBitmap);
	GdipDisposeImageAttributes(pIA);
	return S_OK;
}

static HRESULT DtbAdjustLuma(HTHEME hTheme, HDC hDC, int iPartId, int iStateId,
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

// 任务对话框、按钮、选择夹、工具条、Aero向导背景、时间日期选择器背景修复
static HRESULT WINAPI NewDrawThemeBackgroundEx(HTHEME hTheme, HDC hdc, int iPartId, int iStateId,
	LPCRECT pRect, const DTBGOPTS* pOptions)
{
	if (ShouldAppsUseDarkMode())
	{
		const auto* const ptc = GetThreadCtx();
		if (IsThemeTaskDialog(hTheme))
			switch (iPartId)
			{
			case TDLG_SECONDARYPANEL:
				iPartId = TDLG_FOOTNOTEPANE;
				break;
			case TDLG_FOOTNOTESEPARATOR:
				return DtbAdjustLuma(hTheme, hdc, iPartId, iStateId, pRect, *pOptions, -0.67f);
			}
		else if (IsThemeButton(hTheme))
			switch (iPartId)
			{
			case BP_COMMANDLINKGLYPH:
				return DtbAdjustLuma(hTheme, hdc, iPartId, iStateId, pRect, *pOptions, 0.3f);
			case BP_COMMANDLINK:
				return DtbAdjustLuma(hTheme, hdc, iPartId, iStateId, pRect, *pOptions, 0.9f);
			}
		else if (IsThemeTab(hTheme))
			switch (iPartId)
			{
			case TABP_TABITEM:
			case TABP_TABITEMLEFTEDGE:
			case TABP_TABITEMRIGHTEDGE:
			case TABP_TABITEMBOTHEDGE:
			case TABP_TOPTABITEM:
			case TABP_TOPTABITEMLEFTEDGE:
			case TABP_TOPTABITEMRIGHTEDGE:
			case TABP_TOPTABITEMBOTHEDGE:
			{
				float f;
				switch (iStateId)
				{
				case TIS_NORMAL:
					f = -0.75f;
					break;
				case TIS_HOT:
					f = -0.8f;
					break;
				case TIS_FOCUSED:
				case TIS_SELECTED:
					f = -0.86f;
					break;
				case TIS_DISABLED:
					f = -0.6f;
					break;
				default:
					return E_INVALIDARG;
				}

				if (iStateId == TIS_SELECTED)
				{
					RECT rc{ *pRect };
					--rc.bottom;
					if (pOptions->dwFlags & DTBG_CLIPRECT)
						IntersectRect(rc, rc, pOptions->rcClip);
					const auto hr = DtbAdjustLuma(hTheme, hdc, iPartId, iStateId, pRect,
						&rc /*补齐最左选项卡的一个像素*/, f);
					if (hr == S_OK)// exclude S_FALSE
					{
						const auto sdc = SaveDcClip(hdc);
						IntersectClipRect(hdc, rc.left, rc.top, rc.right, rc.bottom);
						FrameRect(hdc, pRect, GetStockBrush(GRAY_BRUSH));
						RestoreDcClip(hdc, sdc);
					}
					return hr;
				}
				else
					return DtbAdjustLuma(hTheme, hdc, iPartId, iStateId, pRect, *pOptions, f);
			}
			case TABP_PANE:
			case TABP_BODY:
			case TABP_AEROWIZARDBODY:
			{
				const auto hr = DtbAdjustLuma(hTheme, hdc, iPartId, iStateId, pRect, *pOptions, -0.86f);
				if (hr == S_OK)// exclude S_FALSE
				{
					if (pOptions->dwFlags & DTBG_CLIPRECT)
					{
						const auto sdc = SaveDcClip(hdc);
						IntersectClipRect(hdc, pOptions->rcClip);
						FrameRect(hdc, pRect, GetStockBrush(GRAY_BRUSH));
						RestoreDcClip(hdc, sdc);
					}
					else
						FrameRect(hdc, pRect, GetStockBrush(GRAY_BRUSH));
				}
				return hr;
			}
			}
		else if (IsThemeListView(hTheme))
			switch (iPartId)
			{
			case LVP_GROUPHEADERLINE:
				return DtbAdjustLuma(hTheme, hdc, iPartId, iStateId, pRect, *pOptions, 0.3f);
			}
		else
		{
			HTHEME hThemeTBLV;
			if (IsThemeToolBar(hTheme, &hThemeTBLV))
			{
				if (iPartId == 0 && iStateId == 0)
				{
					SetDCBrushColor(hdc, ptc->crDefBkg);
					if (pOptions->dwFlags & DTBG_CLIPRECT)
					{
						const auto sdc = SaveDcClip(hdc);
						IntersectClipRect(hdc, pOptions->rcClip);
						FillRect(hdc, pRect, GetStockBrush(DC_BRUSH));
						RestoreDcClip(hdc, sdc);
					}
					else
						FillRect(hdc, pRect, GetStockBrush(DC_BRUSH));
					return S_OK;
				}
				switch (iPartId)
				{
				case TP_BUTTON:
				case TP_DROPDOWNBUTTON:
				case TP_SPLITBUTTON:
				{
					switch (iStateId)
					{
					case TS_NORMAL:
					{
						SetDCBrushColor(hdc, ptc->crDefBkg);
						if (pOptions->dwFlags & DTBG_CLIPRECT)
						{
							const auto sdc = SaveDcClip(hdc);
							IntersectClipRect(hdc, pOptions->rcClip);
							FillRect(hdc, pRect, GetStockBrush(DC_BRUSH));
							RestoreDcClip(hdc, sdc);
						}
						else
							FillRect(hdc, pRect, GetStockBrush(DC_BRUSH));
					}
					return S_OK;
					case TS_DISABLED:
						iStateId = LISS_SELECTEDNOTFOCUS;
						break;
					case TS_CHECKED:
						iStateId = LISS_SELECTED;
						break;
					case TS_NEARHOT:
					case TS_OTHERSIDEHOT:
						iStateId = LISS_HOT;
						break;
					}
					iPartId = LVP_LISTITEM;
				}
				return s_pfnDrawThemeBackgroundEx(hThemeTBLV, hdc, iPartId, iStateId, pRect, pOptions);

				default:
					return DtbAdjustLuma(hTheme, hdc, iPartId, iStateId, pRect, *pOptions, 0.7f);
				}
			}
			else if (IsThemeAeroWizard(hTheme))
				switch (iPartId)
				{
				case AW_HEADERAREA:
				case AW_CONTENTAREA:
				{
					if (pOptions->dwFlags & DTBG_CLIPRECT)
					{
						const auto sdc = SaveDcClip(hdc);
						IntersectClipRect(hdc, pOptions->rcClip);
						SetDCBrushColor(hdc, ptc->crDefBkg);
						FillRect(hdc, pRect, GetStockBrush(DC_BRUSH));
						RestoreDcClip(hdc, sdc);
					}
					else
					{
						SetDCBrushColor(hdc, ptc->crDefBkg);
						FillRect(hdc, pRect, GetStockBrush(DC_BRUSH));
					}
				}
				return S_OK;
				case AW_COMMANDAREA:
				{
					if (pOptions->dwFlags & DTBG_CLIPRECT)
					{
						const auto sdc = SaveDcClip(hdc);
						IntersectClipRect(hdc, pOptions->rcClip);
						SetDCBrushColor(hdc, ptc->crDefBtnFace);
						FillRect(hdc, pRect, GetStockBrush(DC_BRUSH));
						RestoreDcClip(hdc, sdc);
					}
					else
					{
						SetDCBrushColor(hdc, ptc->crDefBtnFace);
						FillRect(hdc, pRect, GetStockBrush(DC_BRUSH));
					}
				}
				return S_OK;
				}
		}
	}
	return s_pfnDrawThemeBackgroundEx(hTheme, hdc, iPartId, iStateId, pRect, pOptions);
}

// 任务对话框、按钮、选择夹、工具条、Aero向导背景、时间日期选择器背景修复
static HRESULT WINAPI NewDrawThemeBackground(HTHEME hTheme, HDC hdc, int iPartId, int iStateId,
	LPCRECT pRect, LPCRECT pClipRect)
{
	if (ShouldAppsUseDarkMode())
	{
		const auto* const ptc = GetThreadCtx();
		if (IsThemeTaskDialog(hTheme))
			switch (iPartId)
			{
			case TDLG_SECONDARYPANEL:
				iPartId = TDLG_FOOTNOTEPANE;
				break;
			case TDLG_FOOTNOTESEPARATOR:
				return DtbAdjustLuma(hTheme, hdc, iPartId, iStateId, pRect, pClipRect, -0.67f);
			}
		else if (IsThemeButton(hTheme))
			switch (iPartId)
			{
			case BP_COMMANDLINKGLYPH:
				return DtbAdjustLuma(hTheme, hdc, iPartId, iStateId, pRect, pClipRect, 0.3f);
			case BP_COMMANDLINK:
				return DtbAdjustLuma(hTheme, hdc, iPartId, iStateId, pRect, pClipRect, 0.9f);
			}
		else if (IsThemeTab(hTheme))
			switch (iPartId)
			{
			case TABP_TABITEM:
			case TABP_TABITEMLEFTEDGE:
			case TABP_TABITEMRIGHTEDGE:
			case TABP_TABITEMBOTHEDGE:
			case TABP_TOPTABITEM:
			case TABP_TOPTABITEMLEFTEDGE:
			case TABP_TOPTABITEMRIGHTEDGE:
			case TABP_TOPTABITEMBOTHEDGE:
			{
				float f;
				switch (iStateId)
				{
				case TIS_NORMAL:
					f = -0.75f;
					break;
				case TIS_HOT:
					f = -0.8f;
					break;
				case TIS_FOCUSED:
				case TIS_SELECTED:
					f = -0.86f;
					break;
				case TIS_DISABLED:
					f = -0.6f;
					break;
				default:
					return E_INVALIDARG;
				}

				if (iStateId == TIS_SELECTED)
				{
					RECT rc{ *pRect };
					--rc.bottom;
					if (pClipRect)
						IntersectRect(rc, rc, *pClipRect);
					const auto hr = DtbAdjustLuma(hTheme, hdc, iPartId, iStateId, pRect,
						&rc /*补齐最左选项卡的一个像素*/, f);
					if (hr == S_OK)// exclude S_FALSE
					{
						const auto sdc = SaveDcClip(hdc);
						IntersectClipRect(hdc, rc.left, rc.top, rc.right, rc.bottom);
						FrameRect(hdc, pRect, GetStockBrush(GRAY_BRUSH));
						RestoreDcClip(hdc, sdc);
					}
					return hr;
				}
				else
					return DtbAdjustLuma(hTheme, hdc, iPartId, iStateId, pRect, pClipRect, f);
			}
			case TABP_PANE:
			case TABP_BODY:
			case TABP_AEROWIZARDBODY:
			{
				const auto hr = DtbAdjustLuma(hTheme, hdc, iPartId, iStateId, pRect, pClipRect, -0.86f);
				if (hr == S_OK)// exclude S_FALSE
				{
					if (pClipRect)
					{
						const auto sdc = SaveDcClip(hdc);
						IntersectClipRect(hdc, *pClipRect);
						FrameRect(hdc, pRect, GetStockBrush(GRAY_BRUSH));
						RestoreDcClip(hdc, sdc);
					}
					else
						FrameRect(hdc, pRect, GetStockBrush(GRAY_BRUSH));
				}
				return hr;
			}
			}
		else if (IsThemeListView(hTheme))
			switch (iPartId)
			{
			case LVP_GROUPHEADERLINE:
				return DtbAdjustLuma(hTheme, hdc, iPartId, iStateId, pRect, pClipRect, 0.3f);
			}
		else
		{
			HTHEME hTheme2;
			if (IsThemeToolBar(hTheme, &hTheme2))
			{
				if (iPartId == 0 && iStateId == 0)
				{
					SetDCBrushColor(hdc, ptc->crDefBkg);
					if (pClipRect)
					{
						const auto sdc = SaveDcClip(hdc);
						IntersectClipRect(hdc, *pClipRect);
						FillRect(hdc, pRect, GetStockBrush(DC_BRUSH));
						RestoreDcClip(hdc, sdc);
					}
					else
						FillRect(hdc, pRect, GetStockBrush(DC_BRUSH));
					return S_OK;
				}
				switch (iPartId)
				{
				case TP_BUTTON:
				case TP_DROPDOWNBUTTON:
				case TP_SPLITBUTTON:
				{
					switch (iStateId)
					{
					case TS_NORMAL:
					{
						SetDCBrushColor(hdc, ptc->crDefBkg);
						if (pClipRect)
						{
							const auto sdc = SaveDcClip(hdc);
							IntersectClipRect(hdc, *pClipRect);
							FillRect(hdc, pRect, GetStockBrush(DC_BRUSH));
							RestoreDcClip(hdc, sdc);
						}
						else
							FillRect(hdc, pRect, GetStockBrush(DC_BRUSH));
					}
					return S_OK;
					case TS_DISABLED:
						iStateId = LISS_SELECTEDNOTFOCUS;
						break;
					case TS_CHECKED:
						iStateId = LISS_SELECTED;
						break;
					case TS_NEARHOT:
					case TS_OTHERSIDEHOT:
						iStateId = LISS_HOT;
						break;
					}
					iPartId = LVP_LISTITEM;
				}
				return s_pfnDrawThemeBackground(hTheme2, hdc, iPartId, iStateId, pRect, pClipRect);

				default:
					return DtbAdjustLuma(hTheme, hdc, iPartId, iStateId, pRect, pClipRect, 0.7f);
				}
			}
			else if (IsThemeAeroWizard(hTheme))
				switch (iPartId)
				{
				case AW_HEADERAREA:
				case AW_CONTENTAREA:
				{
					if (pClipRect)
					{
						const auto sdc = SaveDcClip(hdc);
						IntersectClipRect(hdc, *pClipRect);
						SetDCBrushColor(hdc, ptc->crDefBkg);
						FillRect(hdc, pRect, GetStockBrush(DC_BRUSH));
						RestoreDcClip(hdc, sdc);
					}
					else
					{
						SetDCBrushColor(hdc, ptc->crDefBkg);
						FillRect(hdc, pRect, GetStockBrush(DC_BRUSH));
					}
				}
				return S_OK;
				case AW_COMMANDAREA:
				{
					if (pClipRect)
					{
						const auto sdc = SaveDcClip(hdc);
						IntersectClipRect(hdc, *pClipRect);
						SetDCBrushColor(hdc, ptc->crDefBtnFace);
						FillRect(hdc, pRect, GetStockBrush(DC_BRUSH));
						RestoreDcClip(hdc, sdc);
					}
					else
					{
						SetDCBrushColor(hdc, ptc->crDefBtnFace);
						FillRect(hdc, pRect, GetStockBrush(DC_BRUSH));
					}
				}
				return S_OK;
				}
			else if (IsThemeDateTimePicker(hTheme, &hTheme2))
				switch (iPartId)
				{
				case DP_DATEBORDER:
					return s_pfnDrawThemeBackground(hTheme2, hdc, CP_READONLY, 1, pRect, pClipRect);
				case DP_SHOWCALENDARBUTTONRIGHT:
					if (iStateId != DPSCBR_DISABLED)
					{
						if (iStateId != DPSCBR_NORMAL)
						{
							if (HRESULT hr; FAILED(hr = s_pfnDrawThemeBackground(hTheme2,
								hdc, CP_READONLY, iStateId, pRect, pClipRect)))
								return hr;
						}
						return s_pfnDrawThemeBackground(hTheme, hdc, DP_SHOWCALENDARBUTTONRIGHT, DPSCBR_NORMAL,
							pRect, pClipRect);
					}
					break;
				}
		}
	}
	return s_pfnDrawThemeBackground(hTheme, hdc, iPartId, iStateId, pRect, pClipRect);
}

// 任务对话框、Aero向导、时间日期选择器文本颜色修复
static HRESULT WINAPI NewGetThemeColor(HTHEME hTheme, int iPartId, int iStateId, int iPropId, COLORREF* pColor)
{
	if (ShouldAppsUseDarkMode())
	{
		const auto* const ptc = GetThreadCtx();
		if (IsThemeTaskDialog(hTheme))
		{
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
				{
					*pColor = ptc->crDefText;
					return S_OK;
				}
			}
			break;

			case TDLG_MAININSTRUCTIONPANE:
			{
				const auto hr = s_pfnGetThemeColor(hTheme, iPartId, iStateId, iPropId, pColor);
				*pColor = AdjustColorrefLuma(*pColor, 300);
				return hr;
			}
			break;
			}
		}
		else if (IsThemeAeroWizard(hTheme))
		{
			switch (iPartId)
			{
			case AW_TITLEBAR:
				if (iPropId == TMT_TEXTCOLOR)
				{
					*pColor = ptc->crDefText;
					return S_OK;
				}
				break;
			case AW_HEADERAREA:
			{
				if (iPropId == TMT_TEXTCOLOR)
				{
					const auto hr = s_pfnGetThemeColor(hTheme, iPartId, iStateId, iPropId, pColor);
					*pColor = AdjustColorrefLuma(*pColor, 300);
					return hr;
				}
				else if (iPropId == TMT_FILLCOLOR)
				{
					*pColor = ptc->crDefBkg;
					return S_OK;
				}
			}
			break;
			case AW_CONTENTAREA:
				if (iPropId == TMT_TEXTCOLOR)
				{
					*pColor = ptc->crDefText;
					return S_OK;
				}
				else if (iPropId == TMT_FILLCOLOR)
				{
					*pColor = ptc->crDefBkg;
					return S_OK;
				}
				break;
			}
		}
		else if (IsThemeDateTimePicker(hTheme))
		{
			switch (iPartId)
			{
			case DP_DATETEXT:
			{
				switch (iStateId)
				{
				case DPDB_DISABLED:
					*pColor = 0x3f3f3f;
					break;
				default:
					*pColor = ptc->crDefText;
					break;
				}
			}
			return S_OK;
			}
		}
		else if (IsThemeLink(hTheme))
		{
			switch (iPartId)
			{
			case TP_LINK_HYPERLINK:
				if (iPropId == TMT_TEXTCOLOR)
				{
					const auto hr = s_pfnGetThemeColor(hTheme, iPartId, iStateId, iPropId, pColor);
					*pColor = AdjustColorrefLuma(*pColor, 200);
					return hr;
				}
				break;
			}
		}
		else if (IsThemeListView(hTheme))
		{
			switch (iPartId)
			{
			case LVP_GROUPHEADER:
			{
				switch (iPropId)
				{
				case TMT_HEADING1TEXTCOLOR:
					*pColor = ptc->crBlue1;
					return S_OK;
				case TMT_TEXTCOLOR:
				case TMT_BODYTEXTCOLOR:
				case TMT_HEADING2TEXTCOLOR:
					*pColor = ptc->crDefText;
					return S_OK;
				}
			}
			break;
			case LVP_GROUPHEADERLINE:
			{
				const auto hr = s_pfnGetThemeColor(hTheme, iPartId, iStateId, iPropId, pColor);
				*pColor = AdjustColorrefLuma(*pColor, 300);
				return hr;
			}
			}
		}
	}
	return s_pfnGetThemeColor(hTheme, iPartId, iStateId, iPropId, pColor);
}

// 平衡主题句柄记录
static HRESULT WINAPI NewCloseThemeData(HTHEME hTheme)
{
	OnThemeClose(hTheme);
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
	PCWSTR pszClass = nullptr;
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
	{ WCN_FORM },
	{ WCN_BK },
	{ WCN_SPLITBAR },
	{ WCN_DRAWPANEL },
	{ WCN_DRAWPANELD2D },
	{ WCN_LISTBOXNEW },
	{ WCN_ANIMATIONBOX },
	{ WCN_TREELIST },
	{ WCN_COMBOBOXNEW },
	{ WCN_PICTUREBOX },
	{ WCN_DUIHOST },
	{ WCN_VECDRAWPANEL },
	{ WCN_HEXEDIT },
	{ WCN_HITTER },
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
	//////////////保存实例句柄
	g_hInstance = hInstance;

	//////////////OS版本检测
	RtlGetNtVersionNumbers(&g_NtVer.uMajor, &g_NtVer.uMinor, &g_NtVer.uBuild);

	//////////////初始化Private API
	EckInitPrivateApi();

	//////////////线程上下文Tls槽
	g_dwTlsSlot = TlsAlloc();

	//////////////初始化线程上下文
	if (!IsBitSet(pInitParam->uFlags, EIF_NOINITTHREAD))
		ThreadInit();

	//////////////初始化GDI+
	if (!IsBitSet(pInitParam->uFlags, EIF_NOINITGDIPLUS))
	{
		GdiplusStartupInput gpsi{};
		const auto gps = GdiplusStartup(&g_uGpToken, &gpsi, nullptr);
		if (gps != Gdiplus::Ok)
		{
			*pdwErrCode = gps;
			return InitStatus::GdiplusInitError;
		}
	}

	//////////////注册窗口类
	g_WndClassInfo[0].iType = RWCT_CUSTOM;
	g_WndClassInfo[0].wc =
	{
		.style = CS_STDWND,
		.lpfnWndProc = DefDlgProcW,
		.cbWndExtra = DLGWINDOWEXTRA + sizeof(void*),
		.hInstance = g_hInstance,
		.hCursor = LoadCursorW(nullptr,IDC_ARROW),
		.lpszClassName = WCN_DLG
	};

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

	//////////////获取运行目录
	g_rsCurrDir.ReSize(32768);
	GetModuleFileNameW(nullptr, g_rsCurrDir.Data(), g_rsCurrDir.Size());
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
		hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT
#ifndef NDEBUG
			| D3D11_CREATE_DEVICE_DEBUG
#endif // !NDEBUG
			, pInitParam->pD3dFeatureLevel, pInitParam->cD3dFeatureLevel,
			D3D11_SDK_VERSION, &pD3DDevice, nullptr, nullptr);
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
		hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&g_pWicFactory));
		if (FAILED(hr))
		{
			*pdwErrCode = hr;
			EckDbgPrintFormatMessage(hr);
			return InitStatus::WicFactoryError;
		}
	}

	if (!IsBitSet(pInitParam->uFlags, EIF_NODARKMODE))
	{
		SetPreferredAppMode(PreferredAppMode::AllowDark);
		RefreshImmersiveColorStuff();
		FlushMenuTheme();
		DetourTransactionBegin();
		s_pfnOpenNcThemeData = pfnOpenNcThemeData;
		DetourAttach(&s_pfnOpenNcThemeData, NewOpenNcThemeData);
		DetourAttach(&s_pfnOpenThemeData, NewOpenThemeData);
		DetourAttach(&s_pfnDrawThemeText, NewDrawThemeText);
		DetourAttach(&s_pfnDrawThemeTextEx, NewDrawThemeTextEx);
		const HMODULE hModUx = LoadLibraryW(L"UxTheme.dll");
		EckAssert(hModUx);
		if (s_pfnOpenThemeDataForDpi = (FOpenThemeDataForDpi)GetProcAddress(hModUx, "OpenThemeDataForDpi"))
			DetourAttach(&s_pfnOpenThemeDataForDpi, NewOpenThemeDataForDpi);
		FreeLibrary(hModUx);
		DetourAttach(&s_pfnDrawThemeBackgroundEx, NewDrawThemeBackgroundEx);
		DetourAttach(&s_pfnDrawThemeBackground, NewDrawThemeBackground);
		DetourAttach(&s_pfnGetThemeColor, NewGetThemeColor);
		DetourAttach(&s_pfnCloseThemeData, NewCloseThemeData);
		DetourAttach(&s_pfnDrawThemeParentBackground, NewDrawThemeParentBackground);
		DetourTransactionCommit();
		DetourUpdateThread(NtCurrentThread());
	}
	return InitStatus::Ok;
}

void UnInit()
{
	if (g_dwTlsSlot)
	{
		TlsFree(g_dwTlsSlot);
		g_dwTlsSlot = 0;
	}
	if (g_uGpToken)
	{
		GdiplusShutdown(g_uGpToken);
		g_uGpToken = 0u;
	}
	g_hInstance = nullptr;
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
				WC_SCROLLBARW,
			};

			constexpr int c_cchMaxClsBuf = (int)std::max(
				{
					ARRAYSIZE(WC_BUTTONW),
					ARRAYSIZE(WC_HEADERW),
					ARRAYSIZE(WC_LISTVIEWW),
					ARRAYSIZE(TOOLBARCLASSNAMEW),
					ARRAYSIZE(TOOLTIPS_CLASSW),
					ARRAYSIZE(WC_TREEVIEWW),
					ARRAYSIZE(UPDOWN_CLASSW),
					ARRAYSIZE(WC_SCROLLBARW),
				}) + 1;

			const auto* const p = GetThreadCtx();
			if (iCode == HCBT_CREATEWND && p->bEnableDarkModeHook)
			{
				const auto pccw = (CBT_CREATEWNDW*)lParam;
				const auto hWnd = (HWND)wParam;
				AllowDarkModeForWindow(hWnd, TRUE);

				const auto lResult = CallNextHookEx(p->hhkCbtDarkMode, iCode, wParam, lParam);
				if (IsWindow(hWnd))
				{
					const BOOL bPszIsId = IS_INTRESOURCE(pccw->lpcs->lpszClass);
					WCHAR szCls[c_cchMaxClsBuf];
					if (bPszIsId)
						GetClassNameW(hWnd, szCls, c_cchMaxClsBuf);

					const auto itEnd = c_pszCommCtrlCls + ARRAYSIZE(c_pszCommCtrlCls);
					const auto it = std::find_if(c_pszCommCtrlCls, itEnd,
						[szCls, bPszIsId, pccw](PCWSTR psz)
						{
							if (bPszIsId)
								return _wcsicmp(szCls, psz) == 0;
							else
								return _wcsicmp(pccw->lpcs->lpszClass, psz) == 0;
						});

					if (it != itEnd)
					{
						if (it == c_pszCommCtrlCls + 1 || it == c_pszCommCtrlCls + 2)// HEADER/LISTVIEW
							SetWindowTheme(hWnd, L"ItemsView", nullptr);
						else ECKLIKELY
							SetWindowTheme(hWnd, L"Explorer", nullptr);
					}
				}
				return lResult;
			}

			return CallNextHookEx(p->hhkCbtDarkMode, iCode, wParam, lParam);
		}, nullptr, GetCurrentThreadId());
}

constexpr PCWSTR c_szErrInitStatus[]
{
	L"操作成功完成",
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
		EckDbgPrintWithPos(L"** WARNING ** 反初始化线程上下文时发现窗口映射表不为空");
		EckDbgBreak();
	}
#endif // _DEBUG
	UnhookWindowsHookEx(p->hhkTempCBT);
	UnhookWindowsHookEx(p->hhkCbtDarkMode);
	delete p;
	TlsSetValue(GetThreadCtxTlsSlot(), nullptr);
}

HHOOK BeginCbtHook(CWnd* pCurrWnd, FWndCreating pfnCreatingProc)
{
	EckAssert(pCurrWnd);
	const auto pCtx = GetThreadCtx();
	pCtx->pCurrWnd = pCurrWnd;
	pCtx->pfnWndCreatingProc = pfnCreatingProc;
	EckAssert(!pCtx->hhkTempCBT);
	pCtx->hhkTempCBT = SetWindowsHookExW(WH_CBT, [](int iCode, WPARAM wParam, LPARAM lParam)->LRESULT
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
		}, nullptr, GetCurrentThreadId());
	return pCtx->hhkTempCBT;
}

void EndCbtHook()
{
	const auto pCtx = GetThreadCtx();
	EckAssert(pCtx->hhkTempCBT);
	UnhookWindowsHookEx(pCtx->hhkTempCBT);
	pCtx->hhkTempCBT = nullptr;
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
	const auto* const pCtx = GetThreadCtx();
	auto s = Format(L"当前线程（TID = %u）窗口映射表内容：\n", GetCurrentThreadId());
	for (const auto& e : pCtx->hmWnd)
	{
		s.AppendFormat(L"\tCWnd指针 = 0x%0p，HWND = 0x%0p，标题 = %s，类名 = %s\n",
			e.second,
			e.first,
			e.second->GetText().Data(),
			e.second->GetClsName().Data());
	}
	s.AppendFormat(L"共有%u个窗口\n", pCtx->hmWnd.size());
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
	GetModuleFileNameW(nullptr, szPath, MAX_PATH);
	const auto rsContent = Format(L"程序位置：%s\n\n源文件：%s\n\n行号：%s\n\n测试表达式：%s",
		szPath, pszFile, pszLine, pszMsg);
	tdc.pszContent = rsContent.Data();

	tdc.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_USE_COMMAND_LINKS;

	TASKDIALOGCTX Ctx{ &tdc };
	CTaskDialog td;

	switch (td.DlgBox(nullptr, &Ctx))
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
	const auto bDark = GetItemsViewForeBackColor(crDefText, crDefBkg);
	crDefBtnFace = (bDark ? 0x303030 : GetSysColor(COLOR_BTNFACE));
	crBlue1 = (bDark ? RGB(0, 168, 255) : RGB(0, 51, 153));
	crGray1 = (bDark ? RGB(180, 180, 180) : GetSysColor(COLOR_GRAYTEXT));
}

void ECKTHREADCTX::SendThemeChangedToAllTopWindow()
{
	for (const auto& e : hmTopWnd)
	{
		BroadcastChildrenMessage(e.first, WM_THEMECHANGED, 0, 0);
		RedrawWindow(e.first, nullptr, nullptr, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW);
	}
}
ECK_NAMESPACE_END

#pragma push_macro("free")
#pragma push_macro("malloc")
#pragma push_macro("realloc")
#undef free
#undef malloc
#undef realloc
#include "YyJson/yyjson.c"
#pragma pop_macro("free")
#pragma pop_macro("malloc")
#pragma pop_macro("realloc")