#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
class CTabHeader :public CWnd
{
private:
	struct ITEM
	{
		CRefStrW rsText;
		LPARAM lParam;
	};
	static ATOM s_Atom;

	HFONT m_hFont = NULL;
	UINT m_uNotifyMsg = 0u;
	std::vector<ITEM> m_Items{};
	

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
public:
	static ATOM RegisterWndClass(HINSTANCE hInstance);

	int InsertItem();
};
ECK_NAMESPACE_END