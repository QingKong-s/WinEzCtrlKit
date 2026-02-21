#pragma once
#include "CWnd.h"
#include "LunarDateTimeHelper.h"

ECK_NAMESPACE_BEGIN
class CLunarCalendar :public CWindow
{
public:
    ECK_RTTI(CLunarCalendar, CWindow);
    ECK_CWND_SINGLEOWNER(CLunarCalendar);
    ECK_CWND_CREATE_CLS_HINST(WCN_LUNARCALENDAR, g_hInstance);

    HTHEME m_hTheme{};

    int m_cxCell{};
    int m_cyCell{};

    int m_cxClient{};
    int m_cyClient{};

    RECT m_rcCenter{};

    WCHAR m_szWeek[7][5]{};

    COLORREF m_crText{ CLR_DEFAULT };
    COLORREF m_crBkg{ CLR_DEFAULT };

    BITBOOL m_bShowBorder : 1{ FALSE };

    void UpdateTipText() noexcept
    {
        EckCounter(7, i)
        {
            GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT,
                LOCALE_SSHORTESTDAYNAME1 + i, EckArrAndLen(m_szWeek[i]));
        }
    }

    void Paint(HDC hDC, const RECT& rcPaint) noexcept
    {
        const RECT rcClient{ 0, 0, m_cxClient, m_cyClient };
        if (m_crBkg == CLR_DEFAULT)
            DrawThemeBackground(m_hTheme, hDC, MC_BACKGROUND, 0, &rcClient, nullptr);
        else
        {
            SetDCBrushColor(hDC, m_crBkg);
            FillRect(hDC, &rcClient, GetStockBrush(DC_BRUSH));
        }
        if (m_bShowBorder)
            DrawThemeBackground(m_hTheme, hDC, MC_BORDERS, 0, &rcClient, nullptr);
    }
public:
    LRESULT OnMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept override
    {
        switch (uMsg)
        {
        case WM_SIZE:
        {
            m_cxClient = LOWORD(lParam);
            m_cyClient = HIWORD(lParam);
        }
        break;

        case WM_CREATE:
        {
            m_hTheme = OpenThemeData(hWnd, L"MonthCal");
            UpdateTipText();
        }
        break;

        case WM_PRINTCLIENT:
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hWnd, wParam, ps);
            Paint(ps.hdc, ps.rcPaint);
            EndPaint(hWnd, wParam, ps);
        }
        return 0;
        }

        return __super::OnMessage(hWnd, uMsg, wParam, lParam);
    }
};
ECK_NAMESPACE_END