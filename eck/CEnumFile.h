#pragma once
#include "NativeWrapper.h"
#include "AutoPtrDef.h"
#include "CNtObject.h"

ECK_NAMESPACE_BEGIN
class CEnumFile : public CNtObject
{
public:
    using TDefInfo = FILE_FULL_DIR_INFORMATION;

    template<class TInfoStruct>
    EckInlineNdCe static FILE_INFORMATION_CLASS StructToClassEnum() noexcept
    {
        if constexpr (std::is_same_v<TInfoStruct, FILE_DIRECTORY_INFORMATION>)
            return FileDirectoryInformation;
        else if constexpr (std::is_same_v<TInfoStruct, FILE_FULL_DIR_INFORMATION>)
            return FileFullDirectoryInformation;
        else if constexpr (std::is_same_v<TInfoStruct, FILE_BOTH_DIR_INFORMATION>)
            return FileBothDirectoryInformation;
        else if constexpr (std::is_same_v<TInfoStruct, FILE_QUOTA_INFORMATION>)
            return FileQuotaInformation;
        else if constexpr (std::is_same_v<TInfoStruct, FILE_ID_BOTH_DIR_INFORMATION>)
            return FileIdBothDirectoryInformation;
        else if constexpr (std::is_same_v<TInfoStruct, FILE_ID_FULL_DIR_INFORMATION>)
            return FileIdFullDirectoryInformation;
        else if constexpr (std::is_same_v<TInfoStruct, FILE_ID_GLOBAL_TX_DIR_INFORMATION>)
            return FileIdGlobalTxDirectoryInformation;
        else if constexpr (std::is_same_v<TInfoStruct, FILE_ID_EXTD_DIR_INFORMATION>)
            return FileIdExtdDirectoryInformation;
        else if constexpr (std::is_same_v<TInfoStruct, FILE_ID_EXTD_BOTH_DIR_INFORMATION>)
            return FileIdExtdBothDirectoryInformation;
        else
            return 0;
    }
public:
    NTSTATUS Open(_In_z_ PCWSTR pszPath, HANDLE hRoot = nullptr) noexcept
    {
        Clear();
        NTSTATUS nts;
        m_hObject = NaOpenFile(
            pszPath,
            FILE_LIST_DIRECTORY | SYNCHRONIZE,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT,
            &nts, nullptr, FALSE, hRoot);
        return nts;
    }

    // 函数为每个枚举到的文件调用回调函数，直至枚举完毕或回调函数取消枚举。
    // 若回调取消枚举，此函数返回STATUS_MORE_ENTRIES
    template<class TInfoStruct = TDefInfo, std::invocable<TInfoStruct&> F>
    NTSTATUS Enumerate(
        F&& Fn,
        std::wstring_view svPattern,
        size_t cbBuf = 4096u,
        _Out_writes_bytes_opt_(cbBuf) void* pExternalBuf = nullptr) noexcept
    {
        constexpr auto eCls = StructToClassEnum<TInfoStruct>();
        if constexpr (!eCls)
            return STATUS_NOT_SUPPORTED;
        NTSTATUS nts;
        IO_STATUS_BLOCK iosb;

        const auto pBuf = (BYTE*)(pExternalBuf ? pExternalBuf : VAlloc(cbBuf));
        UniquePtr<DelVA<BYTE>> _{ pExternalBuf ? nullptr : pBuf };

        auto usPattern = StringViewToNtString(svPattern);
        nts = NtQueryDirectoryFile(m_hObject, nullptr, nullptr, nullptr, &iosb,
            pBuf, (ULONG)cbBuf, eCls, FALSE,
            svPattern.empty() ? nullptr : &usPattern,
            TRUE);
        if (nts == STATUS_NO_MORE_FILES)
            return STATUS_SUCCESS;
        if (!NT_SUCCESS(nts))
            return nts;
        EckLoop()
        {
            auto pInfo = (TInfoStruct*)pBuf;
            EckLoop()
            {
                EckCanCallbackContinue(Fn(*pInfo))
                    return STATUS_MORE_ENTRIES;
                if (pInfo->NextEntryOffset == 0)
                    break;
                pInfo = (TInfoStruct*)((BYTE*)pInfo + pInfo->NextEntryOffset);
            }
            nts = NtQueryDirectoryFile(m_hObject, nullptr, nullptr, nullptr, &iosb,
                pBuf, (ULONG)cbBuf, eCls, FALSE, nullptr, FALSE);
            if (nts == STATUS_NO_MORE_FILES)
                return STATUS_SUCCESS;
            if (!NT_SUCCESS(nts))
                return nts;
        }
        return STATUS_SUCCESS;
    }
};

