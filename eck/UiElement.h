#pragma once
#include "CWindow.h"
#include "KwDef.h"

#define ECK_UIELE_NAMESPACE_BEGIN namespace UiElement {
#define ECK_UIELE_NAMESPACE_END   }

ECK_NAMESPACE_BEGIN
ECK_UIELE_NAMESPACE_BEGIN
namespace Declaration
{
    enum : UINT
    {
        DES_VISIBLE = 1u << 0,
        DES_NO_REDRAW = 1u << 1,
        DES_DISABLE = 1u << 2,
        DES_TABSTOP = 1u << 3,
        DES_DEF_NO_BUBBLE = 1u << 4,// 默认事件过程不应将事件向上冒泡 
        DES_BUBBLE_INPUT = 1u << 5, // 冒泡所有输入事件
        DES_BUBBLE_ALL = 1u << 6,   // 冒泡所有事件
    };

    enum : UINT
    {
        EWM_DUMMY = WM_USER_SAFE,
        EWM_SHOWFOCUS,  // void(bShow, 0)
        EWM_BUBBLE,     // BOOL(0, BBEVENT*)  返回是否拦截
        EWM_SYSBEGIN,
    };
}
using namespace Declaration;

template<class TElement>
class CElementContainer : public CWindow
{
    template<class THost_, bool IsFloat>
    friend class CElement;
public:
    using TRect = typename TElement::TRect;
    using TPoint = typename TElement::TPoint;
    using TCoord = typename TElement::TCoord;
private:
    struct TIMER
    {
        TElement* pEle;
        UINT_PTR uId;
    };

    TElement* m_pFirstChild{};
    TElement* m_pLastChild{};
    TElement* m_pFocus{};
    TElement* m_pHover{};// WM_MOUSELEAVE使用
    TElement* m_pCurrNcHitTest{};
    TElement* m_pMouseCapture{};

    std::vector<TIMER> m_vTimer{};

    int m_iUserDpi{ 96 };
    int m_iDpi{ 96 };

    TPoint m_sizeClientLog{};
    int m_cxClient{};
    int m_cyClient{};

    int m_cChildren{};

    BOOLEAN m_bMouseCaptured{};
    BOOLEAN m_bShowFocus{};
private:
    void ElementDestroying(TElement* pEle) noexcept
    {
        if (m_pFocus == pEle)
            m_pFocus = nullptr;
        if (m_pCurrNcHitTest == pEle)
            m_pCurrNcHitTest = nullptr;
        if (m_pMouseCapture == pEle)
            EleReleaseCapture();
        if (m_pHover == pEle)
            m_pHover = nullptr;
        for (auto it = m_vTimer.begin(); it != m_vTimer.end(); )
        {
            if (it->pEle == pEle)
            {
                KillTimer(HWnd, it->uId);
                it = m_vTimer.erase(it);
            }
            else
                ++it;
        }
    }

