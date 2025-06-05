#pragma once
#include "IScroll.h"

ECK_NAMESPACE_BEGIN
class CWndScroll : public IScroll
{
private:
	HWND m_hWnd{};
	BOOL m_bHorz{};
public:
	CWndScroll() = default;
	CWndScroll(HWND hWnd, BOOL bHorz) : m_hWnd{ hWnd }, m_bHorz{ bHorz } {}

	EckInlineCe void SetWindow(HWND hWnd) { m_hWnd = hWnd; }
	EckInlineNdCe HWND GetWindow() const { return m_hWnd; }

	BOOL ScrIsValid() override
	{
		return !!(GetWindowLongPtrW(m_hWnd, GWL_STYLE) &
			(m_bHorz ? WS_HSCROLL : WS_VSCROLL));
	}
	BOOL ScrIsVisible() override
	{
		return ScrIsValid();
	}
	void ScrSetScrollInfo(const SCROLLINFO& si) override
	{
		SetScrollInfo(m_hWnd, m_bHorz ? SB_HORZ : SB_VERT, &si, TRUE);
	}
	void ScrGetScrollInfo(_Inout_ SCROLLINFO& si) override
	{
		GetScrollInfo(m_hWnd, m_bHorz ? SB_HORZ : SB_VERT, &si);
	}
	void ScrSetViewSize(int iViewSize) override {}
	int ScrGetViewSize() override
	{
		RECT rc;
		GetWindowRect(m_hWnd, &rc);
		return m_bHorz ? rc.right - rc.left : rc.bottom - rc.top;
	}
};
ECK_NAMESPACE_END