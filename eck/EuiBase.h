#pragma once
#include "EuiTheme.h"
#include "CMemoryDC.h"
#include "GpPtr.h"
#include "Color.h"

ECK_NAMESPACE_BEGIN
ECK_EUI_NAMESPACE_BEGIN
class CEuiWindow;
class CElement : public UiBasic::CElement<CElement, false>
{
private:
    CStringW m_rsText{};
    RcPtr<CThemeBase> m_pTheme{};
    RcPtr<CThemeStyle> m_pThemeStyle{};
public:
    HRESULT EhUiaMakeInterface() noexcept override;

    void Create(std::wstring_view svText, UINT uStyle, UINT uExStyle,
        int x, int y, int cx, int cy, CElement* pParent, CEuiWindow* pWnd,
        int iId = 0, PCVOID pData = nullptr) noexcept;

    void Destroy()
    {
        PreDestroy();
        CallEvent(WM_DESTROY, 0, 0);
        m_rsText.Clear();
        PostDestroy();
    }

    void SetPosition(int x, int y) noexcept
    {
        __super::SetPosition(x, y);
        CallEvent(WM_MOVE, 0, 0);
    }
    void SetSize(int cx, int cy) noexcept
    {
        __super::SetSize(cx, cy);
        CallEvent(WM_SIZE, 0, 0);
    }
    void SetRect(const RECT& rc) noexcept
    {
        __super::SetRect(rc);
        CallEvent(WM_MOVE, 0, 0);
        CallEvent(WM_SIZE, 0, 0);
    }
#pragma region ILayout
    void LoSetPosition(LYTPOINT pt) noexcept override { SetPosition((int)pt.x, (int)pt.y); }
    void LoSetSize(LYTSIZE size) noexcept override { SetSize((int)size.cx, (int)size.cy); }
    void LoSetRect(const LYTRECT& rc) noexcept override
    {
        SetRect({ (int)rc.x, (int)rc.y, int(rc.x + rc.cx), int(rc.y + rc.cy) });
    }
#pragma endregion ILayout

    EckInline void SetStyle(UINT uStyle) noexcept
    {
        const auto uOld = GetStyle();
        __super::SetStyle(uStyle);
        CallEvent(WM_STYLECHANGED, uOld, 0);
    }
    EckInlineNdCe UINT GetStyle() const noexcept { return __super::GetStyle(); }

    EckInline void SetText(std::wstring_view svText) noexcept
    {
        m_rsText = svText;
        CallEvent(WM_SETTEXT, 0, 0);
    }
    EckInlineNdCe const CStringW& GetText() const noexcept { return m_rsText; }

    void BeginPaint(_Out_ PAINTINFO& ps, WPARAM wParam, LPARAM lParam) noexcept;
    void EndPaint(const PAINTINFO& ps) noexcept;

    void Redraw(const TRect& rc) noexcept;
    void Redraw() noexcept;

    EckInline CElement* SetCapture() noexcept;
    EckInlineNdCe CElement* GetCapture() const noexcept;
    EckInline void ReleaseCapture() const noexcept;
    EckInline void SetFocus() noexcept;
    EckInlineNdCe CElement* GetFocus() const noexcept;
    EckInline BOOL SetTimer(UINT_PTR uId, UINT uElapse) noexcept;
    EckInline BOOL KillTimer(UINT_PTR uId) noexcept;
    EckInlineNdCe BOOL IsShowingFocus() const noexcept;

    EckInlineNdCe auto& GetWindow() const noexcept { return *(CEuiWindow*)GetContainer(); }
    EckInlineNdCe HDC GetDC() const noexcept;
    EckInlineNdCe GpGraphics* GetGraphics() const noexcept;

    EckInline void SetTheme(CThemeBase* p) noexcept { m_pTheme = p; }
    EckInlineNdCe CThemeBase* GetTheme() const noexcept { return m_pTheme.Get(); }

    EckInline void SetThemeStyle(CThemeStyle* p) noexcept { m_pThemeStyle = p; }
    EckInlineNdCe auto GetThemeStyle() const noexcept { return m_pThemeStyle.Get(); }
};

using BBEVENT = CElement::BBEVENT;

