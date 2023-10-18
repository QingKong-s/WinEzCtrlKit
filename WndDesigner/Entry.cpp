#include "CWndMain.h"

#include "..\eck\Env.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR pszCmdLine, _In_ int nCmdShow)
{
	App = new CApp();
	App->Init(hInstance);

	CWndMain::RegisterWndClass();
	CWndMain WndMain{};

	WndMain.Create(L"ECK窗体设计器", WS_OVERLAPPEDWINDOW, 0, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, 0);
	ShowWindow(WndMain.GetHWND(), nCmdShow);
	UpdateWindow(WndMain.GetHWND());

	MSG msg;
	while (GetMessageW(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	return (int)msg.wParam;
}