    void UpdateLogicalClientSize() noexcept
    {
        m_sizeClientLog = { (TCoord)m_cxClient, (TCoord)m_cyClient };
        PixelToLogical(m_sizeClientLog);
    }
public:
    LRESULT OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept override
    {
        if ((uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST))
        {
            if (uMsg == WM_LBUTTONUP)
                EleShowFocus(FALSE);
            TPoint pt{ (TCoord)GET_X_LPARAM(lParam), (TCoord)GET_Y_LPARAM(lParam) };
            PixelToLogical(pt);
            if (m_bMouseCaptured)
                m_pCurrNcHitTest = TElement::EtHitTest(EtLastChild(), pt);
            const auto pEle = (m_pMouseCapture ? m_pMouseCapture : m_pCurrNcHitTest);
            if (pEle)
            {
                pEle->ClientToElement(pt);
                pEle->EhTransform(pt, FALSE);
                pEle->CallEvent(uMsg, wParam, (LPARAM)&pt);
            }

            if (uMsg == WM_MOUSEMOVE)// 移出监听
            {
                if (m_pHover != m_pCurrNcHitTest && !m_bMouseCaptured)
                {
                    if (m_pHover)
                        m_pHover->CallEvent(WM_MOUSELEAVE, 0, 0);
                    m_pHover = m_pCurrNcHitTest;
                }
                TRACKMOUSEEVENT tme;
                tme.cbSize = sizeof(tme);
                tme.dwFlags = TME_LEAVE;
                tme.hwndTrack = HWnd;
                TrackMouseEvent(&tme);
            }
            goto CallDefault;
        }
        else if (uMsg >= WM_NCMOUSEMOVE && uMsg <= WM_NCXBUTTONDBLCLK)
        {
            if (m_bMouseCaptured)
            {
                POINT pt ECK_GET_PT_LPARAM(lParam);
                ScreenToClient(HWnd, &pt);
                TPoint ptLog{ (TCoord)pt.x, (TCoord)pt.y };
                PixelToLogical(ptLog);
                m_pCurrNcHitTest = TElement::EtHitTest(EtLastChild(), ptLog);
            }
            const auto pEle = (m_pMouseCapture ? m_pMouseCapture : m_pCurrNcHitTest);
            if (pEle)
                pEle->CallEvent(uMsg, wParam, lParam);

            if (uMsg == WM_NCMOUSEMOVE)// 移出监听
            {
                if (m_pHover != m_pCurrNcHitTest && !m_bMouseCaptured)
                {
                    if (m_pHover)
                        m_pHover->CallEvent(WM_MOUSELEAVE, 0, 0);
                    m_pHover = m_pCurrNcHitTest;
                }
                TRACKMOUSEEVENT tme;
                tme.cbSize = sizeof(tme);
                tme.dwFlags = TME_LEAVE | TME_NONCLIENT;
                tme.hwndTrack = HWnd;
                TrackMouseEvent(&tme);
            }
            else if (uMsg == WM_NCLBUTTONDOWN)// 修正放开事件
            {
                if (pEle)
                {
                    const auto lResult = __super::OnMessage(uMsg, wParam, lParam);
                    POINT pt;
                    GetCursorPos(&pt);
                    pEle->CallEvent(WM_NCLBUTTONUP, wParam, MAKELPARAM(pt.x, pt.y));
                    return lResult;
                }
            }
            goto CallDefault;
        }
        else if (uMsg >= WM_KEYFIRST && uMsg <= WM_IME_KEYLAST)
        {
            if (!m_pFocus)
                goto CallDefault;
            const MSG Msg
            {
                .message = uMsg,
                .wParam = wParam,
                .lParam = lParam
            };
            switch (uMsg)
            {
            case WM_KEYDOWN:
            {
                switch (wParam)
                {
                case VK_TAB:
                {
                    const auto uDlgCode = m_pFocus->CallEvent(
                        WM_GETDLGCODE, wParam, (LPARAM)&Msg);
                    if (uDlgCode & (DLGC_WANTMESSAGE | DLGC_WANTTAB))
                        break;
                    const auto pNextFocus =
                        m_pFocus->EtNextTabStop(!(GetKeyState(VK_SHIFT) & 0x8000));
                    if (pNextFocus)
                    {
                        EleShowFocus(TRUE);
                        EleSetFocus(pNextFocus);
                        goto CallDefault;
                    }
                }
                break;
                }
            }
            break;
            }
            m_pFocus->CallEvent(uMsg, wParam, lParam);
            goto CallDefault;
        }

        switch (uMsg)
        {
        case WM_NCHITTEST:
        {
            auto lResult = __super::OnMessage(uMsg, wParam, lParam);
            if (lResult != HTCLIENT)
                return lResult;
            POINT pt ECK_GET_PT_LPARAM(lParam);
            ScreenToClient(HWnd, &pt);
            TPoint ptLog{ (TCoord)pt.x, (TCoord)pt.y };
            PixelToLogical(ptLog);
            LRESULT lResultNew;
            if (m_pCurrNcHitTest = TElement::EtHitTest(EtLastChild(), ptLog, &lResultNew))
                return lResultNew;
            return lResult;
        }

        case WM_SETCURSOR:
        {
            const auto pEle = (m_pMouseCapture ? m_pMouseCapture : m_pCurrNcHitTest);
            if (pEle && pEle->CallEvent(uMsg, wParam, lParam))
                return TRUE;
        }
        break;

        case WM_SIZE:
        {
            ECK_GET_SIZE_LPARAM(m_cxClient, m_cyClient, lParam);
            UpdateLogicalClientSize();
        }
        break;

        case WM_TIMER:
        {
            for (const auto& e : m_vTimer)
            {
                if (e.uId == wParam)
                {
                    e.pEle->CallEvent(WM_TIMER, wParam, 0);
                    return 0;
                }
            }
        }
        return 0;

        case WM_NCMOUSELEAVE:
        case WM_MOUSELEAVE:
            if (m_pHover)
            {
                m_pHover->CallEvent(WM_MOUSELEAVE, 0, 0);
                m_pHover = nullptr;
            }
            break;

        case WM_CAPTURECHANGED:
        {
            if (m_bMouseCaptured)
            {
                m_bMouseCaptured = FALSE;
                if (m_pMouseCapture)
                {
                    m_pMouseCapture->CallEvent(WM_CAPTURECHANGED, 0, NULL);
                    if (m_pHover && m_pHover != m_pMouseCapture)
                    {
                        m_pHover->CallEvent(WM_MOUSELEAVE, 0, 0);
                        m_pHover = nullptr;
                    }
                    m_pMouseCapture = nullptr;
                }
            }
        }
        break;

        case WM_GETOBJECT:
        {

        }
        return 0;

        case WM_DPICHANGED:
            m_iDpi = HIWORD(wParam);
            MsgOnDpiChanged(HWnd, lParam);
            return 0;
        case WM_DPICHANGED_AFTERPARENT:
            m_iDpi = GetDpi(HWnd);
            return 0;

        case WM_CREATE:
        {
            RECT rcClient;
            GetClientRect(HWnd, &rcClient);
            m_cxClient = rcClient.right - rcClient.left;
            m_cyClient = rcClient.bottom - rcClient.top;
            UpdateLogicalClientSize();
        }
        break;
        }
    CallDefault:
        return __super::OnMessage(uMsg, wParam, lParam);
    }
protected:
    EckInlineCe void SetUserDpi(int iDpi) noexcept
    {
        m_iUserDpi = iDpi;
        UpdateLogicalClientSize();
    }
    EckInlineNdCe int GetUserDpi() const noexcept { return m_iUserDpi; }
public:
    template<std::floating_point T>
    EckInlineNdCe T PixelToLogical(T v) const noexcept { return v * (T)96 / m_iUserDpi; }
    template<std::floating_point T>
    EckInlineNdCe T LogicalToPixel(T v) const noexcept { return v * m_iUserDpi / (T)96; }
    template<std::integral T>
    EckInlineNdCe T PixelToLogical(T v) const noexcept
    {
        return T(v * 96.f / m_iUserDpi + 0.5f);
    }
    template<std::integral T>
    EckInlineNdCe T LogicalToPixel(T v) const noexcept
    {
        return T(v * m_iUserDpi / 96.f + 0.5f);
    }

