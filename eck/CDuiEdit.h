#pragma once
#include "CDuiScrollBar.h"
#include "TextSrvDef.h"

#include <RichOle.h>

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
class CEdit;
class CEditTextHost : public ITextHost2
{
private:
    ULONG m_cRef{ 1 };
    BOOL m_bActive{};
    CEdit* m_pEdit{};
public:
    ULONG STDMETHODCALLTYPE AddRef() override { return _InterlockedIncrement(&m_cRef); }
    ULONG STDMETHODCALLTYPE Release() override
    {
        if (_InterlockedDecrement(&m_cRef) == 0)
        {
            delete this;
            return 0;
        }
        return m_cRef;
    }
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject) override
    {
        const static QITAB qit[]
        {
            { &IID_ITextHost, OFFSETOFCLASS(CEditTextHost, ITextHost) },
            { &IID_ITextHost2, OFFSETOFCLASS(CEditTextHost, ITextHost2) },
            {}
        };
        return QISearch(this, qit, riid, ppvObject);
    }

    EckInlineCe void SetEdit(CEdit* pEdit) noexcept { m_pEdit = pEdit; }

    HDC TxGetDC();
    INT TxReleaseDC(HDC hdc);
    BOOL TxShowScrollBar(INT fnBar, BOOL fShow);
    BOOL TxEnableScrollBar(INT fuSBFlags, INT fuArrowflags);
    BOOL TxSetScrollRange(INT fnBar, LONG nMinPos, INT nMaxPos, BOOL fRedraw);
    BOOL TxSetScrollPos(INT fnBar, INT nPos, BOOL fRedraw);
    void TxInvalidateRect(LPCRECT prc, BOOL fMode);
    void TxViewChange(BOOL fUpdate);
    BOOL TxCreateCaret(HBITMAP hbmp, INT xWidth, INT yHeight);
    BOOL TxShowCaret(BOOL fShow);
    BOOL TxSetCaretPos(INT x, INT y);
    BOOL TxSetTimer(UINT idTimer, UINT uTimeout);
    void TxKillTimer(UINT idTimer);
    void TxScrollWindowEx(INT, INT, LPCRECT, LPCRECT, HRGN, LPRECT, UINT) {}
    void TxSetCapture(BOOL fCapture);
    void TxSetFocus();
    void TxSetCursor(HCURSOR hcur, BOOL fText);
    BOOL TxScreenToClient(LPPOINT lppt);
    BOOL TxClientToScreen(LPPOINT lppt);
    HRESULT TxActivate(LONG* plOldState);
    HRESULT TxDeactivate(LONG lNewState);
    HRESULT TxGetClientRect(LPRECT prc);
    HRESULT TxGetViewInset(LPRECT prc);
    HRESULT TxGetCharFormat(const CHARFORMATW** ppCF);
    HRESULT TxGetParaFormat(const PARAFORMAT** ppPF);
    COLORREF TxGetSysColor(int nIndex);
    HRESULT TxGetBackStyle(TXTBACKSTYLE* pstyle);
    HRESULT TxGetMaxLength(DWORD* plength);
    HRESULT TxGetScrollBars(DWORD* pdwScrollBar);
    HRESULT TxGetPasswordChar(_Out_ TCHAR* pch);
    HRESULT TxGetAcceleratorPos(LONG* pcp);
    HRESULT TxGetExtent(LPSIZEL lpExtent);
    HRESULT OnTxCharFormatChange(const CHARFORMATW* pCF);
    HRESULT OnTxParaFormatChange(const PARAFORMAT* pPF);
    HRESULT TxGetPropertyBits(DWORD dwMask, DWORD* pdwBits);
    HRESULT TxNotify(DWORD iNotify, void* pv);
    HIMC TxImmGetContext();
    void TxImmReleaseContext(HIMC himc);
    HRESULT TxGetSelectionBarWidth(LONG* lSelBarWidth) { *lSelBarWidth = 0; return S_OK; }

    BOOL TxIsDoubleClickPending();
    HRESULT TxGetWindow(HWND* phwnd);
    HRESULT TxSetForegroundWindow();
    HPALETTE TxGetPalette() { return nullptr; }
    HRESULT TxGetEastAsianFlags(LONG* pFlags);
    HCURSOR TxSetCursor2(HCURSOR hcur, BOOL bText);
    void TxFreeTextServicesNotification() {}
    HRESULT TxGetEditStyle(DWORD dwItem, DWORD* pdwData) { return E_NOTIMPL; }
    HRESULT TxGetWindowStyles(DWORD* pdwStyle, DWORD* pdwExStyle);
    HRESULT TxShowDropCaret(BOOL fShow, HDC hdc, LPCRECT prc);
    HRESULT TxDestroyCaret();
    HRESULT TxGetHorzExtent(LONG* plHorzExtent);
};

struct NMEDTXNOTIFY : ELENMHDR
{
    int iNotify;
    void* pData;
};

class CEdit : public CElement
{
    friend class CEditTextHost;
public:
    constexpr static DWORD DefaultTxProperty =
        TXTBIT_WORDWRAP | TXTBIT_MULTILINE | TXTBIT_DISABLEDRAG;
    constexpr static float
        CxyMargin = 6,
        CyBottomMargin = 8,
        CxyMarginHIMETRIC = CxyMargin * 2540 / 96,
        CyBottomMarginHIMETRIC = CyBottomMargin * 2540 / 96,
        CyBottomBar = CyBottomMargin - CxyMargin;

    enum : UINT
    {
        SsNormal,
        SsHot,
        SsFocus,
        SsDisabled,
        SsMax
    };
public:
    static RcPtr<CThemeBase> TmMakeDefaultTheme(BOOL bDark) noexcept;
    static RcPtr<CThemeBase> TmDefaultTheme(BOOL bDark) noexcept
    {
        static auto p1{ TmMakeDefaultTheme(TRUE) };
        static auto p2{ TmMakeDefaultTheme(FALSE) };
        return bDark ? p1 : p2;
    }
private:
    std::unique_ptr<CScrollBar> m_pSBHorz{};
    std::unique_ptr<CScrollBar> m_pSBVert{};

    IScrollController* m_pSccH{};
    IScrollController* m_pSccV{};

    ComPtr<CEditTextHost> m_pHost{};
    ComPtr<ITextServices2> m_pSrv{};

    CHARFORMAT2W m_DefCharFormat{ sizeof(CHARFORMAT2W) };
    PARAFORMAT2 m_DefParaFormat{ sizeof(PARAFORMAT2) };
    D2D1_RECT_F m_mgTextAera{ CxyMargin,CxyMargin,CxyMargin,CyBottomMargin };
    RECT m_rcViewInset{};

    D2D1_RECT_F m_rcCaret{};
    DWORD m_dwTxProp{ DefaultTxProperty };
    WCHAR m_chPassword{ L'*' };
    Align m_eSingleLineAlignV{ Align::Center };

    BITBOOL m_bCaretShow : 1{};

    // 收到WM_SETFONT时是否同步文本格式
    BITBOOL m_bAutoSyncTextFmt : 1{ TRUE };
    // 指示使用RichEdit内部自带滚动机制
    BITBOOL m_bREScrollAn : 1{};
    // 单行模式下按m_eSingleLineAlignV字段垂直对齐
    BITBOOL m_bSlAutoAlignV : 1{ TRUE };
    // 使用内置滚动条，FALSE指示使用了外部的滚动控制器
    BITBOOL m_bUseBuiltInScrollBar : 1{ TRUE };

