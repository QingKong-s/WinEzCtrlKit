#include "pch.h"

#include "eck\Env.h"

Cio::CReader<char> ConIn{};
Cio::CWriter<char> ConOut{};
Cio::CWriter<char> ConErr{};

int wmain(int argc, WCHAR** argv)
{
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    ConIn.CloneFrom(eck::NaGetStandardInput());
    ConOut.CloneFrom(eck::NaGetStandardOutput());
    ConErr.CloneFrom(eck::NaGetStandardError());

    eck::INITPARAM ip{};
    ip.uFlags = eck::EIF_CONSOLE_APP;
    DWORD dwErr;
    const auto eInitRet = eck::Init(NtCurrentImageBaseHInst(), &ip, &dwErr);
    if (eInitRet != eck::InitStatus::Ok)
    {
        EckDbgPrintFormatMessage(dwErr);
        ConOut.PrintLine(L"Init failed: ",
            (int)eInitRet, "(0x", Cio::IoInt{ dwErr,16,8 }, ")");
        return 0;
    }

    ConOut.PrintLine(u8"请输入您的年龄：");
    int Age;
    ConIn.Scan(Age);
    ConOut.PrintLine(u8"您的年龄是：", Age);

    eck::UnInit();
    return 0;
}