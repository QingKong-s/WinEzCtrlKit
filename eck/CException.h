/*
* WinEzCtrlKit Library
*
* CException.h ： 异常
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "CRefStr.h"

ECK_NAMESPACE_BEGIN
struct CException
{
	CException() = default;

	virtual ~CException() = default;

	virtual PCWSTR What() const { return L"未知异常"; }
};

struct CConstMsgException :CException
{
	PCWSTR m_pszMsg{};

	CConstMsgException() = default;

	constexpr CConstMsgException(PCWSTR pszMsg) :m_pszMsg{ pszMsg } {}

	PCWSTR What() const override { return m_pszMsg; }
};

struct CFmtMsgException :CException
{
	DWORD m_dwErr{};
	CRefStrW m_rsMsg{};

	CFmtMsgException() = default;

	CFmtMsgException(DWORD dwErr) :m_dwErr{ dwErr }
	{
		Format();
	}

	void Format()
	{
		PWSTR pszInfo;
		FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr,
			m_dwErr, 0, (PWSTR)&pszInfo, 0, nullptr);
		if (pszInfo)
		{
			m_rsMsg.Format(L"(%u) %s", m_dwErr, pszInfo);
			LocalFree(pszInfo);
		}
		else
			m_rsMsg.Format(L"(%u) 未知错误", m_dwErr);
	}

	PCWSTR What() const override { return m_rsMsg.Data(); }
};

struct CMemException :CException
{
	CMemException() = default;

	PCWSTR What() const override { return L"内存分配失败"; }
};

EckInline void DbgPrint(CException& e)
{
	OutputDebugStringW(e.What());
}
ECK_NAMESPACE_END