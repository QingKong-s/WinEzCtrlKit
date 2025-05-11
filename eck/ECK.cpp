#if ECK_OPT_MYDBG
#include "PchInc.h"
#endif// ECK_OPT_MYDBG

#include <coroutine>

// For Private API

FSetWindowCompositionAttribute	pfnSetWindowCompositionAttribute{};
FGetWindowCompositionAttribute	pfnGetWindowCompositionAttribute{};

#if !ECK_OPT_NO_DARKMODE
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
#endif// !ECK_OPT_NO_DARKMODE

FOpenNcThemeData				pfnOpenNcThemeData{};

#ifndef _WIN64
FNtWow64WriteVirtualMemory64	pfnNtWow64WriteVirtualMemory64{};
FNtWow64ReadVirtualMemory64		pfnNtWow64ReadVirtualMemory64{};
FNtWow64QueryVirtualMemory64	pfnNtWow64QueryVirtualMemory64{};
FNtWow64QueryInformationProcess64 pfnNtWow64QueryInformationProcess64{};
#endif

ECK_NAMESPACE_BEGIN
void InitPrivateApi()
{
	const auto hModUser32 = LoadLibraryW(L"User32.dll");
	EckAssert(hModUser32);
	pfnSetWindowCompositionAttribute = (FSetWindowCompositionAttribute)
		GetProcAddress(hModUser32, "SetWindowCompositionAttribute");
	pfnGetWindowCompositionAttribute = (FGetWindowCompositionAttribute)
		GetProcAddress(hModUser32, "GetWindowCompositionAttribute");
	FreeLibrary(hModUser32);

	const auto hModUx = LoadLibraryW(L"UxTheme.dll");
	EckAssert(hModUx);
	pfnOpenNcThemeData = (FOpenNcThemeData)
		GetProcAddress(hModUx, MAKEINTRESOURCEA(49));

#if !ECK_OPT_NO_DARKMODE
	if (g_NtVer.uMajor >= 10 && g_NtVer.uBuild >= WINVER_1809)
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
#endif// !ECK_OPT_NO_DARKMODE
	FreeLibrary(hModUx);

#ifndef _WIN64
	const auto hModNtdll = GetModuleHandleW(L"ntdll.dll");
	EckAssert(hModNtdll);
	pfnNtWow64WriteVirtualMemory64 = (FNtWow64WriteVirtualMemory64)
		GetProcAddress(hModNtdll, "NtWow64WriteVirtualMemory64");
	pfnNtWow64ReadVirtualMemory64 = (FNtWow64ReadVirtualMemory64)
		GetProcAddress(hModNtdll, "NtWow64ReadVirtualMemory64");
	pfnNtWow64QueryVirtualMemory64 = (FNtWow64QueryVirtualMemory64)
		GetProcAddress(hModNtdll, "NtWow64QueryVirtualMemory64");
	pfnNtWow64QueryInformationProcess64 = (FNtWow64QueryInformationProcess64)
		GetProcAddress(hModNtdll, "NtWow64QueryInformationProcess64");
#endif
}

// For program

HINSTANCE	g_hInstance{};
CRefStrW	g_rsCurrDir{};
DWORD		g_dwTlsSlot{};
NTVER		g_NtVer{};

HMODULE		g_hModComCtl32{ (HMODULE)SIZETMax };

// For GdiPlus
ULONG_PTR	g_uGpToken{};

// For DirectX

IWICImagingFactory* g_pWicFactory{};
IDWriteFactory* g_pDwFactory{};
#if !ECK_OPT_NO_DX
ID2D1Factory1* g_pD2dFactory{};
ID2D1Device* g_pD2dDevice{};
ID3D11Device* g_pD3d11Device{};
IDXGIDevice1* g_pDxgiDevice{};
IDXGIFactory2* g_pDxgiFactory{};
#ifdef _DEBUG
IDXGIDebug* g_pDxgiDebug{};
#endif
#endif// !ECK_OPT_NO_DX

// For Text Services

void* g_pfnCreateTextServices{};
void* g_pfnShutdownTextServices{};
void* g_pfnCreateTextServices20{};
void* g_pfnShutdownTextServices20{};

const CRefStrW& GetRunningPath()
{
	return g_rsCurrDir;
}

#pragma region WndClass
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
#if ECK_OPT_NO_SIMPLE_WND_CLS
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
#pragma endregion Wnds

#pragma region UxFixer
// 未在SDK中定义的部件和状态代码
enum
{
	//===部件===
	// Status
	STATUSBAR_PART_COMMON = 0,
	STATUSBAR_PART_PANE = 1,
	STATUSBAR_PART_GRIPPERPANE = 2,
	STATUSBAR_PART_GRIPPER = 3,
};

#if !ECK_OPT_NO_DARKMODE
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
using FSoftModalMessageBox = int(WINAPI*)(void*/* _MSGBOXDATA* */);

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
// Not Ux, but necessary.
static FSoftModalMessageBox		s_pfnSoftModalMessageBox{};