    EckInlineCe void PixelToLogical(_Inout_ TPoint& pt) const noexcept
    {
        pt.x = PixelToLogical(pt.x);
        pt.y = PixelToLogical(pt.y);
    }
    EckInlineCe void PixelToLogical(_Inout_ TRect& rc) const noexcept
    {
        rc.left = PixelToLogical(rc.left);
        rc.top = PixelToLogical(rc.top);
        rc.right = PixelToLogical(rc.right);
        rc.bottom = PixelToLogical(rc.bottom);
    }

    EckInlineCe void LogicalToPixel(_Inout_ TPoint& pt) const noexcept
    {
        pt.x = LogicalToPixel(pt.x);
        pt.y = LogicalToPixel(pt.y);
    }
    EckInlineCe void LogicalToPixel(_Inout_ TRect& rc) const noexcept
    {
        rc.left = LogicalToPixel(rc.left);
        rc.top = LogicalToPixel(rc.top);
        rc.right = LogicalToPixel(rc.right);
        rc.bottom = LogicalToPixel(rc.bottom);
    }
public:
    EckInlineNdCe TElement* EtFirstChild() const noexcept { return m_pFirstChild; }
    EckInlineNdCe TElement* EtLastChild() const noexcept { return m_pLastChild; }

    void EtBroadcastEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept
    {
        auto pEle = EtFirstChild();
        while (pEle)
        {
            pEle->CallEvent(uMsg, wParam, lParam);
            pEle->EtBroadcastEvent(uMsg, wParam, lParam);
            pEle = pEle->EtNext();
        }
    }
public:
    TElement* EleSetFocus(TElement* pEle) noexcept
    {
        SetFocus(HWnd);
        if (m_pFocus == pEle)
            return pEle;
        auto pOld = m_pFocus;
        m_pFocus = pEle;
        if (pOld)
            pOld->CallEvent(WM_KILLFOCUS, (WPARAM)pEle, 0);
        pEle->CallEvent(WM_SETFOCUS, (WPARAM)pOld, 0);
        return pOld;
    }
    EckInlineNdCe TElement* EleGetFocus() const noexcept { return m_pFocus; }

