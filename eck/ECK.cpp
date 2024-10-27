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

	const auto hModUx = LoadLibraryW(L"UxTheme.dll");
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

#pragma region UxTheme Fixer
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

using FOpenThemeData = HTHEME(WINAPI*)(HWND, PCWSTR);
using FDrawThemeText = HRESULT(WINAPI*)(HTHEME, HDC, int, int, PCWSTR, int, DWORD, DWORD, const RECT*);
using FDrawThemeTextEx = HRESULT(WINAPI*)(HTHEME, HDC, int, int, PCWSTR, int, DWORD, RECT*, const DTTOPTS*);
using FOpenThemeDataForDpi = HTHEME(WINAPI*)(HWND, PCWSTR, UINT);
using FDrawThemeBackgroundEx = HRESULT(WINAPI*)(HTHEME, HDC, int, int, const RECT*, const DTBGOPTS*);
using FDrawThemeBackground = HRESULT(WINAPI*)(HTHEME, HDC, int, int, const RECT*, const RECT*);
using FGetThemeColor = HRESULT(WINAPI*)(HTHEME, int, int, int, COLORREF*);
using FCloseThemeData = HRESULT(WINAPI*)(HTHEME);
using FDrawThemeParentBackground = HRESULT(WINAPI*)(HWND, HDC, const RECT*);
using FGetThemePartSize = HRESULT(WINAPI*)(HTHEME, HDC, int, int, const RECT*, THEMESIZE, SIZE*);

static FOpenNcThemeData			s_pfnOpenNcThemeData{};// 以序号导出
static FOpenThemeData			s_pfnOpenThemeData{ OpenThemeData };
static FDrawThemeText			s_pfnDrawThemeText{ DrawThemeText };
static FDrawThemeTextEx			s_pfnDrawThemeTextEx{ DrawThemeTextEx };
static FOpenThemeDataForDpi		s_pfnOpenThemeDataForDpi{};// DPI API引入较晚，动态加载之
static FDrawThemeBackgroundEx	s_pfnDrawThemeBackgroundEx{ DrawThemeBackgroundEx };
static FDrawThemeBackground		s_pfnDrawThemeBackground{ DrawThemeBackground };
static FGetThemeColor			s_pfnGetThemeColor{ GetThemeColor };
static FCloseThemeData			s_pfnCloseThemeData{ CloseThemeData };
static FDrawThemeParentBackground	s_pfnDrawThemeParentBackground{ DrawThemeParentBackground };
static FGetThemePartSize		s_pfnGetThemePartSize{ GetThemePartSize };

enum class ThemeType
{
	Invalid,
	Button,
	TaskDialog,		// +TaskDialogStyle
	Tab,
	ToolBar,		// 附带一个ItemsView::ListView主题，用于绘制背景
	AeroWizard,		// +AeroWizardStyle
	DateTimePicker,	// 附带一个DarkMode_CFD::ComboBox主题，用于绘制背景和下拉按钮
	ListView,		// +ItemsView
	Link,
	Header,			// 附带亮色版主题，弥补暗色没有过滤器按钮和溢出按钮的不足
	TextStyle,
};

struct THEME_INFO
{
	int cRef;
	ThemeType eType;
	HTHEME hThemeExtra;
	HWND hWnd;
};

// 主题记录集
static std::unordered_map<HTHEME, THEME_INFO> s_hsThemeMap{};
// 主题记录集读写锁
static CSrwLock s_LkThemeMap{};
// 若找不到记录，此常量作为占位
constexpr THEME_INFO InvalidThemeInfo{ .eType = ThemeType::Invalid };

// 查询主题信息。此函数返回引用，但在主题句柄被使用期间一定有效
const THEME_INFO& UxfGetThemeInfo(HTHEME hTheme)
{
	CSrwReadGuard _{ s_LkThemeMap };
	const auto it = s_hsThemeMap.find(hTheme);
	return it != s_hsThemeMap.end() ? it->second : InvalidThemeInfo;
}

