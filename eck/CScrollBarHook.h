#pragma once
#define ECK_SBHOOK_INCLUDED 1
#include "CWindow.h"
#include "CScrollView.h"

ECK_NAMESPACE_BEGIN
constexpr inline PCWSTR PROP_SBHOOK = L"Eck.Prop.SBHook";

class CScrollBarHook
{
public:
    enum class Part : BYTE
    {
        Invalid,

        ArrowUp,
        ArrowDown,
        ArrowLeft,
        ArrowRight,

        PageUp,
        PageDown,
        PageLeft,
        PageRight,

        ThumbV,
        ThumbH
    };

    enum class HotState : BYTE
    {
        None,
        VSB,
        HSB
    };
private:
    enum : UINT_PTR
    {
        IDT_REPEAT_DELAY = 0x514B * 233,
        IDT_REPEAT,

        TE_REPEAT_DELAY = 350,
        TE_REPEAT = 60,
    };

    CWindow* m_pWnd{};
    HTHEME m_hTheme{};
    CScrollView m_ViewV{}, m_ViewH{}, m_ViewTracking{};
    int m_nLastTrackPos{ INT_MIN };

    RECT m_rcWnd{};// 窗口矩形（相对自身窗口）

    int m_cxVSB{};
    int m_cyHSB{};
    int m_cxSBArrow{};
    int m_cySBArrow{};

    int m_iDpi{ USER_DEFAULT_SCREEN_DPI };

    MARGINS m_Margins{};

    RECT m_rcSBV{};
    RECT m_rcSBH{};

    int m_cyThumbV{};
    int m_cxThumbH{};

    int m_yThumbV{};
    int m_xThumbH{};

    Part m_eHotPart{ Part::Invalid };
    Part m_eLBtnDownPart{ Part::Invalid };
    HotState m_eHotState{ HotState::None };

    BITBOOL m_bProtectStyle : 1{ FALSE };
    BITBOOL m_bLeftScrollBar : 1{ FALSE };
    BITBOOL m_bVisibleH : 1{ TRUE };
    BITBOOL m_bVisibleV : 1{ TRUE };
    BITBOOL m_bDisableH : 1{ FALSE };
    BITBOOL m_bDisableV : 1{ FALSE };

    BITBOOL m_bStdSize : 1{ TRUE };


    void ReCalculateScrollRect() noexcept
    {
        m_rcSBV.right = m_rcWnd.right - m_Margins.cxRightWidth;
        m_rcSBV.left = m_rcSBV.right - m_cxVSB;
        m_rcSBV.top = m_rcWnd.top + m_Margins.cyTopHeight;
        m_rcSBV.bottom = m_rcWnd.bottom - m_Margins.cyBottomHeight;
        if (m_bVisibleH)
            m_rcSBV.bottom -= m_cyHSB;

        m_rcSBH.bottom = m_rcWnd.bottom - m_Margins.cyBottomHeight;
        m_rcSBH.top = m_rcSBH.bottom - m_cyHSB;
        m_rcSBH.left = m_rcWnd.left + m_Margins.cxLeftWidth;
        m_rcSBH.right = m_rcWnd.right - m_Margins.cxRightWidth;
        if (m_bVisibleV)
            m_rcSBH.right -= m_cxVSB;

        m_ViewH.SetViewSize(m_rcSBH.right - m_rcSBH.left - m_cxSBArrow * 2);
        m_ViewV.SetViewSize(m_rcSBV.bottom - m_rcSBV.top - m_cySBArrow * 2);
    }

    void ReCalculateThumbH() noexcept
    {
        m_cxThumbH = m_ViewH.GetThumbSize();
        m_xThumbH = m_ViewH.GetThumbPosition(m_cxThumbH) + m_rcSBH.left + m_cxSBArrow;
    }

    void ReCalculateThumbV() noexcept
    {
        m_cyThumbV = m_ViewV.GetThumbSize();
        m_yThumbV = m_ViewV.GetThumbPosition(m_cyThumbV) + m_rcSBV.top + m_cySBArrow;
    }

    void ReCalculateThumb() noexcept
    {
        ReCalculateThumbH();
        ReCalculateThumbV();
    }

