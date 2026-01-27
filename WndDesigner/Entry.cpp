#include "pch.h"
#include "CWndMain.h"
#include "CApp.h"

#include "..\eck\Env.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR pszCmdLine, _In_ int nCmdShow)
{
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);

	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr))
	{
		EckDbgPrintFormatMessage(hr);
		eck::MsgBox(eck::Format(L"CoInitialize failed! hr = %08X", hr), L"Error", MB_ICONERROR);
		return 0;
	}

	DWORD dwErr;
	eck::InitStatus iRetInit;
	eck::INITPARAM ip{};
	ip.uFlags = eck::EIF_NOINITD2D;
	if ((iRetInit = eck::Initialize(hInstance, &ip, &dwErr)) != eck::InitStatus::Ok)
	{
		EckDbgPrintFormatMessage(dwErr);
		eck::MsgBox(eck::Format(L"Initialize failed!\nInitStatus = %d\nError code = %08X",
			(int)iRetInit, dwErr), L"Error", MB_ICONERROR);
		return 0;
	}

	App = new CApp{};
	App->Initialize();
	const auto pWnd = new CWndMain{};
	pWnd->Create(L"示例Win32程序", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, 0,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT, NULL, 0);
	pWnd->Visible = TRUE;

	MSG msg;
	while (GetMessageW(&msg, NULL, 0, 0))
	{
		if (!eck::PreTranslateMessage(msg))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}
	delete pWnd;
	delete App;
	eck::ThreadUninitialize();
	eck::Uninitialize();
	CoUninitialize();
	return (int)msg.wParam;
}