// 注册主题句柄
static void UxfOnThemeOpen(HWND hWnd, HTHEME hTheme, PCWSTR pszClassList)
{
	if (!hTheme)
		return;
	//EckDbgPrintFmt(L"UxfOnThemeOpen(%p, %s)", hTheme, pszClassList);
	ThemeType eType;
	if (wcsstr(pszClassList, L"Button"))// Maybe ~";Ok"
		eType = ThemeType::Button;
	else if (_wcsicmp(pszClassList, L"TaskDialog") == 0 ||
		_wcsicmp(pszClassList, L"TaskDialogStyle") == 0)
		eType = ThemeType::TaskDialog;
	else if (_wcsicmp(pszClassList, L"Tab") == 0)
		eType = ThemeType::Tab;
	else if (_wcsicmp(pszClassList, L"ToolBar") == 0)
		eType = ThemeType::ToolBar;
	else if (_wcsicmp(pszClassList, L"AeroWizard") == 0 ||
		_wcsicmp(pszClassList, L"AeroWizardStyle") == 0)
		eType = ThemeType::AeroWizard;
	else if (_wcsicmp(pszClassList, L"DatePicker") == 0)
		eType = ThemeType::DateTimePicker;
	else if (wcsstr(pszClassList, L"ListView") || wcscmp(pszClassList, L"ItemsView") == 0)
		eType = ThemeType::ListView;
	else if (wcscmp(pszClassList, L"Link") == 0)
		eType = ThemeType::Link;
	else if (wcsstr(pszClassList, L"Header"))
		eType = ThemeType::Header;
	else if (_wcsicmp(pszClassList, L"TextStyle") == 0)
		eType = ThemeType::TextStyle;
	else
		eType = ThemeType::Invalid;
	if (eType != ThemeType::Invalid)
	{
		CSrwWriteGuard _{ s_LkThemeMap };
		auto& e = s_hsThemeMap[hTheme];
		e.eType = eType;
		if (!e.cRef)
		{
			switch (eType)
			{
			case ThemeType::ToolBar:
				e.hThemeExtra = s_pfnOpenThemeData(nullptr, L"ItemsView::ListView");
				break;
			case ThemeType::DateTimePicker:
				e.hThemeExtra = s_pfnOpenThemeData(nullptr, L"DarkMode_CFD::ComboBox");
				break;
			case ThemeType::Header:
				e.hThemeExtra = s_pfnOpenThemeData(nullptr, L"Explorer::Header");
				break;
			}
			e.hWnd = hWnd;
		}
		++e.cRef;
	}
}

// 注销主题句柄
static void UxfOnThemeClose(HTHEME hTheme)
{
	CSrwWriteGuard _{ s_LkThemeMap };
	const auto it = s_hsThemeMap.find(hTheme);
	if (it != s_hsThemeMap.end()) ECKLIKELY
	{
		auto & e = it->second;
		if (e.cRef > 1)
			--e.cRef;
		else
		{
			if (e.hThemeExtra)
				s_pfnCloseThemeData(e.hThemeExtra);
			s_hsThemeMap.erase(it);
		}
	}
}

// 绘制主题背景，并调整亮度
static HRESULT UxfAdjustLuma(HTHEME hTheme, HDC hDC, int iPartId, int iStateId,
	_In_ const RECT* prc, _In_opt_ const DTBGOPTS* pOpt, float fPercent)
{
	RECT rcReal{ 0,0,prc->right - prc->left, prc->bottom - prc->top };
	CEzCDC DC{};
	DC.Create32(nullptr, rcReal.right, rcReal.bottom);

	const BOOL bClip = pOpt && (pOpt->dwFlags & DTBG_CLIPRECT);
	HRESULT hr;
	if (bClip)
	{
		DTBGOPTS RealOpt;
		RealOpt.dwSize = sizeof(DTBGOPTS);
		RealOpt.dwFlags = pOpt->dwFlags & ~DTBG_CLIPRECT;
		hr = s_pfnDrawThemeBackgroundEx(hTheme, DC.GetDC(),
			iPartId, iStateId, &rcReal, &RealOpt);
	}
	else
		hr = s_pfnDrawThemeBackgroundEx(hTheme, DC.GetDC(),
			iPartId, iStateId, &rcReal, pOpt);
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
	if (bClip)
	{
		GdipSetClipRectI(pGraphics,
			pOpt->rcClip.left,
			pOpt->rcClip.top,
			pOpt->rcClip.right - pOpt->rcClip.left,
			pOpt->rcClip.bottom - pOpt->rcClip.top,
			Gdiplus::CombineModeReplace);
	}
	GdipDrawImageRectRectI(pGraphics, pBitmap, prc->left, prc->top, rcReal.right, rcReal.bottom,
		0, 0, rcReal.right, rcReal.bottom, Gdiplus::UnitPixel, pIA, nullptr, nullptr);
	GdipDeleteGraphics(pGraphics);
	GdipDisposeImage(pBitmap);
	GdipDisposeImageAttributes(pIA);
	return S_OK;
}

