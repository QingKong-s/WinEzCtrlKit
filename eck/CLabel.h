#pragma once
#include "CWindow.h"
#include "GraphicsHelper.h"

ECK_NAMESPACE_BEGIN
struct LA_HITTEST
{
    BITBOOL bHit : 1;
    BITBOOL bHitText : 1;
    BITBOOL bHitImg : 1;
};


class CLabel : public CWindow
{
public:
    ECK_RTTI(CLabel, CWindow);
    ECK_CWND_SINGLEOWNER(CLabel);
    ECK_CWND_CREATE_CLS_HINST(WCN_LABEL, g_hInstance);

    constexpr static std::array<COLORREF, 3> DefaultGradient{ 0x808080,0xFFFFFF,0x808080 };
    
    enum class Ellipsis :BYTE
    {
        None,
        End,
        Path,
        Word
    };

    enum class Prefix :BYTE
    {
        Normal,
        NoPrefix,
        HidePrefix,
        PrefixOnly,
    };

    enum class MouseOpt :BYTE
    {
        None,
        TransparentSpace,
        Transparent,
    };
private:
    CEzCDC m_DC{};			// 兼容DC
    HBITMAP m_hbmImg{};		// 图片，无需销毁
    HBITMAP m_hbmBkImg{};	// 底图，无需销毁
    int m_cxBkImg{},
        m_cyBkImg{};		// 底图大小
    HFONT m_hFont{};		// 字体，无需销毁
    CStringW m_rsText{};

    BkImgMode m_eBkImgMode{ BkImgMode::TopLeft };		// 底图模式
    GradientMode m_eGradientMode{ GradientMode::None };	// 渐变模式
    Align m_eAlignH{};	// 横向对齐
    Align m_eAlignV{};	// 纵向对齐

    Ellipsis m_eEllipsisMode{};		// 省略号模式
    Prefix m_ePrefixMode{};			// 前缀模式
    MouseOpt m_eMouseOption{};		// 鼠标穿透
    BITBOOL m_bAutoWrap : 1{};		// 自动换行
    BITBOOL m_bFillWndImg : 1{};	// 底图尽量充满控件
    BITBOOL m_bTransparent : 1{};	// 透明标签
    BITBOOL m_bImgAlpha : 1{};		// 图片需要Alpha混合
    BITBOOL m_bBkImgAlpha : 1{};	// 底图需要Alpha混合

    BITBOOL m_bPartMetricsDirty : 1{ TRUE };// 部件矩形需要重新计算

    COLORREF m_crText{ CLR_DEFAULT };	// 文本颜色
    COLORREF m_crTextBk{ CLR_DEFAULT };	// 文本背景颜色
    COLORREF m_crBk{ CLR_DEFAULT };		// 背景颜色
    std::array<COLORREF, 3> m_crGradient{ DefaultGradient };

    int m_cxClient{},
        m_cyClient{};

    RECT m_rcPartImg{};	// 缓存的图片矩形
    RECT m_rcPartText{};// 缓存的文本矩形

    constexpr UINT DtEllipsis() const noexcept
    {
        switch (m_eEllipsisMode)
        {
        case Ellipsis::End:	return DT_END_ELLIPSIS; break;
        case Ellipsis::Path:return DT_PATH_ELLIPSIS; break;
        case Ellipsis::Word:return DT_WORD_ELLIPSIS; break;
        }
        return 0u;
    }

    constexpr UINT DtPrefix() const noexcept
    {
        switch (m_ePrefixMode)
        {
        case Prefix::NoPrefix:	return DT_NOPREFIX; break;
        case Prefix::HidePrefix:return DT_HIDEPREFIX; break;
        case Prefix::PrefixOnly:return DT_PREFIXONLY; break;
        }
        return 0u;
    }

    constexpr UINT DtAlignH() const noexcept
    {
        switch (m_eAlignH)
        {
        case Align::Center:	return DT_CENTER; break;
        case Align::Far:	return DT_RIGHT; break;
        }
        return DT_LEFT;
    }