    void ReCalculateTrackingThumbH() noexcept
    {
        m_cxThumbH = m_ViewTracking.GetThumbSize();
        m_xThumbH = m_ViewTracking.GetThumbPosition(m_cxThumbH) + m_rcSBH.left + m_cxSBArrow;
    }
    void ReCalculateTrackingThumbV() noexcept
    {
        m_cyThumbV = m_ViewTracking.GetThumbSize();
        m_yThumbV = m_ViewTracking.GetThumbPosition(m_cyThumbV) + m_rcSBV.top + m_cySBArrow;
    }

    void UpdateVisibleFromStyle() noexcept
    {
        const auto dwStyle = m_pWnd->GetStyle();
        m_bVisibleH = IsBitSet(dwStyle, WS_HSCROLL);
        m_bVisibleV = IsBitSet(dwStyle, WS_VSCROLL);
    }

    void Redraw(HDC hDC) noexcept
    {
        HTHEME hTheme = m_hTheme;
        RECT rc;
        int iState;
        if (m_bVisibleV)
        {
            rc = m_rcSBV;
            // 上箭头
            rc.bottom = rc.top + m_cySBArrow;
            if (m_bDisableV)
                iState = ABS_UPDISABLED;
            else if (m_eLBtnDownPart == Part::ArrowUp)
                iState = ABS_UPPRESSED;
            else if (m_eHotPart == Part::ArrowUp)
                iState = ABS_UPHOT;
            else if (m_eHotState == HotState::VSB)
                iState = ABS_UPHOVER;
            else
                iState = ABS_UPNORMAL;
            DrawThemeBackground(hTheme, hDC, SBP_ARROWBTN, iState, &rc, nullptr);
            // 上空白
            rc.top = rc.bottom;
            rc.bottom = m_yThumbV;
            if (m_bDisableV)
                iState = SCRBS_DISABLED;
            else if (m_eLBtnDownPart == Part::PageUp)
                iState = SCRBS_PRESSED;
            else if (m_eHotPart == Part::PageUp)
                iState = SCRBS_HOT;
            else if (m_eHotState == HotState::VSB)
                iState = SCRBS_HOVER;
            else
                iState = SCRBS_NORMAL;
            DrawThemeBackground(hTheme, hDC, SBP_LOWERTRACKVERT, iState, &rc, nullptr);
            // 滑块
            rc.top = rc.bottom;
            rc.bottom = rc.top + m_cyThumbV;
            if (m_bDisableV)
                iState = SCRBS_DISABLED;
            else if (m_eLBtnDownPart == Part::ThumbV)
                iState = SCRBS_PRESSED;
            else if (m_eHotPart == Part::ThumbV)
                iState = SCRBS_HOT;
            else if (m_eHotState == HotState::VSB)
                iState = SCRBS_HOVER;
            else
                iState = SCRBS_NORMAL;
            DrawThemeBackground(hTheme, hDC, SBP_THUMBBTNVERT, iState, &rc, nullptr);
            // 下空白
            rc.top = rc.bottom;
            rc.bottom = m_rcSBV.bottom - m_cySBArrow;
            if (m_bDisableV)
                iState = SCRBS_DISABLED;
            else if (m_eLBtnDownPart == Part::PageDown)
                iState = SCRBS_PRESSED;
            else if (m_eHotPart == Part::PageDown)
                iState = SCRBS_HOT;
            else if (m_eHotState == HotState::VSB)
                iState = SCRBS_HOVER;
            else
                iState = SCRBS_NORMAL;
            DrawThemeBackground(hTheme, hDC, SBP_UPPERTRACKVERT, iState, &rc, nullptr);
            // 下箭头
            rc.top = rc.bottom;
            rc.bottom = m_rcSBV.bottom;
            if (m_bDisableV)
                iState = ABS_DOWNDISABLED;
            else if (m_eLBtnDownPart == Part::ArrowDown)
                iState = ABS_DOWNPRESSED;
            else if (m_eHotPart == Part::ArrowDown)
                iState = ABS_DOWNHOT;
            else if (m_eHotState == HotState::VSB)
                iState = ABS_DOWNHOVER;
            else
                iState = ABS_DOWNNORMAL;
            DrawThemeBackground(hTheme, hDC, SBP_ARROWBTN, iState, &rc, nullptr);
        }

        if (m_bVisibleH)
        {
            rc = m_rcSBH;
            // 左箭头
            rc.right = rc.left + m_cxSBArrow;
            if (m_bDisableH)
                iState = ABS_LEFTDISABLED;
            else if (m_eLBtnDownPart == Part::ArrowLeft)
                iState = ABS_LEFTPRESSED;
            else if (m_eHotPart == Part::ArrowLeft)
                iState = ABS_LEFTHOT;
            else if (m_eHotState == HotState::HSB)
                iState = ABS_LEFTHOVER;
            else
                iState = ABS_LEFTNORMAL;
            DrawThemeBackground(hTheme, hDC, SBP_ARROWBTN, iState, &rc, nullptr);
            // 左空白
            rc.left = rc.right;
            rc.right = m_xThumbH;
            if (m_bDisableH)
                iState = SCRBS_DISABLED;
            else if (m_eLBtnDownPart == Part::PageLeft)
                iState = SCRBS_PRESSED;
            else if (m_eHotPart == Part::PageLeft)
                iState = SCRBS_HOT;
            else if (m_eHotState == HotState::HSB)
                iState = SCRBS_HOVER;
            else
                iState = SCRBS_NORMAL;
            DrawThemeBackground(hTheme, hDC, SBP_LOWERTRACKHORZ, iState, &rc, nullptr);
            // 滑块
            rc.left = rc.right;
            rc.right = rc.left + m_cxThumbH;
            if (m_bDisableH)
                iState = SCRBS_DISABLED;
            else if (m_eLBtnDownPart == Part::ThumbH)
                iState = SCRBS_PRESSED;
            else if (m_eHotPart == Part::ThumbH)
                iState = SCRBS_HOT;
            else if (m_eHotState == HotState::HSB)
                iState = SCRBS_HOVER;
            else
                iState = SCRBS_NORMAL;
            DrawThemeBackground(hTheme, hDC, SBP_THUMBBTNHORZ, iState, &rc, nullptr);
            // 右空白
            rc.left = rc.right;
            rc.right = m_rcSBH.right - m_cxSBArrow;
            if (m_bDisableH)
                iState = SCRBS_DISABLED;
            else if (m_eLBtnDownPart == Part::PageRight)
                iState = SCRBS_PRESSED;
            else if (m_eHotPart == Part::PageRight)
                iState = SCRBS_HOT;
            else if (m_eHotState == HotState::HSB)
                iState = SCRBS_HOVER;
            else
                iState = SCRBS_NORMAL;
            DrawThemeBackground(hTheme, hDC, SBP_UPPERTRACKHORZ, iState, &rc, nullptr);
            // 右箭头
            rc.left = rc.right;
            rc.right = m_rcSBH.right;
            if (m_bDisableH)
                iState = ABS_RIGHTDISABLED;
            else if (m_eLBtnDownPart == Part::ArrowRight)
                iState = ABS_RIGHTPRESSED;
            else if (m_eHotPart == Part::ArrowRight)
                iState = ABS_RIGHTHOT;
            else if (m_eHotState == HotState::HSB)
                iState = ABS_RIGHTHOVER;
            else
                iState = ABS_RIGHTNORMAL;
            DrawThemeBackground(hTheme, hDC, SBP_ARROWBTN, iState, &rc, nullptr);
        }

        if (m_bVisibleV && m_bVisibleH &&
            ShouldWindowDisplaySizeGrip(m_pWnd->HWnd, m_iDpi))
        {
            rc.left = m_rcSBH.right;
            rc.top = m_rcSBV.bottom;
            rc.right = m_rcSBV.right;
            rc.bottom = m_rcSBH.bottom;
            DrawThemeBackground(hTheme, hDC, SBP_SIZEBOX, SZB_RIGHTALIGN, &rc, nullptr);
        }
    }