// 取主题颜色，主要是文本颜色
static HRESULT UxfGetThemeColor(const THEME_INFO& ti, const ECKTHREADCTX* ptc,
	HTHEME hTheme, int iPartId, int iStateId, int iPropId, COLORREF& cr)
{
	switch (ti.eType)
	{
	case ThemeType::Button:
		switch (iPartId)
		{
		case BP_CHECKBOX:
		case BP_GROUPBOX:
		case BP_RADIOBUTTON:
			if (iPropId == TMT_TEXTCOLOR)
			{
				cr = ptc->crDefText;
				return S_OK;
			}
			break;
		}
		break;
	case ThemeType::Tab:
		if (iPropId == TMT_TEXTCOLOR)
		{
			cr = ptc->crDefText;
			return S_OK;
		}
		break;
	case ThemeType::ToolBar:
		if (iStateId != TS_DISABLED)
		{
			cr = ptc->crDefText;
			return S_OK;
		}
		break;
	case ThemeType::TaskDialog:
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
				cr = ptc->crDefText;
				return S_OK;
			}
		}
		break;

		case TDLG_MAININSTRUCTIONPANE:
		{
			const auto hr = s_pfnGetThemeColor(hTheme, iPartId, iStateId, iPropId, &cr);
			cr = AdjustColorrefLuma(cr, 300);
			return hr;
		}
		break;
		}
		break;
	case ThemeType::AeroWizard:
		switch (iPartId)
		{
		case AW_TITLEBAR:
			if (iPropId == TMT_TEXTCOLOR)
			{
				cr = ptc->crDefText;
				return S_OK;
			}
			break;
		case AW_HEADERAREA:
		{
			if (iPropId == TMT_TEXTCOLOR)
			{
				/*const auto hr = s_pfnGetThemeColor(hTheme, iPartId, iStateId, iPropId, &cr);
				cr = AdjustColorrefLuma(cr, 300);
				return hr;*/
				cr = ptc->crBlue1;
				return S_OK;
			}
			else if (iPropId == TMT_FILLCOLOR)
			{
				cr = ptc->crDefBkg;
				return S_OK;
			}
		}
		break;
		case AW_CONTENTAREA:
			if (iPropId == TMT_TEXTCOLOR)
			{
				cr = ptc->crDefText;
				return S_OK;
			}
			else if (iPropId == TMT_FILLCOLOR)
			{
				cr = ptc->crDefBkg;
				return S_OK;
			}
			break;
		}
		break;
	case ThemeType::DateTimePicker:
		switch (iPartId)
		{
		case DP_DATETEXT:
		{
			switch (iStateId)
			{
			case DPDB_DISABLED:
				cr = 0x3f3f3f;
				break;
			default:
				cr = ptc->crDefText;
				break;
			}
		}
		return S_OK;
		}
		break;
	case ThemeType::Link:
		switch (iPartId)
		{
		case TP_LINK_HYPERLINK:
			if (iPropId == TMT_TEXTCOLOR)
			{
				const auto hr = s_pfnGetThemeColor(hTheme, iPartId, iStateId, iPropId, &cr);
				cr = AdjustColorrefLuma(cr, 200);
				return hr;
			}
			break;
		}
		break;
	case ThemeType::ListView:
		switch (iPartId)
		{
		case LVP_GROUPHEADER:
		{
			switch (iPropId)
			{
			case TMT_HEADING1TEXTCOLOR:
				cr = ptc->crBlue1;
				return S_OK;
			case TMT_TEXTCOLOR:
			case TMT_BODYTEXTCOLOR:
			case TMT_HEADING2TEXTCOLOR:
				cr = ptc->crDefText;
				return S_OK;
			}
		}
		break;
		case LVP_GROUPHEADERLINE:
		{
			const auto hr = s_pfnGetThemeColor(hTheme, iPartId, iStateId, iPropId, &cr);
			cr = AdjustColorrefLuma(cr, 300);
			return hr;
		}
		}
		break;
	case ThemeType::TextStyle:
		switch (iPartId)
		{
		case TEXT_MAININSTRUCTION:
		case TEXT_INSTRUCTION:
			if (iPropId == TMT_TEXTCOLOR)
			{
				cr = ptc->crBlue1;
				return S_OK;
			}
			break;
		case TEXT_BODYTITLE:
		case TEXT_BODYTEXT:
		case TEXT_SECONDARYTEXT:
		case TEXT_LABEL:
			if (iPropId == TMT_TEXTCOLOR)
			{
				cr = ptc->crDefText;
				return S_OK;
			}
			break;
		}
		break;
	}
	return E_NOTIMPL;
}

