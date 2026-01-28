#include "pch.h"

#include "CApp.h"
#include "CWndMain.h"

#include "eck\Env.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
    _In_ PWSTR pszCmdLine, _In_ int nCmdShow)
{
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
    const auto hr = CoInitialize(nullptr);
    if (FAILED(hr))
    {
        EckDbgPrintFormatMessage(hr);
        eck::MsgBox(eck::Format(L"CoInitialize failed: 0x%08X", hr), L"", MB_ICONERROR);
        return 0;
    }

    UINT uErr;
    const auto eInitRet = eck::Initialize(hInstance, nullptr, &uErr);
    if (eInitRet != eck::InitStatus::Ok)
    {
        EckDbgPrintFormatMessage(uErr);
        eck::MsgBox(eck::Format(L"Initialize failed: %d(0x%08X)", (int)eInitRet, uErr), L"", MB_ICONERROR);
        return 0;
    }

    App = new CApp{};

    const auto pWnd = new CWndMain{};
    const auto hMon = eck::GetOwnerMonitor(nullptr);
    const auto iDpi = eck::GetMonitorDpi(hMon);
    auto size = SIZE{ 640,480 };
    DpiScale(size, iDpi);
    const auto pt = eck::CalculateCenterWindowPosition(nullptr, size.cx, size.cy, FALSE);
    pWnd->SetUserDpi(iDpi);
    pWnd->SetPresentMode(Dui::PresentMode::DCompositionSurface);
    pWnd->Create(L"示例Win32程序", WS_OVERLAPPEDWINDOW, 0,
        pt.x, pt.y, size.cx, size.cy, nullptr, 0);
    pWnd->Visible = TRUE;

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0))
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