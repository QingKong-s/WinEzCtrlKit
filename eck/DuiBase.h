#pragma once
#include "CWindow.h"

#include "DuiStdTheme.h"
#include "DuiDef.h"
#include "DuiCompositor.h"

#include "GraphicsHelper.h"
#include "CDwmWndPartMgr.h"

#include "EasingCurve.h"

#include "CEvent.h"
#include "CCriticalSection.h"
#include "CWaitableTimer.h"

#include <dcomp.h>
#include <oleacc.h>

#ifdef _DEBUG
#include "Random.h"

#	define ECK_DUI_DBG_DRAW_FRAME					\
        {											\
            ID2D1SolidColorBrush* ECKPRIV_pBr___;	\
            m_pDC->CreateSolidColorBrush(D2D1::ColorF(1.f, 0.f, 0.f, 1.f), &ECKPRIV_pBr___); \
            if (ECKPRIV_pBr___) {					\
                m_pDC->DrawRectangle(GetViewRectF(), ECKPRIV_pBr___, 1.f);	\
                ECKPRIV_pBr___->Release();			\
            }										\
        }
#else
#	define ECK_DUI_DBG_DRAW_FRAME ;
#endif

#define ECK_ELEMTOP			((::eck::Dui::CElem*)HWND_TOP)
#define ECK_ELEMBOTTOM		((::eck::Dui::CElem*)HWND_BOTTOM)

#define ECK_DUILOCK			::eck::CCsGuard ECKPRIV_DUI_LOCK_GUARD(GetCriticalSection())
#define ECK_DUILOCKWND		ECK_DUILOCK

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
constexpr inline auto DrawTextLayoutFlags =
D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT | D2D1_DRAW_TEXT_OPTIONS_NO_SNAP;

enum : int
{
    CxyMinScrollThumb = 20,
};

class CElem : public ILayout
{
    friend class CDuiWnd;
private:
    //------元素树------
    CElem* m_pNext{};   // 下一元素，Z序高于当前
    CElem* m_pPrev{};   // 上一元素，Z序低于当前
    CElem* m_pParent{};
    CElem* m_pFirstChild{};
    CElem* m_pLastChild{};
    //------UI系统------
    CDuiWnd* m_pWnd{};
    CSignal<Intercept_T, LRESULT, UINT, WPARAM, LPARAM> m_Sig{};
    //------图形，除DC外渲染和UI线程均可读写------
public:
    using HSlot = decltype(m_Sig)::HSlot;
    ID2D1DeviceContext* m_pDC{};        // 未加引用
private:
    // 缓存已混合的元素矩形，至少完全包含原始元素矩形，相对客户区
    D2D1_RECT_F m_rcCompInClient{};
    D2D1_RECT_F m_rcRealCompInClient{}; // 实际计算得到的混合矩形
    CCompositor* m_pCompositor{};       // 混合操作
    union
    {
        void* PRIV_Dummy{};
        ID2D1Bitmap1* m_pCompBitmap;    // 内容渲染到的位图
        // 内容渲染到的缓存表面，设置DES_OWNER_COMP_CACHE时有效
        CCompCacheSurface* m_pCompCacheSurface;
    };
    //------位置，逻辑坐标------
    D2D1_RECT_F m_rc{};                 // 相对父元素
    D2D1_POINT_2F m_ptOffsetInClient{}; // 相对客户区左上角的偏移
    //------属性------
    CStringW m_rsText{};
    INT_PTR m_iId{};
    ITheme* m_pTheme{};
    IDWriteTextFormat* m_pTextFormat{};
    DWORD m_dwStyle{};
    int m_cChildren{};


    BOOL IntCreate(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
        float x, float y, float cx, float cy, CElem* pParent, CDuiWnd* pWnd,
        INT_PTR iId = 0, PCVOID pData = nullptr) noexcept;

    void utcDestroyChild(CElem* pElem) noexcept
    {
        auto pChild = pElem->GetFirstChildElem();
        while (pChild)
        {
            auto pNext = pChild->GetNextElem();
            pChild->Destroy();
            pChild = pNext;
        }
    }

    void tcIrpUnionContentExpandElemRect(CElem* pElem, _Inout_ D2D1_RECT_F& rcInClient) noexcept;
    void tcIrpInvalidate(const D2D1_RECT_F& rcInClient, BOOL bUpdateNow) noexcept;

    constexpr void tcSrpCorrectChildrenRectInClient() const noexcept
    {
        auto pElem = GetFirstChildElem();
        while (pElem)
        {
            pElem->m_ptOffsetInClient = {
                pElem->m_rc.left + m_ptOffsetInClient.x,
                pElem->m_rc.top + m_ptOffsetInClient.y };
            pElem->tcSrpCorrectChildrenRectInClient();
            pElem = pElem->GetNextElem();
        }
    }

    constexpr void tcSetRectWorker(const D2D1_RECT_F& rc) noexcept
    {
        m_rc = rc;
        m_ptOffsetInClient = { m_rc.left,m_rc.top };
        if (m_pParent)
        {
            m_ptOffsetInClient.x += m_pParent->m_ptOffsetInClient.x;
            m_ptOffsetInClient.y += m_pParent->m_ptOffsetInClient.y;
        }
        tcSrpCorrectChildrenRectInClient();
    }

    constexpr void tcSetStyleWorker(DWORD dwStyle) noexcept;

    void tcPostMoveSize(BOOL bSize, BOOL bMove, const D2D1_RECT_F& rcOld) noexcept;
public:
    virtual BOOL Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
        float x, float y, float cx, float cy, CElem* pParent,
        CDuiWnd* pWnd = nullptr, INT_PTR iId = 0, PCVOID pData = nullptr) noexcept
    {
        return IntCreate(pszText, dwStyle, dwExStyle,
            x, y, cx, cy, pParent, pWnd, iId, pData);
    }

    void Destroy() noexcept;

    // 事件处理函数，一般不直接调用此函数
    virtual LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept;

    virtual LRESULT OnNotify(DUINMHDR* pnm, BOOL& bProcessed) noexcept { return 0; }

    // 调用事件处理
    EckInline LRESULT CallEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept
    {
        if (GetCompositor() && uMsg == WM_DPICHANGED)
            CompInvalidateCacheBitmap();
        SlotCtx Ctx{};
        const auto r = m_Sig.Emit2(Ctx, uMsg, wParam, lParam);
        if (Ctx.IsProcessed())
            return r;
        return OnEvent(uMsg, wParam, lParam);
    }

    // pnm = 通知结构，第一个字段必须为DUINMHDR
    EckInline LRESULT GenElemNotify(void* pnm) noexcept;

    // 仅向父元素发送通知，若无父元素，返回0
    // pnm = 通知结构，第一个字段必须为DUINMHDR
    EckInline LRESULT GenElemNotifyParent(void* pnm) noexcept
    {
        BOOL bProcessed{};
        const auto lResult = OnNotify((DUINMHDR*)pnm, bProcessed);
        if (bProcessed)
            return lResult;
        if (GetParentElem())
            return GetParentElem()->CallEvent(WM_NOTIFY, (WPARAM)this, (LPARAM)pnm);
        else
            return 0;
    }

    EckInline void BroadcastEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept
    {
        CallEvent(uMsg, wParam, lParam);
        auto pElem = GetFirstChildElem();
        while (pElem)
        {
            pElem->BroadcastEvent(uMsg, wParam, lParam);
            pElem = pElem->GetNextElem();
        }
    }

    EckInline void BroadcastEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT lStop) noexcept
    {
        if (CallEvent(uMsg, wParam, lParam) == lStop)
            return;
        auto pElem = GetFirstChildElem();
        while (pElem)
        {
            pElem->BroadcastEvent(uMsg, wParam, lParam, lStop);
            pElem = pElem->GetNextElem();
        }
    }

    // 将缓动曲线对象的自定义参数设为this，并注册
    EckInline void InitEasingCurve(CEasingCurve* pEc) noexcept;

    void CeInflateRectWithExpandRadius(_Inout_ D2D1_RECT_F& rc, float f) noexcept
    {
        auto rcTemp{ rc };
        InflateRect(rcTemp, f, f);
        if (IntersectRect(rcTemp, rcTemp, GetWholeRectInClient()))// 裁剪到元素矩形
            UnionRect(rc, rc, rcTemp);
    }

    EckInlineNdCe CDuiWnd* GetWnd() const noexcept { return m_pWnd; }
    EckInlineNdCe ID2D1DeviceContext* GetDC() const noexcept { return m_pDC; }
    // 注意：进行任何连接/断开操作时必须加锁
    EckInlineNdCe auto& GetSignal() noexcept { return m_Sig; }
    EckInlineNdCe CCriticalSection& GetCriticalSection() const noexcept;
    EckInlineNdCe ID2D1Bitmap1* GetCacheBitmap() const noexcept;
#pragma region PosSize
    EckInlineNdCe auto& GetRectF() const noexcept { return m_rc; }
    EckInlineNdCe D2D1_RECT_F GetViewRectF() const noexcept
    {
        return { 0.f,0.f,m_rc.right - m_rc.left,m_rc.bottom - m_rc.top };
    }
    EckInlineNdCe D2D1_RECT_F GetRectInClientF() const noexcept
    {
        return {
            m_ptOffsetInClient.x,
            m_ptOffsetInClient.y,
            m_ptOffsetInClient.x + m_rc.right - m_rc.left,
            m_ptOffsetInClient.y + m_rc.bottom - m_rc.top
        };
    }
    EckInlineNdCe auto& GetOffsetInClientF() const noexcept { return m_ptOffsetInClient; }

    void SetRect(const D2D1_RECT_F& rc) noexcept
    {
        ECK_DUILOCK;
        const auto rcOld = GetWholeRectInClient();
        tcSetRectWorker(rc);
        tcPostMoveSize(TRUE, TRUE, rcOld);
    }

    void SetPosition(float x, float y) noexcept
    {
        ECK_DUILOCK;
        const auto rcOld = GetWholeRectInClient();
        m_rc = { x,y,x + GetWidthF(),y + GetHeightF() };
        m_ptOffsetInClient = { x,y };
        if (m_pParent)
        {
            m_ptOffsetInClient.x += m_pParent->m_ptOffsetInClient.x;
            m_ptOffsetInClient.y += m_pParent->m_ptOffsetInClient.y;
        }
        tcSrpCorrectChildrenRectInClient();
        tcPostMoveSize(FALSE, TRUE, rcOld);
    }

    void SetSize(float cx, float cy) noexcept
    {
        ECK_DUILOCK;
        const auto rcOld = GetWholeRectInClient();
        m_rc.right = m_rc.left + cx;
        m_rc.bottom = m_rc.top + cy;
        tcPostMoveSize(TRUE, FALSE, rcOld);
    }

    EckInlineNdCe float GetWidthF() const noexcept { return m_rc.right - m_rc.left; }
    EckInlineNdCe float GetHeightF() const noexcept { return m_rc.bottom - m_rc.top; }
    EckInlineNdCe float GetPhyWidthF() const noexcept { return Log2PhyF(GetWidthF()); }
    EckInlineNdCe float GetPhyHeightF() const noexcept { return Log2PhyF(GetHeightF()); }

    EckInlineNdCe int Log2Phy(int i) const noexcept;
    EckInlineNdCe float Log2PhyF(float f) const noexcept;
    EckInlineNdCe int Phy2Log(int i) const noexcept;
    EckInlineNdCe float Phy2LogF(float f) const noexcept;
#pragma endregion PosSize
#pragma region ILayout
    void LoSetPosition(LYTPOINT pt) noexcept override { SetPosition((float)pt.x, (float)pt.y); }
    void LoSetSize(LYTSIZE size) noexcept override { SetSize((float)size.cx, (float)size.cy); }
    void LoSetRect(const LYTRECT& rc) noexcept override
    {
        SetRect({ (float)rc.x, (float)rc.y, float(rc.x + rc.cx), float(rc.y + rc.cy) });
    }
    LYTPOINT LoGetPosition() noexcept override { return { (TLytCoord)m_rc.left, (TLytCoord)m_rc.top }; }
    LYTSIZE LoGetSize() noexcept override { return { (TLytCoord)GetWidthF(), (TLytCoord)GetHeightF() }; }
    void LoShow(BOOL bShow) noexcept override { SetVisible(bShow); }
