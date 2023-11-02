#include "CSplitBar.h"

ECK_NAMESPACE_BEGIN
LRESULT CSplitBar::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	auto p = (CSplitBar*)GetWindowLongPtrW(hWnd, 0);
	switch (uMsg)
	{
	case WM_SETCURSOR:
		if (p->m_bHorizontal)
			SetCursor(LoadCursorW(NULL, IDC_SIZENS));
		else
			SetCursor(LoadCursorW(NULL, IDC_SIZEWE));
		return 0;
	case WM_MOUSEMOVE:
	{
		if (p->m_bLBtnDown)
		{
			POINT ptClient GET_PT_LPARAM(lParam);
			const int xyPos = p->CursorPtToPos(ptClient);
			POINT ptNew{};
			if (p->m_bHorizontal)
			{
				ptNew.y = xyPos;
				ClientToScreen(GetParent(hWnd), &ptNew);
				ptNew.x = p->m_xyFixed;
			}
			else
			{
				ptNew.x = xyPos;
				ClientToScreen(GetParent(hWnd), &ptNew);
				ptNew.y = p->m_xyFixed;
			}
			
			p->MoveMark(ptNew.x, ptNew.y);
		}
	}
	return 0;
	case WM_SIZE:
	{
		GET_SIZE_LPARAM(p->m_cxClient, p->m_cyClient, lParam);
		p->m_DC.ReSize(hWnd, p->m_cxClient, p->m_cyClient);
		SetWindowPos(p->m_BKMark.GetHWND(), NULL, 0, 0, p->m_cxClient, p->m_cyClient,
			SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
	}
	return 0;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		BeginPaint(hWnd, &ps);
		FillRect(ps.hdc, &ps.rcPaint, p->m_hbrBK);
		EndPaint(hWnd, &ps);
	}
	return 0;
	case WM_LBUTTONDOWN:
	{
		SetFocus(hWnd);
		SetCapture(hWnd);
		p->m_bLBtnDown = TRUE;
		RECT rc;
		GetWindowRect(hWnd, &rc);

		if (p->m_bHorizontal)
		{
			p->m_xyFixed = rc.left;
			p->MoveMark(rc.left, rc.top);
			p->m_cxyCursorOffset = GET_Y_LPARAM(lParam);
		}
		else
		{
			p->m_xyFixed = rc.top;
			p->MoveMark(rc.left, rc.top);
			p->m_cxyCursorOffset = GET_X_LPARAM(lParam);
		}
	}
	return 0;
	case WM_LBUTTONUP:
	{
		if (p->m_bLBtnDown)
		{
			ReleaseCapture();
			p->m_bLBtnDown = FALSE;
			p->HideMark();
			const int xyPos = p->CursorPtToPos(GET_PT_LPARAM(lParam));
			if (p->m_uNotifyMsg)
				SendMessageW(GetParent(hWnd), p->m_uNotifyMsg, GetDlgCtrlID(hWnd), xyPos);
		}
	}
	return 0;
	case WM_KEYDOWN:
	{
		if (p->m_bLBtnDown && wParam == VK_ESCAPE)
		{
			ReleaseCapture();
			p->m_bLBtnDown = FALSE;
			p->HideMark();
		}
	}
	return 0;
	case WM_NCCREATE:
	{
		p = (CSplitBar*)(((CREATESTRUCTW*)lParam)->lpCreateParams);
		SetWindowLongPtrW(hWnd, 0, (LONG_PTR)p);
		p->m_BKMark.Create(NULL, WS_POPUP | WS_DISABLED,
			WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED | WS_EX_NOACTIVATE,
			-10000, -10000, 0, 0, hWnd, 0);
		p->UpdateMarkWndAlpha();
		SetWindowLongPtrW(p->m_BKMark.GetHWND(), 0, (LONG_PTR)p);
		p->m_BKMark.SetWindowProc(WndProc_Mark);
		p->m_DC.Create(hWnd);
	}
	break;
	case WM_DESTROY:
		p->m_BKMark.Destroy();
		return 0;
	}

	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

LRESULT CSplitBar::WndProc_Mark(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	auto p = (CSplitBar*)GetWindowLongPtrW(hWnd, 0);
	switch (uMsg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		BeginPaint(hWnd, &ps);
		FillRect(ps.hdc, &ps.rcPaint, p->m_hbrMark);
		EndPaint(hWnd, &ps);
	}
	return 0;
	}
	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

int CSplitBar::CursorPtToPos(POINT ptClient)
{
	POINT ptDst;
	int xyPos;
	if (m_bHorizontal)
	{
		MapWindowPoints(m_hWnd, GetParent(m_hWnd), &ptClient, 1);// 取相对父窗口的光标位置
		xyPos = ptClient.y - m_cxyCursorOffset;
		// 限制位置
		if (m_xyMin == 0 && m_xyMax == 0)// 默认范围为整个父窗口
		{
			RECT rc;
			GetClientRect(GetParent(m_hWnd), &rc);
			if (xyPos < 0)
				xyPos = 0;
			else if (xyPos > rc.bottom - m_cyClient)
				xyPos = rc.bottom - m_cyClient;
		}
		else
		{
			if (xyPos < m_xyMin)
				xyPos = m_xyMin;
			else if (xyPos > m_xyMax)
				xyPos = m_xyMax;
		}
	}
	else
	{
		MapWindowPoints(m_hWnd, GetParent(m_hWnd), &ptClient, 1);// 取相对父窗口的光标位置
		xyPos = ptClient.x - m_cxyCursorOffset;
		// 限制位置
		if (m_xyMin == 0 && m_xyMax == 0)// 默认范围为整个父窗口
		{
			RECT rc;
			GetClientRect(GetParent(m_hWnd), &rc);
			if (xyPos < 0)
				xyPos = 0;
			else if (xyPos > rc.right - m_cxClient)
				xyPos = rc.right - m_cxClient;
		}
		else
		{
			if (xyPos < m_xyMin)
				xyPos = m_xyMin;
			else if (xyPos > m_xyMax)
				xyPos = m_xyMax;
		}
	}
	return xyPos;
}

ATOM CSplitBar::RegisterWndClass(HINSTANCE hInstance)
{
	WNDCLASSW wc{};
	wc.cbWndExtra = sizeof(void*);
	wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wc.hInstance = hInstance;
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = WCN_SPLITBAR;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	return RegisterClassW(&wc);
}

CSplitBar::~CSplitBar()
{
	DeleteObject(m_hbrBK);
	DeleteObject(m_hbrMark);
}
ECK_NAMESPACE_END