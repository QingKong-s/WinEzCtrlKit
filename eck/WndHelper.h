#pragma once
#include "Utility.h"
#include "DpiApi.h"
#include "ComPtr.h"
#include "NativeWrapper.h"

#include <vssym32.h>
#include <ShellScalingApi.h>

#define ECK_DS_BEGIN(StructName)		struct StructName {
#define ECK_DS_END_VAR(VarName)			} VarName{};
#define ECK_DS_END()					};
#define ECK_DS_ENTRY(Name, Size)		const int o_##Name = Size; int Name = Size;
#define ECK_DS_ENTRY_DYN(Name, Size)	int	o_##Name = Size; int Name = Size;
#define ECK_DS_ENTRY_F(Name, Size)		const float o_##Name = Size; float Name = Size;
#define ECK_DS_ENTRY_F_DYN(Name, Size)	float o_##Name = Size; float Name = Size;

#define ECK_HANDLE_WM_MOUSELEAVE(hWnd, wParam, lParam, fn) \
    ((fn)((hWnd)), 0L)
#define ECK_HANDLE_WM_DPICHANGED(hWnd, wParam, lParam, fn) \
    ((fn)((hWnd), (int)(short)LOWORD(wParam), (int)(short)HIWORD(wParam), (RECT*)(lParam)), 0L)

#define ECK_STYLE_GETSET(Name, Style)					\
    BOOL StyleGet##Name() const							\
    {													\
        if constexpr (Style == 0)						\
            return !GetStyle();							\
        else											\
            return IsBitSet(GetStyle(), Style);			\
    }													\
    void StyleSet##Name(BOOL b)	const					\
    {													\
        ModifyStyle((b ? Style : 0), Style, GWL_STYLE); \
    }

#define ECK_STYLE_GETSET_MASK(Name, Style, Mask)		\
    BOOL StyleGet##Name() const							\
    {													\
        if constexpr (Style == 0)						\
            return !(GetStyle() & Mask);				\
        else											\
            return IsBitSet(GetStyle(), Style);			\
    }													\
    void StyleSet##Name(BOOL b)	const					\
    {													\
        SetStyle((GetStyle() & ~Mask) | (b ? Style : 0));\
    }