#pragma endregion ILayout
#pragma region ElemTree
    EckInlineNdCe CElem* GetFirstChildElem() const noexcept { return m_pFirstChild; }
    EckInlineNdCe CElem* GetLastChildElem() const noexcept { return m_pLastChild; }
    EckInlineNdCe CElem* GetParentElem() const noexcept { return m_pParent; }
    // 取下一元素，Z序高于当前
    EckInlineNdCe CElem* GetNextElem() const noexcept { return m_pNext; }
    // 取上一元素，Z序低于当前
    EckInlineNdCe CElem* GetPrevElem() const noexcept { return m_pPrev; }

    static CElem* ElemFromPoint(CElem* pElem, POINT pt,
        _Out_opt_ LRESULT* pResult = nullptr) noexcept
    {
        POINT pt0;
        while (pElem)
        {
            if (pElem->GetStyle() & DES_VISIBLE)
            {
                pt0 = pt;
                if (pElem->CompIsNeedCoordinateTransform())
                {
                    pElem->ClientToElem(pt0);
                    pElem->CompTransformCoordinate(pt0, TRUE);
                    pElem->ElemToClient(pt0);
                }
                if (PtInRect(pElem->GetRectInClientF(), pt0))
                {
                    const auto pHit = pElem->ElemFromPoint(pt, pResult);
                    if (pHit)
                        return pHit;
                    else if (LRESULT lResult; (lResult = pElem->CallEvent(
                        WM_NCHITTEST, 0, MAKELPARAM(pt0.x, pt0.y))) != HTTRANSPARENT)
                    {
                        if (pResult)
                            *pResult = lResult;
                        return pElem;
                    }
                }
            }
            pElem = pElem->GetPrevElem();
        }
        if (pResult)
            *pResult = HTNOWHERE;
        return nullptr;
    }
    EckInline CElem* ElemFromPoint(POINT pt, _Out_opt_ LRESULT* pResult = nullptr) noexcept
    {
        return ElemFromPoint(GetLastChildElem(), pt, pResult);
    }

    EckInlineCe void ClientToElem(_Inout_ RECT& rc) const noexcept
    {
        OffsetRect(rc, (int)-m_ptOffsetInClient.x, (int)-m_ptOffsetInClient.y);
    }
    EckInlineCe void ClientToElem(_Inout_ D2D1_RECT_F& rc) const noexcept
    {
        OffsetRect(rc, -m_ptOffsetInClient.x, -m_ptOffsetInClient.y);
    }
    EckInlineCe void ClientToElem(_Inout_ POINT& pt) const noexcept
    {
        pt.x -= (int)m_ptOffsetInClient.x;
        pt.y -= (int)m_ptOffsetInClient.y;
    }
    EckInlineCe void ClientToElem(_Inout_ D2D1_POINT_2F& pt) const noexcept
    {
        pt.x -= m_ptOffsetInClient.x;
        pt.y -= m_ptOffsetInClient.y;
    }
    EckInlineCe void ElemToClient(_Inout_ RECT& rc) const noexcept
    {
        OffsetRect(rc, (int)m_ptOffsetInClient.x, (int)m_ptOffsetInClient.y);
    }
    EckInlineCe void ElemToClient(_Inout_ D2D1_RECT_F& rc) const noexcept
    {
        OffsetRect(rc, m_ptOffsetInClient.x, m_ptOffsetInClient.y);
    }
    EckInlineCe void ElemToClient(_Inout_ POINT& pt) const noexcept
    {
        pt.x += (int)m_ptOffsetInClient.x;
        pt.y += (int)m_ptOffsetInClient.y;
    }
    EckInlineCe void ElemToClient(_Inout_ D2D1_POINT_2F& pt) const noexcept
    {
        pt.x += m_ptOffsetInClient.x;
        pt.y += m_ptOffsetInClient.y;
    }

    void ForEachElem(const std::invocable<CElem*> auto& Fn) noexcept
    {
        auto pElem = GetFirstChildElem();
        while (pElem)
        {
            Fn(pElem);
            pElem->ForEachElem(Fn);
            pElem = pElem->GetNextElem();
        }
    }
#pragma endregion ElemTree
#pragma region OthersProp
    // 【不能在渲染线程调用】
    void SetText(PCWSTR pszText, int cchText = -1) noexcept
    {
        if (cchText < 0)
            cchText = (int)TcsLength(pszText);
        ECK_DUILOCK;
        if (!CallEvent(WM_SETTEXT, cchText, (LPARAM)pszText))
            m_rsText.Assign(pszText, cchText);
    }
    EckInlineNdCe auto& GetText() const noexcept { return m_rsText; }
protected:
    EckInlineNdCe auto& GetText() noexcept { return m_rsText; }
public:
    void SetStyle(DWORD dwStyle) noexcept
    {
        ECK_DUILOCK;
        const auto dwOldStyle = m_dwStyle;
        tcSetStyleWorker(dwStyle);
        CallEvent(WM_STYLECHANGED, dwOldStyle, m_dwStyle);
    }
    EckInlineNdCe DWORD GetStyle() const noexcept { return m_dwStyle; }

    void SetVisible(BOOL b) noexcept
    {
        ECK_DUILOCK;
        DWORD dwStyle = GetStyle();
        if (b)
            if (dwStyle & DES_VISIBLE)
                return;
            else
                dwStyle |= DES_VISIBLE;
        else
            if (dwStyle & DES_VISIBLE)
                dwStyle &= ~DES_VISIBLE;
            else
                return;
        SetStyle(dwStyle);
        InvalidateRect();
    }
    // 是否可见。函数自底向上遍历父元素，如果存在不可见父元素，则返回FALSE，否则返回TRUE。
    EckInlineNdCe BOOL IsVisible() const noexcept
    {
        auto pParent = this;
        do
        {
            if (!(pParent->GetStyle() & DES_VISIBLE))
                return FALSE;
            pParent = pParent->GetParentElem();
        } while (pParent);
        return TRUE;
    }

    void SetZOrder(CElem* pElemAfter) noexcept;

    // 【不能在渲染线程调用】
    void SetTextFormat(IDWriteTextFormat* pTf) noexcept
    {
        ECK_DUILOCK;
        std::swap(m_pTextFormat, pTf);
        if (m_pTextFormat)
            m_pTextFormat->AddRef();
        if (pTf)
            pTf->Release();
        CallEvent(WM_SETFONT, 0, 0);
    }
    EckInlineNdCe IDWriteTextFormat* GetTextFormat() const noexcept { return m_pTextFormat; }

    // 【不能在渲染线程调用】
    EckInlineCe void SetID(INT_PTR iId) noexcept { m_iId = iId; }
    EckInlineNdCe INT_PTR GetID() const noexcept { return m_iId; }

    // 【不能在渲染线程调用】
    void SetTheme(ITheme* pTheme) noexcept
    {
        ECK_DUILOCK;
        std::swap(m_pTheme, pTheme);
        if (m_pTheme)
            m_pTheme->AddRef();
        if (pTheme)
            pTheme->Release();
        CallEvent(WM_THEMECHANGED, 0, 0);
    }
    EckInlineNdCe auto GetTheme() const noexcept { return m_pTheme; }

    void SetCompositor(CCompositor* pCompositor, BOOL bAutoMarkParentComp = TRUE) noexcept
    {
        ECK_DUILOCK;
        if (m_pCompositor == pCompositor)
            return;
        std::swap(m_pCompositor, pCompositor);
        if (m_pCompositor)
            m_pCompositor->AddRef();
        if (pCompositor)
            pCompositor->Release();
        CompInvalidateCacheBitmap();
        if (m_pCompositor)
            CompReCalcCompositedRect();
        if (m_pCompositor)
        {
            CompMarkDirty();
            if (bAutoMarkParentComp)
                CompMarkChildrenParentComp(TRUE);
        }
        else
        {
            if (bAutoMarkParentComp)
                CompMarkChildrenParentComp(FALSE);
        }
    }
    EckInlineNdCe CCompositor* GetCompositor() const noexcept { return m_pCompositor; }
#pragma endregion OthersProp
#pragma region ElemFunc

    EckInline void InvalidateRect(const D2D1_RECT_F& rc, BOOL bUpdateNow = TRUE) noexcept
    {
        auto rcInClient{ rc };
        ElemToClient(rcInClient);
        tcIrpInvalidate(rcInClient, bUpdateNow);
    }
    EckInline void InvalidateRect(BOOL bUpdateNow = TRUE) noexcept
    {
        tcIrpInvalidate(GetWholeRectInClient(), bUpdateNow);
    }

    /// <summary>
    /// 开始画图。
    /// 目前所有元素仅能在处理WM_PAINT事件时绘制，处理时必须调用此函数且必须与EndPaint配对
    /// </summary>
    /// <param name="eps">画图信息结构的引用，将返回画图参数</param>
    /// <param name="wParam">事件wParam，目前未用</param>
    /// <param name="lParam">事件lParam</param>
    /// <param name="uFlags">标志，EBPF_常量</param>
    void BeginPaint(_Out_ ELEMPAINTSTRU& eps, WPARAM wParam, LPARAM lParam, UINT uFlags = 0u) noexcept;

    /// <summary>
    /// 结束画图
    /// </summary>
    /// <param name="eps">BeginPaint返回的结构，目前未用</param>
    EckInline void EndPaint(const ELEMPAINTSTRU& eps) noexcept
    {
        m_pDC->PopAxisAlignedClip();
    }

    // 【下列函数不能在渲染线程调用】

    EckInline CElem* SetCapture() noexcept;
    EckInlineNdCe CElem* GetCapture() noexcept;
    EckInline void ReleaseCapture() noexcept;
    EckInline void SetFocus() noexcept;
    EckInlineNdCe CElem* GetFocus() noexcept;
    EckInline BOOL SetTimer(UINT_PTR uId, UINT uElapse) noexcept;
    EckInline BOOL KillTimer(UINT_PTR uId) noexcept;

    EckInlineNdCe BOOL IsValid() const noexcept { return !!GetWnd(); }
#pragma endregion ElemFunc
#pragma region Composite
    // 取完全包围元素的矩形，相对客户区
    EckInlineNdCe D2D1_RECT_F GetWholeRectInClient() const noexcept
    {
        return (GetCompositor() && !GetCompositor()->IsInPlace()) ?
            m_rcCompInClient : GetRectInClientF();
    }

    EckInlineNdCe CElem* CompGetFirstCompositedAncestor() const noexcept
    {
        if (GetStyle() & DES_PARENT_COMP)
        {
            auto pParent = GetParentElem();
            while (pParent && !pParent->GetCompositor())
                pParent = pParent->GetParentElem();
            return pParent;
        }
        return nullptr;
    }

    void CompReCalcCompositedRect() noexcept
    {
        ECK_DUILOCK;
        if (!GetCompositor()->IsInPlace())
        {
            GetCompositor()->CalcCompositedRect(this, m_rcCompInClient, TRUE);
            m_rcRealCompInClient = m_rcCompInClient;
            UnionRect(m_rcCompInClient, m_rcCompInClient, GetRectInClientF());
        }
    }

    EckInlineNdCe BOOL CompIsNeedCoordinateTransform() const noexcept
    {
        return GetCompositor() || (GetStyle() & DES_PARENT_COMP);
    }

    void CompTransformCoordinate(_Inout_ POINT& pt, BOOL bNormalToComposited) noexcept
    {
        if (GetCompositor() && !(GetStyle() & DES_PARENT_COMP))// OPTIMIZATION
        {
            if (bNormalToComposited)
                GetCompositor()->PtNormalToComposited(this, pt);
            else
                GetCompositor()->PtCompositedToNormal(this, pt);
            return;
        }
        CElem* pTrans[16]{};
        auto pp = pTrans;
        auto pElem{ this };
        do
        {
            const auto pCompositor = pElem->GetCompositor();
            if (pCompositor)
            {
                *pp++ = pElem;
                if (!(pElem->GetStyle() & DES_PARENT_COMP))
                    break;
                pElem = pElem->GetParentElem();
            }
            else if (pElem->GetStyle() & DES_PARENT_COMP)
            {
                const auto pAncestor = pElem->CompGetFirstCompositedAncestor();
                if (pAncestor)
                {
                    *pp++ = pAncestor;
                    pElem = pAncestor->GetParentElem();
                }
                else
                    break;
            }
            else
                break;
        } while (pElem);

        ElemToClient(pt);
        --pp;
        for (; pp >= pTrans; --pp)
        {
            pElem = *pp;
            const auto pCompositor = pElem->GetCompositor();
            if (pCompositor)
            {
                pElem->ClientToElem(pt);
                if (bNormalToComposited)
                    pCompositor->PtNormalToComposited(pElem, pt);
                else
                    pCompositor->PtCompositedToNormal(pElem, pt);
                pElem->ElemToClient(pt);
            }
            else if (pElem->GetStyle() & DES_PARENT_COMP)
            {
                const auto pAncestor = pElem->CompGetFirstCompositedAncestor();
                pAncestor->ClientToElem(pt);
                pAncestor->CompTransformCoordinate(pt, bNormalToComposited);
                pAncestor->ElemToClient(pt);
            }
        }
        ClientToElem(pt);
    }

    HRESULT CompUpdateCacheBitmap(float cx, float cy) noexcept;

    EckInline void CompInvalidateCacheBitmap() noexcept
    {
        SafeRelease(m_pCompBitmap);
    }

    EckInlineCe void CompMarkDirty() noexcept
    {
        if (GetCompositor())
            m_dwStyle |= DESP_COMP_CONTENT_INVALID;
    }

    void CompMarkChildrenParentComp(BOOL bAdd) noexcept
    {
        auto pElem = GetFirstChildElem();
        while (pElem)
        {
            if (bAdd)
                pElem->SetStyle(pElem->GetStyle() | DES_PARENT_COMP);
            else
                pElem->SetStyle(pElem->GetStyle() & ~DES_PARENT_COMP);
            pElem->CompMarkChildrenParentComp(bAdd);
            pElem = pElem->GetNextElem();
        }
    }