    SimpleStyle m_Style[SsMax]
    {
        // Normal
        { IdTmInvalid, IdCrBack,         IdTmInvalid },
        // Hot
        { IdTmInvalid, IdCrBack,         IdTmInvalid },
        // Focus
        { IdTmInvalid, IdCrBack,         IdCrBorderPressed, 0.f, 1.f },
        // Disabled
        { IdTmInvalid, IdCrBackDisabled, IdTmInvalid },
    };

    void OnDpiChanged()
    {
        // RE启动时使用系统DPI，随后整个生命周期内只有下列两种情况下
        // msftedit!CTxtEdit::UpdateDPI被调用并更新DPI
        // 1. 接收WM_DPICHANGED_BEFOREPARENT，RE调用ITextHost2::TxGetWindow，
        // 并使用返回窗口的DPI
        // 2. 接收WM_USER + 0x148（即0x548），wParam和lParam分别为
        // X和Y方向的DPI。此消息未公开
        const auto iDpi = GetWindow().GetUserDpi();
        m_pSrv->TxSendMessage(0x548, iDpi, iDpi, nullptr);
        const auto pTf = GetTextFormat();
        if (pTf)
        {
            m_DefCharFormat.yHeight = (LONG)LogicalToPixel(
                pTf->GetFontSize() * 1440.f / 96.f);
            constexpr auto uBits = TXTBIT_CHARFORMATCHANGE;
            TxPropertyChanged(uBits, uBits);
        }
    }

    void UpdateInsetRect()
    {
        if (m_bSlAutoAlignV && !(m_dwTxProp & TXTBIT_MULTILINE))
        {
            if (m_eSingleLineAlignV == Align::Near)
                goto Normal;
            const auto cy = m_DefCharFormat.yHeight * 96.f / 1440.f +
                LogicalToPixel(2.f);
            float cyTop, cyBtm;
            const auto cyExtra = LogicalToPixel(
                GetHeight() - m_mgTextAera.top - m_mgTextAera.bottom);
            cyTop = LogicalToPixel(m_mgTextAera.top);
            cyBtm = LogicalToPixel(m_mgTextAera.bottom);
            if (m_eSingleLineAlignV == Align::Center)
            {
                const auto t = (cyExtra - cy) / 2;
                cyTop += t;
                cyBtm += t;
            }
            else
                cyTop += (cyExtra - cy);
            m_rcViewInset =
            {
                (int)LogicalToPixel(m_mgTextAera.left * 2540.f / 96.f),
                int(cyTop * 2540.f / 96.f),
                (int)LogicalToPixel(m_mgTextAera.right * 2540.f / 96.f),
                int(cyBtm * 2540.f / 96.f)
            };
        }
        else
        {
        Normal:
            m_rcViewInset =
            {
                (int)LogicalToPixel(m_mgTextAera.left * 2540.f / 96.f),
                (int)LogicalToPixel(m_mgTextAera.top * 2540.f / 96.f),
                (int)LogicalToPixel(m_mgTextAera.right * 2540.f / 96.f),
                (int)LogicalToPixel(m_mgTextAera.bottom * 2540.f / 96.f),
            };
        }
    }

    EckInlineNdCe static UINT TmSimpleStyleFromThemeState(UINT sa) noexcept
    {
        if (sa & SaDisable)
            return SsDisabled;
        if (sa & SaFocus)
            return SsFocus;
        if (sa & SaHot)
            return SsHot;
        return SsNormal;
    }

    void ScbCreateElement(BOOL bVert, BOOL bHorz) noexcept
    {
        if (!m_bUseBuiltInScrollBar)
            return;
        SccDisconnectEvent();
        if (bVert)
        {
            if (!m_pSBVert)
                m_pSBVert = std::make_unique<CScrollBar>();
            m_pSccV = m_pSBVert.get();
            if (!m_pSBVert->IsValid())
            {
                m_pSBVert->Create({}, DES_NO_FOCUSABLE | DES_VISIBLE, 0, 0, 0, 0, 0, this);
                m_pSBVert->SetVertical(TRUE);
            }
        }
        if (bHorz)
        {
            if (!m_pSBHorz)
                m_pSBHorz = std::make_unique<CScrollBar>();
            m_pSccH = m_pSBHorz.get();
            if (!m_pSBHorz->IsValid())
                m_pSBHorz->Create({}, DES_NO_FOCUSABLE | DES_VISIBLE, 0, 0, 0, 0, 0, this);
        }
        SccConnectEvent();
    }

    void ScbOnSize() noexcept
    {
        if ((!m_pSBHorz && !m_pSBVert) || !m_bUseBuiltInScrollBar)
            return;
        const auto cx = GetWidth();
        const auto cy = GetHeight();
        const auto cxySB = GetTheme()->GetMetric(IdMeScrollBar);
        if (m_pSBVert && m_pSBVert->IsValid())
            m_pSBVert->SetRect({ cx - cxySB, 0.f, cx, cy });
        if (m_pSBHorz && m_pSBHorz->IsValid())
            m_pSBHorz->SetRect({
                0.f,
                cy - cxySB - CyBottomBar,
                cx,
                cy - CyBottomBar });
    }

    void SccUpdatePage() noexcept
    {
        if (m_pSccV)
            m_pSccV->SccSetPage(GetHeight() - m_mgTextAera.top - m_mgTextAera.bottom);
        if (m_pSccH)
            m_pSccH->SccSetPage(GetWidth() - m_mgTextAera.left - m_mgTextAera.right);
    }

    EckInline void SccpScrollCallback(BOOL bHorz, float fPos) noexcept
    {
        GetWindow().RdLockUpdate();
        Scroll(bHorz, fPos);
        GetWindow().RdUnlockUpdate();
    }