#define ECK_CWNDPROP_STYLE(Name, Style)					\
    ECKPROP(StyleGet##Name, StyleSet##Name) BOOL Name;	\
    ECK_STYLE_GETSET(Name, Style)

#define ECK_CWNDPROP_STYLE_MASK(Name, Style, Mask)		\
    ECKPROP(StyleGet##Name, StyleSet##Name) BOOL Name;	\
    ECK_STYLE_GETSET_MASK(Name, Style, Mask)

ECK_NAMESPACE_BEGIN
#pragma region DPI
inline int GetDpi(HWND hWnd) noexcept
{
#if ECK_OPT_DYN_NF
    if (hWnd)
    {
        if (g_pfnGetDpiForWindow)
            return (int)g_pfnGetDpiForWindow(hWnd);
    }
    else
    {
        if (g_pfnGetDpiForSystem)
            return (int)g_pfnGetDpiForSystem();
    }
    HDC hDC = GetDC(hWnd);
    int i = GetDeviceCaps(hDC, LOGPIXELSX);
    ReleaseDC(hWnd, hDC);
    return i;
#else
#  if ECKDPIAPI
    if (hWnd)
        return (int)GetDpiForWindow(hWnd);
    else
        return (int)GetDpiForSystem();
#  else
    HDC hDC = GetDC(hWnd);
    int i = GetDeviceCaps(hDC, LOGPIXELSX);
    ReleaseDC(hWnd, hDC);
    return i;
#  endif// ECKDPIAPI
#endif// ECK_OPT_DYN_NF
}

EckInline int GetMonitorDpi(HMONITOR hMonitor) noexcept
{
#if NTDDI_VERSION >= NTDDI_WINBLUE
    UINT xDpi, yDpi;
    if (SUCCEEDED(GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &xDpi, &yDpi)))
        return (int)xDpi;
    else
        return USER_DEFAULT_SCREEN_DPI;
#else
    const auto hIC = CreateICW(L"DISPLAY", nullptr, nullptr, nullptr);
    const int i = GetDeviceCaps(hIC, LOGPIXELSX);
    DeleteDC(hIC);
    return i;
#endif
}
#pragma endregion DPI

#pragma region Font
EckInline HFONT EzFont(
    int iDpi,
    PCWSTR pszFontName,
    int iPoint,
    int iWeight = 400,
    BOOL bItalic = FALSE,
    BOOL bUnderline = FALSE,
    BOOL bStrikeOut = FALSE,
    DWORD dwCharSet = DEFAULT_CHARSET) noexcept
{
    return CreateFontW(-iPoint * iDpi / 72, 0, 0, 0, iWeight, bItalic, bUnderline,
        bStrikeOut, dwCharSet, 0, 0, 0, 0, pszFontName);
}

// 函数枚举子窗口并悉数设置字体
EckInline void ApplyWindowFont(HWND hWnd, HFONT hFont, BOOL bRedraw = FALSE) noexcept
{
    EnumChildWindows(hWnd, [](HWND hWnd, LPARAM lParam) -> BOOL
        {
            SendMessageW(hWnd, WM_SETFONT, lParam, FALSE);
            return TRUE;
        }, (LPARAM)hFont);
    if (bRedraw)
        RedrawWindow(hWnd, nullptr, nullptr, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW);
}

EckNfInlineNd HFONT ReCreateFontForDpiChanged(HFONT hFont,
    int iDpiNew, int iDpiOld, BOOL bDeletePrevFont = FALSE) noexcept
{
    LOGFONTW lf;
    GetObjectW(hFont, sizeof(lf), &lf);
    if (bDeletePrevFont)
        DeleteObject(hFont);
    lf.lfHeight = DpiScale(lf.lfHeight, iDpiNew, iDpiOld);
    return CreateFontIndirectW(&lf);
}

inline void ReCreateAndApplyFontForDpiChanged(
    HWND hWnd, _Inout_ HFONT& hFontVar, int iDpiNew, int iDpiOld) noexcept
{
    auto hFont = ReCreateFontForDpiChanged(hFontVar, iDpiNew, iDpiOld);
    ApplyWindowFont(hWnd, hFont, FALSE);
    std::swap(hFontVar, hFont);
    DeleteObject(hFont);
}
#pragma endregion Font

#pragma region Wrapper
EckInline DWORD ModifyWindowStyle(HWND hWnd, DWORD dwNew, DWORD dwMask, int idx = GWL_STYLE) noexcept
{
    DWORD dwStyle = (DWORD)GetWindowLongPtrW(hWnd, idx);
    dwStyle &= ~dwMask;
    dwStyle |= dwNew;
    SetWindowLongPtrW(hWnd, idx, dwStyle);
    return dwStyle;
}

// 通过设置图像列表来设置ListView行高，已弃用
EckInline HIMAGELIST LVSetItemHeight(HWND hLV, int cy) noexcept
{
    return (HIMAGELIST)SendMessageW(hLV, LVM_SETIMAGELIST,
        LVSIL_SMALL, (LPARAM)ImageList_Create(1, (cy), 0, 1, 0));
}

EckInline BOOL BitBltPs(const PAINTSTRUCT* pps, HDC hdcSrc) noexcept
{
    return BitBlt(pps->hdc,
        pps->rcPaint.left,
        pps->rcPaint.top,
        pps->rcPaint.right - pps->rcPaint.left,
        pps->rcPaint.bottom - pps->rcPaint.top,
        hdcSrc,
        pps->rcPaint.left,
        pps->rcPaint.top,
        SRCCOPY);
}
EckInline BOOL BitBltPs(const PAINTSTRUCT& pps, HDC hdcSrc) noexcept
{
    return BitBltPs(&pps, hdcSrc);
}

EckInline WNDPROC SetWindowProcedure(HWND hWnd, WNDPROC pfnWndProc) noexcept
{
    return (WNDPROC)SetWindowLongPtrW(hWnd, GWLP_WNDPROC, (LONG_PTR)pfnWndProc);
}

#if NTDDI_VERSION < NTDDI_WIN10_NI// 22621 SDK将NTDDI_VERSION设置为NTDDI_WIN10_NI
EckInline HRESULT EnableWindowMica(HWND hWnd, UINT uType = 2) noexcept
{
    return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
}
#else
EckInline HRESULT EnableWindowMica(HWND hWnd,
    DWM_SYSTEMBACKDROP_TYPE uType = DWMSBT_MAINWINDOW) noexcept
{
    if (g_NtVer.uBuild >= 22621)
        return DwmSetWindowAttribute(hWnd, DWMWA_SYSTEMBACKDROP_TYPE, &uType, sizeof(uType));
    else
        return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
}
#endif// NTDDI_VERSION < NTDDI_WIN10_NI

EckInline WNDPROC GetClassWindowProcedure(HINSTANCE hInstance, PCWSTR pszClass) noexcept
{
    WNDCLASSEXW wcex{ sizeof(wcex) };
    GetClassInfoExW(hInstance, pszClass, &wcex);
    return wcex.lpfnWndProc;
}

inline ATOM RegisterWindowClass(PCWSTR pszClass,
    UINT uStyle = CS_STDWND, HBRUSH hbrBK = nullptr) noexcept
{
    WNDCLASSW wc{};
    wc.cbWndExtra = sizeof(void*);
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hInstance = g_hInstance;
    wc.lpfnWndProc = DefWindowProcW;
    wc.lpszClassName = pszClass;
    wc.style = uStyle;
    wc.hbrBackground = hbrBK;
    return RegisterClassW(&wc);
}

EckInline void BroadcastChildrenMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept
{
    const MSG msg{ nullptr,uMsg,wParam,lParam };
    EnumChildWindows(hWnd, [](HWND hWnd, LPARAM lParam)->BOOL
        {
            const auto pMsg = (const MSG*)lParam;
            SendMessageW(hWnd, pMsg->message, pMsg->wParam, pMsg->lParam);
            return TRUE;
        }, (LPARAM)&msg);
}

EckInline void ScreenToClient(HWND hWnd, _Inout_ RECT* prc) noexcept
{
    ::ScreenToClient(hWnd, (POINT*)prc);
    ::ScreenToClient(hWnd, ((POINT*)prc) + 1);
}
EckInline void ClientToScreen(HWND hWnd, _Inout_ RECT* prc) noexcept
{
    ::ClientToScreen(hWnd, (POINT*)prc);
    ::ClientToScreen(hWnd, ((POINT*)prc) + 1);
}

// WM_PAINT和WM_PRINTCLIENT的合并处理
EckInline HDC BeginPaint(HWND hWnd, WPARAM wParam, PAINTSTRUCT& ps) noexcept
{
    if (wParam)
    {
        ps = {};
        ps.hdc = (HDC)wParam;
        GetClipBox(ps.hdc, &ps.rcPaint);
        return ps.hdc;
    }
    else
        return BeginPaint(hWnd, &ps);
}
EckInline void EndPaint(HWND hWnd, WPARAM wParam, const PAINTSTRUCT& ps) noexcept
{
    if (!wParam)
        EndPaint(hWnd, &ps);
}
#pragma endregion Wrapper

#pragma region Color
EckInline HRESULT EnableWindowNcDarkMode(HWND hWnd, BOOL bAllow) noexcept
{
    if (g_NtVer.uBuild > WINVER_11_21H2)
        return DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &bAllow, sizeof(bAllow));
    else
    {
        WINDOWCOMPOSITIONATTRIBDATA t{ WCA_USEDARKMODECOLORS, &bAllow, sizeof(bAllow) };
        return SetWindowCompositionAttribute(hWnd, &t) ? S_OK : E_FAIL;
    }
}