// 绘制主题背景
static HRESULT UxfDrawThemeBackground(const THEME_INFO& ti, const ECKTHREADCTX* ptc,
	HTHEME hTheme, HDC hDC, int iPartId, int iStateId,
	_In_ const RECT* prc, _In_opt_ const DTBGOPTS* pOptions)
{
	switch (ti.eType)
	{
	case ThemeType::TaskDialog:
		switch (iPartId)
		{
		case TDLG_SECONDARYPANEL:
			return s_pfnDrawThemeBackgroundEx(hTheme, hDC, TDLG_FOOTNOTEPANE, iStateId,
				prc, pOptions);
		case TDLG_FOOTNOTESEPARATOR:
			return UxfAdjustLuma(hTheme, hDC, iPartId, iStateId, prc, pOptions, -0.67f);
		}
		break;
	case ThemeType::Button:
		switch (iPartId)
		{
		case BP_COMMANDLINKGLYPH:
			return UxfAdjustLuma(hTheme, hDC, iPartId, iStateId, prc, pOptions, 0.3f);
		case BP_COMMANDLINK:
			return UxfAdjustLuma(hTheme, hDC, iPartId, iStateId, prc, pOptions, 0.9f);
		}
		break;
	case ThemeType::Tab:
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
			case TIS_NORMAL:	f = -0.75f; break;
			case TIS_HOT:		f = -0.8f;	break;
			case TIS_FOCUSED:
			case TIS_SELECTED:	f = -0.86f;	break;
			case TIS_DISABLED:	f = -0.6f;	break;
			default:			return E_INVALIDARG;
			}

			if (iStateId == TIS_SELECTED)
			{
				DTBGOPTS NewOpt;
				NewOpt.dwSize = sizeof(DTBGOPTS);
				NewOpt.dwFlags = (pOptions ? pOptions->dwFlags : 0) | DTBG_CLIPRECT;
				NewOpt.rcClip = *prc;
				--NewOpt.rcClip.bottom;
				if (pOptions->dwFlags & DTBG_CLIPRECT)
					IntersectRect(NewOpt.rcClip, NewOpt.rcClip, pOptions->rcClip);
				const auto hr = UxfAdjustLuma(hTheme, hDC, iPartId, iStateId, prc, &NewOpt, f);
				if (hr == S_OK)// Exclude S_FALSE.
				{
					const auto sdc = SaveDcClip(hDC);
					IntersectClipRect(hDC, NewOpt.rcClip.left, NewOpt.rcClip.top,
						NewOpt.rcClip.right, NewOpt.rcClip.bottom);
					FrameRect(hDC, prc, GetStockBrush(GRAY_BRUSH));
					RestoreDcClip(hDC, sdc);
				}
				return hr;
			}
			else
				return UxfAdjustLuma(hTheme, hDC, iPartId, iStateId, prc, pOptions, f);
		}
		break;
		case TABP_PANE:
		case TABP_BODY:
		case TABP_AEROWIZARDBODY:
		{
			const auto hr = UxfAdjustLuma(hTheme, hDC, iPartId, iStateId, prc, pOptions, -0.86f);
			if (hr == S_OK)// exclude S_FALSE
			{
				if (pOptions && pOptions->dwFlags & DTBG_CLIPRECT)
				{
					const auto sdc = SaveDcClip(hDC);
					IntersectClipRect(hDC, pOptions->rcClip);
					FrameRect(hDC, prc, GetStockBrush(GRAY_BRUSH));
					RestoreDcClip(hDC, sdc);
				}
				else
					FrameRect(hDC, prc, GetStockBrush(GRAY_BRUSH));
			}
			return hr;
		}
		break;
		}
		break;
	case ThemeType::ListView:
		switch (iPartId)
		{
		case LVP_GROUPHEADERLINE:
			return UxfAdjustLuma(hTheme, hDC, iPartId, iStateId, prc, pOptions, 0.3f);
		}
		break;
	case ThemeType::ToolBar:
	{
		if (iPartId == 0 && iStateId == 0)// 指示正在填充整个控件的背景
		{
			SetDCBrushColor(hDC, ptc->crDefBkg);
			if (pOptions && pOptions->dwFlags & DTBG_CLIPRECT)
			{
				const auto sdc = SaveDcClip(hDC);
				IntersectClipRect(hDC, pOptions->rcClip);
				FillRect(hDC, prc, GetStockBrush(DC_BRUSH));
				RestoreDcClip(hDC, sdc);
			}
			else
				FillRect(hDC, prc, GetStockBrush(DC_BRUSH));
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
				SetDCBrushColor(hDC, ptc->crDefBkg);
				if (pOptions && pOptions->dwFlags & DTBG_CLIPRECT)
				{
					const auto sdc = SaveDcClip(hDC);
					IntersectClipRect(hDC, pOptions->rcClip);
					FillRect(hDC, prc, GetStockBrush(DC_BRUSH));
					RestoreDcClip(hDC, sdc);
				}
				else
					FillRect(hDC, prc, GetStockBrush(DC_BRUSH));
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
		return s_pfnDrawThemeBackgroundEx(ti.hThemeExtra, hDC, iPartId, iStateId, prc, pOptions);

		default:
			return UxfAdjustLuma(hTheme, hDC, iPartId, iStateId, prc, pOptions, 0.7f);
		}
	}
	break;
	case ThemeType::AeroWizard:
		switch (iPartId)
		{
		case AW_HEADERAREA:
		case AW_CONTENTAREA:
		{
			if (pOptions && pOptions->dwFlags & DTBG_CLIPRECT)
			{
				const auto sdc = SaveDcClip(hDC);
				IntersectClipRect(hDC, pOptions->rcClip);
				SetDCBrushColor(hDC, ptc->crDefBkg);
				FillRect(hDC, prc, GetStockBrush(DC_BRUSH));
				RestoreDcClip(hDC, sdc);
			}
			else
			{
				SetDCBrushColor(hDC, ptc->crDefBkg);
				FillRect(hDC, prc, GetStockBrush(DC_BRUSH));
			}
		}
		return S_OK;
		case AW_COMMANDAREA:
		{
			if (pOptions && pOptions->dwFlags & DTBG_CLIPRECT)
			{
				const auto sdc = SaveDcClip(hDC);
				IntersectClipRect(hDC, pOptions->rcClip);
				SetDCBrushColor(hDC, ptc->crDefBtnFace);
				FillRect(hDC, prc, GetStockBrush(DC_BRUSH));
				RestoreDcClip(hDC, sdc);
			}
			else
			{
				SetDCBrushColor(hDC, ptc->crDefBtnFace);
				FillRect(hDC, prc, GetStockBrush(DC_BRUSH));
			}
		}
		return S_OK;
		}
		break;
	case ThemeType::DateTimePicker:
		switch (iPartId)
		{
		case DP_DATEBORDER:
		{
			const auto* const pClipRect =
				pOptions && (pOptions->dwFlags & DTBG_CLIPRECT) ?
				&pOptions->rcClip : nullptr;
			return s_pfnDrawThemeBackground(ti.hThemeExtra, hDC,
				CP_READONLY, 1, prc, pClipRect);
		}
		case DP_SHOWCALENDARBUTTONRIGHT:
			if (iStateId != DPSCBR_DISABLED)
			{
				const auto* const pClipRect =
					pOptions && (pOptions->dwFlags & DTBG_CLIPRECT) ?
					&pOptions->rcClip : nullptr;
				if (iStateId != DPSCBR_NORMAL)
				{
					if (HRESULT hr; FAILED(hr = s_pfnDrawThemeBackground(ti.hThemeExtra,
						hDC, CP_READONLY, iStateId, prc, pClipRect)))
						return hr;
				}
				return s_pfnDrawThemeBackground(hTheme, hDC,
					DP_SHOWCALENDARBUTTONRIGHT, DPSCBR_NORMAL, prc, pClipRect);
			}
			break;
		}
		break;
	case ThemeType::Header:
		switch (iPartId)
		{
		case 0:
			if (iStateId == 0)
			{
				if (FAILED(s_pfnDrawThemeBackgroundEx(hTheme, hDC,
					iPartId, iStateId, prc, pOptions)))
				{
					if (pOptions && pOptions->dwFlags & DTBG_CLIPRECT)
					{
						auto sdc = SaveDcClip(hDC);
						IntersectClipRect(hDC, pOptions->rcClip);
						SetDCBrushColor(hDC, ptc->crDefBkg);
						FillRect(hDC, prc, GetStockBrush(DC_BRUSH));
						RestoreDcClip(hDC, sdc);
					}
					else
					{
						SetDCBrushColor(hDC, ptc->crDefBkg);
						FillRect(hDC, prc, GetStockBrush(DC_BRUSH));
					}
				}
				return S_OK;
			}
			break;
		case HP_HEADERDROPDOWNFILTER:
			return UxfAdjustLuma(ti.hThemeExtra, hDC, iPartId, iStateId,
				prc, pOptions, iStateId != HDDFS_HOT ? 0.7f : -0.4f);
		case HP_HEADERSORTARROW:
			return UxfAdjustLuma(ti.hThemeExtra, hDC, iPartId, iStateId,
				prc, pOptions, iStateId == HOFS_NORMAL ? 0.6f : -0.4f);
		}
		break;
	}
	return E_NOTIMPL;
}


