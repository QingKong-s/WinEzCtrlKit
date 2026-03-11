#pragma once
#include "CWindow.h"
#include "KwDef.h"
#include "CUnknown.h"
#include "ComPtr.h"

#include <UIAutomation.h>

#pragma comment(lib, "Uiautomationcore.lib")

ECK_NAMESPACE_BEGIN
ECK_UIBASIC_NAMESPACE_BEGIN
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
        DES_NO_FOCUSABLE = 1u << 7,
        DES_NOTIFY_WND = 1u << 8,   // 元素产生的通知发送到窗口
        DES_NOTIFY_PARENT = 1u << 9,// 元素产生的通知发送到父元素
        DES_DBG_FRAME = 1u << 10,
        DES_NO_CLIP = 1u << 11,     // 重画时不剪辑到元素矩形
    };

    enum : UINT
    {
        EWM_DUMMY = WM_USER_SAFE,

        EWM_SHOWFOCUS,  // void(bShow, 0)
        EWM_BUBBLE,     // BOOL(0, BBEVENT*)  返回是否拦截

        // 转发OLE拖放
        EWM_DRAGENTER,  // HRESULT(DRAGDROPINFO*, 0)
        EWM_DRAGLEAVE,  // HRESULT(0, 0)
        EWM_DRAGOVER,   // HRESULT(DRAGDROPINFO*, 0)
        EWM_DROP,       // HRESULT(DRAGDROPINFO*, 0)

        EWM_DROP_WIN31, // void(HDROP, 0)  转发Win3.1风格拖放WM_DROPFILES

        WM_TICK,        // 定时器线程发送到窗口

        EWM_SYSBEGIN,
    };

    enum : UINT
    {
        ENM_DUMMY,

        ENM_COMMAND,
        ENM_SCROLL,

        ENM_SYSBEGIN,
    };

    enum : UINT
    {
        SaNormal = 0u,
        SaHot = 1u << 0,
        SaActive = 1u << 1,
        SaDisable = 1u << 2,
        SaFocus = 1u << 3,
        SaSelected = 1u << 4,
        SaMixed = 1u << 5,
        SapLButtonDown = 1u << 6,
    };

    struct ELENMHDR
    {
        UINT uNotify;
        BOOL bProcessed;
    };
}
using namespace Declaration;

// Win8+
static HRESULT UiaDisconnectProvider_I(IRawElementProviderSimple* p) noexcept
{
    using FUiaDisconnectProvider = HRESULT(WINAPI*)(IRawElementProviderSimple*);
    const static auto s_pfn = []
        {
            const auto h = GetModuleHandleW(L"UIAutomationCore.dll");
            if (h)
                return (FUiaDisconnectProvider)GetProcAddress(h, "UiaDisconnectProvider");
            return (FUiaDisconnectProvider)nullptr;
        }();
    if (s_pfn)
        return s_pfn(p);
    return S_FALSE;
}

template<class TElement>
class CElementContainer : public CWindow
{
    template<class THost_, bool IsFloat>
    friend class CElement;
public:
    using TRect = typename TElement::TRect;
    using TPoint = typename TElement::TPoint;
    using TCoord = typename TElement::TCoord;