    void Redraw() noexcept
    {
        const auto hDC = GetWindowDC(m_pWnd->HWnd);
        Redraw(hDC);
        ReleaseDC(m_pWnd->HWnd, hDC);
    }

    void GenerateScrollMessage(Part ePart) noexcept
    {
        UINT uCode;
        switch (ePart)
        {
        case Part::ArrowUp:		uCode = SB_LINEUP;		goto GenV;
        case Part::ArrowDown:	uCode = SB_LINEDOWN;	goto GenV;
        case Part::PageUp:		uCode = SB_PAGEUP;		goto GenV;
        case Part::PageDown:	uCode = SB_PAGEDOWN;	goto GenV;
        case Part::ArrowLeft:	uCode = SB_LINELEFT;	goto GenH;
        case Part::ArrowRight:	uCode = SB_LINERIGHT;	goto GenH;
        case Part::PageLeft:	uCode = SB_PAGELEFT;	goto GenH;
        case Part::PageRight:	uCode = SB_PAGERIGHT;	goto GenH;
        default:				return;
        }
    GenH:;
        m_pWnd->SendMessage(WM_HSCROLL, MAKEWPARAM(uCode, 0), 0);
        return;
    GenV:;
        m_pWnd->SendMessage(WM_VSCROLL, MAKEWPARAM(uCode, 0), 0);
    }