    TElement* EleSetCapture(TElement* pEle) noexcept
    {
        auto pOld = m_pMouseCapture;
        m_pMouseCapture = pEle;
        if (GetCapture() != HWnd)
        {
            SetCapture(HWnd);
            m_bMouseCaptured = TRUE;
        }
        if (pOld)
            pOld->CallEvent(WM_CAPTURECHANGED, 0, (LPARAM)pEle);
        return pOld;
    }
    EckInline void EleReleaseCapture() noexcept
    {
        ReleaseCapture();
        // WM_CAPTURECHANGED will process it:
        // m_pMouseCapture->CallEvent(WM_CAPTURECHANGED, 0, nullptr);
        // m_pMouseCapture = nullptr;
    }
    EckInlineNdCe TElement* EleGetCapture() const noexcept { return m_pMouseCapture; }

    BOOL EleSetTimer(TElement* pEle, UINT_PTR uId, UINT uElapse) noexcept
    {
        if (!SetTimer(HWnd, uId, uElapse, nullptr))
            return FALSE;
        for (auto& e : m_vTimer)
        {
            if (e.pEle == pEle && e.uId == uId)
                return TRUE;
        }
        m_vTimer.emplace_back(pEle, uId);
        return TRUE;
    }
    BOOL EleKillTimer(TElement* pEle, UINT_PTR uId) noexcept
    {
        for (auto it = m_vTimer.begin(); it != m_vTimer.end(); ++it)
        {
            if (it->pEle == pEle && it->uId == uId)
            {
                KillTimer(HWnd, uId);
                m_vTimer.erase(it);
                return TRUE;
            }
        }
        return FALSE;
    }

    void EleShowFocus(BOOL bShow) noexcept
    {
        if (m_bShowFocus == !!bShow)
            return;
        m_bShowFocus = !!bShow;
        EtBroadcastEvent(EWM_SHOWFOCUS, m_bShowFocus, 0);
    }
    EckInlineNdCe BOOL EleIsShowingFocus() const noexcept { return m_bShowFocus; }
public:
    EckInlineNdCe TCoord GetClientWidthLogical() const noexcept { return m_sizeClientLog.x; }
    EckInlineNdCe TCoord GetClientHeightLogical() const noexcept { return m_sizeClientLog.y; }
    EckInlineNdCe int GetClientWidth() const noexcept { return m_cxClient; }
    EckInlineNdCe int GetClientHeight() const noexcept { return m_cyClient; }
};

/*
* 实现了以下事件（除非特别说明，所有坐标均为逻辑坐标）
* 未标注参数类型的原样转发Win32消息
*
* WM_MOUSEFIRST ~ WM_MOUSELAST          (MkFlags, Kw::Vec2*   ) InElement
* WM_MOUSELEAVE                         (0      , 0           )
* WM_NCMOUSEMOVE ~ WM_NCXBUTTONDBLCLK
* WM_KEYFIRST ~ WM_IME_KEYLAST          (Vk     , KeyDownFlags)
* WM_NCHITTEST                          (0      , Kw::Vec2    ) InClient
* WM_SETCURSOR
* WM_TIMER                              (Id     , 0           )
* WM_CAPTURECHANGED                     (0      , pEleNew     )
* WM_SETFOCUS                           (pEleOld, 0           )
* WM_KILLFOCUS                          (pEleNew, 0           )
*/
template<class THost_, bool IsFloat>
class CElement : public ILayout
{
public:
    using THost = THost_;
    using TCoord = std::conditional_t<IsFloat, float, int>;
    using TRect = std::conditional_t<IsFloat, Kw::Rect, RECT>;
    using TPoint = std::conditional_t<IsFloat, Kw::Vec2, POINT>;

    const static inline CElement* EtTop{};
    const static inline CElement* EtBottom{ (CElement*)(UINT_PTR)1 };

    struct BBEVENT
    {
        THost* pEle;
        UINT uMsg;
        WPARAM wParam;
        LPARAM lParam;
        LRESULT lResult;// Out
    };
private:
    THost* m_pNext{};    // 下一元素，Z序高于当前
    THost* m_pPrev{};    // 上一元素，Z序低于当前
    THost* m_pParent{};
    THost* m_pFirstChild{};
    THost* m_pLastChild{};
    CElementContainer<THost>* m_pContainer{};