    struct DRAGDROPINFO
    {
        IDataObject* pDataObj;
        DWORD grfKeyState;
        DWORD* pdwEffect;
        POINT ptInClient;// 物理坐标
        TPoint ptInClientLog;
    };
private:
    class CUiaContainer final : public CUnknown<CUiaContainer,
        IRawElementProviderSimple,
        IRawElementProviderFragment,
        IRawElementProviderFragmentRoot>
    {
    private:
        CElementContainer* m_pContainer{};
    public:
        virtual ~CUiaContainer() = default;
        void SetContainer(CElementContainer* p) noexcept
        {
            if (!p)
                UiaDisconnectProvider_I(this);
            m_pContainer = p;
        }
        // -----IRawElementProviderSimple-----

        STDMETHODIMP get_ProviderOptions(ProviderOptions* pRetVal) override
        {
            *pRetVal = ProviderOptions_ServerSideProvider | ProviderOptions_UseComThreading;
            return S_OK;
        }
        STDMETHODIMP GetPatternProvider(PATTERNID idPattern, IUnknown** pRetVal) override
        {
            *pRetVal = nullptr;
            return S_OK;
        }
        STDMETHODIMP GetPropertyValue(PROPERTYID idProp, VARIANT* pRetVal) override
        {
            switch (idProp)
            {
            case UIA_ControlTypePropertyId:
                pRetVal->vt = VT_I4;
                pRetVal->intVal = UIA_WindowControlTypeId;
                break;
            case UIA_IsEnabledPropertyId:
                pRetVal->vt = VT_BOOL;
                pRetVal->boolVal = VARIANT_TRUE;
                break;
            case UIA_IsKeyboardFocusablePropertyId:
                pRetVal->vt = VT_BOOL;
                pRetVal->boolVal = VARIANT_TRUE;
                break;
            default:
                pRetVal->vt = VT_EMPTY;
                break;
            }
            return S_OK;
        }
        STDMETHODIMP get_HostRawElementProvider(IRawElementProviderSimple** pRetVal) override
        {
            if (!m_pContainer)
            {
                *pRetVal = nullptr;
                return UIA_E_ELEMENTNOTAVAILABLE;
            }
            return UiaHostProviderFromHwnd(m_pContainer->HWnd, pRetVal);
        }
        // -----IRawElementProviderFragment-----

        STDMETHODIMP Navigate(NavigateDirection eDirection,
            IRawElementProviderFragment** pRetVal) override
        {
            *pRetVal = nullptr;
            if (!m_pContainer)
                return UIA_E_ELEMENTNOTAVAILABLE;
            TElement* pEle;
            switch (eDirection)
            {
            case NavigateDirection_FirstChild: pEle = m_pContainer->EtFirstChild(); break;
            case NavigateDirection_LastChild:  pEle = m_pContainer->EtLastChild();  break;
            default:                           pEle = nullptr;
            }
            if (pEle)
                return pEle->UiaQueryInterface(pRetVal);
            return S_OK;
        }

        STDMETHODIMP GetRuntimeId(SAFEARRAY** pRetVal) override
        {
            *pRetVal = nullptr;
            return S_OK;
        }
        STDMETHODIMP get_BoundingRectangle(UiaRect* pRetVal) override
        {
            if (!m_pContainer)
            {
                *pRetVal = {};
                return UIA_E_ELEMENTNOTAVAILABLE;
            }
            RECT rcInScr{ 0, 0, m_pContainer->m_cxClient, m_pContainer->m_cyClient };
            ClientToScreen(m_pContainer->HWnd, &rcInScr);
            pRetVal->left = (double)rcInScr.left;
            pRetVal->top = (double)rcInScr.top;
            pRetVal->width = double(rcInScr.right - rcInScr.left);
            pRetVal->height = double(rcInScr.bottom - rcInScr.top);
            return S_OK;
        }
        STDMETHODIMP GetEmbeddedFragmentRoots(SAFEARRAY** pRetVal) override
        {
            *pRetVal = nullptr;
            return S_OK;
        }
        STDMETHODIMP SetFocus() override { return S_OK; }
        STDMETHODIMP get_FragmentRoot(IRawElementProviderFragmentRoot** pRetVal) override
        {
            *pRetVal = this;
            this->AddRef();
            return S_OK;
        }
        // -----IRawElementProviderFragmentRoot-----

        STDMETHODIMP ElementProviderFromPoint(double x, double y,
            IRawElementProviderFragment** pRetVal) override
        {
            *pRetVal = nullptr;
            if (!m_pContainer)
                return UIA_E_ELEMENTNOTAVAILABLE;
            POINT ptInClient{ (int)x, (int)y };
            ScreenToClient(m_pContainer->HWnd, &ptInClient);
            TPoint pt{ (TCoord)ptInClient.x, (TCoord)ptInClient.y };
            m_pContainer->PixelToLogical(pt);
            const auto pEle = TElement::EtHitTest(m_pContainer->EtLastChild(), pt);
            if (pEle)
                return pEle->UiaQueryInterface(pRetVal);
            return S_OK;
        }
        STDMETHODIMP GetFocus(IRawElementProviderFragment** pRetVal) override
        {
            *pRetVal = nullptr;
            if (!m_pContainer)
                return UIA_E_ELEMENTNOTAVAILABLE;
            const auto pEle = m_pContainer->EleGetFocus();
            if (pEle)
                return pEle->UiaQueryInterface(pRetVal);
            return S_OK;
        }
    };

    class CDropTarget final : public CUnknown<CDropTarget, IDropTarget>
    {
    private:
        CElementContainer* m_pContainer{};
    public:
        EckInlineCe void SetContainer(CElementContainer* p) noexcept { m_pContainer = p; }

        STDMETHODIMP DragEnter(IDataObject* pDataObject,
            DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override
        {
            if (!m_pContainer)
            {
                *pdwEffect = DROPEFFECT_NONE;
                return E_UNEXPECTED;
            }
            return m_pContainer->DragEnter(pDataObject, grfKeyState, pt, pdwEffect);
        }

        STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override
        {
            if (!m_pContainer)
            {
                *pdwEffect = DROPEFFECT_NONE;
                return E_UNEXPECTED;
            }
            return m_pContainer->DragOver(grfKeyState, pt, pdwEffect);
        }

        STDMETHODIMP DragLeave() override
        {
            if (!m_pContainer)
                return E_UNEXPECTED;
            return m_pContainer->DragLeave();
        }

        STDMETHODIMP Drop(IDataObject* pDataObject,
            DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) override
        {
            if (!m_pContainer)
            {
                *pdwEffect = DROPEFFECT_NONE;
                return E_UNEXPECTED;
            }
            return m_pContainer->Drop(pDataObject, grfKeyState, pt, pdwEffect);
        }
    };

    struct TIMER
    {
        TElement* pEle;
        UINT_PTR uId;
    };

    TElement* m_pEleFirstChild{};
    TElement* m_pEleLastChild{};
    TElement* m_pEleFocus{};
    TElement* m_pEleHover{};// WM_MOUSELEAVE使用
    TElement* m_pEleCurrNcHitTest{};
    TElement* m_pEleMouseCapture{};
    TElement* m_pEleDragDrop{};

    ComPtr<CUiaContainer> m_pUia{};

    ComPtr<IDataObject> m_pDataObject{};// DragOver使用
    ComPtr<CDropTarget> m_pDropTarget{};

    std::vector<TIMER> m_vTimer{};

    int m_iUserDpi{ 96 };
    int m_iDpi{ 96 };

    TPoint m_sizeClientLog{};
    int m_cxClient{};
    int m_cyClient{};

    UINT m_UiaId{};

