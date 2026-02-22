#pragma once
#include "CWindow.h"

ECK_NAMESPACE_BEGIN
class CTrayIcon
{
public:
    const inline static UINT MessageTray = RegisterWindowMessageW(MSGREG_TRAY);
    const inline static UINT MessageTaskbarCreated = RegisterWindowMessageW(L"TaskbarCreated");

    struct MsgParam
    {
        USHORT uEvent;
        USHORT uId;
        short x;
        short y;

        constexpr MsgParam(WPARAM wParam, LPARAM lParam) noexcept :
            uEvent{ LOWORD(lParam) },
            uId{ HIWORD(lParam) },
            x{ GET_X_LPARAM(wParam) },
            y{ GET_Y_LPARAM(wParam) }
        {}
    };
private:
    struct ITEM
    {
        USHORT uId;
        BOOLEAN bUseStdTip;
        DWORD dwState;
        HICON hIcon;
        CStringW rsTip;
    };

    std::vector<ITEM> m_vItem{};
    CWindow* m_pWnd{};
    CWindow::HSlot m_hSlot{};

    EckInlineNd BOOL Add(NOTIFYICONDATAW& nid) noexcept
    {
        EckAssert(nid.uVersion == NOTIFYICON_VERSION_4);
        if (!Shell_NotifyIconW(NIM_ADD, &nid))
            return FALSE;
        if (!Shell_NotifyIconW(NIM_SETVERSION, &nid))
        {
            Shell_NotifyIconW(NIM_DELETE, &nid);
            return FALSE;
        }
        return TRUE;
    }

    LRESULT OnMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, SlotCtx& Ctx) noexcept
    {
        if (uMsg == WM_DESTROY)
        {
            DeleteAll();
            Detach();
        }
        else if (uMsg == MessageTaskbarCreated)
        {
            NOTIFYICONDATAW nid;
            nid.cbSize = sizeof(nid);
            nid.hWnd = hWnd;
            nid.uCallbackMessage = MessageTray;
            for (auto it = m_vItem.begin(); it != m_vItem.end(); )
            {
                const auto& e = *it;
                nid.uID = e.uId;
                nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP | NIF_STATE |
                    (e.bUseStdTip ? NIF_SHOWTIP : 0);
                nid.dwState = nid.dwStateMask = e.dwState;
                nid.hIcon = e.hIcon;
                nid.uVersion = NOTIFYICON_VERSION_4;
                if (e.rsTip.IsEmpty())
                    nid.szTip[0] = L'\0';
                else
                    TcsCopyLengthEnd(nid.szTip, e.rsTip.Data(),
                        std::min(e.rsTip.Size(), int(ARRAYSIZE(nid.szTip) - 1)));
                if (Add(nid))
                    ++it;
                else
                    it = m_vItem.erase(it);
            }
        }
        return 0;
    }
