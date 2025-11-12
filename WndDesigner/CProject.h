#pragma once
struct DsWnd
{
    eck::CRefStrW rsName{};     // 名称
    eck::CRefStrW rsTitle{};    // 标题
    eck::RCWH rcwh{};           // 位置和大小
    DWORD dwStyle{};            // 窗口样式，总有WS_CHILD | WS_VISIBLE
    DWORD dwExStyle{};          // 扩展样式
    int idxBuiltinCtrl{ -1 };   // 内置控件索引
    eck::CWnd* pWnd{};          // 设计器创建的窗口，可能为nullptr
    std::vector<std::shared_ptr<DsWnd>> vChild{};// 子窗口
};

struct DsForm : DsWnd
{

};

class CProject
{
    // Rs = Resource
public:
    enum class Result
    {
        Ok,
    };
private:
    std::vector<std::shared_ptr<DsForm>> m_vForm{};
public:
    Result New();
    Result Open(PCWSTR pszFileName);
    Result Save(PCWSTR pszFileName);
    Result Close();

    std::shared_ptr<DsForm> RsAddForm(std::wstring_view svName);
};