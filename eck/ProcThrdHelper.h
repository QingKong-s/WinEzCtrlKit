#pragma once
#include "FileHelper.h"

ECK_NAMESPACE_BEGIN
EckInline NTSTATUS GetProcessPeb(HANDLE hProcess, _Out_ void*& Peb) noexcept
{
    PROCESS_BASIC_INFORMATION pbi;
    NTSTATUS nts;
    if (!NT_SUCCESS(nts = NtQueryInformationProcess(hProcess,
        ProcessBasicInformation, &pbi, sizeof(pbi), nullptr)))
    {
        Peb = nullptr;
        return nts;
    }
    Peb = pbi.PebBaseAddress;
    return STATUS_SUCCESS;
}

EckInline NTSTATUS GetProcessPeb32(HANDLE hProcess, _Out_ void*& Peb) noexcept
{
    return NtQueryInformationProcess(hProcess,
        ProcessWow64Information, &Peb, sizeof(Peb), nullptr);
}

EckInline NTSTATUS GetProcessPeb64(HANDLE hProcess, _Out_ ULONG64& Peb) noexcept
{
#ifndef _WIN64
    PROCESS_BASIC_INFORMATION64 pbi;
    NTSTATUS nts;
    if (!NT_SUCCESS(nts = NtWow64QueryInformationProcess64(hProcess,
        ProcessBasicInformation, &pbi, sizeof(pbi), nullptr)))
    {
        Peb = 0u;
        return nts;
    }
    Peb = pbi.PebBaseAddress;
    return STATUS_SUCCESS;
#else
    Peb = 0u;
    return STATUS_NOT_SUPPORTED;
#endif// !defined(_WIN64)
}

inline NTSTATUS GetProcessPath(UINT uPid, CStringW& rsPath, BOOL bDosPath = TRUE) noexcept
{
    SYSTEM_PROCESS_ID_INFORMATION spii{ .ProcessId = DwordToPtr<HANDLE>(uPid) };
    NTSTATUS nts = NtQuerySystemInformation(SystemProcessIdInformation,
        &spii, sizeof(spii), nullptr);
    if (spii.ImageName.MaximumLength &&
        nts == STATUS_INFO_LENGTH_MISMATCH)
    {
        rsPath.ReSize(spii.ImageName.MaximumLength / sizeof(WCHAR));
        spii.ImageName.Buffer = rsPath.Data();
        spii.ImageName.Length = 0;
        nts = NtQuerySystemInformation(SystemProcessIdInformation,
            &spii, sizeof(spii), nullptr);
        if (NT_SUCCESS(nts))
        {
            if (bDosPath)
                return FileNtPathToDosPath(rsPath);
            else
                return STATUS_SUCCESS;
        }
    }
    rsPath.Clear();
    return nts;
}

inline NTSTATUS GetProcessPath(HANDLE hProcess,
    CStringW& rsPath, BOOL bDosPath = TRUE) noexcept
{
    rsPath.ReSize(MAX_PATH + sizeof(UNICODE_STRING)/* 多一点无所谓 */);
    ULONG cbReal;
    auto nts = NtQueryInformationProcess(hProcess,
        bDosPath ? ProcessImageFileNameWin32 : ProcessImageFileName,
        rsPath.Data(), (ULONG)rsPath.ByteSizePure(), &cbReal);
    if (nts == STATUS_INFO_LENGTH_MISMATCH)
    {
        rsPath.ReSize((cbReal + 2/* Space */) / sizeof(WCHAR));
        nts = NtQueryInformationProcess(hProcess,
            bDosPath ? ProcessImageFileNameWin32 : ProcessImageFileName,
            rsPath.Data(), (ULONG)rsPath.ByteSizePure(), nullptr);
    }

    const auto pus = (UNICODE_STRING*)rsPath.Data();
    if (NT_SUCCESS(nts) && !RtlIsNullOrEmptyUnicodeString(pus))
    {
        TcsMoveLengthEnd(rsPath.Data(),
            PCWCH((BYTE*)rsPath.Data() + sizeof(UNICODE_STRING)),
            pus->Length / sizeof(WCHAR));
        rsPath.ReSize(pus->Length / sizeof(WCHAR));
        if (bDosPath)
            return FileNtPathToDosPath(rsPath);
        return STATUS_SUCCESS;
    }

    rsPath.Clear();
    return nts;
}