#pragma endregion Composite
};

class CDuiWnd : public CWindow, public IDropTarget
{
    friend class CElem;
private:
    struct TIMER
    {
        CElem* pElem;
        UINT_PTR uId;
    };
    struct EXPAND_ITEM
    {
        CElem* pElem;
        D2D1_RECT_F rcExpand;
        BOOL bCombined;
        BOOL bVisible;
    };

    //------元素树------
    CElem* m_pFirstChild{};	// 第一个子元素
    CElem* m_pLastChild{};	// 最后一个子元素
    std::vector<EXPAND_ITEM> m_vContentExpandElem{};	// 内容扩展元素列表
    //------UI系统------
    CElem* m_pFocusElem{};	        // 当前焦点元素
    CElem* m_pHoverElem{};	        // 当前鼠标悬停元素，WM_MOUSELEAVE使用
    CElem* m_pCurrNcHitTestElem{};	// 当前非客户区命中元素
    CElem* m_pMouseCaptureElem{};	// 当前鼠标捕获元素

    inline static CCriticalSection m_cs{};// 渲染线程同步临界区
    CEvent m_EvtRender{};	// 渲染线程事件对象
    HANDLE m_hthRender{};	// 渲染线程句柄

    std::vector<ITimeLine*> m_vTimeLine{};
    std::vector<TIMER> m_vTimer{};      // 需要定时器的元素
    //------拖放------
    CElem* m_pDragDropElem{};           // 当前拖放元素
    ComPtr<IDataObject> m_pDataObj{};
    //------图形------
    D2D1_RECT_F m_rcInvalid{};
    ID2D1SolidColorBrush* m_pBrBkg{};   // 背景画刷
    CEzD2D m_D2D{};
    union
    {
        void* PRIV_Dummy[4];
        // DComp呈现使用
        struct
        {
            IDCompositionDevice* m_pDcDevice;
            IDCompositionTarget* m_pDcTarget;	// DCompVisual不使用
            IDCompositionVisual* m_pDcVisual;	// 根视觉对象
            IDCompositionSurface* m_pDcSurface;	// 根视觉对象的内容，DCompVisual下仅此字段是由自身创建的
        };
        // 窗口渲染目标呈现使用
        ID2D1HwndRenderTarget* m_pRtHwnd;
        // 分层窗口使用
        ID2D1GdiInteropRenderTarget* m_pGdiInterop;
        // 翻转交换链使用
        HANDLE m_hEvtSwapChain;                 // 交换链事件对象
    };

    ID2D1Bitmap1* m_pBmpCache{};
    int m_cxCache{}, m_cyCache{};

    // [0] = 标准浅色，[1] = 标准深色
    std::vector<ComPtr<ITheme>> m_vTheme{};

    ID2D1Effect* m_pFxBlur{};   // 缓存模糊效果
    ID2D1Effect* m_pFxCrop{};   // 缓存裁剪效果

    Priv::CUSTOM_LAYER m_CustomLayer{};
    //------其他------
    int m_cxClient{}, m_cyClient{};
    float m_cxClientLog{}, m_cyClientLog{};
    int m_cChildren{};		    // 子元素数量，UIAccessible使用
    float m_fBlurDeviation{ 15.f };

    PresentMode m_ePresentMode{ PresentMode::FlipSwapChain };

#ifdef _DEBUG
    CPcg32 m_Pcg{};

    BITBOOL m_bDrawDirtyRect : 1{};     // 是否绘制脏矩形
#endif
    BITBOOL m_bEnableDragDrop : 1{};    // 启用拖放
    BITBOOL m_bBlurUseLayer : 1{};      // 模糊是否使用图层
    BITBOOL m_bTransparent : 1{};       // 窗口是透明的

    BITBOOL m_bMouseCaptured : 1{};     // 鼠标是否被捕获
    BITBOOL m_bExit : 1{};              // 渲染线程应当退出
    BITBOOL m_bSizeChanged : 1{};       // 渲染线程应当重设图面大小
    BITBOOL m_bUserDpiChanged : 1{};    // 渲染线程应当重设DPI
    BITBOOL m_bFullUpdate : 1{ TRUE };  // 当前是否需要完全重绘
    BITBOOL m_bDarkMode : 1{};          // 当前是否处于深色模式

    BYTE m_eAlphaMode{ D2D1_ALPHA_MODE_IGNORE };		// 缓存D2D透明模式
    BYTE m_eDxgiAlphaMode{ DXGI_ALPHA_MODE_IGNORE };	// 缓存DXGI透明模式

    USHORT m_iUserDpi{ USER_DEFAULT_SCREEN_DPI };
    USHORT m_iDpi{ USER_DEFAULT_SCREEN_DPI };


    void ElemDestroying(CElem* pElem) noexcept
    {
        if (m_pFocusElem == pElem)
            m_pFocusElem = nullptr;
        if (m_pCurrNcHitTestElem == pElem)
            m_pCurrNcHitTestElem = nullptr;
        if (m_pMouseCaptureElem == pElem)
            ElemReleaseCapture();
        if (m_pHoverElem == pElem)
            m_pHoverElem = nullptr;
        for (size_t i{}; i < m_vTimer.size();)
        {
            if (m_vTimer[i].pElem == pElem)
            {
                KillTimer(HWnd, m_vTimer[i].uId);
                m_vTimer.erase(m_vTimer.begin() + i);
            }
            else
                ++i;
        }
    }

    void BlurpDrawStyleBkg(CElem* pElem, const D2D1_RECT_F& rcClipInClient,
        float ox, float oy) noexcept
    {
        EckAssert(pElem->GetStyle() & DES_BLURBKG);
        GetDeviceContext()->Flush();
        auto rcfClipInElem{ rcClipInClient };
        auto rcfClipInClient{ rcfClipInElem };
        pElem->ClientToElem(rcfClipInElem);
        CacheReserveLogSize(rcfClipInClient.right - rcfClipInClient.left,
            rcfClipInClient.bottom - rcfClipInClient.top);
        OffsetRect(rcfClipInClient, ox, oy);
        BlurDrawDC(rcfClipInClient, { rcfClipInElem.left,rcfClipInElem.top },
            BlurGetDeviation(), BlurGetUseLayer());
    }

    void CeAdd(CElem* pElem) noexcept
    {
        auto& v = m_vContentExpandElem;
        EckAssert(std::find_if(v.begin(), v.end(),
            [=](const EXPAND_ITEM& e) {return e.pElem == pElem; }) == v.end());
        v.emplace_back(pElem);
    }
    void CeRemove(CElem* pElem) noexcept
    {
        auto& v = m_vContentExpandElem;
        for (auto it = v.begin(); it != v.end(); ++it)
        {
            if (it->pElem == pElem)
            {
                v.erase(it);
                return;
            }
        }
        EckAssert(FALSE);// 未找到匹配元素
    }
    void CeUnion(_Inout_ D2D1_RECT_F& rcInClient) noexcept
    {
        if (m_vContentExpandElem.empty())
            return;
        BOOL bChanged{};
        for (auto& e : m_vContentExpandElem)
        {
            if (!(e.bVisible = e.pElem->IsVisible()))
                continue;
            const auto rcElem = e.pElem->GetWholeRectInClient();
            if (IsRectsIntersect(rcElem, rcInClient))
            {
                if (e.pElem->GetStyle() & DES_CONTENT_EXPAND)
                    UnionRect(rcInClient, rcInClient, rcElem);
                else
                    e.pElem->CallEvent(EWM_QUERY_EXPAND_RECT, (WPARAM)&rcInClient, 0);
                e.bCombined = bChanged = TRUE;
            }
            else
                e.bCombined = FALSE;
        }
        bChanged = bChanged && m_vContentExpandElem.size() > 1;
        while (bChanged)
        {
            bChanged = FALSE;
            for (auto& e : m_vContentExpandElem)
            {
                if (!e.bVisible)
                    continue;
                if ((e.pElem->GetStyle() & DES_CONTENT_EXPAND) && e.bCombined)
                    continue;
                const auto rcElem = e.pElem->GetWholeRectInClient();
                if (IsRectsIntersect(rcElem, rcInClient))
                {
                    e.bCombined = TRUE;
                    if (e.pElem->GetStyle() & DES_CONTENT_EXPAND)
                    {
                        UnionRect(rcInClient, rcInClient, rcElem);
                        bChanged = TRUE;
                    }
                    else
                    {
                        const auto rcOld{ rcInClient };
                        e.pElem->CallEvent(EWM_QUERY_EXPAND_RECT, (WPARAM)&rcInClient, 0);
                        bChanged = (rcInClient != rcOld);
                    }
                }
            }
        }
    }

