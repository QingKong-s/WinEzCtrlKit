#include "WndMain.h"
#include "Resource.h"

#include <CommCtrl.h>
#include <Shlwapi.h>
#include <vector>
#include <string>

#include "../eck/CListView.h"


int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR pszCmdLine, _In_ int nCmdShow)
{
	WNDCLASSEXW wcex{ sizeof(WNDCLASSEX) };
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc_Main;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIconW(NULL, IDI_APPLICATION);
	wcex.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszClassName = WCN_MAIN;
	//wcex.cbClsExtra =
	//wcex.cbWndExtra =
	//wcex.lpszMenuName =
	//wcex.hIconSm = LoadIconW(hInstance, MAKEINTRESOURCE());
	if (!RegisterClassExW(&wcex))
		return 1;

	HWND hWnd = CreateWindowExW(0, WCN_MAIN, L"示例Win32程序", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
	if (!hWnd)
		return 1;
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	MSG msg;
	while (GetMessageW(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	return (int)msg.wParam;
}

eck::CListView m_LVRes{};
HWND m_hDlg[4]{};

struct
{
	int cxPadding;
}
m_Size{};

constexpr int
c_cxPadding = 6;

static int csize[] =
{
	0
};

void UpdateDpiSize(int iDpi)
{
	m_Size.cxPadding = eck::DpiScale(c_cxPadding, iDpi);
}

LRESULT CALLBACK WndProc_Main(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_SIZE:
	{

	}
	return 0;

	case WM_COMMAND:
	{

	}
	break;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hDC = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
	}
	return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_CREATE:
	{
		UpdateDpiSize(eck::GetDpi(hWnd));
		//m_LVRes.Create(NULL,WS_VISIBLE,0,m_Size.cxPadding, m_Size.cxPadding,0,0,)
	}
	return 0;
	}

	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

INT_PTR CALLBACK DlgProc_About(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{

	}
	return TRUE;

	case WM_COMMAND:
	{

	}
	break;

	case WM_CLOSE:
		EndDialog(hDlg, 0);
		return TRUE;
	}

	return FALSE;
}