class CEuiWindow : public UiBasic::CElementContainer<CElement>
{
public:
    struct RENDER_STOCK
    {
        HDC hCDC{};
        GpPtr<GpSolidFill> pBrush{};
        GpPtr<GpPen> pPen{};
        GpPtr<GpStringFormat> pStrFmt{};
        BOOLEAN bStrFmtSingleLine{ TRUE };
        BOOLEAN bStrFmtNoClip{ TRUE };
        BYTE eAlign{};
        BYTE eLineAlign{};

        void SetStringFormatFromDtFlags(UINT uDt) noexcept
        {
            BOOL bSetFlags{};
            int uNewFlags{};
            const BOOLEAN bNoClip = !!(uDt & DT_NOCLIP);
            if (bNoClip != bStrFmtNoClip)
            {
                bSetFlags = TRUE;
                bStrFmtNoClip = bNoClip;
                if (bNoClip)
                    uNewFlags |= Gdiplus::StringFormatFlagsNoClip;
            }
            const BOOLEAN bSingleLine = !!(uDt & DT_SINGLELINE);
            if (bSingleLine != bStrFmtSingleLine)
            {
                bSetFlags = TRUE;
                bStrFmtSingleLine = bSingleLine;
                if (bSingleLine)
                    uNewFlags |= Gdiplus::StringFormatFlagsNoWrap;
            }
            if (bSetFlags)
                GdipSetStringFormatFlags(pStrFmt.Get(), uNewFlags);

            GdipSetStringFormatAlign(pStrFmt.Get(),
                (uDt & DT_RIGHT) ? Gdiplus::StringAlignmentFar :
                (uDt & DT_CENTER) ? Gdiplus::StringAlignmentCenter :
                Gdiplus::StringAlignmentNear);
            GdipSetStringFormatLineAlign(pStrFmt.Get(),
                (uDt & DT_VCENTER) ? Gdiplus::StringAlignmentCenter :
                Gdiplus::StringAlignmentNear);
        }
    };
private:
    CMemoryDC m_DC{};
    GpPtr<GpGraphics> m_pGraphics{};
    RENDER_STOCK m_Stock{};

    UINT m_uRenderFlags{};

    void RdRenderTree(CElement* pEle, const TRect& rc) noexcept
    {
        TRect rcClip;
        TRect rcOldClip{};
        while (pEle)
        {
            const auto rcElem = pEle->GetRectInClient();
            const auto dwStyle = pEle->GetStyle();
            if (!(dwStyle & DES_VISIBLE) || (dwStyle & DES_NO_REDRAW) ||
                IsRectEmpty(rcElem))
                goto NextElement;
            if (!IntersectRect(rcClip, rcElem, rc))
                goto NextElement;
            pEle->CallEvent(WM_PAINT, 0, (LPARAM)&rcClip);
            RdRenderTree(pEle->EtFirstChild(), rcClip);
        NextElement:
            pEle = pEle->EtNext();
        }
    }

    void RdCreateObjectCache() noexcept
    {
        if (m_Stock.hCDC)
            DeleteDC(m_Stock.hCDC);
        const auto hDC = GetDC(HWnd);
        m_Stock.hCDC = CreateCompatibleDC(hDC);
        ReleaseDC(HWnd, hDC);
        GdipCreateSolidFill(0, m_Stock.pBrush.AtClear());
        GdipCreatePen1(0, 1.f, Gdiplus::UnitPixel, m_Stock.pPen.AtClear());
        GdipCreateStringFormat(
            Gdiplus::StringFormatFlagsNoWrap | Gdiplus::StringFormatFlagsNoClip,
            LANG_NEUTRAL, m_Stock.pStrFmt.AtClear());
    }
public:
    LRESULT OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept override
    {
        switch (uMsg)
        {
        case WM_SIZE:
        {
            const auto lResult = __super::OnMessage(uMsg, wParam, lParam);
            m_DC.ReSize(HWnd, GetClientWidth(), GetClientHeight());
            Redraw({ 0, 0, GetClientWidth(), GetClientHeight() });
            return lResult;
        }

        case WM_PAINT:
        case WM_PRINTCLIENT:
        {
            PAINTSTRUCT ps;
            BeginPaint(HWnd, wParam, ps);
            BitBltPs(ps, m_DC.GetDC());
            EndPaint(HWnd, wParam, ps);
        }
        return 0;

        case WM_CREATE:
        {
            const auto lResult = __super::OnMessage(uMsg, wParam, lParam);
            m_DC.FromWindow(HWnd, GetClientWidth(), GetClientHeight());
            const auto hDC = m_DC.GetDC();
            SelectClipRgn(hDC, nullptr);
            SetBkMode(hDC, TRANSPARENT);
            Redraw({ 0, 0, GetClientWidth(), GetClientHeight() });
            return lResult;
        }
        case WM_DESTROY:
        {
            const auto lResult = __super::OnMessage(uMsg, wParam, lParam);
            m_DC.Destroy();
            return lResult;
        }
        }
        return __super::OnMessage(uMsg, wParam, lParam);
    }