EckInline BOOL IsColorSchemeChangeMessage(LPARAM lParam) noexcept
{
    return CompareStringOrdinal((PCWCH)lParam, -1,
        EckStrAndLen(L"ImmersiveColorSet"), TRUE) == CSTR_EQUAL;
}

EckInline void RefreshImmersiveColorStuff() noexcept
{
    RefreshImmersiveColorPolicyState();
    GetIsImmersiveColorUsingHighContrast(IHCM_REFRESH);
}

// 取ItemsView前景背景色。返回当前是否为暗色
inline BOOL GetItemsViewForeBackColor(_Out_ COLORREF& crText, _Out_ COLORREF& crBk) noexcept
{
    crBk = GetSysColor(COLOR_WINDOW);
    crText = GetSysColor(COLOR_WINDOWTEXT);
#if !ECK_OPT_NO_DARKMODE
    if (ShouldAppsUseDarkMode())
    {
        const auto hThemeIV = OpenThemeData(nullptr, L"ItemsView");
        if (hThemeIV)
        {
            if (FAILED(GetThemeColor(hThemeIV, 0, 0, TMT_FILLCOLOR, &crBk)))
                crBk = GetSysColor(COLOR_WINDOW);

            if (FAILED(GetThemeColor(hThemeIV, 0, 0, TMT_TEXTCOLOR, &crText)))
                crText = GetSysColor(COLOR_WINDOWTEXT);

            CloseThemeData(hThemeIV);
        }
        return TRUE;
    }
#endif// !ECK_OPT_NO_DARKMODE
    return FALSE;
}
#pragma endregion Color

#pragma region MessageHandler
inline constexpr LRESULT MsgOnNcCalculateSize(
    WPARAM wParam, LPARAM lParam, const MARGINS& Margins) noexcept
{
    if (wParam)
    {
        auto pnccsp = (NCCALCSIZE_PARAMS*)lParam;
        pnccsp->rgrc[0].left += Margins.cxLeftWidth;
        pnccsp->rgrc[0].top += Margins.cyTopHeight;
        pnccsp->rgrc[0].right -= Margins.cxRightWidth;
        pnccsp->rgrc[0].bottom -= Margins.cyBottomHeight;
    }
    else
    {
        auto prc = (RECT*)lParam;
        prc->left += Margins.cxLeftWidth;
        prc->top += Margins.cyTopHeight;
        prc->right -= Margins.cxRightWidth;
        prc->bottom -= Margins.cyBottomHeight;
    }
    return 0;
}

/// <summary>
/// 按边距执行非客户区命中测试
/// </summary>
/// <param name="pt">测试点的坐标，相对客户区</param>
/// <param name="Margins">边距</param>
/// <param name="cxWnd">窗口宽度</param>
/// <param name="cyWnd">窗口高度</param>
/// <returns>若指定点在边框内，返回对应的测试代码，否则返回HTCAPTION</returns>
EckNfInlineNdCe LRESULT MsgOnNcHitTest(POINT pt, const MARGINS& Margins, int cxWnd, int cyWnd) noexcept
{
    if (pt.x < Margins.cxLeftWidth)
    {
        if (pt.y < Margins.cyTopHeight)
            return HTTOPLEFT;
        else if (pt.y > cyWnd - Margins.cyBottomHeight)
            return HTBOTTOMLEFT;
        else
            return HTLEFT;
    }
    else if (pt.x > cxWnd - Margins.cxRightWidth)
    {
        if (pt.y < Margins.cyTopHeight)
            return HTTOPRIGHT;
        else if (pt.y > cyWnd - Margins.cyBottomHeight)
            return HTBOTTOMRIGHT;
        else
            return HTRIGHT;
    }
    else
    {
        if (pt.y < Margins.cyTopHeight)
            return HTTOP;
        else if (pt.y > cyWnd - Margins.cyBottomHeight)
            return HTBOTTOM;
        else
            return HTCAPTION;
    }
}

EckInline void MsgOnDpiChanged(HWND hWnd, LPARAM lParam) noexcept
{
    const auto* const prc = (RECT*)lParam;
    SetWindowPos(hWnd, nullptr,
        prc->left,
        prc->top,
        prc->right - prc->left,
        prc->bottom - prc->top,
        SWP_NOZORDER | SWP_NOACTIVATE);
}