struct MODULE_INFO
{
    CStringW rsModuleName{};
    CStringW rsModulePath{};
#ifdef _WIN64
    void* BaseAddress{};
#else
    union
    {
        void* BaseAddress;
        ULONG64 BaseAddress64{};
    };
#endif// !defined(_WIN64)
    SIZE_T cbImage{};
};

/// <summary>
/// 枚举进程模块
/// </summary>
/// <param name="hProcess">进程句柄，必须具有PROCESS_QUERY_INFORMATION | PROCESS_VM_READ权限</param>
/// <param name="vResult">枚举结果，不会清空该容器</param>
/// <returns>NTSTATUS</returns>
inline NTSTATUS EnumerateProcessModules(
    HANDLE hProcess, std::vector<MODULE_INFO>& vResult) noexcept
{
    NTSTATUS nts;
    // 取PEB
    UINT_PTR pPeb;
    if (!NT_SUCCESS(nts = GetProcessPeb(hProcess, (void*&)pPeb)))
        return nts;
    // 取PEB_LDR_DATA
    PEB_LDR_DATA LdrData;
    UINT_PTR pLdr;
    if (!NT_SUCCESS(nts = NtReadVirtualMemory(hProcess, (void*)(pPeb + offsetof(PEB, Ldr)),
        &pLdr, sizeof(pLdr), nullptr)))
        return nts;
    if (!NT_SUCCESS(nts = NtReadVirtualMemory(hProcess, (void*)pLdr,
        &LdrData, sizeof(LdrData), nullptr)))
        return nts;
    if (!LdrData.Initialized)
        return STATUS_UNSUCCESSFUL;

    LDR_DATA_TABLE_ENTRY Entry;// 不要使用Win7之后添加的字段
    UINT_PTR pBegin = pLdr + offsetof(PEB_LDR_DATA, InLoadOrderModuleList);
    for (UINT_PTR p = (UINT_PTR)LdrData.InLoadOrderModuleList.Flink; p != pBegin; )
    {
        if (!NT_SUCCESS(nts = NtReadVirtualMemory(hProcess, (void*)p,
            &Entry, LDR_DATA_TABLE_ENTRY_SIZE_WIN7, nullptr)))
            return nts;
        if (Entry.DllBase)
        {
            auto& e = vResult.emplace_back();
            if (!RtlIsNullOrEmptyUnicodeString(&Entry.BaseDllName))
            {
                const int cch = Entry.BaseDllName.Length / sizeof(WCHAR);
                e.rsModuleName.ReSize(cch);
                if (!NT_SUCCESS(nts = NtReadVirtualMemory(
                    hProcess,
                    (void*)Entry.BaseDllName.Buffer,
                    e.rsModuleName.Data(),
                    cch * sizeof(WCHAR),
                    nullptr)))
                    return nts;
            }
            if (!RtlIsNullOrEmptyUnicodeString(&Entry.FullDllName))
            {
                const int cch = Entry.FullDllName.Length / sizeof(WCHAR);
                e.rsModulePath.ReSize(cch);
                if (!NT_SUCCESS(nts = NtReadVirtualMemory(hProcess, (void*)Entry.FullDllName.Buffer,
                    e.rsModulePath.Data(), cch * sizeof(WCHAR), nullptr)))
                    return nts;
            }
            e.BaseAddress = (void*)Entry.DllBase;
            e.cbImage = Entry.SizeOfImage;
        }
        p = (UINT_PTR)Entry.InLoadOrderLinks.Flink;
    }
    return STATUS_SUCCESS;
}

