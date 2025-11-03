#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
EckInline void* VAlloc(SIZE_T cb, ULONG ulProtect)
{
    void* p{};
    (void)NtAllocateVirtualMemory(NtCurrentProcess(), &p, 0, &cb, MEM_COMMIT, ulProtect);
    return p;
}
EckInline void* VAlloc(SIZE_T cb)
{
    void* p{};
    (void)NtAllocateVirtualMemory(NtCurrentProcess(), &p, 0, &cb, MEM_COMMIT, PAGE_READWRITE);
    return p;
}
EckInline NTSTATUS VFree(void* p)
{
    SIZE_T cb{};
    return NtFreeVirtualMemory(NtCurrentProcess(), &p, &cb, MEM_RELEASE);
}

template<class T_ = void>
struct DelVA
{
    using T = T_;
    void operator()(T* p) { VFree(p); }
};

inline HANDLE NaOpenProcess(DWORD dwDesiredAccess, BOOL bInheritHandle,
    UINT uProcessId, NTSTATUS* pnts = nullptr)
{
    OBJECT_ATTRIBUTES oa
    {
        .Length = sizeof(oa),
        .Attributes = ULONG(bInheritHandle ? OBJ_INHERIT : 0)
    };
    CLIENT_ID cid{ ULongToHandle(uProcessId) };
    HANDLE hProcess;
    NTSTATUS nts;
    nts = NtOpenProcess(&hProcess, dwDesiredAccess, &oa, &cid);
    if (pnts) *pnts = nts;
    return hProcess;
}

inline HANDLE NaOpenFile(_In_ const UNICODE_STRING* pusFile,
    DWORD dwAccess, DWORD dwShareMode, DWORD dwOptions = 0u,
    _Out_opt_ NTSTATUS* pnts = nullptr,
    _Out_opt_ IO_STATUS_BLOCK* piost = nullptr,
    BOOL bInheritHandle = FALSE, HANDLE hRootDirectory = nullptr)
{
    OBJECT_ATTRIBUTES oa;
    InitializeObjectAttributes(&oa, pusFile, OBJ_CASE_INSENSITIVE, hRootDirectory, nullptr);
    HANDLE hFile;
    NTSTATUS nts;
    IO_STATUS_BLOCK iost;
    nts = NtOpenFile(&hFile, dwAccess, &oa, piost ? piost : &iost, dwShareMode, dwOptions);
    if (pnts)
        *pnts = nts;
    if (NT_SUCCESS(nts))
        return hFile;
    return INVALID_HANDLE_VALUE;
}

inline HANDLE NaOpenFile(_In_z_ PCWSTR pszFile,
    DWORD dwAccess, DWORD dwShareMode, DWORD dwOptions = 0u,
    _Out_opt_ NTSTATUS* pnts = nullptr,
    _Out_opt_ IO_STATUS_BLOCK* piost = nullptr,
    BOOL bInheritHandle = FALSE, HANDLE hRootDirectory = nullptr)
{
    UNICODE_STRING usFile;
    if (hRootDirectory)
        RtlInitUnicodeString(&usFile, pszFile);
    else
        if (!RtlDosPathNameToNtPathName_U(pszFile, &usFile, nullptr, nullptr))
        {
            if (pnts)
                *pnts = STATUS_OBJECT_PATH_NOT_FOUND;
            if (piost)
            {
                piost->Status = STATUS_OBJECT_PATH_NOT_FOUND;
                piost->Information = 0;
            }
            return INVALID_HANDLE_VALUE;
        }
    const auto hFile = NaOpenFile(&usFile, dwAccess, dwShareMode, dwOptions,
        pnts, piost, bInheritHandle, hRootDirectory);
    if (!hRootDirectory)
        RtlFreeHeap(RtlProcessHeap(), 0, usFile.Buffer);
    return hFile;
}