    void SccConnectEvent() noexcept
    {
        if (m_pSccV)
            m_pSccV->SccSetCallback(
                [](const IScrollController::SCC_CALLBACK_DATA& Data)
                {
                    if ((int)Data.fPos == (int)Data.fPrevPos)
                        return;
                    ((CEdit*)Data.pUser)->SccpScrollCallback(FALSE, Data.fPos);
                }, this);
        if (m_pSccH)
            m_pSccH->SccSetCallback(
                [](const IScrollController::SCC_CALLBACK_DATA& Data)
                {
                    if ((int)Data.fPos == (int)Data.fPrevPos)
                        return;
                    ((CEdit*)Data.pUser)->SccpScrollCallback(TRUE, Data.fPos);
                }, this);
    }
    void SccDisconnectEvent() noexcept
    {
        if (m_pSccV)
            m_pSccV->SccSetCallback(nullptr, nullptr);
        if (m_pSccH)
            m_pSccH->SccSetCallback(nullptr, nullptr);
    }
public:
    LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept override
    {
        switch (uMsg)
        {
        case WM_SETCURSOR:
        {
            if (!(GetStyle() & DES_DISABLE))
            {
                SetCursor(LoadCursorW(nullptr, IDC_IBEAM));
                return TRUE;
            }
        }
        break;
        case WM_PAINT:
        {
            // NOTE 260428
            // TxInvalidateRect假定实现是“类InvalidateRect”的，即仅记录
            // 无效矩形，而不立即触发重绘（重绘间接导致对TxDrawD2D的调用）。
            // 必须尊重此种假设，否则将导致无穷递归调用
            GetWindow().RdLockUpdate();

            PAINTINFO ps;
            BeginPaint(ps, wParam, lParam);

            auto rcF = GetRectInClientD2D();
            const auto& Ss = m_Style[TmSimpleStyleFromThemeState(TmGetState())];
            GetTheme()->Draw(this, &Ss, IdPtNormal, rcF, nullptr);

            D2D1_RECT_F rcClipF{ ps.rcfClip };
            BOOL bNewClip{};
            if (rcClipF.top - rcF.top < m_mgTextAera.top)
            {
                rcClipF.top = rcF.top + m_mgTextAera.top;
                bNewClip = TRUE;
            }
            if (rcF.bottom - rcClipF.bottom < m_mgTextAera.bottom)
            {
                rcClipF.bottom = rcF.bottom - m_mgTextAera.bottom;
                bNewClip = TRUE;
            }
            if (!IsRectEmpty(rcClipF))
            {
                if (bNewClip)
                    GetDC()->PushAxisAlignedClip(rcClipF, D2D1_ANTIALIAS_MODE_ALIASED);
                LogicalToPixel(rcClipF);
                LogicalToPixel(rcF);
                RECT rcClip, rc{ MakeRect(rcF) };
                CeilRect(rcClipF, rcClip);
                m_pSrv->TxDrawD2D(GetDC(), (RECTL*)&rc, &rcClip, TXTVIEW_ACTIVE);
                if (bNewClip)
                    GetDC()->PopAxisAlignedClip();
            }

            if (m_bCaretShow)
            {
                const auto pBrush = GetWindow().CcSetBrushColor(
                    ArgbToD2DColorF(GetTheme()->GetColor(IdCrFore)));
                auto rcCaret{ m_rcCaret };
                ElementToClient(rcCaret);
                GetDC()->FillRectangle(rcCaret, pBrush);
            }

            DbgDrawFrame();
            EndPaint(ps);

            GetWindow().RdUnlockUpdate(FALSE);
        }
        return 0;

        case WM_MOUSEMOVE:
            if (!(TmState() & SaHot))
            {
                TmState() |= SaHot;
                Invalidate();
            }
            break;

        case WM_MOUSELEAVE:
            if (TmState() & SaHot)
            {
                TmState() &= ~SaHot;
                Invalidate();
            }
            return 0;

        case WM_SIZE:
        {
            ScbOnSize();
            SccUpdatePage();
            UpdateInsetRect();
            constexpr auto uBits = TXTBIT_CLIENTRECTCHANGE | TXTBIT_EXTENTCHANGE;
            TxPropertyChanged(uBits, uBits);
        }
        return 0;

        case WM_SETFOCUS:
            if (TmState() & SaFocus)
                break;
            TmState() |= SaFocus;

            m_pSrv->OnTxUIActivate();
            m_pSrv->TxSendMessage(WM_SETFOCUS, 0, 0, nullptr);
            Invalidate();
            return 0;

        case WM_KILLFOCUS:
            if (!(TmState() & SaFocus))
                break;
            TmState() &= ~SaFocus;

            m_pSrv->OnTxUIDeactivate();
            m_pSrv->TxSendMessage(WM_KILLFOCUS, 0, 0, nullptr);
            m_bCaretShow = FALSE;
            Invalidate();
            return 0;

        case WM_SETTEXT:
            m_pSrv->TxSetText(GetText().Data());
            return 0;

        case WM_DPICHANGED:
            UpdateInsetRect();
            OnDpiChanged();
            break;

        case WM_SETFONT:
        {
            const auto pTf = GetTextFormat();
            if (!pTf)
                break;
            m_DefCharFormat.dwMask = CFM_FACE | CFM_SIZE |
                CFM_WEIGHT | CFM_ITALIC | CFM_CHARSET;
            pTf->GetFontFamilyName(m_DefCharFormat.szFaceName, LF_FACESIZE);
            m_DefCharFormat.yHeight = (LONG)roundf(LogicalToPixel(
                pTf->GetFontSize() * 1440.f / 96.f));
            m_DefCharFormat.wWeight = (WORD)pTf->GetFontWeight();
            if (pTf->GetFontStyle() != DWRITE_FONT_STYLE_NORMAL)
                m_DefCharFormat.dwEffects |= CFE_ITALIC;
            else
                m_DefCharFormat.dwEffects &= ~CFE_ITALIC;
            m_DefCharFormat.bCharSet = DEFAULT_CHARSET;

            m_DefParaFormat.dwMask = PFM_ALIGNMENT;
            switch (pTf->GetTextAlignment())
            {
            case DWRITE_TEXT_ALIGNMENT_LEADING:
                m_DefParaFormat.wAlignment = PFA_LEFT;
                break;
            case DWRITE_TEXT_ALIGNMENT_TRAILING:
                m_DefParaFormat.wAlignment = PFA_RIGHT;
                break;
            case DWRITE_TEXT_ALIGNMENT_CENTER:
                m_DefParaFormat.wAlignment = PFA_CENTER;
                break;
            case DWRITE_TEXT_ALIGNMENT_JUSTIFIED:
                m_DefParaFormat.wAlignment = PFA_JUSTIFY;
                break;
            }
            UpdateInsetRect();
            constexpr auto uBits =
                TXTBIT_CHARFORMATCHANGE | TXTBIT_PARAFORMATCHANGE;
            TxPropertyChanged(uBits, uBits);
        }
        break;

        case WM_CREATE:
        {
            GetWindow().RdLockUpdate();
            ScbCreateElement(TRUE, TRUE);
            SetTheme(TmDefaultTheme(TmIsDarkMode()).Get());

            if (!TsiIsAvailable())
                TsiInit();
            m_pHost.Attach(new CEditTextHost{});
            m_pHost->SetEdit(this);
            ComPtr<IUnknown> pUnk;

            TsiCreateTextServices(nullptr, m_pHost.Get(), &pUnk);
            pUnk->QueryInterface(IID_ITextServices2, (void**)m_pSrv.AtClear());

            constexpr auto uBits = TO_DEFAULTCOLOREMOJI | TO_DISPLAYFONTCOLOR;
            SetTypographyOptions(uBits, uBits);
            m_pSrv->TxSetText(GetText().Data());
            m_pSrv->OnTxInPlaceActivate(nullptr);
            m_pSrv->OnTxUIActivate();
            if (GetWindow().GetUserDpi() != GetDpi(nullptr))
                OnDpiChanged();
            GetWindow().RdUnlockUpdate();
        }
        break;

        case WM_DESTROY:
        {
            m_pSrv->OnTxInPlaceDeactivate();
            TsiShutdownTextServices(m_pSrv.Get());
            m_pSrv.Clear();
            m_pHost->SetEdit(nullptr);
            m_pHost.Clear();
        }
        break;
        }

        if (uMsg == WM_MOUSEWHEEL)
        {
            if (m_bREScrollAn || (wParam & MK_CONTROL))
                goto TxSend;// RE处理平滑滚动 或 缩放
            if (wParam & MK_SHIFT)
                goto ScrollH;// 横向滚动
            m_pSccV->SccMouseWheel(float(-GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA));
            KctWake();
        }
        else if (uMsg == WM_MOUSEHWHEEL)
        {
            if (m_bREScrollAn)
                goto TxSend;
        ScrollH:
            m_pSccH->SccMouseWheel(float(-GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA));
            KctWake();
        }
        else if (uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST)
        {
        TxSend:
            if (uMsg == WM_LBUTTONDOWN || uMsg == WM_RBUTTONDOWN)
                SetFocus();
            auto pt = *(Kw::Vec2*)lParam;
            ElementToClient(pt);
            LogicalToPixel(pt);
            LRESULT lResult;
            const POINT pt1{ (int)pt.x, (int)pt.y };
            if (m_pSrv->TxSendMessage(uMsg,
                wParam, POINTTOPOINTS(pt1), &lResult) == S_OK)
                return lResult;
        }
        else if ((uMsg >= WM_KEYFIRST && uMsg <= WM_IME_KEYLAST) ||
            uMsg == WM_TIMER)
        {
            if (uMsg == WM_KEYDOWN)
            {
                if ((wParam == '0' || wParam == VK_NUMPAD0) &&
                    (GetKeyState(VK_CONTROL) & 0x8000))
                    SetZoom(0, 0);// Ctrl + 0 复位缩放
            }
            LRESULT lResult;
            if (m_pSrv->TxSendMessage(uMsg, wParam, lParam, &lResult) == S_OK)
                return lResult;
        }
        return __super::OnEvent(uMsg, wParam, lParam);
    }