// Win11存在一个问题：若某顶级窗口设置了WS_EX_TOOLWINDOW，则更改系统DPI后此窗口
// 将无法接收WM_DPICHANGED消息，直到窗口尺寸被更改时WM_DPICHANGED才会再次发送。
// 此函数提供一种简易修正方法。
// 一般WM_DPICHANGED会重新调整窗口尺寸到一个合适的值，但若存在特殊情况，
// 即WM_DPICHANGED下尺寸不变时，将bRestoreSize设为TRUE
// 截止到2025.10.4  26100.6725此问题仍然存在
inline BOOL MsgOnSettingChangeFixDpiAwareV2(HWND hWnd, WPARAM wParam, LPARAM lParam,
    BOOL bRestoreSize = FALSE) noexcept
{
    if (wParam == SPI_SETLOGICALDPIOVERRIDE /*since NT 6.2*/ &&
        g_NtVer.uBuild > WINVER_11_23H2 /*经粗略测试Bug在此版本之后引入*/ &&
        (GetWindowLongPtrW(hWnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW))
    {
        RECT rc;
        GetWindowRect(hWnd, &rc);
        SetWindowPos(hWnd, nullptr, 0, 0, rc.right - rc.left + 1, rc.bottom - rc.top,
            SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
        if (bRestoreSize)
            SetWindowPos(hWnd, nullptr, 0, 0, rc.right - rc.left, rc.bottom - rc.top,
                SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
        return TRUE;
    }
    return FALSE;
}

// 提供对亮暗切换的默认处理。
// 一般仅用于**主**顶级窗口，除了产生相关更新消息外还更新当前线程上下文的默认颜色
inline BOOL MsgOnSettingChangeMainWindow(HWND hWnd, WPARAM wParam, LPARAM lParam,
    BOOL bRefreshUxColorMode = FALSE) noexcept
{
    if (IsColorSchemeChangeMessage(lParam))
    {
        const auto ptc = PtcCurrent();
        if (bRefreshUxColorMode)
            RefreshImmersiveColorStuff();
        if (ptc->bAppDarkMode != ShouldAppsUseDarkMode())
        {
            ptc->UpdateDefaultColor();
            ptc->TwmEnableNcDarkMode(ShouldAppsUseDarkMode());
            BroadcastChildrenMessage(hWnd, WM_SETTINGCHANGE, wParam, lParam);
            ptc->TwmBroadcastThemeChanged();
        }
        return TRUE;
    }
    else
        return FALSE;
}

// 提供对WM_SYSCOLORCHANGE的默认处理。
// 一般仅用于**主**顶级窗口，除了向子窗口广播消息外还更新当前线程上下文的默认颜色
EckInline void MsgOnSystemColorChangeMainWindow(HWND hWnd, WPARAM wParam, LPARAM lParam) noexcept
{
    PtcCurrent()->UpdateDefaultColor();
    BroadcastChildrenMessage(hWnd, WM_SYSCOLORCHANGE, wParam, lParam);
}

// 提供对WM_SYSCOLORCHANGE的默认处理。一般仅用于顶级窗口
EckInline void MsgOnSystemColorChange(HWND hWnd, WPARAM wParam, LPARAM lParam) noexcept
{
    BroadcastChildrenMessage(hWnd, WM_SYSCOLORCHANGE, wParam, lParam);
}

/// <summary>
/// 处理控件着色
/// </summary>
/// <param name="wParam"></param>
/// <param name="lParam"></param>
/// <param name="bHandled">是否已处理</param>
/// <param name="bColorDisableEdit">着色禁用的编辑框</param>
/// <param name="crBk">背景色</param>
/// <param name="crText">文本色</param>
/// <returns>LRESULT</returns>
inline LRESULT MsgOnControlColorXxx(
    WPARAM wParam,
    LPARAM lParam,
    BOOL& bHandled,
    BOOL bColorDisableEdit = TRUE,
    COLORREF crBk = CLR_DEFAULT,
    COLORREF crText = CLR_DEFAULT) noexcept
{
    switch (wParam)
    {
    case WM_CTLCOLORSTATIC:
        if (!bColorDisableEdit)
        {
            WCHAR szCls[ARRAYSIZE(WC_EDITW) + 1];
            if (GetClassNameW((HWND)lParam, szCls, ARRAYSIZE(szCls)) &&
                _wcsicmp(szCls, WC_EDITW) == 0)
                break;
        }
        [[fallthrough]];
    case WM_CTLCOLORBTN:
    case WM_CTLCOLORDLG:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX:
    {
        bHandled = TRUE;
        const auto* const ptc = PtcCurrent();
        SetTextColor((HDC)wParam, ptc->crDefText);
        SetBkColor((HDC)wParam, crBk != CLR_DEFAULT ? crBk : ptc->crDefBkg);
        SetDCBrushColor((HDC)wParam, crBk != CLR_DEFAULT ? crBk : ptc->crDefBkg);
        return (LRESULT)GetStockObject(DC_BRUSH);
    }
    break;
    }
    bHandled = FALSE;
    return 0;
}
#pragma endregion MessageHandler

#pragma region Thread
inline HWND GetThreadFirstWindow(UINT uTid) noexcept
{
    HWND hWnd = GetActiveWindow();
    if (!hWnd)
        return hWnd;
    EnumThreadWindows(uTid, [](HWND hWnd, LPARAM lParam)->BOOL
        {
            if (IsWindowVisible(hWnd))
            {
                *(HWND*)lParam = hWnd;
                return TRUE;
            }
            else
                return FALSE;
        }, (LPARAM)&hWnd);
    return hWnd;
}

inline HWND GetSafeOwner(HWND hParent, _Out_opt_ HWND* phWndTop) noexcept
{
    HWND hWnd = hParent;
    if (!hWnd)
        hWnd = GetThreadFirstWindow(NtCurrentThreadId32());

    while (hWnd && (GetWindowLongPtrW(hWnd, GWL_STYLE) & WS_CHILD))
        hWnd = GetParent(hWnd);

    HWND hWndTop = hWnd, hWndTemp = hWnd;
    EckLoop()
    {
        if (!hWndTemp)
            break;
        else
            hWndTop = hWndTemp;
        hWndTemp = GetParent(hWndTop);
    }

    if (!hParent && hWnd)
        hWnd = GetLastActivePopup(hWnd);

    if (phWndTop)
    {
        if (hWndTop && IsWindowEnabled(hWndTop) && hWndTop != hWnd)
        {
            *phWndTop = hWndTop;
            EnableWindow(hWndTop, FALSE);
        }
        else
            *phWndTop = nullptr;
    }

    return hWnd;
}

/// <summary>
/// 鼠标是否移动出拖放距离阈值。
/// 给定起始点，判断自调用函数的瞬间起鼠标移动距离是否超过(dx, dy)。
/// 警告：1. 此函数启动拖放消息循环，返回后窗口可能已被销毁；
///      2. 函数使用SetCapture和ReleaseCapture。
/// </summary>
/// <param name="hWnd">窗口句柄</param>
/// <param name="x">起始X，相对屏幕</param>
/// <param name="y">起始Y，相对屏幕</param>
/// <param name="dx">x方向的阈值，若为负，则使用GetSystemMetrics(SM_CXDRAG)</param>
/// <param name="dy">y方向的阈值，若为负，则使用GetSystemMetrics(SM_CYDRAG)</param>
/// <param name="iDpi">DPI，若为负，则自动获取</param>
/// <returns>移动距离超出阈值返回TRUE，此时调用方可执行拖放操作；否则返回FALSE</returns>
inline BOOL IsMouseMovedBeforeDragging(
    HWND hWnd,
    int x, int y,
    int dx = -1, int dy = -1,
    int iDpi = -1) noexcept
{
    if ((dx < 0 || dy < 0) && iDpi < 0)
        iDpi = GetDpi(hWnd);
    if (dx < 0)
        dx = DaGetSystemMetrics(SM_CXDRAG, iDpi);
    if (dy < 0)
        dy = DaGetSystemMetrics(SM_CYDRAG, iDpi);
    SetCapture(hWnd);
    MSG msg;
    while (GetCapture() == hWnd)
    {
        if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            switch (msg.message)
            {
            case WM_LBUTTONUP:
            case WM_LBUTTONDOWN:
            case WM_RBUTTONUP:
            case WM_RBUTTONDOWN:
            case WM_MBUTTONUP:
            case WM_MBUTTONDOWN:
                ReleaseCapture();
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
                return FALSE;
            case WM_MOUSEMOVE:
                if (Abs(x - msg.pt.x) >= dx && Abs(y - msg.pt.y) >= dx)
                {
                    ReleaseCapture();
                    return TRUE;
                }
                [[fallthrough]];
            default:
                PtcCurrent()->DoCallback();
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
                break;
            }
        }
        else
            WaitMessage();
    }
    ReleaseCapture();
    return FALSE;
}

inline void DoEvents() noexcept
{
    MSG msg;
    while (PeekMessageW(&msg, nullptr, 0u, 0u, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            PostQuitMessage((int)msg.wParam);
            return;
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}
#pragma endregion Thread

#pragma region Window
inline HWND GetWindowFromPoint(POINT pt, UINT uFlags = CWP_SKIPINVISIBLE) noexcept
{
    const HWND hParent = WindowFromPoint(pt);
    if (!hParent)
        return nullptr;
    POINT pt0{ pt };
    ScreenToClient(hParent, &pt0);

    HWND hChild0{ hParent }, hChild1;
    EckLoop()
    {
        pt0 = pt;
        ScreenToClient(hParent, &pt0);
        hChild1 = ChildWindowFromPointEx(hChild0, pt0, uFlags);
        if (!hChild1 || hChild1 == hChild0)
            return hChild0;
        hChild0 = hChild1;
    }
}

/// <summary>
/// 取窗口客户区尺寸。
/// 在最小化时也有效
/// </summary>
/// <param name="hWnd">窗口句柄</param>
/// <param name="rcClient">客户区尺寸，其中left和top总为0</param>
/// <returns>成功返回TRUE，失败返回FALSE</returns>
inline BOOL GetWindowClientRect(HWND hWnd, _Out_ RECT& rcClient) noexcept
{
    const int iDpi = GetDpi(hWnd);
    RECT rcMainClient;
    RECT rcNcOnly{};
    WINDOWPLACEMENT wp;
    wp.length = sizeof(wp);
    if (DaAdjustWindowRectEx(&rcNcOnly, GetWindowStyle(hWnd),
        !!GetMenu(hWnd), GetWindowExStyle(hWnd), iDpi) &&
        GetWindowPlacement(hWnd, &wp))
    {
        if (wp.flags & WPF_RESTORETOMAXIMIZED)
        {
            const auto hMonitor = MonitorFromRect(&wp.rcNormalPosition, MONITOR_DEFAULTTONULL);
            if (hMonitor)
            {
                MONITORINFO mi;
                mi.cbSize = sizeof(mi);
                GetMonitorInfoW(hMonitor, &mi);
                rcMainClient = mi.rcMonitor;
            }
            else
                goto Failed;
        }
        else
            rcMainClient = wp.rcNormalPosition;
        // 对齐到(0, 0)
        OffsetRect(rcMainClient, -rcMainClient.left, -rcMainClient.top);
        // 减掉非客户区
        rcMainClient.right -= (rcNcOnly.right - rcNcOnly.left);
        rcMainClient.bottom -= (rcNcOnly.bottom - rcNcOnly.top);
        rcClient = rcMainClient;
        return TRUE;
    }
    else
    {
    Failed:
        rcClient = {};
        return FALSE;
    }
}
#pragma endregion Window

#pragma region Monitor
EckInlineNd HMONITOR MonitorFromRectByWorkArea(const RECT& rc,
    HMONITOR* phMonMain = nullptr, HMONITOR* phMonNearest = nullptr) noexcept
{
    struct CTX
    {
        const RECT& rc;
        int iMinDistance;
        int iMaxArea;
        HMONITOR hMon;
        HMONITOR hMonMain;
        HMONITOR hMonNearest;
    }
    Ctx{ rc,INT_MAX };

    EnumDisplayMonitors(nullptr, nullptr, [](HMONITOR hMonitor, HDC, RECT*, LPARAM lParam)->BOOL
        {
            const auto pCtx = (CTX*)lParam;

            MONITORINFO mi;
            mi.cbSize = sizeof(mi);
            GetMonitorInfoW(hMonitor, &mi);
            if (IsBitSet(mi.dwFlags, MONITORINFOF_PRIMARY))
                pCtx->hMonMain = hMonitor;

            RECT rc;
            if (IntersectRect(rc, mi.rcWork, pCtx->rc))
            {
                const int iArea = (rc.right - rc.left) * (rc.bottom - rc.top);
                if (iArea > pCtx->iMaxArea)
                {
                    pCtx->iMaxArea = iArea;
                    pCtx->hMon = hMonitor;
                }
            }

            const int dx = (pCtx->rc.left + pCtx->rc.right) / 2 -
                (mi.rcWork.left + mi.rcWork.right) / 2;
            const int dy = (pCtx->rc.top + pCtx->rc.bottom) / 2 -
                (mi.rcWork.top + mi.rcWork.bottom) / 2;
            const int d = dx * dx + dy * dy;
            if (d < pCtx->iMinDistance)
            {
                pCtx->iMinDistance = d;
                pCtx->hMonNearest = hMonitor;
            }
            return TRUE;
        }, (LPARAM)&Ctx);

    if (phMonMain)
        *phMonMain = Ctx.hMonMain;
    if (phMonNearest)
        *phMonNearest = Ctx.hMonNearest;
    return Ctx.hMon;
}

EckInline HMONITOR GetPrimaryMonitor() noexcept
{
    HMONITOR hMonitor{};
    EnumDisplayMonitors(nullptr, nullptr, [](HMONITOR hMonitor, HDC, RECT*, LPARAM lParam)->BOOL
        {
            MONITORINFO mi;
            mi.cbSize = sizeof(mi);
            GetMonitorInfoW(hMonitor, &mi);
            if (IsBitSet(mi.dwFlags, MONITORINFOF_PRIMARY))
            {
                *(HMONITOR*)lParam = hMonitor;
                return FALSE;
            }
            else
                return TRUE;
        }, (LPARAM)&hMonitor);
    return hMonitor;
}

inline HMONITOR GetOwnerMonitor(HWND hWnd) noexcept
{
    if (hWnd)
        return MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY);
    else
    {
        const auto hForegnd = GetForegroundWindow();
        DWORD dwPid;
        GetWindowThreadProcessId(hForegnd, &dwPid);
        if (dwPid == NtCurrentProcessId32())// 如果前台窗口是自进程窗口，返回这个窗口所在的显示器
            return MonitorFromWindow(hForegnd, MONITOR_DEFAULTTOPRIMARY);
        else// 否则返回主显示器
            return GetPrimaryMonitor();
    }
}

/// <summary>
/// 计算窗口居中坐标
/// </summary>
/// <param name="hParent">父窗口，若为空，则使用最佳显示器</param>
/// <param name="cx">子窗口宽度</param>
/// <param name="cy">子窗口高度</param>
/// <param name="bEnsureInMonitor">是否确保在显示器内</param>
/// <returns>子窗口左上角坐标，相对屏幕</returns>
inline POINT CalculateCenterWindowPosition(HWND hParent, int cx, int cy,
    BOOL bEnsureInMonitor = TRUE) noexcept
{
    if (hParent)
    {
        RECT rc;
        GetWindowRect(hParent, &rc);
        rc.left += (rc.right - rc.left - cx) / 2;
        rc.top += (rc.bottom - rc.top - cy) / 2;
        if (bEnsureInMonitor)
        {
            rc.right = rc.left + cx;
            rc.bottom = rc.top + cy;
            const auto hMonitor = GetOwnerMonitor(hParent);
            MONITORINFO mi;
            mi.cbSize = sizeof(mi);
            GetMonitorInfoW(hMonitor, &mi);
            AdjustRectIntoAnother(rc, mi.rcWork);
            return { rc.left, rc.top };
        }
        else
            return { rc.left, rc.top };
    }
    else
    {
        const auto hMonitor = GetOwnerMonitor(nullptr);
        MONITORINFO mi;
        mi.cbSize = sizeof(mi);
        GetMonitorInfoW(hMonitor, &mi);
        return
        {
            mi.rcWork.left + (mi.rcWork.right - mi.rcWork.left - cx) / 2,
            mi.rcWork.top + (mi.rcWork.bottom - mi.rcWork.top - cy) / 2
        };
    }
}
#pragma endregion Monitor

#pragma region Input
inline LRESULT GenerateCharMessage(HWND hWnd, WCHAR ch, BOOL bExtended = FALSE) noexcept
{
    switch (ch)
    {
    case 58:
    case 59:
        ch = VK_OEM_1;
        break;
    case 43:
        ch = VK_OEM_PLUS;
        break;
    case 44:
        ch = VK_OEM_COMMA;
        break;
    case 45:
        ch = VK_OEM_MINUS;
        break;
    case 46:
        ch = VK_OEM_PERIOD;
        break;
    case 47:
    case 63:
        ch = VK_OEM_2;
        break;
    case 96:
    case 126:
        ch = VK_OEM_3;
        break;
    default:
        if (ch >= 97 && ch <= 122)
            ch -= 32;
        break;
    }
    const auto uScanCode = MapVirtualKeyW(ch, MAPVK_VK_TO_VSC);
    return PostMessageW(hWnd, WM_CHAR, ch, MakeKeyStrokeFlag(1, uScanCode, bExtended, FALSE, FALSE, FALSE));
}

inline void GenerateCharMessage(HWND hWnd, PCWSTR pszText, int cchText = -1,
    BOOL bExtended = FALSE, BOOL bReplaceEndOfLine = TRUE) noexcept
{
    if (cchText < 0)
        cchText = (int)wcslen(pszText);
    if (bReplaceEndOfLine)
        for (int i{}; i < cchText;)
        {
            auto ch = pszText[i++];
            if (ch == L'\r' && i < cchText && pszText[i] == L'\n')
            {
                ch = L'\n';
                ++i;
            }
            GenerateCharMessage(hWnd, ch, bExtended);
        }
    else
    {
        EckCounter(cchText, i)
            GenerateCharMessage(hWnd, pszText[i], bExtended);
    }
}

enum class KeyType
{
    Down,
    Up,
    Press,
};

inline void GenerateKeyMessage(HWND hWnd, UINT Vk,
    KeyType eType, BOOL bExtended = FALSE) noexcept
{
    switch (eType)
    {
    case KeyType::Down:
        PostMessageW(hWnd, bExtended ? WM_SYSKEYDOWN : WM_KEYDOWN, Vk,
            MakeKeyStrokeFlag(1, MapVirtualKeyW(Vk, MAPVK_VK_TO_VSC), bExtended, FALSE, FALSE, FALSE));
        break;
    case KeyType::Up:
        PostMessageW(hWnd, bExtended ? WM_SYSKEYUP : WM_KEYUP, Vk,
            MakeKeyStrokeFlag(1, MapVirtualKeyW(Vk, MAPVK_VK_TO_VSC), bExtended, FALSE, TRUE, TRUE));
        break;
    case KeyType::Press:
        GenerateKeyMessage(hWnd, Vk, KeyType::Down, bExtended);
        GenerateKeyMessage(hWnd, Vk, KeyType::Up, bExtended);
        break;
    default:
        ECK_UNREACHABLE;
    }
}
#pragma endregion Input

#pragma region Others
inline SIZE GetCharDimension(HWND hWnd, HFONT hFont) noexcept
{
    const auto hDC = GetDC(hWnd);
    const auto hFontOld = (HFONT)SelectObject(hDC, hFont);
    TEXTMETRICW tm;
    GetTextMetricsW(hDC, &tm);
    if (tm.tmPitchAndFamily & TMPF_FIXED_PITCH)
    {
        SIZE size;
        GetTextExtentPoint32W(hDC,
            L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", 52, &size);
        ReleaseDC(hWnd, hDC);
        size.cx = ((size.cx / 26) + 1) / 2;
        size.cy = tm.tmHeight;
        return size;
    }
    else
    {
        ReleaseDC(hWnd, hDC);
        return { tm.tmAveCharWidth, tm.tmHeight };
    }
}

// 根据位置判断窗口是否应显示大小调整控件。该函数无视被判断窗口的样式
inline BOOL ShouldWindowDisplaySizeGrip(HWND hWnd, int iDpi = -1) noexcept
{
    const auto hDesktop = GetDesktopWindow();
    if (iDpi < 0)
        iDpi = GetDpi(hWnd);
    RECT rc, rcTmp;
    GetWindowRect(hWnd, &rc);
    rc.right += DaGetSystemMetrics(SM_CXFRAME, iDpi);
    rc.bottom += DaGetSystemMetrics(SM_CYFRAME, iDpi);
    while (hWnd && hWnd != hDesktop)
    {
        const auto dwStyle = GetWindowLongPtrW(hWnd, GWL_STYLE);
        if (IsBitSet(dwStyle, WS_SIZEBOX))// 向上找到第一个可调窗口
        {
            if (IsBitSet(dwStyle, WS_MAXIMIZE))
                return FALSE;// 最大化时不应显示
            GetWindowRect(hWnd, &rcTmp);
            if (rc.right < rcTmp.right || rc.bottom < rcTmp.bottom)
                return FALSE;// 没有贴近右下角时不应显示
            return TRUE;
        }
        else
            hWnd = GetParent(hWnd);
    }
    return FALSE;
}

inline HRESULT MatchLogFontFromFamilyName(PCWSTR pszFamilyName,
    _Out_ LOGFONTW& lf, IDWriteFontCollection* pFontCollection = nullptr) noexcept
{
    HRESULT hr;
    ComPtr<IDWriteFontCollection> pFontCollection_;
    if (!pFontCollection)
    {
        if (!g_pDwFactory)
            return HRESULT_FROM_WIN32(ERROR_NOT_READY);
        g_pDwFactory->GetSystemFontCollection(
            &pFontCollection_, FALSE);
        pFontCollection = pFontCollection_.Get();
    }
    UINT32 idxFamily;
    BOOL bExists;
    hr = pFontCollection->FindFamilyName(
        *pszFamilyName == L'@' ? pszFamilyName + 1 : pszFamilyName,
        &idxFamily, &bExists);
    if (bExists)
    {
        ComPtr<IDWriteFontFamily> pFamily;
        hr = pFontCollection->GetFontFamily(idxFamily, &pFamily);
        if (FAILED(hr))
            goto FallbackToGdi;
        ComPtr<IDWriteFontList> pFontList;
        hr = pFamily->GetMatchingFonts((DWRITE_FONT_WEIGHT)0,
            DWRITE_FONT_STRETCH_NORMAL, DWRITE_FONT_STYLE_NORMAL,
            &pFontList);
        if (FAILED(hr))
            goto FallbackToGdi;
        ComPtr<IDWriteFont> pFont;
        hr = pFontList->GetFont(0, &pFont);
        if (FAILED(hr))
            goto FallbackToGdi;
        ComPtr<IDWriteGdiInterop> pGdiInterop;
        g_pDwFactory->GetGdiInterop(&pGdiInterop);
        BOOL bSysFont;
        hr = pGdiInterop->ConvertFontToLOGFONT(
            pFont.Get(), &lf, &bSysFont);
        if (FAILED(hr))
            goto FallbackToGdi;
        return S_OK;
    }
FallbackToGdi:;
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfPitchAndFamily = 0;
    wcscpy_s(lf.lfFaceName, pszFamilyName);
    struct PARAM
    {
        LOGFONTW* plf;
        BOOL bFound;
    } Param{ &lf };

    const auto hDC = GetDC(nullptr);
    EnumFontFamiliesExW(hDC, &lf,
        [](const LOGFONTW* plf, const TEXTMETRICW*, DWORD, LPARAM lParam) -> int
        {
            const auto pParam = (PARAM*)lParam;
            *pParam->plf = *plf;
            pParam->bFound = TRUE;
            return FALSE;
        }, (LPARAM)&Param, 0);
    ReleaseDC(nullptr, hDC);
    if (Param.bFound)
        return S_OK;
    else
        return hr;
}
#pragma endregion Others

#pragma region SystemFont
EckInline BOOL DftGetLogFont(_Out_ LOGFONTW& lf, int iDpi = 96) noexcept
{
    return DaSystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, 0, iDpi);
}

EckNfInlineNd HFONT DftCreate(int iDpi = 96) noexcept
{
    LOGFONTW lf;
    if (!DftGetLogFont(lf, iDpi))
        return nullptr;
    return CreateFontIndirectW(&lf);
}
EckNfInlineNd HFONT DftCreateWithSize(int cy, int iDpi = 96) noexcept
{
    LOGFONTW lf;
    if (!DftGetLogFont(lf, iDpi))
        return nullptr;
    lf.lfHeight = cy;
    return CreateFontIndirectW(&lf);
}

namespace Priv
{
    EckNfInlineNd HRESULT DftpCreate(_Out_ IDWriteTextFormat*& pTf,
        const LOGFONTW& lf, float cy, int iDpi = 96) noexcept
    {
        WCHAR szLocaleName[LOCALE_NAME_MAX_LENGTH];
        if (!GetUserDefaultLocaleName(EckArrAndLen(szLocaleName)))
            return HRESULT_FROM_WIN32(NaGetLastError());
        return g_pDwFactory->CreateTextFormat(
            lf.lfFaceName,
            nullptr,
            (DWRITE_FONT_WEIGHT)std::clamp(lf.lfWeight, 1L, 999L),
            lf.lfItalic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            cy * iDpi / 96.f,
            szLocaleName,
            &pTf);
    }
}

EckNfInlineNd HRESULT DftCreateDWriteWithSize(
    _Out_ IDWriteTextFormat*& pTf, float cy, int iDpi = 96) noexcept
{
    LOGFONTW lf;
    if (!DftGetLogFont(lf, iDpi))
        return HRESULT_FROM_WIN32(NaGetLastError());
    return Priv::DftpCreate(pTf, lf, cy, iDpi);
}
EckNfInlineNd HRESULT DftCreateDWrite(_Out_ IDWriteTextFormat*& pTf, int iDpi = 96) noexcept
{
    LOGFONTW lf;
    if (!DftGetLogFont(lf, iDpi))
        return HRESULT_FROM_WIN32(NaGetLastError());
    return Priv::DftpCreate(pTf, lf, fabs((float)lf.lfHeight), 96);
}
#pragma endregion SystemFont
ECK_NAMESPACE_END