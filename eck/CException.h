#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
struct CException
{
	CException() = default;
	virtual ~CException() {}
};

struct CConstMsgException :CException
{
	PCWSTR m_pszMsg = NULL;

	CConstMsgException() = default;
	CConstMsgException(PCWSTR pszMsg) :m_pszMsg{ pszMsg } {}

	PCWSTR GetMsg() const { return m_pszMsg; }
};

struct CFmtMsgException :CException
{
	DWORD m_dwErr = 0;
	CFmtMsgException() = default;
	CFmtMsgException(DWORD dwErr) :m_dwErr{ dwErr } {}

	DWORD GetErrCode() const { return m_dwErr; }
};

struct CAttachSingleOwnerWndException :CException
{};

struct CDetachSingleOwnerWndException :CException
{};

struct CCreateOnPureCWndException :CException
{};

struct CDlgBoxOnPureCDialogException :CException
{};

struct CCreateDlgOnPureCDialogException :CException
{};

struct CMemException :CException
{};

#define EckCheckMem(p) if (!(p)) throw ::eck::CMemException{};

EckInline void DbgPrint(const CConstMsgException& e)
{
	OutputDebugStringW(e.GetMsg());
}

//inline void DbgPrint(const CFmtMsgException& e)
//{
//	PWSTR pszInfo;
//	FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
//		NULL, e.GetErrCode(), 0, (PWSTR)&pszInfo, 0, NULL);
//	OutputDebugStringW(Format(L"错误码 = %u，格式化信息 = %s", e.GetErrCode(), pszInfo).Data());
//	LocalFree(pszInfo);
//}

ECK_NAMESPACE_END