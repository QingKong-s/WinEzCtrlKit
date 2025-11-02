#include "pch.h"

#include "CWndMain.h"


void CWndMain::OnDestory()
{
}

LRESULT CWndMain::OnCreate()
{
    eck::GetThreadCtx()->UpdateDefaultColor();
    StSwitchStdThemeMode(eck::GetThreadCtx()->bAppDarkMode);
    StUpdateColorizationColor();

    ComPtr<IDWriteTextFormat> pTf;
    eck::DftCreateDWrite(pTf.RefOf());

    const MARGINS Mar{ .cyBottomHeight = 8 };
    {
        m_EDUserName.TxSetProp(TXTBIT_MULTILINE, 0, FALSE);
        m_EDUserName.Create(nullptr, Dui::DES_VISIBLE, 0,
            0, 0, 300, 30, nullptr, this);
        m_EDUserName.SetTextFormat(pTf.Get());
        m_Layout.Add(&m_EDUserName, Mar, eck::LF_FIX | eck::LF_ALIGN_CENTER);

        m_EDPassword.TxSetProp(TXTBIT_MULTILINE, 0, FALSE);
        m_EDPassword.Create(nullptr, Dui::DES_VISIBLE, 0,
            0, 0, 300, 30, nullptr, this);
        m_EDPassword.SetTextFormat(pTf.Get());
        m_Layout.Add(&m_EDPassword, Mar, eck::LF_FIX | eck::LF_ALIGN_CENTER);

        m_BT.Create(L"登录", Dui::DES_VISIBLE, 0,
            0, 0, 70, 30, nullptr, this);
        m_BT.SetTextFormat(pTf.Get());
        m_Layout.Add(&m_BT, Mar, eck::LF_FIX | eck::LF_ALIGN_CENTER);
    }
    return 0;
}

LRESULT CWndMain::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_SIZE:
    {
        const auto lResult = __super::OnMsg(hWnd, uMsg, wParam, lParam);
        m_Layout.Arrange(0, 70, (int)GetClientWidthLog(), (int)GetClientHeightLog());
        return lResult;
    }

    case WM_CREATE:
    {
        const auto lResult = __super::OnMsg(hWnd, uMsg, wParam, lParam);
        OnCreate();
        return lResult;
    }
    case WM_DESTROY:
    {
        const auto lResult = __super::OnMsg(hWnd, uMsg, wParam, lParam);
        OnDestory();
        PostQuitMessage(0);
        return lResult;
    }

    case WM_SYSCOLORCHANGE:
        eck::MsgOnSysColorChangeMainWnd(hWnd, wParam, lParam);
        break;
    case WM_SETTINGCHANGE:
        if (eck::MsgOnSettingChangeMainWnd(hWnd, wParam, lParam))
        {
            StSwitchStdThemeMode(eck::GetThreadCtx()->bAppDarkMode);
            Redraw();
        }
        break;
    case WM_DPICHANGED:
        SetUserDpi(LOWORD(wParam));
        break;
    case WM_DWMCOLORIZATIONCOLORCHANGED:
        StUpdateColorizationColor();
        Redraw();
        break;
    }
    return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}