    void SccSetController(IScrollController* pSccV, IScrollController* pSccH)
    {
        SccDisconnectEvent();
        m_bUseBuiltInScrollBar = (!pSccH && !pSccV);
        m_pSccV = pSccV;
        m_pSccH = pSccH;
        SccConnectEvent();
    }

    void ShowScrollBar(int nBar, BOOL bShow)
    {
        if (nBar == SB_BOTH)
        {
            m_pSccV->SccSetVisible(bShow);
            m_pSccH->SccSetVisible(bShow);
        }
        else if (nBar == SB_HORZ)
            m_pSccH->SccSetVisible(bShow);
        else if (nBar == SB_VERT)
            m_pSccV->SccSetVisible(bShow);
    }

    void Scroll(BOOL bHorz, float fPos, BOOL bRedraw = TRUE)
    {
        POINT pt;
        GetScrollPosition(&pt);
        (bHorz ? pt.x : pt.y) = (int)LogicalToPixel(fPos);
        SetScrollPosition(&pt);
        if (bRedraw)
            Invalidate();
    }

    HRESULT LoadRtf(IStream* pStream)
    {
        pStream->AddRef();
        EDITSTREAM es;
        es.dwCookie = (DWORD_PTR)pStream;
        es.dwError = 0;
        es.pfnCallback = [](DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG* pcb) -> DWORD
            {
                const auto pStream = (IStream*)dwCookie;
                return (DWORD)pStream->Read(pbBuff, (ULONG)cb, (ULONG*)pcb);
            };
        StreamIn(SF_RTF, &es);
        pStream->Release();
        return es.dwError;
    }

    HRESULT LoadRtf(PCWSTR pszFile)
    {
        HRESULT hr;
        ComPtr<IStream> pStream;
        hr = SHCreateStreamOnFileEx(pszFile, STGM_READ | STGM_SHARE_DENY_WRITE,
            0, FALSE, nullptr, &pStream);
        if (FAILED(hr))
            return hr;
        return LoadRtf(pStream.Get());
    }

    HRESULT SaveRtf(IStream* pStream) const
    {
        EDITSTREAM es;
        es.dwCookie = (DWORD_PTR)pStream;
        es.dwError = 0;
        es.pfnCallback = [](DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG* pcb) -> DWORD
            {
                const auto pStream = (IStream*)dwCookie;
                return (DWORD)pStream->Write(pbBuff, (ULONG)cb, (ULONG*)pcb);
            };
        StreamOut(SF_RTF, &es);
        return es.dwError;
    }

    // WARNING 此函数覆盖目标文件
    HRESULT SaveRtf(PCWSTR pszFile) const
    {
        HRESULT hr;
        ComPtr<IStream> pStream;
        hr = SHCreateStreamOnFileEx(pszFile,
            STGM_CREATE | STGM_WRITE | STGM_SHARE_EXCLUSIVE,
            0, 0, nullptr, &pStream);
        if (FAILED(hr))
            return hr;
        return SaveRtf(pStream.Get());
    }

    void TxPropertyChanged(DWORD dwMask, DWORD dwBits)
    {
        m_pSrv->OnTxPropertyBitsChange(dwMask, dwBits);
    }

    EckInline LRESULT TxSendMessage(UINT uMsg, WPARAM wParam, LPARAM lParam,
        _Out_opt_ HRESULT* phr = nullptr) const
    {
        LRESULT lResult;
        const auto hr = m_pSrv->TxSendMessage(uMsg, wParam, lParam, &lResult);
        if (phr) *phr = hr;
        return lResult;
    }

    EckInlineCe void TxSetProperty(DWORD dwProp) noexcept { m_dwTxProp = dwProp; }
    void TxSetProperty(DWORD dwMask, DWORD dwBits, BOOL bUpdate = TRUE) noexcept
    {
        m_dwTxProp = (m_dwTxProp & ~dwMask) | dwBits;
        if (bUpdate)
            TxPropertyChanged(dwMask, dwBits);
    }
    EckInlineNdCe DWORD TxGetProperty() const noexcept { return m_dwTxProp; }
    EckInlineNdCe DWORD TxGetProperty(DWORD dwMask) const noexcept
    {
        return m_dwTxProp & dwMask;
    }

    /// <summary>
    /// 启用/禁用自动URL检测
    /// </summary>
    /// <param name="iType">AURL_*</param>
    /// <param name="pszUrlPatterns">为NULL使用默认URL方案，否则为自定义URL方案，如（news:http:ftp:telnet:），最多50个</param>
    /// <returns>HRESULT</returns>
    EckInline HRESULT AutoUrlDetect(int iType, PCWSTR pszUrlPatterns = nullptr) const
    {
        return (HRESULT)TxSendMessage(EM_AUTOURLDETECT, iType, (LPARAM)pszUrlPatterns);
    }

    EckInline HRESULT CallAutoCorrectProcedure(WCHAR ch) const
    {
        return (HRESULT)TxSendMessage(EM_CALLAUTOCORRECTPROC, ch, 0);
    }

    EckInline HRESULT CanPaste(UINT uClipboardFormat) const
    {
        return (HRESULT)TxSendMessage(EM_CANPASTE, uClipboardFormat, 0);
    }

    EckInline HRESULT CanRedo() const
    {
        return (HRESULT)TxSendMessage(EM_CANREDO, 0, 0);
    }

    EckInline BOOL DisplayBand(RECT* prc) const
    {
        return (BOOL)TxSendMessage(EM_DISPLAYBAND, 0, (LPARAM)prc);
    }