    CEventChain<Intercept_T, LRESULT, UINT, WPARAM, LPARAM> m_ec{};

    TRect m_rc{};           // 相对父元素
    TPoint m_ptInClient{};  // 相对于客户区的坐标

    UINT m_uStyle{};
    UINT m_cChildren{};     // 暂时未用

    constexpr void CorrectChildrenRectInClient() const noexcept
    {
        auto pEle = EtFirstChild();
        while (pEle)
        {
            pEle->m_ptInClient =
            {
                pEle->m_rc.left + m_ptInClient.x,
                pEle->m_rc.top + m_ptInClient.y
            };
            pEle->CorrectChildrenRectInClient();
            pEle = pEle->EtNext();
        }
    }
public:
    LYTPOINT LoGetPosition() noexcept override
    {
        return { (TLytCoord)m_rc.left, (TLytCoord)m_rc.top };
    }
    LYTSIZE LoGetSize() noexcept override
    {
        return { (TLytCoord)GetWidth(), (TLytCoord)GetHeight() };
    }
    void LoShow(BOOL bShow) noexcept override { SetVisible(bShow); }

    virtual LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept
    {
        switch (uMsg)
        {
        case WM_NCHITTEST:
            return HTCLIENT;
        }

        if (!(GetStyle() & DES_DEF_NO_BUBBLE) &&
            IsNeedBubble(uMsg))
        {
            BOOL b;
            const auto lResult = BubbleEvent(uMsg, wParam, lParam, b);
            if (b)
                return lResult;
        }
        return 0;
    }

    virtual BOOL EhTransform(_Inout_ TPoint& pt, BOOL bInClient) noexcept { return FALSE; }

    EckInline LRESULT CallEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept
    {
        SlotCtx Ctx{};
        const auto r = m_ec.EmitWithContext(Ctx, uMsg, wParam, lParam);
        if (Ctx.IsProcessed())
            return r;
        return OnEvent(uMsg, wParam, lParam);
    }

    LRESULT BubbleEvent(UINT uMsg, WPARAM wParam, LPARAM lParam,
        _Out_ BOOL& bProcessed) noexcept
    {
        bProcessed = FALSE;
        auto pParent = EtParent();
        if (!pParent)
            return 0;
        BBEVENT Event{ (THost*)this, uMsg, wParam, lParam };
        while (pParent)
        {
            if (pParent->CallEvent(EWM_BUBBLE, 0, (LPARAM)&Event))
            {
                bProcessed = TRUE;
                return Event.lResult;
            }
        }
        return 0;
    }

    EckInlineNdCe BOOL IsNeedBubble(UINT uMsg) const noexcept
    {
        if (GetStyle() & DES_BUBBLE_ALL)
            return TRUE;
        if ((GetStyle() & DES_BUBBLE_INPUT) && IsInputMessage(uMsg))
            return TRUE;
        return FALSE;
    }
protected:
    void OnCreate(
        const TRect& rc, UINT uStyle,
        THost* pParent,
        CElementContainer<THost>* pContainer) noexcept
    {
        m_pParent = pParent;
        m_pContainer = pContainer;
        SetRect(rc);
        SetStyle(uStyle);

        auto& pParentLastChild =
            (pParent ? pParent->m_pLastChild : pContainer->m_pLastChild);
        auto& pParentFirstChild =
            (pParent ? pParent->m_pFirstChild : pContainer->m_pFirstChild);
        if (pParent)
            ++pParent->m_cChildren;
        else
            ++pContainer->m_cChildren;

        if (pParentLastChild)
        {
            m_pPrev = pParentLastChild;
            m_pNext = nullptr;
            m_pPrev->m_pNext = (THost*)this;
            pParentLastChild = (THost*)this;
        }
        else
        {
            m_pPrev = m_pNext = nullptr;
            pParentFirstChild = (THost*)this;
            pParentLastChild = (THost*)this;
        }
    }