    void CancelRepeat() noexcept
    {
        KillTimer(m_pWnd->HWnd, IDT_REPEAT_DELAY);
        KillTimer(m_pWnd->HWnd, IDT_REPEAT);
    }

    void Cleanup() noexcept
    {
        CloseThemeData(m_hTheme);
        m_hTheme = nullptr;
        m_eHotPart = m_eLBtnDownPart = Part::Invalid;
        m_eHotState = HotState::None;
        m_pWnd = nullptr;
    }
public:
    LRESULT OnWindowMessage(HWND hWnd, UINT uMsg,
        WPARAM wParam, LPARAM lParam, SlotCtx& Ctx) noexcept
    {
        if (Ctx.IsDeleting())
        {
            Cleanup();
            return 0;
        }
        switch (uMsg)
        {
        case WM_NCHITTEST:
        {
            if (!m_bVisibleH && !m_bVisibleV)
                break;
            POINT pt ECK_GET_PT_LPARAM(lParam);
            ScreenToClient(hWnd, &pt);
            if (m_bVisibleV && PtInRect(m_rcSBV, pt))
            {
                Ctx.Processed();
                return HTVSCROLL;
            }
            if (m_bVisibleH && PtInRect(m_rcSBH, pt))
            {
                Ctx.Processed();
                return HTHSCROLL;
            }
        }
        break;

        case WM_MOUSEMOVE:
        {
            if (m_eLBtnDownPart == Part::ThumbV)
            {
                Ctx.Processed();
                m_ViewTracking.OnMouseMove(GET_Y_LPARAM(lParam) + m_Margins.cyTopHeight
                    - m_rcSBV.top - m_cySBArrow);
                ReCalculateTrackingThumbV();
                Redraw();
                if (m_ViewTracking.GetPosition() != m_nLastTrackPos)
                {
                    m_pWnd->SendMessage(WM_VSCROLL, MAKEWPARAM(SB_THUMBTRACK, m_ViewTracking.GetPosition()), 0);
                    m_nLastTrackPos = m_ViewTracking.GetPosition();
                }
                return 0;
            }
            else if (m_eLBtnDownPart == Part::ThumbH)
            {
                Ctx.Processed();
                m_ViewTracking.OnMouseMove(GET_X_LPARAM(lParam) + m_Margins.cxLeftWidth
                    - m_rcSBH.left - m_cxSBArrow);
                ReCalculateTrackingThumbH();
                Redraw();
                if (m_ViewTracking.GetPosition() != m_nLastTrackPos)
                {
                    m_pWnd->SendMessage(WM_HSCROLL, MAKEWPARAM(SB_THUMBTRACK, m_ViewTracking.GetPosition()), 0);
                    m_nLastTrackPos = m_ViewTracking.GetPosition();
                }
                return 0;
            }
        }
        break;

        case WM_NCLBUTTONUP:
        case WM_LBUTTONUP:
        {
            if (m_eLBtnDownPart != Part::Invalid)
            {
                CancelRepeat();
                Ctx.Processed();// Eat it.
                if (m_eLBtnDownPart == Part::ThumbV)
                {
                    m_ViewTracking.OnLButtonUp();
                    m_pWnd->SendMessage(WM_VSCROLL, MAKEWPARAM(SB_THUMBPOSITION, m_ViewTracking.GetPosition()), 0);
                    m_pWnd->SendMessage(WM_VSCROLL, MAKEWPARAM(SB_ENDSCROLL, 0), 0);
                }
                else if (m_eLBtnDownPart == Part::ThumbH)
                {
                    m_ViewTracking.OnLButtonUp();
                    m_pWnd->SendMessage(WM_HSCROLL, MAKEWPARAM(SB_THUMBPOSITION, m_ViewTracking.GetPosition()), 0);
                    m_pWnd->SendMessage(WM_HSCROLL, MAKEWPARAM(SB_ENDSCROLL, 0), 0);
                }
                m_eLBtnDownPart = Part::Invalid;
                ReleaseCapture();

                POINT pt ECK_GET_PT_LPARAM(lParam);
                if (uMsg == WM_NCLBUTTONUP)
                    ScreenToClient(hWnd, &pt);
                const auto ePart = HitTest(pt);
                const auto eHot = (
                    ePart == Part::ThumbV ? HotState::VSB :
                    ePart == Part::ThumbH ? HotState::HSB : HotState::None);
                if (eHot != m_eHotState)
                {
                    m_eHotState = eHot;
                    Redraw();
                }
                return 0;
            }
        }
        break;

        case WM_TIMER:
        {
            if (wParam != IDT_REPEAT)
                break;
            Ctx.Processed();
            POINT pt;
            GetCursorPos(&pt);
            ScreenToClient(hWnd, &pt);
            GenerateScrollMessage(HitTest(pt));
        }
        break;

        case WM_WINDOWPOSCHANGED:
        {
            const auto* const pwp = (WINDOWPOS*)lParam;
            m_rcWnd.left = 0;
            m_rcWnd.top = 0;
            m_rcWnd.right = pwp->cx;
            m_rcWnd.bottom = pwp->cy;
            ReCalculateScrollRect();
            ReCalculateThumb();
        }
        break;

        case WM_NCCALCSIZE:
        {
            Ctx.Processed();
            const auto dwStyle = m_pWnd->GetStyle();
            if (dwStyle & (WS_HSCROLL | WS_VSCROLL))
            {
                m_bProtectStyle = TRUE;
                m_pWnd->SetStyle(dwStyle & ~(WS_HSCROLL | WS_VSCROLL));
            }
            const RECT rcWnd{ *(RECT*)lParam };
            m_pWnd->OnMessage(hWnd, uMsg, wParam, lParam);
            if (dwStyle & (WS_HSCROLL | WS_VSCROLL))
            {
                m_pWnd->SetStyle(dwStyle);
                m_bProtectStyle = FALSE;
            }
            const auto prcClient = (RECT*)lParam;
            m_Margins.cxLeftWidth = prcClient->left - rcWnd.left;
            m_Margins.cxRightWidth = rcWnd.right - prcClient->right;
            m_Margins.cyTopHeight = prcClient->top - rcWnd.top;
            m_Margins.cyBottomHeight = rcWnd.bottom - prcClient->bottom;

            if (m_bVisibleV && (prcClient->right - prcClient->left) > m_cxVSB)
                prcClient->right -= m_cxVSB;
            if (m_bVisibleH && (prcClient->bottom - prcClient->top) > m_cyHSB)
                prcClient->bottom -= m_cyHSB;

            ReCalculateScrollRect();
            ReCalculateThumb();
        }
        return 0;

        case WM_NCPAINT:
        {
            Ctx.Processed();
            auto Ctx1{ Ctx };
            m_pWnd->GetEventChain().CallNext(Ctx1, hWnd, uMsg, wParam, lParam);
            if (!Ctx1.IsProcessed())
                m_pWnd->OnMessage(hWnd, uMsg, wParam, lParam);
            Redraw();
        }
        return 0;

        case WM_NCLBUTTONDOWN:
        case WM_NCLBUTTONDBLCLK:
        {
            if (wParam != HTVSCROLL && wParam != HTHSCROLL)
                break;
            Ctx.Processed();
            POINT pt ECK_GET_PT_LPARAM(lParam);
            ScreenToClient(hWnd, &pt);
            const auto ePart = HitTest(pt);
            m_eLBtnDownPart = ePart;
            if (wParam == HTVSCROLL)
            {
                if (ePart == Part::ThumbV)
                {
                    m_ViewTracking = m_ViewV;
                    m_nLastTrackPos = m_ViewTracking.GetPosition();
                    m_ViewTracking.OnLButtonDown(pt.y + m_Margins.cyTopHeight
                        - m_rcSBV.top - m_cySBArrow);
                }
            }
            else
            {
                if (ePart == Part::ThumbH)
                {
                    m_ViewTracking = m_ViewH;
                    m_nLastTrackPos = m_ViewTracking.GetPosition();
                    m_ViewTracking.OnLButtonDown(pt.x + m_Margins.cxLeftWidth
                        - m_rcSBH.left - m_cxSBArrow);
                }
            }
            GenerateScrollMessage(ePart);
            SetCapture(hWnd);
            CancelRepeat();
            if (m_eLBtnDownPart != Part::Invalid &&
                m_eLBtnDownPart != Part::ThumbV &&
                m_eLBtnDownPart != Part::ThumbH)
            {
                SetTimer(hWnd, IDT_REPEAT_DELAY, TE_REPEAT_DELAY,
                    [](HWND hWnd, UINT, UINT_PTR uId, DWORD)
                    {
                        KillTimer(hWnd, uId);
                        SetTimer(hWnd, IDT_REPEAT, TE_REPEAT, nullptr);
                    });
                Redraw();
            }
        }
        return 0;

        case WM_NCMOUSEMOVE:
        {
            Ctx.Processed();
            if (m_eLBtnDownPart != Part::Invalid)
                break;
            const auto eOldHot = m_eHotState;
            switch (wParam)
            {
            case HTVSCROLL:
                m_eHotState = HotState::VSB;
                break;
            case HTHSCROLL:
                m_eHotState = HotState::HSB;
                break;
            default:
                m_eHotState = HotState::None;
                break;
            }
            TRACKMOUSEEVENT tme;
            tme.cbSize = sizeof(tme);
            tme.dwFlags = TME_LEAVE | TME_NONCLIENT;
            tme.hwndTrack = hWnd;
            TrackMouseEvent(&tme);
            if (eOldHot != m_eHotState)
                Redraw();
        }
        return 0;

        case WM_NCMOUSELEAVE:
        {
            if (m_eLBtnDownPart != Part::Invalid)
                break;
            if (m_eHotState != HotState::None)
            {
                m_eHotState = HotState::None;
                Redraw();
            }
        }
        return 0;

        case WM_STYLECHANGING:
            Ctx.Processed(m_bProtectStyle);
            return 0;

        case WM_STYLECHANGED:
        {
            Ctx.Processed(m_bProtectStyle);
            const auto* const pss = (STYLESTRUCT*)lParam;
            if (wParam == GWL_STYLE)
            {
                const auto bVisibleH = IsBitSet(pss->styleNew, WS_HSCROLL);
                const auto bVisibleV = IsBitSet(pss->styleNew, WS_VSCROLL);
                if (m_bVisibleH != bVisibleH || m_bVisibleV != bVisibleV)
                {
                    m_bVisibleH = bVisibleH;
                    m_bVisibleV = bVisibleV;
                    ReCalculateScrollRect();
                    ReCalculateThumb();
                    Redraw();
                }
            }
        }
        break;

        case WM_THEMECHANGED:
        {
            CloseThemeData(m_hTheme);
            m_hTheme = OpenNcThemeData(m_pWnd->HWnd, L"ScrollBar");
            Redraw();
        }
        break;

        case WM_DPICHANGED:
        case WM_DPICHANGED_BEFOREPARENT:
        {
            m_iDpi = GetDpi(hWnd);
            if (m_bStdSize)
                UpdateStandardSize();
        }
        break;

        case WM_DESTROY:
            Cleanup();
            break;
        }
        return 0;
    }