// 某些控件使用此函数填充背景，仅父窗口实现WM_ERASEBKGND时成功，这里追加未实现的处理
static HRESULT WINAPI NewDrawThemeParentBackground(HWND hWnd, HDC hDC, const RECT* prc)
{
	const auto hr = s_pfnDrawThemeParentBackground(hWnd, hDC, prc);
	if (hr != S_OK)// Include S_FALSE.
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
		return s_pfnOpenNcThemeData(nullptr, L"Explorer::ScrollBar");
	else ECKLIKELY
		return s_pfnOpenNcThemeData(hWnd, pszClassList);
}

static HTHEME WINAPI NewOpenThemeData(HWND hWnd, PCWSTR pszClassList)
{
	HTHEME hTheme;
	if (ShouldAppsUseDarkMode())
	{
		if (_wcsicmp(pszClassList, L"Combobox") == 0)
			hTheme = s_pfnOpenThemeData(hWnd, L"DarkMode_CFD::Combobox");
		else if (_wcsicmp(pszClassList, L"Edit") == 0)
			hTheme = s_pfnOpenThemeData(hWnd, L"DarkMode_CFD::Edit");
		else if (_wcsicmp(pszClassList, L"TaskDialog") == 0 ||
			_wcsicmp(pszClassList, L"TaskDialogStyle") == 0)
			hTheme = s_pfnOpenThemeData(hWnd, L"DarkMode_Explorer::TaskDialog");
		else
			hTheme = s_pfnOpenThemeData(hWnd, pszClassList);
	}
	else
		hTheme = s_pfnOpenThemeData(hWnd, pszClassList);
	UxfOnThemeOpen(hWnd, hTheme, pszClassList);
	return hTheme;
}

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
	UxfOnThemeOpen(hWnd, hTheme, pszClassList);
	return hTheme;
}

