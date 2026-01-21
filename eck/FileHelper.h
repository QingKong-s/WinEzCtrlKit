#pragma once
#include "NativeWrapper.h"
#include "CRefBin.h"
#include "CRefStr.h"
#include "AutoPtrDef.h"
#include "CFile.h"

ECK_NAMESPACE_BEGIN
inline LONGLONG FileGetSizeByPath(
    _In_z_ PCWSTR pszFile,
    _Out_opt_ NTSTATUS* pnts = nullptr) noexcept
{
    CFile File{};
    const auto nts = File.Create(pszFile, FILE_OPEN, FILE_READ_ATTRIBUTES, FILE_SHARE_READ);
    if (File.IsValid())
        return File.GetSize(pnts);
    if (pnts) *pnts = nts;
    return 0;
}

inline NTSTATUS ReadInFile(_In_z_ PCWSTR pszFile, CRefBin& rb) noexcept
{
    CFile File{};
    auto nts = File.Create(pszFile,
        FILE_OPEN,
        FILE_READ_DATA | SYNCHRONIZE,
        FILE_SHARE_READ,
        FILE_NON_DIRECTORY_FILE | FILE_SEQUENTIAL_ONLY | FILE_SYNCHRONOUS_IO_NONALERT);
    if (!File.IsValid())
        return nts;

    const auto cbFile = (size_t)File.GetSize(&nts);
    if (!NT_SUCCESS(nts))
        return nts;
    if (cbFile > (size_t)1'073'741'824u)// 大于1G，不读
        return STATUS_BUFFER_TOO_SMALL;
    rb.Reserve(rb.Size() + cbFile + 4/*给调用方预留，例如添加结尾NULL等*/);
    const auto cbOld = rb.Size();
    DWORD cbRead;
    File.Read(rb.PushBack(cbFile), (DWORD)cbFile, &cbRead, &nts);
    if (!NT_SUCCESS(nts) || cbRead != cbFile)
    {
        rb.ReSize(cbOld);
        return nts;
    }
    return STATUS_SUCCESS;
}
inline CRefBin ReadInFile(
    _In_z_ PCWSTR pszFile,
    _Out_opt_ NTSTATUS* pnts = nullptr) noexcept
{
    CRefBin rb{};
    const auto nts = ReadInFile(pszFile, rb);
    if (pnts) *pnts = nts;
    return rb;
}

inline NTSTATUS WriteToFile(
    _In_z_ PCWSTR pszFile,
    _In_reads_bytes_(cb) PCVOID pData,
    DWORD cb) noexcept
{
    CFile File{};
    auto nts = File.Create(pszFile,
        FILE_OVERWRITE_IF,
        FILE_WRITE_DATA | SYNCHRONIZE,
        FILE_SHARE_READ,
        FILE_NON_DIRECTORY_FILE | FILE_SEQUENTIAL_ONLY | FILE_SYNCHRONOUS_IO_NONALERT);
    if (File.IsValid())
        File.Write(pData, cb, nullptr, &nts);
    return nts;
}
EckInline NTSTATUS WriteToFile(_In_z_ PCWSTR pszFile, const CRefBin& rb) noexcept
{
    return WriteToFile(pszFile, rb.Data(), (DWORD)rb.Size());
}

namespace Priv
{
    // 不得关闭返回的句柄
    NTSTATUS FilepOpenMountPointManager(_Out_ HANDLE& hMpmDevice) noexcept
    {
        constexpr UNICODE_STRING MpmName RTL_CONSTANT_STRING(MOUNTMGR_DEVICE_NAME);
        static std::atomic<HANDLE> s_hMpmDevice{};
        hMpmDevice = s_hMpmDevice.load(std::memory_order_acquire);
        if (hMpmDevice)
            return STATUS_SUCCESS;
        else
        {
            NTSTATUS nts;
            const auto hNew = NaOpenFile(&MpmName,
                FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
                &nts);
            if (NT_SUCCESS(nts))
            {
                s_hMpmDevice.compare_exchange_strong(
                    hMpmDevice, hNew, std::memory_order_acq_rel);
                if (hMpmDevice)
                    NtClose(hNew);
                else
                    hMpmDevice = hNew;
            }
            return nts;
        }
    }
}

inline NTSTATUS FileNtPathToDosPath(CRefStrW& rsBuf) noexcept
{
    if (rsBuf.IsStartWith(EckStrAndLen(LR"(\??\)")))
        rsBuf.Erase(0, 4);
    else if (rsBuf.IsStartWithI(EckStrAndLen(LR"(\SystemRoot)")))
    {
        const auto pszRoot = NaGetNtSystemRoot();
        rsBuf.Replace(0, 11, pszRoot, (int)TcsLen(pszRoot));
    }
    else if (rsBuf.IsStartWithI(EckStrAndLen(LR"(System32\)"))
#ifdef _WIN64
        || rsBuf.IsStartWithI(EckStrAndLen(LR"(SysWOW64\)"))
#ifdef _M_ARM64
        || rsBuf.IsStartWithI(EckStrAndLen(LR"(SysArm32\)"))
        || rsBuf.IsStartWithI(EckStrAndLen(LR"(SyChpe32\)"))
#endif
#endif
        )
    {
        const auto pszRoot = NaGetNtSystemRoot();
        const auto cchRoot = (int)TcsLen(pszRoot);
        const auto pszDst = rsBuf.Insert(0, cchRoot + 1);
        TcsCopyLen(pszDst, pszRoot, cchRoot);
        *(pszDst + cchRoot) = L'\\';
    }
    else if (rsBuf.IsStartWithI(EckStrAndLen(LR"(\Windows)")))
    {
        const auto pszRoot = NaGetNtSystemRoot();
        const auto pszDst = rsBuf.Insert(0, 2);
        TcsCopyLen(pszDst, pszRoot, 2);
    }
    else if (rsBuf.Front() == L'\\')
    {
        HANDLE hMpm;
        auto nts = Priv::FilepOpenMountPointManager(hMpm);
        if (!NT_SUCCESS(nts))
            return nts;
        auto posDevEnd = rsBuf.FindChar(L'\\', 1);
        if (posDevEnd < 0)
            return STATUS_SUCCESS;
        posDevEnd = rsBuf.FindChar(L'\\', posDevEnd + 1);
        if (posDevEnd < 0)
            return STATUS_SUCCESS;
        size_t cbBufTotal{ posDevEnd + sizeof(USHORT) + MAX_PATH };
        while (1)
        {
            UniquePtr<DelMA<BYTE>> pBuf{ (BYTE*)malloc(cbBufTotal) };
            const auto pInBuf = (MOUNTMGR_TARGET_NAME*)pBuf.get();
            const auto cbInBuf = ULONG(posDevEnd * sizeof(WCHAR) + sizeof(USHORT) + sizeof(WCHAR));
            pInBuf->DeviceNameLength = (USHORT)posDevEnd * sizeof(WCHAR);
            TcsCopyLenEnd(pInBuf->DeviceName, rsBuf.Data(), posDevEnd);
            const auto pOutBuf = (MOUNTMGR_VOLUME_PATHS*)(pBuf.get() + cbInBuf);
            const auto cbOutBuf = ULONG(cbBufTotal - cbInBuf);

            IO_STATUS_BLOCK iosb;
            nts = NtDeviceIoControlFile(hMpm, nullptr, nullptr, nullptr, &iosb,
                IOCTL_MOUNTMGR_QUERY_DOS_VOLUME_PATH,
                pInBuf, cbInBuf, pOutBuf, cbOutBuf);
            if (NT_SUCCESS(nts))
            {
                rsBuf.Replace(0, posDevEnd, pOutBuf->MultiSz, -1);
                break;
            }
            else if (
                nts == STATUS_BUFFER_TOO_SMALL ||
                nts == STATUS_BUFFER_OVERFLOW ||
                nts == STATUS_INFO_LENGTH_MISMATCH)
                cbBufTotal += MAX_PATH;
            else
                break;
        }
        return nts;
    }
    return STATUS_SUCCESS;
}

inline NTSTATUS FileEnsureDirectoryExist(
    _In_ HANDLE hRoot,
    std::wstring_view svRelative,
    _Out_opt_ HANDLE* phFinalDir = nullptr) noexcept
{
    if (phFinalDir)
        *phFinalDir = nullptr;
    if (svRelative.empty())
        return STATUS_SUCCESS;
    if (svRelative.back() == L'\\')
        svRelative = svRelative.substr(0, svRelative.size() - 1);

    OBJECT_ATTRIBUTES oa
    {
        .Length = sizeof(OBJECT_ATTRIBUTES),
        .RootDirectory = hRoot,
        .Attributes = OBJ_CASE_INSENSITIVE,
    };

    NTSTATUS nts;
    IO_STATUS_BLOCK iosb;
    size_t pos = 0;
    HANDLE hDir;

    do
    {
        const auto posNextSlash = svRelative.find(L'\\', pos);
        const auto bLast = (posNextSlash == std::wstring_view::npos);
        UNICODE_STRING usRel;
        usRel.Buffer = (PWSTR)svRelative.data() + pos;
        usRel.Length = usRel.MaximumLength = USHORT(sizeof(WCHAR) *
            (bLast ? svRelative.size() - pos : posNextSlash - pos));
        oa.ObjectName = &usRel;
        nts = NtCreateFile(&hDir,
            FILE_LIST_DIRECTORY | SYNCHRONIZE,
            &oa, &iosb, nullptr,
            FILE_ATTRIBUTE_NORMAL,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            FILE_OPEN_IF,
            FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
            nullptr, 0);
        if (oa.RootDirectory != hRoot)
            NtClose(oa.RootDirectory);
        if (!NT_SUCCESS(nts))
            return nts;
        if (bLast)
            break;
        oa.RootDirectory = hDir;
        pos = posNextSlash + 1;
    } while (pos != std::wstring_view::npos);

    EckAssert(hDir != hRoot);
    if (phFinalDir)
        *phFinalDir = hDir;
    else
        NtClose(hDir);
    return STATUS_SUCCESS;
}
/// <summary>
/// 保证目录存在
/// </summary>
/// <param name="pszRoot">根目录路径，若不存在则函数失败</param>
/// <param name="svRelative">相对目录，不能以反斜杠开头，但可以以反斜杠结尾</param>
/// <param name="phFinalDir">返回(根目录+相对目录)所指向目录的句柄</param>
/// <returns>NTSTATUS</returns>
inline NTSTATUS FileEnsureDirectoryExist(
    _In_z_ PCWSTR pszRoot,
    std::wstring_view svRelative,
    _Out_opt_ HANDLE* phFinalDir = nullptr) noexcept
{
    NTSTATUS nts;
    const auto hRoot = NaOpenFile(pszRoot,
        FILE_READ_ATTRIBUTES | SYNCHRONIZE,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
        &nts);
    if (!NT_SUCCESS(nts))
    {
        if (phFinalDir) *phFinalDir = nullptr;
        return nts;
    }
    nts = FileEnsureDirectoryExist(hRoot, svRelative, phFinalDir);
    NtClose(hRoot);
    return nts;
}
ECK_NAMESPACE_END