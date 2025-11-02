#include "pch.h"
#include "CNormalWnd.h"

void CNormalWnd::DmNewDpi(int iDpi)
{
    const int iDpiOld = m_iDpi;
    m_iDpi = iDpi;
    eck::ReCreateAndApplyFontForDpiChanged(HWnd, m_hFont, iDpi, iDpiOld);
    DmUpdateFixedSize();
}

void CNormalWnd::DmUpdateFixedSize()
{
}

void CNormalWnd::OnDestory()
{
    DeleteObject(m_hFont);
    m_hFont = nullptr;
}

LRESULT CNormalWnd::OnCreate(HWND hWnd, CREATESTRUCT* pcs)
{
    m_iDpi = eck::GetDpi(hWnd);
    m_hFont = eck::DftCreate(m_iDpi);

    eck::ApplyWindowFont(hWnd, m_hFont);
    return 0;
}

LRESULT CNormalWnd::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_SIZE:
        break;

    case WM_COMMAND:
        break;

    case WM_CREATE:
        return OnCreate(hWnd, (CREATESTRUCT*)lParam);
    case WM_DESTROY:
        OnDestory();
        return 0;

    case WM_SYSCOLORCHANGE:
        eck::MsgOnSysColorChange(hWnd, wParam, lParam);
        break;
    case WM_DPICHANGED:
        DmNewDpi(HIWORD(wParam));
        eck::MsgOnDpiChanged(hWnd, lParam);
        return 0;
    }
    return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}

BOOL CNormalWnd::PreTranslateMessage(const MSG& Msg)
{
    return __super::PreTranslateMessage(Msg);
}