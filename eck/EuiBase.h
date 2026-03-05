#pragma once
#include "EuiDef.h"
#include "CMemoryDC.h"
#include "GpPtr.h"

ECK_NAMESPACE_BEGIN
ECK_EUI_NAMESPACE_BEGIN
class CEuiWindow;
class CElement : public UiElement::CElement<CElement, false>
{
private:
    CStringW m_rsText{};
    INT_PTR m_iId{};
public:
    void Create(std::wstring_view svText, UINT uStyle, UINT uExStyle,
        int x, int y, int cx, int cy, CElement* pParent, CEuiWindow* pWnd,
        INT_PTR iId = 0, PCVOID pData = nullptr) noexcept;

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
        __super::SetStyle(uStyle);
        CallEvent(WM_STYLECHANGED, 0, 0);
    }
    EckInlineNdCe UINT GetStyle() const noexcept { return __super::GetStyle(); }

    EckInline void SetId(INT_PTR iId) noexcept { m_iId = iId; }
    EckInlineNdCe INT_PTR GetId() const noexcept { return m_iId; }

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
    EckInlineNdCe CElement* GetCapture() noexcept;
    EckInline void ReleaseCapture() noexcept;
    EckInline void SetFocus() noexcept;
    EckInlineNdCe CElement* GetFocus() noexcept;
    EckInline BOOL SetTimer(UINT_PTR uId, UINT uElapse) noexcept;
    EckInline BOOL KillTimer(UINT_PTR uId) noexcept;
    EckInlineNdCe BOOL IsShowingFocus() const noexcept;

    EckInlineNdCe auto& GetWindow() const noexcept { return *(CEuiWindow*)GetContainer(); }
    EckInlineNdCe HDC GetDC() const noexcept;
    EckInlineNdCe GpGraphics* GetGraphics() const noexcept;
};

class CEuiWindow : public UiElement::CElementContainer<CElement>
{
private:
    CMemoryDC m_DC{};
    GpPtr<GpGraphics> m_pGraphics{};

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
public:
    ECK_CWND_SINGLEOWNER(CEuiWindow);
    ECK_CWND_CREATE_CLS_HINST(WCN_DUIHOST, g_hInstance);
public:
    LRESULT OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept override
    {
        switch (uMsg)
        {
        case WM_CREATE:
        {
            const auto lResult = __super::OnMessage(uMsg, wParam, lParam);
            m_DC.FromWindow(HWnd, GetClientWidth(), GetClientHeight());
            Redraw({ 0, 0, GetClientWidth(), GetClientHeight() });
            return lResult;
        }

        case WM_SIZE:
        {
            const auto lResult = __super::OnMessage(uMsg, wParam, lParam);
            m_DC.ReSize(HWnd, LOWORD(lParam), HIWORD(lParam));
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

    EckInlineCe void SetRenderFlags(UINT uFlags) noexcept { m_uRenderFlags = uFlags; }
    EckInlineNdCe UINT GetRenderFlags() const noexcept { return m_uRenderFlags; }
};

inline void CElement::Create(std::wstring_view svText, UINT uStyle, UINT uExStyle,
    int x, int y, int cx, int cy, CElement* pParent, CEuiWindow* pWnd,
    INT_PTR iId, PCVOID pData) noexcept
{
    m_rsText = svText;
    m_iId = iId;
    OnCreate(TRect{ x, y, x + cx, y + cy }, uStyle, pParent, pWnd);
    CallEvent(WM_CREATE, 0, (LPARAM)pData);
    CallEvent(WM_MOVE, 0, 0);
    CallEvent(WM_SIZE, 0, 0);
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
        if (GetWindow().GetRenderFlags() & RDF_GDIX)
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
        const auto hRgn = CreateRectRgnIndirect(&ps.rcOldClip);
        SelectClipRgn(hDC, hRgn);
        DeleteObject(hRgn);
        if (GetWindow().GetRenderFlags() & RDF_GDIX)
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
EckInlineNdCe CElement* CElement::GetCapture() noexcept { return GetWindow().EleGetCapture(); }
EckInline void CElement::ReleaseCapture() noexcept { GetWindow().EleReleaseCapture(); }
EckInline void CElement::SetFocus() noexcept { GetWindow().EleSetFocus(this); }
EckInlineNdCe CElement* CElement::GetFocus() noexcept { return GetWindow().EleGetFocus(); }
EckInline BOOL CElement::SetTimer(UINT_PTR uId, UINT uElapse) noexcept { return GetWindow().EleSetTimer(this, uId, uElapse); }
EckInline BOOL CElement::KillTimer(UINT_PTR uId) noexcept { return GetWindow().EleKillTimer(this, uId); }
EckInlineNdCe BOOL CElement::IsShowingFocus() const noexcept { return GetWindow().EleIsShowingFocus(); }
ECK_EUI_NAMESPACE_END
ECK_NAMESPACE_END