#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
class CDialog : public CWindow
{
public:
    ECK_RTTI(CDialog, CWindow);
protected:
    INT_PTR m_iResult{};
    HWND m_hTop{};
    COLORREF m_crBkg{ CLR_DEFAULT };

    BITBOOL m_bUseDefBkClr : 1{ TRUE };
#	ifdef _DEBUG
    BITBOOL m_bDlgProcInit : 1{};
#	endif
    BITBOOL m_bModal : 1{};
    BITBOOL m_bClrDisableEdit : 1{};

    EckInline HWND PreModal(HWND hParent) noexcept
    {
        m_bModal = TRUE;
        return GetSafeOwner(hParent, &m_hTop);
    }

    EckInline void PostModal() noexcept
    {
        if (!m_hTop)
            return;
        m_bModal = FALSE;
        EnableWindow(m_hTop, TRUE);
        m_hTop = nullptr;
    }

    void MessageLoop() noexcept
    {
        MSG msg;
        while (GetMessageW(&msg, nullptr, 0, 0))
        {
            if (!eck::PreTranslateMessage(msg))
            {
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
        }
    }

    EckInline INT_PTR IntCreateModalDialog(
        HINSTANCE hInst, PCWSTR pszTemplate, HWND hParent,
        LPARAM lParam = 0, FWndCreating pfnCreatingProc = nullptr) noexcept
    {
        m_bModal = TRUE;

        const HWND hOwner = PreModal(hParent);
        BOOL bNeedEnableOwner;
        if (hOwner && hOwner != GetDesktopWindow() && IsWindowEnabled(hOwner))
        {
            bNeedEnableOwner = TRUE;
            EnableWindow(hOwner, FALSE);
        }
        else
            bNeedEnableOwner = FALSE;

        BeginCbtHook(this, pfnCreatingProc);
        CreateDialogParamW(hInst, pszTemplate, hParent, EckDlgProc, lParam);
        EckAssert(m_bDlgProcInit);

        MessageLoop();
        if (bNeedEnableOwner)
            EnableWindow(hOwner, TRUE);
        if (hParent)
            SetActiveWindow(hParent);
        PostModal();
        Destroy();
        return m_iResult;
    }

    EckInline HWND IntCreateModelessDialog(
        HINSTANCE hInst, PCWSTR pszTemplate, HWND hParent,
        LPARAM lParam = 0, FWndCreating pfnCreatingProc = nullptr) noexcept
    {
        m_bModal = FALSE;
        BeginCbtHook(this, pfnCreatingProc);
        const auto h = CreateDialogParamW(hInst, pszTemplate, hParent, EckDlgProc, lParam);
        EckAssert(m_bDlgProcInit);
        return h;
    }
public:
    static INT_PTR CALLBACK EckDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        const auto pDlg = DbgDynamicCast<CDialog*>(CWndFromHWND(hDlg));
#ifdef _DEBUG
        if (!pDlg)
        {
            EckDbgPrintWithPos(L"** WARNING **  CDialog指针为NULL");
            EckDbgBreak();
            return FALSE;
        }
#endif // _DEBUG
        if (uMsg == WM_INITDIALOG)
            return pDlg->OnInitializeDialog(hDlg, (HWND)wParam, lParam);
        return FALSE;
    }

    LRESULT OnMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept override
    {
        switch (uMsg)
        {
        case WM_NCCREATE:
            m_iResult = 0;
            SetWindowLongPtrW(hWnd, DWLP_DLGPROC, (LONG_PTR)EckDlgProc);
#ifdef _DEBUG
            m_bDlgProcInit = TRUE;
            break;
        case WM_NCDESTROY:
            m_bDlgProcInit = FALSE;
#endif // _DEBUG
            break;
        case WM_DESTROY:
            m_bModal = FALSE;
            break;
        case WM_COMMAND:
        {
            if (HIWORD(wParam) == BN_CLICKED)
                switch (LOWORD(wParam))
                {
                case IDOK:
                    OnOk((HWND)lParam);
                    break;
                case IDCANCEL:
                    OnCancel((HWND)lParam);
                    break;
                }
        }
        break;
        case WM_CTLCOLORSTATIC:
            if ((!m_bClrDisableEdit && CWindow((HWND)lParam).GetWindowClass() == WC_EDITW))
                break;
            [[fallthrough]];
        case WM_CTLCOLORBTN:
        case WM_CTLCOLORDLG:
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORLISTBOX:
        {
            const auto* const ptc = PtcCurrent();
            SetTextColor((HDC)wParam, ptc->crDefText);
            SetBkColor((HDC)wParam, m_crBkg != CLR_DEFAULT ? m_crBkg : ptc->crDefBkg);
            SetDCBrushColor((HDC)wParam, m_crBkg != CLR_DEFAULT ? m_crBkg : ptc->crDefBkg);
            return (LRESULT)GetStockBrush(DC_BRUSH);
        }
        break;
        }

        return __super::OnMessage(hWnd, uMsg, wParam, lParam);
    }

