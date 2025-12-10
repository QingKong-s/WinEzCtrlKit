#pragma once
#include "CRefStr.h"
#include "CRefBin.h"

ECK_NAMESPACE_BEGIN
struct CClipboardGuard { ~CClipboardGuard() { CloseClipboard(); } };

inline BOOL GetClipboardString(HWND hWnd, CRefStrW& rs) noexcept
{
    if (!OpenClipboard(hWnd))
    {
        EckDbgPrintFmt(L"OpenClipboard failed, Owner = %p", GetClipboardOwner());
        return FALSE;
    }
    CClipboardGuard _{};

    const HGLOBAL hData = GetClipboardData(CF_UNICODETEXT);
    if (!hData)
        return FALSE;
    const PCVOID pData = GlobalLock(hData);
    if (!pData)
        return FALSE;
    auto cch = int(GlobalSize(hData) / sizeof(WCHAR));
    if (cch)
    {
        rs.PushBack((PCWCH)pData, cch);
        while (cch && !rs.IsEmpty() && rs.Back() == L'\0')
        {
            rs.PopBack();
            --cch;
        }
    }
    return TRUE;
}
inline BOOL SetClipboardString(HWND hWnd,
    _In_reads_or_z_(cch) PCWSTR pszText, int cch = -1) noexcept
{
    if (!OpenClipboard(hWnd))
    {
        EckDbgPrintFmt(L"OpenClipboard failed, Owner = %p", GetClipboardOwner());
        return FALSE;
    }
    CClipboardGuard _{};

    if (cch < 0)
        cch = (int)TcsLen(pszText);
    const auto hData = GlobalAlloc(GMEM_MOVEABLE, Cch2CbW(cch));
    if (!hData)
        return FALSE;
    void* const pData = GlobalLock(hData);
    if (!pData)
    {
        GlobalFree(hData);
        return FALSE;
    }
    TcsCopyLenEnd((PWCH)pData, pszText, cch);
    GlobalUnlock(hData);

    EmptyClipboard();
    if (SetClipboardData(CF_UNICODETEXT, hData))
        return TRUE;
    else
    {
        GlobalFree(hData);
        return FALSE;
    }
}



ECK_NAMESPACE_END