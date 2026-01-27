#pragma once
#include "CRefStr.h"
#include "ProcThrdHelper.h"

ECK_NAMESPACE_BEGIN
// ======================================
// https://github.com/killtimer0/uiaccess
// ======================================

// 仅Win8+

struct UIA
{
    HANDLE hTokenWinlogon;
};

inline BOOL UiaIsAcquired() noexcept
{
    DWORD dw;
    ULONG cbRet;
    if (NT_SUCCESS(NtQueryInformationToken(NtCurrentProcessToken(),
        TokenUIAccess, &dw, sizeof(dw), &cbRet)))
        return !!dw;
    return FALSE;
}

inline NTSTATUS UiaTryAcquire(_Out_ UIA& uia) noexcept
{
    NTSTATUS nts;
    DWORD dwSid;
    ULONG cbRet;
    // 取当前会话ID
    if (!NT_SUCCESS(nts = NtQueryInformationToken(NtCurrentProcessToken(),
        TokenSessionId, &dwSid, sizeof(dwSid), &cbRet)))
        return nts;
    // 克隆winlogon.exe的令牌以取得SeTcbPrivilege
    constexpr static WCHAR szWinlogon[]{ L"winlogon.exe" };
    PRIVILEGE_SET ps
    {
        .PrivilegeCount = 1,
        .Control = PRIVILEGE_SET_ALL_NECESSARY
    };
    UNICODE_STRING TcbPrivilege RTL_CONSTANT_STRING(SE_TCB_NAME);
    OBJECT_ATTRIBUTES oaLsa{ sizeof(oaLsa) };

    HANDLE hPolicy;
    if (!NT_SUCCESS(nts = LsaOpenPolicy(nullptr, &oaLsa,
        POLICY_LOOKUP_NAMES, &hPolicy)))
        return nts;
    if (!NT_SUCCESS(nts = LsaLookupPrivilegeValue(
        hPolicy, &TcbPrivilege, &ps.Privilege[0].Luid)))
    {
        LsaClose(hPolicy);
        return nts;
    }
    LsaClose(hPolicy);

    nts = EnumerateProcess([&](SYSTEM_PROCESS_INFORMATION* pspi)
        {
            if (!pspi->ImageName.Buffer)
                return TRUE;
            const auto cch = pspi->ImageName.Length / sizeof(WCHAR);
            if (ARRAYSIZE(szWinlogon) - 1 != cch ||
                !TcsEqualLenI(szWinlogon, pspi->ImageName.Buffer, cch))
                return TRUE;
            // 打开进程
            const auto hProcess = NaOpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,
                FALSE, HandleToULong(pspi->UniqueProcessId));
            if (!hProcess)
                return TRUE;
            // 打开令牌
            HANDLE hToken;
            NtOpenProcessTokenEx(hProcess, TOKEN_QUERY | TOKEN_DUPLICATE,
                0, &hToken);
            NtClose(hProcess);
            if (!hToken)
                return TRUE;
            // 检查
            BOOLEAN bResult;
            if (!NT_SUCCESS(NtPrivilegeCheck(hToken, &ps,
                &bResult)) || !bResult)
            {
                NtClose(hToken);
                return TRUE;
            }
            DWORD dwWinlogonSid;
            if (!NT_SUCCESS(NtQueryInformationToken(hToken,
                TokenSessionId, &dwWinlogonSid, sizeof(dwWinlogonSid),
                &cbRet)) || dwWinlogonSid != dwSid)
            {
                NtClose(hToken);
                return TRUE;
            }
            // It's OK...
            SECURITY_QUALITY_OF_SERVICE sqos
            {
                .Length = sizeof(SECURITY_QUALITY_OF_SERVICE),
                .ImpersonationLevel = SecurityImpersonation,
                .ContextTrackingMode = SECURITY_DYNAMIC_TRACKING,
                .EffectiveOnly = FALSE
            };
            OBJECT_ATTRIBUTES oa
            {
                .Length = sizeof(OBJECT_ATTRIBUTES),
                .SecurityQualityOfService = &sqos,
            };
            NtDuplicateToken(hToken, TOKEN_IMPERSONATE, &oa,
                FALSE, TokenImpersonation, &uia.hTokenWinlogon);
            NtClose(hToken);
            return FALSE;
        });
    if (!NT_SUCCESS(nts))
        return nts;
    if (!uia.hTokenWinlogon)
        return STATUS_ACCESS_DENIED;
    return STATUS_SUCCESS;
}

EckInline void UiaRelease(const UIA& uia) noexcept
{
    if (uia.hTokenWinlogon)
        NtClose(uia.hTokenWinlogon);
}

inline NTSTATUS UiaRestart(const UIA& uia) noexcept
{
    NTSTATUS nts;
    if (!NT_SUCCESS(nts = NtSetInformationThread(NtCurrentThread(),
        ThreadImpersonationToken, (void*)&uia.hTokenWinlogon, sizeof(uia.hTokenWinlogon))))
        return nts;
    HANDLE hTokenUia;
    SECURITY_QUALITY_OF_SERVICE sqos
    {
        .Length = sizeof(SECURITY_QUALITY_OF_SERVICE),
        .ImpersonationLevel = SecurityAnonymous,
        .ContextTrackingMode = SECURITY_DYNAMIC_TRACKING,
        .EffectiveOnly = FALSE
    };
    OBJECT_ATTRIBUTES oa
    {
        .Length = sizeof(OBJECT_ATTRIBUTES),
        .SecurityQualityOfService = &sqos,
    };

    HANDLE hTokenCurr;
    // 伪句柄无法被克隆
    nts = NtOpenProcessTokenEx(NtCurrentProcess(), TOKEN_ALL_ACCESS, 0, &hTokenCurr);
    if (!NT_SUCCESS(nts))
        return nts;
    nts = NtDuplicateToken(hTokenCurr, TOKEN_ALL_ACCESS, &oa,
        FALSE, TokenPrimary, &hTokenUia);
    NtClose(hTokenCurr);
    if (!NT_SUCCESS(nts))
        return nts;
    DWORD dw{ 1 };
    nts = NtSetInformationToken(hTokenUia, TokenUIAccess, &dw, sizeof(dw));
    if (!NT_SUCCESS(nts))
    {
        NtClose(hTokenUia);
        return nts;
    }
    // 启动新实例
    STARTUPINFOW si{ sizeof(si) };
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = (WORD)NtCurrentPeb()->ProcessParameters->ShowWindowFlags;
    PROCESS_INFORMATION pi;
    CRefStrW rsCmdLine{ NtCurrentPeb()->ProcessParameters->CommandLine };
    PWSTR pszAppName, pszCmdLine;
    int cchAppName, cchCmdLine;
    rsCmdLine.PazParseCommandLineAndCut(pszAppName, cchAppName,
        pszCmdLine, cchCmdLine);
    if (CreateProcessAsUserW(hTokenUia, pszAppName, pszCmdLine,
        nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi))
    {
        NtClose(pi.hThread);
        NtClose(pi.hProcess);
        NtClose(hTokenUia);
        UiaRelease(uia);
        exit(0);
    }
    return NTSTATUS_FROM_WIN32(NaGetLastError());
}
ECK_NAMESPACE_END