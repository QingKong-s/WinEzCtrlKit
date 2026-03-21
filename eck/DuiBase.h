#pragma once
#include "DuiCompositor.h"
#include "DuiTheme.h"
#include "CEasyD2D.h"
#include "EasingCurve.h"
#include "CMessageTimer.h"
#include "Color.h"
#ifdef _DEBUG
#include "Random.h"
#endif

#include <dcomp.h>

#define ECK_DUILOCK
#define ECK_DUILOCKWND

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
class CElement : public UiBasic::CElement<CElement, true>
{
    friend class CDuiWindow;
    using TBase = UiBasic::CElement<CElement, true>;
private:
    // 缓存已混合的元素矩形，至少完全包含原始元素矩形，相对客户区
    Kw::Rect m_rcCompInClient{};
    Kw::Rect m_rcRealCompInClient{};// 实际计算得到的混合矩形
    RcPtr<ICompositor> m_pCompositor{};
    union
    {
        void* PRIV_Dummy{};
        ID2D1Bitmap1* m_pCompBitmap;// 内容渲染到的位图
        // 内容渲染到的缓存表面，设置DES_OWNER_COMP_CACHE时有效
        CCompCacheSurface* m_pCompCacheSurface;
    };

    CStringW m_rsText{};
    RcPtr<CThemeBase> m_pTheme{};
    RcPtr<CThemeStyle> m_pThemeStyle{};
    ComPtr<IDWriteTextFormat> m_pTextFormat{};

    void InvalidateInternal(const Kw::Rect* prcInEle, BOOL bUpdateNow) noexcept;

    void SetStyleWorker(DWORD uStyle) noexcept;

    void PostMoveSize(BOOL bSize, BOOL bMove, const Kw::Rect& rcOld) noexcept;
public:
    BOOL Create(std::wstring_view svText, DWORD uStyle, DWORD dwExStyle,
        float x, float y, float cx, float cy, CElement* pParent,
        CDuiWindow* pWnd = nullptr, int iId = 0, PCVOID pData = nullptr) noexcept;

    void Destroy() noexcept;

    HRESULT EhUiaMakeInterface() noexcept override;

    BOOL EhTransform(_Inout_ Kw::Vec2& pt, BOOL bInClient) noexcept override
    {
        if (CompIsNeedCoordinateTransform())
        {
            if (bInClient)
                ClientToElement(pt);
            CompTransformCoordinate(pt, TRUE);
            if (bInClient)
                ElementToClient(pt);
            return TRUE;
        }
        return FALSE;
    }

    virtual LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept;

    // 将缓动曲线对象的自定义参数设为this，并注册
    EckInline void InitializeEasingCurve(CEasingCurve* pec) noexcept;

    void CeInflateRectWithExpandRadius(_Inout_ Kw::Rect& rc, float f) noexcept
    {
        auto rcTemp{ rc };
        InflateRect(rcTemp, f, f);
        if (IntersectRect(rcTemp, rcTemp, GetWholeRectInClient()))// 裁剪到元素矩形
            UnionRect(rc, rc, rcTemp);
    }

    EckInlineNdCe CDuiWindow& GetWindow() const noexcept { return *(CDuiWindow*)GetContainer(); }
    EckInlineNdCe ID2D1DeviceContext* GetDC() const noexcept;
    EckInlineNdCe ID2D1Bitmap1* CcGetBitmap() const noexcept;

    void SetRect(const Kw::Rect& rc) noexcept
    {
        const auto rcOld = GetRect();
        __super::SetRect(rc);
        PostMoveSize(TRUE, TRUE, rcOld);
    }

    void SetPosition(float x, float y) noexcept
    {
        const auto rcOld = GetWholeRectInClient();
        __super::SetPosition(x, y);
        PostMoveSize(FALSE, TRUE, rcOld);
    }

    void SetSize(float cx, float cy) noexcept
    {
        const auto rcOld = GetWholeRectInClient();
        __super::SetSize(cx, cy);
        PostMoveSize(TRUE, FALSE, rcOld);
    }

    void LoSetPosition(LYTPOINT pt) noexcept override { SetPosition((float)pt.x, (float)pt.y); }
    void LoSetSize(LYTSIZE size) noexcept override { SetSize((float)size.cx, (float)size.cy); }
    void LoSetRect(const LYTRECT& rc) noexcept override
    {
        SetRect({ (float)rc.x, (float)rc.y, float(rc.x + rc.cx), float(rc.y + rc.cy) });
    }

    EckInlineNdCe D2D1_RECT_F GetRectInClientD2D() const noexcept { return Kw::MakeD2DRectF(GetRectInClient()); }
    EckInlineNdCe D2D1_RECT_F GetViewRectD2D() const noexcept { return Kw::MakeD2DRectF(GetViewRect()); }

    void SetText(std::wstring_view sv) noexcept
    {
        if (!CallEvent(WM_SETTEXT, sv.size(), (LPARAM)sv.data()))
            m_rsText.Assign(sv.data(), (int)sv.size());
    }
    EckInlineNdCe auto& GetText() const noexcept { return m_rsText; }
protected:
    EckInlineNdCe auto& GetTextMutable() noexcept { return m_rsText; }
public:
    void SetStyle(UINT uStyle) noexcept
    {
        const auto dwOldStyle = GetStyle();
        SetStyleWorker(uStyle);
        CallEvent(WM_STYLECHANGED, dwOldStyle, GetStyle());
    }

    using TBase::GetStyle;
    using TBase::SetZOrder;
    using TBase::SetVisible;

    EckInline void SetTheme(CThemeBase* p) noexcept { m_pTheme = p; }
    EckInlineNdCe auto& GetTheme() const noexcept { return m_pTheme; }

    EckInline void SetThemeStyle(CThemeStyle* p) noexcept { m_pThemeStyle = p; }
    EckInlineNdCe auto& GetThemeStyle() const noexcept { return m_pThemeStyle; }

    void SetCompositor(ICompositor* pCompositor, BOOL bAutoMarkParentComp = TRUE) noexcept
    {
        if (m_pCompositor.Get() == pCompositor)
            return;
        m_pCompositor = pCompositor;
        CompInvalidateCacheBitmap();
        if (m_pCompositor.Get())
            CompUpdateCompositedRect();
        if (m_pCompositor.Get())
        {
            CompMarkDirty();
            if (bAutoMarkParentComp)
                CompMarkChildrenParentComposited(TRUE);
        }
        else
        {
            if (bAutoMarkParentComp)
                CompMarkChildrenParentComposited(FALSE);
        }
    }
    EckInlineNdCe const RcPtr<ICompositor>& GetCompositor() const noexcept { return m_pCompositor; }

    void SetTextFormat(IDWriteTextFormat* pTf) noexcept
    {
        m_pTextFormat = pTf;
        CallEvent(WM_SETFONT, 0, 0);
    }
    EckInlineNdCe IDWriteTextFormat* GetTextFormat() const noexcept { return m_pTextFormat.Get(); }

    EckInline void Invalidate(const Kw::Rect& rcInEle, BOOL bUpdateNow = TRUE) noexcept
    {
        InvalidateInternal(&rcInEle, bUpdateNow);
    }
    EckInline void Invalidate(BOOL bUpdateNow = TRUE) noexcept
    {
        InvalidateInternal(nullptr, bUpdateNow);
    }
    EckInline void Invalidate(const D2D1_RECT_F& rcInEle, BOOL bUpdateNow = TRUE) noexcept
    {
        InvalidateInternal(&Kw::MakeRect(rcInEle), bUpdateNow);
    }

    void BeginPaint(_Out_ PAINTINFO& ps, WPARAM wParam, LPARAM lParam) noexcept;

    EckInline void EndPaint(const PAINTINFO& ps) noexcept
    {
        if (ps.bClip)
            GetDC()->PopAxisAlignedClip();
    }

    EckInline CElement* SetCapture() noexcept;
    EckInlineNdCe CElement* GetCapture() noexcept;
    EckInline void ReleaseCapture() noexcept;
    EckInline void SetFocus() noexcept;
    EckInlineNdCe CElement* GetFocus() noexcept;
    EckInline BOOL SetTimer(UINT_PTR uId, UINT uElapse) noexcept;
    EckInline BOOL KillTimer(UINT_PTR uId) noexcept;

    EckInline void KctWake() noexcept;

    EckInlineNdCe BOOL IsValid() const noexcept { return !!__super::GetContainer(); }

    // 取完全包围元素的矩形，相对客户区
    EckInlineNdCe Kw::Rect GetWholeRectInClient() const noexcept
    {
        return (GetCompositor() && !GetCompositor()->IsInPlace()) ?
            m_rcCompInClient : GetRectInClient();
    }

    EckInlineNdCe CElement* CompGetFirstCompositedAncestor() const noexcept
    {
        if (GetStyle() & DES_PARENT_COMP)
        {
            auto pParent = EtParent();
            while (pParent && !pParent->GetCompositor())
                pParent = pParent->EtParent();
            return pParent;
        }
        return nullptr;
    }

