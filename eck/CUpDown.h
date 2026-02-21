#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
class CUpDown : public CWindow
{
public:
    ECK_RTTI(CUpDown, CWindow);
    ECK_CWND_NOSINGLEOWNER(CUpDown);
    ECK_CWND_CREATE_CLS(UPDOWN_CLASSW);

    ECK_CWNDPROP_STYLE(AlignLeft, UDS_ALIGNLEFT);
    ECK_CWNDPROP_STYLE(AlignRight, UDS_ALIGNRIGHT);
    ECK_CWNDPROP_STYLE(ArrowKeys, UDS_ARROWKEYS);
    ECK_CWNDPROP_STYLE(AutoBuddy, UDS_AUTOBUDDY);
    ECK_CWNDPROP_STYLE(Horizontal, UDS_HORZ);
    ECK_CWNDPROP_STYLE(NoThousands, UDS_NOTHOUSANDS);
    ECK_CWNDPROP_STYLE(SetBuddyInt, UDS_SETBUDDYINT);
    ECK_CWNDPROP_STYLE(Wrap, UDS_WRAP);

    EckInline int GetAcceleration(
        _Out_writes_opt_(cBuf) UDACCEL* pBuf, int cBuf) const noexcept
    {
        return (int)SendMsg(UDM_GETACCEL, cBuf, (LPARAM)pBuf);
    }

    EckInline void GetAcceleration(std::vector<UDACCEL>& vAccel) const noexcept
    {
        int cAccel = (int)GetAcceleration(nullptr, 0);
        if (!cAccel)
            return;
        vAccel.resize(cAccel);
        GetAcceleration(vAccel.data(), cAccel);
    }

    EckInline int GetBase() const noexcept
    {
        return (int)SendMsg(UDM_GETBASE, 0, 0);
    }

    EckInline HWND GetBuddy() const noexcept
    {
        return (HWND)SendMsg(UDM_GETBUDDY, 0, 0);
    }

    EckInline int GetPosition(_Out_opt_ BOOL* pbSuccess = nullptr) const noexcept
    {
        return (int)SendMsg(UDM_GETPOS32, 0, (LPARAM)pbSuccess);
    }

    EckInline void GetRange(
        _Out_opt_ int* piMin = nullptr,
        _Out_opt_ int* piMax = nullptr) const noexcept
    {
        SendMsg(UDM_GETRANGE32, (WPARAM)piMin, (LPARAM)piMax);
    }

    EckInline BOOL SetAcceleration(_In_reads_(c) const UDACCEL* puda, int c) const noexcept
    {
        return (BOOL)SendMsg(UDM_SETACCEL, c, (LPARAM)puda);
    }

    EckInline BOOL SetBase(int iBase) const noexcept
    {
        return (BOOL)SendMsg(UDM_SETBASE, iBase, 0);
    }

    EckInline HWND SetBuddy(HWND hBuddy) const noexcept
    {
        return (HWND)SendMsg(UDM_SETBUDDY, (WPARAM)hBuddy, 0);
    }

    EckInline int SetPosition(int iPos) const noexcept
    {
        return (int)SendMsg(UDM_SETPOS, 0, iPos);
    }

    EckInline void SetRange(int iMin, int iMax) const noexcept
    {
        SendMsg(UDM_SETRANGE32, iMin, iMax);
    }

    EckInline void SetMinimum(int iMin) const noexcept
    {
        int iMax;
        GetRange(nullptr, &iMax);
        SetRange(iMin, iMax);
    }

    EckInline void SetMaximum(int iMax) const noexcept
    {
        int iMin;
        GetRange(&iMin, nullptr);
        SetRange(iMin, iMax);
    }
};
ECK_NAMESPACE_END