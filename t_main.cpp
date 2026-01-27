#include "CTestWnd.h"

#ifdef _DEBUG
#define ECK_OPT_CRT_DLL 1
#endif
#include "eck/Env.h"

using namespace std::literals;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR pszCmdLine, _In_ int nCmdShow)
{
	OleInitialize(NULL);
	if (eck::InitStatus b; (b = eck::Initialize(hInstance)) != eck::InitStatus::Ok)
	{
		MessageBoxW(NULL, L"初始化失败", std::to_wstring((int)b).c_str(), 0);
		return 1;
	}

	using namespace eck;
	using namespace eck::Literals;

	MSG msg;
	
	CTestWnd w;
	w.Create(L"示例Win32程序", WS_OVERLAPPEDWINDOW, 0, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, 0);
	w.Show(SW_SHOW);

	while (GetMessageW(&msg, NULL, 0, 0))
	{
		if (!eck::PreTranslateMessage(msg))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}
	return (int)msg.wParam;
}