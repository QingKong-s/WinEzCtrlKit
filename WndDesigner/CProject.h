#pragma once
class CProject
{
public:
	struct FORM
	{
		eck::CRefStrW rsName;	// 窗体名称
		eck::CRefStrW rsIni;	// 布局INI
	};
private:
	std::vector<FORM> m_vForm{};
	eck::CIniExtMut m_Ini{};
	eck::CRefStrW m_rsFileName{};
	eck::IniResult m_eLastIniError{ eck::IniResult::Ok };
public:
	W32ERR NewProject();
	
	W32ERR OpenProject(PCWSTR pszFileName);

	W32ERR SaveProject(PCWSTR pszFileName);

	W32ERR CloseProject();

	W32ERR AddResForm(std::wstring_view svName, std::wstring_view svIni = {});
};