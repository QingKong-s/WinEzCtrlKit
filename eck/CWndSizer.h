#pragma once
#include "Utility.h"

ECK_NAMESPACE_BEGIN
class CWndSizer
{
private:
	struct SIZERINFO
	{
		HWND hWnd;
		int x;
		int y;
		int cx;
		int cy;
		RECT rcOrg;
	};

	HWND m_hContainer = NULL;
	std::vector<SIZERINFO> m_Info{};
public:
	CWndSizer(HWND hContainer) :m_hContainer(hContainer)
	{

	}
	~CWndSizer()
	{

	}
	//void Add(HWND hWnd, int x, int y)
	//{
	//	m_Info.push_back({ hWnd, x, y });

	//}
};
ECK_NAMESPACE_END