    void Redraw(const TRect& rc) noexcept
    {
        TRect rc0;
        IntersectRect(rc0, rc, { 0, 0, GetClientWidth(), GetClientHeight() });
        if (IsRectEmpty(rc0))
            return;
        RdRenderTree(EtFirstChild(), rc0);
        const auto hDC = GetDC(HWnd);
        BitBlt(hDC, rc0.left, rc0.top, rc0.right - rc0.left, rc0.bottom - rc0.top,
            m_DC.GetDC(), rc0.left, rc0.top, SRCCOPY);
        ReleaseDC(HWnd, hDC);
    }

    EckInlineNdCe auto& GetMemoryDC() const noexcept { return m_DC; }
    EckInlineNdCe auto GetGraphics() const noexcept { return m_pGraphics.Get(); }
    EckInlineNdCe auto& GetRenderStock() const noexcept { return m_Stock; }
    EckInline void SetStockStringFormatFromDtFlags(UINT uDt) noexcept
    {
        m_Stock.SetStringFormatFromDtFlags(uDt);
    }

    EckInlineCe void SetRenderFlags(UINT uFlags) noexcept { m_uRenderFlags = uFlags; }
    EckInlineNdCe UINT GetRenderFlags() const noexcept { return m_uRenderFlags; }
};

inline void CElement::Create(std::wstring_view svText, UINT uStyle, UINT uExStyle,
    int x, int y, int cx, int cy, CElement* pParent, CEuiWindow* pWnd,
    int iId, PCVOID pData) noexcept
{
    if (!pWnd && pParent)
        pWnd = &pParent->GetWindow();
    m_rsText = svText;
    SetId(iId);
    PreCreate(TRect{ x, y, x + cx, y + cy }, uStyle, pParent, pWnd);
    CallEvent(WM_CREATE, 0, (LPARAM)pData);
    CallEvent(WM_MOVE, 0, 0);
    CallEvent(WM_SIZE, 0, 0);
    PostCreate();
}

inline void CElement::BeginPaint(_Out_ PAINTINFO& ps,
    WPARAM wParam, LPARAM lParam) noexcept
{
    const auto hDC = GetDC();
    const auto* const prcClip = (TRect*)lParam;
    ps.rcClipInClient = *prcClip;
    if (!(GetStyle() & DES_NO_CLIP))
    {
        GetClipBox(hDC, &ps.rcOldClip);
        IntersectClipRect(hDC,
            prcClip->left, prcClip->top, prcClip->right, prcClip->bottom);
        if (GetWindow().GetRenderFlags() & (RDF_GDIX_GEO | RDF_GDIX_TEXT))
        {
            GdipSetClipRectI(GetGraphics(),
                prcClip->left, prcClip->top,
                prcClip->right - prcClip->left,
                prcClip->bottom - prcClip->top,
                Gdiplus::CombineModeIntersect);
        }
    }
}
inline void CElement::EndPaint(const PAINTINFO& ps) noexcept
{
    const auto hDC = GetDC();
    if (!(GetStyle() & DES_NO_CLIP))
    {
        if (EtParent())
        {
            const auto hRgn = CreateRectRgnIndirect(&ps.rcOldClip);
            SelectClipRgn(hDC, hRgn);
            DeleteObject(hRgn);
        }
        else
            SelectClipRgn(hDC, nullptr);
        if (GetWindow().GetRenderFlags() & (RDF_GDIX_GEO | RDF_GDIX_TEXT))
        {
            GdipSetClipRectI(GetGraphics(),
                ps.rcOldClip.left, ps.rcOldClip.top,
                ps.rcOldClip.right - ps.rcOldClip.left,
                ps.rcOldClip.bottom - ps.rcOldClip.top,
                Gdiplus::CombineModeReplace);
        }
    }
}

inline void CElement::Redraw(const TRect& rc) noexcept
{
    TRect rc0{ rc };
    ElementToClient(rc0);
    IntersectRect(rc0, rc0, GetRectInClient());
    GetWindow().Redraw(rc);
}
inline void CElement::Redraw() noexcept
{
    GetWindow().Redraw(GetRectInClient());
}