    void Paint(HDC hDC, const RECT& rcPaint) noexcept
    {
        if (m_bPartMetricsDirty)
        {
            m_bPartMetricsDirty = FALSE;
            CalculatePartsRect();
        }
        const auto hCDC = (m_hbmImg || m_hbmBkImg) ?
            CreateCompatibleDC(hDC) : nullptr;
        RECT rc{ 0,0,m_cxClient,m_cyClient };
        // 画纯色背景
        if (!m_bTransparent)
        {
            FillRect(hDC, &rc, GetStockBrush(DC_BRUSH));
        }
        // 画渐变背景或底图
        if (m_eGradientMode != GradientMode::None)
            FillGradientRect(hDC, rc, m_crGradient.data(), m_eGradientMode);
        else if (m_hbmBkImg)
        {
            SelectObject(hCDC, m_hbmBkImg);
            if (m_bBkImgAlpha)
                DrawBackgroundImage32(hDC, hCDC, rc, m_cxBkImg, m_cyBkImg,
                    m_eBkImgMode, m_bFillWndImg);
            else
                DrawBackgroundImage(hDC, hCDC, rc, m_cxBkImg, m_cyBkImg,
                    m_eBkImgMode, m_bFillWndImg);
        }
        // 画图片
        if (m_hbmImg)
        {
            SelectObject(hCDC, m_hbmImg);
            if (m_bImgAlpha)
                AlphaBlend(hDC,
                    m_rcPartImg.left,
                    m_rcPartImg.top,
                    m_rcPartImg.right - m_rcPartImg.left,
                    m_rcPartImg.bottom - m_rcPartImg.top,
                    hCDC, 0, 0,
                    m_rcPartImg.right - m_rcPartImg.left,
                    m_rcPartImg.bottom - m_rcPartImg.top, BlendFuncAlpha);
            else
                BitBlt(hDC,
                    m_rcPartImg.left,
                    m_rcPartImg.top,
                    m_rcPartImg.right - m_rcPartImg.left,
                    m_rcPartImg.bottom - m_rcPartImg.top, hCDC, 0, 0, SRCCOPY);
        }
        // 画文本
        const UINT uDTFlags = DT_NOCLIP | DtEllipsis() | DtPrefix() | DtAlignH() |
            (m_bAutoWrap ? DT_WORDBREAK : DT_SINGLELINE);
        DrawTextW(hDC, m_rsText.Data(), m_rsText.Size(), &m_rcPartText, uDTFlags);
        if (hCDC)
            DeleteDC(hCDC);
    }

    void CalculatePartsRect() noexcept
    {
        BITMAP bm;
        int cxImg, cyImg;
        if (GetObjectW(m_hbmImg, sizeof(bm), &bm))
        {
            cxImg = bm.bmWidth;
            cyImg = bm.bmHeight;
        }
        else
            cxImg = cyImg = 0;

        RECT rc{ 0,0,m_cxClient - cxImg,m_cyClient };
        UINT uDtFlags = DT_NOCLIP | DT_CALCRECT | DtEllipsis() | DtPrefix();
        int xPic, yPic;

        if (m_bAutoWrap)
        {
            uDtFlags |= DT_WORDBREAK;
            DrawTextW(m_DC.GetDC(), m_rsText.Data(), m_rsText.Size(), &rc, uDtFlags);

            int cyText = rc.bottom - rc.top;
            switch (m_eAlignV)
            {
            case Align::Near:// 上边
                rc.top = 0;
                rc.bottom = rc.top + cyText;
                yPic = rc.top;
                break;
            case Align::Center:// 中间
                rc.top = (m_cyClient - cyText) / 2;
                rc.bottom = rc.top + cyText;
                yPic = (m_cyClient - cyImg) / 2;
                break;
            case Align::Far:// 下边
                rc.bottom = m_cyClient;
                rc.top = rc.bottom - cyText;
                yPic = m_cyClient - cyImg;
                break;
            default: ECK_UNREACHABLE;
            }
        }
        else
        {
            uDtFlags |= DT_SINGLELINE;
            DrawTextW(m_DC.GetDC(), m_rsText.Data(), m_rsText.Size(), &rc, uDtFlags);

            int cyText = rc.bottom - rc.top;
            switch (m_eAlignV)
            {
            case Align::Near:// 上边
                rc.top = 0;
                rc.bottom = rc.top + cyText;
                yPic = rc.top;
                break;
            case Align::Center:// 中间
                rc.top = (m_cyClient - cyText) / 2;
                rc.bottom = rc.top + cyText;
                yPic = (m_cyClient - cyImg) / 2;
                break;
            case Align::Far:// 下边
                rc.bottom = m_cyClient;
                rc.top = rc.bottom - cyText;
                yPic = m_cyClient - cyImg;
                break;
            default: ECK_UNREACHABLE;
            }
        }
        uDtFlags &= (~DT_CALCRECT);

        const int cxText = rc.right - rc.left;
        switch (m_eAlignH)
        {
        case Align::Near:// 左边
            uDtFlags |= DT_LEFT;
            rc.left = cxImg;
            rc.right = rc.left + cxText;
            xPic = 0;
            break;
        case Align::Center:// 中间
            rc.left = (m_cxClient - (cxText + cxImg)) / 2 + cxImg;
            rc.right = rc.left + cxText;
            xPic = rc.left - cxImg;
            break;
        case Align::Far:// 右边
            uDtFlags |= DT_RIGHT;
            rc.right = m_cxClient - cxImg;
            rc.left = rc.right - cxText;
            xPic = rc.right;
            break;
        default: ECK_UNREACHABLE;
        }

        m_rcPartImg.left = xPic;
        m_rcPartImg.top = yPic;
        m_rcPartImg.right = m_rcPartImg.left + cxImg;
        m_rcPartImg.bottom = m_rcPartImg.top + cyImg;

        m_rcPartText = rc;
    }