public:
    ECK_DISABLE_COPY_DEF_CONS(CTrayIcon);
    ~CTrayIcon() noexcept
    {
        DeleteAll();
        Detach();
    }

    void Attach(CWindow* pWnd) noexcept
    {
        Detach();
        m_pWnd = pWnd;
        m_hSlot = m_pWnd->GetEventChain().Connect(this, &CTrayIcon::OnMessage);
    }
    void Detach() noexcept
    {
        if (m_pWnd)
        {
            m_pWnd->GetEventChain().Disconnect(m_hSlot);
            m_hSlot = nullptr;
            m_pWnd = nullptr;
            m_vItem.clear();
        }
    }

    // bUseStdTip指示是否使用标准工具提示，若为TRUE则设置NIF_SHOWTIP
    BOOL Add(
        USHORT uId,
        HICON hIcon,
        std::wstring_view svTip = {},
        DWORD dwState = 0u,
        BOOL bUseStdTip = TRUE) noexcept
    {
        NOTIFYICONDATAW nid;
        nid.cbSize = sizeof(nid);
        nid.hWnd = m_pWnd->HWnd;
        nid.uID = uId;
        nid.uFlags = NIF_MESSAGE | NIF_STATE | (bUseStdTip ? NIF_SHOWTIP : 0);
        nid.uCallbackMessage = MessageTray;
        nid.dwState = nid.dwStateMask = dwState;
        nid.uVersion = NOTIFYICON_VERSION_4;
        if (hIcon)
        {
            nid.uFlags |= NIF_ICON;
            nid.hIcon = hIcon;
        }
        if (!svTip.empty())
        {
            nid.uFlags |= NIF_TIP;
            TcsCopyLengthEnd(nid.szTip, svTip.data(),
                std::min(svTip.size(), ARRAYSIZE(nid.szTip) - 1));
        }
        if (!Add(nid))
            return FALSE;
        m_vItem.emplace_back(uId, bUseStdTip, dwState, hIcon, svTip);
        return TRUE;
    }

    // 仅处理NIF_ICON、NIF_TIP、NIF_STATE，其余标志被忽略
    BOOL Modify(
        USHORT uId,
        UINT uFlags,
        HICON hIcon = nullptr,
        std::wstring_view svTip = {},
        DWORD dwState = 0u,
        BOOL bUseStdTip = TRUE) noexcept
    {
        const auto it = std::find_if(m_vItem.begin(), m_vItem.end(),
            [uId](const ITEM& x) { return x.uId == uId; });
        if (it == m_vItem.end())
        {
            EckDbgBreak();
            return FALSE;
        }
        uFlags &= (NIF_ICON | NIF_TIP | NIF_STATE);
        if (bUseStdTip)
            uFlags |= NIF_SHOWTIP;

        NOTIFYICONDATAW nid;
        nid.cbSize = sizeof(nid);
        nid.hWnd = m_pWnd->HWnd;
        nid.uID = uId;
        nid.uFlags = uFlags;

        if (uFlags & NIF_ICON)
            nid.hIcon = hIcon;

        if (uFlags & NIF_TIP)
        {
            TcsCopyLengthEnd(nid.szTip, svTip.data(),
                std::min(svTip.size(), ARRAYSIZE(nid.szTip) - 1));
        }

        if (uFlags & NIF_STATE)
            nid.dwState = nid.dwStateMask = dwState;

        if (!Shell_NotifyIconW(NIM_MODIFY, &nid))
            return FALSE;

        it->bUseStdTip = bUseStdTip;
        if (uFlags & NIF_ICON)
            it->hIcon = hIcon;
        if (uFlags & NIF_TIP)
            it->rsTip = svTip;
        if (uFlags & NIF_STATE)
            it->dwState = dwState;
        return TRUE;
    }

    BOOL Delete(USHORT uId) noexcept
    {
        const auto it = std::find_if(m_vItem.begin(), m_vItem.end(),
            [uId](const ITEM& x) { return x.uId == uId; });
        if (it == m_vItem.end())
        {
            EckDbgBreak();
            return FALSE;
        }
        m_vItem.erase(it);
        NOTIFYICONDATAW nid;
        nid.cbSize = sizeof(nid);
        nid.hWnd = m_pWnd->HWnd;
        nid.uID = uId;
        nid.uFlags = 0;
        return Shell_NotifyIconW(NIM_DELETE, &nid);
    }

    void DeleteAll() noexcept
    {
        NOTIFYICONDATAW nid;
        nid.cbSize = sizeof(nid);
        nid.uFlags = 0;
        for (const auto& e : m_vItem)
        {
            nid.hWnd = m_pWnd->HWnd;
            nid.uID = e.uId;
            Shell_NotifyIconW(NIM_DELETE, &nid);
        }
        m_vItem.clear();
    }

    BOOL SetFocus(USHORT uId) noexcept
    {
        NOTIFYICONDATAW nid;
        nid.cbSize = sizeof(nid);
        nid.hWnd = m_pWnd->HWnd;
        nid.uID = uId;
        nid.uFlags = 0;
        return Shell_NotifyIconW(NIM_SETFOCUS, &nid);
    }

    BOOL PopBalloon(
        USHORT uId,
        std::wstring_view svContent,
        std::wstring_view svTitle = {},
        DWORD dwInfoFlags = 0u,
        HICON hBalloonIcon = nullptr,
        BOOL bRealTime = FALSE) noexcept
    {
        NOTIFYICONDATAW nid;
        nid.cbSize = sizeof(nid);
        nid.hWnd = m_pWnd->HWnd;
        nid.uID = uId;
        nid.uFlags = NIF_INFO | (bRealTime ? NIF_REALTIME : 0);

        if (!svContent.empty())
            TcsCopyLengthEnd(nid.szInfo, svContent.data(),
                std::min(svContent.size(), ARRAYSIZE(nid.szInfo) - 1));
        else
            nid.szInfo[0] = L'\0';

        if (!svTitle.empty())
            TcsCopyLengthEnd(nid.szInfoTitle, svTitle.data(),
                std::min(svTitle.size(), ARRAYSIZE(nid.szInfoTitle) - 1));
        else
            nid.szInfoTitle[0] = L'\0';

        nid.hBalloonIcon = hBalloonIcon;
        nid.dwInfoFlags = dwInfoFlags;
        return Shell_NotifyIconW(NIM_MODIFY, &nid);
    }

    BOOL DismissBalloon(USHORT uId) noexcept
    {
        NOTIFYICONDATAW nid;
        nid.cbSize = sizeof(nid);
        nid.hWnd = m_pWnd->HWnd;
        nid.uID = uId;
        nid.uFlags = NIF_INFO;
        nid.szInfo[0] = L'\0';
        nid.szInfoTitle[0] = L'\0';
        nid.hBalloonIcon = nullptr;
        nid.dwInfoFlags = 0;
        return Shell_NotifyIconW(NIM_MODIFY, &nid);
    }
};
ECK_NAMESPACE_END