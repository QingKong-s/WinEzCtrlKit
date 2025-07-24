#pragma once
struct DsWnd
{
	eck::CRefStrW rsName{};			// 名称
	eck::CRefStrW rsTitle{};		// 标题
	eck::RCWH rcwh{};				// 位置和大小
	DWORD dwStyle{};				// 窗口样式，总有WS_CHILD | WS_VISIBLE
	DWORD dwExStyle{};				// 扩展样式
	int idxBuiltinCtrl{ -1 };		// 内置控件索引
	eck::CWnd* pWnd{};				// 设计器创建的控件，若当前设计器不显示该控件所在的窗口，则为nullptr
	std::vector<eck::UnqPtrMA<DsWnd>> vChild{};	// 子窗口
};

struct DsForm : DsWnd
{

};

class CProject
{
private:
	std::vector<eck::UnqPtrMA<DsForm>> m_vForm{};
	eck::CIniExtMut m_Ini{};
	eck::CRefStrW m_rsFileName{};
	eck::IniResult m_eLastIniError{ eck::IniResult::Ok };
public:
	W32ERR NewProject();
	
	W32ERR OpenProject(PCWSTR pszFileName);

	W32ERR SaveProject(PCWSTR pszFileName);

	W32ERR CloseProject();

	DsForm* ResAddForm(std::wstring_view svName);
};