    EckInline void GetSelection(CHARRANGE* pchrg) const
    {
        TxSendMessage(EM_EXGETSEL, 0, (LPARAM)pchrg);
    }

    EckInline void LimitText(int cch) const
    {
        TxSendMessage(EM_EXLIMITTEXT, 0, cch);
    }

    EckInline int LineFromChar(int ich) const
    {
        return (int)TxSendMessage(EM_EXLINEFROMCHAR, 0, ich);
    }

    EckInline LRESULT SetSelection(CHARRANGE* pchrg) const
    {
        return TxSendMessage(EM_EXSETSEL, 0, (LPARAM)pchrg);
    }

    EckInline int FindTextW(UINT uFlags, FINDTEXTW* pFindText) const
    {
        return (int)TxSendMessage(EM_FINDTEXTW, uFlags, (LPARAM)pFindText);
    }

    EckInline int FindTextEx(UINT uFlags, FINDTEXTEXW* pFindText) const
    {
        return (int)TxSendMessage(EM_FINDTEXTEXW, uFlags, (LPARAM)pFindText);
    }

    EckInline int FindWordBreak(UINT uFlags, int idxChar) const
    {
        return (int)TxSendMessage(EM_FINDWORDBREAK, uFlags, idxChar);
    }

    EckInline int FormatRange(BOOL bInPlace, FORMATRANGE* pfr) const
    {
        return (int)TxSendMessage(EM_FORMATRANGE, bInPlace, (LPARAM)pfr);
    }

    EckInline AutoCorrectProc GetAutoCorrectProcedure() const
    {
        return (AutoCorrectProc)TxSendMessage(EM_GETAUTOCORRECTPROC, 0, 0);
    }

    EckInline BOOL GetAutoUrlDetect() const
    {
        return (BOOL)TxSendMessage(EM_GETAUTOURLDETECT, 0, 0);
    }

    EckInline void GetBidiOptions(BIDIOPTIONS* pbidio) const
    {
        TxSendMessage(EM_GETBIDIOPTIONS, 0, (LPARAM)pbidio);
    }

    EckInline DWORD GetCharFormat(int iRange, CHARFORMAT2W* pcf) const
    {
        return (DWORD)TxSendMessage(EM_GETCHARFORMAT, iRange, (LPARAM)pcf);
    }

    EckInline int GetCtfModeBias() const
    {
        return (int)TxSendMessage(EM_GETCTFMODEBIAS, 0, 0);
    }

    EckInline BOOL GetCtfOpenStatus() const
    {
        return (BOOL)TxSendMessage(EM_GETCTFOPENSTATUS, 0, 0);
    }

    EckInline UINT GetEditStyle() const
    {
        return (UINT)TxSendMessage(EM_GETEDITSTYLE, 0, 0);
    }

    EckInline UINT GetEditStyleEx() const
    {
        return (UINT)TxSendMessage(EM_GETEDITSTYLEEX, 0, 0);
    }

    EckInline DWORD GetEllipsisMode() const
    {
        DWORD dw;
        TxSendMessage(EM_GETELLIPSISMODE, 0, (LPARAM)&dw);
        return dw;
    }

    EckInline BOOL GetEllipsisState() const
    {
        return (BOOL)TxSendMessage(EM_GETELLIPSISSTATE, 0, 0);
    }

    EckInline UINT GetEventMask() const
    {
        return (UINT)TxSendMessage(EM_GETEVENTMASK, 0, 0);
    }

    EckInline void GetHyphenateInfomation(HYPHRESULT* phr) const
    {
        TxSendMessage(EM_GETHYPHENATEINFO, (WPARAM)phr, 0);
    }

    EckInline int GetImeCompositionMode() const
    {
        return (int)TxSendMessage(EM_GETIMECOMPMODE, 0, 0);
    }

    EckInline int GetImeCompositionText(IMECOMPTEXT* pict, PWSTR pszBuf) const
    {
        return (int)TxSendMessage(EM_GETIMECOMPTEXT, (WPARAM)pict, (LPARAM)pszBuf);
    }

    EckInline int GetImeModeBias() const
    {
        return (int)TxSendMessage(EM_GETIMEMODEBIAS, 0, 0);
    }

    EckInline UINT GetImeProperty(int iType) const
    {
        return (UINT)TxSendMessage(EM_GETIMEPROPERTY, iType, 0);
    }

    EckInline UINT GetLangOptions() const
    {
        return (UINT)TxSendMessage(EM_GETLANGOPTIONS, 0, 0);
    }

    EckInline HRESULT GetOleInterface(IRichEditOle** ppRichEditOle) const
    {
        return (HRESULT)TxSendMessage(EM_GETOLEINTERFACE, 0, (LPARAM)ppRichEditOle);
    }

    EckInline UINT GetOptions() const
    {
        return (UINT)TxSendMessage(EM_GETOPTIONS, 0, 0);
    }

    EckInline UINT GetPageRotate() const
    {
        return (UINT)TxSendMessage(EM_GETPAGEROTATE, 0, 0);
    }

    EckInline UINT GetParagraphFormat(PARAFORMAT2* ppf) const
    {
        return (UINT)TxSendMessage(EM_GETPARAFORMAT, 0, (LPARAM)ppf);
    }

    EckInline UNDONAMEID GetRedoName() const
    {
        return (UNDONAMEID)TxSendMessage(EM_GETREDONAME, 0, 0);
    }

    EckInline void GetScrollPosition(POINT* ppt) const
    {
        TxSendMessage(EM_GETSCROLLPOS, 0, (LPARAM)ppt);
    }

    EckInline int GetSelectedText(PWSTR pszBuf) const
    {
        return (int)TxSendMessage(EM_GETSELTEXT, 0, (LPARAM)pszBuf);
    }

    EckInline int GetStoryType(int idxStory) const
    {
        return (int)TxSendMessage(EM_GETSTORYTYPE, idxStory, 0);
    }

    EckInline HRESULT GetTableParameters(TABLEROWPARMS* ptrp, TABLECELLPARMS* ptcp) const
    {
        return (HRESULT)TxSendMessage(EM_GETTABLEPARMS, (WPARAM)ptrp, (LPARAM)ptcp);
    }

    EckInline int GetTextEx(GETTEXTEX* pgt, PWSTR pszBuf) const
    {
        return (int)TxSendMessage(EM_GETTEXTEX, (WPARAM)pgt, (LPARAM)pszBuf);
    }

    EckInline int GetTextLengthEx(GETTEXTLENGTHEX* pgtl) const
    {
        return (int)TxSendMessage(EM_GETTEXTLENGTHEX, (WPARAM)pgtl, 0);
    }

    EckInline TEXTMODE GetTextMode() const
    {
        return (TEXTMODE)TxSendMessage(EM_GETTEXTMODE, 0, 0);
    }

    EckInline int GetTextRange(TEXTRANGEW* pTextRange) const
    {
        return (int)TxSendMessage(EM_GETTEXTRANGE, 0, (LPARAM)pTextRange);
    }

    EckInline BOOL GetTouchOptions() const
    {
        return (BOOL)TxSendMessage(EM_GETTOUCHOPTIONS, RTO_SHOWHANDLES, 0);
    }

    EckInline UINT GetTypographyOptions() const
    {
        return (UINT)TxSendMessage(EM_GETTYPOGRAPHYOPTIONS, 0, 0);
    }