    void CompUpdateCompositedRect() noexcept
    {
        if (!GetCompositor()->IsInPlace())
        {
            GetCompositor()->CalculateCompositedRect(
                this, *(D2D1_RECT_F*)&m_rcCompInClient, TRUE);
            m_rcRealCompInClient = m_rcCompInClient;
            UnionRect(m_rcCompInClient, m_rcCompInClient, GetRectInClient());
        }
    }

    EckInlineNdCe BOOL CompIsNeedCoordinateTransform() const noexcept
    {
        return GetCompositor() || (GetStyle() & DES_PARENT_COMP);
    }

    // 坐标相对元素
    void CompTransformCoordinate(_Inout_ Kw::Vec2& pt, BOOL bNormalToComposited) noexcept
    {
        if (GetCompositor() && !(GetStyle() & DES_PARENT_COMP))// OPTIMIZATION
        {
            GetCompositor()->TransformPoint(this, pt, bNormalToComposited);
            return;
        }
        CElement* pTrans[16]{};
        auto pp = pTrans;
        auto pEle{ this };
        do
        {
            const auto pCompositor = pEle->GetCompositor();
            if (pCompositor)
            {
                *pp++ = pEle;
                if (!(pEle->GetStyle() & DES_PARENT_COMP))
                    break;
                pEle = pEle->EtParent();
            }
            else if (pEle->GetStyle() & DES_PARENT_COMP)
            {
                const auto pAncestor = pEle->CompGetFirstCompositedAncestor();
                if (pAncestor)
                {
                    *pp++ = pAncestor;
                    pEle = pAncestor->EtParent();
                }
                else
                    break;
            }
            else
                break;
        } while (pEle);

        ElementToClient(pt);
        --pp;
        for (; pp >= pTrans; --pp)
        {
            pEle = *pp;
            const auto pCompositor = pEle->GetCompositor();
            if (pCompositor)
            {
                pEle->ClientToElement(pt);
                pCompositor->TransformPoint(pEle, pt, bNormalToComposited);
                pEle->ElementToClient(pt);
            }
            else if (pEle->GetStyle() & DES_PARENT_COMP)
            {
                const auto pAncestor = pEle->CompGetFirstCompositedAncestor();
                pAncestor->ClientToElement(pt);
                pAncestor->CompTransformCoordinate(pt, bNormalToComposited);
                pAncestor->ElementToClient(pt);
            }
        }
        ClientToElement(pt);
    }

    HRESULT CompUpdateCacheBitmap(float cx, float cy) noexcept;

    EckInline void CompInvalidateCacheBitmap() noexcept
    {
        SafeRelease(m_pCompBitmap);
    }

    EckInlineCe void CompMarkDirty() noexcept
    {
        if (GetCompositor())
            __super::SetStyle(__super::GetStyle() | DESP_COMP_CONTENT_INVALID);
    }

    void CompMarkChildrenParentComposited(BOOL bAdd) noexcept
    {
        auto pEle = EtFirstChild();
        while (pEle)
        {
            if (bAdd)
                pEle->SetStyle(pEle->GetStyle() | DES_PARENT_COMP);
            else
                pEle->SetStyle(pEle->GetStyle() & ~DES_PARENT_COMP);
            pEle->CompMarkChildrenParentComposited(bAdd);
            pEle = pEle->EtNext();
        }
    }

    TmResult TmGenericDrawBackground(
        const CThemeStyle::Style* pStyle,
        const D2D1_RECT_F& rc) const noexcept;

    const CThemeStyle::Style* TmSelectSubStyle() const noexcept
    {
        const auto pStyle = GetThemeStyle();
        if (!pStyle)
            return nullptr;
        const auto uState = GetThemeState();
        const CThemeStyle::Style* pSub;
        if (GetStyle() & DES_DISABLE)
            pSub = pStyle->FindStyle(SaDisable);
        else
        {
            if (uState & SaActive)
                pSub = pStyle->FindStyle(SaActive);
            else if (uState & SaHot)
                pSub = pStyle->FindStyle(SaHot);
            else
                pSub = nullptr;
        }

        if (!pSub)
            pSub = pStyle->FindStyle(SaNormal);
        return pSub;
    }

    void DbgDrawFrame() const noexcept;
};

class CDuiWindow : public UiBasic::CElementContainer<CElement>
{
    friend class CElement;
public:
    struct RENDER_STOCK
    {
        ComPtr<ID2D1SolidColorBrush> pBrush{};
        ComPtr<ID2D1Bitmap1> pCacheBitmap{};
        UINT cxCache{}, cyCache{};
        ComPtr<ID2D1Effect> pFxBlur{};
        ComPtr<ID2D1Effect> pFxCrop{};

        void Clear() noexcept
        {
            pBrush.Clear();
            pCacheBitmap.Clear();
            cxCache = cyCache = 0;
            pFxBlur.Clear();
            pFxCrop.Clear();
        }
    };
private:
    constexpr static int TickInterval = 14;

    struct EXPAND_ITEM
    {
        CElement* pEle;
        Kw::Rect rcExpand;
        BOOLEAN bCombined;
        BOOLEAN bVisible;
    };

    struct CUSTOM_LAYER
    {
        const D2D1_LAYER_PARAMETERS1* pParam{};
        ComPtr<ID2D1Layer> pLayer;
    };

#ifdef _DEBUG
    CPcg32 m_Pcg{};
#endif

    std::vector<EXPAND_ITEM> m_vCeEle{};
    std::vector<ITimeLine*> m_vTimeLine{};
    CMessageTimer m_MsgTimer{};

    Kw::Rect m_rcInvalid{};// 注意可能超出客户区
    CEasyD2D m_D2D{};
    union
    {
        void* PRIV_Dummy[4];
        // DComp呈现使用
        struct
        {
            IDCompositionDevice* m_pDcDevice;
            IDCompositionTarget* m_pDcTarget;	// DCompVisual不使用
            IDCompositionVisual* m_pDcVisual;	// 根视觉对象
            // 根视觉对象的内容，DCompVisual下仅此字段是由自身创建的
            IDCompositionSurface* m_pDcSurface;
        };
        // 窗口渲染目标呈现使用
        ID2D1HwndRenderTarget* m_pRtHwnd;
        // 分层窗口使用
        ID2D1GdiInteropRenderTarget* m_pGdiInterop;
        // 翻转交换链使用
        HANDLE m_hEvtSwapChain;                 // 交换链事件对象
    };
    RENDER_STOCK m_Stock{};
    CUSTOM_LAYER m_CustomLayer{};

    float m_fBlurDeviation{ 15.f };