    HRESULT Attach(CWindow* pWnd) noexcept
    {
        if (pWnd->GetEventChain().FindSlot(MHI_SCROLLBAR_HOOK))
            return S_FALSE;
        SetPropW(pWnd->HWnd, PROP_SBHOOK, this);
        m_pWnd = pWnd;
        m_iDpi = GetDpi(pWnd->HWnd);

        UpdateStandardSize();

        SCROLLINFO si{ sizeof(si), SIF_ALL };
        pWnd->ScbGetInfomation(SB_HORZ, &si);
        m_ViewH.SetScrollInfomation(si);
        pWnd->ScbGetInfomation(SB_VERT, &si);
        m_ViewV.SetScrollInfomation(si);

        m_hTheme = OpenNcThemeData(pWnd->HWnd, L"ScrollBar");

        GetWindowRect(pWnd->HWnd, &m_rcWnd);
        m_rcWnd.right -= m_rcWnd.left;
        m_rcWnd.bottom -= m_rcWnd.top;
        m_rcWnd.left = 0;
        m_rcWnd.top = 0;

        pWnd->GetEventChain().Connect(this, &CScrollBarHook::OnWindowMessage, MHI_SCROLLBAR_HOOK);
        pWnd->FrameChanged();

        UpdateVisibleFromStyle();
        ReCalculateScrollRect();
        ReCalculateThumb();
        return S_OK;
    }

