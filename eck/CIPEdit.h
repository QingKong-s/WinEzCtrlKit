#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
class CIPEdit : public CWindow
{
public:
    ECK_RTTI(CIPEdit, CWindow);
    ECK_CWND_NOSINGLEOWNER(CIPEdit);
    ECK_CWND_CREATE_CLS(WC_IPADDRESSW);

    EckInline void Clear() const noexcept
    {
        SendMsg(IPM_CLEARADDRESS, 0, 0);
    }

    EckInline int GetAddress(_Out_ UINT* puAddress) const noexcept
    {
        return (int)SendMsg(IPM_GETADDRESS, 0, (LPARAM)puAddress);
    }

    EckInline BOOL IsBlank() const noexcept
    {
        return (BOOL)SendMsg(IPM_ISBLANK, 0, 0);
    }

    EckInline void SetAddress(UINT uAddress) const noexcept
    {
        SendMsg(IPM_SETADDRESS, 0, (LPARAM)uAddress);
    }

    EckInline void SetFieldFocus(int iField) const noexcept
    {
        SendMsg(IPM_SETFOCUS, iField, 0);
    }

    EckInline void SetFieldRange(int iField, WORD wLimit) const noexcept
    {
        SendMsg(IPM_SETRANGE, iField, (LPARAM)wLimit);
    }
};
ECK_NAMESPACE_END