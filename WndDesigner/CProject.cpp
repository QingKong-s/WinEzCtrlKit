#include "pch.h"
#include "CProject.h"

W32ERR CProject::NewProject()
{
	m_vForm.clear();
	m_rsFileName.Clear();
	return ERROR_SUCCESS;
}

W32ERR CProject::OpenProject(PCWSTR pszFileName)
{
	const auto rbFile = eck::ReadInFile(pszFileName);
	NewProject();
	if (rbFile.IsEmpty())
	{
		if (NaGetLastError())
			return NaGetLastError();
		return ERROR_SUCCESS;// 空文档
	}
	const auto r = m_Ini.Load((PCWSTR)rbFile.Data(),
		rbFile.Size() / sizeof(WCHAR));
	if (r != eck::IniResult::Ok)
	{
		m_eLastIniError = r;
		return ERROR_BAD_FORMAT;
	}
	return ERROR_SUCCESS;
}

W32ERR CProject::SaveProject(PCWSTR pszFileName)
{
	return W32ERR();
}

W32ERR CProject::CloseProject()
{
	return W32ERR();
}

W32ERR CProject::AddResForm(std::wstring_view svName, std::wstring_view svIni)
{
	auto SecRoot = m_Ini.GetSection(L"EckWin32Res"sv);
	if (!SecRoot)
		SecRoot = m_Ini.CreateSection(L"EckWin32Res"sv, eck::INIE_EF_IS_CONTAINER);
	auto SecResForm = m_Ini.GetSection(SecRoot, L"ResForm"sv);
	if (!SecResForm)// 空文档
		SecResForm = m_Ini.CreateSection(SecResForm,
			L"ResForm"sv, eck::INIE_EF_IS_CONTAINER);
	return W32ERR();
}