    HRESULT Detach() noexcept
    {
        if (!m_pWnd)
            return S_FALSE;
        const auto hSlot = m_pWnd->GetEventChain().FindSlot(MHI_SCROLLBAR_HOOK);
        if (!hSlot)
            return S_FALSE;
        m_pWnd->GetEventChain().Disconnect(hSlot);
        m_pWnd->FrameChanged();
        m_pWnd = nullptr;
        return S_OK;
    }

    Part HitTest(POINT pt) noexcept
    {
        if (m_bVisibleV && PtInRect(m_rcSBV, pt))
        {
            /*
            * ---	m_rcSBV.top
            *  ↑
            * ---   ySpace1
            *
            * ---	m_yThumb
            *  T
            *  h
            *  u
            *  m
            *  b
            * ---   ySpace2
            *
            * ---	yArrow2
            *  ↓
            * ---	m_rcSBV.bottom
            */

            // ySpace1 = m_rcSBV.top + m_cySBArrow
            // ySpace2 = m_yThumb + m_cyVThumb
            // yArrow2 = m_rcSBV.bottom - m_cySBArrow

            if (pt.y < m_rcSBV.top + m_cySBArrow)
                return Part::ArrowUp;
            if (pt.y < m_yThumbV)
                return Part::PageUp;
            else if (pt.y < m_yThumbV + m_cyThumbV)
                return Part::ThumbV;
            else if (pt.y < m_rcSBV.bottom - m_cySBArrow)
                return Part::PageDown;
            else// if (pt.y < m_rcSBV.bottom)
                return Part::ArrowDown;
        }
        else if (m_bVisibleH && PtInRect(&m_rcSBH, pt))
        {
            // m_rcSBH.left    m_xThumb	           m_rcSBH.right
            // |←|            | Thumb |        |→|
            //	  xSpace1			   xSpace2  xArrow2

            // xSpace1 = m_rcSBH.left + m_cxSBArrow
            // xSpace2 = m_xThumb + m_cxHThumb
            // xArrow2 = m_rcSBH.right - m_cxSBArrow

            if (pt.x < m_rcSBH.left + m_cxSBArrow)
                return Part::ArrowLeft;
            else if (pt.x < m_xThumbH)
                return Part::PageLeft;
            else if (pt.x < m_xThumbH + m_cxThumbH)
                return Part::ThumbH;
            else if (pt.x < m_rcSBH.right - m_cxSBArrow)
                return Part::PageRight;
            else// if (pt.x < m_rcSBH.right)
                return Part::ArrowRight;
        }
        return Part::Invalid;
    }