    ARGB m_argbAccent{ 0xFF'66CCFF };

    PresentMode m_ePresentMode{ PresentMode::FlipSwapChain };

    BITBOOL m_bBlurUseLayer : 1{};      // 模糊是否使用图层
    BITBOOL m_bTransparent : 1{ TRUE };       // 窗口是透明的

    BITBOOL m_bFullUpdate : 1{ TRUE };  // 当前是否需要完全重绘
    BITBOOL m_bWaitSwapChain : 1{};

    void BlurpDrawStyle(CElement* pEle,
        const D2D1_RECT_F& rcClipInClient, float ox, float oy) noexcept
    {
        EckAssert(pEle->GetStyle() & DES_BLURBKG);
        GetDeviceContext()->Flush();
        auto rcfClipInElem{ rcClipInClient };
        auto rcfClipInClient{ rcfClipInElem };
        pEle->ClientToElement(rcfClipInElem);
        CcReserveBitmapLogical(rcfClipInClient.right - rcfClipInClient.left,
            rcfClipInClient.bottom - rcfClipInClient.top);
        OffsetRect(rcfClipInClient, ox, oy);
        BlurDrawDC(rcfClipInClient, { rcfClipInElem.left, rcfClipInElem.top },
            BlurGetDeviation(), BlurGetUseLayer());
    }

    void CeAdd(CElement* pEle) noexcept
    {
        auto& v = m_vCeEle;
        EckAssert(std::find_if(v.begin(), v.end(),
            [=](const EXPAND_ITEM& e) {return e.pEle == pEle; }) == v.end());
        v.emplace_back(pEle);
    }
    void CeRemove(CElement* pEle) noexcept
    {
        auto& v = m_vCeEle;
        for (auto it = v.begin(); it != v.end(); ++it)
        {
            if (it->pEle == pEle)
            {
                v.erase(it);
                return;
            }
        }
        EckAssert(FALSE);// 未找到匹配元素
    }
    void CeUnion(_Inout_ Kw::Rect& rcInClient) noexcept
    {
        if (m_vCeEle.empty())
            return;
        BOOL bChanged{};
        for (auto& e : m_vCeEle)
        {
            if (!(e.bVisible = e.pEle->IsVisible()))
                continue;
            const auto rcElem = e.pEle->GetWholeRectInClient();
            if (IsRectsIntersect(rcElem, rcInClient))
            {
                if (e.pEle->GetStyle() & DES_CONTENT_EXPAND)
                    UnionRect(rcInClient, rcInClient, rcElem);
                else
                    e.pEle->CallEvent(EWM_QUERY_EXPAND_RECT, (WPARAM)&rcInClient, 0);
                e.bCombined = bChanged = TRUE;
            }
            else
                e.bCombined = FALSE;
        }
        bChanged = bChanged && m_vCeEle.size() > 1;
        while (bChanged)
        {
            bChanged = FALSE;
            for (auto& e : m_vCeEle)
            {
                if (!e.bVisible)
                    continue;
                if ((e.pEle->GetStyle() & DES_CONTENT_EXPAND) && e.bCombined)
                    continue;
                const auto rcElem = e.pEle->GetWholeRectInClient();
                if (IsRectsIntersect(rcElem, rcInClient))
                {
                    e.bCombined = TRUE;
                    if (e.pEle->GetStyle() & DES_CONTENT_EXPAND)
                    {
                        UnionRect(rcInClient, rcInClient, rcElem);
                        bChanged = TRUE;
                    }
                    else
                    {
                        const auto rcOld{ rcInClient };
                        e.pEle->CallEvent(EWM_QUERY_EXPAND_RECT, (WPARAM)&rcInClient, 0);
                        bChanged = !EqualRect(rcInClient, rcOld);
                    }
                }
            }
        }
    }

    void CcCreateBrush() noexcept
    {
        GetDeviceContext()->CreateSolidColorBrush({}, m_Stock.pBrush.AddrOfClear());
    }

    static EckInlineNdCe D2D1_ALPHA_MODE RdcD2DAlphaMode() noexcept { return D2D1_ALPHA_MODE_PREMULTIPLIED; }
    static EckInlineNdCe DXGI_ALPHA_MODE RdcDxgiAlphaMode() noexcept { return DXGI_ALPHA_MODE_PREMULTIPLIED; }

    void RdRenderTree(CElement* pEle, const Kw::Rect& rc, Priv::PAINT_EXTRA* pExtra) noexcept
    {
        const auto pDC = GetDeviceContext();
        Kw::Rect rcClip;// 相对客户区
        ID2D1Image* pOldTarget{};
        COMP_RENDER_INFO cri;
        IDXGISurface1* pDxgiSurface{};
        ID2D1Bitmap1* pBitmap{};
        D2D1_MATRIX_3X2_F Mat;
        while (pEle)
        {
            const auto rcElem = pEle->GetRectInClient();
            const auto uStyle = pEle->GetStyle();
            if (!(uStyle & DES_VISIBLE) || (uStyle & DES_NO_REDRAW) ||
                IsRectEmpty(rcElem))
                goto NextElem;
            if ((pEle->GetCompositor() &&
                !(uStyle & DES_COMP_NO_REDIRECTION) &&
                IsRectsIntersect(pEle->GetWholeRectInClient(), rc)))
                rcClip = rcElem;
            else if (!IntersectRect(rcClip, rcElem, rc))
                goto NextElem;

            if (pEle->GetCompositor())
            {
                cri.pEle = pEle;
                cri.pDC = GetDeviceContext();
                cri.rcDst = Kw::MakeD2DRectF(pEle->GetViewRect());
                if (uStyle & DES_COMP_NO_REDIRECTION)
                    pEle->GetCompositor()->PreRender(cri);
                else
                {
                    if (!(uStyle & DESP_COMP_CONTENT_INVALID) &&
                        pEle->m_pCompBitmap)
                        goto SkipCompReRender;
                    pDC->Flush();
                    pDC->GetTarget(&pOldTarget);
                    pEle->CompUpdateCacheBitmap(rcElem.right - rcElem.left,
                        rcElem.bottom - rcElem.top);
                    pDC->GetTransform(&Mat);
                    if (pEle->GetStyle() & DES_OWNER_COMP_CACHE)
                    {
                        pDC->SetTarget(pEle->m_pCompCacheSurface->GetBitmap());
                        const auto& rcValid = pEle->m_pCompCacheSurface->GetValidRect();
                        pDC->SetTransform(D2D1::Matrix3x2F::Translation(
                            rcValid.left - rcElem.left, rcValid.top - rcElem.top));
                    }
                    else
                    {
                        pDC->SetTarget(pEle->m_pCompBitmap);
                        pDC->SetTransform(D2D1::Matrix3x2F::Translation(
                            -rcElem.left, -rcElem.top));
                    }
                    pDC->PushAxisAlignedClip((D2D1_RECT_F*)&rcElem, D2D1_ANTIALIAS_MODE_ALIASED);
                    pDC->Clear({});
                    pDC->PopAxisAlignedClip();
                }
            }
            pExtra->prcClipInClient = (D2D1_RECT_F*)&rcClip;
            pEle->CallEvent(WM_PAINT, 0, (LPARAM)pExtra);
            if (pEle->GetCompositor())
            {
                if (uStyle & DES_COMP_NO_REDIRECTION)
                    RdRenderTree(pEle->EtFirstChild(), rcClip, pExtra);
                else
                {
                    RdRenderTree(pEle->EtFirstChild(), rcClip, pExtra);
                    pDC->Flush();
                    pDC->SetTarget(pOldTarget);
                    pOldTarget->Release();
                }
            SkipCompReRender:;
                if (pEle->GetStyle() & DES_OWNER_COMP_CACHE)
                {
                    cri.pBitmap = pEle->m_pCompCacheSurface->GetBitmap();
                    cri.rcSrc = pEle->m_pCompCacheSurface->GetValidRect();
                }
                else
                {
                    cri.pBitmap = pEle->m_pCompBitmap;
                    cri.rcSrc = Kw::MakeD2DRectF(pEle->GetViewRect());
                }
                auto rcRealClip{ pEle->GetWholeRectInClient() };
                IntersectRect(rcRealClip, rcRealClip, rc);
                pEle->ClientToElement(rcRealClip);
                pDC->SetTransform(D2D1::Matrix3x2F::Translation(
                    rcElem.left, rcElem.top));
                pDC->PushAxisAlignedClip((D2D1_RECT_F*)&rcRealClip, D2D1_ANTIALIAS_MODE_ALIASED);
                if (uStyle & DES_BLURBKG)
                {
                    Kw::Rect rc0{ pEle->m_rcRealCompInClient };
                    IntersectRect(rc0, rc0, rc);
                    BlurpDrawStyle(pEle, Kw::MakeD2DRectF(rc0),
                        pExtra->ox, pExtra->oy);
                }
                pEle->GetCompositor()->PostRender(cri);
                pDC->PopAxisAlignedClip();
                pDC->SetTransform(Mat);
#ifdef _DEBUG
                const auto pBr = CcGetBrush();
                pBr->SetColor(D2D1::ColorF(D2D1::ColorF::Aqua));
                pDC->DrawRectangle(Kw::MakeD2DRectF(pEle->m_rcCompInClient), pBr);

                pBr->SetColor(D2D1::ColorF(D2D1::ColorF::Green));
                pDC->DrawRectangle(Kw::MakeD2DRectF(pEle->m_rcRealCompInClient), pBr);

                pBr->SetColor(D2D1::ColorF(D2D1::ColorF::Orange));
                pDC->DrawRectangle(pEle->GetRectInClientD2D(), pBr);
#endif
            }
            else
                RdRenderTree(pEle->EtFirstChild(), rcClip, pExtra);
        NextElem:
            pEle = pEle->EtNext();
        }
    }

    void RdpRender_DComposition(const Kw::Rect& rc, BOOL bFullUpdate = FALSE) noexcept
    {
        EckAssert(GetPresentMode() == PresentMode::DCompositionSurface ||
            GetPresentMode() == PresentMode::DCompositionVisual);
        const auto pDC = GetDeviceContext();
        RENDER_EVENT e;
        ComPtr<IDXGISurface1> pDxgiSurface;
        auto rcPhyF{ rc };
        LogicalToPixel(rcPhyF);
        RECT rcPhy, rcNewPhy;
        CeilRect(rcPhyF, rcPhy);
        // 准备
        e.PreRender.prcDirtyPhy = bFullUpdate ? nullptr : &rcPhy;
        m_pDcSurface->BeginDraw(e.PreRender.prcDirtyPhy,
            IID_PPV_ARGS(&pDxgiSurface), &e.PreRender.ptOffsetPhy);
        e.PreRender.pSfcFinalDst = pDxgiSurface.Get();
        e.PreRender.prcNewDirtyPhy = &rcNewPhy;
        e.PreRender.pSfcNewDst = nullptr;
        const auto rer = OnRenderEvent(RE_PRERENDER, e);
        if (!(rer & RER_REDIRECTION))
        {
            e.PreRender.ptOffsetPhy.x -= rcPhy.left;
            e.PreRender.ptOffsetPhy.y -= rcPhy.top;
        }
        // 准备D2D渲染目标
        const D2D1_BITMAP_PROPERTIES1 D2dBmpProp
        {
            { DXGI_FORMAT_B8G8R8A8_UNORM, RdcD2DAlphaMode() },
            (float)GetUserDpi(),
            (float)GetUserDpi(),
            D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
            nullptr
        };
        ComPtr<ID2D1Bitmap1> pBitmap;
        pDC->CreateBitmapFromDxgiSurface(
            (rer & RER_REDIRECTION) ? e.PreRender.pSfcNewDst : pDxgiSurface.Get(),
            &D2dBmpProp, &pBitmap);
        pDC->SetTarget(pBitmap.Get());
        pDC->BeginDraw();
        pDC->SetTransform(D2D1::Matrix3x2F::Identity());

        Kw::Rect rcF, rcFinalF;
        rcFinalF = Kw::MakeRect(rcPhy);
        OffsetRect(rcFinalF, (float)e.PreRender.ptOffsetPhy.x,
            (float)e.PreRender.ptOffsetPhy.y);
        PixelToLogical(rcFinalF);
        if (rer & RER_REDIRECTION)
        {
            rcF = Kw::MakeRect(rcNewPhy);
            PixelToLogical(rcF);
        }
        else
            rcF = rcFinalF;
        // 画背景
        if (!(rer & RER_NO_ERASE))
        {
            if (bFullUpdate)
            {
                ++rcF.right;
                ++rcF.bottom;
            }
            if (m_bTransparent)
            {
                pDC->PushAxisAlignedClip(Kw::MakeD2DRectF(rcF), D2D1_ANTIALIAS_MODE_ALIASED);
                pDC->Clear({});
                pDC->PopAxisAlignedClip();
            }
            OnRenderEvent(RE_FILLBACKGROUND, *(RENDER_EVENT*)&rcF);
            if (bFullUpdate)
            {
                --rcF.right;
                --rcF.bottom;
            }
        }
        // 画元素树
        const D2D1_POINT_2F ptLogOffsetFinalF
        {
            PixelToLogical((float)e.PreRender.ptOffsetPhy.x),
            PixelToLogical((float)e.PreRender.ptOffsetPhy.y)
        };
        OffsetRect(rcFinalF, -ptLogOffsetFinalF.x, -ptLogOffsetFinalF.y);
        CeilRect(rcFinalF);
        Priv::PAINT_EXTRA Extra;
        if (rer & RER_REDIRECTION)
        {
            const D2D1_POINT_2F ptOrgLogF
            {
                PixelToLogical((float)rcNewPhy.left),
                PixelToLogical((float)rcNewPhy.top)
            };
            Extra.ox = ptOrgLogF.x - rc.left;
            Extra.oy = ptOrgLogF.y - rc.top;
        }
        else
        {
            Extra.ox = ptLogOffsetFinalF.x;
            Extra.oy = ptLogOffsetFinalF.y;
        }
        pDC->SetTransform(D2D1::Matrix3x2F::Translation(Extra.ox, Extra.oy));
        RdRenderTree(EtFirstChild(), rcFinalF, &Extra);

#ifdef _DEBUG
        if (DbgGetDrawDirtyRect())
        {
            ComPtr<ID2D1SolidColorBrush> pBr;
            InflateRect(rcF, -1.f, -1.f);
            pDC->SetTransform(D2D1::Matrix3x2F::Identity());
            ARGB Cr = m_Pcg.Next() | 0xFF000000;
            pDC->CreateSolidColorBrush(D2D1::ColorF(Cr), &pBr);
            pDC->DrawRectangle((D2D1_RECT_F*)&rcF, pBr.Get(), 2.f);
        }
#endif // _DEBUG
        pDC->EndDraw();
        if (rer & RER_REDIRECTION)
            OnRenderEvent(RE_POSTRENDER, e);
        m_pDcSurface->EndDraw();
        pDC->SetTarget(nullptr);

        if (m_ePresentMode == PresentMode::DCompositionVisual)
            OnRenderEvent(RE_COMMIT, e);
        else
            m_pDcDevice->Commit();
    }
    void RdRender(const Kw::Rect& rc, BOOL bFullUpdate = FALSE,
        RECT* prcPhy = nullptr) noexcept
    {
        const auto pDC = GetDeviceContext();
        switch (m_ePresentMode)
        {
        case PresentMode::BitBltSwapChain:
        case PresentMode::FlipSwapChain:
        case PresentMode::WindowRenderTarget:
        case PresentMode::UpdateLayeredWindow:
        {
            pDC->BeginDraw();
            pDC->SetTransform(D2D1::Matrix3x2F::Identity());
            auto rcF{ rc };
            LogicalToPixel(rcF);
            CeilRect(rcF);
            const RECT rcPhy{ MakeRect(rcF) };
            if (prcPhy) *prcPhy = rcPhy;
            PixelToLogical(rcF);
            CeilRect(rcF);
            RENDER_EVENT e;
            const auto rer = OnRenderEvent(RE_PRERENDER, e);
            if (!(rer & RER_NO_ERASE))
            {
                if (bFullUpdate) [[unlikely]]
                {
                    ++rcF.right;
                    ++rcF.bottom;
                }
                if (m_bTransparent)
                {
                    pDC->PushAxisAlignedClip(
                        Kw::MakeD2DRectF(rcF), D2D1_ANTIALIAS_MODE_ALIASED);
                    pDC->Clear({});
                    pDC->PopAxisAlignedClip();
                }
                OnRenderEvent(RE_FILLBACKGROUND, *(RENDER_EVENT*)&rcF);
                if (bFullUpdate) [[unlikely]]
                {
                    --rcF.right;
                    --rcF.bottom;
                }
            }
            Priv::PAINT_EXTRA Extra{};
            RdRenderTree(EtFirstChild(), rcF, &Extra);
            if (m_ePresentMode == PresentMode::UpdateLayeredWindow)
            {
                HDC hDC;
                auto hr = m_pGdiInterop->GetDC(D2D1_DC_INITIALIZE_MODE_COPY, &hDC);
#ifdef _DEBUG
                if (FAILED(hr))
                {
                    EckDbgPrintFormatMessage(hr);
                    EckDbgBreak();
                }
#endif
                const SIZE Size{ GetClientWidth(), GetClientHeight() };
                constexpr POINT ptSrc{};
                const UPDATELAYEREDWINDOWINFO ulwi
                {
                    .cbSize = sizeof(ulwi),
                    .psize = &Size,
                    .hdcSrc = hDC,
                    .pptSrc = &ptSrc,
                    .pblend = &BlendFunctionAlpha,
                    .dwFlags = ULW_ALPHA,
                    .prcDirty = bFullUpdate ? nullptr : &rcPhy,
                };
                UpdateLayeredWindowIndirect(HWnd, &ulwi);
                constexpr RECT rcEmpty{};
                hr = m_pGdiInterop->ReleaseDC(&rcEmpty);
#ifdef _DEBUG
                if (FAILED(hr))
                {
                    EckDbgPrintFormatMessage(hr);
                    EckDbgBreak();
                }
#endif
            }
            pDC->EndDraw();
        }
        return;
        case PresentMode::DCompositionSurface:
        case PresentMode::DCompositionVisual:
            RdpRender_DComposition(rc, bFullUpdate);
            return;
        }
        ECK_UNREACHABLE;
    }

    void RdReSize() noexcept
    {
        m_bFullUpdate = TRUE;

        constexpr auto BitmapOptions =
            D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
        const auto cx = (UINT)std::max(GetClientWidth(), 8);
        const auto cy = (UINT)std::max(GetClientHeight(), 8);
        const auto fDpi = (float)GetUserDpi();
        switch (m_ePresentMode)
        {
        case PresentMode::BitBltSwapChain:
            m_D2D.ReSize(1, cx, cy, 0,
                RdcD2DAlphaMode(), BitmapOptions, fDpi);
            break;
        case PresentMode::FlipSwapChain:
            m_D2D.ReSize(2, cx, cy, DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT,
                RdcD2DAlphaMode(), BitmapOptions, fDpi);
            break;
        case PresentMode::DCompositionSurface:
        case PresentMode::DCompositionVisual:
        {
            IDCompositionSurface* pDcSurface = nullptr;
            m_pDcDevice->CreateSurface(cx, cy,
                DXGI_FORMAT_B8G8R8A8_UNORM,
                DXGI_ALPHA_MODE_PREMULTIPLIED,
                &pDcSurface);
            m_pDcVisual->SetContent(pDcSurface);
            if (m_pDcSurface)
                m_pDcSurface->Release();
            m_pDcSurface = pDcSurface;
        }
        break;
        case PresentMode::WindowRenderTarget:
            m_pRtHwnd->Resize({ cx, cy });
            break;
        case PresentMode::UpdateLayeredWindow:
        {
            SafeRelease(m_D2D.m_pBitmap);
            const D2D1_BITMAP_PROPERTIES1 BmpProp
            {
                { DXGI_FORMAT_B8G8R8A8_UNORM, RdcD2DAlphaMode() },
                fDpi, fDpi,
                D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW |
                D2D1_BITMAP_OPTIONS_GDI_COMPATIBLE,
            };
            GetDeviceContext()->CreateBitmap({ cx, cy },
                nullptr, 0, BmpProp, &m_D2D.m_pBitmap);
            GetDeviceContext()->SetTarget(m_D2D.m_pBitmap);
        }
        break;
        default:
            ECK_UNREACHABLE;
        }
    }

    void RdSetUserDpi() noexcept
    {
        CcDeleteBitmap();
        m_bFullUpdate = TRUE;
        GetDeviceContext()->SetDpi((float)GetUserDpi(), (float)GetUserDpi());
    }

    void RdInitialize() noexcept
    {
        const auto cx = (UINT)std::max(GetClientWidth(), 8);
        const auto cy = (UINT)std::max(GetClientHeight(), 8);
        const auto fDpi = (float)GetUserDpi();
        switch (m_ePresentMode)
        {
        case PresentMode::BitBltSwapChain:
        {
            auto Param = EZD2D_PARAM::MakeBitblt(HWnd, g_pDxgiFactory, g_pDxgiDevice,
                g_pD2DDevice, cx, cy, fDpi);
            Param.uBmpAlphaMode = RdcD2DAlphaMode();
            m_D2D.Create(Param);
        }
        break;
        case PresentMode::FlipSwapChain:
        {
            auto Param = EZD2D_PARAM::MakeFlip(HWnd, g_pDxgiFactory, g_pDxgiDevice,
                g_pD2DDevice, cx, cy, fDpi);
            Param.uBmpAlphaMode = RdcD2DAlphaMode();
            Param.uFlags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
            m_D2D.Create(Param);
            ComPtr<IDXGISwapChain2> pSwapChain2;
            m_D2D.GetSwapChain()->QueryInterface(IID_PPV_ARGS(&pSwapChain2));
            if (pSwapChain2.Get())
            {
                pSwapChain2->SetMaximumFrameLatency(0);
                m_hEvtSwapChain = pSwapChain2->GetFrameLatencyWaitableObject();
            }
        }
        break;
        case PresentMode::DCompositionSurface:
        {
            g_pD2DDevice->CreateDeviceContext(
                EZD2D_PARAM::MakeFlip(0, nullptr, nullptr, nullptr, 0, 0).uDcOptions,
                &m_D2D.m_pDC);

            DCompositionCreateDevice3(g_pDxgiDevice, IID_PPV_ARGS(&m_pDcDevice));
            m_pDcDevice->CreateTargetForHwnd(HWnd, TRUE, &m_pDcTarget);
            m_pDcDevice->CreateVisual(&m_pDcVisual);
            m_pDcDevice->CreateSurface(cx, cy,
                DXGI_FORMAT_B8G8R8A8_UNORM,
                RdcDxgiAlphaMode(),
                &m_pDcSurface);
            m_pDcVisual->SetContent(m_pDcSurface);
            m_pDcTarget->SetRoot(m_pDcVisual);
            m_pDcVisual->SetOffsetX(0.f);
            m_pDcVisual->SetOffsetY(0.f);
            m_pDcDevice->Commit();
        }
        break;
        case PresentMode::WindowRenderTarget:
        {
            D2D1_RENDER_TARGET_PROPERTIES RtProp;
            RtProp.type = D2D1_RENDER_TARGET_TYPE_DEFAULT;
            RtProp.pixelFormat = { DXGI_FORMAT_B8G8R8A8_UNORM, RdcD2DAlphaMode() };
            RtProp.dpiX = RtProp.dpiY = fDpi;
            RtProp.usage = D2D1_RENDER_TARGET_USAGE_NONE;
            RtProp.minLevel = D2D1_FEATURE_LEVEL_DEFAULT;
            D2D1_HWND_RENDER_TARGET_PROPERTIES HwRtProp;
            HwRtProp.hwnd = HWnd;
            HwRtProp.pixelSize = { cx, cy };
            HwRtProp.presentOptions = D2D1_PRESENT_OPTIONS_IMMEDIATELY;
            g_pD2DFactory->CreateHwndRenderTarget(RtProp, HwRtProp, &m_pRtHwnd);
            const auto hr = m_pRtHwnd->QueryInterface(&m_D2D.m_pDC);
            EckAssert(SUCCEEDED(hr));
        }
        break;
        case PresentMode::DCompositionVisual:
        {
            g_pD2DDevice->CreateDeviceContext(
                D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS,
                &m_D2D.m_pDC);

            m_pDcDevice->CreateSurface(cx, cy,
                DXGI_FORMAT_B8G8R8A8_UNORM,
                RdcDxgiAlphaMode(),
                &m_pDcSurface);
            m_pDcVisual->SetContent(m_pDcSurface);
            m_pDcVisual->SetOffsetX(0.f);
            m_pDcVisual->SetOffsetY(0.f);
            RENDER_EVENT e;
            OnRenderEvent(RE_COMMIT, e);
        }
        break;
        case PresentMode::UpdateLayeredWindow:
        {
            g_pD2DDevice->CreateDeviceContext(
                D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS,
                &m_D2D.m_pDC);
            const D2D1_BITMAP_PROPERTIES1 BmpProp
            {
                { DXGI_FORMAT_B8G8R8A8_UNORM, RdcD2DAlphaMode() },
                fDpi, fDpi,
                D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW |
                D2D1_BITMAP_OPTIONS_GDI_COMPATIBLE,
            };
            GetDeviceContext()->CreateBitmap({ cx, cy },
                nullptr, 0, BmpProp, &m_D2D.m_pBitmap);
            GetDeviceContext()->SetTarget(m_D2D.m_pBitmap);
            GetDeviceContext()->QueryInterface(&m_pGdiInterop);
        }
        break;
        default:
            ECK_UNREACHABLE;
        }
        const auto pDC = GetDeviceContext();
        pDC->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
        if (m_ePresentMode != PresentMode::WindowRenderTarget)
            pDC->SetDpi((float)GetUserDpi(), (float)GetUserDpi());

        m_bFullUpdate = TRUE;
        m_rcInvalid = { 0, 0, GetClientWidthLogical(), GetClientHeightLogical() };
        CcCreateBrush();
    }
    void RdUninitialize() noexcept
    {
        m_D2D.Destroy();
        switch (m_ePresentMode)
        {
        case PresentMode::FlipSwapChain:
            if (m_hEvtSwapChain)
            {
                NtClose(m_hEvtSwapChain);
                m_hEvtSwapChain = nullptr;
            }
            break;
        case PresentMode::WindowRenderTarget:
            SafeReleaseAssert0(m_pRtHwnd);
            break;
        case PresentMode::DCompositionSurface:
            SafeReleaseAssert0(m_pDcTarget);
            SafeReleaseAssert0(m_pDcSurface);
            SafeReleaseAssert0(m_pDcVisual);
            m_pDcDevice->Commit();// 冲洗所有清理操作
            m_pDcDevice->WaitForCommitCompletion();
            SafeReleaseAssert0(m_pDcDevice);
            break;
        case PresentMode::DCompositionVisual:
            SafeRelease(m_pDcSurface);
            SafeRelease(m_pDcVisual);
            m_pDcDevice->Commit();// 冲洗所有清理操作
            m_pDcDevice->WaitForCommitCompletion();
            SafeRelease(m_pDcDevice);// 外部对象可能未清理完成
            break;
        case PresentMode::UpdateLayeredWindow:
            SafeReleaseAssert0(m_pGdiInterop);
            break;
        }
    }

    // 若仍存在活动的时间线，返回TRUE
    BOOL KctClockTick(int ms) noexcept
    {
        BOOL bActiveTimeLine{};
        constexpr int iDeltaTime = TickInterval;
        for (const auto e : m_vTimeLine)
        {
            if (e->TlIsValid())
                e->TlTick(iDeltaTime);
            if (!bActiveTimeLine && e->TlIsValid())
                bActiveTimeLine = TRUE;
        }

        if (!bActiveTimeLine)
            m_MsgTimer.Pause();
        return bActiveTimeLine;
    }

    void KctInitialize() noexcept
    {
        m_MsgTimer.SetTargetWindow(HWnd);
        m_MsgTimer.SetMessageValue(WM_TICK);
    }

    void KctShutdown() noexcept
    {
        m_MsgTimer.Stop();
        m_vTimeLine.clear();
    }
public:
    ECK_CWND_CREATE;
    // 一般不覆写此方法
    HWND Create(PCWSTR pszText, DWORD uStyle, DWORD dwExStyle,
        int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = nullptr) noexcept override
    {
        if (m_ePresentMode == PresentMode::FlipSwapChain ||
            m_ePresentMode == PresentMode::DCompositionSurface ||
            m_ePresentMode == PresentMode::DCompositionVisual)
            dwExStyle |= WS_EX_NOREDIRECTIONBITMAP;
        else if (m_ePresentMode == PresentMode::UpdateLayeredWindow)
            dwExStyle |= WS_EX_LAYERED;
        return IntCreate(dwExStyle, WCN_DUIHOST, pszText, uStyle,
            x, y, cx, cy, hParent, hMenu, g_hInstance, nullptr);
    }

    LRESULT OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept override
    {
        switch (uMsg)
        {
        case WM_TICK:
            if (m_bWaitSwapChain)
                NtWaitForSingleObject(m_hEvtSwapChain, FALSE, nullptr);
            KctClockTick(TickInterval);
            return 0;

        case WM_PAINT:
        {
            if (m_ePresentMode == PresentMode::BitBltSwapChain ||
                m_ePresentMode == PresentMode::WindowRenderTarget)
            {
                RECT rcInvalid;
                GetUpdateRect(HWnd, &rcInvalid, FALSE);
                ValidateRect(HWnd, nullptr);
                Kw::Rect rcF{ Kw::MakeRect(rcInvalid) };
                PixelToLogical(rcF);
                RdInvalidate(rcF);
            }
            else
                ValidateRect(HWnd, nullptr);
        }
        return 0;

        case WM_SIZE:
        {
            const auto lResult = __super::OnMessage(uMsg, wParam, lParam);
            RdReSize();
            RdRenderAndPresent();
            return lResult;
        }
        break;

        case WM_COMMAND:// 应用程序可能需要使用菜单
            EtBroadcastEvent(uMsg, wParam, lParam);
            return 0;

        case WM_SETTINGCHANGE:
        {
            if (IsColorSchemeChangeMessage(lParam))
                EtBroadcastEvent(EWM_COLORSCHEMECHANGED, ShouldAppsUseDarkMode(), 0);
        }
        break;

        case WM_CREATE:
        {
            auto lResult = __super::OnMessage(uMsg, wParam, lParam);
            if (!lResult)
            {
                RdInitialize();
                KctInitialize();
            }
            return lResult;
        }
        break;

        case WM_DESTROY:
            KctShutdown();
            RdUninitialize();
            break;
        }

        return __super::OnMessage(uMsg, wParam, lParam);
    }

    virtual LRESULT OnRenderEvent(UINT uMsg, RENDER_EVENT& e) noexcept
    {
        if (uMsg == RE_FILLBACKGROUND)
        {
            //if (m_pBrBkg)
            //    GetDeviceContext()->FillRectangle(e.FillBkg.rc, m_pBrBkg);
        }
        return RER_NONE;
    }

    void SetUserDpi(int iDpi) noexcept
    {
        const auto iOld = GetUserDpi();
        __super::SetUserDpi(iDpi);
        EtBroadcastEvent(WM_DPICHANGED, iOld, 0);
    }

    void KctStartTimer() noexcept { m_MsgTimer.Start(); }

    void KctWake() noexcept { m_MsgTimer.Resume(); }

    EckInline void KctRegisterTimeLine(ITimeLine* pTl) noexcept
    {
        m_vTimeLine.emplace_back(pTl);
    }
    EckInline void KctUnregisterTimeLine(ITimeLine* pTl) noexcept
    {
        const auto it = std::find(m_vTimeLine.begin(), m_vTimeLine.end(), pTl);
        if (it != m_vTimeLine.end())
            m_vTimeLine.erase(it);
    }

    EckInline void Redraw(BOOL bWake = TRUE) noexcept
    {
        m_bFullUpdate = TRUE;
        m_rcInvalid = { 0, 0, GetClientWidthLogical(), GetClientHeightLogical() };
    }

    // 必须在创建窗口之前调用
    EckInline void SetPresentMode(PresentMode ePresentMode) noexcept
    {
        EckAssert(!IsValid());
        m_ePresentMode = ePresentMode;
    }
    EckInlineNdCe PresentMode GetPresentMode() const noexcept { return m_ePresentMode; }

    EckInlineCe void SetTransparent(BOOL bTransparent) noexcept { m_bTransparent = bTransparent; }
    EckInlineNdCe BOOL GetTransparent() const noexcept { return m_bTransparent; }

    EckInlineNdCe ID2D1Bitmap1* CcGetBitmap() const noexcept { return m_Stock.pCacheBitmap.Get(); }

    void CcReserveBitmap(UINT cxPhy, UINT cyPhy) noexcept
    {
        if (cxPhy > m_Stock.cxCache || cyPhy > m_Stock.cyCache)
        {
            m_Stock.cxCache = cxPhy;
            m_Stock.cyCache = cyPhy;
            BmCreate(cxPhy, cyPhy, m_Stock.pCacheBitmap.RefOfClear());
        }
    }
    void CcReserveBitmapLogical(float cx, float cy) noexcept
    {
        CcReserveBitmap((UINT)ceilf(LogicalToPixel(cx)), (UINT)ceilf(LogicalToPixel(cy)));
    }
    void CcDeleteBitmap() noexcept
    {
        m_Stock.cxCache = m_Stock.cyCache = 0;
        m_Stock.pCacheBitmap.Clear();
    }

    ID2D1SolidColorBrush* CcSetBrushColor(const D2D1_COLOR_F& cr) noexcept
    {
        m_Stock.pBrush->SetColor(cr);
        return m_Stock.pBrush.Get();
    }
    ID2D1SolidColorBrush* CcGetBrush() const noexcept { return m_Stock.pBrush.Get(); }

    void BlurInitialize() noexcept
    {
        if (!m_Stock.pFxBlur.Get())
        {
            GetDeviceContext()->CreateEffect(
                CLSID_D2D1GaussianBlur, m_Stock.pFxBlur.AddrOfClear());
            m_Stock.pFxBlur->SetValue(D2D1_GAUSSIANBLUR_PROP_BORDER_MODE,
                D2D1_BORDER_MODE_HARD);
        }
        if (!m_Stock.pFxCrop.Get())
            GetDeviceContext()->CreateEffect(CLSID_D2D1Crop, m_Stock.pFxCrop.AddrOfClear());
    }
private:
    HRESULT BlurpDrawEffect(ID2D1Effect* pFx,
        D2D1_POINT_2F ptDrawing, BOOL bUseLayer) noexcept
    {
        const auto pDC = GetDeviceContext();
#ifdef _DEBUG
        D2D1_LAYER_PARAMETERS1 LyParam
        {
            .contentBounds = D2D1::InfiniteRect(),
            .opacity = 1.f,
        };
        pDC->CreateSolidColorBrush({ .a = 1.f }, (ID2D1SolidColorBrush**)&LyParam.opacityBrush);
#else
        const static D2D1_LAYER_PARAMETERS1 LyParam
        {
            .contentBounds = D2D1::InfiniteRect(),
            .opacity = 1.f,
        };
#endif
        const auto iBlend = pDC->GetPrimitiveBlend();
        pDC->SetPrimitiveBlend(D2D1_PRIMITIVE_BLEND_COPY);
        if (bUseLayer)
            if (m_CustomLayer.pParam)
                pDC->PushLayer(m_CustomLayer.pParam, m_CustomLayer.pLayer.Get());
            else
                pDC->PushLayer(LyParam, nullptr);
        pDC->DrawImage(m_Stock.pFxBlur.Get(), ptDrawing);
        if (bUseLayer)
            pDC->PopLayer();
        pDC->SetPrimitiveBlend(iBlend);
#ifdef _DEBUG
        LyParam.opacityBrush->Release();
#endif
        return S_OK;
    }
public:
    /// <summary>
    /// 模糊当前设备上下文的内容，并画出。
    /// 调用方负责初始化效果与位图缓存，还负责刷新DC上任何挂起的操作
    /// </summary>
    /// <param name="rc">范围，**相对当前位图**，
    /// 也就是说若逻辑上的位图原点不为(0, 0)，则必须手动添加偏移量
    /// </param>
    /// <param name="ptDrawing">效果画出点</param>
    /// <param name="fDeviation">标准差</param>
    /// <param name="bUseLayer">是否使用图层</param>
    /// <returns>HRESULT</returns>
    HRESULT BlurDrawDC(const D2D1_RECT_F& rc,
        D2D1_POINT_2F ptDrawing, float fDeviation, BOOL bUseLayer = FALSE) noexcept
    {
        ComPtr<ID2D1Bitmap1> pBmp;
        ComPtr<ID2D1Image> pTarget;
        GetDeviceContext()->GetTarget(&pTarget);
        pTarget->QueryInterface(&pBmp);
        HRESULT hr;
        float xDpi, yDpi;
        pBmp->GetDpi(&xDpi, &yDpi);
        const D2D1_RECT_U rcU
        {
            UINT32(rc.left * xDpi / 96.f),
            UINT32(rc.top * yDpi / 96.f),
            UINT32(rc.right * xDpi / 96.f),
            UINT32(rc.bottom * yDpi / 96.f)
        };
        if (FAILED(hr = CcGetBitmap()->CopyFromBitmap(nullptr, pBmp.Get(), &rcU)))
            return hr;

        m_Stock.pFxBlur->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, fDeviation);
        m_Stock.pFxCrop->SetValue(D2D1_CROP_PROP_RECT,
            D2D1::RectF(0.f, 0.f, rc.right - rc.left, rc.bottom - rc.top));

        m_Stock.pFxCrop->SetInput(0, CcGetBitmap());
        m_Stock.pFxBlur->SetInputEffect(0, m_Stock.pFxCrop.Get());
        return BlurpDrawEffect(m_Stock.pFxBlur.Get(), ptDrawing, bUseLayer);
    }