EckInlineNdCe HDC CElement::GetDC() const noexcept
{
    return GetWindow().GetMemoryDC().GetDC();
}
EckInlineNdCe GpGraphics* CElement::GetGraphics() const noexcept
{
    return GetWindow().GetGraphics();
}

EckInline CElement* CElement::SetCapture() noexcept { return GetWindow().EleSetCapture(this); }
EckInlineNdCe CElement* CElement::GetCapture() const noexcept { return GetWindow().EleGetCapture(); }
EckInline void CElement::ReleaseCapture() const noexcept { GetWindow().EleReleaseCapture(); }
EckInline void CElement::SetFocus() noexcept { GetWindow().EleSetFocus(this); }
EckInlineNdCe CElement* CElement::GetFocus() const noexcept { return GetWindow().EleGetFocus(); }
EckInline BOOL CElement::SetTimer(UINT_PTR uId, UINT uElapse) noexcept { return GetWindow().EleSetTimer(this, uId, uElapse); }
EckInline BOOL CElement::KillTimer(UINT_PTR uId) noexcept { return GetWindow().EleKillTimer(this, uId); }
EckInlineNdCe BOOL CElement::IsShowingFocus() const noexcept { return GetWindow().EleIsShowingFocus(); }

class CUiaBase : public CElement::CUiaElement
{
public:
    STDMETHODIMP GetPropertyValue(PROPERTYID idProp, VARIANT* pRetVal) override
    {
        const auto pEle = (CElement*)m_pEle;
        switch (idProp)
        {
        case UIA_NamePropertyId:
            pRetVal->vt = VT_BSTR;
            pRetVal->bstrVal = pEle->GetText().ToBSTR();
            return S_OK;
        }
        return __super::GetPropertyValue(idProp, pRetVal);
    }
};
inline HRESULT CElement::EhUiaMakeInterface() noexcept
{
    const auto p = new CUiaBase{};
    UiaSetInterface(p);
    p->Release();
    return S_OK;
}

inline TmResult TmGenericDrawBackground(
    CElement* pEle,
    const CThemeStyle::Style* pStyle,
    const RECT& rc) noexcept
{
    const auto uRenderFlags = pEle->GetWindow().GetRenderFlags();
    const auto& Stock = pEle->GetWindow().GetRenderStock();
    if (uRenderFlags & RDF_GDI_GEO)
    {
        const auto hDC = pEle->GetDC();
        SetDCBrushColor(hDC, ArgbToColorref(pStyle->argbBack));
        const auto hBrush = GetStockBrush(DC_BRUSH);
        FillRect(hDC, &rc, hBrush);

        if (pStyle->dLeft || pStyle->dRight ||
            pStyle->dTop || pStyle->dBottom)
        {
            if (const auto pImg = pStyle->pImgBack.Get();
                pImg && pImg->GdiGet())
            {
                const auto hOld = SelectObject(Stock.hCDC, pImg->GdiGet());
                DrawBackgroundImage32(hDC, Stock.hCDC,
                    rc, pImg->GdiGetWidth(), pImg->GdiGetHeight(),
                    pStyle->eImgModeBack, TRUE);
                SelectObject(Stock.hCDC, hOld);
            }
            SetDCBrushColor(hDC, ArgbToColorref(pStyle->argbBorder));
            RECT rcBorder{ rc };
            if (pStyle->dLeft)
            {
                rcBorder.right = rc.left + pStyle->dLeft;
                FillRect(hDC, &rcBorder, hBrush);
                rcBorder.right = rc.right;
            }
            if (pStyle->dTop)
            {
                rcBorder.bottom = rc.top + pStyle->dTop;
                FillRect(hDC, &rcBorder, hBrush);
                rcBorder.bottom = rc.bottom;
            }
            if (pStyle->dRight)
            {
                rcBorder.left = rc.right - pStyle->dRight;
                FillRect(hDC, &rcBorder, hBrush);
                rcBorder.left = rc.left;
            }
            if (pStyle->dBottom)
            {
                rcBorder.top = rc.bottom - pStyle->dBottom;
                FillRect(hDC, &rcBorder, hBrush);
                rcBorder.top = rc.top;
            }
        }
    }
    else if (uRenderFlags & RDF_GDIX_GEO)
    {
        const auto pGraphics = pEle->GetGraphics();
        GdipSetSolidFillColor(Stock.pBrush.Get(), pStyle->argbBack);
        GdipFillRectangle(pGraphics, Stock.pBrush.Get(),
            (float)rc.left, (float)rc.top,
            float(rc.right - rc.left),
            float(rc.bottom - rc.top));
        if (pStyle->dLeft || pStyle->dRight ||
            pStyle->dTop || pStyle->dBottom)
        {
            if (const auto pImg = pStyle->pImgBack.Get();
                pImg && pImg->GpGet())
            {
                UINT cx, cy;
                GdipGetImageWidth(pImg->GpGet(), &cx);
                GdipGetImageHeight(pImg->GpGet(), &cy);
                DrawBackgroundImage(pGraphics, pImg->GpGet(),
                    rc, (int)cx, (int)cy,
                    pStyle->eImgModeBack, TRUE);
            }
            if (pStyle->dLeft)
                GdipFillRectangle(pGraphics, Stock.pBrush.Get(),
                    (float)rc.left,
                    (float)rc.top,
                    (float)pStyle->dLeft,
                    float(rc.bottom - rc.top));
            if (pStyle->dTop)
                GdipFillRectangle(pGraphics, Stock.pBrush.Get(),
                    (float)rc.left,
                    (float)rc.top,
                    float(rc.right - rc.left),
                    (float)pStyle->dTop);
            if (pStyle->dRight)
                GdipFillRectangle(pGraphics, Stock.pBrush.Get(),
                    float(rc.right - pStyle->dRight),
                    (float)rc.top,
                    (float)pStyle->dRight,
                    float(rc.bottom - rc.top));
            if (pStyle->dBottom)
                GdipFillRectangle(pGraphics, Stock.pBrush.Get(),
                    (float)rc.left,
                    float(rc.top - pStyle->dBottom),
                    float(rc.right - rc.left),
                    (float)pStyle->dBottom);
        }
    }
    return TmResult::Ok;
}