enum class ThemeType
{
	Invalid,
	Button,
	TaskDialog,		// +TaskDialogStyle
	Tab,
	// 附带一个ItemsView::ListView主题，用于绘制背景
	// 不使用DarkMode::ToolBar，因为它的某些部件不正确
	ToolBar,
	AeroWizard,		// +AeroWizardStyle
	DateTimePicker,	// 附带一个DarkMode_CFD::ComboBox主题，用于绘制背景和下拉按钮
	ListView,		// +ItemsView
	Link,
	Header,			// 附带亮色版主题，弥补暗色没有过滤器按钮和溢出按钮的不足
	TextStyle,
	Progress,		// +Indeterminate::Progress
	ControlPanel,	// +ControlPanelStyle
	MonthCalendar,
	StatusBar,		// +StatusBarStyle
	Menu,
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
EckInline const THEME_INFO& UxfpGetThemeInfo(HTHEME hTheme)
{
	CSrwReadGuard _{ s_LkThemeMap };
	const auto it = s_hsThemeMap.find(hTheme);
	return it != s_hsThemeMap.end() ? it->second : InvalidThemeInfo;
}

// 注册主题句柄
static void UxfpOnThemeOpen(HWND hWnd, HTHEME hTheme, PCWSTR pszClassList)
{
	if (!hTheme)
		return;
	/*EckDbgPrintFmt(L"UxfpOnThemeOpen: hWnd = %p, hTheme = %p, pszClassList = %s",
		hWnd, hTheme, pszClassList);*/
	ThemeType eType;
	if (EckIsStartWithConstStringIW(pszClassList, L"Button"))
		eType = ThemeType::Button;
	else if (EckIsStartWithConstStringIW(pszClassList, L"TaskDialog"))
		eType = ThemeType::TaskDialog;
	else if (TcsEqualI(pszClassList, L"Tab"))
		eType = ThemeType::Tab;
	else if (TcsEqualI(pszClassList, L"ToolBar"))
		eType = ThemeType::ToolBar;
	else if (EckIsStartWithConstStringIW(pszClassList, L"AeroWizard"))
		eType = ThemeType::AeroWizard;
	else if (TcsEqualI(pszClassList, L"DatePicker"))
		eType = ThemeType::DateTimePicker;
	else if (TcsStrI(pszClassList, L"ListView") ||
		TcsEqualI(pszClassList, L"ItemsView"))
		eType = ThemeType::ListView;
	else if (TcsEqualI(pszClassList, L"Link"))
		eType = ThemeType::Link;
	else if (TcsStrI(pszClassList, L"Header"))
		eType = ThemeType::Header;
	else if (TcsEqualI(pszClassList, L"TextStyle"))
		eType = ThemeType::TextStyle;
	else if (TcsEqualI(pszClassList, L"Progress") ||
		TcsEqualI(pszClassList, L"Indeterminate::Progress"))
		eType = ThemeType::Progress;
	else if (EckIsStartWithConstStringIW(pszClassList, L"ControlPanel"))
		eType = ThemeType::ControlPanel;
	else if (TcsEqualI(pszClassList, L"MonthCal"))
		eType = ThemeType::MonthCalendar;
	else if (EckIsStartWithConstStringIW(pszClassList, L"Status"))
		eType = ThemeType::StatusBar;
	else if (TcsEqualI(pszClassList, L"Menu"))
		eType = ThemeType::Menu;
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
static void UxfpOnThemeClose(HTHEME hTheme)
{
	CSrwWriteGuard _{ s_LkThemeMap };
	const auto it = s_hsThemeMap.find(hTheme);
	if (it != s_hsThemeMap.end()) ECKLIKELY
	{
		auto& e = it->second;
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

EckInline constexpr BOOL UxfpIsDarkTaskDialogAvailable()
{
	return g_NtVer.uBuild >= WINVER_11_21H2;
}

HRESULT UxfMenuInit(CWnd* pWnd)
{
	auto& Sig = pWnd->GetSignal();
	if (Sig.FindSlot(MHI_UXF_MENU))
		return S_FALSE;
	const auto hTheme = OpenThemeData(pWnd->GetHWND(), L"Menu");
	auto Fn = [hTheme = hTheme, pWnd](HWND hWnd, UINT uMsg,
		WPARAM wParam, LPARAM lParam, SlotCtx& Ctx) mutable -> LRESULT
		{
			if (Ctx.IsDeleting())
			{
				CloseThemeData(hTheme);
				hTheme = nullptr;
				return 0;
			}
			switch (uMsg)
			{
			case WM_UAHDRAWMENU:
			{
				if (!ShouldAppsUseDarkMode())
					break;
				Ctx.Processed();

				MENUBARINFO mbi;
				mbi.cbSize = sizeof(mbi);
				GetMenuBarInfo(hWnd, OBJID_MENU, 0, &mbi);
				RECT rcWindow;
				GetWindowRect(hWnd, &rcWindow);
				OffsetRect(&mbi.rcBar, -rcWindow.left, -rcWindow.top);
				DrawThemeBackground(hTheme, ((UAHMENU*)lParam)->hdc,
					MENU_BARBACKGROUND,
					(GetForegroundWindow() == hWnd) ? MB_ACTIVE : MB_INACTIVE,
					&mbi.rcBar, nullptr);
			}
			return 0;
			case WM_UAHDRAWMENUITEM:
			{
				if (!ShouldAppsUseDarkMode())
					break;
				const auto* const pudmi = (UAHDRAWMENUITEM*)lParam;

				WCHAR szText[96];
				szText[0] = L'\0';
				MENUITEMINFO mii;
				mii.cbSize = sizeof(mii);
				mii.fMask = MIIM_STRING;
				mii.dwTypeData = szText;
				mii.cch = ARRAYSIZE(szText);
				GetMenuItemInfoW(pudmi->um.hmenu, pudmi->umi.iPosition, TRUE, &mii);
				if (mii.fType & MFT_OWNERDRAW)
					break;
				Ctx.Processed();

				const auto uState = pudmi->dis.itemState;
				int iState;
				if (uState & ODS_SELECTED)
					if ((uState & ODS_INACTIVE) ||
						(uState & ODS_GRAYED))
						iState = MBI_DISABLEDPUSHED;
					else
						iState = MBI_PUSHED;
				else if (uState & ODS_HOTLIGHT)
					if ((uState & ODS_INACTIVE) ||
						(uState & ODS_GRAYED))
						iState = MBI_DISABLEDHOT;
					else
						iState = MBI_HOT;
				else
					if ((uState & ODS_INACTIVE) ||
						(uState & ODS_GRAYED))
						iState = MBI_DISABLED;
					else
						iState = MBI_NORMAL;
				const DWORD dwDtFlags = DT_CENTER | DT_SINGLELINE | DT_VCENTER |
					((uState & ODS_NOACCEL) ? DT_HIDEPREFIX : 0);
				if ((uState & ODS_SELECTED) || (uState & ODS_HOTLIGHT))
					DrawThemeBackground(hTheme, pudmi->um.hdc,
						MENU_BARITEM, iState, &pudmi->dis.rcItem, nullptr);
				else
					DrawThemeBackground(hTheme, pudmi->um.hdc,
						MENU_BARBACKGROUND,
						(GetForegroundWindow() == hWnd) ? MB_ACTIVE : MB_INACTIVE,
						&pudmi->dis.rcItem, nullptr);
				DrawThemeText(hTheme, pudmi->um.hdc, MENU_BARITEM, iState,
					szText, mii.cch, dwDtFlags, 0, &pudmi->dis.rcItem);
			}
			return 0;
			case WM_NCACTIVATE:
				if (IsIconic(hWnd))
					break;
				[[fallthrough]];
			case WM_NCPAINT:
			{
				if (!ShouldAppsUseDarkMode())
					break;
				auto TmpCtx{ Ctx };
				const auto lResult = pWnd->GetSignal().CallNext(TmpCtx,
					hWnd, uMsg, wParam, lParam);
				if (TmpCtx.IsProcessed())
				{
					Ctx.Processed();
					return lResult;
				}
				pWnd->OnMsg(hWnd, uMsg, wParam, lParam);

				MENUBARINFO mbi;
				mbi.cbSize = sizeof(mbi);
				if (!GetMenuBarInfo(hWnd, OBJID_MENU, 0, &mbi))
					break;

				RECT rcClient, rcWindow;
				GetClientRect(hWnd, &rcClient);
				MapWindowRect(hWnd, nullptr, &rcClient);
				GetWindowRect(hWnd, &rcWindow);
				OffsetRect(&rcClient, -rcWindow.left, -rcWindow.top);

				RECT rcLine = rcClient;
				rcLine.bottom = rcLine.top;
				--rcLine.top;

				const auto hDC = GetWindowDC(hWnd);
				SetDCBrushColor(hDC, GetThreadCtx()->crDefBtnFace);
				FillRect(hDC, &rcLine, GetStockBrush(DC_BRUSH));
				ReleaseDC(hWnd, hDC);
				Ctx.Processed();

				if (uMsg == WM_NCACTIVATE)
					return TRUE;
			}
			return 0;

			case WM_CREATE:
			case WM_THEMECHANGED:
				CloseThemeData(hTheme);
				hTheme = OpenThemeData(hWnd, L"Menu");
				break;
			case WM_DESTROY:
				CloseThemeData(hTheme);
				hTheme = nullptr;
				break;
			case WM_NCDESTROY:
				UxfMenuUnInit(pWnd);
				break;
			}
			return 0;
		};

	Sig.Connect(Fn, MHI_UXF_MENU);
	return S_OK;
}

HRESULT UxfMenuUnInit(CWnd* pWnd)
{
	return pWnd->GetSignal().Disconnect(MHI_UXF_MENU) ? S_OK : S_FALSE;
}

/// <summary>
/// 绘制主题背景，并调整亮度
/// </summary>
/// <param name="fDelta">颜色分量的变化量，-1~1</param>
/// <param name="bInvert">是否反转颜色</param>
static HRESULT UxfpAdjustLuma(HTHEME hTheme, HDC hDC, int iPartId, int iStateId,
	_In_ const RECT* prc, _In_opt_ const DTBGOPTS* pOpt,
	float fDelta, BOOL bInvert = FALSE)
{
	const BOOL bClip = pOpt && (pOpt->dwFlags & DTBG_CLIPRECT);
	HRESULT hr;
	int eBgType;
	GetThemeEnumValue(hTheme, iPartId, iStateId, TMT_BGTYPE, &eBgType);
	if (eBgType == BT_IMAGEFILE)
	{
		CEzCDC DC{};
		int cxBuf, cyBuf;
		if (bClip)
		{
			// OPTIMIZATION：减少GDI+处理的像素
			cxBuf = pOpt->rcClip.right - pOpt->rcClip.left;
			cyBuf = pOpt->rcClip.bottom - pOpt->rcClip.top;
			DC.CreateFromDC32(hDC, cxBuf, cyBuf);
			SetWindowOrgEx(DC.GetDC(), pOpt->rcClip.left, pOpt->rcClip.top, nullptr);
		}
		else
		{
			cxBuf = prc->right - prc->left;
			cyBuf = prc->bottom - prc->top;
			DC.CreateFromDC32(hDC, cxBuf, cyBuf);
		}

		if (bClip)
		{
			DTBGOPTS RealOpt;
			RealOpt.dwSize = sizeof(DTBGOPTS);
			RealOpt.dwFlags = pOpt->dwFlags & ~DTBG_CLIPRECT;
			hr = s_pfnDrawThemeBackgroundEx(hTheme, DC.GetDC(),
				iPartId, iStateId, prc/*剪辑区左上角 - 窗口原点 == (0, 0)*/, &RealOpt);
		}
		else
		{
			const RECT rc{ 0,0,cxBuf,cyBuf };
			hr = s_pfnDrawThemeBackgroundEx(hTheme, DC.GetDC(),
				iPartId, iStateId, &rc, pOpt);
		}
		if (FAILED(hr))
			return hr;
		BITMAP bmp;
		GetObjectW(DC.GetBitmap(), sizeof(bmp), &bmp);
		GpBitmap* pBitmap;
		GdipCreateBitmapFromScan0(bmp.bmWidth, bmp.bmHeight, bmp.bmWidthBytes,
			PixelFormat32bppARGB, (BYTE*)bmp.bmBits, &pBitmap);
		GpImageAttributes* pIA;
		GdipCreateImageAttributes(&pIA);

		Gdiplus::ColorMatrix Mat;
		if (bInvert)
		{
			Mat =
			{
				-1, 0, 0, 0, 0,
				0, -1, 0, 0, 0,
				0, 0, -1, 0, 0,
				0, 0, 0, 1, 0,
				1 + fDelta, 1 + fDelta, 1 + fDelta, 0, 1
			};
		}
		else
		{
			Mat =
			{
				1, 0, 0, 0, 0,
				0, 1, 0, 0, 0,
				0, 0, 1, 0, 0,
				0, 0, 0, 1, 0,
				fDelta, fDelta, fDelta, 0, 1
			};
		}

		GdipSetImageAttributesColorMatrix(pIA, Gdiplus::ColorAdjustTypeDefault,
			TRUE, &Mat, nullptr, Gdiplus::ColorMatrixFlagsDefault);
		GpGraphics* pGraphics;
		GdipCreateFromHDC(hDC, &pGraphics);
		if (bClip)
			GdipDrawImageRectRectI(pGraphics, pBitmap,
				pOpt->rcClip.left,
				pOpt->rcClip.top,
				cxBuf, cyBuf, 0, 0, cxBuf, cyBuf,
				Gdiplus::UnitPixel, pIA, nullptr, nullptr);
		else
			GdipDrawImageRectRectI(pGraphics, pBitmap,
				prc->left,
				prc->top,
				cxBuf, cyBuf, 0, 0, cxBuf, cyBuf,
				Gdiplus::UnitPixel, pIA, nullptr, nullptr);
		GdipDeleteGraphics(pGraphics);
		GdipDisposeImage(pBitmap);
		GdipDisposeImageAttributes(pIA);
		return S_OK;
	}
	else
	{
		// 特判该路径有两个原因
		// 1. Ux绘制纯色背景时使用GDI，这导致结果位图中所有像素的Alpha通道为0，
		// 颜色矩阵需要修改以拉回透明度
		// 2. 调整一副位图的亮度显然比调整一个颜色值的亮度要慢得多，此处可作为一个优化
		COLORREF crFill;
		if (SUCCEEDED(hr = s_pfnGetThemeColor(hTheme, iPartId, iStateId,
			TMT_FILLCOLOR, &crFill)))
		{
			SetDCBrushColor(hDC, DeltaColorrefLuma(crFill, fDelta));
			if (bClip)
				FillRect(hDC, &pOpt->rcClip, GetStockBrush(DC_BRUSH));
			else
				FillRect(hDC, prc, GetStockBrush(DC_BRUSH));
			return S_OK;
		}
		return hr;
	}
}

// 取主题颜色，主要是文本颜色
static HRESULT UxfpGetThemeColor(const THEME_INFO& ti, const THREADCTX* ptc,
	HTHEME hTheme, int iPartId, int iStateId, int iPropId, _Out_ COLORREF& cr)
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
		case BP_COMMANDLINK:
			if (iPropId == TMT_TEXTCOLOR)
			{
				s_pfnGetThemeColor(hTheme, iPartId, iStateId, iPropId, &cr);
				if (iStateId == CMDLS_PRESSED)
					cr = AdjustColorrefLuma(cr, 500);
				else
					cr = AdjustColorrefLuma(cr, 160);
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
			cr = ptc->crBlue1;
			return S_OK;
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
		case LP_HYPERLINK:
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
		case TEXT_HYPERLINKTEXT:
			if (iPropId == TMT_TEXTCOLOR)
			{
				const auto hr = s_pfnGetThemeColor(hTheme, iPartId, iStateId, iPropId, &cr);
				cr = AdjustColorrefLuma(cr, 200);
				return hr;
			}
			break;
		}
		break;
	case ThemeType::ControlPanel:
		switch (iPartId)
		{
		case CPANEL_CONTENTPANE:
		case CPANEL_LARGECOMMANDAREA:
		case CPANEL_SMALLCOMMANDAREA:
			if (iPropId == TMT_FILLCOLOR)
			{
				cr = ptc->crDefBkg;
				return S_OK;
			}
			break;
		case CPANEL_BODYTEXT:
		case CPANEL_BODYTITLE:
		case CPANEL_CONTENTPANELABEL:
		case CPANEL_MESSAGETEXT:
			if (iPropId == TMT_TEXTCOLOR)
			{
				cr = ptc->crDefText;
				return S_OK;
			}
			break;
		case CPANEL_TITLE:
			if (iPropId == TMT_TEXTCOLOR)
			{
				cr = ptc->crBlue1;
				return S_OK;
			}
			break;
		case CPANEL_HELPLINK:
		case CPANEL_TASKLINK:
		case CPANEL_CONTENTLINK:
		case CPANEL_BUTTON:
			if (iPropId == TMT_TEXTCOLOR)
			{
				const auto hr = s_pfnGetThemeColor(hTheme, iPartId, iStateId, iPropId, &cr);
				cr = AdjustColorrefLuma(cr, 200);
				return hr;
			}
			break;
		}
		break;
	case ThemeType::MonthCalendar:
		if (iPropId == TMT_TEXTCOLOR)
		{
			switch (iPartId)
			{
			case MC_GRIDCELL:
				switch (iStateId)
				{
				case 0:
				case MCGC_HASSTATE:
				case MCGC_SELECTED:
					cr = ptc->crDefText;
					return S_OK;
				default:
					s_pfnGetThemeColor(hTheme, iPartId, iStateId, iPropId, &cr);
					cr = AdjustColorrefLuma(cr, 200);
					return S_OK;
				}
				break;
			case MC_GRIDCELLUPPER:
				switch (iStateId)
				{
				case 0:
				case MCGCU_SELECTED:
					cr = ptc->crDefText;
					return S_OK;
				case MCGC_HASSTATE:
					cr = ptc->crGray1;
					return S_OK;
				default:
					s_pfnGetThemeColor(hTheme, iPartId, iStateId, iPropId, &cr);
					cr = AdjustColorrefLuma(cr, 200);
					return S_OK;
				}
			}
		}
		break;
	case ThemeType::StatusBar:
		if (iPropId == TMT_TEXTCOLOR)
		{
			cr = ptc->crDefText;
			return S_OK;
		}
		break;
	case ThemeType::Menu:
		switch (iPartId)
		{
		case MENU_BARITEM:
			if (iPropId == TMT_TEXTCOLOR)
			{
				switch (iStateId)
				{
				case 0:
				case MBI_NORMAL:
				case MBI_HOT:
				case MBI_PUSHED:
					cr = ptc->crDefText;
					return S_OK;
				case MBI_DISABLED:
				case MBI_DISABLEDHOT:
				case MBI_DISABLEDPUSHED:
					cr = ptc->crGray1;
					return S_OK;
				}
			}
			break;
		}
		break;
	}
	return E_NOTIMPL;
}

// 绘制主题背景
static HRESULT UxfpDrawThemeBackground(const THEME_INFO& ti, const THREADCTX* ptc,
	HTHEME hTheme, HDC hDC, int iPartId, int iStateId,
	_In_ const RECT* prc, _In_opt_ const DTBGOPTS* pOptions)
{
	switch (ti.eType)
	{
	case ThemeType::TaskDialog:
		if (UxfpIsDarkTaskDialogAvailable())
			switch (iPartId)
			{
			case TDLG_SECONDARYPANEL:
				return s_pfnDrawThemeBackgroundEx(hTheme, hDC, TDLG_FOOTNOTEPANE, iStateId,
					prc, pOptions);
			case TDLG_FOOTNOTESEPARATOR:
				return UxfpAdjustLuma(hTheme, hDC, iPartId, iStateId, prc, pOptions, -0.67f);
			}
		else// Win10任务对话框(或整个DUser？)实际上使用ControlPanelStyle绘制主次面板
			switch (iPartId)
			{
			case TDLG_PRIMARYPANEL:
				return UxfpAdjustLuma(hTheme, hDC, iPartId, iStateId, prc, pOptions, -0.9f);
			case TDLG_MAININSTRUCTIONPANE:
			case TDLG_CONTENTPANE:
			case TDLG_EXPANDEDCONTENT:
			case TDLG_COMMANDLINKPANE:
			case TDLG_CONTROLPANE:
			case TDLG_FOOTNOTEAREA:
			case TDLG_FOOTNOTEPANE:
			case TDLG_EXPANDEDFOOTERAREA:
			case TDLG_SECONDARYPANEL:
				return UxfpAdjustLuma(hTheme, hDC, iPartId, iStateId, prc, pOptions, -0.79f);
			case TDLG_FOOTNOTESEPARATOR:
				return UxfpAdjustLuma(hTheme, hDC, iPartId, iStateId, prc, pOptions, -0.67f);
			}
		break;
	case ThemeType::Button:
		switch (iPartId)
		{
		case BP_COMMANDLINKGLYPH:
			return UxfpAdjustLuma(hTheme, hDC, iPartId, iStateId, prc, pOptions, 0.4f);
		case BP_COMMANDLINK:
			return UxfpAdjustLuma(hTheme, hDC, iPartId, iStateId, prc, pOptions, 0.9f);
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
				--NewOpt.rcClip.bottom;// 底部貌似有一条线，会遮挡一像素边框
				if (pOptions && pOptions->dwFlags & DTBG_CLIPRECT)
					IntersectRect(NewOpt.rcClip, NewOpt.rcClip, pOptions->rcClip);
				const auto hr = UxfpAdjustLuma(hTheme, hDC, iPartId, iStateId, prc, &NewOpt, f);
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
				return UxfpAdjustLuma(hTheme, hDC, iPartId, iStateId, prc, pOptions, f);
		}
		break;
		case TABP_PANE:
		case TABP_BODY:
		case TABP_AEROWIZARDBODY:
		{
			const auto hr = UxfpAdjustLuma(hTheme, hDC, iPartId, iStateId, prc, pOptions, -0.86f);
			if (hr == S_OK)// Exclude S_FALSE.
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
			return UxfpAdjustLuma(hTheme, hDC, iPartId, iStateId, prc, pOptions, 0.3f);
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
				if (pOptions && (pOptions->dwFlags & DTBG_CLIPRECT))
					FillRect(hDC, &pOptions->rcClip, GetStockBrush(DC_BRUSH));
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
			return UxfpAdjustLuma(hTheme, hDC, iPartId, iStateId, prc, pOptions, 0.7f);
		}
	}
	break;
	case ThemeType::AeroWizard:
		switch (iPartId)
		{
		case AW_HEADERAREA:
		case AW_CONTENTAREA:
		{
			SetDCBrushColor(hDC, ptc->crDefBkg);
			if (pOptions && pOptions->dwFlags & DTBG_CLIPRECT)
				FillRect(hDC, &pOptions->rcClip, GetStockBrush(DC_BRUSH));
			else
				FillRect(hDC, prc, GetStockBrush(DC_BRUSH));
		}
		return S_OK;
		case AW_COMMANDAREA:
		{
			SetDCBrushColor(hDC, ptc->crDefBtnFace);
			if (pOptions && pOptions->dwFlags & DTBG_CLIPRECT)
				FillRect(hDC, &pOptions->rcClip, GetStockBrush(DC_BRUSH));
			else
				FillRect(hDC, prc, GetStockBrush(DC_BRUSH));
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
					SetDCBrushColor(hDC, ptc->crDefBkg);
					if (pOptions && pOptions->dwFlags & DTBG_CLIPRECT)
						FillRect(hDC, &pOptions->rcClip, GetStockBrush(DC_BRUSH));
					else
						FillRect(hDC, prc, GetStockBrush(DC_BRUSH));
				}
				return S_OK;
			}
			break;
		case HP_HEADERDROPDOWNFILTER:
			return UxfpAdjustLuma(ti.hThemeExtra, hDC, iPartId, iStateId,
				prc, pOptions, iStateId != HDDFS_HOT ? 0.7f : -0.4f);
		case HP_HEADERSORTARROW:
			return UxfpAdjustLuma(ti.hThemeExtra, hDC, iPartId, iStateId,
				prc, pOptions, 0.f, TRUE);
		}
		break;
	case ThemeType::Progress:
		switch (iPartId)
		{
		case PP_BAR:
		case PP_BARVERT:
		case PP_TRANSPARENTBAR:
		case PP_TRANSPARENTBARVERT:
			return UxfpAdjustLuma(hTheme, hDC, iPartId, iStateId, prc, pOptions, -0.45f);
		case PP_PULSEOVERLAY:
		case PP_PULSEOVERLAYVERT:
			return UxfpAdjustLuma(hTheme, hDC, iPartId, iStateId, prc, pOptions, -0.2f);
		}
		break;
	case ThemeType::ControlPanel:
		switch (iPartId)
		{
		case CPANEL_CONTENTPANE:
			SetDCBrushColor(hDC, ptc->crDefBkg);
			if (pOptions && pOptions->dwFlags & DTBG_CLIPRECT)
				FillRect(hDC, &pOptions->rcClip, GetStockBrush(DC_BRUSH));
			else
				FillRect(hDC, prc, GetStockBrush(DC_BRUSH));
			return S_OK;
		case CPANEL_LARGECOMMANDAREA:
		case CPANEL_SMALLCOMMANDAREA:
			return UxfpAdjustLuma(hTheme, hDC, iPartId, iStateId, prc, pOptions, -0.79f);
		}
		break;
	case ThemeType::MonthCalendar:
		switch (iPartId)
		{
		case MC_BACKGROUND:
		case MC_GRIDBACKGROUND:
			SetDCBrushColor(hDC, ptc->crDefBkg);
			if (pOptions && pOptions->dwFlags & DTBG_CLIPRECT)
				FillRect(hDC, &pOptions->rcClip, GetStockBrush(DC_BRUSH));
			else
				FillRect(hDC, prc, GetStockBrush(DC_BRUSH));
			return S_OK;
		case MC_GRIDCELLBACKGROUND:
			return UxfpAdjustLuma(hTheme, hDC, iPartId, iStateId, prc, pOptions, 0.5f);
		case MC_NAVNEXT:
		case MC_NAVPREV:
			return UxfpAdjustLuma(hTheme, hDC, iPartId, iStateId, prc, pOptions, 0.7f);
		}
		break;
	case ThemeType::StatusBar:
		switch (iPartId)
		{
		case STATUSBAR_PART_COMMON:
			return UxfpAdjustLuma(hTheme, hDC, iPartId, iStateId, prc, pOptions, 0.1f, TRUE);
		case STATUSBAR_PART_PANE:
			return UxfpAdjustLuma(hTheme, hDC, iPartId, iStateId, prc, pOptions, 0.1f, TRUE);
		}
		break;
	case ThemeType::Menu:
		switch (iPartId)
		{
		case MENU_BARBACKGROUND:
			SetDCBrushColor(hDC, ptc->crDefBkg);
			if (pOptions && (pOptions->dwFlags & DTBG_CLIPRECT))
				FillRect(hDC, &pOptions->rcClip, GetStockBrush(DC_BRUSH));
			else
				FillRect(hDC, prc, GetStockBrush(DC_BRUSH));
			return S_OK;
		case MENU_BARITEM:
			SetDCBrushColor(hDC, ptc->crDefBtnFace);
			if (pOptions && (pOptions->dwFlags & DTBG_CLIPRECT))
				FillRect(hDC, &pOptions->rcClip, GetStockBrush(DC_BRUSH));
			else
				FillRect(hDC, prc, GetStockBrush(DC_BRUSH));
			return S_OK;
		}
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
	if (TcsEqualI(pszClassList, L"ScrollBar"))
		return s_pfnOpenNcThemeData(nullptr, L"Explorer::ScrollBar");
	else ECKLIKELY
		return s_pfnOpenNcThemeData(hWnd, pszClassList);
}

static HTHEME WINAPI NewOpenThemeData(HWND hWnd, PCWSTR pszClassList)
{
	HTHEME hTheme;
	if (ShouldAppsUseDarkMode())
	{
		if (TcsEqualI(pszClassList, L"Combobox"))
			hTheme = s_pfnOpenThemeData(hWnd, L"DarkMode_CFD::Combobox");
		else if (TcsEqualI(pszClassList, L"Edit"))
			hTheme = s_pfnOpenThemeData(hWnd, L"DarkMode_CFD::Edit");
		else if (UxfpIsDarkTaskDialogAvailable() &&
			EckIsStartWithConstStringIW(pszClassList, L"TaskDialog"))
			hTheme = s_pfnOpenThemeData(hWnd, L"DarkMode_Explorer::TaskDialog");
		else
			hTheme = s_pfnOpenThemeData(hWnd, pszClassList);
	}
	else
		hTheme = s_pfnOpenThemeData(hWnd, pszClassList);
	UxfpOnThemeOpen(hWnd, hTheme, pszClassList);
	return hTheme;
}

static HTHEME WINAPI NewOpenThemeDataForDpi(HWND hWnd, PCWSTR pszClassList, UINT uDpi)
{
	HTHEME hTheme;
	if (ShouldAppsUseDarkMode())
	{
		if (TcsEqualI(pszClassList, L"Combobox"))
			hTheme = s_pfnOpenThemeDataForDpi(hWnd, L"DarkMode_CFD::Combobox", uDpi);
		else if (TcsEqualI(pszClassList, L"Edit"))
			hTheme = s_pfnOpenThemeDataForDpi(hWnd, L"DarkMode_CFD::Edit", uDpi);
		else if (UxfpIsDarkTaskDialogAvailable() &&
			EckIsStartWithConstStringIW(pszClassList, L"TaskDialog"))
			hTheme = s_pfnOpenThemeDataForDpi(hWnd, L"DarkMode_Explorer::TaskDialog", uDpi);
		else
			hTheme = s_pfnOpenThemeDataForDpi(hWnd, pszClassList, uDpi);
	}
	else
		hTheme = s_pfnOpenThemeDataForDpi(hWnd, pszClassList, uDpi);
	UxfpOnThemeOpen(hWnd, hTheme, pszClassList);
	return hTheme;
}

static HRESULT WINAPI NewDrawThemeText(HTHEME hTheme, HDC hDC, int iPartId, int iStateId,
	PCWSTR pszText, int cchText, DWORD dwTextFlags, DWORD dwTextFlags2, const RECT* pRect)
{
	if (ShouldAppsUseDarkMode() && !(dwTextFlags & DT_CALCRECT))
	{
		COLORREF cr;
		if (SUCCEEDED(UxfpGetThemeColor(UxfpGetThemeInfo(hTheme), GetThreadCtx(),
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
			if (SUCCEEDED(UxfpGetThemeColor(UxfpGetThemeInfo(hTheme), GetThreadCtx(),
				hTheme, iPartId, iStateId,
				(pOptions && (pOptions->dwFlags & DTT_COLORPROP)) ?
				pOptions->iColorPropId : TMT_TEXTCOLOR, NewOpt.crText)))
			{
				if (pOptions)
				{
					const auto t = NewOpt.crText;
					NewOpt = *pOptions;
					NewOpt.dwFlags |= DTT_TEXTCOLOR;
					NewOpt.dwFlags &= ~DTT_COLORPROP;
					NewOpt.crText = t;
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
		if (SUCCEEDED(UxfpDrawThemeBackground(UxfpGetThemeInfo(hTheme), GetThreadCtx(),
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
		if (SUCCEEDED(UxfpDrawThemeBackground(UxfpGetThemeInfo(hTheme), GetThreadCtx(),
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
		if (SUCCEEDED(UxfpGetThemeColor(UxfpGetThemeInfo(hTheme), GetThreadCtx(),
			hTheme, iPartId, iStateId, iPropId, *pColor)))
			return S_OK;
	}
	return s_pfnGetThemeColor(hTheme, iPartId, iStateId, iPropId, pColor);
}

static HRESULT WINAPI NewCloseThemeData(HTHEME hTheme)
{
	UxfpOnThemeClose(hTheme);
	return s_pfnCloseThemeData(hTheme);
}

// 修正表头溢出按钮大小
static HRESULT WINAPI NewGetThemePartSize(HTHEME hTheme, HDC hDC, int iPartId, int iStateId,
	const RECT* prc, THEMESIZE eSize, SIZE* psz)
{
	const auto& ti = UxfpGetThemeInfo(hTheme);
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

static int WINAPI NewSoftModalMessageBox(void* p)
{
	const auto ptc = GetThreadCtx();
	if (!ptc)
		return s_pfnSoftModalMessageBox(p);
	CMsgBoxHook Mb{};
	BeginCbtHook(&Mb);
	return s_pfnSoftModalMessageBox(p);
}
#endif// !ECK_OPT_NO_DARKMODE
#pragma endregion UxFixer

#pragma region ScrollBarHook
#if ECK_SBHOOK_INCLUDED
using FSetScrollInfo = int(WINAPI*)(HWND, int, const SCROLLINFO*, BOOL);
using FGetScrollInfo = BOOL(WINAPI*)(HWND, int, SCROLLINFO*);
using FShowScrollBar = BOOL(WINAPI*)(HWND, int, BOOL);

static FSetScrollInfo s_pfnSetScrollInfo{ SetScrollInfo };
static FGetScrollInfo s_pfnGetScrollInfo{ GetScrollInfo };
static FShowScrollBar s_pfnShowScrollBar{ ShowScrollBar };

static int WINAPI NewSetScrollInfo(HWND hWnd, int nBar, const SCROLLINFO* psi, BOOL bRedraw)
{
	const auto pThis = (CScrollBarHook*)GetPropW(hWnd, PROP_SBHOOK);
	if (pThis)
		return pThis->HkSetScrollInfo(hWnd, nBar, psi, bRedraw);
	return s_pfnSetScrollInfo(hWnd, nBar, psi, bRedraw);
}
static BOOL WINAPI NewGetScrollInfo(HWND hWnd, int nBar, SCROLLINFO* psi)
{
	const auto pThis = (CScrollBarHook*)GetPropW(hWnd, PROP_SBHOOK);
	if (pThis)
		return pThis->HkGetScrollInfo(hWnd, nBar, psi);
	return s_pfnGetScrollInfo(hWnd, nBar, psi);
}
static BOOL WINAPI NewShowScrollBar(HWND hWnd, int nBar, BOOL bShow)
{
	const auto pThis = (CScrollBarHook*)GetPropW(hWnd, PROP_SBHOOK);
	if (pThis)
		return pThis->HkShowScrollBar(hWnd, nBar, bShow);
	return s_pfnShowScrollBar(hWnd, nBar, bShow);
}
#endif// ECK_SBHOOK_INCLUDED
#pragma endregion ScrollBarHook

#pragma region Init
constexpr static PCWSTR c_szErrInitStatus[]
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

	g_hInstance = hInstance;
	RtlGetNtVersionNumbers(&g_NtVer.uMajor, &g_NtVer.uMinor, &g_NtVer.uBuild);
	InitPrivateApi();
	g_dwTlsSlot = TlsAlloc();
	if (!(pInitParam->uFlags & EIF_NOINITTHREAD))
		ThreadInit();
	//////////////初始化GDI+
	if (!(pInitParam->uFlags & EIF_NOINITGDIPLUS))
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
#if !ECK_OPT_NO_DX
	if (!(pInitParam->uFlags & EIF_NOINITD2D))
	{
#ifdef _DEBUG
		D2D1_FACTORY_OPTIONS D2DFactoryOptions;
		D2DFactoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
		//D2DFactoryOptions.debugLevel = D2D1_DEBUG_LEVEL_WARNING;
		hr = D2D1CreateFactory(pInitParam->uD2dFactoryType,
			__uuidof(ID2D1Factory1), &D2DFactoryOptions, (void**)&g_pD2dFactory);
#else
		hr = D2D1CreateFactory(pInitParam->uD2dFactoryType, IID_PPV_ARGS(&g_pD2dFactory));
#endif // _DEBUG
		if (FAILED(hr))
		{
			*pdwErrCode = hr;
			EckDbgPrintFormatMessage(hr);
			return InitStatus::D2dFactoryError;
		}
		//////////////创建DXGI工厂
		hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
			D3D11_CREATE_DEVICE_BGRA_SUPPORT
#ifndef NDEBUG
			| D3D11_CREATE_DEVICE_DEBUG
#endif // !NDEBUG
			, pInitParam->pD3dFeatureLevel, pInitParam->cD3dFeatureLevel,
			D3D11_SDK_VERSION, &g_pD3d11Device, nullptr, nullptr);
		if (FAILED(hr))
		{
			*pdwErrCode = hr;
			EckDbgPrintFormatMessage(hr);
			return InitStatus::D3dDeviceError;
		}
		g_pD3d11Device->QueryInterface(&g_pDxgiDevice);
		if (FAILED(hr) || !g_pDxgiDevice)
		{
			*pdwErrCode = hr;
			EckDbgPrintFormatMessage(hr);
			return InitStatus::DxgiDeviceError;
		}

		ComPtr<IDXGIAdapter> pDxgiAdapter;
		g_pDxgiDevice->GetAdapter(&pDxgiAdapter);
		pDxgiAdapter->GetParent(IID_PPV_ARGS(&g_pDxgiFactory));
		//////////////创建DXGI设备
		hr = g_pD2dFactory->CreateDevice(g_pDxgiDevice, &g_pD2dDevice);
		if (FAILED(hr))
		{
			*pdwErrCode = hr;
			EckDbgPrintFormatMessage(hr);
			return InitStatus::DxgiDeviceError;
		}
		//////////////获取调试接口
#ifdef _DEBUG
		const auto hModDxgiDbg = GetModuleHandleW(L"dxgidebug.dll");
		if (hModDxgiDbg)
		{
			using FDXGIGetDebugInterface = HRESULT(WINAPI*)(REFIID, void**);
			const auto pfnDXGIGetDebugInterface =
				(FDXGIGetDebugInterface)GetProcAddress(hModDxgiDbg,
					"DXGIGetDebugInterface");
			if (pfnDXGIGetDebugInterface)
				pfnDXGIGetDebugInterface(IID_PPV_ARGS(&g_pDxgiDebug));
		}
#endif // _DEBUG
	}
#endif// !ECK_OPT_NO_DX
	//////////////创建DWrite工厂
	if (!(pInitParam->uFlags & EIF_NOINITDWRITE))
	{
		hr = DWriteCreateFactory(pInitParam->uDWriteFactoryType,
			__uuidof(IDWriteFactory), (IUnknown**)&g_pDwFactory);
		if (FAILED(hr))
		{
			*pdwErrCode = hr;
			EckDbgPrintFormatMessage(hr);
			return InitStatus::DWriteFactoryError;
		}
	}
	//////////////创建WIC工厂
	if (!(pInitParam->uFlags & EIF_NOINITWIC))
	{
		hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr,
			CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&g_pWicFactory));
		if (FAILED(hr))
		{
			*pdwErrCode = hr;
			EckDbgPrintFormatMessage(hr);
			return InitStatus::WicFactoryError;
		}
	}

#if !ECK_OPT_NO_DARKMODE
	if (!(pInitParam->uFlags & EIF_NODARKMODE) &&
		g_NtVer.uMajor >= 10 && g_NtVer.uBuild >= WINVER_1809)
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
		if (s_pfnOpenThemeDataForDpi = (FOpenThemeDataForDpi)
			GetProcAddress(hModUx, "OpenThemeDataForDpi"))
			DetourAttach(&s_pfnOpenThemeDataForDpi, NewOpenThemeDataForDpi);
		FreeLibrary(hModUx);
		DetourAttach(&s_pfnDrawThemeBackgroundEx, NewDrawThemeBackgroundEx);
		DetourAttach(&s_pfnDrawThemeBackground, NewDrawThemeBackground);
		DetourAttach(&s_pfnGetThemeColor, NewGetThemeColor);
		DetourAttach(&s_pfnCloseThemeData, NewCloseThemeData);
		DetourAttach(&s_pfnDrawThemeParentBackground, NewDrawThemeParentBackground);
		DetourAttach(&s_pfnGetThemePartSize, NewGetThemePartSize);
		const HMODULE hModUser32 = GetModuleHandleW(L"user32.dll");
		EckAssert(hModUser32);
		s_pfnSoftModalMessageBox = (FSoftModalMessageBox)
			GetProcAddress(hModUser32, "SoftModalMessageBox");
		if (s_pfnSoftModalMessageBox)
			DetourAttach(&s_pfnSoftModalMessageBox, NewSoftModalMessageBox);
		DetourTransactionCommit();
	}
#endif// !ECK_OPT_NO_DARKMODE

#if ECK_SBHOOK_INCLUDED
	DetourTransactionBegin();
	DetourAttach(&s_pfnSetScrollInfo, NewSetScrollInfo);
	DetourAttach(&s_pfnGetScrollInfo, NewGetScrollInfo);
	DetourAttach(&s_pfnShowScrollBar, NewShowScrollBar);
	DetourTransactionCommit();
#endif // ECK_SBHOOK_INCLUDED
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
	SafeReleaseAssert0(g_pWicFactory);
#if !ECK_OPT_NO_DX
	SafeRelease(g_pDwFactory);
	SafeRelease(g_pD3d11Device);
	SafeRelease(g_pD2dFactory);
	SafeRelease(g_pD2dDevice);
	SafeRelease(g_pDxgiDevice);
	SafeRelease(g_pDxgiFactory);
#endif// !ECK_OPT_NO_DX
}
#pragma endregion Init

#pragma region Thread
DWORD GetThreadCtxTlsSlot()
{
	return g_dwTlsSlot;
}

void ThreadInit()
{
	if ((SIZE_T)g_hModComCtl32 == SIZETMax)
		g_hModComCtl32 = GetModuleHandleW(L"comctl32.dll");
	EckAssert(!TlsGetValue(GetThreadCtxTlsSlot()));
	const auto p = new THREADCTX{};
	TlsSetValue(GetThreadCtxTlsSlot(), p);
	p->UpdateDefColor();
#if ECK_OPT_NO_DARKMODE
	p->bEnableDarkModeHook = FALSE;
#else
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
						if (it == c_pszCommCtrlCls + 1/*Header*/ ||
							it == c_pszCommCtrlCls + 2/*ListView*/)
							SetWindowTheme(hWnd, L"ItemsView", nullptr);
						else ECKLIKELY
							SetWindowTheme(hWnd, L"Explorer", nullptr);
					}
				}
				return lResult;
			}

			return CallNextHookEx(p->hhkCbtDarkMode, iCode, wParam, lParam);
		}, nullptr, NtCurrentThreadId32());
#endif // ECK_OPT_NO_DARKMODE

	p->hhkMsgFilter = SetWindowsHookExW(WH_MSGFILTER, [](int iCode, WPARAM wParam, LPARAM lParam)->LRESULT
		{
			const auto pCtx = GetThreadCtx();
			pCtx->DoCallback();
			return CallNextHookEx(pCtx->hhkMsgFilter, iCode, wParam, lParam);
		}, nullptr, NtCurrentThreadId32());
}

void ThreadUnInit()
{
	EckAssert(TlsGetValue(GetThreadCtxTlsSlot()));
	const auto p = (THREADCTX*)TlsGetValue(GetThreadCtxTlsSlot());
#ifdef _DEBUG
	if (!p->hmWnd.empty())
	{
		EckDbgPrintWithPos(L"** WARNING ** 反初始化线程上下文时发现窗口映射表不为空");
		EckDbgBreak();
	}
#endif // _DEBUG
	UnhookWindowsHookEx(p->hhkTempCBT);
	UnhookWindowsHookEx(p->hhkCbtDarkMode);
	UnhookWindowsHookEx(p->hhkMsgFilter);
	delete p;
	TlsSetValue(GetThreadCtxTlsSlot(), nullptr);
}

void THREADCTX::SetNcDarkModeForAllTopWnd(BOOL bDark)
{
#if !ECK_OPT_NO_DARKMODE
	for (const auto& e : hmTopWnd)
		EnableWindowNcDarkMode(e.first, bDark);
#endif // !ECK_OPT_NO_DARKMODE
}

void THREADCTX::UpdateDefColor()
{
	const auto bDark = GetItemsViewForeBackColor(crDefText, crDefBkg);
	crDefBtnFace = (bDark ? 0x303030 : GetSysColor(COLOR_BTNFACE));
	crBlue1 = (bDark ? RGB(0, 168, 255) : RGB(0, 51, 153));
	crGray1 = (bDark ? RGB(180, 180, 180) : GetSysColor(COLOR_GRAYTEXT));
	crTip1 = (bDark ? RGB(131, 162, 198) : RGB(97, 116, 139));
	HIGHCONTRASTW hc{ sizeof(HIGHCONTRASTW) };
	if (SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(HIGHCONTRASTW), &hc, FALSE) &&
		(hc.dwFlags & HCF_HIGHCONTRASTON))
		crHiLightText = GetSysColor(COLOR_HIGHLIGHTTEXT);
	else
		crHiLightText = crDefText;
}

void THREADCTX::SendThemeChangedToAllTopWindow()
{
#if !ECK_OPT_NO_DARKMODE
	for (const auto& [hWnd, pWnd] : hmTopWnd)
	{
		// WM_THEMECHANGED
		// wParam
		// 改变的部分，分为高低WORD两个代码，-1表示所有，0似乎是无效值。
		// DUser借助此进行主题句柄缓存，见
		// comctl32!DirectUI::ThemeHandleCache::FlushHandles，
		// 伪代码：
		// static WPARAM s_wParam{};
		// if (wParam == -1 || wParam == s_wParam) {
		//	s_wParam = wParam;
		//	无效化缓存();
		// }
		// 
		// lParam - 一组位标志
		if (GetWindowLongPtrW(hWnd, GWL_STYLE) & WS_VISIBLE)
		{
			SendMessageW(hWnd, WM_SETREDRAW, FALSE, 0);
			SendMessageW(hWnd, WM_THEMECHANGED, -1, 0);
			BroadcastChildrenMessage(hWnd, WM_THEMECHANGED, -1, 0);
			SendMessageW(hWnd, WM_SETREDRAW, TRUE, 0);
			RedrawWindow(hWnd, nullptr, nullptr,
				RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
		}
		else
		{
			SendMessageW(hWnd, WM_THEMECHANGED, -1, 0);
			BroadcastChildrenMessage(hWnd, WM_THEMECHANGED, -1, 0);
		}
	}
#endif // !ECK_OPT_NO_DARKMODE
}

void THREADCTX::DoCallback()
{
	if (bEnterCallback)
		return;
	RtlAcquireSRWLockExclusive(&Callback.Lk);
	EckLoop()
	{
		if (Callback.q.empty())
			break;
		const auto Top{ std::move(Callback.q.front()) };
		Callback.UnlockedDeQueue();
		RtlReleaseSRWLockExclusive(&Callback.Lk);
		bEnterCallback = TRUE;
		if (Top.Callback.index() == 0)
			std::get<0>(Top.Callback)();
		else
			std::coroutine_handle<>::from_address(std::get<1>(Top.Callback)).resume();
		bEnterCallback = FALSE;
		RtlAcquireSRWLockExclusive(&Callback.Lk);
	}
	RtlReleaseSRWLockExclusive(&Callback.Lk);
}
#pragma endregion Thread

#pragma region Wnd
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
				// 执行CWnd初始化
				EckAssert(!pCtx->pCurrWnd->m_hWnd);
				pCtx->pCurrWnd->m_pfnRealProc =
					SetWindowProc((HWND)wParam, CWnd::EckWndProc);
				pCtx->pCurrWnd->m_hWnd = (HWND)wParam;
				// 插入窗口映射
				pCtx->WmAdd((HWND)wParam, pCtx->pCurrWnd);
				// 插入顶级窗口映射
				if (!IsBitSet(pcbtcw->lpcs->style, WS_CHILD))
					pCtx->TwmAdd((HWND)wParam, pCtx->pCurrWnd);
				// 执行用户回调
				if (pCtx->pfnWndCreatingProc)
					pCtx->pfnWndCreatingProc((HWND)wParam, pcbtcw, pCtx);
				// 立即卸载钩子
				EndCbtHook();
			}
			return CallNextHookEx(pCtx->hhkTempCBT, iCode, wParam, lParam);
		}, nullptr, NtCurrentThreadId32());
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
	HWND hWnd = Msg.hwnd;
	CWnd* pWnd;
	const auto pCtx = GetThreadCtx();
	pCtx->DoCallback();

	while (hWnd)
	{
		pWnd = pCtx->WmAt(hWnd);
		if (pWnd && pWnd->PreTranslateMessage(Msg))
			return TRUE;
		hWnd = GetParent(hWnd);
	}
	return FALSE;
}
#pragma endregion Wnd