    /// <summary>
    /// 重画元素树
    /// </summary>
    /// <param name="pElem">起始元素</param>
    /// <param name="rc">重画区域，逻辑坐标</param>
    /// <param name="ox">用于DComp图面的X偏移</param>
    /// <param name="oy">用于DComp图面的Y偏移</param>
    /// <param name="oxComp">用于手动混合元素子级的X偏移</param>
    /// <param name="oyComp">用于手动混合元素子级的Y偏移</param>
    void RdRenderTree(CElem* pElem, const D2D1_RECT_F& rc, float ox, float oy,
        BOOL bCompParent = FALSE, float oxComp = 0.f, float oyComp = 0.f) noexcept
    {
        const auto pDC = GetDeviceContext();
        D2D1_RECT_F rcClip;// 相对客户区
        ID2D1Image* pOldTarget{};
        COMP_RENDER_INFO cri;
        IDXGISurface1* pDxgiSurface{};
        ID2D1Bitmap1* pBitmap{};
        Priv::PAINT_EXTRA Extra{ ox,oy };
        while (pElem)
        {
            const auto rcElem = pElem->GetRectInClientF();
            const auto dwStyle = pElem->GetStyle();
            if (!(dwStyle & DES_VISIBLE) || (dwStyle & DES_NO_REDRAW) ||
                IsRectEmpty(rcElem))
                goto NextElem;
            if ((pElem->GetCompositor() && !(dwStyle & DES_COMP_NO_REDIRECTION) &&
                IsRectsIntersect(pElem->GetWholeRectInClient(), rc)))
                rcClip = rcElem;
            else if (!IntersectRect(rcClip, rcElem, rc))
                goto NextElem;

            if (pElem->GetCompositor())
            {
                cri.pElem = pElem;
                cri.pDC = GetDeviceContext();
                cri.rcDst = pElem->GetViewRectF();
                if (dwStyle & DES_COMP_NO_REDIRECTION)
                {
                    pDC->SetTransform(D2D1::Matrix3x2F::Translation(
                        pElem->GetOffsetInClientF().x + ox + oxComp,
                        pElem->GetOffsetInClientF().y + oy + oyComp));
                    pElem->GetCompositor()->PreRender(cri);
                }
                else
                {
                    if (!(dwStyle & DESP_COMP_CONTENT_INVALID) &&
                        pElem->m_pCompBitmap)
                        goto SkipCompReRender;
                    pDC->Flush();
                    pDC->GetTarget(&pOldTarget);
                    pElem->CompUpdateCacheBitmap(rcElem.right - rcElem.left,
                        rcElem.bottom - rcElem.top);
                    if (pElem->GetStyle() & DES_OWNER_COMP_CACHE)
                    {
                        pDC->SetTarget(pElem->m_pCompCacheSurface->GetBitmap());
                        const auto& rcValid = pElem->m_pCompCacheSurface->GetValidRect();
                        pDC->SetTransform(D2D1::Matrix3x2F::Translation(
                            rcValid.left, rcValid.top));
                    }
                    else
                    {
                        pDC->SetTarget(pElem->m_pCompBitmap);
                        pDC->SetTransform(D2D1::Matrix3x2F::Identity());
                    }
                }
            }
            else
            {
                pDC->SetTransform(D2D1::Matrix3x2F::Translation(
                    pElem->GetOffsetInClientF().x + ox + oxComp,
                    pElem->GetOffsetInClientF().y + oy + oyComp));
            }
            pElem->CallEvent(WM_PAINT, (WPARAM)&Extra, (LPARAM)&rcClip);
            if (pElem->GetCompositor())
            {
                if (dwStyle & DES_COMP_NO_REDIRECTION)
                    RdRenderTree(pElem->GetFirstChildElem(), rcClip, ox, oy);
                else
                {
                    RdRenderTree(pElem->GetFirstChildElem(), rcClip, 0.f, 0.f,
                        TRUE, oxComp - rcElem.left, oyComp - rcElem.top);
                    pDC->Flush();
                    pDC->SetTarget(pOldTarget);
                    pOldTarget->Release();
                }
            SkipCompReRender:;
                pDC->SetTransform(D2D1::Matrix3x2F::Translation(
                    pElem->GetOffsetInClientF().x + ox + oxComp,
                    pElem->GetOffsetInClientF().y + oy + oyComp));
                if (pElem->GetStyle() & DES_OWNER_COMP_CACHE)
                {
                    cri.pBitmap = pElem->m_pCompCacheSurface->GetBitmap();
                    cri.rcSrc = pElem->m_pCompCacheSurface->GetValidRect();
                }
                else
                {
                    cri.pBitmap = pElem->m_pCompBitmap;
                    cri.rcSrc = pElem->GetViewRectF();
                }
                auto rcRealClip{ pElem->GetWholeRectInClient() };
                IntersectRect(rcRealClip, rcRealClip, rc);
                pElem->ClientToElem(rcRealClip);
                pDC->PushAxisAlignedClip(rcRealClip, D2D1_ANTIALIAS_MODE_ALIASED);
                if (dwStyle & DES_BLURBKG)
                {
                    D2D1_RECT_F rc0{ pElem->m_rcRealCompInClient };
                    IntersectRect(rc0, rc0, rc);
                    BlurpDrawStyleBkg(pElem, rc0, ox, oy);
                }
                pElem->GetCompositor()->PostRender(cri);
                pDC->PopAxisAlignedClip();
#ifdef _DEBUG
                ID2D1SolidColorBrush* pBr{};
                pDC->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Aqua), &pBr);
                if (pBr)
                {
                    auto rcComp{ pElem->m_rcCompInClient };
                    pElem->ClientToElem(rcComp);
                    pDC->DrawRectangle(rcComp, pBr);

                    pBr->SetColor(D2D1::ColorF(D2D1::ColorF::Green));
                    rcComp = pElem->m_rcRealCompInClient;
                    pElem->ClientToElem(rcComp);
                    pDC->DrawRectangle(rcComp, pBr);

                    pBr->SetColor(D2D1::ColorF(D2D1::ColorF::Orange));
                    pDC->DrawRectangle(pElem->GetViewRectF(), pBr);
                    pBr->Release();
                }
#endif
            }
            else
            {
                if (bCompParent)
                    RdRenderTree(pElem->GetFirstChildElem(), rcClip, ox, oy,
                        TRUE, oxComp, oyComp);
                else
                    RdRenderTree(pElem->GetFirstChildElem(), rcClip, ox, oy);
            }
        NextElem:
            pElem = pElem->GetNextElem();
        }
    }
    void RdRender_DComp(const D2D1_RECT_F& rc, BOOL bFullUpdate = FALSE) noexcept
    {
        EckAssert(GetPresentMode() == PresentMode::DCompositionSurface ||
            GetPresentMode() == PresentMode::DCompositionVisual);
        const auto pDC = GetDeviceContext();
        RENDER_EVENT e;
        ComPtr<IDXGISurface1> pDxgiSurface;
        auto rcPhyF{ rc };
        Log2Phy(rcPhyF);
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
            { DXGI_FORMAT_B8G8R8A8_UNORM,(D2D1_ALPHA_MODE)m_eAlphaMode },
            (float)m_iUserDpi,
            (float)m_iUserDpi,
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

        D2D1_RECT_F rcF, rcFinalF;
        rcFinalF = MakeD2DRectF(rcPhy);
        OffsetRect(rcFinalF, (float)e.PreRender.ptOffsetPhy.x,
            (float)e.PreRender.ptOffsetPhy.y);
        Phy2Log(rcFinalF);
        if (rer & RER_REDIRECTION)
        {
            rcF = MakeD2DRectF(rcNewPhy);
            Phy2Log(rcF);
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
                pDC->PushAxisAlignedClip(rcF, D2D1_ANTIALIAS_MODE_ALIASED);
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
            Phy2LogF((float)e.PreRender.ptOffsetPhy.x),
            Phy2LogF((float)e.PreRender.ptOffsetPhy.y)
        };
        OffsetRect(rcFinalF, -ptLogOffsetFinalF.x, -ptLogOffsetFinalF.y);
        CeilRect(rcFinalF);
        if (rer & RER_REDIRECTION)
        {
            const D2D1_POINT_2F ptOrgLogF
            {
                Phy2LogF((float)rcNewPhy.left),
                Phy2LogF((float)rcNewPhy.top)
            };
            RdRenderTree(GetFirstChildElem(), rcFinalF,
                ptOrgLogF.x - rc.left, ptOrgLogF.y - rc.top);
        }
        else
            RdRenderTree(GetFirstChildElem(), rcFinalF,
                ptLogOffsetFinalF.x, ptLogOffsetFinalF.y);

#ifdef _DEBUG
        if (m_bDrawDirtyRect)
        {
            ComPtr<ID2D1SolidColorBrush> pBr;
            InflateRect(rcF, -1.f, -1.f);
            pDC->SetTransform(D2D1::Matrix3x2F::Identity());
            ARGB Cr = m_Pcg.Next() | 0xFF000000;
            pDC->CreateSolidColorBrush(D2D1::ColorF(Cr), &pBr);
            pDC->DrawRectangle(rcF, pBr.Get(), 2.f);
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
    void RdRender(const D2D1_RECT_F& rc, BOOL bFullUpdate = FALSE,
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
            Log2Phy(rcF);
            CeilRect(rcF);
            const RECT rcPhy{ MakeRect(rcF) };
            if (prcPhy) *prcPhy = rcPhy;
            Phy2Log(rcF);
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
                    pDC->PushAxisAlignedClip(rcF, D2D1_ANTIALIAS_MODE_ALIASED);
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

            RdRenderTree(GetFirstChildElem(), rcF, 0.f, 0.f);
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
                const SIZE Size{ m_cxClient, m_cyClient };
                constexpr POINT ptSrc{};
                const UPDATELAYEREDWINDOWINFO ulwi
                {
                    .cbSize = sizeof(ulwi),
                    .psize = &Size,
                    .hdcSrc = hDC,
                    .pptSrc = &ptSrc,
                    .pblend = &BlendFuncAlpha,
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
            RdRender_DComp(rc, bFullUpdate);
            return;
        }
        ECK_UNREACHABLE;
    }

    void RdThread() noexcept
    {
        constexpr int MinTickInterval = 14;
        CWaitableTimer Timer{};
        BOOL bActiveTimeLine, bWaitSwapChain{};

        WaitObject(m_EvtRender);
        ULONGLONG ullTime = NtGetTickCount64() - MinTickInterval;
        EckLoop()
        {
            if (m_hEvtSwapChain && bWaitSwapChain)
                NtWaitForSingleObject(m_hEvtSwapChain, TRUE, nullptr);
            m_cs.Enter();
            if (m_bExit)
            {
                m_cs.Leave();
                break;
            }

            const auto ullCurrTime = NtGetTickCount64();
            // 滴答所有时间线
            bActiveTimeLine = FALSE;
            int iDeltaTime = int(ullCurrTime - ullTime);
            for (const auto e : m_vTimeLine)
            {
                if (e->TlIsValid())
                    e->TlTick(iDeltaTime);
                if (!bActiveTimeLine && e->TlIsValid())
                    bActiveTimeLine = TRUE;
            }

            D2D1_RECT_F rcClient{ 0,0,m_cxClientLog,m_cyClientLog };
            if (m_bSizeChanged || m_bUserDpiChanged)
            {
                constexpr auto D2dBmpOpt = D2D1_BITMAP_OPTIONS_TARGET |
                    D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
                if (m_bUserDpiChanged)
                {
                    CacheClear();
                    GetDeviceContext()->SetDpi((float)m_iUserDpi, (float)m_iUserDpi);
                    m_bUserDpiChanged = FALSE;
                    m_bFullUpdate = TRUE;
                    BroadcastEvent(WM_DPICHANGED, m_iUserDpi, 0);
                    if (!m_bSizeChanged)
                        goto SkipReSize;
                }
                m_bSizeChanged = FALSE;
                switch (m_ePresentMode)
                {
                case PresentMode::BitBltSwapChain:
                    m_D2D.ReSize(1, m_cxClient, m_cyClient, 0,
                        (D2D1_ALPHA_MODE)m_eAlphaMode, D2dBmpOpt, (float)m_iUserDpi);
                    break;
                case PresentMode::FlipSwapChain:
                    m_D2D.ReSize(2, m_cxClient, m_cyClient, DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT,
                        (D2D1_ALPHA_MODE)m_eAlphaMode, D2dBmpOpt, (float)m_iUserDpi);
                    break;
                case PresentMode::DCompositionSurface:
                case PresentMode::DCompositionVisual:
                {
                    IDCompositionSurface* pDcSurface = nullptr;
                    m_pDcDevice->CreateSurface(m_cxClient, m_cyClient,
                        DXGI_FORMAT_B8G8R8A8_UNORM,
                        m_bTransparent ? DXGI_ALPHA_MODE_PREMULTIPLIED : DXGI_ALPHA_MODE_IGNORE,
                        &pDcSurface);
                    m_pDcVisual->SetContent(pDcSurface);
                    if (m_pDcSurface)
                        m_pDcSurface->Release();
                    m_pDcSurface = pDcSurface;
                }
                break;
                case PresentMode::WindowRenderTarget:
                    m_pRtHwnd->Resize(D2D1::SizeU(m_cxClient, m_cyClient));
                    break;
                case PresentMode::UpdateLayeredWindow:
                {
                    SafeRelease(m_D2D.m_pBitmap);
                    const D2D1_BITMAP_PROPERTIES1 BmpProp
                    {
                        { DXGI_FORMAT_B8G8R8A8_UNORM,(D2D1_ALPHA_MODE)m_eAlphaMode },
                        (float)m_iUserDpi,(float)m_iUserDpi,
                        D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW |
                        D2D1_BITMAP_OPTIONS_GDI_COMPATIBLE,
                    };
                    GetDeviceContext()->CreateBitmap(D2D1::SizeU(m_cxClient, m_cyClient),
                        nullptr, 0, BmpProp, &m_D2D.m_pBitmap);
                    GetDeviceContext()->SetTarget(m_D2D.m_pBitmap);
                }
                break;
                default:
                    ECK_UNREACHABLE;
                }
            SkipReSize:
                if (m_cxClient && m_cyClient) [[likely]]
                {
                    m_rcInvalid = rcClient;
                    RdRender(rcClient, TRUE);
                    m_cs.Leave();
                    switch (m_ePresentMode)
                    {
                    case PresentMode::FlipSwapChain:
                        bWaitSwapChain = TRUE;
                        [[fallthrough]];
                    case PresentMode::BitBltSwapChain:
                        m_D2D.GetSwapChain()->Present(0, 0);
                        break;
                    }
                }
                else
                {
                    bWaitSwapChain = FALSE;
                    m_rcInvalid = {};
                    m_cs.Leave();
                }
            }
            else [[likely]]
            {
                // 更新脏矩形
                if (!IsRectEmpty(m_rcInvalid)) [[likely]]
                {
                    D2D1_RECT_F rc;
                    IntersectRect(rc, m_rcInvalid, rcClient);
                    if (IsRectEmpty(rc))
                        goto NoRedraw;
                    const auto bWantPhyRect =
                        (m_ePresentMode == PresentMode::FlipSwapChain ||
                            m_ePresentMode == PresentMode::BitBltSwapChain);
                    RECT rcPhy;
                    RdRender(rc, m_bFullUpdate, bWantPhyRect ? &rcPhy : nullptr);
                    DXGI_PRESENT_PARAMETERS pp{};
                    if (m_bFullUpdate)
                        m_bFullUpdate = FALSE;
                    else if (bWantPhyRect)
                    {
                        pp.DirtyRectsCount = 1;
                        pp.pDirtyRects = &rcPhy;
                    }
                    m_rcInvalid = {};
                    m_cs.Leave();
                    // 呈现
                    switch (m_ePresentMode)
                    {
                    case PresentMode::FlipSwapChain:
                        bWaitSwapChain = TRUE;
                        [[fallthrough]];
                    case PresentMode::BitBltSwapChain:
                        m_D2D.GetSwapChain()->Present1(0, 0, &pp);
                        break;
                    }
                }
                else
                {
                NoRedraw:
                    m_cs.Leave();
                    bWaitSwapChain = FALSE;
                }
            }

            iDeltaTime = int(NtGetTickCount64() - ullCurrTime);
            ullTime = ullCurrTime;
            if (bActiveTimeLine)
            {
                if (iDeltaTime < MinTickInterval)// 延时
                {
                    Timer.SetDueTime(MinTickInterval - iDeltaTime);
                    WaitObject(Timer);
                }
            }
            else
            {
                WaitObject(m_EvtRender);
                ullTime = NtGetTickCount64() - MinTickInterval;
            }
        }

        Timer.Cancel();
    }
    void RdStartup() noexcept
    {
        m_hthRender = CrtCreateThread([](void* p)->UINT
            {
                ((CDuiWnd*)p)->RdThread();
                return 0;
            }, this);
    }

    void RduInitGraphics() noexcept
    {
        switch (m_ePresentMode)
        {
        case PresentMode::BitBltSwapChain:
        {
            auto Param = EZD2D_PARAM::MakeBitblt(HWnd, g_pDxgiFactory, g_pDxgiDevice,
                g_pD2DDevice, m_cxClient, m_cyClient, (float)m_iUserDpi);
            Param.uBmpAlphaMode = (D2D1_ALPHA_MODE)m_eAlphaMode;
            m_D2D.Create(Param);
        }
        break;
        case PresentMode::FlipSwapChain:
        {
            auto Param = EZD2D_PARAM::MakeFlip(HWnd, g_pDxgiFactory, g_pDxgiDevice,
                g_pD2DDevice, m_cxClient, m_cyClient, (float)m_iUserDpi);
            Param.uBmpAlphaMode = (D2D1_ALPHA_MODE)m_eAlphaMode;
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
            m_pDcDevice->CreateSurface(m_cxClient, m_cyClient,
                DXGI_FORMAT_B8G8R8A8_UNORM,
                (DXGI_ALPHA_MODE)m_eDxgiAlphaMode,
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
            RtProp.pixelFormat = { DXGI_FORMAT_B8G8R8A8_UNORM,(D2D1_ALPHA_MODE)m_eAlphaMode };
            RtProp.dpiX = RtProp.dpiY = (float)m_iUserDpi;
            RtProp.usage = D2D1_RENDER_TARGET_USAGE_NONE;
            RtProp.minLevel = D2D1_FEATURE_LEVEL_DEFAULT;
            D2D1_HWND_RENDER_TARGET_PROPERTIES HwRtProp;
            HwRtProp.hwnd = HWnd;
            HwRtProp.pixelSize = D2D1::SizeU(m_cxClient, m_cyClient);
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

            m_pDcDevice->CreateSurface(m_cxClient, m_cyClient,
                DXGI_FORMAT_B8G8R8A8_UNORM,
                (DXGI_ALPHA_MODE)m_eDxgiAlphaMode,
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
                { DXGI_FORMAT_B8G8R8A8_UNORM,(D2D1_ALPHA_MODE)m_eAlphaMode },
                (float)m_iUserDpi,(float)m_iUserDpi,
                D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW |
                D2D1_BITMAP_OPTIONS_GDI_COMPATIBLE,
            };
            GetDeviceContext()->CreateBitmap(D2D1::SizeU(m_cxClient, m_cyClient),
                nullptr, 0, BmpProp, &m_D2D.m_pBitmap);
            GetDeviceContext()->SetTarget(m_D2D.m_pBitmap);
            GetDeviceContext()->QueryInterface(&m_pGdiInterop);
        }
        break;
        default:
            ECK_UNREACHABLE;
        }
    }
public:
    ECK_CWND_CREATE;
    // 一般不覆写此方法
    HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
        int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = nullptr) noexcept override
    {
        if (m_ePresentMode == PresentMode::FlipSwapChain ||
            m_ePresentMode == PresentMode::DCompositionSurface ||
            m_ePresentMode == PresentMode::DCompositionVisual)
            dwExStyle |= WS_EX_NOREDIRECTIONBITMAP;
        else if (m_ePresentMode == PresentMode::UpdateLayeredWindow)
            dwExStyle |= WS_EX_LAYERED;
        return IntCreate(dwExStyle, WCN_DUIHOST, pszText, dwStyle,
            x, y, cx, cy, hParent, hMenu, g_hInstance, nullptr);
    }

    LRESULT OnMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept override
    {
        if ((uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST))
        {
            POINT pt ECK_GET_PT_LPARAM(lParam);
            Phy2Log(pt);
            if (m_bMouseCaptured)
                m_pCurrNcHitTestElem = ElemFromPoint(pt);
            const auto pElem = (m_pMouseCaptureElem ? m_pMouseCaptureElem : m_pCurrNcHitTestElem);
            if (pElem)
            {
                pElem->ClientToElem(pt);
                if (pElem->CompIsNeedCoordinateTransform())
                {
                    POINT pt0{ pt };
                    pElem->CompTransformCoordinate(pt0, TRUE);
                    pElem->CallEvent(uMsg, wParam, MAKELPARAM(pt0.x, pt0.y));
                }
                else
                    pElem->CallEvent(uMsg, wParam, MAKELPARAM(pt.x, pt.y));
            }
            if (uMsg == WM_MOUSEMOVE)// 移出监听
            {
                if (m_pHoverElem != m_pCurrNcHitTestElem && !m_bMouseCaptured)
                {
                    if (m_pHoverElem)
                        m_pHoverElem->CallEvent(WM_MOUSELEAVE, 0, 0);
                    m_pHoverElem = m_pCurrNcHitTestElem;
                }
                TRACKMOUSEEVENT tme;
                tme.cbSize = sizeof(tme);
                tme.dwFlags = TME_LEAVE;
                tme.hwndTrack = hWnd;
                TrackMouseEvent(&tme);
            }
            return 0;
        }
        else if (uMsg >= WM_NCMOUSEMOVE && uMsg <= WM_NCXBUTTONDBLCLK)
        {
            if (m_bMouseCaptured)
            {
                POINT pt ECK_GET_PT_LPARAM(lParam);
                ScreenToClient(hWnd, &pt);
                m_pCurrNcHitTestElem = ElemFromPoint(pt);
            }
            auto pElem = (m_pMouseCaptureElem ? m_pMouseCaptureElem : m_pCurrNcHitTestElem);
            if (pElem)
                pElem->CallEvent(uMsg, wParam, lParam);

            if (uMsg == WM_NCMOUSEMOVE)// 移出监听
            {
                if (m_pHoverElem != m_pCurrNcHitTestElem && !m_bMouseCaptured)
                {
                    if (m_pHoverElem)
                        m_pHoverElem->CallEvent(WM_MOUSELEAVE, 0, 0);
                    m_pHoverElem = m_pCurrNcHitTestElem;
                }
                TRACKMOUSEEVENT tme;
                tme.cbSize = sizeof(tme);
                tme.dwFlags = TME_LEAVE | TME_NONCLIENT;
                tme.hwndTrack = hWnd;
                TrackMouseEvent(&tme);
            }
            else if (uMsg == WM_NCLBUTTONDOWN)// 修正放开事件
            {
                if (pElem)
                {
                    const auto lResult = CWindow::OnMessage(hWnd, uMsg, wParam, lParam);
                    POINT pt;
                    GetCursorPos(&pt);
                    pElem->CallEvent(WM_NCLBUTTONUP, wParam, MAKELPARAM(pt.x, pt.y));
                    return lResult;
                }
            }
        }
        else if (uMsg >= WM_KEYFIRST && uMsg <= WM_IME_KEYLAST)
        {
            if (m_pFocusElem)
                m_pFocusElem->CallEvent(uMsg, wParam, lParam);
            return 0;
        }

        switch (uMsg)
        {
        case WM_NCHITTEST:
        {
            POINT pt ECK_GET_PT_LPARAM(lParam);
            ScreenToClient(hWnd, &pt);
            Phy2Log(pt);
            LRESULT lResult;
            if (m_pCurrNcHitTestElem = ElemFromPoint(pt, &lResult))
                return lResult;
        }
        break;

        case WM_PAINT:
        {
            if (m_ePresentMode == PresentMode::BitBltSwapChain ||
                m_ePresentMode == PresentMode::WindowRenderTarget)
            {
                RECT rcInvalid;
                GetUpdateRect(hWnd, &rcInvalid, FALSE);
                ValidateRect(hWnd, nullptr);
                D2D1_RECT_F rcF{ MakeD2DRectF(rcInvalid) };
                Phy2Log(rcF);
                IrUnion(rcF);
                WakeRenderThread();
            }
            else
                ValidateRect(hWnd, nullptr);
        }
        return 0;

        case WM_SIZE:
        {
            ECK_DUILOCKWND;
            ECK_GET_SIZE_LPARAM(m_cxClient, m_cyClient, lParam);
            m_cxClientLog = Phy2LogF((float)m_cxClient);
            m_cyClientLog = Phy2LogF((float)m_cyClient);
            m_bSizeChanged = TRUE;
            WakeRenderThread();
        }
        break;

        case WM_SETCURSOR:
        {
            const auto pElem = (m_pMouseCaptureElem ? m_pMouseCaptureElem : m_pCurrNcHitTestElem);
            if (pElem && pElem->CallEvent(uMsg, wParam, lParam))
                return TRUE;
        }
        break;

        case WM_TIMER:
        {
            for (const auto e : m_vTimer)
            {
                if (e.uId == wParam)
                {
                    e.pElem->CallEvent(WM_TIMER, wParam, 0);
                    return 0;
                }
            }
        }
        return 0;

        case WM_NCMOUSELEAVE:
        case WM_MOUSELEAVE:
            if (m_pHoverElem)
            {
                m_pHoverElem->CallEvent(WM_MOUSELEAVE, 0, 0);
                m_pHoverElem = nullptr;
            }
            break;

        case WM_CAPTURECHANGED:
        {
            if (m_bMouseCaptured)
            {
                m_bMouseCaptured = FALSE;
                if (m_pMouseCaptureElem)
                {
                    m_pMouseCaptureElem->CallEvent(WM_CAPTURECHANGED, 0, NULL);
                    if (m_pHoverElem && m_pHoverElem != m_pMouseCaptureElem)
                    {
                        m_pHoverElem->CallEvent(WM_MOUSELEAVE, 0, 0);
                        m_pHoverElem = nullptr;
                    }
                    m_pMouseCaptureElem = nullptr;
                }
            }
        }
        break;

        case WM_GETOBJECT:
        {

        }
        return 0;

        case WM_COMMAND:// 应用程序可能需要使用菜单
            BroadcastEvent(uMsg, wParam, lParam, TRUE);
            return 0;

        case WM_SETTINGCHANGE:
        {
            if (IsColorSchemeChangeMessage(lParam))
            {
                ECK_DUILOCKWND;
                BroadcastEvent(EWM_COLORSCHEMECHANGED, ShouldAppsUseDarkMode(), 0);
            }
        }
        break;

        case WM_CREATE:
        {
            auto lResult = CWindow::OnMessage(hWnd, uMsg, wParam, lParam);
            if (!lResult)
            {
                m_iDpi = GetDpi(hWnd);
                RECT rc;
                GetClientRect(hWnd, &rc);
                rc.right = std::max(rc.right, 8L);
                rc.bottom = std::max(rc.bottom, 8L);
                m_cxClient = rc.right;
                m_cyClient = rc.bottom;
                m_cxClientLog = Phy2LogF((float)m_cxClient);
                m_cyClientLog = Phy2LogF((float)m_cyClient);

                RduInitGraphics();

                const auto pDC = GetDeviceContext();

                m_vTheme.resize(2);
                StMakeTheme(m_vTheme[0].RefOf(), FALSE);
                m_vTheme[0]->RealizeForDC(pDC);
                StMakeTheme(m_vTheme[1].RefOf(), TRUE);
                m_vTheme[1]->RealizeForDC(pDC);

                pDC->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
                if (m_ePresentMode != PresentMode::WindowRenderTarget)
                    pDC->SetDpi((float)m_iUserDpi, (float)m_iUserDpi);

                if (m_bTransparent)
                    pDC->CreateSolidColorBrush({}, &m_pBrBkg);
                else
                    pDC->CreateSolidColorBrush(
                        ColorrefToD2DColorF(PtcCurrent()->crDefBkg), &m_pBrBkg);

                m_bFullUpdate = TRUE;
                m_rcInvalid = { 0,0,m_cxClientLog,m_cyClientLog };

                m_EvtRender.NoSignal();
                RdStartup();
            }
            return lResult;
        }
        break;

        case WM_DPICHANGED:// For top-level window.
            m_iDpi = HIWORD(wParam);
            MsgOnDpiChanged(hWnd, lParam);
            return 0;

        case WM_DPICHANGED_AFTERPARENT:// For child window.
            m_iDpi = GetDpi(hWnd);
            return 0;

        case WM_DESTROY:
        {
            // 终止渲染线程
            m_cs.Enter();
            m_bExit = TRUE;
            WakeRenderThread();
            m_cs.Leave();
            // 等待渲染线程退出
            NtWaitForSingleObject(m_hthRender, FALSE, nullptr);
            m_hthRender = nullptr;
            // 销毁所有元素
            auto pElem = m_pFirstChild;
            while (pElem)
            {
                auto pNext = pElem->GetNextElem();
                pElem->Destroy();
                pElem = pNext;
            }
            m_pFirstChild = m_pLastChild = nullptr;
            m_pFocusElem = m_pCurrNcHitTestElem = nullptr;

            m_D2D.Destroy();
            SafeReleaseAssert0(m_pBmpCache);
            SafeReleaseAssert0(m_pBrBkg);
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
            SafeReleaseAssert0(m_pFxBlur);
            SafeReleaseAssert0(m_pFxCrop);
            // 销毁其他接口
            m_pDataObj = nullptr;

            for (const auto p : m_vTimeLine)
                p->Release();
            m_vTimeLine.clear();

            m_cxClient = m_cyClient = 0;
        }
        break;
        }

        return CWindow::OnMessage(hWnd, uMsg, wParam, lParam);
    }

    virtual LRESULT OnElemEvent(CElem* pElem, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept { return 0; }

    virtual LRESULT OnRenderEvent(UINT uMsg, RENDER_EVENT& e) noexcept
    {
        if (uMsg == RE_FILLBACKGROUND)
        {
            if (m_pBrBkg)
                GetDeviceContext()->FillRectangle(e.FillBkg.rc, m_pBrBkg);
        }
        return RER_NONE;
    }

    EckInlineNdCe auto BbrGet() const noexcept { return m_pBrBkg; }
    EckInline void BbrDelete() noexcept { SafeRelease(m_pBrBkg); }
    EckInline void BbrCreate() noexcept
    {
        if (!m_pBrBkg)
            GetDeviceContext()->CreateSolidColorBrush(
                ColorrefToD2DColorF(PtcCurrent()->crDefBkg), &m_pBrBkg);
    }

    EckInline CElem* ElemFromPoint(POINT pt, _Out_opt_ LRESULT* pResult = nullptr) noexcept
    {
        return CElem::ElemFromPoint(GetLastChildElem(), pt, pResult);
    }

    CElem* ElemSetFocus(CElem* pElem) noexcept
    {
        SetFocus(HWnd);
        if (m_pFocusElem == pElem)
            return pElem;
        auto pOld = m_pFocusElem;
        if (pOld)
            pOld->CallEvent(WM_KILLFOCUS, (WPARAM)pElem, 0);
        m_pFocusElem = pElem;
        pElem->CallEvent(WM_SETFOCUS, (WPARAM)pOld, 0);
        return pOld;
    }
    EckInline constexpr CElem* ElemGetFocus() const noexcept { return m_pFocusElem; }

    CElem* ElemSetCapture(CElem* pElem) noexcept
    {
        auto pOld = m_pMouseCaptureElem;
        m_pMouseCaptureElem = pElem;
        if (GetCapture() != m_hWnd)
        {
            SetCapture(m_hWnd);
            m_bMouseCaptured = TRUE;
        }
        if (pOld)
            pOld->CallEvent(WM_CAPTURECHANGED, 0, (LPARAM)pElem);
        return pOld;
    }
    EckInline void ElemReleaseCapture() noexcept
    {
        ReleaseCapture();

        // WM_CAPTURECHANGED will process it:
        // m_pMouseCaptureElem->CallEvent(WM_CAPTURECHANGED, 0, nullptr);
        // m_pMouseCaptureElem = nullptr;
    }
    EckInline constexpr CElem* ElemGetCapture() const noexcept { return m_pMouseCaptureElem; }

    BOOL ElemSetTimer(CElem* pElem, UINT_PTR uId, UINT uElapse) noexcept
    {
        if (!SetTimer(HWnd, uId, uElapse, nullptr))
            return FALSE;
        for (auto& e : m_vTimer)
        {
            if (e.pElem == pElem && e.uId == uId)
                return TRUE;
        }
        m_vTimer.emplace_back(pElem, uId);
        return TRUE;
    }
    BOOL ElemKillTimer(CElem* pElem, UINT_PTR uId) noexcept
    {
        for (auto it = m_vTimer.begin(); it != m_vTimer.end(); ++it)
        {
            if (it->pElem == pElem && it->uId == uId)
            {
                KillTimer(HWnd, uId);
                m_vTimer.erase(it);
                return TRUE;
            }
        }
        return FALSE;
    }

    EckInlineNdCe CElem* GetFirstChildElem() const noexcept { return m_pFirstChild; }
    EckInlineNdCe CElem* GetLastChildElem() const noexcept { return m_pLastChild; }

    EckInlineNdCe int GetDpiValue() const noexcept { return m_iDpi; }
    EckInlineNdCe int GetUserDpi() const noexcept { return m_iUserDpi; }
    void SetUserDpi(int iDpi) noexcept
    {
        ECK_DUILOCKWND;
        m_iUserDpi = iDpi;
        if (IsValid())
        {
            m_bUserDpiChanged = TRUE;
            WakeRenderThread();
        }
    }

    EckInlineNdCe int Phy2Log(int i) const noexcept { return i * 96 / m_iUserDpi; }
    EckInlineNdCe float Phy2LogF(float i) const noexcept { return i * 96.f / m_iUserDpi; }
    EckInlineNdCe int Log2Phy(int i) const noexcept { return i * m_iUserDpi / 96; }
    EckInlineNdCe float Log2PhyF(float i) const noexcept { return i * m_iUserDpi / 96.f; }

    EckInlineCe void Phy2Log(_Inout_ POINT& pt) const noexcept
    {
        pt.x = Phy2Log(pt.x);
        pt.y = Phy2Log(pt.y);
    }
    EckInlineCe void Phy2Log(_Inout_ RECT& rc) const noexcept
    {
        rc.left = Phy2Log(rc.left);
        rc.top = Phy2Log(rc.top);
        rc.right = Phy2Log(rc.right);
        rc.bottom = Phy2Log(rc.bottom);
    }
    EckInlineCe void Phy2Log(_Inout_ D2D1_POINT_2F& pt) const noexcept
    {
        pt.x = Phy2LogF(pt.x);
        pt.y = Phy2LogF(pt.y);
    }
    EckInlineCe void Phy2Log(_Inout_ D2D1_RECT_F& rc) const noexcept
    {
        rc.left = Phy2LogF(rc.left);
        rc.top = Phy2LogF(rc.top);
        rc.right = Phy2LogF(rc.right);
        rc.bottom = Phy2LogF(rc.bottom);
    }
    EckInlineCe void Log2Phy(_Inout_ POINT& pt) const noexcept
    {
        pt.x = Log2Phy(pt.x);
        pt.y = Log2Phy(pt.y);
    }
    EckInlineCe void Log2Phy(_Inout_ RECT& rc) const noexcept
    {
        rc.left = Log2Phy(rc.left);
        rc.top = Log2Phy(rc.top);
        rc.right = Log2Phy(rc.right);
        rc.bottom = Log2Phy(rc.bottom);
    }
    EckInlineCe void Log2Phy(_Inout_ D2D1_POINT_2F& pt) const noexcept
    {
        pt.x = Log2PhyF(pt.x);
        pt.y = Log2PhyF(pt.y);
    }
    EckInlineCe void Log2Phy(_Inout_ D2D1_RECT_F& rc) const noexcept
    {
        rc.left = Log2PhyF(rc.left);
        rc.top = Log2PhyF(rc.top);
        rc.right = Log2PhyF(rc.right);
        rc.bottom = Log2PhyF(rc.bottom);
    }

    EckInline void StGetCurrentTheme(ITheme*& pTheme) noexcept
    {
        ECK_DUILOCKWND;
        pTheme = m_vTheme[!!m_bDarkMode].Get();
        pTheme->AddRef();
    }
    EckInline void StSwitchStdThemeMode(BOOL bDark) noexcept
    {
        ECK_DUILOCKWND;
        m_bDarkMode = bDark;
        const auto pTheme = m_vTheme[!!m_bDarkMode].Get();
        for (size_t i = 2; i < m_vTheme.size(); ++i)
            m_vTheme[i]->SetParent(pTheme);
        D2D1_COLOR_F cr;
        pTheme->GetSysColor(SysColor::Bk, cr);
        if (m_pBrBkg)
            m_pBrBkg->SetColor(cr);
        EtForEachElem([&](CElem* pElem)
            {
                pElem->SetTheme(pTheme);
            }, GetFirstChildElem());
    }
    void StUpdateColorizationColor(const D2D1_COLOR_F& cr) noexcept
    {
        ECK_DUILOCKWND;
        for (auto& pTheme : m_vTheme)
            pTheme->SetColorizationColor(cr);
    }
    BOOL StUpdateColorizationColor() noexcept
    {
        HKEY hKey;
        if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\DWM",
            0, KEY_READ, &hKey) != ERROR_SUCCESS)
            return FALSE;
        DWORD dw, cbBuf{ sizeof(DWORD) };
        if (RegQueryValueExW(hKey, L"ColorizationColor", nullptr, nullptr,
            (BYTE*)&dw, &cbBuf) != ERROR_SUCCESS)
        {
            RegCloseKey(hKey);
            return FALSE;
        }
        if (((dw >> 24) & 0xFF) < 0xC4)
        {
            dw &= 0xFFFFFF;
            dw |= 0xC4000000;
        }
        StUpdateColorizationColor(ArgbToD2DColorF(dw));
        RegCloseKey(hKey);
        return TRUE;
    }
    void StRegisterAutoTheme(ITheme* pTheme) noexcept
    {
        ECK_DUILOCKWND;
        EckAssert(std::find_if(m_vTheme.begin(), m_vTheme.end(),
            [=](const auto& p) { return p.Get() == pTheme; }) == m_vTheme.end());
        m_vTheme.emplace_back(pTheme);
        ComPtr<ITheme> pParent;
        StGetCurrentTheme(pParent.RefOf());
        pTheme->SetParent(pParent.Get());
        pTheme->RealizeForDC(GetDeviceContext());
    }

    EckInline void BroadcastEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept
    {
        auto pElem = GetFirstChildElem();
        while (pElem)
        {
            pElem->BroadcastEvent(uMsg, wParam, lParam);
            pElem = pElem->GetNextElem();
        }
    }

    EckInline void BroadcastEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT lStop) noexcept
    {
        auto pElem = GetFirstChildElem();
        while (pElem)
        {
            pElem->BroadcastEvent(uMsg, wParam, lParam, lStop);
            pElem = pElem->GetNextElem();
        }
    }

    EckInlineNdCe CCriticalSection& GetCriticalSection() noexcept { return m_cs; }

    EckInline void WakeRenderThread() noexcept
    {
        m_EvtRender.Signal();
    }

    EckInline void RegisterTimeLine(ITimeLine* pTl) noexcept
    {
        ECK_DUILOCKWND;
        pTl->AddRef();
        m_vTimeLine.emplace_back(pTl);
    }
    EckInline void UnregisterTimeLine(ITimeLine* pTl) noexcept
    {
        ECK_DUILOCKWND;
        const auto it = std::find(m_vTimeLine.begin(), m_vTimeLine.end(), pTl);
        if (it != m_vTimeLine.end())
        {
            (*it)->Release();
            m_vTimeLine.erase(it);
        }
    }

    EckInline void Redraw(BOOL bWake = TRUE) noexcept
    {
        ECK_DUILOCKWND;
        m_bFullUpdate = TRUE;
        m_rcInvalid = { 0,0,m_cxClientLog,m_cyClientLog };
        if (bWake)
            WakeRenderThread();
    }

    // 必须在创建窗口之前调用
    EckInline void SetPresentMode(PresentMode ePresentMode) noexcept
    {
        EckAssert(!IsValid());
        m_ePresentMode = ePresentMode;
    }
    EckInlineNdCe PresentMode GetPresentMode() const noexcept { return m_ePresentMode; }

    // 必须在创建窗口之前调用。
    // 设置透明后不会生成默认背景画刷，渲染开始前总是清除图面，并且在呈现时不忽略Alpha通道
    EckInlineCe void SetTransparent(BOOL bTransparent) noexcept
    {
        m_bTransparent = bTransparent;
        m_eAlphaMode = (bTransparent ? D2D1_ALPHA_MODE_PREMULTIPLIED : D2D1_ALPHA_MODE_IGNORE);
        m_eDxgiAlphaMode = (bTransparent ? DXGI_ALPHA_MODE_PREMULTIPLIED : DXGI_ALPHA_MODE_IGNORE);
    }
    EckInlineNdCe BOOL GetTransparent() const noexcept { return m_bTransparent; }

    EckInlineNdCe int GetChildrenCount() const noexcept { return m_cChildren; }

    EckInlineNdCe ID2D1Bitmap1* GetCacheBitmap() const noexcept { return m_pBmpCache; }

    void CacheReserve(int cxPhy, int cyPhy) noexcept
    {
        if (cxPhy > m_cxCache || cyPhy > m_cyCache)
        {
            if (m_pBmpCache)
                m_pBmpCache->Release();
            m_cxCache = cxPhy;
            m_cyCache = cyPhy;
            BmpNew(m_cxCache, m_cyCache, m_pBmpCache);
        }
    }
    void CacheReserveLogSize(float cx, float cy) noexcept
    {
        CacheReserve((int)ceilf(Log2PhyF(cx)), (int)ceilf(Log2PhyF(cy)));
    }
    void CacheClear() noexcept
    {
        if (m_pBmpCache)
            m_pBmpCache->Release();
        m_pBmpCache = nullptr;
        m_cxCache = m_cyCache = 0;
    }

    void BlurInit() noexcept
    {
        ECK_DUILOCKWND;
        if (!m_pFxBlur)
        {
            GetDeviceContext()->CreateEffect(CLSID_D2D1GaussianBlur, &m_pFxBlur);
            m_pFxBlur->SetValue(D2D1_GAUSSIANBLUR_PROP_BORDER_MODE,
                D2D1_BORDER_MODE_HARD);
        }
        if (!m_pFxCrop)
            GetDeviceContext()->CreateEffect(CLSID_D2D1Crop, &m_pFxCrop);
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
                pDC->PushLayer(m_CustomLayer.pParam, m_CustomLayer.pLayer);
            else
                pDC->PushLayer(LyParam, nullptr);
        pDC->DrawImage(m_pFxBlur, ptDrawing);
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
    /// <param name="rc">范围，相对当前位图。如果重画时有任何偏移量，必须手动与之相加</param>
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
        if (FAILED(hr = GetCacheBitmap()->CopyFromBitmap(nullptr, pBmp.Get(), &rcU)))
            return hr;

        m_pFxBlur->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, fDeviation);
        m_pFxCrop->SetValue(D2D1_CROP_PROP_RECT,
            D2D1::RectF(0.f, 0.f, rc.right - rc.left, rc.bottom - rc.top));

        m_pFxCrop->SetInput(0, GetCacheBitmap());
        m_pFxBlur->SetInputEffect(0, m_pFxCrop);
        return BlurpDrawEffect(m_pFxBlur, ptDrawing, bUseLayer);
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
    HRESULT BlurDrawDirect(ID2D1Bitmap1* pBmp, const D2D1_RECT_F& rc,
        D2D1_POINT_2F ptDrawing, float fDeviation, BOOL bUseLayer = FALSE) noexcept
    {
        m_pFxBlur->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, fDeviation);
        m_pFxCrop->SetValue(D2D1_CROP_PROP_RECT,
            D2D1::RectF(0.f, 0.f, rc.right - rc.left, rc.bottom - rc.top));

        m_pFxCrop->SetInput(0, pBmp);
        m_pFxBlur->SetInputEffect(0, m_pFxCrop);
        return BlurpDrawEffect(m_pFxBlur, ptDrawing, bUseLayer);
    }

    // 模糊指定位图的内容，并画出。
    // 忽略裁剪效果
    HRESULT BlurDrawDirect(ID2D1Bitmap1* pBmp,
        D2D1_POINT_2F ptDrawing, float fDeviation, BOOL bUseLayer = FALSE) noexcept
    {
        m_pFxBlur->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, fDeviation);
        m_pFxBlur->SetInput(0, pBmp);
        return BlurpDrawEffect(m_pFxBlur, ptDrawing, bUseLayer);
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

    HRESULT BmpNew(int cxPhy, int cyPhy, _Out_ ID2D1Bitmap1*& pBmp) noexcept
    {
        const D2D1_BITMAP_PROPERTIES1 Prop
        {
            { DXGI_FORMAT_B8G8R8A8_UNORM,D2D1_ALPHA_MODE_PREMULTIPLIED },
            (float)m_iUserDpi,
            (float)m_iUserDpi,
            D2D1_BITMAP_OPTIONS_TARGET
        };
        return GetDeviceContext()->CreateBitmap(D2D1::SizeU(cxPhy, cyPhy),
            nullptr, 0, Prop, &pBmp);
    }
    HRESULT BmpNewLogSize(float cx, float cy, _Out_ ID2D1Bitmap1*& pBmp) noexcept
    {
        return BmpNew((int)ceilf(Log2PhyF(cx)), (int)ceilf(Log2PhyF(cy)), pBmp);
    }

    EckInlineNdCe int GetClientWidth() const noexcept { return m_cxClient; }
    EckInlineNdCe int GetClientHeight() const noexcept { return m_cyClient; }
    EckInlineNdCe float GetClientWidthLog() const noexcept { return m_cxClientLog; }
    EckInlineNdCe float GetClientHeightLog() const noexcept { return m_cyClientLog; }

    /// <summary>
    /// 初始化目标
    /// 使用DCompositionVisual呈现时，初始化渲染到的目标视觉对象
    /// </summary>
    /// <param name="pVisual">目标视觉对象，窗口内容渲染到此对象</param>
    /// <param name="pDevice">DComp设备</param>
    void DcvInit(IDCompositionVisual* pVisual, IDCompositionDevice* pDevice) noexcept
    {
        EckAssert(m_ePresentMode == PresentMode::DCompositionVisual);
        EckAssert(!m_pDcVisual && !m_pDcSurface && !m_pDcDevice);
        m_pDcVisual = pVisual;
        m_pDcVisual->AddRef();
        m_pDcDevice = pDevice;
        m_pDcDevice->AddRef();
    }

    EckInlineNdCe ID2D1DeviceContext* GetDeviceContext() const noexcept { return m_D2D.GetDC(); }

    void EtForEachElem(const std::invocable<CElem*> auto& Fn, CElem* pElemBegin) noexcept
    {
        auto p{ pElemBegin };
        while (p)
        {
            EckCanCallbackContinue(Fn(p))
                return;
            EtForEachElem(Fn, p->GetFirstChildElem());
            p = p->GetNextElem();
        }
    }

    EckInlineCe void SetDrawDirtyRect(BOOL b) noexcept
    {
#ifdef _DEBUG
        m_bDrawDirtyRect = b;
#endif// _DEBUG
    }

    EckInlineNdCe CElem* GetCurrNcHitElem() const noexcept { return m_pCurrNcHitTestElem; }

    void IrUnion(const D2D1_RECT_F& rc)
    {
        EckAssert(!IsRectEmpty(rc));
        UnionRect(m_rcInvalid, m_rcInvalid, rc);
    }

    HRESULT EnableDragDrop(BOOL bEnable) noexcept
    {
        ECK_DUILOCKWND;
        if (!!bEnable == m_bEnableDragDrop)
            return S_FALSE;
        m_bEnableDragDrop = !!bEnable;
        if (bEnable)
            return RegisterDragDrop(HWnd, this);
        else
            return RevokeDragDrop(HWnd);
    }

    ULONG STDMETHODCALLTYPE AddRef() override { return 1; }
    ULONG STDMETHODCALLTYPE Release() override { return 1; }
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override
    {
        if (riid == IID_IUnknown || riid == IID_IDropTarget)
        {
            *ppvObject = static_cast<IDropTarget*>(this);
            return S_OK;
        }
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }
    HRESULT STDMETHODCALLTYPE DragEnter(IDataObject* pDataObj,
        DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
    {
        m_pDataObj = pDataObj;
        POINT pt0{ pt.x, pt.y };
        ScreenToClient(HWnd, &pt0);
        DRAGDROPINFO ddi{ pDataObj,grfKeyState,pt,pdwEffect };

        if (SendMsg(WM_DRAGENTER, (WPARAM)&ddi, MAKELPARAM(pt0.x, pt0.y)))
            return ddi.hr;

        const auto pElem = ElemFromPoint(pt0);
        EckAssert(!m_pDragDropElem);
        m_pDragDropElem = pElem;

        if (pElem)
            return (HRESULT)pElem->CallEvent(WM_DRAGENTER,
                (WPARAM)&ddi, MAKELPARAM(pt0.x, pt0.y));
        else
            return S_OK;
    }
    HRESULT STDMETHODCALLTYPE DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
    {
        POINT pt0{ pt.x, pt.y };
        ScreenToClient(HWnd, &pt0);
        DRAGDROPINFO ddi{ nullptr,grfKeyState,pt,pdwEffect };

        if (SendMsg(WM_DRAGOVER, (WPARAM)&ddi, MAKELPARAM(pt0.x, pt0.y)))
            return ddi.hr;

        auto pElem = ElemFromPoint(pt0);
        const auto pOldElem = m_pDragDropElem;
        m_pDragDropElem = pElem;

        if (pOldElem != pElem)
        {
            if (pOldElem)
                pOldElem->CallEvent(WM_DRAGLEAVE, 0, 0);
            if (pElem)
            {
                ddi.pDataObj = m_pDataObj.Get();
                return (HRESULT)pElem->CallEvent(WM_DRAGENTER,
                    (WPARAM)&ddi, MAKELPARAM(pt0.x, pt0.y));
            }
        }
        else if (pElem)
            return (HRESULT)pElem->CallEvent(WM_DRAGOVER,
                (WPARAM)&ddi, MAKELPARAM(pt0.x, pt0.y));
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE DragLeave(void)
    {
        if (SendMsg(WM_DRAGLEAVE, 0, 0))
        {
            EckAssert(!m_pDragDropElem);
            m_pDataObj = nullptr;
            return S_OK;
        }
        m_pDataObj = nullptr;

        const auto pElem = m_pDragDropElem;
        if (pElem)
        {
            m_pDragDropElem = nullptr;
            return (HRESULT)pElem->CallEvent(WM_DRAGLEAVE, 0, 0);
        }
        else
            return S_OK;
    }
    HRESULT STDMETHODCALLTYPE Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
    {
        POINT pt0{ pt.x, pt.y };
        ScreenToClient(HWnd, &pt0);
        DRAGDROPINFO ddi{ pDataObj, grfKeyState, pt, pdwEffect };

        if (SendMsg(WM_DROP, (WPARAM)&ddi, MAKELPARAM(pt0.x, pt0.y)))
        {
            EckAssert(!m_pDragDropElem);
            m_pDataObj = nullptr;
            return S_OK;
        }
        m_pDataObj = nullptr;

        auto pElem = ElemFromPoint(pt0);
        m_pDragDropElem = nullptr;
        if (pElem)
            return (HRESULT)pElem->CallEvent(WM_DROP,
                (WPARAM)&ddi, MAKELPARAM(pt0.x, pt0.y));
        else
            return S_OK;
    }
};

inline BOOL CElem::IntCreate(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
    float x, float y, float cx, float cy, CElem* pParent,
    CDuiWnd* pWnd, INT_PTR iId, PCVOID pData) noexcept
{
    EckAssert(!m_pWnd && !m_pDC);

    if (!pWnd)
        pWnd = pParent->GetWnd();
    m_iId = iId;
    m_pWnd = pWnd;
    m_pDC = pWnd->GetDeviceContext();
    m_pParent = pParent;
    GetWnd()->StGetCurrentTheme(m_pTheme);

    tcSetStyleWorker(dwStyle);

    ECK_DUILOCK;
#ifdef _DEBUG
    if (pParent)
        EckAssert(pParent->m_pWnd == pWnd);
#endif
    auto& pParentLastChild = (pParent ? pParent->m_pLastChild : pWnd->m_pLastChild);
    auto& pParentFirstChild = (pParent ? pParent->m_pFirstChild : pWnd->m_pFirstChild);
    if (m_pParent)
        ++m_pParent->m_cChildren;
    else
        ++m_pWnd->m_cChildren;

    if (pParentLastChild)
    {
        m_pPrev = pParentLastChild;
        m_pNext = nullptr;
        m_pPrev->m_pNext = this;
        pParentLastChild = this;
    }
    else
    {
        m_pPrev = m_pNext = nullptr;
        pParentFirstChild = this;
        pParentLastChild = this;
    }

    CallEvent(WM_NCCREATE, 0, (LPARAM)pData);

    m_rsText = pszText;
    tcSetRectWorker({ x,y,x + cx,y + cy });

    if (CallEvent(WM_CREATE, 0, (LPARAM)pData))
    {
        Destroy();
        return FALSE;
    }
    else
    {
        tcPostMoveSize(TRUE, TRUE, GetWholeRectInClient());
        return TRUE;
    }
}

inline void CElem::Destroy() noexcept
{
    ECK_DUILOCK;
    if (m_pParent)
        --m_pParent->m_cChildren;
    else
        --m_pWnd->m_cChildren;
    m_pWnd->ElemDestroying(this);
    CallEvent(WM_DESTROY, 0, 0);
    utcDestroyChild(this);
    if (GetCompositor())
    {
        CompInvalidateCacheBitmap();
        SafeRelease(m_pCompositor);
    }
    SafeRelease(m_pTextFormat);
    SafeRelease(m_pTheme);

    if (m_pPrev)
        m_pPrev->m_pNext = m_pNext;
    else
    {
        if (m_pParent)
            m_pParent->m_pFirstChild = m_pNext;
        else
            m_pWnd->m_pFirstChild = m_pNext;
    }

    if (m_pNext)
        m_pNext->m_pPrev = m_pPrev;
    else
    {
        if (m_pParent)
            m_pParent->m_pLastChild = m_pPrev;
        else
            m_pWnd->m_pLastChild = m_pPrev;
    }

    m_pNext = nullptr;
    m_pPrev = nullptr;
    m_pParent = nullptr;
    m_pFirstChild = nullptr;
    m_pLastChild = nullptr;
    m_pWnd = nullptr;
    m_pDC = nullptr;

    m_rc = {};
    m_ptOffsetInClient = {};
    m_rsText.Clear();
    m_dwStyle = 0;
}

inline LRESULT CElem::OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept
{
    switch (uMsg)
    {
    case WM_NCHITTEST:
        return HTCLIENT;
    case WM_PAINT:
        if (GetStyle() & DES_BASE_BEGIN_END_PAINT)
        {
            ELEMPAINTSTRU ps;
            BeginPaint(ps, wParam, lParam);
            EndPaint(ps);
        }
        return 0;
    case WM_ERASEBKGND:
        if ((GetCompositor() && !(GetStyle() & DES_COMP_NO_REDIRECTION)))
            m_pDC->Clear({});
        return TRUE;
    case WM_SIZE:
        if (GetCompositor())
        {
            CompReCalcCompositedRect();
            CompInvalidateCacheBitmap();
        }
        break;
    }
    return 0;
}

inline void CElem::SetZOrder(CElem* pElemAfter) noexcept
{
    ECK_DUILOCK;
    auto& pParentLastChild = (m_pParent ? m_pParent->m_pLastChild : m_pWnd->m_pLastChild);
    auto& pParentFirstChild = (m_pParent ? m_pParent->m_pFirstChild : m_pWnd->m_pFirstChild);

    if (pElemAfter == ECK_ELEMBOTTOM)
    {
        if (m_pPrev)
        {
            m_pPrev->m_pNext = m_pNext;
            if (m_pNext)
                m_pNext->m_pPrev = m_pPrev;
            else
                pParentLastChild = m_pPrev;

            m_pPrev = nullptr;
            m_pNext = pParentFirstChild;
            if (m_pNext)
                m_pNext->m_pPrev = this;
            pParentFirstChild = this;
        }
    }
    else if (pElemAfter == ECK_ELEMTOP)
    {
        if (m_pNext)
        {
            m_pNext->m_pPrev = m_pPrev;
            if (m_pPrev)
                m_pPrev->m_pNext = m_pNext;
            else
                pParentFirstChild = m_pNext;

            m_pNext = nullptr;
            m_pPrev = pParentLastChild;
            if (m_pPrev)
                m_pPrev->m_pNext = this;
            pParentLastChild = this;
        }
    }
    else
    {
        // pElemAfter一定不为nullptr
        if (m_pPrev)
            m_pPrev->m_pNext = m_pNext;
        else
            pParentFirstChild = m_pNext;

        if (m_pNext)
            m_pNext->m_pPrev = m_pPrev;
        else
            pParentLastChild = m_pPrev;

        m_pPrev = pElemAfter;
        m_pNext = pElemAfter->m_pNext;

        m_pPrev->m_pNext = this;
        if (m_pNext)
            m_pNext->m_pPrev = this;
        else
            pParentLastChild = this;
    }
}

EckInline LRESULT CElem::GenElemNotify(void* pnm) noexcept
{
    BOOL bProcessed{};
    const auto lResult = OnNotify((DUINMHDR*)pnm, bProcessed);
    if (bProcessed)
        return lResult;
    if (GetParentElem() && !(GetStyle() & DES_NOTIFY_TO_WND))
        return GetParentElem()->CallEvent(WM_NOTIFY, (WPARAM)this, (LPARAM)pnm);
    else
        return GetWnd()->OnElemEvent(this, ((DUINMHDR*)pnm)->uCode, 0, (LPARAM)pnm);
}

inline void CElem::tcIrpInvalidate(const D2D1_RECT_F& rcInClient, BOOL bUpdateNow) noexcept
{
    ECK_DUILOCK;
    if (!IsVisible())
        return;
    D2D1_RECT_F rcTemp;
    if (GetStyle() & DES_CONTENT_EXPAND)
        rcTemp = GetWholeRectInClient();
    else
    {
        rcTemp = rcInClient;
        IntersectRect(rcTemp, rcTemp, GetWholeRectInClient());// 裁剪到元素矩形
    }
    if (IsRectEmpty(rcTemp))
        return;
    if (!(GetStyle() & DES_COMP_NO_REDIRECTION) &&
        ((GetStyle() & DES_PARENT_COMP) || GetCompositor()))
    {
        auto pElem{ this };
        do
        {
            pElem->CompMarkDirty();
        } while (pElem = pElem->GetParentElem());
    }
    GetWnd()->CeUnion(rcTemp);
    GetWnd()->IrUnion(rcTemp);
    if (bUpdateNow)
        GetWnd()->WakeRenderThread();
}

inline void CElem::BeginPaint(_Out_ ELEMPAINTSTRU& eps,
    WPARAM wParam, LPARAM lParam, UINT uFlags) noexcept
{
    const auto pExtra = (Priv::PAINT_EXTRA*)wParam;
    eps.rcfClip = *(const D2D1_RECT_F*)lParam;
    eps.rcfClipInElem = eps.rcfClip;
    ClientToElem(eps.rcfClipInElem);
    eps.ox = pExtra->ox;
    eps.oy = pExtra->oy;
    m_pDC->PushAxisAlignedClip(eps.rcfClipInElem, D2D1_ANTIALIAS_MODE_ALIASED);
    if ((GetStyle() & DES_BLURBKG) && !GetCompositor())
        GetWnd()->BlurpDrawStyleBkg(this, eps.rcfClip, eps.ox, eps.oy);
    else if (!(uFlags & EBPF_DO_NOT_FILLBK))
        CallEvent(WM_ERASEBKGND, 0, (LPARAM)&eps);
}

EckInline void CElem::InitEasingCurve(CEasingCurve* pEc) noexcept
{
    pEc->SetCallbackData((LPARAM)this);
    GetWnd()->RegisterTimeLine(pEc);
}

inline void CElem::tcPostMoveSize(BOOL bSize, BOOL bMove, const D2D1_RECT_F& rcOld) noexcept
{
    if (bSize)
    {
        if (GetCompositor() && m_pCompBitmap)
        {
            if (GetStyle() & DES_OWNER_COMP_CACHE)
            {
                const auto& rcValid = m_pCompCacheSurface->GetValidRect();
                if (rcValid.right - rcValid.left < GetWidthF() ||
                    rcValid.bottom - rcValid.top < GetHeightF())
                    CompInvalidateCacheBitmap();
            }
            else
            {
                const auto size = m_pCompBitmap->GetSize();
                if (size.width < GetWidthF() || size.height < GetHeightF())
                    CompInvalidateCacheBitmap();
            }
        }
        CallEvent(WM_SIZE, 0, 0);
    }
    if (bMove)
        CallEvent(WM_MOVE, 0, 0);
    D2D1_RECT_F rc;
    UnionRect(rc, rcOld, GetWholeRectInClient());
    if (!IsRectEmpty(rc))
    {
        InvalidateRect(rc);
        GetWnd()->IrUnion(rc);
    }
}

inline void CElem::tcIrpUnionContentExpandElemRect(
    CElem* pElem, _Inout_ D2D1_RECT_F& rcInClient) noexcept
{
    while (pElem)
    {
        if (pElem->GetStyle() & DES_CONTENT_EXPAND)
            UnionRect(rcInClient, rcInClient, pElem->GetWholeRectInClient());
        else if (pElem->GetStyle() & DES_CONTENT_EXPAND_RECT)
        {
            D2D1_RECT_F rcExpand{};
            pElem->CallEvent(EWM_QUERY_EXPAND_RECT, (WPARAM)&rcExpand, 0);
            UnionRect(rcInClient, rcInClient, rcExpand);
        }
        tcIrpUnionContentExpandElemRect(pElem->GetLastChildElem(), rcInClient);
        pElem = pElem->GetNextElem();
    }
}

inline constexpr void CElem::tcSetStyleWorker(DWORD dwStyle) noexcept
{
    if (dwStyle & DES_BLURBKG)
        dwStyle |= DES_CONTENT_EXPAND;
    const auto dwOld = m_dwStyle;
    m_dwStyle = dwStyle;
    // 检查DES_CONTENT_EXPAND(_RECT)变动
    const auto bExpandChanged = ((dwOld ^ dwStyle) & DESP_EXPANDED);
    const auto bExpand = (dwStyle & DESP_EXPANDED);
    if (bExpandChanged)
        bExpand ? GetWnd()->CeAdd(this) : GetWnd()->CeRemove(this);
    // 检查DES_OWNER_COMP_CACHE变动
    if (((dwOld ^ dwStyle) & DES_OWNER_COMP_CACHE) && GetCompositor())
        CompInvalidateCacheBitmap();
}

inline HRESULT CElem::CompUpdateCacheBitmap(float cx, float cy) noexcept
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

    const int cxPhy = (int)ceilf(Log2PhyF((float)cx));
    const int cyPhy = (int)ceilf(Log2PhyF((float)cy));
    if (GetStyle() & DES_OWNER_COMP_CACHE)
    {
        if (FAILED(GetCompositor()->CreateCacheBitmap(
            cxPhy, cyPhy, m_pCompCacheSurface)))
        {
            CREATE_CACHE_BITMAP_INFO ccbi;
            ccbi.cxPhy = cxPhy;
            ccbi.cyPhy = cyPhy;
            ccbi.hr = E_NOTIMPL;
            const auto pNotifyElem = GetParentElem() ? GetParentElem() : this;
            if (pNotifyElem->CallEvent(
                EWM_CREATE_CACHE_BITMAP, (WPARAM)&ccbi, 0))
                m_pCompCacheSurface = ccbi.pCacheSurface;
            return ccbi.hr;
        }
    }
    else
        GetWnd()->BmpNew(cxPhy, cyPhy, m_pCompBitmap);
    return S_OK;
}