    /// <summary>
    /// 模糊指定位图的内容，并画出。
    /// </summary>
    /// <param name="pBmp">输入位图，必须可作输入，即不能是“不能画”的</param>
    /// <param name="rc">范围</param>
    /// <param name="ptDrawing">画出点</param>
    /// <param name="fDeviation">标准差</param>
    /// <param name="bUseLayer">是否使用图层</param>
    /// <returns>HRESULT</returns>
    HRESULT BlurDrawDirect(ID2D1Bitmap1* pBmp, const Kw::Rect& rc,
        D2D1_POINT_2F ptDrawing, float fDeviation, BOOL bUseLayer = FALSE) noexcept
    {
        m_Stock.pFxBlur->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, fDeviation);
        m_Stock.pFxCrop->SetValue(D2D1_CROP_PROP_RECT,
            D2D1::RectF(0.f, 0.f, rc.right - rc.left, rc.bottom - rc.top));

        m_Stock.pFxCrop->SetInput(0, pBmp);
        m_Stock.pFxBlur->SetInputEffect(0, m_Stock.pFxCrop.Get());
        return BlurpDrawEffect(m_Stock.pFxBlur.Get(), ptDrawing, bUseLayer);
    }

    // 模糊指定位图的内容，并画出。
    // 忽略裁剪效果
    HRESULT BlurDrawDirect(ID2D1Bitmap1* pBmp,
        D2D1_POINT_2F ptDrawing, float fDeviation, BOOL bUseLayer = FALSE) noexcept
    {
        m_Stock.pFxBlur->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, fDeviation);
        m_Stock.pFxBlur->SetInput(0, pBmp);
        return BlurpDrawEffect(m_Stock.pFxBlur.Get(), ptDrawing, bUseLayer);
    }

    void BlurSetCustomLayer(const D2D1_LAYER_PARAMETERS1* pParam = nullptr,
        ID2D1Layer* pLayer = nullptr) noexcept
    {
        m_CustomLayer.pParam = pParam;
        m_CustomLayer.pLayer = pLayer;
    }

    EckInlineCe void BlurSetUseLayer(BOOL bUseLayer) noexcept { m_bBlurUseLayer = bUseLayer; }
    EckInlineNdCe BOOL BlurGetUseLayer() const noexcept { return m_bBlurUseLayer; }
    EckInlineCe void BlurSetDeviation(float fDeviation) noexcept { m_fBlurDeviation = fDeviation; }
    EckInlineNdCe float BlurGetDeviation() const noexcept { return m_fBlurDeviation; }

    HRESULT BmCreate(int cxPhy, int cyPhy, _Out_ ID2D1Bitmap1*& pBmp) noexcept
    {
        const D2D1_BITMAP_PROPERTIES1 Prop
        {
            { DXGI_FORMAT_B8G8R8A8_UNORM,D2D1_ALPHA_MODE_PREMULTIPLIED },
            (float)GetUserDpi(),
            (float)GetUserDpi(),
            D2D1_BITMAP_OPTIONS_TARGET
        };
        return GetDeviceContext()->CreateBitmap(D2D1::SizeU(cxPhy, cyPhy),
            nullptr, 0, Prop, &pBmp);
    }
    HRESULT BmCreateLogical(float cx, float cy, _Out_ ID2D1Bitmap1*& pBmp) noexcept
    {
        return BmCreate((int)ceilf(LogicalToPixel(cx)), (int)ceilf(LogicalToPixel(cy)), pBmp);
    }

    /// <summary>
    /// 初始化目标
    /// 使用DCompositionVisual呈现时，初始化渲染到的目标视觉对象
    /// </summary>
    /// <param name="pVisual">目标视觉对象，窗口内容渲染到此对象</param>
    /// <param name="pDevice">DComp设备</param>
    void InitializeDCompositionVisual(IDCompositionVisual* pVisual, IDCompositionDevice* pDevice) noexcept
    {
        EckAssert(m_ePresentMode == PresentMode::DCompositionVisual);
        EckAssert(!m_pDcVisual && !m_pDcSurface && !m_pDcDevice);
        m_pDcVisual = pVisual;
        m_pDcVisual->AddRef();
        m_pDcDevice = pDevice;
        m_pDcDevice->AddRef();
    }

    EckInlineNdCe ID2D1DeviceContext* GetDeviceContext() const noexcept { return m_D2D.GetDC(); }

    void RdInvalidate(const Kw::Rect& rc)
    {
        EckAssert(!IsRectEmpty(rc));
        UnionRect(m_rcInvalid, m_rcInvalid, rc);
        RdRenderAndPresent();
    }

    void RdRenderAndPresent() noexcept
    {
        m_bWaitSwapChain = FALSE;
        const Kw::Rect rcClient{ 0, 0, GetClientWidthLogical(), GetClientHeightLogical() };
        Kw::Rect rc;
        if (m_bFullUpdate)
            rc = rcClient;
        else
        {
            IntersectRect(rc, m_rcInvalid, rcClient);
            if (IsRectEmpty(rc))
                return;
        }

        const auto bWantPhyRect = !m_bFullUpdate && (
            m_ePresentMode == PresentMode::FlipSwapChain ||
            m_ePresentMode == PresentMode::BitBltSwapChain);
        RECT rcPhy;
        RdRender(rc, m_bFullUpdate, bWantPhyRect ? &rcPhy : nullptr);

        DXGI_PRESENT_PARAMETERS pp{};
        if (bWantPhyRect)
        {
            pp.DirtyRectsCount = 1;
            pp.pDirtyRects = &rcPhy;
        }

        switch (m_ePresentMode)
        {
        case PresentMode::FlipSwapChain:
            m_bWaitSwapChain = TRUE;
            [[fallthrough]];
        case PresentMode::BitBltSwapChain:
            if (m_bFullUpdate)
                m_D2D.GetSwapChain()->Present(0, 0);
            else
                m_D2D.GetSwapChain()->Present1(0, 0, &pp);
            break;
        }
        m_rcInvalid = {};
        m_bFullUpdate = FALSE;
    }

    EckInlineNdCe ARGB TmAccentColor() const noexcept { return m_argbAccent; }
};