/// <summary>
/// 枚举64位进程模块。
/// 若当前进程不为32位则返回STATUS_NOT_SUPPORTED
/// </summary>
/// <param name="hProcess">进程句柄，必须具有PROCESS_QUERY_INFORMATION | PROCESS_VM_READ权限</param>
/// <param name="vResult">枚举结果，不会清空该容器</param>
/// <returns>NTSTATUS</returns>
inline NTSTATUS EnumerateProcessModules64On32(
    HANDLE hProcess, std::vector<MODULE_INFO>& vResult) noexcept
{
#ifndef _WIN64
    NTSTATUS nts;
    // 取PEB
    ULONG64 pPeb;
    if (!NT_SUCCESS(nts = GetProcessPeb64(hProcess, pPeb)))
        return nts;
    // 取PEB_LDR_DATA
    PEB_LDR_DATA64 LdrData;
    ULONG64 pLdr;
    if (!NT_SUCCESS(nts = NtWow64ReadVirtualMemory64(hProcess, (pPeb + offsetof(PEB64, Ldr)),
        &pLdr, 8, nullptr)))
        return nts;
    if (!NT_SUCCESS(nts = NtWow64ReadVirtualMemory64(hProcess, pLdr,
        &LdrData, sizeof(LdrData), nullptr)))
        return nts;
    if (!LdrData.Initialized)
        return STATUS_UNSUCCESSFUL;

    LDR_DATA_TABLE_ENTRY64 Entry;// 不要使用Win7之后添加的字段
    ULONG64 pBegin = pLdr + offsetof(PEB_LDR_DATA64, InLoadOrderModuleList);
    for (ULONG64 p = (ULONG64)LdrData.InLoadOrderModuleList.Flink; p != pBegin; )
    {
        if (!NT_SUCCESS(nts = NtWow64ReadVirtualMemory64(hProcess, p,
            &Entry, LDR_DATA_TABLE_ENTRY_SIZE_WIN7_64, nullptr)))
            return nts;
        if (Entry.DllBase)
        {
            auto& e = vResult.emplace_back();
            if (Entry.BaseDllName.Length)
            {
                const int cch = Entry.BaseDllName.Length / sizeof(WCHAR);
                e.rsModuleName.ReSize(cch);
                if (!NT_SUCCESS(nts = NtWow64ReadVirtualMemory64(
                    hProcess, Entry.BaseDllName.Buffer,
                    e.rsModuleName.Data(), cch * sizeof(WCHAR), nullptr)))
                    return nts;
            }
            if (Entry.FullDllName.Length)
            {
                const int cch = Entry.FullDllName.Length / sizeof(WCHAR);
                e.rsModulePath.ReSize(cch);
                if (!NT_SUCCESS(nts = NtWow64ReadVirtualMemory64(
                    hProcess, Entry.FullDllName.Buffer,
                    e.rsModulePath.Data(), cch * sizeof(WCHAR), nullptr)))
                    return nts;
            }
            e.BaseAddress64 = Entry.DllBase;
            e.cbImage = Entry.SizeOfImage;
        }
        p = (ULONG64)Entry.InLoadOrderLinks.Flink;
    }
    return STATUS_SUCCESS;
#else
    return STATUS_NOT_SUPPORTED;
#endif// !defined(_WIN64)
}

/// <summary>
/// 枚举32位进程模块。
/// 若当前进程不为64位则返回STATUS_NOT_SUPPORTED
/// </summary>
/// <param name="hProcess">进程句柄，必须具有PROCESS_QUERY_INFORMATION | PROCESS_VM_READ权限</param>
/// <param name="vResult">枚举结果，不会清空该容器</param>
/// <returns>NTSTATUS</returns>
inline NTSTATUS EnumerateProcessModules32On64(
    HANDLE hProcess, std::vector<MODULE_INFO>& vResult) noexcept
{
#ifdef _WIN64
    NTSTATUS nts;
    // 取PEB
    UINT_PTR pPeb;
    if (!NT_SUCCESS(nts = GetProcessPeb32(hProcess, (void*&)pPeb)))
        return nts;
    // 取PEB_LDR_DATA
    PEB_LDR_DATA32 LdrData;
    UINT_PTR pLdr{};
    if (!NT_SUCCESS(nts = NtReadVirtualMemory(hProcess, (void*)(pPeb + offsetof(PEB32, Ldr)),
        &pLdr, 4, nullptr)))
        return nts;
    if (!NT_SUCCESS(nts = NtReadVirtualMemory(hProcess, (void*)pLdr,
        &LdrData, sizeof(LdrData), nullptr)))
        return nts;
    if (!LdrData.Initialized)
        return STATUS_UNSUCCESSFUL;

    LDR_DATA_TABLE_ENTRY32 Entry;// 不要使用Win7之后添加的字段
    UINT_PTR pBegin = pLdr + offsetof(PEB_LDR_DATA32, InLoadOrderModuleList);
    for (UINT_PTR p = (UINT_PTR)LdrData.InLoadOrderModuleList.Flink; p != pBegin; )
    {
        if (!NT_SUCCESS(nts = NtReadVirtualMemory(hProcess, (void*)p,
            &Entry, LDR_DATA_TABLE_ENTRY_SIZE_WIN7_32, nullptr)))
            return nts;
        if (Entry.DllBase)
        {
            auto& e = vResult.emplace_back();
            if (Entry.BaseDllName.Length)
            {
                const int cch = Entry.BaseDllName.Length / sizeof(WCHAR);
                e.rsModuleName.ReSize(cch);
                if (!NT_SUCCESS(nts = NtReadVirtualMemory(hProcess,
                    ULongToPtr(Entry.BaseDllName.Buffer),
                    e.rsModuleName.Data(), cch * sizeof(WCHAR), nullptr)))
                    return nts;
            }
            if (Entry.FullDllName.Length)
            {
                const int cch = Entry.FullDllName.Length / sizeof(WCHAR);
                e.rsModulePath.ReSize(cch);
                if (!NT_SUCCESS(nts = NtReadVirtualMemory(hProcess,
                    ULongToPtr(Entry.FullDllName.Buffer),
                    e.rsModulePath.Data(), cch * sizeof(WCHAR), nullptr)))
                    return nts;
            }
            e.BaseAddress = ULongToPtr(Entry.DllBase);
            e.cbImage = Entry.SizeOfImage;
        }
        EckDbgPrint(Entry.InLoadOrderLinks.Flink);
        p = (UINT_PTR)Entry.InLoadOrderLinks.Flink;
    }
    return STATUS_SUCCESS;
#else
    return STATUS_NOT_SUPPORTED;
#endif// defined(_WIN64)
}


