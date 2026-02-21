#pragma once
#include "CString.h"
#include "ProcThrdHelper.h"
#include "CNtObject.h"

ECK_NAMESPACE_BEGIN
// https://github.com/killtimer0/uiaccess

class CUiAccess
{
private:
    CNtObject m_hTokenWinlogon;
    CNtObject m_hTokenCurrProc;
public:
    ECK_DISABLE_COPY_MOVE(CUiAccess);
    CUiAccess() noexcept
    {
        NtOpenProcessTokenEx(NtCurrentProcess(),
            TOKEN_ALL_ACCESS, 0, m_hTokenCurrProc.AddrOfClear());
    }

    BOOL IsAcquired() const noexcept
    {
        UINT u;
        ULONG cbRet;
        if (NT_SUCCESS(NtQueryInformationToken(m_hTokenCurrProc.Get(),
            TokenUIAccess, &u, sizeof(u), &cbRet)))
            return !!u;
        return FALSE;
    }

    NTSTATUS TryAcquire() noexcept
    {
        NTSTATUS nts;
        UINT uSid;
        ULONG cbRet;
        // 取当前会话ID
        if (!NT_SUCCESS(nts = NtQueryInformationToken(m_hTokenCurrProc.Get(),
            TokenSessionId, &uSid, sizeof(uSid), &cbRet)))
            return nts;
        // 克隆winlogon.exe令牌
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
                    !TcsEqualLengthI(szWinlogon, pspi->ImageName.Buffer, cch))
                    return TRUE;
                // 打开进程
                CNtObject hProcess;
                hProcess.Attach(NaOpenProcess(
                    PROCESS_QUERY_LIMITED_INFORMATION,
                    FALSE,
                    HandleToULong(pspi->UniqueProcessId),
                    &nts));
                if (!hProcess.Get())
                    return TRUE;
                // 打开令牌
                CNtObject hToken;
                nts = NtOpenProcessTokenEx(
                    hProcess.Get(),
                    TOKEN_QUERY | TOKEN_DUPLICATE,
                    0,
                    hToken.AddrOf());
                if (!hToken.Get())
                    return TRUE;
                // 检查
                BOOLEAN bResult;
                nts = NtPrivilegeCheck(hToken.Get(), &ps, &bResult);
                if (!NT_SUCCESS(nts) || !bResult)
                    return TRUE;
                UINT uWinlogonSid;
                nts = NtQueryInformationToken(hToken.Get(), TokenSessionId,
                    &uWinlogonSid, sizeof(uWinlogonSid), &cbRet);
                if (!NT_SUCCESS(nts) || uWinlogonSid != uSid)
                    return TRUE;
                // 克隆
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
                NtDuplicateToken(hToken.Get(), TOKEN_IMPERSONATE, &oa,
                    FALSE, TokenImpersonation, m_hTokenWinlogon.AddrOfClear());
                return FALSE;
            });
        if (!NT_SUCCESS(nts))
            return nts;
        if (!m_hTokenWinlogon.Get())
            return STATUS_ACCESS_DENIED;
        return STATUS_SUCCESS;
    }

    NTSTATUS Restart() noexcept
    {
        NTSTATUS nts;
        nts = NtSetInformationThread(
            NtCurrentThread(),
            ThreadImpersonationToken,
            m_hTokenWinlogon.AddrOf(),
            sizeof(HANDLE));
        if (!NT_SUCCESS(nts))
            return nts;
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

        CNtObject hTokenUia;
        nts = NtDuplicateToken(
            m_hTokenCurrProc.Get(),
            TOKEN_ALL_ACCESS,
            &oa,
            FALSE,
            TokenPrimary,
            hTokenUia.AddrOf());
        if (!NT_SUCCESS(nts))
            return nts;
        UINT u{ 1 };
        nts = NtSetInformationToken(hTokenUia.Get(), TokenUIAccess, &u, sizeof(u));
        if (!NT_SUCCESS(nts))
            return nts;
        // 启动新实例
        STARTUPINFOW si{ sizeof(si) };
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = (WORD)NtCurrentPeb()->ProcessParameters->ShowWindowFlags;
        PROCESS_INFORMATION pi;
        CStringW rsCmdLine{ NtCurrentPeb()->ProcessParameters->CommandLine };
        PWSTR pszAppName, pszCmdLine;
        int cchAppName, cchCmdLine;
        rsCmdLine.PazParseCommandLineAndCut(pszAppName, cchAppName,
            pszCmdLine, cchCmdLine);
        if (CreateProcessAsUserW(hTokenUia.Get(), pszAppName, pszCmdLine,
            nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi))
        {
            NtClose(pi.hThread);
            NtClose(pi.hProcess);
            exit(0);
        }
        return NTSTATUS_FROM_WIN32(NaGetLastError());
    }
};
ECK_NAMESPACE_END