    int HkSetScrollInfo(HWND hWnd, int nBar, const SCROLLINFO* psi, BOOL bRedraw) noexcept
    {
        switch (nBar)
        {
        case SB_HORZ:
            m_ViewH.SetScrollInfomation(*psi);
            m_bDisableH = FALSE;
            if (!m_ViewH.IsVisible())
            {
                if (psi->fMask & SIF_DISABLENOSCROLL)
                    m_bDisableH = TRUE;
                else if (m_bVisibleH)
                    HkShowScrollBar(hWnd, SB_HORZ, FALSE);
            }
            else if (!m_bVisibleH)
                HkShowScrollBar(hWnd, SB_HORZ, TRUE);
            ReCalculateThumbH();
            if (bRedraw)
                Redraw();
            return m_ViewH.GetPosition();
        case SB_VERT:
            m_ViewV.SetScrollInfomation(*psi);
            m_bDisableV = FALSE;
            if (!m_ViewV.IsVisible())
            {
                if (psi->fMask & SIF_DISABLENOSCROLL)
                    m_bDisableV = TRUE;
                else if (m_bVisibleV)
                    HkShowScrollBar(hWnd, SB_VERT, FALSE);
            }
            else if (!m_bVisibleV)
                HkShowScrollBar(hWnd, SB_VERT, TRUE);
            ReCalculateThumbV();
            if (bRedraw)
                Redraw();
            return m_ViewV.GetPosition();
        }
        return 0;
    }

