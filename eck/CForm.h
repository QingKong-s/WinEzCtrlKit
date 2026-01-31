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
    HBITMAP m_hbmBk{};
    int m_cxImage{},
        m_cyImage{};

    COLORREF m_crBk{ CLR_DEFAULT };

    int m_cxClient{},
        m_cyClient{};

    BkImgMode m_eBkImageMode{ BkImgMode::TopLeft };

    BITBOOL m_bMoveable : 1 = TRUE;
    BITBOOL m_bFillWndImage : 1 = FALSE;
    BITBOOL m_bEscClose : 1 = FALSE;
    BITBOOL m_bMoveAnywhere : 1 = FALSE;
    BITBOOL m_bClrDisableEdit : 1 = FALSE;
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
                    m_eBkImageMode, m_bFillWndImage);
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
        }

        return __super::OnMessage(hWnd, uMsg, wParam, lParam);
    }

    EckInline void SetBackgroundImage(HBITMAP hbmBk) noexcept
    {
        m_hbmBk = hbmBk;
        BITMAP bm;
        GetObjectW(hbmBk, sizeof(bm), &bm);
        m_cxImage = bm.bmWidth;
        m_cyImage = bm.bmHeight;
    }
    EckInlineNdCe HBITMAP GetBackgroundImage() const noexcept { return m_hbmBk; }

    EckInlineCe void SetBackgroundMode(BkImgMode iMode) noexcept { m_eBkImageMode = iMode; }
    EckInlineNdCe BkImgMode GetBackgroundMode() const noexcept { return m_eBkImageMode; }

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
};
ECK_NAMESPACE_END