    EckInline UNDONAMEID GetUndoName() const
    {
        return (UNDONAMEID)TxSendMessage(EM_GETUNDONAME, 0, 0);
    }

    EckInline EDITWORDBREAKPROCEX GetWordBreakProcedure() const
    {
        return (EDITWORDBREAKPROCEX)TxSendMessage(EM_GETWORDBREAKPROCEX, 0, 0);
    }

    EckInline BOOL GetZoom(int* pnZoomNumerator, int* pnZoomDenominator) const
    {
        return (BOOL)TxSendMessage(EM_GETZOOM, (WPARAM)pnZoomNumerator, (LPARAM)pnZoomDenominator);
    }

    EckInline void HideSelection(BOOL bHide) const
    {
        TxSendMessage(EM_HIDESELECTION, bHide, 0);
    }

    EckInline HRESULT InsertImage(RICHEDIT_IMAGE_PARAMETERS* pImageParams) const
    {
        return (HRESULT)TxSendMessage(EM_INSERTIMAGE, 0, (LPARAM)pImageParams);
    }

    EckInline HRESULT InsertTable(TABLEROWPARMS* ptrp, TABLECELLPARMS* ptcp) const
    {
        return (HRESULT)TxSendMessage(EM_INSERTTABLE, (WPARAM)ptrp, (LPARAM)ptcp);
    }

    EckInline BOOL IsIme() const
    {
        return (BOOL)TxSendMessage(EM_ISIME, 0, 0);
    }

    EckInline void PasteSpecial(UINT uClipFormat, REPASTESPECIAL* prps) const
    {
        TxSendMessage(EM_PASTESPECIAL, uClipFormat, (LPARAM)prps);
    }

    EckInline void Reconversion() const
    {
        TxSendMessage(EM_RECONVERSION, 0, 0);
    }

    EckInline BOOL Redo() const
    {
        return (BOOL)TxSendMessage(EM_REDO, 0, 0);
    }

    EckInline void RequestResize() const
    {
        TxSendMessage(EM_REQUESTRESIZE, 0, 0);
    }

    EckInline UINT SelectionType() const
    {
        return (UINT)TxSendMessage(EM_SELECTIONTYPE, 0, 0);
    }

    EckInline BOOL SetAutoCorrectProcedure(AutoCorrectProc pfnNewProc) const
    {
        return (BOOL)TxSendMessage(EM_SETAUTOCORRECTPROC, (WPARAM)pfnNewProc, 0);
    }

    EckInline void SetBidiOptions(BIDIOPTIONS* pbidio) const
    {
        TxSendMessage(EM_SETBIDIOPTIONS, 0, (LPARAM)pbidio);
    }

    EckInline COLORREF SetBackgroundColor(BOOL bSysColor, COLORREF cr) const
    {
        return (COLORREF)TxSendMessage(EM_SETBKGNDCOLOR, bSysColor, cr);
    }

    EckInline BOOL SetCharFormat(int iFmt, CHARFORMAT2W* pcf) const
    {
        return (BOOL)TxSendMessage(EM_SETCHARFORMAT, iFmt, (LPARAM)pcf);
    }

    EckInline int SetCtfModeBias(int iModeBias) const
    {
        return (int)TxSendMessage(EM_SETCTFMODEBIAS, iModeBias, 0);
    }

    EckInline BOOL SetCtfOpenStatus(BOOL bOpen) const
    {
        return (BOOL)TxSendMessage(EM_SETCTFOPENSTATUS, bOpen, 0);
    }

    EckInline BOOL SetDisableOleLinkConversion(BOOL bDisable) const
    {
        return (BOOL)TxSendMessage(EM_SETDISABLEOLELINKCONVERSION, 0, bDisable);
    }

    EckInline UINT SetEditStyle(UINT dwStyle, UINT uMask) const
    {
        return (UINT)TxSendMessage(EM_SETEDITSTYLE, dwStyle, uMask);
    }

    EckInline UINT SetEditStyleEx(UINT dwStyleEx, UINT dwMask) const
    {
        return (UINT)TxSendMessage(EM_SETEDITSTYLEEX, dwStyleEx, dwMask);
    }

    EckInline BOOL SetEllipsisMode(UINT uEllipsisMode) const
    {
        return (BOOL)TxSendMessage(EM_SETELLIPSISMODE, 0, uEllipsisMode);
    }

    EckInline UINT SetEventMask(UINT dwEventMask) const
    {
        return (UINT)TxSendMessage(EM_SETEVENTMASK, 0, dwEventMask);
    }

    EckInline BOOL SetFontSize(int iSize) const
    {
        return (BOOL)TxSendMessage(EM_SETFONTSIZE, iSize, 0);
    }

    EckInline void SetHyphenateInfomation(HYPHENATEINFO* phr) const
    {
        TxSendMessage(EM_SETHYPHENATEINFO, (LPARAM)phr, 0);
    }

    EckInline int SetImeModeBias(int iModeBias) const
    {
        return (int)TxSendMessage(EM_SETIMEMODEBIAS, iModeBias, iModeBias);
    }

    EckInline void SetLangOptions(UINT dwOptions) const
    {
        TxSendMessage(EM_SETLANGOPTIONS, 0, dwOptions);
    }

    EckInline BOOL SetOleCallback(IRichEditOleCallback* pCallback) const
    {
        return (BOOL)TxSendMessage(EM_SETOLECALLBACK, 0, (LPARAM)pCallback);
    }

    EckInline UINT SetOptions(int iType, UINT dwOptions) const
    {
        return (UINT)TxSendMessage(EM_SETOPTIONS, iType, dwOptions);
    }

    EckInline BOOL SetParagraphFormat(PARAFORMAT2* ppf) const
    {
        return (BOOL)TxSendMessage(EM_SETPARAFORMAT, 0, (LPARAM)ppf);
    }

    EckInline int SetPageRotate(int iRotate) const
    {
        return (int)TxSendMessage(EM_SETPAGEROTATE, iRotate, 0);
    }

    EckInline void SetPalette(HPALETTE hPal) const
    {
        TxSendMessage(EM_SETPALETTE, (WPARAM)hPal, 0);
    }

    EckInline HRESULT SetQueryConvertOleLinkCallback(WPARAM Context,
        OLESTREAMQUERYCONVERTOLELINKCALLBACK pfnCallback) const
    {
        return (HRESULT)TxSendMessage(EM_SETQUERYCONVERTOLELINKCALLBACK,
            Context, (LPARAM)pfnCallback);
    }

    EckInline void SetScrollPosition(POINT* ppt) const
    {
        TxSendMessage(EM_SETSCROLLPOS, 0, (LPARAM)ppt);
    }

    EckInline int SetStoryType(int idxStory, int iStoryType) const
    {
        return (int)TxSendMessage(EM_SETSTORYTYPE, idxStory, iStoryType);
    }

    EckInline HRESULT SetTableParameters(TABLEROWPARMS* ptrp, TABLECELLPARMS* ptcp) const
    {
        return (HRESULT)TxSendMessage(EM_SETTABLEPARMS, (WPARAM)ptrp, (LPARAM)ptcp);
    }