EckInline CElem* CElem::SetCapture() noexcept { return GetWnd()->ElemSetCapture(this); }
EckInlineNdCe CElem* CElem::GetCapture() noexcept { return GetWnd()->ElemGetCapture(); }
EckInline void CElem::ReleaseCapture() noexcept { GetWnd()->ElemReleaseCapture(); }
EckInline void CElem::SetFocus() noexcept { GetWnd()->ElemSetFocus(this); }
EckInlineNdCe CElem* CElem::GetFocus() noexcept { return GetWnd()->ElemGetFocus(); }
EckInline BOOL CElem::SetTimer(UINT_PTR uId, UINT uElapse) noexcept { return GetWnd()->ElemSetTimer(this, uId, uElapse); }
EckInline BOOL CElem::KillTimer(UINT_PTR uId) noexcept { return GetWnd()->ElemKillTimer(this, uId); }
EckInlineNdCe CCriticalSection& CElem::GetCriticalSection() const noexcept { return GetWnd()->GetCriticalSection(); }
EckInlineNdCe int CElem::Log2Phy(int i) const noexcept { return GetWnd()->Log2Phy(i); }
EckInlineNdCe float CElem::Log2PhyF(float f) const noexcept { return GetWnd()->Log2PhyF(f); }
EckInlineNdCe int CElem::Phy2Log(int i) const noexcept { return GetWnd()->Phy2Log(i); }
EckInlineNdCe float CElem::Phy2LogF(float f) const noexcept { return GetWnd()->Phy2LogF(f); }
EckInlineNdCe ID2D1Bitmap1* CElem::GetCacheBitmap() const noexcept { return GetWnd()->GetCacheBitmap(); }
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END