    void OnDestroy() noexcept
    {
        if (m_pParent)
            --m_pParent->m_cChildren;
        else
            --m_pContainer->m_cChildren;
        m_pContainer->ElementDestroying(this);

        if (m_pPrev)
            m_pPrev->m_pNext = m_pNext;
        else
        {
            if (m_pParent)
                m_pParent->m_pFirstChild = m_pNext;
            else
                m_pContainer->m_pFirstChild = m_pNext;
        }

        if (m_pNext)
            m_pNext->m_pPrev = m_pPrev;
        else
        {
            if (m_pParent)
                m_pParent->m_pLastChild = m_pPrev;
            else
                m_pContainer->m_pLastChild = m_pPrev;
        }

        m_pNext = nullptr;
        m_pPrev = nullptr;
        m_pParent = nullptr;
        m_pFirstChild = nullptr;
        m_pLastChild = nullptr;
        m_pContainer = nullptr;

        m_rc = {};
        m_ptInClient = {};
    }
protected:
    EckInlineCe void SetStyle(UINT uStyle) noexcept { m_uStyle = uStyle; }
    EckInlineNdCe UINT GetStyle() const noexcept { return m_uStyle; }

    EckInlineCe void SetContainer(CElementContainer<THost>* p) noexcept { m_pContainer = p; }
    EckInlineNdCe CElementContainer<THost>* GetContainer() const noexcept { return m_pContainer; }
#pragma region Coordinate
protected:
    constexpr void SetRect(const TRect& rc) noexcept
    {
        m_rc = rc;
        m_ptInClient = { rc.left, rc.top };
        if (m_pParent)
        {
            m_ptInClient.x += m_pParent->m_ptInClient.x;
            m_ptInClient.y += m_pParent->m_ptInClient.y;
        }
        CorrectChildrenRectInClient();
    }
    constexpr void SetPosition(TCoord x, TCoord y) noexcept
    {
        m_rc = { x, y, x + GetWidth(), y + GetHeight() };
        m_ptInClient = { x, y };
        if (m_pParent)
        {
            m_ptInClient.x += m_pParent->m_ptInClient.x;
            m_ptInClient.y += m_pParent->m_ptInClient.y;
        }
        CorrectChildrenRectInClient();
    }
    void SetSize(TCoord cx, TCoord cy) noexcept
    {
        m_rc.right = m_rc.left + cx;
        m_rc.bottom = m_rc.top + cy;
    }
public:
    EckInlineNdCe TCoord GetWidth() const noexcept { return m_rc.right - m_rc.left; }
    EckInlineNdCe TCoord GetHeight() const noexcept { return m_rc.bottom - m_rc.top; }

    EckInlineNdCe auto& GetRect() const noexcept { return m_rc; }
    EckInlineNdCe TRect GetViewRect() const noexcept
    {
        return { 0, 0, m_rc.right - m_rc.left, m_rc.bottom - m_rc.top };
    }
    EckInlineNdCe TRect GetRectInClient() const noexcept
    {
        return
        {
            m_ptInClient.x,
            m_ptInClient.y,
            m_ptInClient.x + m_rc.right - m_rc.left,
            m_ptInClient.y + m_rc.bottom - m_rc.top
        };
    }

    EckInlineCe void ClientToElement(_Inout_ CcpRect auto& rc) const noexcept
    {
        OffsetRect(rc, -m_ptInClient.x, -m_ptInClient.y);
    }
    EckInlineCe void ClientToElement(_Inout_ TPoint& pt) const noexcept
    {
        pt.x -= m_ptInClient.x;
        pt.y -= m_ptInClient.y;
    }
    EckInlineCe void ElementToClient(_Inout_ CcpRect auto& rc) const noexcept
    {
        OffsetRect(rc, m_ptInClient.x, m_ptInClient.y);
    }
    EckInlineCe void ElementToClient(_Inout_ TPoint& pt) const noexcept
    {
        pt.x += m_ptInClient.x;
        pt.y += m_ptInClient.y;
    }
#pragma endregion Coordinate

#pragma region ElementTree
public:
    EckInlineNdCe THost* EtFirstChild() const noexcept { return m_pFirstChild; }
    EckInlineNdCe THost* EtLastChild() const noexcept { return m_pLastChild; }
    EckInlineNdCe THost* EtParent() const noexcept { return m_pParent; }
    // 取下一元素，Z序高于当前
    EckInlineNdCe THost* EtNext() const noexcept { return m_pNext; }
    // 取上一元素，Z序低于当前
    EckInlineNdCe THost* EtPrevious() const noexcept { return m_pPrev; }