    EckInline HRESULT SetTargetDevice(HDC hDC, int cxLine) const
    {
        return (HRESULT)TxSendMessage(EM_SETTARGETDEVICE, (WPARAM)hDC, cxLine);
    }

    EckInline int SetTextEx(SETTEXTEX* pst, PCWSTR pszText) const
    {
        EckAssert(pst->codepage == 1200);
        return (int)TxSendMessage(EM_SETTEXTEX, (WPARAM)pst, (LPARAM)pszText);
    }

    EckInline int SetTextEx(SETTEXTEX* pst, PCSTR pszText) const
    {
        EckAssert(pst->codepage != 1200);
        return (int)TxSendMessage(EM_SETTEXTEX, (WPARAM)pst, (LPARAM)pszText);
    }

    EckInline BOOL SetTextMode(TEXTMODE tm) const
    {
        return (BOOL)TxSendMessage(EM_SETTEXTMODE, tm, 0);
    }

    EckInline void SetTouchOptions(BOOL bShowHandles) const
    {
        TxSendMessage(EM_SETTOUCHOPTIONS, RTO_SHOWHANDLES, bShowHandles);
    }

    EckInline BOOL SetTypographyOptions(UINT dwOptions, UINT dwMask) const
    {
        return (BOOL)TxSendMessage(EM_SETTYPOGRAPHYOPTIONS, dwOptions, dwMask);
    }

    EckInline BOOL SetUiAutomationName(PCWSTR pszText) const
    {
        return (BOOL)TxSendMessage(EM_SETUIANAME, 0, (LPARAM)pszText);
    }

    EckInline int SetUndoLimit(int cLimit) const
    {
        return (int)TxSendMessage(EM_SETUNDOLIMIT, cLimit, 0);
    }

    EckInline EDITWORDBREAKPROCEX SetWordBreakProcedure(EDITWORDBREAKPROCEX pfnNewProc) const
    {
        return (EDITWORDBREAKPROCEX)TxSendMessage(EM_SETWORDBREAKPROCEX, 0, (LPARAM)pfnNewProc);
    }

    EckInline BOOL SetZoom(int iZoomNumerator, int iZoomDenominator) const
    {
        return (BOOL)TxSendMessage(EM_SETZOOM, iZoomNumerator, iZoomDenominator);
    }

    EckInline void ShowScrollBar(int iBar, BOOL bShow) const
    {
        TxSendMessage(EM_SHOWSCROLLBAR, iBar, bShow);
    }

    EckInline void StopGroupTyping() const
    {
        TxSendMessage(EM_STOPGROUPTYPING, 0, 0);
    }

    EckInline int StreamIn(UINT uFlags, EDITSTREAM* es) const
    {
        return (int)TxSendMessage(EM_STREAMIN, uFlags, (LPARAM)es);
    }

    EckInline int StreamOut(UINT uFlags, EDITSTREAM* es) const
    {
        return (int)TxSendMessage(EM_STREAMOUT, uFlags, (LPARAM)es);
    }
};

class CTmEdit : public CThemeBase
{
public:
    TmResult Draw(
        CElement* pEle,
        const SimpleStyle* pStyle,
        UINT idPart,
        const D2D1_RECT_F& rc,
        _In_opt_ const D2D1_RECT_F* prcClip) noexcept override
    {
        if (idPart != IdPtNormal)
            return TmResult::NotSupport;
        return pEle->TmGenericDrawBackground(pStyle, rc);
    }
};
inline RcPtr<CThemeBase> CEdit::TmMakeDefaultTheme(BOOL bDark) noexcept
{
    return TmMakeTheme<CTmEdit>(bDark);
}

inline HDC CEditTextHost::TxGetDC() { return GetDC(m_pEdit->GetWindow().Handle); }
inline INT CEditTextHost::TxReleaseDC(HDC hdc) { return ReleaseDC(m_pEdit->GetWindow().Handle, hdc); }

inline BOOL CEditTextHost::TxShowScrollBar(INT fnBar, BOOL fShow)
{
    m_pEdit->ShowScrollBar(fnBar, fShow);
    return TRUE;
}

inline BOOL CEditTextHost::TxEnableScrollBar(INT fuSBFlags, INT fuArrowflags)
{
    // TODO
    return 0;
}

inline BOOL CEditTextHost::TxSetScrollRange(INT fnBar, LONG nMinPos, INT nMaxPos, BOOL fRedraw)
{
    const auto fMin = m_pEdit->PixelToLogical((float)nMinPos);
    const auto fMax = m_pEdit->PixelToLogical((float)nMaxPos);
    if (fnBar == SB_VERT || fnBar == SB_BOTH)
    {
        m_pEdit->m_pSccV->SccSetRange(fMin, fMax);
        if (fRedraw)
            m_pEdit->m_pSccV->SccRedraw();
    }
    if (fnBar == SB_HORZ || fnBar == SB_BOTH)
    {
        m_pEdit->m_pSccH->SccSetRange(fMin, fMax);
        if (fRedraw)
            m_pEdit->m_pSccH->SccRedraw();
    }
    return TRUE;
}

inline BOOL CEditTextHost::TxSetScrollPos(INT fnBar, INT nPos, BOOL fRedraw)
{
    const auto fPos = m_pEdit->PixelToLogical((float)nPos);
    if (fnBar == SB_VERT || fnBar == SB_BOTH)
    {
        m_pEdit->m_pSccV->SccSetPosition(fPos);
        if (fRedraw)
            m_pEdit->m_pSccV->SccRedraw();
    }
    if (fnBar == SB_HORZ || fnBar == SB_BOTH)
    {
        m_pEdit->m_pSccH->SccSetPosition(fPos);
        if (fRedraw)
            m_pEdit->m_pSccH->SccRedraw();
    }
    return TRUE;
}

inline void CEditTextHost::TxInvalidateRect(LPCRECT prc, BOOL fMode)
{
    if (prc)
    {
        D2D1_RECT_F rc{ MakeD2DRectF(*prc) };
        m_pEdit->PixelToLogical(rc);
        m_pEdit->ClientToElement(rc);
        m_pEdit->Invalidate(rc);
    }
    else
        m_pEdit->Invalidate();
}

inline void CEditTextHost::TxViewChange(BOOL fUpdate)
{
    if (fUpdate)
        m_pEdit->GetWindow().RdRenderAndPresent();
}

inline BOOL CEditTextHost::TxCreateCaret(HBITMAP hbmp, INT xWidth, INT yHeight)
{
    m_pEdit->m_rcCaret = { 0.f, 0.f, (float)xWidth, (float)yHeight };
    return TRUE;
}

inline BOOL CEditTextHost::TxShowCaret(BOOL fShow)
{
    m_pEdit->m_bCaretShow = fShow;
    auto rc{ m_pEdit->m_rcCaret };
    m_pEdit->Invalidate(rc);
    return TRUE;
}

inline BOOL CEditTextHost::TxSetCaretPos(INT x, INT y)
{
    auto& rc = m_pEdit->m_rcCaret;
    const auto cx = rc.right - rc.left;
    const auto cy = rc.bottom - rc.top;
    rc = { (float)x, (float)y, float(x + cx), float(y + cy) };
    m_pEdit->PixelToLogical(rc);
    rc.left = ceilf(rc.left);
    rc.right = ceilf(rc.right);
    if (rc.right - rc.left < 1.f)
        rc.right = rc.left + 1.f;
    m_pEdit->ClientToElement(rc);
    m_pEdit->Invalidate(rc);
    return TRUE;
}

