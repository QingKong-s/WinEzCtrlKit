/*
* WinEzCtrlKit Library
*
* CIPEdit.h ： 标准IP编辑框
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
class CIPEdit : public CWnd
{
public:
	ECK_RTTI(CIPEdit);
	ECK_CWND_NOSINGLEOWNER(CIPEdit);
	ECK_CWND_CREATE_CLS(WC_IPADDRESSW);

	EckInline void Clear() const
	{
		SendMsg(IPM_CLEARADDRESS, 0, 0);
	}

	EckInline int GetAddress(DWORD* pdwAddress) const
	{
		return (int)SendMsg(IPM_GETADDRESS, 0, (LPARAM)pdwAddress);
	}

	EckInline BOOL IsBlank() const
	{
		return (BOOL)SendMsg(IPM_ISBLANK, 0, 0);
	}

	EckInline void SetAddress(DWORD dwAddress) const
	{
		SendMsg(IPM_SETADDRESS, 0, (LPARAM)dwAddress);
	}

	EckInline void SetFieldFocus(int iField) const
	{
		SendMsg(IPM_SETFOCUS, iField, 0);
	}

	EckInline void SetFieldRange(int iField, WORD wLimit) const
	{
		SendMsg(IPM_SETRANGE, iField, (LPARAM)wLimit);
	}
};
ECK_NAMESPACE_END