inline BOOL CElement::Create(std::wstring_view svText, DWORD uStyle, DWORD dwExStyle,
    float x, float y, float cx, float cy, CElement* pParent,
    CDuiWindow* pWnd, int iId, PCVOID pData) noexcept
{
    if (!pWnd)
        pWnd = &pParent->GetWindow();
    m_rsText = svText;
    SetId(iId);
    __super::PreCreate({ x, y, x + cx, y + cy }, 0, pParent, pWnd);
    SetStyleWorker(uStyle);
    CallEvent(WM_CREATE, 0, (LPARAM)pData);
    __super::PostCreate();
    PostMoveSize(TRUE, TRUE, GetWholeRectInClient());
    return TRUE;
}

inline void CElement::Destroy() noexcept
{
    __super::PreDestroy();
    CallEvent(WM_DESTROY, 0, 0);
    __super::PostDestroy();
    if (GetCompositor())
    {
        CompInvalidateCacheBitmap();
        m_pCompositor.Clear();
    }
    m_pTheme.Clear();
    m_pThemeStyle.Clear();
    m_rsText.Clear();
}

inline LRESULT CElement::OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept
{
    switch (uMsg)
    {
    case WM_PAINT:
        if (GetStyle() & DES_BASE_BEGIN_END_PAINT)
        {
            PAINTINFO ps;
            BeginPaint(ps, wParam, lParam);
            EndPaint(ps);
        }
        return 0;
    case WM_ERASEBKGND:
        if ((GetCompositor() && !(GetStyle() & DES_COMP_NO_REDIRECTION)))
            GetDC()->Clear({});
        return TRUE;
    case WM_SIZE:
        if (GetCompositor())
        {
            CompUpdateCompositedRect();
            CompInvalidateCacheBitmap();
        }
        break;
    }
    return __super::OnEvent(uMsg, wParam, lParam);
}

