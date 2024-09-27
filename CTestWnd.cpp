#include "CTestWnd.h"

void CTestWnd::Test()
{
	using namespace eck;
	using namespace eck::Literals;
	
	CRefBin rb{};
	rb.PushBack({ 1,2,3 });
	rb.PushBack({ 4,5,6 });
	rb.PushBack({ 7,8,9 });
	rb.PushBack({ 10,11,12 });
	rb.PushBack({ 13,14,15 });

	rb = {};

}

LRESULT CTestWnd::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		Test();
		break;
	}
	return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}
