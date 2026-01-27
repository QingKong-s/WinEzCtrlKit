#pragma once
#include "CRefStr.h"
#include "WndHelper.h"

ECK_NAMESPACE_BEGIN
struct CWindowPosSetting
{
    enum class Result
    {
        // 窗口至少部分可见
        GoodRect,
        // 窗口不可见，但已成功调整为全部可见
        Adjusted,
        // 窗口不可见，且处理过程中发生错误，
        // 当前坐标应被丢弃，转而使用程序定义的默认坐标
        Error
    };

    constexpr static WCHAR Version[8]{ L"EckWps0" };
    constexpr static int VersionLength = ARRAYSIZE(Version) - 1;

    const int iVer = 0;
    int x{}, y{}, cx{}, cy{};
    BOOL bMaximized{};

    BOOL FromWindow(HWND hWnd) noexcept
    {
        WINDOWPLACEMENT wp;
        wp.length = sizeof(wp);
        if (!GetWindowPlacement(hWnd, &wp))
            return FALSE;
        x = wp.rcNormalPosition.left;
        y = wp.rcNormalPosition.top;
        cx = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
        cy = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
        bMaximized = !!(wp.flags & WPF_RESTORETOMAXIMIZED);
        return TRUE;
    }

    BOOL FromString(PCWSTR psz) noexcept
    {
        if (TcsEqualMaxLenI(psz, Version, VersionLength) != 0)
            return FALSE;
        return swscanf(psz + VersionLength, L",%d,%d,%d,%d,%d",
            &x, &y, &cx, &cy, &bMaximized) == 5;
    }

    BOOL ToWindow(HWND hWnd) noexcept
    {
        if (bMaximized)
            return ShowWindow(hWnd, SW_SHOWMAXIMIZED);
        else
            return SetWindowPos(hWnd, nullptr, x, y, cx, cy, SWP_NOZORDER);
    }

    void ToString(CRefStrW& rs) noexcept
    {
        rs.PushBackFormat(L"%s,%d,%d,%d,%d,%d", Version, x, y, cx, cy, bMaximized);
    }

    Result EnsureVisible() noexcept
    {
        RECT rcWnd{ x,y,x + cx,y + cy };
        if (IsRectEmpty(rcWnd))
            return Result::Error;
        HMONITOR hMonNearest;
        if (MonitorFromRectByWorkArea(rcWnd, nullptr, &hMonNearest))
            return Result::GoodRect;
        if (!hMonNearest)
            return Result::Error;
        MONITORINFO mi;
        mi.cbSize = sizeof(mi);
        GetMonitorInfoW(hMonNearest, &mi);
        if (!AdjustRectIntoAnother(rcWnd, mi.rcWork))
            return Result::Error;
        x = rcWnd.left;
        y = rcWnd.top;
        return Result::Adjusted;
    }
};
ECK_NAMESPACE_END