    BOOL HkGetScrollInfo(HWND hWnd, int nBar, SCROLLINFO* psi) noexcept
    {
        switch (nBar)
        {
        case SB_HORZ:
            m_ViewH.GetScrollInfomation(*psi);
            if ((psi->fMask & SIF_TRACKPOS) && m_eLBtnDownPart == Part::ThumbH)
                psi->nTrackPos = m_ViewTracking.GetPosition();
            return TRUE;
        case SB_VERT:
            m_ViewV.GetScrollInfomation(*psi);
            if ((psi->fMask & SIF_TRACKPOS) && m_eLBtnDownPart == Part::ThumbV)
                psi->nTrackPos = m_ViewTracking.GetPosition();
            return TRUE;
        }
        return FALSE;
    }

    BOOL HkShowScrollBar(HWND hWnd, int nBar, BOOL bShow) noexcept
    {
        switch (nBar)
        {
        case SB_HORZ:
        {
            const auto dwStyle = m_pWnd->GetStyle();
            if (bShow)
                if (dwStyle & WS_HSCROLL)
                    return TRUE;
                else
                    m_pWnd->SetStyle(dwStyle | WS_HSCROLL);
            else
                if (dwStyle & WS_HSCROLL)
                    m_pWnd->SetStyle(dwStyle & ~WS_HSCROLL);
                else
                    return TRUE;
            m_pWnd->FrameChanged();
        }
        return TRUE;
        case SB_VERT:
        {
            const auto dwStyle = m_pWnd->GetStyle();
            if (bShow)
                if (dwStyle & WS_VSCROLL)
                    return TRUE;
                else
                    m_pWnd->SetStyle(dwStyle | WS_VSCROLL);
            else
                if (dwStyle & WS_VSCROLL)
                    m_pWnd->SetStyle(dwStyle & ~WS_VSCROLL);
                else
                    return TRUE;
            m_pWnd->FrameChanged();
        }
        return TRUE;
        }
        return FALSE;
    }

    void UpdateStandardSize() noexcept
    {
        if (!m_bStdSize)
            return;
        m_cxVSB = DaGetSystemMetrics(SM_CXVSCROLL, m_iDpi);
        m_cyHSB = DaGetSystemMetrics(SM_CYHSCROLL, m_iDpi);
        m_cxSBArrow = DaGetSystemMetrics(SM_CXHSCROLL, m_iDpi);
        m_cySBArrow = DaGetSystemMetrics(SM_CYVSCROLL, m_iDpi);
    }

    EckInlineCe void SetStandardSize(BOOL bStdSize) noexcept { m_bStdSize = bStdSize; }
    EckInlineNdCe BOOL GetStandardSize() const noexcept { return m_bStdSize; }

    EckInlineNdCe HTHEME GetHTheme() const noexcept { return m_hTheme; }
};
ECK_NAMESPACE_END