static HRESULT WINAPI NewDrawThemeText(HTHEME hTheme, HDC hDC, int iPartId, int iStateId,
	PCWSTR pszText, int cchText, DWORD dwTextFlags, DWORD dwTextFlags2, const RECT* pRect)
{
	if (ShouldAppsUseDarkMode() && !(dwTextFlags & DT_CALCRECT))
	{
		COLORREF cr;
		if (SUCCEEDED(UxfGetThemeColor(UxfGetThemeInfo(hTheme), GetThreadCtx(),
			hTheme, iPartId, iStateId, TMT_TEXTCOLOR, cr)))
		{
			DTTOPTS dtto;
			dtto.dwSize = sizeof(DTTOPTS);
			dtto.dwFlags = DTT_TEXTCOLOR;
			dtto.crText = cr;
			return s_pfnDrawThemeTextEx(hTheme, hDC, iPartId, iStateId,
				pszText, cchText, dwTextFlags, (RECT*)pRect, &dtto);
		}
	}
	return s_pfnDrawThemeText(hTheme, hDC, iPartId, iStateId, pszText, cchText,
		dwTextFlags, dwTextFlags2, pRect);
}

// comctl32!SHThemeDrawText调用DrawThemeTextEx，若失败，回落到DrawTextW
static HRESULT WINAPI NewDrawThemeTextEx(HTHEME hTheme, HDC hDC, int iPartId, int iStateId,
	PCWSTR pszText, int cchText, DWORD dwTextFlags, RECT* pRect, const DTTOPTS* pOptions)
{
	DTTOPTS NewOpt;
	if (ShouldAppsUseDarkMode() && !(dwTextFlags & DT_CALCRECT))
	{
		if (!pOptions || (pOptions->dwFlags & DTT_COLORPROP) ||
			!(pOptions->dwFlags & DTT_TEXTCOLOR))// 未指定颜色
		{
			if (SUCCEEDED(UxfGetThemeColor(UxfGetThemeInfo(hTheme), GetThreadCtx(),
				hTheme, iPartId, iStateId,
				(pOptions && (pOptions->dwFlags & DTT_COLORPROP)) ?
				pOptions->iColorPropId : TMT_TEXTCOLOR, NewOpt.crText)))
			{
				if (pOptions)
				{
					NewOpt = *pOptions;
					NewOpt.dwFlags |= DTT_TEXTCOLOR;
					NewOpt.dwFlags &= ~DTT_COLORPROP;
				}
				else
				{
					NewOpt.dwSize = sizeof(DTTOPTS);
					NewOpt.dwFlags = DTT_TEXTCOLOR;
				}
				pOptions = &NewOpt;
			}
		}
	}
	return s_pfnDrawThemeTextEx(hTheme, hDC, iPartId, iStateId, pszText, cchText,
		dwTextFlags, pRect, pOptions);
}