    BITBOOL m_bMouseCaptured : 1{};
    BITBOOL m_bShowFocus : 1{};
    BITBOOL m_bDbgDrawFrame : 1{};
    BITBOOL m_bDbgDrawDirtyRect : 1{};
    BITBOOL m_bEnableOleDragDrop : 1{};
private:
    void ElementDestroying(TElement* pEle) noexcept
    {
        if (m_pEleFocus == pEle)
            m_pEleFocus = nullptr;
        if (m_pEleCurrNcHitTest == pEle)
            m_pEleCurrNcHitTest = nullptr;
        if (m_pEleMouseCapture == pEle)
            EleReleaseCapture();
        if (m_pEleHover == pEle)
            m_pEleHover = nullptr;
        if (m_pEleDragDrop)
            m_pEleDragDrop = nullptr;
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

    EckInlineNdCe UINT UiaGenerateId() noexcept { return m_UiaId++; }

    void UiaMakeInterface() noexcept
    {
        if (!m_pUia.Get())
        {
            m_pUia.Attach(new CUiaContainer{});
            m_pUia->SetContainer(this);
        }
    }

    EckInlineNdCe BOOL UiaIsInitialized() const noexcept { return !!m_pUia.Get(); }
public:
    ECK_CWND_SINGLEOWNER(CElementContainer);
    ECK_CWND_CREATE_CLS_HINST(WCN_DUIHOST, g_hInstance);
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
                m_pEleCurrNcHitTest = TElement::EtHitTest(EtLastChild(), pt);
            const auto pEle = (m_pEleMouseCapture ? m_pEleMouseCapture : m_pEleCurrNcHitTest);
            if (pEle)
            {
                pEle->ClientToElement(pt);
                pEle->EhTransform(pt, FALSE);
                pEle->CallEvent(uMsg, wParam, (LPARAM)&pt);
            }

            if (uMsg == WM_MOUSEMOVE)// 移出监听
            {
                if (m_pEleHover != m_pEleCurrNcHitTest && !m_bMouseCaptured)
                {
                    if (m_pEleHover)
                        m_pEleHover->CallEvent(WM_MOUSELEAVE, 0, 0);
                    m_pEleHover = m_pEleCurrNcHitTest;
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
                m_pEleCurrNcHitTest = TElement::EtHitTest(EtLastChild(), ptLog);
            }
            const auto pEle = (m_pEleMouseCapture ? m_pEleMouseCapture : m_pEleCurrNcHitTest);
            if (pEle)
                pEle->CallEvent(uMsg, wParam, lParam);

            if (uMsg == WM_NCMOUSEMOVE)// 移出监听
            {
                if (m_pEleHover != m_pEleCurrNcHitTest && !m_bMouseCaptured)
                {
                    if (m_pEleHover)
                        m_pEleHover->CallEvent(WM_MOUSELEAVE, 0, 0);
                    m_pEleHover = m_pEleCurrNcHitTest;
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
            if (!m_pEleFocus)
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
                    const auto uDlgCode = m_pEleFocus->CallEvent(
                        WM_GETDLGCODE, wParam, (LPARAM)&Msg);
                    if (uDlgCode & (DLGC_WANTMESSAGE | DLGC_WANTTAB))
                        break;
                    const auto pNextFocus =
                        m_pEleFocus->EtNextTabStop(!(GetKeyState(VK_SHIFT) & 0x8000));
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
            m_pEleFocus->CallEvent(uMsg, wParam, lParam);
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
            if (m_pEleCurrNcHitTest = TElement::EtHitTest(EtLastChild(), ptLog, &lResultNew))
                return lResultNew;
            return lResult;
        }

        case WM_SETCURSOR:
        {
            const auto pEle = (m_pEleMouseCapture ? m_pEleMouseCapture : m_pEleCurrNcHitTest);
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
            if (m_pEleHover)
            {
                m_pEleHover->CallEvent(WM_MOUSELEAVE, 0, 0);
                m_pEleHover = nullptr;
            }
            break;

        case WM_CAPTURECHANGED:
        {
            if (m_bMouseCaptured)
            {
                m_bMouseCaptured = FALSE;
                if (m_pEleMouseCapture)
                {
                    m_pEleMouseCapture->CallEvent(WM_CAPTURECHANGED, 0, NULL);
                    if (m_pEleHover && m_pEleHover != m_pEleMouseCapture)
                    {
                        m_pEleHover->CallEvent(WM_MOUSELEAVE, 0, 0);
                        m_pEleHover = nullptr;
                    }
                    m_pEleMouseCapture = nullptr;
                }
            }
        }
        break;

        case WM_DROPFILES:
        {
            POINT pt;
            if (DragQueryPoint((HDROP)wParam, &pt))// 落点位于客户区中
            {
                ScreenToClient(HWnd, &pt);
                TPoint ptLog{ (TCoord)pt.x, (TCoord)pt.y };
                PixelToLogical(ptLog);
                const auto pEle = TElement::EtHitTest(EtLastChild(), ptLog);
                if (pEle)
                    pEle->CallEvent(EWM_DROP_WIN31, wParam, lParam);
            }
        }
        return 0;

        case WM_GETOBJECT:
        {
            if (lParam == UiaRootObjectId)
            {
                UiaMakeInterface();
                return UiaReturnRawElementProvider(HWnd, wParam, lParam, m_pUia.Get());
            }
        }
        break;

        case WM_DPICHANGED:
            m_iDpi = HIWORD(wParam);
            MsgOnDpiChanged(HWnd, lParam);
            return 0;
        case WM_DPICHANGED_AFTERPARENT:
            m_iDpi = GetDpi(HWnd);
            return 0;

        case WM_CREATE:
        {
            m_iDpi = GetDpi(HWnd);
            RECT rcClient;
            GetClientRect(HWnd, &rcClient);
            m_cxClient = rcClient.right - rcClient.left;
            m_cyClient = rcClient.bottom - rcClient.top;
            UpdateLogicalClientSize();
        }
        break;
        case WM_DESTROY:
        {
            auto pEle = EtFirstChild();
            while (pEle)
            {
                auto pNext = pEle->EtNext();
                pEle->Destroy();
                pEle = pNext;
            }
            m_pEleFirstChild = m_pEleLastChild = nullptr;
            m_pEleFocus = m_pEleHover = m_pEleCurrNcHitTest =
                m_pEleMouseCapture = m_pEleDragDrop = nullptr;

            if (m_pUia.Get())
            {
                m_pUia->SetContainer(nullptr);
                UiaRaiseStructureChangedEvent(
                    m_pUia.Get(),
                    StructureChangeType_ChildRemoved,
                    nullptr, 0);
                m_pUia.Clear();
                UiaReturnRawElementProvider(HWnd, 0, 0, nullptr);
            }

            if (m_pDropTarget.Get())
            {
                m_pDropTarget->SetContainer(nullptr);
                m_pDropTarget.Clear();
            }
        }
        break;
        }
    CallDefault:
        return __super::OnMessage(uMsg, wParam, lParam);
    }

    virtual LRESULT OnElementNotify(TElement* pEle, ELENMHDR* pnm) noexcept { return 0; }

    template<CcpComInterface T>
    EckInline HRESULT UiaQueryInterface(T** p) noexcept
    {
        UiaMakeInterface();
        return m_pUia->QueryInterface(IID_PPV_ARGS(p));
    }
protected:
    EckInlineCe void SetUserDpi(int iDpi) noexcept
    {
        m_iUserDpi = iDpi;
        UpdateLogicalClientSize();
    }
    EckInlineNdCe int GetUserDpi() const noexcept { return m_iUserDpi; }

    EckInlineNdCe int GetWindowDpi() const noexcept { return m_iDpi; }
public:
    template<std::floating_point T>
    EckInlineNdCe T PixelToLogical(T v) const noexcept { return v * (T)96 / m_iUserDpi; }
    template<std::floating_point T>
    EckInlineNdCe T LogicalToPixel(T v) const noexcept { return v * m_iUserDpi / (T)96; }
    template<std::integral T>
    EckInlineNdCe T PixelToLogical(T v) const noexcept
    {
        return T(v * 96.f / m_iUserDpi);
    }
    template<std::integral T>
    EckInlineNdCe T LogicalToPixel(T v) const noexcept
    {
        return T(v * m_iUserDpi / 96.f);
    }
    template<std::integral T>
    EckInlineNd T PixelToLogicalRound(T v) const noexcept
    {
        return (T)roundf(v * 96.f / m_iUserDpi);
    }
    template<std::integral T>
    EckInlineNd T LogicalToPixelRound(T v) const noexcept
    {
        return (T)roundf(v * m_iUserDpi / 96.f);
    }

    EckInlineCe void PixelToLogical(_Inout_ TPoint& pt) const noexcept
    {
        pt.x = PixelToLogical(pt.x);
        pt.y = PixelToLogical(pt.y);
    }
    EckInlineCe void PixelToLogical(_Inout_ CcpRect auto& rc) const noexcept
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
    EckInlineCe void LogicalToPixel(_Inout_ CcpRect auto& rc) const noexcept
    {
        rc.left = LogicalToPixel(rc.left);
        rc.top = LogicalToPixel(rc.top);
        rc.right = LogicalToPixel(rc.right);
        rc.bottom = LogicalToPixel(rc.bottom);
    }
public:
    EckInlineNdCe TElement* EtFirstChild() const noexcept { return m_pEleFirstChild; }
    EckInlineNdCe TElement* EtLastChild() const noexcept { return m_pEleLastChild; }

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
        if (m_pEleFocus == pEle)
            return m_pEleFocus;
        if (pEle && (pEle->GetStyle() & DES_NO_FOCUSABLE))
            return m_pEleFocus;
        auto pOld = m_pEleFocus;
        m_pEleFocus = pEle;
        if (pOld)
            pOld->CallEvent(WM_KILLFOCUS, (WPARAM)pEle, 0);
        if (pEle)
            pEle->CallEvent(WM_SETFOCUS, (WPARAM)pOld, 0);
        return pOld;
    }
    EckInlineNdCe TElement* EleGetFocus() const noexcept { return m_pEleFocus; }

    TElement* EleSetCapture(TElement* pEle) noexcept
    {
        auto pOld = m_pEleMouseCapture;
        m_pEleMouseCapture = pEle;
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
        // m_pEleMouseCapture->CallEvent(WM_CAPTURECHANGED, 0, nullptr);
        // m_pEleMouseCapture = nullptr;
    }
    EckInlineNdCe TElement* EleGetCapture() const noexcept { return m_pEleMouseCapture; }

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
public:
    HRESULT GetDropTarget(_Out_ IDropTarget*& p) noexcept
    {
        if (!m_pDropTarget.Get())
        {
            m_pDropTarget.Attach(new CDropTarget{});
            m_pDropTarget->SetContainer(this);
        }
        p = m_pDropTarget.Get();
        p->AddRef();
        return S_OK;
    }

    HRESULT EnableDragDrop(BOOL bEnable) noexcept
    {
        if (!!m_bEnableOleDragDrop == !!bEnable)
            return S_FALSE;
        HRESULT hr;
        if (bEnable)
        {
            ComPtr<IDropTarget> pTarget;
            hr = GetDropTarget(pTarget.RefOf());
            if (FAILED(hr))
                return hr;
            hr = RegisterDragDrop(HWnd, pTarget.Get());
        }
        else
        {
            m_pDropTarget->SetContainer(nullptr);
            m_pDropTarget.Clear();
            hr = RevokeDragDrop(HWnd);
        }
        if (SUCCEEDED(hr))
            m_bEnableOleDragDrop = !!bEnable;
        return hr;
    }
protected:
    virtual HRESULT DragEnter(IDataObject* pDataObject,
        DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) noexcept
    {
        EckAssert(!m_pDataObject.Get() && !m_pEleDragDrop);
        m_pDataObject = pDataObject;

        POINT ptInClient{ pt.x, pt.y };
        ScreenToClient(HWnd, &ptInClient);
        TPoint ptLog{ (TCoord)ptInClient.x, (TCoord)ptInClient.y };
        PixelToLogical(ptLog);

        m_pEleDragDrop = TElement::EtHitTest(EtLastChild(), ptLog);
        if (m_pEleDragDrop)
        {
            DRAGDROPINFO ddi{ pDataObject, grfKeyState, pdwEffect, ptInClient, ptLog };
            return (HRESULT)m_pEleDragDrop->CallEvent(
                EWM_DRAGENTER, (WPARAM)&ddi, 0);
        }
        return S_OK;
    }
    virtual HRESULT DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) noexcept
    {
        POINT ptInClient{ pt.x, pt.y };
        ScreenToClient(HWnd, &ptInClient);
        TPoint ptLog{ (TCoord)ptInClient.x, (TCoord)ptInClient.y };
        PixelToLogical(ptLog);

        const auto pOldElem = m_pEleDragDrop;
        m_pEleDragDrop = TElement::EtHitTest(EtLastChild(), ptLog);;

        DRAGDROPINFO ddi{ m_pDataObject.Get(), grfKeyState, pdwEffect, ptInClient, ptLog };
        if (pOldElem != m_pEleDragDrop)
        {
            if (pOldElem)
                pOldElem->CallEvent(EWM_DRAGLEAVE, 0, 0);
            if (m_pEleDragDrop)
            {
                return (HRESULT)m_pEleDragDrop->CallEvent(
                    EWM_DRAGENTER, (WPARAM)&ddi, 0);
            }
        }
        else if (m_pEleDragDrop)
            return (HRESULT)m_pEleDragDrop->CallEvent(
                EWM_DRAGOVER, (WPARAM)&ddi, 0);
        return S_OK;
    }
    virtual HRESULT DragLeave() noexcept
    {
        m_pDataObject.Clear();
        const auto pEle = m_pEleDragDrop;
        if (pEle)
        {
            m_pEleDragDrop = nullptr;
            return (HRESULT)pEle->CallEvent(EWM_DRAGLEAVE, 0, 0);
        }
        return S_OK;
    }
    virtual HRESULT Drop(IDataObject* pDataObject,
        DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) noexcept
    {
        m_pDataObject.Clear();

        POINT ptInClient{ pt.x, pt.y };
        ScreenToClient(HWnd, &ptInClient);
        TPoint ptLog{ (TCoord)ptInClient.x, (TCoord)ptInClient.y };
        PixelToLogical(ptLog);

        DRAGDROPINFO ddi{ pDataObject, grfKeyState, pdwEffect, ptInClient, ptLog };
        const auto pEle = TElement::EtHitTest(EtLastChild(), ptLog);;
        if (pEle)
        {
            m_pEleDragDrop = nullptr;
            return (HRESULT)pEle->CallEvent(
                EWM_DROP, (WPARAM)&ddi, 0);
        }
        return S_OK;
    }
public:
    EckInlineCe void DbgSetDrawFrame(BOOL b) noexcept { m_bDbgDrawFrame = b; }
    EckInlineNdCe BOOL DbgGetDrawFrame() const noexcept { return m_bDbgDrawFrame; }
    EckInlineCe void DbgSetDrawDirtyRect(BOOL b) noexcept { m_bDbgDrawDirtyRect = b; }
    EckInlineNdCe BOOL DbgGetDrawDirtyRect() const noexcept { return m_bDbgDrawDirtyRect; }
};

/*
* 实现了以下事件（除非特别说明，所有坐标均为逻辑坐标）
* 未标注参数类型的原样转发Win32消息
*
* WM_MOUSEFIRST  ~ WM_MOUSELAST         (MkFlags, TPoint*     ) InElement
* WM_MOUSELEAVE                         (0      , 0           )
* WM_NCMOUSEMOVE ~ WM_NCXBUTTONDBLCLK
* WM_KEYFIRST    ~ WM_IME_KEYLAST       (Vk     , KeyDownFlags)
* WM_NCHITTEST                          (0      , TPoint      ) InClient
* WM_SETCURSOR
* WM_TIMER                              (Id     , 0           )
* WM_CAPTURECHANGED                     (0      , pEleNew     )
* WM_SETFOCUS                           (pEleOld, 0           )
* WM_KILLFOCUS                          (pEleNew, 0           )
* WM_GETDLGCODE                         (Vk     , MSG*        )
* WM_NOTIFY                             (pEle   , ELENMHDR*   )
*
* 若某类派生自此类，则必须有下列方法
* Destroy
*
* 且必须实现下列事件
* WM_STYLECHANGED                       (uOld   , 0           )
* WM_CREATE                             参数由派生类定义
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
public:
    class CUiaElement : public CUnknown<CUiaElement,
        IRawElementProviderSimple,
        IRawElementProviderFragment>
    {
    protected:
        CElement* m_pEle{};
    public:
        virtual ~CUiaElement() = default;
        void SetElement(CElement* p) noexcept
        {
            if (!p)
                UiaDisconnectProvider_I(this);
            m_pEle = p;
        }
        // -----IRawElementProviderSimple-----

        STDMETHODIMP get_ProviderOptions(ProviderOptions* pRetVal) override
        {
            *pRetVal = ProviderOptions_ServerSideProvider | ProviderOptions_UseComThreading;
            return S_OK;
        }
        STDMETHODIMP GetPatternProvider(PATTERNID idPattern, IUnknown** pRetVal) override
        {
            *pRetVal = nullptr;
            return S_OK;
        }
        STDMETHODIMP GetPropertyValue(PROPERTYID idProp, VARIANT* pRetVal) override
        {
            if (!m_pEle)
            {
                pRetVal->vt = VT_EMPTY;
                return UIA_E_ELEMENTNOTAVAILABLE;
            }
            switch (idProp)
            {
            case UIA_ControlTypePropertyId:
                pRetVal->vt = VT_I4;
                pRetVal->intVal = UIA_CustomControlTypeId;
                break;
            case UIA_IsEnabledPropertyId:
                pRetVal->vt = VT_BOOL;
                pRetVal->boolVal = (m_pEle->GetStyle() & DES_DISABLE) ?
                    VARIANT_FALSE : VARIANT_TRUE;
                break;
            case UIA_IsKeyboardFocusablePropertyId:
                pRetVal->vt = VT_BOOL;
                pRetVal->boolVal = (m_pEle->GetStyle() & DES_NO_FOCUSABLE) ?
                    VARIANT_FALSE : VARIANT_TRUE;
                break;
            default:
                pRetVal->vt = VT_EMPTY;
                break;
            }
            return S_OK;
        }
        STDMETHODIMP get_HostRawElementProvider(IRawElementProviderSimple** pRetVal) override
        {
            *pRetVal = nullptr;
            return S_OK;
        }
        // -----IRawElementProviderFragment-----

        STDMETHODIMP Navigate(NavigateDirection eDirection,
            IRawElementProviderFragment** pRetVal) override
        {
            *pRetVal = nullptr;
            if (!m_pEle)
                return UIA_E_ELEMENTNOTAVAILABLE;
            THost* pEle;
            switch (eDirection)
            {
            case NavigateDirection_Parent:
            {
                pEle = m_pEle->EtParent();
                if (!pEle)
                    return m_pEle->GetContainer()->UiaQueryInterface(pRetVal);
            }
            break;
            case NavigateDirection_NextSibling:     pEle = m_pEle->EtNext();       break;
            case NavigateDirection_PreviousSibling: pEle = m_pEle->EtPrevious();   break;
            case NavigateDirection_FirstChild:      pEle = m_pEle->EtFirstChild(); break;
            case NavigateDirection_LastChild:       pEle = m_pEle->EtLastChild();     break;
            default:                                pEle = nullptr;
            }
            if (pEle)
                return pEle->UiaQueryInterface(pRetVal);
            return S_OK;
        }

        STDMETHODIMP GetRuntimeId(SAFEARRAY** pRetVal) override
        {
            const auto psa = SafeArrayCreateVector(VT_I4, 0, 2);
            *pRetVal = psa;
            if (!psa)
                return E_OUTOFMEMORY;
            int iId{ UiaAppendRuntimeId };
            LONG i{};
            (void)SafeArrayPutElement(psa, &i, (void*)&iId);
            iId = m_pEle->UiaGetRuntimeId();
            i = 1;
            (void)SafeArrayPutElement(psa, &i, (void*)&iId);
            return S_OK;
        }
        STDMETHODIMP get_BoundingRectangle(UiaRect* pRetVal) override
        {
            if (!m_pEle)
            {
                *pRetVal = {};
                return UIA_E_ELEMENTNOTAVAILABLE;
            }
            TRect rc{ m_pEle->GetRectInClient() };
            m_pEle->GetContainer()->LogicalToPixel(rc);
            RECT rcInScr{ (int)rc.left, (int)rc.top, (int)rc.right, (int)rc.bottom };
            ClientToScreen(m_pEle->GetContainer()->HWnd, &rcInScr);
            pRetVal->left = (double)rcInScr.left;
            pRetVal->top = (double)rcInScr.top;
            pRetVal->width = double(rcInScr.right - rcInScr.left);
            pRetVal->height = double(rcInScr.bottom - rcInScr.top);
            return S_OK;
        }
        STDMETHODIMP GetEmbeddedFragmentRoots(SAFEARRAY** pRetVal) override
        {
            *pRetVal = nullptr;
            return S_OK;
        }
        STDMETHODIMP SetFocus() override
        {
            if (!m_pEle)
                return UIA_E_ELEMENTNOTAVAILABLE;
            m_pEle->GetContainer()->EleSetFocus((THost*)m_pEle);
            return S_OK;
        }
        STDMETHODIMP get_FragmentRoot(IRawElementProviderFragmentRoot** pRetVal) override
        {
            if (!m_pEle)
            {
                *pRetVal = nullptr;
                return UIA_E_ELEMENTNOTAVAILABLE;
            }
            return m_pEle->GetContainer()->UiaQueryInterface(pRetVal);
        }
    };
private:
    THost* m_pNext{};    // 下一元素，Z序高于当前
    THost* m_pPrev{};    // 上一元素，Z序低于当前
    THost* m_pParent{};
    THost* m_pEleFirstChild{};
    THost* m_pEleLastChild{};
    CElementContainer<THost>* m_pContainer{};

    CEventChain<Intercept_T, LRESULT, UINT, WPARAM, LPARAM> m_ec{};

    ComPtr<CUiaElement> m_pUia{};

    TRect m_rc{};           // 相对父元素
    TPoint m_ptInClient{};  // 相对于客户区的坐标

    UINT m_uStyle{};
    UINT m_UiaId{};

    int m_iId{};
    UINT m_uTmState{ SaNormal };
public:
    using HSlot = decltype(m_ec)::HSlot;
private:
    constexpr void CorrectChildrenOffsetInClient() const noexcept
    {
        auto pEle = EtFirstChild();
        while (pEle)
        {
            pEle->m_ptInClient =
            {
                pEle->m_rc.left + m_ptInClient.x,
                pEle->m_rc.top + m_ptInClient.y
            };
            pEle->CorrectChildrenOffsetInClient();
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

    virtual LRESULT OnNotify(ELENMHDR* pnm) noexcept { return 0; }

    virtual BOOL EhTransform(_Inout_ TPoint& pt, BOOL bInClient) noexcept { return FALSE; }

    virtual HRESULT EhUiaMakeInterface() noexcept
    {
        const auto p = new CUiaElement{};
        UiaSetInterface(p);
        p->Release();
        return S_OK;
    }

    EckInline void UiaSetInterface(CUiaElement* p) noexcept
    {
        if (m_pUia.Get())
            m_pUia->SetElement(nullptr);
        m_pUia = p;
        m_pUia->SetElement(this);
    }
    template<CcpComInterface T>
    EckInline HRESULT UiaQueryInterface(T** p) noexcept
    {
        if (!m_pUia.Get())
            EhUiaMakeInterface();
        return m_pUia->QueryInterface(IID_PPV_ARGS(p));
    }

    EckInlineNdCe int UiaGetRuntimeId() const noexcept { return (int)m_UiaId; }

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
            pParent = pParent->EtParent();
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

    LRESULT SendNotify(ELENMHDR* pnm) noexcept
    {
        LRESULT lResult;
        lResult = OnNotify(pnm);
        if (pnm->bProcessed)
            return lResult;
        if (GetStyle() & DES_NOTIFY_WND)
        {
            lResult = GetContainer()->OnElementNotify((THost*)this, pnm);
            if (pnm->bProcessed)
                return lResult;
        }
        if ((GetStyle() & DES_NOTIFY_PARENT) && EtParent())
        {
            lResult = EtParent()->CallEvent(WM_NOTIFY, (WPARAM)this, (LPARAM)pnm);
            if (pnm->bProcessed)
                return lResult;
        }
        return 0;
    }

    EckInlineNdCe auto& GetEventChain() noexcept { return m_ec; }
protected:
    // 函数返回后，由于派生类还未初始化，当前对象状态无效
    // 派生类执行初始化使对象有效后产生创建事件，然后调用PostCreate
    void PreCreate(
        const TRect& rc, UINT uStyle,
        THost* pParent,
        CElementContainer<THost>* pContainer) noexcept
    {
        m_pParent = pParent;
        m_pContainer = pContainer;
        SetRect(rc);
        SetStyle(uStyle);
        m_UiaId = pContainer->UiaGenerateId();

        auto& pParentLastChild =
            (pParent ? pParent->m_pEleLastChild : pContainer->m_pEleLastChild);
        auto& pParentFirstChild =
            (pParent ? pParent->m_pEleFirstChild : pContainer->m_pEleFirstChild);
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
    // 函数调用时对象完全初始化完毕
    void PostCreate() noexcept
    {
        if (GetContainer()->UiaIsInitialized() &&
            UiaClientsAreListening())
        {
            EhUiaMakeInterface();
            UiaRaiseStructureChangedEvent(
                m_pUia.Get(),
                StructureChangeType_ChildAdded,
                nullptr, 0);
        }
    }

    // 函数返回后当前对象仍然有效，派生类产生销毁事件，然后调用PostDestroy
    void PreDestroy() noexcept
    {
        auto pChild = EtFirstChild();
        while (pChild)
        {
            auto pNext = pChild->EtNext();
            pChild->Destroy();
            pChild = pNext;
        }
        m_pContainer->ElementDestroying((THost*)this);
        if (m_pUia.Get())
        {
            UiaRaiseStructureChangedEvent(
                m_pUia.Get(),
                StructureChangeType_ChildRemoved,
                nullptr, 0);
            m_pUia->SetElement(nullptr);
            m_pUia.Clear();
        }
    }
    // 函数返回后对象无效
    void PostDestroy() noexcept
    {
        if (m_pPrev)
            m_pPrev->m_pNext = m_pNext;
        else
        {
            if (m_pParent)
                m_pParent->m_pEleFirstChild = m_pNext;
            else
                m_pContainer->m_pEleFirstChild = m_pNext;
        }

        if (m_pNext)
            m_pNext->m_pPrev = m_pPrev;
        else
        {
            if (m_pParent)
                m_pParent->m_pEleLastChild = m_pPrev;
            else
                m_pContainer->m_pEleLastChild = m_pPrev;
        }

        m_pNext = nullptr;
        m_pPrev = nullptr;
        m_pParent = nullptr;
        m_pEleFirstChild = nullptr;
        m_pEleLastChild = nullptr;
        m_pContainer = nullptr;

        m_rc = {};
        m_ptInClient = {};

        m_iId = 0;
        m_uTmState = SaNormal;
    }
protected:
    void SetStyle(UINT uStyle) noexcept
    {
        m_uStyle = uStyle;
        if ((m_uStyle & DES_NO_FOCUSABLE) &&
            GetContainer()->EleGetFocus() == this)
            GetContainer()->EleSetFocus(nullptr);
    }
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
        CorrectChildrenOffsetInClient();
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
        CorrectChildrenOffsetInClient();
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
    EckInlineNdCe auto& GetOffsetInClient() const noexcept { return m_ptInClient; }

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
    EckInlineNdCe THost* EtFirstChild() const noexcept { return m_pEleFirstChild; }
    EckInlineNdCe THost* EtLastChild() const noexcept { return m_pEleLastChild; }
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
    static THost* EtpNextElement(CElement* pCurr, BOOL bNextOrPrev) noexcept
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
        constexpr UINT Style = DES_VISIBLE | DES_TABSTOP;
        constexpr UINT StyleExclude = DES_DISABLE | DES_NO_FOCUSABLE;
        auto pEle = EtpNextElement(this, bNextOrPrev);
        while (pEle != this)
        {
            const auto uStyle = pEle->GetStyle();
            if ((uStyle & (Style | StyleExclude)) == Style)
                return pEle;
            pEle = EtpNextElement(pEle, bNextOrPrev);
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

    void EtForEachElem(std::invocable<THost*> auto&& Fn, THost* pElemBegin) noexcept
    {
        auto p{ pElemBegin };
        while (p)
        {
            EckCanCallbackContinue(Fn(p))
                return;
            EtForEachElem(Fn, p->EtFirstChild());
            p = p->EtNext();
        }
    }
protected:
    void SetZOrder(THost* pEleAfter) noexcept
    {
        auto& pParentLastChild = (m_pParent ?
            m_pParent->m_pEleLastChild :
            m_pContainer->m_pEleLastChild);
        auto& pParentFirstChild = (m_pParent ?
            m_pParent->m_pEleFirstChild :
            m_pContainer->m_pEleFirstChild);

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
public:
    EckInlineCe void PixelToLogical(_Inout_ TPoint& pt) const noexcept { GetContainer()->PixelToLogical(pt); }
    EckInlineCe void PixelToLogical(_Inout_ CcpRect auto& rc) const noexcept { GetContainer()->PixelToLogical(rc); }
    EckInlineCe void LogicalToPixel(_Inout_ TPoint& pt) const noexcept { GetContainer()->LogicalToPixel(pt); }
    EckInlineCe void LogicalToPixel(_Inout_ CcpRect auto& rc) const noexcept { GetContainer()->LogicalToPixel(rc); }

    EckInlineNd auto PixelToLogical(CcpNumber auto v) const noexcept { return GetContainer()->PixelToLogical(v); }
    EckInlineNd auto LogicalToPixel(CcpNumber auto v) const noexcept { return GetContainer()->LogicalToPixel(v); }
public:
    EckInlineCe void SetId(int iId) noexcept { m_iId = iId; }
    EckInlineNdCe int GetId() const noexcept { return m_iId; }
protected:
    // 返回状态是否改变
    BOOL TmOnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept
    {
        switch (uMsg)
        {
        case WM_MOUSEMOVE:
            if (m_uTmState & SaHot)
                break;
            m_uTmState |= SaHot;
            return TRUE;
        case WM_MOUSELEAVE:
            if (!(m_uTmState & SaHot))
                break;
            m_uTmState &= ~SaHot;
            return TRUE;
        case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
            m_uTmState |= (SaActive | SapLButtonDown);
            GetContainer()->EleSetCapture((THost*)this);
            return TRUE;
        case WM_LBUTTONUP:
            if (m_uTmState & SapLButtonDown)
                GetContainer()->EleReleaseCapture();
            return TRUE;
        case WM_CAPTURECHANGED:
            if (m_uTmState & SapLButtonDown)
            {
                m_uTmState &= ~(SaActive | SapLButtonDown);
                return TRUE;
            }
            break;
        case WM_SETFOCUS:
            if (m_uTmState & SaFocus)
                break;
            m_uTmState |= SaFocus;
            return TRUE;
        case WM_KILLFOCUS:
            if (!(m_uTmState & SaFocus))
                break;
            m_uTmState &= ~SaFocus;
            return TRUE;
        case WM_STYLECHANGED:
            if (!(((UINT)wParam ^ GetStyle()) & DES_DISABLE))
                break;
            [[fallthrough]];
        case WM_CREATE:
            if (GetStyle() & DES_DISABLE)
            {
                m_uTmState |= SaDisable;
                return TRUE;
            }
            break;
        }
        return FALSE;
    }

    EckInlineNdCe UINT& TmState() noexcept { return m_uTmState; }
public:
    EckInlineNdCe UINT GetThemeState() const noexcept { return m_uTmState; }
};
ECK_UIBASIC_NAMESPACE_END
ECK_NAMESPACE_END