inline TmResult TmGenericDrawText(
    CElement* pEle,
    const CThemeStyle::Style* pStyle,
    const RECT& rc,
    UINT uDtFlags) noexcept
{
    const auto uRenderFlags = pEle->GetWindow().GetRenderFlags();
    const auto& Stock = pEle->GetWindow().GetRenderStock();
    const auto& rsText = pEle->GetText();
    if (rsText.IsEmpty())
        return TmResult::Ok;
    uDtFlags &= ~(DT_CALCRECT);
    uDtFlags |= (DT_NOPREFIX);
    if (uRenderFlags & (RDF_GDI_TEXT | RDF_PREFER_GDI))
    {
        const auto hDC = pEle->GetDC();
        SetTextColor(hDC, ArgbToColorref(pStyle->argbFore));
        const auto hOld = SelectObject(hDC, pStyle->pFont->GdiGet());
        DrawTextW(hDC, rsText.Data(), rsText.Size(),
            (RECT*)&rc, uDtFlags);
        SelectObject(hDC, hOld);
    }
    else if (uRenderFlags & RDF_GDIX_TEXT)
    {
        const auto pFont = pStyle->pFont->GpGet();
        if (!pFont)
            return TmResult::NoFont;
        const auto rcF = MakeGpRectF(rc);
        pEle->GetWindow().SetStockStringFormatFromDtFlags(uDtFlags);
        GdipSetSolidFillColor(Stock.pBrush.Get(), pStyle->argbFore);
        GdipDrawString(pEle->GetGraphics(), rsText.Data(), rsText.Size(),
            pFont, &rcF, Stock.pStrFmt.Get(), Stock.pBrush.Get());
    }
    return TmResult::Ok;
}

inline auto TmSelectSubStyle(CElement* pEle) noexcept
{
    const auto pStyle = pEle->GetThemeStyle();
    const auto uState = pEle->TmGetState();
    const CThemeStyle::Style* pSub;
    if (pEle->GetStyle() & DES_DISABLE)
        pSub = pStyle->FindStyle(SaDisable);
    else
    {
        if (uState & SaPressed)
            pSub = pStyle->FindStyle(SaPressed);
        else if (uState & SaHot)
            pSub = pStyle->FindStyle(SaHot);
        else
            pSub = nullptr;
    }

    if (!pSub)
        pSub = pStyle->FindStyle(SaNormal);
    return pSub;
}
ECK_EUI_NAMESPACE_END
ECK_NAMESPACE_END