class CEnumFileSingle : public CEnumFile
{
private:
    void* m_pBuffer{};
    ULONG m_cbBuffer{ 4096 };
    BOOL m_bExternalBuf{};
    // 若第一次枚举还未进行，则为nullptr
    // 若当前缓冲区遍历完成，则为INVALID_HANDLE_VALUE
    void* m_pCurrItem{};

    void AllocateBuffer() noexcept
    {
        m_bExternalBuf = FALSE;
        m_pBuffer = VAlloc(m_cbBuffer);
    }

    void FreeBuffer() noexcept
    {
        if (!m_bExternalBuf && m_pBuffer)
            VFree(m_pBuffer);
    }
public:
    ~CEnumFileSingle() { FreeBuffer(); }

    // 调用本函数后获得一个枚举条目信息。
    // 调用方使用NT_SUCCESS宏判断枚举是否已终止
    template<class TInfoStruct = TDefInfo>
    NTSTATUS Next(
        _Out_ TInfoStruct*& pInfo,
        std::wstring_view svPattern = {}) noexcept
    {
        constexpr auto eCls = StructToClassEnum<TInfoStruct>();
        if constexpr (!eCls)
            return STATUS_NOT_SUPPORTED;
        NTSTATUS nts;
        IO_STATUS_BLOCK iosb;
        if (!m_pBuffer)
            AllocateBuffer();
        if (!m_pBuffer)
            return STATUS_INSUFFICIENT_RESOURCES;

        if (!m_pCurrItem)
        {
            auto usPattern = StringViewToNtString(svPattern);
            nts = NtQueryDirectoryFile(m_hObject, nullptr, nullptr, nullptr, &iosb,
                m_pBuffer, m_cbBuffer, eCls, FALSE,
                svPattern.empty() ? nullptr : &usPattern,
                TRUE);
        }
        else if (m_pCurrItem == INVALID_HANDLE_VALUE)
        {
            nts = NtQueryDirectoryFile(m_hObject, nullptr, nullptr, nullptr, &iosb,
                m_pBuffer, m_cbBuffer, eCls, FALSE, nullptr, FALSE);
        }
        else
        {
            pInfo = (TInfoStruct*)m_pCurrItem;
            if (pInfo->NextEntryOffset == 0)
                m_pCurrItem = INVALID_HANDLE_VALUE;
            else
                m_pCurrItem = (BYTE*)m_pCurrItem + pInfo->NextEntryOffset;
            return STATUS_SUCCESS;
        }

        if (!NT_SUCCESS(nts))
            return nts;
        m_pCurrItem = m_pBuffer;
        return Next(pInfo);
    }

    // 在下一次调用Next时重启枚举
    // 若为不同的目录重用本类，则打开新目录时必须调用此方法
    void ReStart() noexcept { m_pCurrItem = nullptr; }

    void SetBuffer(size_t cbBuffer, void* pBuffer = nullptr) noexcept
    {
        m_pCurrItem = nullptr;
        if (pBuffer)
        {
            FreeBuffer();
            m_pBuffer = pBuffer;
            m_cbBuffer = (ULONG)cbBuffer;
            m_bExternalBuf = TRUE;
        }
        else
        {
            if (cbBuffer < 4096)
                cbBuffer = 4096;
            if (m_bExternalBuf)
            {
                m_cbBuffer = (ULONG)cbBuffer;
                m_pBuffer = nullptr;
                m_bExternalBuf = FALSE;
            }
            else if ((ULONG)cbBuffer != m_cbBuffer)
            {
                FreeBuffer();
                m_cbBuffer = (ULONG)cbBuffer;
                m_pBuffer = nullptr;
            }
        }
    }
};
ECK_NAMESPACE_END