inline void CElement::InvalidateInternal(const Kw::Rect* prcInEle, BOOL bUpdateNow) noexcept
{
    if (!IsVisible())
        return;
    Kw::Rect rcTemp;
    if (!prcInEle || (GetStyle() & DES_CONTENT_EXPAND))
        rcTemp = GetWholeRectInClient();
    else
    {
        rcTemp = *prcInEle;
        ElementToClient(rcTemp);
        IntersectRect(rcTemp, rcTemp, GetWholeRectInClient());// 裁剪到元素矩形
    }
    if (IsRectEmpty(rcTemp))
        return;
    if (!(GetStyle() & DES_COMP_NO_REDIRECTION) &&
        ((GetStyle() & DES_PARENT_COMP) || GetCompositor()))
    {
        auto pEle = this;
        do
        {
            pEle->CompMarkDirty();
        } while (pEle = pEle->EtParent());
    }
    GetWindow().CeUnion(rcTemp);
    GetWindow().RdInvalidate(rcTemp);
    if (bUpdateNow)
        GetWindow().RdRenderAndPresent();
}

inline void CElement::BeginPaint(_Out_ PAINTINFO& ps, WPARAM, LPARAM lParam) noexcept
{
    const auto pExtra = (Priv::PAINT_EXTRA*)lParam;
    ps.rcfClip = *pExtra->prcClipInClient;
    ps.rcfClipInElem = ps.rcfClip;
    ClientToElement(ps.rcfClipInElem);
    ps.ox = pExtra->ox;
    ps.oy = pExtra->oy;
    if (!(GetStyle() & DES_NO_CLIP))
    {
        ps.bClip = TRUE;
        GetDC()->PushAxisAlignedClip(pExtra->prcClipInClient, D2D1_ANTIALIAS_MODE_ALIASED);
    }
    if ((GetStyle() & DES_BLURBKG) && !GetCompositor())
        GetWindow().BlurpDrawStyle(this, ps.rcfClip, ps.ox, ps.oy);
}

