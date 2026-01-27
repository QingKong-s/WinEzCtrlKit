#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
class CIPEdit : public CWnd
{
public:
    ECK_RTTI(CIPEdit, CWnd);
    ECK_CWND_NOSINGLEOWNER(CIPEdit);
    ECK_CWND_CREATE_CLS(WC_IPADDRESSW);

    EckInline void Clear() const noexcept
    {
        SendMsg(IPM_CLEARADDRESS, 0, 0);
    }

    EckInline int GetAddress(_Out_ DWORD* pdwAddress) const noexcept
    {
        return (int)SendMsg(IPM_GETADDRESS, 0, (LPARAM)pdwAddress);
    }

    EckInline BOOL IsBlank() const noexcept
    {
        return (BOOL)SendMsg(IPM_ISBLANK, 0, 0);
    }

    EckInline void SetAddress(DWORD dwAddress) const noexcept
    {
        SendMsg(IPM_SETADDRESS, 0, (LPARAM)dwAddress);
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