struct THREAD_INFO
{
    UINT uTid;
    void* StartAddress;
    KPRIORITY Priority;
    KPRIORITY BasePriority;
};

struct PROCESS_INFO
{
    CStringW rsImageName;	// 进程名
    ULONG uPid;				// 进程ID
    ULONG uParentPid;		// 父进程ID
    ULONG cThreads;			// 线程数
    ULONG uSessionID;		// 会话ID
    ULONG cHandles;			// 句柄数
    ULONG cPageFaults;		// 页面错误数
    SIZE_T cbPrivateWorkingSet;			// 专用工作集
    SIZE_T cbWorkingSet;				// 工作集
    SIZE_T cbPeakWorkingSet;			// 峰值工作集
    SIZE_T cbQuotaPagedPoolUsage;		// 页面缓冲池
    SIZE_T cbPeakQuotaPagedPoolUsage;	// 峰值页面缓冲池
    SIZE_T cbQuotaNonPagedPoolUsage;	// 非页面缓冲池
    SIZE_T cbPeakQuotaNonPagedPoolUsage;// 峰值非页面缓冲池
    SIZE_T cbPageFileUsage;				// 已提交
    SIZE_T cbPeakPageFileUsage;			// 峰值已提交
    CStringW rsFilePath;	// 进程路径
    std::vector<THREAD_INFO> vThreads;	// 线程信息
    std::vector<MODULE_INFO> vModules;	// 模块信息
};

enum EPFLAGS :UINT
{
    EPF_NONE = 0u,
    EPF_THREAD_INFO = 1u << 0,
    EPF_PROCESS_PATH = 1u << 1,
    EPF_MODULE_INFO = 1u << 2,
};
ECK_ENUM_BIT_FLAGS(EPFLAGS);

/// <summary>
/// 枚举进程
/// </summary>
/// <param name="Fn">回调，参数：(SYSTEM_PROCESS_INFORMATION*)</param>
/// <returns>NTSTATUS</returns>
inline NTSTATUS EnumerateProcess(
    std::invocable<SYSTEM_PROCESS_INFORMATION*> auto&& Fn) noexcept
{
    NTSTATUS nts;
    ULONG cb{ 0x4000u };
    void* pBuf;
    EckLoop()
    {
        pBuf = VAlloc(cb);
        if (!pBuf)
            return STATUS_NO_MEMORY;
        nts = NtQuerySystemInformation(SystemProcessInformation, pBuf, cb, &cb);
        if (nts == STATUS_INFO_LENGTH_MISMATCH || nts == STATUS_BUFFER_TOO_SMALL)
        {
            VFree(pBuf);
            continue;
        }
        break;
    }

    auto pspi = (SYSTEM_PROCESS_INFORMATION*)pBuf;
    EckLoop()
    {
        EckCanCallbackContinue(Fn(pspi))
            break;
        if (pspi->NextEntryOffset == 0)
            break;
        pspi = PointerStepBytes(pspi, pspi->NextEntryOffset);
    }
    VFree(pBuf);
    return STATUS_SUCCESS;
}