EckInline void CElement::InitializeEasingCurve(CEasingCurve* pec) noexcept
{
    pec->SetCallbackData((LPARAM)this);
    GetWindow().KctRegisterTimeLine(pec);
}

inline void CElement::PostMoveSize(BOOL bSize, BOOL bMove, const Kw::Rect& rcOld) noexcept
{
    if (bSize)
    {
        if (GetCompositor() && m_pCompBitmap)
        {
            if (GetStyle() & DES_OWNER_COMP_CACHE)
            {
                const auto& rcValid = m_pCompCacheSurface->GetValidRect();
                if (rcValid.right - rcValid.left < GetWidth() ||
                    rcValid.bottom - rcValid.top < GetHeight())
                    CompInvalidateCacheBitmap();
            }
            else
            {
                const auto size = m_pCompBitmap->GetSize();
                if (size.width < GetWidth() || size.height < GetHeight())
                    CompInvalidateCacheBitmap();
            }
        }
        CallEvent(WM_SIZE, 0, 0);
    }
    if (bMove)
        CallEvent(WM_MOVE, 0, 0);
    Kw::Rect rc;
    UnionRect(rc, rcOld, GetWholeRectInClient());
    if (!IsRectEmpty(rc))
    {
        Invalidate(rc);
        GetWindow().RdInvalidate(rc);
    }
}

