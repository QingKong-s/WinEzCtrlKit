#include "pch.h"

#include "CWndMain.h"

void CWndMain::DmNewDpi(int iDpi)
{
    const int iDpiOld = m_iDpi;
    m_iDpi = iDpi;
    m_Layout.LoOnDpiChanged(iDpi);
    eck::ReCreateAndApplyFontForDpiChanged(HWnd, m_hFont, iDpi, iDpiOld);
    DmUpdateFixedSize();
}

void CWndMain::DmUpdateFixedSize()
{
}

void CWndMain::OnDestory()
{
    DeleteObject(m_hFont);
    m_hFont = nullptr;
}

LRESULT CWndMain::OnCreate(HWND hWnd, CREATESTRUCT* pcs)
{
    eck::GetThreadCtx()->UpdateDefaultColor();

    m_iDpi = eck::GetDpi(hWnd);
    m_hFont = eck::DftCreate(m_iDpi);

    constexpr DWORD Style = WS_CHILD | WS_VISIBLE | WS_TABSTOP;
    constexpr DWORD GroupStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_GROUP;

    const eck::LYTMARGINS Mar{ .b = DpiScale((eck::TLytCoord)8, m_iDpi) };
    const auto cy = DpiScale(30, m_iDpi);
    const auto cxED = DpiScale(300, m_iDpi);
    {
        m_EDUserName.Create(nullptr, GroupStyle, WS_EX_CLIENTEDGE,
            0, 0, cxED, cy, hWnd, 0);
        m_EDUserName.SetCueBanner(L"用户名");
        m_Layout.Add(&m_EDUserName, Mar, eck::LF_FIX | eck::LF_ALIGN_CENTER);

        m_EDPassword.Create(nullptr, Style, WS_EX_CLIENTEDGE,
            0, 0, cxED, cy, hWnd, 0);
        m_EDPassword.SetCueBanner(L"密码");
        m_Layout.Add(&m_EDPassword, Mar, eck::LF_FIX | eck::LF_ALIGN_CENTER);

        m_BT.Create(L"登录", GroupStyle | BS_PUSHBUTTON, 0,
            0, 0, DpiScale(70, m_iDpi), cy, hWnd, 0);
        m_Layout.Add(&m_BT, Mar, eck::LF_FIX | eck::LF_ALIGN_CENTER);
    }
    m_Layout.LoInitializeDpi(m_iDpi);

    eck::ApplyWindowFont(hWnd, m_hFont);
    return 0;
}

LRESULT CWndMain::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_SIZE:
        m_Layout.Arrange(
            0,
            eck::DpiScale((eck::TLytCoord)70, m_iDpi),
            (eck::TLytCoord)LOWORD(lParam),
            (eck::TLytCoord)HIWORD(lParam));
        break;

    case WM_COMMAND:
    {
        switch (HIWORD(wParam))
        {
        case BN_CLICKED:
            if ((LPARAM)m_BT.HWnd == lParam)
                MessageBoxW(hWnd, L"Hello World!", L"Hello", MB_OK);
            return 0;
        }
    }
    break;

    case WM_CREATE:
        return OnCreate(hWnd, (CREATESTRUCT*)lParam);
    case WM_DESTROY:
        OnDestory();
        PostQuitMessage(0);
        return 0;

    case WM_SYSCOLORCHANGE:
        eck::MsgOnSysColorChangeMainWnd(hWnd, wParam, lParam);
        break;
    case WM_SETTINGCHANGE:
        eck::MsgOnSettingChangeMainWnd(hWnd, wParam, lParam);
        break;
    case WM_DPICHANGED:
        DmNewDpi(HIWORD(wParam));
        eck::MsgOnDpiChanged(hWnd, lParam);
        return 0;
    }
    return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}

BOOL CWndMain::PreTranslateMessage(const MSG& Msg)
{
    if (IsDialogMessageW(HWnd, (MSG*)&Msg))
        return TRUE;
    return __super::PreTranslateMessage(Msg);
}