#pragma once
#include "CWindow.h"

ECK_NAMESPACE_BEGIN
class CMessageBoxHook : public CWindow
{
private:
    RECT m_rcMainPanel{};
    RECT m_rcCommandEdge{};
    HICON m_hIcon{};

    void UpdateMetrics(int iDpi) noexcept
    {
        const auto hStatic = GetDlgItem(HWnd, 0xFFFF);
        const auto hStaticIcon = GetDlgItem(HWnd, 0x14);
        // 内部测高机制过于复杂，这里使用静态控件的高度
        RECT rcTemp;
        GetClientRect(hStatic, &rcTemp);
        const auto cyText = rcTemp.bottom - DpiScale(2, iDpi);
        if (hStaticIcon)
            GetClientRect(hStaticIcon, &rcTemp);
        else
            rcTemp.bottom = 0;
        const auto cyIcon = rcTemp.bottom;
        // 其字符高度来源为GdiGetCharDimensions(Ex)，
        // 该函数简单地将字符高度设为tmHeight
        const auto hCDC = CreateCompatibleDC(nullptr);
        SelectObject(hCDC, (HGDIOBJ)SendMessageW(hStatic, WM_GETFONT, 0, 0));
        TEXTMETRICW tm;
        GetTextMetricsW(hCDC, &tm);
        DeleteDC(hCDC);
        // User32的计算方法，不应仿照其底边计算方式，
        // 因为其得出的数值不准确，实际上超过了客户区高度
        const int cyTextMargin = (14 * tm.tmHeight + 4) >> 3;
        GetClientRect(HWnd, &m_rcMainPanel);
        m_rcCommandEdge = m_rcMainPanel;
        m_rcMainPanel.bottom = std::max(cyText, cyIcon) + cyTextMargin * 2;
        m_rcCommandEdge.top = m_rcMainPanel.bottom;
    }
public:
    LRESULT OnMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept override
    {
        switch (uMsg)
        {
        case WM_PAINT:
        {
            if (!ShouldAppsUseDarkMode())
                break;
            const auto* const ptc = PtcCurrent();
            PAINTSTRUCT ps;
            BeginPaint(hWnd, &ps);
            SetDCBrushColor(ps.hdc, ptc->crDefBkg);
            FillRect(ps.hdc, &m_rcMainPanel, GetStockBrush(DC_BRUSH));
            SetDCBrushColor(ps.hdc, ptc->crDefBtnFace);
            FillRect(ps.hdc, &m_rcCommandEdge, GetStockBrush(DC_BRUSH));
            EndPaint(hWnd, &ps);
        }
        return 0;

        case WM_INITDIALOG:
        {
            // Call默认过程，先执行初始化
            const auto lResult = __super::OnMessage(hWnd, uMsg, wParam, lParam);
            if (HWND hStaticIcon; hStaticIcon = GetDlgItem(hWnd, 0x14))
            {
                m_hIcon = (HICON)SendMessageW(hStaticIcon, STM_GETICON, 0, 0);
                // OD修正图标白底
                SetWindowLongPtrW(hStaticIcon, GWL_STYLE,
                    (GetWindowLongPtrW(hStaticIcon, GWL_STYLE) & ~SS_ICON) | SS_OWNERDRAW);
            }

            UpdateMetrics(GetDpi(hWnd));
            EnableWindowNcDarkMode(hWnd, ShouldAppsUseDarkMode());
            return lResult;
        }
        break;

        case WM_CTLCOLORBTN:
            if (ShouldAppsUseDarkMode())
            {
                const auto* const ptc = PtcCurrent();
                SetDCBrushColor((HDC)wParam, ptc->crDefBtnFace);
                RECT rc;
                GetClipBox((HDC)wParam, &rc);
                FillRect((HDC)wParam, &rc, GetStockBrush(DC_BRUSH));
                return (LRESULT)GetStockBrush(DC_BRUSH);
            }
            break;
        case WM_CTLCOLORSTATIC:
            if (ShouldAppsUseDarkMode())
            {
                const auto* const ptc = PtcCurrent();
                SetTextColor((HDC)wParam, ptc->crDefText);
                SetBkMode((HDC)wParam, TRANSPARENT);
                SetBkColor((HDC)wParam, ptc->crDefBkg);
                SetDCBrushColor((HDC)wParam, ptc->crDefBkg);
                return (LRESULT)GetStockBrush(DC_BRUSH);
            }
            break;
        case WM_DRAWITEM:
        {
            const auto* const ptc = PtcCurrent();
            const auto* const pdis = (DRAWITEMSTRUCT*)lParam;
            if (pdis->CtlType == ODT_STATIC && pdis->CtlID == 0x14)
            {
                DrawIconEx(pdis->hDC, pdis->rcItem.left, pdis->rcItem.top, m_hIcon,
                    0, 0, 0, nullptr, DI_NORMAL);
                return TRUE;
            }
        }
        break;

        case WM_DPICHANGED:
        {
            const auto lResult = __super::OnMessage(hWnd, uMsg, wParam, lParam);
            UpdateMetrics(HIWORD(wParam));
            return lResult;
        }
        break;

        case WM_CHANGEUISTATE:
        {
            if (ShouldAppsUseDarkMode())// 焦点指示器状态改变将导致重绘错误。。。。
            {
                const auto lResult = __super::OnMessage(hWnd, uMsg, wParam, lParam);
                RedrawWindow(hWnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
                return lResult;
            }
        }
        break;
        }
        return __super::OnMessage(hWnd, uMsg, wParam, lParam);
    }
};
ECK_NAMESPACE_END