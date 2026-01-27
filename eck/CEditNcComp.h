#pragma once
#include "CEditExt.h"

ECK_NAMESPACE_BEGIN
class CEditNcComp : public CEditExt
{
public:
    ECK_RTTI(CEditNcComp, CEditExt);
protected:
    RECT m_rcBtn{};
    int m_cxBtn{};

    BITBOOL m_bBtnHot : 1 = FALSE;
    BITBOOL m_bLBtnDown : 1 = FALSE;

    EckInline void FixCursorPosition(POINT& pt) noexcept
    {
        pt.x += m_rcMargins.left;
        pt.y += m_rcMargins.top;
    }

    EckInline void RedrawButton()
    {
        SendMsg(WM_NCPAINT, 0, 0);
    }

    void CleanupForDestroyWindow()
    {
        m_cxBtn = 0;
        m_rcBtn = {};
        m_bBtnHot = FALSE;
        m_bLBtnDown = FALSE;
    }
public:
    void DetachNew() noexcept override
    {
        CEditExt::DetachNew();
        CleanupForDestroyWindow();
    }

    LRESULT OnMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept override
    {
        switch (uMsg)
        {
        case WM_NCHITTEST:
        {
            POINT pt ECK_GET_PT_LPARAM(lParam);
            ScreenToClient(hWnd, &pt);
            FixCursorPosition(pt);
            if (PtInRect(m_rcBtn, pt))
                return HTCLIENT;
        }
        break;

        case WM_NCCALCSIZE:
        {
            const auto lResult = CEditExt::OnMessage(hWnd, uMsg, wParam, lParam);
            const auto prcClient = (RECT*)lParam;

            int cyWnd;
            if (wParam)
                cyWnd = ((NCCALCSIZE_PARAMS*)lParam)->lppos->cy;
            else
            {
                RECT rc;
                GetWindowRect(hWnd, &rc);
                cyWnd = rc.bottom - rc.top;
            }

            const int cxBtn = (m_cxBtn ? m_cxBtn : DpiScale(20, GetDpi(hWnd)));
            m_rcBtn.right = m_rcMargins.left + (prcClient->right - prcClient->left);
            m_rcBtn.left = m_rcBtn.right - cxBtn;
            m_rcBtn.top = m_rcMargins.top;
            m_rcBtn.bottom = cyWnd - m_rcMargins.bottom;

            prcClient->right -= cxBtn;
            return lResult;
        }
        break;

        case WM_NCPAINT:
        {
            const auto lResult = CEditExt::OnMessage(hWnd, uMsg, wParam, lParam);
            const HDC hDC = GetWindowDC(hWnd);
            OnPaintButton(hDC);
            ReleaseDC(hWnd, hDC);
            return lResult;
        }
        break;

        case WM_MOUSEMOVE:
        {
            POINT pt ECK_GET_PT_LPARAM(lParam);
            FixCursorPosition(pt);
            if (PtInRect(m_rcBtn, pt))
            {
                m_bBtnHot = TRUE;
                RedrawButton();

                TRACKMOUSEEVENT tme;
                tme.cbSize = sizeof(tme);
                tme.dwFlags = TME_LEAVE;
                tme.hwndTrack = hWnd;
                TrackMouseEvent(&tme);
                return 0;
            }
            else if (m_bBtnHot)
            {
                m_bBtnHot = FALSE;
                RedrawButton();
            }
        }
        break;

        case WM_MOUSELEAVE:
        {
            if (m_bBtnHot)
            {
                m_bBtnHot = FALSE;
                RedrawButton();
                return 0;
            }
        }
        break;

        case WM_LBUTTONDBLCLK:
        case WM_LBUTTONDOWN:
        {
            POINT pt ECK_GET_PT_LPARAM(lParam);
            FixCursorPosition(pt);
            if (PtInRect(m_rcBtn, pt))
            {
                SetCapture(hWnd);
                m_bLBtnDown = TRUE;
                RedrawButton();
                return 0;
            }
        }
        break;

        case WM_LBUTTONUP:
        {
            if (m_bLBtnDown)
            {
                m_bLBtnDown = FALSE;
                ReleaseCapture();
                RedrawButton();

                POINT pt ECK_GET_PT_LPARAM(lParam);
                FixCursorPosition(pt);
                if (PtInRect(m_rcBtn, pt))
                {
                    OnButtonClick();
                    return 0;
                }
            }
        }
        break;

        case WM_CAPTURECHANGED:
        {
            if (m_bLBtnDown)
            {
                m_bLBtnDown = FALSE;
                RedrawButton();
            }
        }
        break;

        case WM_DESTROY:
            CleanupForDestroyWindow();
            break;
        }
        return CEditExt::OnMessage(hWnd, uMsg, wParam, lParam);
    }

    virtual void OnPaintButton(HDC hDC) noexcept
    {
        COLORREF crOld;
        if (m_bLBtnDown)
        {
            FillRect(hDC, &m_rcBtn, GetSysColorBrush(COLOR_HIGHLIGHT));
            crOld = SetTextColor(hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
        }
        else if (m_bBtnHot)
        {
            FillRect(hDC, &m_rcBtn, GetSysColorBrush(COLOR_HOTLIGHT));
            crOld = SetTextColor(hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
        }
        else
        {
            const auto* const ptc = PtcCurrent();
            SetDCBrushColor(hDC, m_crBK != CLR_DEFAULT ? m_crBK : ptc->crDefBkg);
            FillRect(hDC, &m_rcBtn, GetStockBrush(DC_BRUSH));
            crOld = SetTextColor(hDC, m_crText != CLR_DEFAULT ? m_crText : ptc->crDefText);
        }

        const int iOldMode = SetBkMode(hDC, TRANSPARENT);
        DrawTextW(hDC, L"...", -1, &m_rcBtn,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP | DT_NOPREFIX);
        SetBkMode(hDC, iOldMode);

        SetTextColor(hDC, crOld);
    }

    virtual void OnButtonClick() noexcept
    {

    }

    EckInline void SetButtonSize(int i) noexcept
    {
        m_cxBtn = i;
        FrameChanged();
    }

    EckInline int GetButtonSize() const noexcept { return m_cxBtn; }
};
ECK_NAMESPACE_END