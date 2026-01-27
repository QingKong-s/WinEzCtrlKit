#pragma once
#include "CWnd.h"
#include "GraphicsHelper.h"
#include "CMenu.h"

ECK_NAMESPACE_BEGIN
class CForm : public CWnd
{
public:
    ECK_RTTI(CForm, CWnd);
    ECK_CWND_NOSINGLEOWNER(CForm);
    ECK_CWND_CREATE_CLS_HINST(WCN_FORM, g_hInstance);
private:
    struct TRAY
    {
        UINT uID;
        UINT uFlags;
        DWORD dwState;
        HICON hIcon;
        CRefStrW rsTip;
    };

    HBITMAP m_hbmBk = nullptr;
    int m_cxImage = 0,
        m_cyImage = 0;
    BkImgMode m_iBkImageMode = BkImgMode::TopLeft;
    COLORREF m_crBk = CLR_DEFAULT;

    int m_cxClient = 0,
        m_cyClient = 0;

    std::vector<TRAY> m_Tray{};

    BITBOOL m_bMoveable : 1 = TRUE;
    BITBOOL m_bFillWndImage : 1 = FALSE;
    BITBOOL m_bEscClose : 1 = FALSE;
    BITBOOL m_bMoveAnywhere : 1 = FALSE;
    BITBOOL m_bClrDisableEdit : 1 = FALSE;
public:
    static UINT s_uTrayMsg;
    static UINT s_uTaskbarCreatedMsg;
public:
    ECKPROP(GetBackgroundImage, SetBackgroundImage) HBITMAP		BkImage;		// 背景图片
    ECKPROP(GetBackgroundMode, SetBackgroundMode)   BkImgMode	BkImageMode;	// 背景图片模式
    ECKPROP(GetBackgroundFillWindow, SetBackgroundFillWindow)   BOOL		FillWndImage;   // 全窗口绘制背景图片
    ECKPROP(GetMoveable, SetMoveable)               BOOL		Moveable;		// 可否移动
    ECKPROP(GetEscClose, SetEscClose)               BOOL		EscClose;		// ESC关闭
    ECKPROP(GetDragMoveAnywhere, SetDragMoveAnywhere)           BOOL		MoveAnywhere;   // 随意移动
    ECKPROP(GetBackgroundColor, SetBackgroundColor) COLORREF	BkColor;		// 背景颜色
    ECKPROP_R(GetBackgroundImageSize)               SIZE		BkImageSize;	// 背景图片大小

    BOOL PreTranslateMessage(const MSG& Msg) noexcept override
    {
        switch (Msg.message)
        {
        case WM_KEYDOWN:
        {
            if (m_bEscClose && Msg.wParam == VK_ESCAPE && IsWindowEnabled(GetHWND()))
            {
                PostMessageW(GetHWND(), WM_CLOSE, 0, 0);
                return TRUE;
            }
        }
        break;
        case WM_LBUTTONDOWN:
        {
            if (m_bMoveAnywhere && IsWindowEnabled(GetHWND()))
                if ((Msg.hwnd == GetHWND() ||
                    (SendMessageW(Msg.hwnd, WM_GETDLGCODE, Msg.wParam, (LPARAM)&Msg) & DLGC_STATIC)))
                {
                    PostMessageW(GetHWND(), WM_NCLBUTTONDOWN, HTCAPTION, 0);
                    return TRUE;
                }
        }
        break;
        }
        return __super::PreTranslateMessage(Msg);
    }