inline HANDLE NaCreateFile(_In_ const UNICODE_STRING* pusFile,
    DWORD dwAccess, DWORD dwShareMode, DWORD dwOptions, DWORD dwCreationDisposition,
    _Out_opt_ NTSTATUS* pnts = nullptr,
    _Out_opt_ IO_STATUS_BLOCK* piost = nullptr,
    DWORD dwAttributes = FILE_ATTRIBUTE_NORMAL, ULONGLONG cbInit = 0ll,
    BOOL bInheritHandle = FALSE, HANDLE hRootDirectory = nullptr)
{
    OBJECT_ATTRIBUTES oa;
    InitializeObjectAttributes(&oa, pusFile, OBJ_CASE_INSENSITIVE, hRootDirectory, nullptr);
    HANDLE hFile;
    NTSTATUS nts;
    IO_STATUS_BLOCK iost;
    LARGE_INTEGER li{ .QuadPart = (LONGLONG)cbInit };
    nts = NtCreateFile(&hFile, dwAccess, &oa,
        piost ? piost : &iost,
        cbInit ? &li : nullptr,
        dwAttributes, dwShareMode, dwCreationDisposition, dwOptions, nullptr, 0);
    if (pnts)
        *pnts = nts;
    if (NT_SUCCESS(nts))
        return hFile;
    return INVALID_HANDLE_VALUE;
}

inline HANDLE NaCreateFile(_In_z_ PCWSTR pszFile,
    DWORD dwAccess, DWORD dwShareMode, DWORD dwOptions, DWORD dwCreationDisposition,
    _Out_opt_ NTSTATUS* pnts = nullptr,
    _Out_opt_ IO_STATUS_BLOCK* piost = nullptr,
    DWORD dwAttributes = FILE_ATTRIBUTE_NORMAL, ULONGLONG cbInit = 0ll,
    BOOL bInheritHandle = FALSE)
{
    UNICODE_STRING usFile;
    if (!RtlDosPathNameToNtPathName_U(pszFile, &usFile, nullptr, nullptr))
    {
        if (pnts)
            *pnts = STATUS_OBJECT_PATH_NOT_FOUND;
        if (piost)
        {
            piost->Status = STATUS_OBJECT_PATH_NOT_FOUND;
            piost->Information = 0;
        }
        return INVALID_HANDLE_VALUE;
    }
    const auto hFile = NaCreateFile(&usFile, dwAccess, dwShareMode, dwOptions,
        dwCreationDisposition, pnts, piost, dwAttributes, cbInit, bInheritHandle);
    RtlFreeHeap(RtlProcessHeap(), 0, usFile.Buffer);
    return hFile;
}

inline NTSTATUS NaDeviceIoControl(HANDLE hDevice, DWORD dwIoControlCode,
    _In_reads_bytes_opt_(cbInBuf) PVOID pInBuf, DWORD cbInBuf,
    _Out_writes_bytes_opt_(cbOutBuf) PVOID pOutBuf, DWORD cbOutBuf,
    _Out_opt_  DWORD* pcbReturned = nullptr)
{
    NTSTATUS nts;
    IO_STATUS_BLOCK iosb;
    if ((dwIoControlCode >> 16) == FILE_DEVICE_FILE_SYSTEM)
    {
        nts = NtFsControlFile(hDevice, nullptr, nullptr, nullptr, &iosb,
            dwIoControlCode, pInBuf, cbInBuf, pOutBuf, cbOutBuf);
    }
    else
    {
        nts = NtDeviceIoControlFile(hDevice, nullptr, nullptr, nullptr, &iosb,
            dwIoControlCode, pInBuf, cbInBuf, pOutBuf, cbOutBuf);
    }

    if (nts == STATUS_PENDING)
    {
        NtWaitForSingleObject(hDevice, FALSE, nullptr);
        nts = iosb.Status;
    }
    if (pcbReturned)
        *pcbReturned = (DWORD)iosb.Information;
    return nts;
}

EckInlineNd ULONG NaGetLastError() { return NtCurrentTeb()->LastErrorValue; }

EckInlineNd PCWSTR NaGetNtSystemRoot()
{
    if (g_pfnRtlGetNtSystemRoot)
        return g_pfnRtlGetNtSystemRoot();
    return USER_SHARED_DATA->NtSystemRoot;
}

EckInlineNd HANDLE NaGetStandardOutput() { return NtCurrentPeb()->ProcessParameters->StandardOutput; }
EckInlineNd HANDLE NaGetStandardInput() { return NtCurrentPeb()->ProcessParameters->StandardInput; }
EckInlineNd HANDLE NaGetStandardError() { return NtCurrentPeb()->ProcessParameters->StandardError; }
ECK_NAMESPACE_END