/// <summary>
/// 枚举进程
/// </summary>
/// <param name="vResult">枚举结果，不会清空该容器</param>
/// <param name="uFlags">EPF_常量</param>
/// <returns>NTSTATUS</returns>
inline NTSTATUS EnumerateProcess(std::vector<PROCESS_INFO>& vResult,
    EPFLAGS uFlags = EPF_NONE) noexcept
{
    vResult.reserve(150u);
    return EnumerateProcess([&](SYSTEM_PROCESS_INFORMATION* pspi)
        {
            auto& e = vResult.emplace_back(
                pspi->ImageName,
                PtrToDword<ULONG>(pspi->UniqueProcessId),
                PtrToDword<ULONG>(pspi->InheritedFromUniqueProcessId),
                pspi->NumberOfThreads,
                pspi->SessionId,
                pspi->HandleCount,
                pspi->PageFaultCount,
                (SIZE_T)pspi->WorkingSetPrivateSize.QuadPart,
                pspi->WorkingSetSize,
                pspi->PeakWorkingSetSize,
                pspi->QuotaPagedPoolUsage,
                pspi->QuotaPeakPagedPoolUsage,
                pspi->QuotaNonPagedPoolUsage,
                pspi->QuotaPeakNonPagedPoolUsage,
                pspi->PagefileUsage,
                pspi->PeakPagefileUsage);

            if (uFlags & EPF_PROCESS_PATH)
                GetProcessPath(e.uPid, e.rsFilePath);

            if (uFlags & EPF_THREAD_INFO)
            {
                e.vThreads.resize(pspi->NumberOfThreads);
                SYSTEM_THREAD_INFORMATION* const pBegin = pspi->Threads;
                SYSTEM_THREAD_INFORMATION* const pEnd = pBegin + pspi->NumberOfThreads;
                for (auto p = pBegin; p < pEnd; ++p)
                {
                    auto& t = e.vThreads[p - pBegin];
                    t.uTid = PtrToDword<ULONG>(p->ClientId.UniqueThread);
                    t.StartAddress = p->StartAddress;
                    t.Priority = p->Priority;
                    t.BasePriority = p->BasePriority;
                }
            }

            if (uFlags & EPF_MODULE_INFO)
            {
                const auto hProcess = NaOpenProcess(
                    PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, e.uPid);
                if (hProcess)
                {
                    EnumerateProcessModules(hProcess, e.vModules);
                    NtClose(hProcess);
                }
            }
        });
}

EckInline NTSTATUS GetProcessIdByName(
    std::wstring_view svImage, _Out_ UINT& uPid) noexcept
{
    uPid = 0u;
    return EnumerateProcess([&](SYSTEM_PROCESS_INFORMATION* pspi)
        {
            if (pspi->ImageName.Length &&
                TcsCompareLength2I(svImage.data(), svImage.size(),
                    pspi->ImageName.Buffer, pspi->ImageName.Length / sizeof(WCHAR)) == 0)
            {
                uPid = PtrToDword<UINT>(pspi->UniqueProcessId);
                return FALSE;
            }
            return TRUE;
        });
}

EckInline NTSTATUS GetProcessIdByName(
    std::wstring_view svImage, std::vector<UINT>& vPid) noexcept
{
    vPid.clear();
    return EnumerateProcess([&](SYSTEM_PROCESS_INFORMATION* pspi)
        {
            if (pspi->ImageName.Length &&
                TcsCompareLength2I(svImage.data(), svImage.size(),
                    pspi->ImageName.Buffer, pspi->ImageName.Length / sizeof(WCHAR)) == 0)
                vPid.emplace_back(PtrToDword<UINT>(pspi->UniqueProcessId));
        });
}