    LRESULT OnMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept override
    {
        switch (uMsg)
        {
        case WM_CTLCOLORSTATIC:
            if (!m_bClrDisableEdit)
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
            const auto* const ptc = PtcCurrent();
            SetTextColor((HDC)wParam, ptc->crDefText);
            SetBkColor((HDC)wParam, m_crBk != CLR_DEFAULT ? m_crBk : ptc->crDefBkg);
            SetDCBrushColor((HDC)wParam, m_crBk != CLR_DEFAULT ? m_crBk : ptc->crDefBkg);
            return (LRESULT)GetStockBrush(DC_BRUSH);
        }
        break;

        case WM_SIZE:
            ECK_GET_SIZE_LPARAM(m_cxClient, m_cyClient, lParam);
            return 0;

        case WM_PRINTCLIENT:
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hWnd, wParam, ps);
            SetDCBrushColor(ps.hdc, m_crBk == CLR_DEFAULT ?
                PtcCurrent()->crDefBkg : m_crBk);
            FillRect(ps.hdc, &ps.rcPaint, GetStockBrush(DC_BRUSH));
            if (m_hbmBk)
            {
                HDC hCDC = CreateCompatibleDC(ps.hdc);
                SelectObject(hCDC, m_hbmBk);
                const RECT rc{ 0,0,m_cxClient,m_cyClient };
                DrawBackgroundImage32(ps.hdc, hCDC, rc, m_cxImage, m_cyImage,
                    m_iBkImageMode, m_bFillWndImage);
                DeleteDC(hCDC);
            }
            EndPaint(hWnd, wParam, ps);
        }
        return 0;

        case WM_ERASEBKGND:
        {
            const auto hDC = (HDC)wParam;
            RECT rc;
            GetClipBox(hDC, &rc);
            SetDCBrushColor(hDC, m_crBk == CLR_DEFAULT ?
                PtcCurrent()->crDefBkg : m_crBk);
            FillRect(hDC, &rc, GetStockBrush(DC_BRUSH));
        }
        return TRUE;

        case WM_DESTROY:
        {
            for (const auto& e : m_Tray)
            {
                NOTIFYICONDATAW nid;
                nid.cbSize = sizeof(nid);
                nid.hWnd = hWnd;
                nid.uID = e.uID;
                nid.uFlags = 0;
                Shell_NotifyIconW(NIM_DELETE, &nid);
            }
            m_Tray.clear();
        }
        break;
        }

        if (uMsg == s_uTrayMsg)
        {
            OnTrayNotify(LOWORD(lParam), HIWORD(lParam),
                GET_X_LPARAM(wParam), GET_Y_LPARAM(wParam));
            return 0;
        }
        else if (uMsg == s_uTaskbarCreatedMsg)
        {
            NOTIFYICONDATAW nid;
            nid.cbSize = sizeof(nid);
            nid.hWnd = hWnd;
            nid.uCallbackMessage = s_uTrayMsg;
            for (const auto& e : m_Tray)
            {
                nid.uID = e.uID;
                nid.uFlags = e.uFlags | NIF_MESSAGE;
                nid.dwState = nid.dwStateMask = e.dwState;
                nid.hIcon = e.hIcon;
                if (e.rsTip.IsEmpty())
                    nid.szTip[0] = L'\0';
                else
                    TcsCopyLenEnd(nid.szTip, e.rsTip.Data(),
                        std::min(e.rsTip.Size(), (int)ARRAYSIZE(nid.szTip)));
                Shell_NotifyIconW(NIM_SETVERSION, &nid);
                Shell_NotifyIconW(NIM_ADD, &nid);
            }
            return 0;
        }
        return CWnd::OnMessage(hWnd, uMsg, wParam, lParam);
    }

    virtual void OnTrayNotify(UINT uMsg, UINT uID, int x, int y) noexcept
    {

    }

    EckInlineNdCe HBITMAP GetBackgroundImage() const noexcept { return m_hbmBk; }

    EckInline void SetBackgroundImage(HBITMAP hbmBk) noexcept
    {
        m_hbmBk = hbmBk;
        BITMAP bm;
        GetObjectW(hbmBk, sizeof(bm), &bm);
        m_cxImage = bm.bmWidth;
        m_cyImage = bm.bmHeight;
    }

    EckInlineCe void SetBackgroundMode(BkImgMode iMode) noexcept { m_iBkImageMode = iMode; }
    EckInlineNdCe BkImgMode GetBackgroundMode() const noexcept { return m_iBkImageMode; }

    EckInlineCe void SetBackgroundFillWindow(BOOL b) noexcept { m_bFillWndImage = b; }
    EckInlineNdCe BOOL GetBackgroundFillWindow() const noexcept { return m_bFillWndImage; }

    EckInline void SetMoveable(BOOL bMoveable) noexcept
    {
        m_bMoveable = bMoveable;
        CMenu SysMenu(GetSystemMenu(GetHWND(), FALSE));
        if (bMoveable)
            SysMenu.EnableItem(SC_MOVE, MF_GRAYED, FALSE);
        else
            SysMenu.EnableItem(SC_MOVE, MF_ENABLED, FALSE);
        (void)SysMenu.Detach();
    }
    EckInlineNdCe BOOL GetMoveable() const noexcept { return m_bMoveable; }

    EckInlineCe void SetEscClose(BOOL b) noexcept { m_bEscClose = b; }
    EckInlineNdCe BOOL GetEscClose() const noexcept { return m_bEscClose; }

    EckInlineCe void SetDragMoveAnywhere(BOOL b) noexcept { m_bMoveAnywhere = b; }
    EckInlineNdCe BOOL GetDragMoveAnywhere() const noexcept { return m_bMoveAnywhere; }

    EckInlineCe void SetBackgroundColor(COLORREF crBk) noexcept { m_crBk = crBk; }
    EckInlineNdCe COLORREF GetBackgroundColor() const noexcept { return m_crBk; }

    EckInlineNdCe SIZE GetBackgroundImageSize() const noexcept { return { m_cxImage, m_cyImage }; }

    BOOL TrayAdd(UINT uID, HICON hIcon, PCWSTR pszTip,
        DWORD dwState = 0u, BOOL bShowTip = TRUE) noexcept
    {
        NOTIFYICONDATAW nid;
        nid.cbSize = sizeof(nid);
        nid.hWnd = HWnd;
        nid.uID = uID;
        nid.uFlags = NIF_MESSAGE | NIF_STATE | (bShowTip ? NIF_TIP : 0);
        nid.uCallbackMessage = s_uTrayMsg;
        nid.uVersion = NOTIFYICON_VERSION_4;
        nid.dwState = nid.dwStateMask = dwState;
        if (hIcon)
        {
            nid.uFlags |= NIF_ICON;
            nid.hIcon = hIcon;
        }
        if (pszTip)
        {
            nid.uFlags |= NIF_TIP;
            wcscpy_s(nid.szTip, pszTip);
        }

        Shell_NotifyIconW(NIM_SETVERSION, &nid);
        m_Tray.emplace_back(uID, nid.uFlags, dwState, hIcon, pszTip);
        if (!Shell_NotifyIconW(NIM_ADD, &nid))
            return FALSE;
        return TRUE;
    }

    BOOL TrayModify(UINT uID, UINT uFlags, HICON hIcon = nullptr,
        PCWSTR pszTip = nullptr, DWORD dwState = 0u) noexcept
    {
        auto it = std::find_if(m_Tray.begin(), m_Tray.end(), [uID](const TRAY& x)
            {
                return x.uID == uID;
            });
        if (it == m_Tray.end())
        {
            EckDbgPrint(L"** WARNING ** 试图修改未经内部维护的托盘图标");
            return FALSE;
        }
        EckAssert(uFlags == (uFlags & ~(NIF_ICON | NIF_TIP | NIF_STATE)));
        uFlags &= ~(NIF_ICON | NIF_TIP | NIF_STATE | NIF_SHOWTIP);
        NOTIFYICONDATAW nid;
        nid.cbSize = sizeof(nid);
        nid.hWnd = HWnd;
        nid.uID = uID;
        nid.uFlags = uFlags;
        it->uFlags = uFlags;
        if (IsBitSet(uFlags, NIF_ICON))
        {
            it->hIcon = hIcon;
            nid.hIcon = hIcon;
        }
        if (IsBitSet(uFlags, NIF_TIP))
        {
            it->rsTip = pszTip;
            wcscpy_s(nid.szTip, pszTip);
        }
        if (IsBitSet(uFlags, NIF_STATE))
        {
            it->dwState = dwState;
            nid.dwState = nid.dwStateMask = dwState;
        }

        return Shell_NotifyIconW(NIM_MODIFY, &nid);
    }

    BOOL TrayDelete(UINT uID) noexcept
    {
        auto it = std::find_if(m_Tray.begin(), m_Tray.end(), [uID](const TRAY& x)
            {
                return x.uID == uID;
            });
        if (it == m_Tray.end())
        {
            EckDbgPrint(L"** WARNING ** 试图删除未经内部维护的托盘图标");
            return FALSE;
        }
        NOTIFYICONDATAW nid;
        nid.cbSize = sizeof(nid);
        nid.hWnd = HWnd;
        nid.uID = uID;
        nid.uFlags = 0;

        if (!Shell_NotifyIconW(NIM_DELETE, &nid))
            return FALSE;
        else
        {
            m_Tray.erase(it);
            return TRUE;
        }
    }

    BOOL TraySetFocus(UINT uID) noexcept
    {
        NOTIFYICONDATAW nid;
        nid.cbSize = sizeof(nid);
        nid.hWnd = HWnd;
        nid.uID = uID;
        nid.uFlags = 0;
        return Shell_NotifyIconW(NIM_SETFOCUS, &nid);
    }

    BOOL TrayPopBalloon(UINT uID, PCWSTR pszContent, PCWSTR pszTitle = nullptr,
        HICON hBalloonIcon = nullptr, DWORD dwInfoFlags = 0u, BOOL bRealTime = FALSE) noexcept
    {
        NOTIFYICONDATAW nid;
        nid.cbSize = sizeof(nid);
        nid.hWnd = HWnd;
        nid.uID = uID;
        nid.uFlags = NIF_INFO | (bRealTime ? NIF_REALTIME : 0);

        if (pszContent)
            wcscpy_s(nid.szInfo, pszContent);
        else
            nid.szInfo[0] = L'\0';
        if (pszTitle)
            wcscpy_s(nid.szInfoTitle, pszTitle);
        else
            nid.szInfoTitle[0] = L'\0';
        nid.hBalloonIcon = hBalloonIcon;
        nid.dwInfoFlags = dwInfoFlags;
        return Shell_NotifyIconW(NIM_MODIFY, &nid);
    }
};
inline UINT CForm::s_uTrayMsg = RegisterWindowMessageW(MSGREG_FORMTRAY);
inline UINT CForm::s_uTaskbarCreatedMsg = RegisterWindowMessageW(L"TaskbarCreated");
ECK_NAMESPACE_END