inline BOOL CEditTextHost::TxSetTimer(UINT idTimer, UINT uTimeout)
{
    return m_pEdit->SetTimer(idTimer, uTimeout);
}

inline void CEditTextHost::TxKillTimer(UINT idTimer)
{
    m_pEdit->KillTimer(idTimer);
}

inline void CEditTextHost::TxSetCapture(BOOL fCapture)
{
    if (fCapture)
        m_pEdit->SetCapture();
    else
        m_pEdit->ReleaseCapture();
}

inline void CEditTextHost::TxSetFocus()
{
    m_pEdit->SetFocus();
}

inline void CEditTextHost::TxSetCursor(HCURSOR hcur, BOOL fText)
{
    SetCursor(hcur);
}

inline BOOL CEditTextHost::TxScreenToClient(LPPOINT lppt)
{
    return ScreenToClient(m_pEdit->GetWindow().Handle, lppt);
}

inline BOOL CEditTextHost::TxClientToScreen(LPPOINT lppt)
{
    return ClientToScreen(m_pEdit->GetWindow().Handle, lppt);
}

inline HRESULT CEditTextHost::TxActivate(LONG* plOldState)
{
    *plOldState = m_bActive;
    m_bActive = TRUE;
    return S_OK;
}

inline HRESULT CEditTextHost::TxDeactivate(LONG lNewState)
{
    m_bActive = lNewState;
    return S_OK;
}

inline HRESULT CEditTextHost::TxGetClientRect(LPRECT prc)
{
    auto rcF{ m_pEdit->GetRectInClient() };
    m_pEdit->LogicalToPixel(rcF);
    *prc = MakeRect(rcF);
    return S_OK;
}

inline HRESULT CEditTextHost::TxGetViewInset(LPRECT prc)
{
    *prc = m_pEdit->m_rcViewInset;
    return S_OK;
}

inline HRESULT CEditTextHost::TxGetCharFormat(const CHARFORMATW** ppCF)
{
    *ppCF = &m_pEdit->m_DefCharFormat;
    return S_OK;
}

inline HRESULT CEditTextHost::TxGetParaFormat(const PARAFORMAT** ppPF)
{
    *ppPF = &m_pEdit->m_DefParaFormat;
    return S_OK;
}

inline COLORREF CEditTextHost::TxGetSysColor(int nIndex)
{
    switch (nIndex)
    {
    case COLOR_WINDOWTEXT:
    {
        const auto v = m_pEdit->GetTheme()->GetColorOptional(IdCrFore);
        return v ? ArgbToColorref(v.value()) : PtcCurrent()->crDefText;
    }
    case COLOR_WINDOW:
    {
        const auto v = m_pEdit->GetTheme()->GetColorOptional(IdCrBack);
        return v ? ArgbToColorref(v.value()) : PtcCurrent()->crDefBkg;
    }
    }
    return GetSysColor(nIndex);
}

inline HRESULT CEditTextHost::TxGetBackStyle(TXTBACKSTYLE* pstyle)
{
    *pstyle = TXTBACK_TRANSPARENT;
    return S_OK;
}

inline HRESULT CEditTextHost::TxGetMaxLength(DWORD* plength)
{
    *plength = INFINITE;
    return S_OK;
}

inline HRESULT CEditTextHost::TxGetScrollBars(DWORD* pdwScrollBar)
{
    *pdwScrollBar = WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL;
    return S_OK;
}

inline HRESULT CEditTextHost::TxGetPasswordChar(_Out_ TCHAR* pch)
{
    *pch = m_pEdit->m_chPassword;
    return S_OK;
}

inline HRESULT CEditTextHost::TxGetAcceleratorPos(LONG* pcp)
{
    *pcp = -1;
    return E_NOTIMPL;
}

inline HRESULT CEditTextHost::TxGetExtent(LPSIZEL lpExtent)
{
    lpExtent->cx = (LONG)m_pEdit->LogicalToPixel(m_pEdit->GetWidth() * 2540.f / 96.f);
    lpExtent->cy = (LONG)m_pEdit->LogicalToPixel(m_pEdit->GetHeight() * 2540.f / 96.f);
    return S_OK;
}

inline HRESULT CEditTextHost::OnTxCharFormatChange(const CHARFORMATW* pCF)
{
    return E_NOTIMPL;
}

inline HRESULT CEditTextHost::OnTxParaFormatChange(const PARAFORMAT* pPF)
{
    return E_NOTIMPL;
}

inline HRESULT CEditTextHost::TxGetPropertyBits(DWORD dwMask, DWORD* pdwBits)
{
    *pdwBits = (m_pEdit->TxGetProperty() | TXTBIT_D2DDWRITE) & dwMask;
    return S_OK;
}

inline HRESULT CEditTextHost::TxNotify(DWORD iNotify, void* pv)
{
    NMEDTXNOTIFY nm{ EDE_TXNOTIFY };
    nm.iNotify = iNotify;
    nm.pData = pv;
    m_pEdit->SendNotify(&nm);
    return S_OK;
}

inline HIMC CEditTextHost::TxImmGetContext()
{
    return ImmGetContext(m_pEdit->GetWindow().Handle);
}

inline void CEditTextHost::TxImmReleaseContext(HIMC himc)
{
    ImmReleaseContext(m_pEdit->GetWindow().Handle, himc);
}

inline BOOL CEditTextHost::TxIsDoubleClickPending()
{
    MSG Msg;
    return PeekMessageA(&Msg, m_pEdit->GetWindow().Handle,
        WM_LBUTTONDBLCLK, WM_LBUTTONDBLCLK, PM_NOREMOVE | PM_NOYIELD);
}

inline HRESULT CEditTextHost::TxGetWindow(HWND* phwnd)
{
    *phwnd = m_pEdit->GetWindow().Handle;
    return S_OK;
}

inline HRESULT CEditTextHost::TxSetForegroundWindow()
{
    if (!SetForegroundWindow(m_pEdit->GetWindow().Handle))
        SetFocus(m_pEdit->GetWindow().Handle);
    return S_OK;
}

inline HRESULT CEditTextHost::TxGetEastAsianFlags(LONG* pFlags)
{
    *pFlags = 0;
    return S_OK;
}

inline HCURSOR CEditTextHost::TxSetCursor2(HCURSOR hcur, BOOL bText)
{
    return SetCursor(hcur);
}

inline HRESULT CEditTextHost::TxGetWindowStyles(DWORD* pdwStyle, DWORD* pdwExStyle)
{
    *pdwStyle = m_pEdit->GetWindow().GetStyle();
    *pdwExStyle = m_pEdit->GetWindow().GetExStyle();
    return S_OK;
}

inline HRESULT CEditTextHost::TxShowDropCaret(BOOL fShow, HDC hdc, LPCRECT prc) { return E_NOTIMPL; }
inline HRESULT CEditTextHost::TxDestroyCaret() { return E_NOTIMPL; }
inline HRESULT CEditTextHost::TxGetHorzExtent(LONG* plHorzExtent)
{
    return E_NOTIMPL;
}
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END