    static THost* EtHitTest(
        THost* pEle,
        TPoint ptInClient,
        _Out_opt_ LRESULT* pResult = nullptr) noexcept
    {
        TPoint pt0;
        while (pEle)
        {
            if (pEle->GetStyle() & DES_VISIBLE)
            {
                pt0 = ptInClient;
                pEle->EhTransform(pt0, TRUE);
                if (PtInRect(pEle->GetRectInClient(), pt0))
                {
                    const auto pHit = pEle->EtHitTest(ptInClient, pResult);
                    if (pHit)
                        return pHit;
                    else
                    {
                        const auto lResult = pEle->CallEvent(
                            WM_NCHITTEST, 0, (LPARAM)&pt0);
                        if (lResult != HTTRANSPARENT)
                        {
                            if (pResult)
                                *pResult = lResult;
                            return pEle;
                        }
                    }
                }
            }
            pEle = pEle->EtPrevious();
        }
        if (pResult)
            *pResult = HTNOWHERE;
        return nullptr;
    }
    EckInline THost* EtHitTest(
        TPoint ptInClient,
        _Out_opt_ LRESULT* pResult = nullptr) noexcept
    {
        return EtHitTest(EtLastChild(), ptInClient, pResult);
    }
private:
    static THost* EtpNextConteol(CElement* pCurr, BOOL bNextOrPrev) noexcept
    {
        THost* pEle;
        if (bNextOrPrev)
        {
            pEle = pCurr->EtNext();
            if (!pEle)
                pEle = (pCurr->EtParent() ?
                    pCurr->EtParent()->EtFirstChild() :
                    pCurr->GetContainer()->EtFirstChild());
        }
        else
        {
            pEle = pCurr->EtPrevious();
            if (!pEle)
                pEle = (pCurr->EtParent() ?
                    pCurr->EtParent()->EtLastChild() :
                    pCurr->GetContainer()->EtLastChild());
        }
        return pEle;
    }
public:
    THost* EtNextTabStop(BOOL bNextOrPrev) noexcept
    {
        auto pEle = EtpNextConteol(this, bNextOrPrev);
        while (pEle != this)
        {
            const auto uStyle = pEle->GetStyle();
            if ((uStyle & (DES_VISIBLE | DES_DISABLE | DES_TABSTOP)) ==
                (DES_VISIBLE | DES_TABSTOP))
                return pEle;
            pEle = EtpNextConteol(pEle, bNextOrPrev);
        }
        return nullptr;
    }

    void EtBroadcastEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept
    {
        auto pEle = EtFirstChild();
        while (pEle)
        {
            pEle->CallEvent(uMsg, wParam, lParam);
            pEle->EtBroadcastEvent(uMsg, wParam, lParam);
            pEle = pEle->EtNext();
        }
    }
protected:
    void SetZOrder(THost* pEleAfter) noexcept
    {
        auto& pParentLastChild = (m_pParent ?
            m_pParent->m_pLastChild :
            m_pContainer->m_pLastChild);
        auto& pParentFirstChild = (m_pParent ?
            m_pParent->m_pFirstChild :
            m_pContainer->m_pFirstChild);

        if (pEleAfter == EtBottom)
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
        else if (pEleAfter == EtTop)
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

            m_pPrev = pEleAfter;
            m_pNext = pEleAfter->m_pNext;

            m_pPrev->m_pNext = this;
            if (m_pNext)
                m_pNext->m_pPrev = this;
            else
                pParentLastChild = this;
        }
    }
#pragma endregion ElementTree
protected:
    void SetVisible(BOOL b) noexcept
    {
        auto u = GetStyle();
        if (b)
            if (u & DES_VISIBLE)
                return;
            else
                u |= DES_VISIBLE;
        else
            if (u & DES_VISIBLE)
                u &= ~DES_VISIBLE;
            else
                return;
        SetStyle(u);
    }
public:
    // 是否可见。函数自底向上遍历父元素，如果存在不可见父元素，则返回FALSE，否则返回TRUE。
    EckInlineNdCe BOOL IsVisible() const noexcept
    {
        auto pParent = this;
        do
        {
            if (!(pParent->GetStyle() & DES_VISIBLE))
                return FALSE;
            pParent = pParent->EtParent();
        } while (pParent);
        return TRUE;
    }
};
ECK_UIELE_NAMESPACE_END
ECK_NAMESPACE_END