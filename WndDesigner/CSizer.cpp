#include "CSizer.h"
LRESULT CALLBACK CSizerBlock::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	auto p = (CSizerBlock*)GetWindowLongPtrW(hWnd, 0);
	switch (uMsg)
	{
	case WM_SIZE:
		p->m_rcClient = { 0,0,LOWORD(lParam),HIWORD(lParam) };
		return 0;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		BeginPaint(hWnd, &ps);
		HGDIOBJ hOldBrush = SelectObject(ps.hdc, p->m_hbrBlock);
		Rectangle(ps.hdc, 0, 0, p->m_rcClient.right, p->m_rcClient.bottom);
		SelectObject(ps.hdc, hOldBrush);
		EndPaint(hWnd, &ps);
	}
	return 0;

	case WM_SETCURSOR:
		SetCursor(p->m_hCursor);
		return 0;

	case WM_MOUSEMOVE:
		p->m_pSizer->OnBlockMouseMove(p, lParam);
		return 0;

	case WM_LBUTTONDOWN:
		p->m_pSizer->OnBlockLButtonDown(p, lParam);
		return 0;

	case WM_LBUTTONUP:
		p->m_pSizer->OnBlockLButtonUp(p, lParam);
		return 0;
	}

	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

HWND CSizerBlock::Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
	int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData)
{
	m_hWnd = CreateWindowExW(dwExStyle, eck::WCN_BK, pszText, dwStyle,
		x, y, cx, cy, hParent, eck::hMenu, eck::g_hInstance, NULL);

	m_hbrBlock = CreateSolidBrush(eck::Colorref::Teal);

	m_rcClient = { 0,0,cx,cy };

	SetWindowLongPtrW(m_hWnd, 0, (LONG_PTR)this);
	SetWindowLongPtrW(m_hWnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
	return m_hWnd;
}

void CSizerBlock::BindSizer(CSizer* pSizer, SizerHTCode uType)
{
	m_pSizer = pSizer;
	m_uType = uType;
	switch (uType)
	{
	case SizerHTCode::LeftTop:
	case SizerHTCode::RightBottom:
		m_hCursor = LoadCursorW(NULL, IDC_SIZENWSE);
		break;
	case SizerHTCode::RightTop:
	case SizerHTCode::LeftBottom:
		m_hCursor = LoadCursorW(NULL, IDC_SIZENESW);
		break;
	case SizerHTCode::Left:
	case SizerHTCode::Right:
		m_hCursor = LoadCursorW(NULL, IDC_SIZEWE);
		break;
	case SizerHTCode::Top:
	case SizerHTCode::Bottom:
		m_hCursor = LoadCursorW(NULL, IDC_SIZENS);
		break;
	}
}



void CSizer::Create(HWND hBK, HWND hBottomWorkWnd)
{
	m_hBK = hBK;
	m_hBottomWorkWnd = hBottomWorkWnd;
	constexpr DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	constexpr DWORD dwExStyle = WS_EX_TOPMOST;
	m_Block[0].Create(NULL, dwStyle, dwExStyle, 0, 0, m_sizeBlock, m_sizeBlock, hBK, 30000);
	m_Block[0].BindSizer(this, SizerHTCode::LeftTop);
	m_Block[1].Create(NULL, dwStyle, dwExStyle, 0, 0, m_sizeBlock, m_sizeBlock, hBK, 30001);
	m_Block[1].BindSizer(this, SizerHTCode::Top);
	m_Block[2].Create(NULL, dwStyle, dwExStyle, 0, 0, m_sizeBlock, m_sizeBlock, hBK, 30002);
	m_Block[2].BindSizer(this, SizerHTCode::RightTop);
	m_Block[3].Create(NULL, dwStyle, dwExStyle, 0, 0, m_sizeBlock, m_sizeBlock, hBK, 30003);
	m_Block[3].BindSizer(this, SizerHTCode::Left);
	m_Block[4].Create(NULL, dwStyle, dwExStyle, 0, 0, m_sizeBlock, m_sizeBlock, hBK, 30004);
	m_Block[4].BindSizer(this, SizerHTCode::Right);
	m_Block[5].Create(NULL, dwStyle, dwExStyle, 0, 0, m_sizeBlock, m_sizeBlock, hBK, 30005);
	m_Block[5].BindSizer(this, SizerHTCode::LeftBottom);
	m_Block[6].Create(NULL, dwStyle, dwExStyle, 0, 0, m_sizeBlock, m_sizeBlock, hBK, 30006);
	m_Block[6].BindSizer(this, SizerHTCode::Bottom);
	m_Block[7].Create(NULL, dwStyle, dwExStyle, 0, 0, m_sizeBlock, m_sizeBlock, hBK, 30007);
	m_Block[7].BindSizer(this, SizerHTCode::RightBottom);

	m_hPen = CreatePen(PS_SOLID, 2, eck::Colorref::Red);
}

HWND CSizer::SetTargetWindow(HWND hWnd)
{
	HWND hOld = m_hWorkWnd;
	m_hWorkWnd = hWnd;
	m_hWorkWndParent = GetParent(hWnd);
	MoveToTargetWindow();
	return hOld;
}

SizerHTCode CSizer::HitTest(POINT pt)
{
	HWND hWnd = ChildWindowFromPoint(m_hBK, pt);
	for (auto& x : m_Block)
	{
		if (x.m_hWnd == hWnd)
			return x.m_uType;
	}
	return SizerHTCode::None;
}

void CSizer::MoveToTargetWindow()
{
	RECT rc;
	GetWindowRect(m_hWorkWnd, &rc);
	eck::ScreenToClient(m_hBK, &rc);

	SetWindowPos(m_Block[0], NULL,
		rc.left - m_sizeBlock,
		rc.top - m_sizeBlock,
		0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	SetWindowPos(m_Block[1], NULL,
		rc.left + (rc.right - rc.left - m_sizeBlock) / 2,
		rc.top - m_sizeBlock,
		0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	SetWindowPos(m_Block[2], NULL,
		rc.right,
		rc.top - m_sizeBlock,
		0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	SetWindowPos(m_Block[3], NULL,
		rc.left - m_sizeBlock,
		rc.top + (rc.bottom - rc.top - m_sizeBlock) / 2,
		0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	SetWindowPos(m_Block[4], NULL,
		rc.right,
		rc.top + (rc.bottom - rc.top - m_sizeBlock) / 2,
		0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	SetWindowPos(m_Block[5], NULL,
		rc.left - m_sizeBlock,
		rc.bottom,
		0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	SetWindowPos(m_Block[6], NULL,
		rc.left + (rc.right - rc.left - m_sizeBlock) / 2,
		rc.bottom,
		0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	SetWindowPos(m_Block[7], NULL,
		rc.right,
		rc.bottom,
		0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);

	for (auto& x : m_Block)
		SetWindowPos(x, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	SetWindowPos(m_hBottomWorkWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
}

RECT CSizer::SizerMakeRect(POINT ptCursor, SizerHTCode uType)
{
	RECT rc;
	switch (uType)
	{
	case SizerHTCode::LeftTop:
		rc.left = ptCursor.x;
		rc.top = ptCursor.y;
		rc.right = m_rcOrg.right;
		rc.bottom = m_rcOrg.bottom;
		break;
	case SizerHTCode::RightTop:
		rc.left = m_rcOrg.left;
		rc.top = ptCursor.y;
		rc.right = ptCursor.x;
		rc.bottom = m_rcOrg.bottom;
		break;
	case SizerHTCode::LeftBottom:
		rc.left = ptCursor.x;
		rc.top = m_rcOrg.top;
		rc.right = m_rcOrg.right;
		rc.bottom = ptCursor.y;
		break;
	case SizerHTCode::RightBottom:
		rc.left = m_rcOrg.left;
		rc.top = m_rcOrg.top;
		rc.right = ptCursor.x;
		rc.bottom = ptCursor.y;
		break;
	case SizerHTCode::Top:
		rc.left = m_rcOrg.left;
		rc.top = ptCursor.y;
		rc.right = m_rcOrg.right;
		rc.bottom = m_rcOrg.bottom;
		break;
	case SizerHTCode::Bottom:
		rc.left = m_rcOrg.left;
		rc.top = m_rcOrg.top;
		rc.right = m_rcOrg.right;
		rc.bottom = ptCursor.y;
		break;
	case SizerHTCode::Left:
		rc.left = ptCursor.x;
		rc.top = m_rcOrg.top;
		rc.right = m_rcOrg.right;
		rc.bottom = m_rcOrg.bottom;
		break;
	case SizerHTCode::Right:
		rc.left = m_rcOrg.left;
		rc.top = m_rcOrg.top;
		rc.right = ptCursor.x;
		rc.bottom = m_rcOrg.bottom;
		break;
	}

	return rc;
}

void CSizer::OnBlockLButtonDown(CSizerBlock* pBlock, LPARAM lParam)
{
	SetCapture(pBlock->m_hWnd);
	m_bLBtnDown = TRUE;
	m_hDC = GetDC(m_hBK);
	SetROP2(m_hDC, R2_NOTXORPEN);
	m_hOld = SelectObject(m_hDC, m_hPen);

	GetWindowRect(m_hWorkWnd, &m_rcOrg);
	eck::ScreenToClient(m_hBK, &m_rcOrg);
}

void CSizer::OnBlockLButtonUp(CSizerBlock* pBlock, LPARAM lParam)
{
	if (!m_bLBtnDown)
		return;
	ReleaseCapture();
	m_bLBtnDown = FALSE;
	m_rcLast = {};
	SelectObject(m_hDC, m_hOld);
	ReleaseDC(pBlock->m_hWnd, m_hDC);

	POINT pt = GET_PT_LPARAM(lParam);
	MapWindowPoints(pBlock->m_hWnd, m_hBK, &pt, 1);
	RECT rc = SizerMakeRect(pt, pBlock->m_uType);
	MapWindowRect(m_hBK, m_hWorkWndParent, &rc);
	rc.right -= rc.left;
	rc.bottom -= rc.top;
	SetWindowPos(m_hWorkWnd, NULL, rc.left, rc.top, rc.right, rc.bottom, SWP_NOZORDER | SWP_NOACTIVATE);
	MoveToTargetWindow();

	InvalidateRect(m_hBK, NULL, FALSE);
	UpdateWindow(m_hBK);
}

void CSizer::OnBlockMouseMove(CSizerBlock* pBlock, LPARAM lParam)
{
	if (!m_bLBtnDown)
		return;
	POINT pt = GET_PT_LPARAM(lParam);
	MapWindowPoints(pBlock->m_hWnd, m_hBK, &pt, 1);

	RECT rc = SizerMakeRect(pt, pBlock->m_uType);

	if (!IsRectEmpty(&m_rcLast))
		Rectangle(m_hDC, m_rcLast.left, m_rcLast.top, m_rcLast.right, m_rcLast.bottom);
	m_rcLast = rc;
	Rectangle(m_hDC, rc.left, rc.top, rc.right, rc.bottom);
}