    virtual BOOL OnInitializeDialog(HWND hDlg, HWND hFocus, LPARAM lParam) noexcept { return TRUE; }

    virtual HWND CreateModelessDialog(HWND hParent, void* pData = nullptr) noexcept
    {
        EckDbgPrintWithPos(L"** ERROR ** 未实现方法");
        EckDbgBreak();
        abort();
    }

    virtual INT_PTR CreateModalDialog(HWND hParent, void* pData = nullptr) noexcept
    {
        EckDbgPrintWithPos(L"** ERROR ** 未实现方法");
        EckDbgBreak();
        abort();
    }

    virtual BOOL EndDialog(INT_PTR nResult) noexcept
    {
        m_iResult = nResult;
        if (m_bModal)
        {
            PostQuitMessage(0);
            return TRUE;
        }
        else
            return Destroy();
    }

    virtual void OnOk(HWND hCtrl) noexcept
    {
        EndDialog(0);
    }

    virtual void OnCancel(HWND hCtrl) noexcept
    {
        EndDialog(0);
    }

    EckInline void SetBackgroundColor(COLORREF cr) noexcept { m_crBkg = cr; }
    EckInline void SetAllowColorDisableEdit(BOOL b) noexcept { m_bClrDisableEdit = b; }
    EckInline void SetUseDefaultBackgroundColor(BOOL b) noexcept { m_bUseDefBkClr = b; }
};

enum : UINT
{
    DLGNCF_CENTERSCREEN = (1u << 0),
    DLGNCF_CENTERPARENT = (1u << 1),
};

class CDialogNew : public CDialog
{
public:
    ECK_RTTI(CDialogNew, CDialog);

    constexpr static LONG_PTR OcbPtr = DLGWINDOWEXTRA;
protected:
    INT_PTR IntCreateModalDialog(DWORD dwExStyle, PCWSTR pszClass, PCWSTR pszText, DWORD dwStyle,
        int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, HINSTANCE hInst, void* pParam,
        UINT uDlgFlags = 0u, FWndCreating pfnCreatingProc = nullptr) noexcept
    {
        const HWND hOwner = PreModal(hParent);
        BOOL bNeedEnableOwner;
        if (hOwner && hOwner != GetDesktopWindow() && IsWindowEnabled(hOwner))
        {
            bNeedEnableOwner = TRUE;
            EnableWindow(hOwner, FALSE);
        }
        else
            bNeedEnableOwner = FALSE;

        IntCreateModelessDialog(dwExStyle, pszClass, pszText, dwStyle,
            x, y, cx, cy, hParent, hMenu, hInst, pParam, uDlgFlags, pfnCreatingProc);

        MessageLoop();
        if (bNeedEnableOwner)
            EnableWindow(hOwner, TRUE);
        if (hParent)
            SetActiveWindow(hParent);
        PostModal();
        Destroy();
        return m_iResult;
    }

    HWND IntCreateModelessDialog(DWORD dwExStyle, PCWSTR pszClass, PCWSTR pszText, DWORD dwStyle,
        int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, HINSTANCE hInst, void* pParam,
        UINT uDlgFlags = 0u, FWndCreating pfnCreatingProc = nullptr) noexcept
    {
        POINT pt;
        if (IsBitSet(uDlgFlags, DLGNCF_CENTERPARENT))
            pt = CalculateCenterWindowPosition(hParent, cx, cy);
        else if (IsBitSet(uDlgFlags, DLGNCF_CENTERSCREEN))
            pt = CalculateCenterWindowPosition(nullptr, cx, cy);
        else
            pt = { x,y };

        IntCreate(dwExStyle, pszClass, pszText, dwStyle,
            pt.x, pt.y, cx, cy, hParent, hMenu, hInst, pParam, pfnCreatingProc);
        EckAssert(m_bDlgProcInit);

        if (m_hWnd)
        {
            HWND hFirstCtrl = GetNextDlgTabItem(m_hWnd, nullptr, FALSE);
            if (SendMsg(WM_INITDIALOG, (WPARAM)hFirstCtrl, (LPARAM)pParam))
            {
                hFirstCtrl = GetNextDlgTabItem(m_hWnd, nullptr, FALSE);
                SetFocus(hFirstCtrl);
            }
        }
        return m_hWnd;
    }
};
ECK_NAMESPACE_END