static HRESULT WINAPI NewDrawThemeBackgroundEx(HTHEME hTheme, HDC hDC, int iPartId, int iStateId,
	const RECT* pRect, const DTBGOPTS* pOptions)
{
	if (ShouldAppsUseDarkMode())
	{
		if (SUCCEEDED(UxfDrawThemeBackground(UxfGetThemeInfo(hTheme), GetThreadCtx(),
			hTheme, hDC, iPartId, iStateId, pRect, pOptions)))
		{
			return S_OK;
		}
	}
	return s_pfnDrawThemeBackgroundEx(hTheme, hDC, iPartId, iStateId, pRect, pOptions);
}

static HRESULT WINAPI NewDrawThemeBackground(HTHEME hTheme, HDC hDC, int iPartId, int iStateId,
	const RECT* pRect, const RECT* pClipRect)
{
	if (ShouldAppsUseDarkMode())
	{
		DTBGOPTS Options;
		Options.dwSize = sizeof(DTBGOPTS);
		if (pClipRect)
		{
			Options.dwFlags = DTBG_CLIPRECT;
			Options.rcClip = *pClipRect;
		}
		else
			Options.dwFlags = 0;
		if (SUCCEEDED(UxfDrawThemeBackground(UxfGetThemeInfo(hTheme), GetThreadCtx(),
			hTheme, hDC, iPartId, iStateId, pRect, &Options)))
		{
			return S_OK;
		}
	}
	return s_pfnDrawThemeBackground(hTheme, hDC, iPartId, iStateId, pRect, pClipRect);
}

static HRESULT WINAPI NewGetThemeColor(HTHEME hTheme, int iPartId, int iStateId,
	int iPropId, COLORREF* pColor)
{
	if (ShouldAppsUseDarkMode())
	{
		if (SUCCEEDED(UxfGetThemeColor(UxfGetThemeInfo(hTheme), GetThreadCtx(),
			hTheme, iPartId, iStateId, iPropId, *pColor)))
			return S_OK;
	}
	return s_pfnGetThemeColor(hTheme, iPartId, iStateId, iPropId, pColor);
}