    void SetDCAttributes(HDC hDC) noexcept
    {
        const auto* const ptc = PtcCurrent();
        SelectObject(hDC, m_hFont);
        SetTextColor(hDC, m_crText == CLR_DEFAULT ? ptc->crDefText : m_crText);
        SetDCBrushColor(hDC, m_crBk == CLR_DEFAULT ? ptc->crDefBkg : m_crBk);
        if (m_crTextBk == CLR_DEFAULT)
            SetBkMode(hDC, TRANSPARENT);
        else
        {
            SetBkMode(hDC, OPAQUE);
            SetBkColor(hDC, m_crTextBk);
        }
    }
public:
    BOOL LoGetIdealSize(_Inout_ LYTSIZE& Size) noexcept override
    {
        if (m_bPartMetricsDirty)
        {
            m_bPartMetricsDirty = FALSE;
            CalculatePartsRect();
        }
        Size.cx = TLytCoord((m_rcPartImg.right - m_rcPartImg.left) +
            (m_rcPartText.right - m_rcPartText.left));
        Size.cy = (TLytCoord)std::max(m_rcPartImg.bottom - m_rcPartImg.top,
            m_rcPartText.bottom - m_rcPartText.top);
        return TRUE;
    }

    LRESULT OnMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept override
    {
        switch (uMsg)
        {
        case WM_NCHITTEST:
        {
            if (m_bTransparent)
                switch (m_eMouseOption)
                {
                case MouseOpt::TransparentSpace:
                {
                    POINT pt{ GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam) };
                    ScreenToClient(hWnd, &pt);
                    LA_HITTEST laht;
                    HitTest(pt, laht);
                    if (!laht.bHitImg && !laht.bHitText)
                        return HTTRANSPARENT;
                }
                break;
                case MouseOpt::Transparent:
                    return HTTRANSPARENT;
                }
        }
        break;

        case WM_WINDOWPOSCHANGING:
        {
            if (m_bTransparent)
            {
                const auto pwp = (WINDOWPOS*)lParam;
                pwp->flags |= SWP_NOCOPYBITS;
            }
        }
        break;

        case WM_PRINTCLIENT:
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hWnd, wParam, ps);
            if (m_bTransparent)
            {
                SaveDC(ps.hdc);
                SetDCAttributes(ps.hdc);
                Paint(ps.hdc, ps.rcPaint);
                RestoreDC(ps.hdc, -1);
            }
            else
            {
                Paint(m_DC.GetDC(), ps.rcPaint);
                BitBltPs(ps, m_DC.GetDC());
            }
            EndPaint(hWnd, wParam, ps);
        }
        return 0;

        case WM_THEMECHANGED:
            SetDCAttributes(m_DC.GetDC());
            break;

        case WM_SETFONT:
        {
            m_hFont = (HFONT)wParam;
            SelectObject(m_DC.GetDC(), m_hFont);
            CalculatePartsRect();
            if (LOWORD(lParam))
                Redraw();
        }
        return 0;
        case WM_GETFONT:
            return (LRESULT)m_hFont;

        case WM_SETTEXT:
            m_rsText = (PWSTR)lParam;
            CalculatePartsRect();
            Redraw();
            return TRUE;
        case WM_GETTEXTLENGTH:
            return m_rsText.Size();
        case WM_GETTEXT:
            if ((int)wParam > 0)
                return m_rsText.CopyTo((PWSTR)lParam, (int)wParam - 1);
            else
                return 0;

        case WM_GETDLGCODE:
            return DLGC_STATIC;

        case WM_SIZE:
        {
            m_cxClient = LOWORD(lParam);
            m_cyClient = HIWORD(lParam);
            m_DC.ReSize(hWnd, m_cxClient, m_cyClient);
            SetDCAttributes(m_DC.GetDC());
            CalculatePartsRect();
        }
        return 0;

        case WM_CREATE:
        {
            m_rsText = ((CREATESTRUCTW*)lParam)->lpszName;
            m_DC.Create(hWnd);
            SetDCAttributes(m_DC.GetDC());
        }
        break;

        case WM_DESTROY:
        {
            m_DC.Destroy();
            m_hbmBkImg = nullptr;
            m_hbmImg = nullptr;
            m_hFont = nullptr;
            m_eBkImgMode = BkImgMode::TopLeft;
            m_eGradientMode = GradientMode::None;
            m_eAlignH = Align::Near;
            m_eAlignV = Align::Near;
            m_eEllipsisMode = Ellipsis::None;
            m_ePrefixMode = Prefix::Normal;
            m_eMouseOption = MouseOpt::None;
            m_bAutoWrap = m_bFillWndImg = m_bTransparent =
                m_bImgAlpha = m_bBkImgAlpha = FALSE;
            m_bPartMetricsDirty = TRUE;
            m_crText = m_crTextBk = m_crBk = CLR_DEFAULT;
            m_crGradient = DefaultGradient;
        }
        break;
        }

        return CWindow::OnMessage(hWnd, uMsg, wParam, lParam);
    }

    void Redraw() noexcept
    {
        if (!m_hWnd)
            return;
        if (m_bTransparent)
        {
            RECT rc{ 0,0,m_cxClient,m_cyClient };
            HWND hParent = GetParent(m_hWnd);
            MapWindowPoints(m_hWnd, hParent, (POINT*)&rc, 2);
            RedrawWindow(hParent, &rc, nullptr,
                RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW);
        }
        else
        {
            InvalidateRect(m_hWnd, nullptr, FALSE);
            UpdateWindow(m_hWnd);
        }
    }

    HBITMAP SetBackgroundImage(HBITMAP hBitmap) noexcept
    {
        BITMAP bm;
        if (GetObjectW(hBitmap, sizeof(bm), &bm))
        {
            m_cxBkImg = bm.bmWidth;
            m_cyBkImg = bm.bmHeight;
            std::swap(m_hbmBkImg, hBitmap);
            return hBitmap;
        }
        else
            return nullptr;
    }
    EckInlineNdCe HBITMAP GetBackgroundImage() const noexcept { return m_hbmBkImg; }

    EckInlineCe void SetBackgroundMode(BkImgMode iBkImgMode) noexcept { m_eBkImgMode = iBkImgMode; }
    EckInlineNdCe BkImgMode GetBackgroundMode() const noexcept { return m_eBkImgMode; }

    HBITMAP SetImage(HBITMAP hBitmap) noexcept
    {
        m_bPartMetricsDirty = TRUE;
        std::swap(m_hbmImg, hBitmap);
        return hBitmap;
    }
    EckInlineNdCe HBITMAP GetImage() const noexcept { return m_hbmImg; }

    EckInlineCe void SetAlignment(BOOL bHAlign, Align eAlign) noexcept
    {
        (bHAlign ? m_eAlignH : m_eAlignV) = eAlign;
        m_bPartMetricsDirty = TRUE;
    }
    EckInlineNdCe Align GetAlignment(BOOL bHAlign) const noexcept
    {
        return (bHAlign ? m_eAlignH : m_eAlignV);
    }

    EckInlineCe void SetAutoWrap(BOOL bAutoWrap) noexcept
    {
        m_bAutoWrap = bAutoWrap;
        m_bPartMetricsDirty = TRUE;
    }
    EckInlineNdCe BOOL GetAutoWrap() const noexcept { return m_bAutoWrap; }

    EckInlineCe void SetEllipsisMode(Ellipsis eEllipsisMode) noexcept
    {
        m_eEllipsisMode = eEllipsisMode;
        m_bPartMetricsDirty = TRUE;
    }
    EckInlineNdCe Ellipsis GetEllipsisMode() const noexcept { return m_eEllipsisMode; }

    EckInlineCe void SetPrefixMode(Prefix ePrefixMode) noexcept
    {
        m_ePrefixMode = ePrefixMode;
        m_bPartMetricsDirty = TRUE;
    }
    EckInlineNdCe Prefix GetPrefixMode() const noexcept { return m_ePrefixMode; }

    void SetColor(ClrPart ePart, COLORREF cr) noexcept
    {
        switch (ePart)
        {
        case ClrPart::Text:
            m_crText = cr;
            SetTextColor(m_DC.GetDC(), cr);
            break;
        case ClrPart::Bk:
            m_crBk = cr;
            if (cr == CLR_DEFAULT)
                cr = GetSysColor(COLOR_BTNFACE);
            SetDCBrushColor(m_DC.GetDC(), cr);
            break;
        case ClrPart::TextBk:
            m_crTextBk = cr;
            if (cr == CLR_DEFAULT)
                SetBkMode(m_DC.GetDC(), TRANSPARENT);
            else
            {
                SetBkMode(m_DC.GetDC(), OPAQUE);
                SetBkColor(m_DC.GetDC(), cr);
            }
            break;
        default: ECK_UNREACHABLE;
        }

        Redraw();
    }

    constexpr COLORREF GetColor(ClrPart ePart) const noexcept
    {
        switch (ePart)
        {
        case ClrPart::Text: return m_crText;
        case ClrPart::Bk: return m_crBk;
        case ClrPart::TextBk: return m_crTextBk;
        }
        ECK_UNREACHABLE;
    }

    EckInlineCe void SetGradientMode(GradientMode eMode) noexcept { m_eGradientMode = eMode; }
    EckInlineNdCe GradientMode GetGradientMode() const noexcept { return m_eGradientMode; }

    EckInlineCe void SetGradientColor(_In_range_(0, 2) int idx, COLORREF cr) noexcept
    {
        m_crGradient[idx] = cr;
    }
    EckInlineNdCe COLORREF GetGradientColor(_In_range_(0, 2) int idx) const noexcept
    {
        return m_crGradient[idx];
    }

    EckInline void SetTransparent(BOOL bTransparent) noexcept
    {
        m_bTransparent = bTransparent;
        ModifyStyle(bTransparent ? WS_EX_TRANSPARENT : 0, WS_EX_TRANSPARENT, GWL_EXSTYLE);
    }
    EckInlineNdCe BOOL GetTransparent() const noexcept { return m_bTransparent; }

    EckInlineCe void SetBackgroundFillWindow(BOOL bFullWndPic) noexcept { m_bFillWndImg = bFullWndPic; }
    EckInlineNdCe BOOL GetBackgroundFillWindow() const noexcept { return m_bFillWndImg; }

    EckInlineCe void SetMouseOption(MouseOpt eOption) noexcept { m_eMouseOption = eOption; }
    EckInlineNdCe MouseOpt GetMouseOption() const noexcept { return m_eMouseOption; }

    BOOL HitTest(POINT pt, _Out_ LA_HITTEST& laht) noexcept
    {
        laht.bHit = FALSE;
        laht.bHitImg = FALSE;
        laht.bHitText = FALSE;
        if (PtInRect(RECT{ 0,0,m_cxClient,m_cyClient }, pt))
        {
            laht.bHit = TRUE;
            if (m_bPartMetricsDirty)
                CalculatePartsRect();
            if (PtInRect(m_rcPartImg, pt))
                laht.bHitImg = TRUE;
            else if (PtInRect(m_rcPartText, pt))
                laht.bHitText = TRUE;
            return TRUE;
        }
        return FALSE;
    }
};
ECK_NAMESPACE_END