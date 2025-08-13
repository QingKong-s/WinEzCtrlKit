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
		const auto e = eck::NaGetLastError();
		if (e)
			return e;
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

DsForm* CProject::ResAddForm(std::wstring_view svName)
{
	//auto& Form = m_vForm.emplace_back(std::make_unique<DsForm>());
	//Form->rsName = svName;
	return 0;
}