/// <summary>
/// 调整进程令牌特权
/// </summary>
/// <param name="hProcess">进程句柄，必须具有PROCESS_QUERY_LIMITED_INFORMATION权限</param>
/// <param name="bEnable">是否启用</param>
/// <param name="svPrivilege">特权名</param>
/// <returns>NTSTATUS</returns>
inline NTSTATUS AdjustProcessPrivilege(HANDLE hProcess,
    BOOL bEnable, std::wstring_view svPrivilege) noexcept
{
    HANDLE hToken;
    NTSTATUS nts = NtOpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES, &hToken);
    if (!NT_SUCCESS(nts))
        return nts;

    UNICODE_STRING usPrivilege;
    usPrivilege.Length = usPrivilege.MaximumLength =
        USHORT(svPrivilege.size() * sizeof(WCHAR));
    usPrivilege.Buffer = (PWCH)svPrivilege.data();
    TOKEN_PRIVILEGES tp;
    if (!NT_SUCCESS(nts = LsaLookupPrivilegeValue(hToken,
        &usPrivilege, &tp.Privileges[0].Luid)))
    {
        NtClose(hToken);
        return nts;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Attributes = bEnable ? SE_PRIVILEGE_ENABLED : 0;
    nts = NtAdjustPrivilegesToken(hToken, FALSE, &tp, 0, nullptr, nullptr);
    NtClose(hToken);
    return nts;
}

[[nodiscard]] inline HICON GetWindowSmallIcon(HWND hWnd, int msTimeOut = 300) noexcept
{
    HICON hIcon;
    if (!SendMessageTimeoutW(hWnd, WM_GETICON, ICON_SMALL, 0,
        SMTO_ABORTIFHUNG | SMTO_BLOCK | SMTO_ERRORONEXIT, msTimeOut, (DWORD_PTR*)&hIcon))
        return nullptr;
    if (!hIcon)
    {
        hIcon = (HICON)GetClassLongPtrW(hWnd, GCLP_HICONSM);
        if (!hIcon)
        {
            if (!SendMessageTimeoutW(hWnd, WM_GETICON, ICON_SMALL, 0,
                SMTO_ABORTIFHUNG | SMTO_BLOCK | SMTO_ERRORONEXIT, msTimeOut, (DWORD_PTR*)&hIcon))
                return nullptr;
        }
    }
    return hIcon;
}

[[nodiscard]] inline HICON GetWindowLargeIcon(HWND hWnd, int msTimeOut = 300) noexcept
{
    HICON hIcon;
    if (!SendMessageTimeoutW(hWnd, WM_GETICON, ICON_BIG, 0,
        SMTO_ABORTIFHUNG | SMTO_BLOCK | SMTO_ERRORONEXIT, msTimeOut, (DWORD_PTR*)&hIcon))
        return nullptr;
    if (!hIcon)
        hIcon = (HICON)GetClassLongPtrW(hWnd, GCLP_HICON);
    return hIcon;
}

/// <summary>
/// 取窗口图标。
/// 此函数根据bSmall参数选择调用GetWindowSmallIcon或GetWindowLargeIcon，
/// 如果两者都失败，则尝试获取进程映像文件图标
/// </summary>
/// <param name="hWnd">窗口句柄</param>
/// <param name="bFileIcon">返回图标类型，若为文件图标则为TRUE</param>
/// <param name="bSmall">是否获取小图标</param>
/// <param name="msTimeOut">超时</param>
/// <returns>若成功返回图标句柄，失败返回nullptr</returns>
_Ret_maybenull_
[[nodiscard]] inline HICON GetWindowIcon(HWND hWnd,
    BOOL& bFileIcon, BOOL bSmall = FALSE, int msTimeOut = 300) noexcept
{
    bFileIcon = FALSE;
    const HICON hIcon = (bSmall ?
        GetWindowSmallIcon(hWnd, msTimeOut) :
        GetWindowLargeIcon(hWnd, msTimeOut));
    if (hIcon)
        return hIcon;

    DWORD dwPid;
    GetWindowThreadProcessId(hWnd, &dwPid);
    if (!dwPid)
        return nullptr;
    CStringW rsPath{};
    if (!NT_SUCCESS(GetProcessPath(dwPid, rsPath)))
        return nullptr;

    SHFILEINFOW sfi;
    const UINT uFlags = (bSmall ? (SHGFI_ICON | SHGFI_SMALLICON) : SHGFI_ICON);
#pragma warning(suppress:6001)
    if (!SHGetFileInfoW(rsPath.Data(), 0, &sfi, sizeof(sfi), uFlags))
        SHGetFileInfoW(rsPath.Data(), FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(sfi),
            uFlags | SHGFI_USEFILEATTRIBUTES);
    bFileIcon = !!sfi.hIcon;
    return sfi.hIcon;
}
ECK_NAMESPACE_END