#pragma region Dbg
void DbgPrintWndMap()
{
#ifdef _DEBUG
	const auto* const pCtx = GetThreadCtx();
	auto s = Format(L"当前线程（TID = %u）窗口映射表内容：\n", NtCurrentThreadId32());
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
#endif // _DEBUG
}

void DbgPrintFmt(_Printf_format_string_ PCWSTR pszFormat, ...)
{
#ifdef _DEBUG
	va_list vl;
	va_start(vl, pszFormat);
	CRefStrW rs{};
	rs.FormatV(pszFormat, vl);
	DbgPrint(rs);
	va_end(vl);
#endif // _DEBUG
}

void DbgPrintWithPos(PCWSTR pszFile, PCWSTR pszFunc, int iLine, PCWSTR pszMsg)
{
#ifdef _DEBUG
	DbgPrint(Format(L"%s(%u) -> %s\n%s\n", pszFile, iLine, pszFunc, pszMsg));
#endif // _DEBUG
}

void Assert(PCWSTR pszMsg, PCWSTR pszFile, PCWSTR pszLine)
{
#ifdef _DEBUG
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
	const auto rsContent = Format(L"程序位置：%s\n\n源文件：%s\n\n行号：%s\n\n测试表达式：%s",
		g_rsCurrDir.Data(), pszFile, pszLine, pszMsg);
	tdc.pszContent = rsContent.Data();
	tdc.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_USE_COMMAND_LINKS;

	int nBtn, Dummy;
	TaskDialogIndirect(&tdc, &nBtn, &Dummy, &Dummy);

	switch (nBtn)
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
#endif // _DEBUG
}
#pragma endregion Dbg
ECK_NAMESPACE_END

#if !ECK_OPT_NO_YYJSON
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
#endif // !ECK_OPT_NO_YYJSON

#if !ECK_OPT_NO_PUGIXML
#include "PugiXml/pugixml.cpp"
#endif // !ECK_OPT_NO_PUGIXML