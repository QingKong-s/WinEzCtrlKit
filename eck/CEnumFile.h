/*
* WinEzCtrlKit Library
*
* CEnumFile.h ： 文件枚举
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
class CEnumFile
{
private:
	HANDLE m_hFind{};
	WIN32_FIND_DATA m_wfd;
public:
	ECK_DISABLE_COPY_MOVE_DEF_CONS(CEnumFile);
	CEnumFile(PCWSTR pszPath, BOOL bQueryShortNames = FALSE, DWORD dwAdditionalFlags = 0u)
		: m_hFind{ FindFirstFileExW(pszPath,
			bQueryShortNames ? FindExInfoStandard : FindExInfoBasic,
			&m_wfd, FindExSearchNameMatch, nullptr, dwAdditionalFlags) } {
	}

	HANDLE Open(PCWSTR pszPath, BOOL bQueryShortNames = FALSE, DWORD dwAdditionalFlags = 0u)
	{
		return m_hFind = FindFirstFileExW(pszPath,
			bQueryShortNames ? FindExInfoStandard : FindExInfoBasic,
			&m_wfd, FindExSearchNameMatch, nullptr, dwAdditionalFlags);
	}

	EckInline BOOL Next() { return FindNextFileW(m_hFind, &m_wfd); }

	EckInline constexpr HANDLE GetHFind() const { return m_hFind; }

	EckInline constexpr auto& Data() { return m_wfd; }
};
ECK_NAMESPACE_END