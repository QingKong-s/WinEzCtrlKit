#pragma once
#include "CProject.h"

class CWndMain;
class CWndWork : public eck::CWnd
{
private:
	friend class CWndMain;

	BOOLEAN m_bLBtnDown{};
	POINT m_ptSelStart{};

	CWndMain* m_pWndMain{};

	std::vector<RECT> m_vRect{};
	std::shared_ptr<DsForm> m_pForm{};

	void SelBeginDragging(POINT pt);
	void SelDraggingMove(POINT pt);
	void SelCancel();

	EckInlineNdCe auto Host() const noexcept { return m_pWndMain; }
public:
	ECK_CWND_CREATE_CLS_HINST(eck::WCN_DUMMY, eck::g_hInstance);

	CWndWork(std::shared_ptr<DsForm> pForm, CWndMain* pWndMain) : m_pForm{ pForm }, m_pWndMain{ pWndMain } {}

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
};