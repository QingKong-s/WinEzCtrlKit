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

void DbgPrint(const CConstMsgException& e)
{
	OutputDebugStringW(e.GetMsg());
}

void DbgPrint(const CFmtMsgException& e)
{
	PWSTR pszInfo;
	FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, e.GetErrCode(), 0, (PWSTR)&pszInfo, 0, NULL);
	OutputDebugStringW(std::format(L"错误码 = {}，格式化信息 = {}", e.GetErrCode(), pszInfo).c_str());
	LocalFree(pszInfo);
}

ECK_NAMESPACE_END