static HRESULT WINAPI NewCloseThemeData(HTHEME hTheme)
{
	UxfOnThemeClose(hTheme);
	return s_pfnCloseThemeData(hTheme);
}

// 修正表头溢出按钮大小
static HRESULT WINAPI NewGetThemePartSize(HTHEME hTheme, HDC hDC, int iPartId, int iStateId,
	const RECT* prc, THEMESIZE eSize, SIZE* psz)
{
	const auto& ti = UxfGetThemeInfo(hTheme);
	if (ti.eType == ThemeType::Header)
		switch (iPartId)
		{
		case HP_HEADEROVERFLOW:
		{
			int iDpi;
			if (hDC)
				iDpi = GetDeviceCaps(hDC, LOGPIXELSX);
			else if (ti.hWnd)
				iDpi = GetDpi(ti.hWnd);
			else
				iDpi = USER_DEFAULT_SCREEN_DPI;
			psz->cx = DpiScale(16, iDpi);
			psz->cy = DpiScale(32, iDpi);
		}
		return S_OK;
		}
	return s_pfnGetThemePartSize(hTheme, hDC, iPartId, iStateId, prc, eSize, psz);
}
#pragma endregion UxTheme Fixer

enum
{
	RWCT_EZREG,
	RWCT_CUSTOM,
};

struct EZREGWNDINFO
{
	PCWSTR pszClass = nullptr;
	UINT uClassStyle = CS_STDWND;
};

static struct
{
	union
	{
		EZREGWNDINFO ez{};
		WNDCLASSW wc;
	};
	int iType = RWCT_EZREG;
}
s_WndClassInfo[]
{
	{ },// WCN_DLG
#ifdef ECK_OPT_NO_SIMPLE_WND_CLS
	{ WCN_LABEL },
	{ WCN_BK },
	{ WCN_LUNARCALENDAR },
	{ WCN_FORM },
	{ WCN_TABHEADER },
	{ WCN_SPLITBAR },
	{ WCN_DRAWPANEL },
	{ WCN_LISTBOXNEW },
	{ WCN_TREELIST },
	{ WCN_COMBOBOXNEW },
	{ WCN_PICTUREBOX },
	{ WCN_DUIHOST },
	{ WCN_VECDRAWPANEL },
	{ WCN_HEXEDIT },
	{ WCN_HITTER },
#else
	{ WCN_DUMMY },
#endif // defined(ECK_OPT_NO_SIMPLE_WND_CLS)
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
	s_WndClassInfo[0].iType = RWCT_CUSTOM;
	s_WndClassInfo[0].wc =
	{
		.style = CS_STDWND,
		.lpfnWndProc = DefDlgProcW,
		.cbWndExtra = DLGWINDOWEXTRA + sizeof(void*),
		.hInstance = g_hInstance,
		.hCursor = LoadCursorW(nullptr, IDC_ARROW),
		.lpszClassName = WCN_DLG
	};

	for (const auto& e : s_WndClassInfo)
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
		DetourAttach(&s_pfnGetThemePartSize, NewGetThemePartSize);
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
	s.AppendFormat(L"共有%u个窗口\n", (UINT)pCtx->hmWnd.size());
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
	crTip1 = (bDark ? RGB(131, 162, 198) : RGB(97, 116, 139));
}

void ECKTHREADCTX::SendThemeChangedToAllTopWindow()
{
	for (const auto& [hWnd, _] : hmTopWnd)
	{
		if (GetWindowLongPtrW(hWnd, GWL_STYLE) & WS_VISIBLE)
		{
			SendMessageW(hWnd, WM_SETREDRAW, FALSE, 0);
			SendMessageW(hWnd, WM_THEMECHANGED, 0, 0);
			BroadcastChildrenMessage(hWnd, WM_THEMECHANGED, 0, 0);
			SendMessageW(hWnd, WM_SETREDRAW, TRUE, 0);
		}
		else
		{
			SendMessageW(hWnd, WM_THEMECHANGED, 0, 0);
			BroadcastChildrenMessage(hWnd, WM_THEMECHANGED, 0, 0);
		}
		RedrawWindow(hWnd, nullptr, nullptr,
			RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
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