inline void CElement::SetStyleWorker(DWORD uStyle) noexcept
{
    if (uStyle & DES_BLURBKG)
        uStyle |= DES_CONTENT_EXPAND;
    const auto dwOld = GetStyle();
    __super::SetStyle(uStyle);
    // 检查DES_CONTENT_EXPAND(_RECT)变动
    const auto bExpandChanged = ((dwOld ^ uStyle) & DESP_EXPANDED);
    const auto bExpand = (uStyle & DESP_EXPANDED);
    if (bExpandChanged)
        bExpand ? GetWindow().CeAdd(this) : GetWindow().CeRemove(this);
    // 检查DES_OWNER_COMP_CACHE变动
    if (((dwOld ^ uStyle) & DES_OWNER_COMP_CACHE) && GetCompositor())
        CompInvalidateCacheBitmap();
}

inline HRESULT CElement::CompUpdateCacheBitmap(float cx, float cy) noexcept
{
    if (m_pCompBitmap)
    {
        float cxOld, cyOld;
        if (GetStyle() & DES_OWNER_COMP_CACHE)
        {
            const auto rc = m_pCompCacheSurface->GetValidRect();
            cxOld = rc.right - rc.left;
            cyOld = rc.bottom - rc.top;
        }
        else
        {
            const auto size = m_pCompBitmap->GetSize();
            cxOld = size.width;
            cyOld = size.height;
        }
        if (cxOld >= cx && cyOld >= cy)
            return S_FALSE;
        else
            CompInvalidateCacheBitmap();
    }

    const int cxPhy = (int)ceilf(LogicalToPixel((float)cx));
    const int cyPhy = (int)ceilf(LogicalToPixel((float)cy));
    if (GetStyle() & DES_OWNER_COMP_CACHE)
    {
        if (FAILED(GetCompositor()->CreateCacheBitmap(
            cxPhy, cyPhy, m_pCompCacheSurface)))
        {
            CREATE_CACHE_BITMAP_INFO ccbi;
            ccbi.cxPhy = cxPhy;
            ccbi.cyPhy = cyPhy;
            ccbi.hr = E_NOTIMPL;
            const auto pNotifyElem = EtParent() ? EtParent() : this;
            if (pNotifyElem->CallEvent(
                EWM_CREATE_CACHE_BITMAP, (WPARAM)&ccbi, 0))
                m_pCompCacheSurface = ccbi.pCacheSurface;
            return ccbi.hr;
        }
    }
    else
        GetWindow().BmCreate(cxPhy, cyPhy, m_pCompBitmap);
    return S_OK;
}

inline TmResult CElement::TmGenericDrawBackground(
    const CThemeStyle::Style* pStyle, const D2D1_RECT_F& rc) const noexcept
{
    const auto pDC = GetDC();
    const auto pBrush = GetWindow().CcSetBrushColor(ArgbToD2DColorF(pStyle->argbBack));
    pDC->FillRectangle(rc, pBrush);

    if (pStyle->dLeft == pStyle->dRight &&
        pStyle->dTop == pStyle->dBottom &&
        pStyle->dLeft == pStyle->dTop)
    {
        if (pStyle->dLeft > 0.f)
        {
            auto rcFrame{ rc };
            InflateRect(rcFrame, -pStyle->dLeft * 0.5f, -pStyle->dLeft * 0.5f);
            pBrush->SetColor(ArgbToD2DColorF(pStyle->argbBorder));
            pDC->DrawRectangle(rc, pBrush, pStyle->dLeft);
        }
        return TmResult::Ok;
    }
    auto rcFrame{ rc };
    pBrush->SetColor(ArgbToD2DColorF(pStyle->argbBorder));
    if (pStyle->dLeft > 0.f)
    {
        const auto t = rcFrame.right;
        rcFrame.right = std::min(rcFrame.left + pStyle->dLeft, t);
        pDC->FillRectangle(rc, pBrush);
        rcFrame.left = rcFrame.right;
        rcFrame.right = t;
    }
    if (pStyle->dRight > 0.f)
    {
        const auto t = rcFrame.left;
        rcFrame.left = std::max(rcFrame.right - pStyle->dRight, t);
        pDC->FillRectangle(rc, pBrush);
        rcFrame.right = rcFrame.left;
        rcFrame.left = t;
    }
    if (pStyle->dTop > 0.f)
    {
        const auto t = rcFrame.bottom;
        rcFrame.bottom = std::min(rcFrame.top + pStyle->dTop, t);
        pDC->FillRectangle(rc, pBrush);
        rcFrame.top = rcFrame.bottom;
        rcFrame.bottom = t;
    }
    if (pStyle->dBottom > 0.f)
    {
        const auto t = rcFrame.top;
        rcFrame.top = std::max(rcFrame.bottom - pStyle->dBottom, t);
        pDC->FillRectangle(rc, pBrush);
        rcFrame.bottom = rcFrame.top;
        rcFrame.top = t;
    }
    return TmResult::Ok;
}

inline void CElement::DbgDrawFrame() const noexcept
{
    if (!(GetStyle() & DES_DBG_FRAME))
        return;
    const auto pBrush = GetWindow().CcSetBrushColor(D2D1::ColorF{ D2D1::ColorF::Red });
    constexpr float FrameWidth = 1.f;
    auto rc{ GetRectInClientD2D() };
    InflateRect(rc, -FrameWidth * 0.5f, -FrameWidth * 0.5f);
    GetDC()->DrawRectangle(rc, pBrush, FrameWidth);
}

EckInlineNdCe ID2D1DeviceContext* CElement::GetDC() const noexcept
{
    return GetWindow().GetDeviceContext();
}

EckInline     CElement* CElement::SetCapture()     noexcept { return GetWindow().EleSetCapture(this); }
EckInlineNdCe CElement* CElement::GetCapture()     noexcept { return GetWindow().EleGetCapture(); }
EckInline     void      CElement::ReleaseCapture() noexcept { GetWindow().EleReleaseCapture(); }
EckInline     void      CElement::SetFocus()       noexcept { GetWindow().EleSetFocus(this); }
EckInlineNdCe CElement* CElement::GetFocus()       noexcept { return GetWindow().EleGetFocus(); }
EckInline     BOOL      CElement::SetTimer(UINT_PTR uId, UINT uElapse) noexcept { return GetWindow().EleSetTimer(this, uId, uElapse); }
EckInline     BOOL      CElement::KillTimer(UINT_PTR uId) noexcept { return GetWindow().EleKillTimer(this, uId); }
EckInline     void      CElement::KctWake() noexcept { GetWindow().KctWake(); }
EckInlineNdCe ID2D1Bitmap1* CElement::CcGetBitmap() const noexcept { return GetWindow().CcGetBitmap(); }

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
    STDMETHODIMP get_BoundingRectangle(UiaRect* pRetVal) override
    {
        if (!m_pEle)
        {
            *pRetVal = {};
            return UIA_E_ELEMENTNOTAVAILABLE;
        }
        auto rc{ ((CElement*)m_pEle)->GetWholeRectInClient() };
        ((CElement*)m_pEle)->LogicalToPixel(rc);
        RECT rcInScr{ (int)rc.left, (int)rc.top, (int)rc.right, (int)rc.bottom };
        ClientToScreen(((CElement*)m_pEle)->GetWindow().HWnd, &rcInScr);
        pRetVal->left = (double)rcInScr.left;
        pRetVal->top = (double)rcInScr.top;
        pRetVal->width = double(rcInScr.right - rcInScr.left);
        pRetVal->height = double(rcInScr.bottom - rcInScr.top);
        return S_OK;
    }

    EckInlineNdCe auto GetElement() const noexcept { return (CElement*)m_pEle; }
};
inline HRESULT CElement::EhUiaMakeInterface() noexcept
{
    const auto p = new CUiaBase{};
    UiaSetInterface(p);
    p->Release();
    return S_OK;
}
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END