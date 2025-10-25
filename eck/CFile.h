#pragma once
#include "NativeWrapper.h"

ECK_NAMESPACE_BEGIN
class CFile
{
private:
    HANDLE m_hFile{};
public:
    CFile() = default;
    CFile(const CFile& e) = delete;
    CFile& operator=(const CFile& e) = delete;
    CFile(CFile&& e) noexcept { std::swap(m_hFile, e.m_hFile); }
    CFile& operator=(CFile&& e) noexcept
    {
        std::swap(m_hFile, e.m_hFile);
        return *this;
    }
    ~CFile() { Close(); }

    BOOL CreateWin32(
        PCWSTR pszFile,
        DWORD dwMode = OPEN_EXISTING,
        DWORD dwAccess = FILE_GENERIC_READ,
        DWORD dwShareMode = FILE_SHARE_READ,
        DWORD dwAttr = FILE_ATTRIBUTE_NORMAL) noexcept
    {
        Close();
        return !!(m_hFile = CreateFileW(pszFile, dwAccess,
            dwShareMode, nullptr, dwMode, dwAttr, nullptr));
    }
    NTSTATUS Create(
        PCWSTR pszFile,
        DWORD dwDispostion = FILE_OPEN,
        DWORD dwAccess = FILE_GENERIC_READ,
        DWORD dwShareMode = FILE_SHARE_READ,
        DWORD dwOptions = FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
        DWORD dwAttr = FILE_ATTRIBUTE_NORMAL,
        DWORD cbInit = 0,
        _Out_opt_ IO_STATUS_BLOCK* piosb = nullptr) noexcept
    {
        Close();
        NTSTATUS nts;
        m_hFile = NaCreateFile(pszFile, dwAccess, dwShareMode,
            dwOptions, dwDispostion, &nts, piosb, dwAttr, cbInit);
        return nts;
    }

    EckInlineNdCe BOOL IsValid() const { return m_hFile && m_hFile != INVALID_HANDLE_VALUE; }

    EckInline void Close()
    {
        if (IsValid())
        {
            NtClose(m_hFile);
            m_hFile = nullptr;
        }
    }

    EckInlineNdCe HANDLE GetHandle() const { return m_hFile; }

    [[nodiscard]] LONGLONG GetSize(_Out_opt_ NTSTATUS* pnts = nullptr) noexcept
    {
        IO_STATUS_BLOCK iosb;
        FILE_STANDARD_INFORMATION Info;
        const auto nts = NtQueryInformationFile(m_hFile, &iosb,
            &Info, sizeof(Info), FileStandardInformation);
        if (pnts) *pnts = nts;
        return Info.EndOfFile.QuadPart;
    }
    EckInlineNd DWORD GetSize32(NTSTATUS* pnts = nullptr) noexcept { return (DWORD)GetSize(pnts); }

    EckInline CFile& Read(
        _Out_writes_bytes_(cbBuf) void* pBuf,
        DWORD cbBuf,
        _Out_opt_ DWORD* pcbRead = nullptr,
        _Out_opt_ NTSTATUS* pnts = nullptr) noexcept
    {
        IO_STATUS_BLOCK iosb;
        const auto nts = NtReadFile(m_hFile, nullptr, nullptr, nullptr,
            &iosb, pBuf, cbBuf, nullptr, nullptr);
        if (pcbRead) *pcbRead = (DWORD)iosb.Information;
        if (pnts) *pnts = nts;
        return *this;
    }
    template<class T>
    EckInline CFile& operator>>(_Out_ T& Buf) noexcept
    {
        Read(&Buf, sizeof(T));
        return *this;
    }

    EckInline CFile& Write(
        _In_reads_bytes_(cbBuf) PCVOID pBuf,
        DWORD cbBuf,
        _Out_opt_ DWORD* pcbWritten = nullptr,
        _Out_opt_ NTSTATUS* pnts = nullptr) noexcept
    {
        IO_STATUS_BLOCK iosb;
        const auto nts = NtWriteFile(m_hFile, nullptr, nullptr, nullptr,
            &iosb, (void*)pBuf, cbBuf, nullptr, nullptr);
        if (pcbWritten) *pcbWritten = (DWORD)iosb.Information;
        if (pnts) *pnts = nts;
        return *this;
    }
    template<class T>
    EckInline CFile& operator<<(const T& Buf) noexcept
    {
        Write(&Buf, sizeof(T));
        return *this;
    }

    NTSTATUS End() noexcept
    {
        NTSTATUS nts;
        IO_STATUS_BLOCK iosb;
        LARGE_INTEGER Info;
        nts = NtQueryInformationFile(m_hFile, &iosb, &Info, sizeof(Info), FilePositionInformation);
        if (!NT_SUCCESS(nts))
            return nts;
        nts = NtSetInformationFile(m_hFile, &iosb, &Info, sizeof(Info), FileEndOfFileInformation);
        if (!NT_SUCCESS(nts))
            return nts;
        nts = NtSetInformationFile(m_hFile, &iosb, &Info, sizeof(Info), FileAllocationInformation);
        return nts;
    }

    LONGLONG PosGet(_Out_opt_ NTSTATUS* pnts = nullptr) noexcept
    {
        IO_STATUS_BLOCK iosb;
        FILE_POSITION_INFORMATION Info;
        const auto nts = NtQueryInformationFile(m_hFile, &iosb,
            &Info, sizeof(Info), FilePositionInformation);
        if (pnts) *pnts = nts;
        return Info.CurrentByteOffset.QuadPart;
    }
    CFile& PosSet(LONGLONG pos, _Out_opt_ NTSTATUS* pnts = nullptr) noexcept
    {
        IO_STATUS_BLOCK iosb;
        const auto nts = NtSetInformationFile(m_hFile, &iosb,
            &pos, sizeof(pos), FilePositionInformation);
        if (pnts) *pnts = nts;
        return *this;
    }
    CFile& PosDelta(LONGLONG d, _Out_opt_ NTSTATUS* pnts = nullptr) noexcept
    {
        NTSTATUS nts;
        const auto pos = PosGet(&nts);
        if (NT_SUCCESS(nts))
            PosSet(pos + d, pnts);
        else
            if (pnts) *pnts = nts;
        return *this;
    }
    EckInline CFile& operator+=(LONGLONG d) noexcept { return PosDelta(d); }
    EckInline CFile& operator-=(LONGLONG d) noexcept { return PosDelta(-d); }

    EckInline CFile& PosToBegin() noexcept { return PosSet(0); }
    EckInline CFile& PosToEnd() noexcept { return PosSet(GetSize()); }

    EckInline NTSTATUS Flush() noexcept
    {
        IO_STATUS_BLOCK iosb;
        return NtFlushBuffersFile(m_hFile, &iosb);
    }

    EckInline NTSTATUS GetInformation(_Out_ auto& Info,
        FILE_INFORMATION_CLASS eCls) noexcept
    {
        IO_STATUS_BLOCK iosb;
        return NtQueryInformationFile(m_hFile, &iosb, &Info, sizeof(Info), eCls);
    }
    EckInline NTSTATUS GetInformationBuffer(
        _Out_writes_bytes_(cbBuf) void* pBuf,
        ULONG cbBuf,
        FILE_INFORMATION_CLASS eCls,
        _Out_opt_ ULONG* pcbRet = nullptr) noexcept
    {
        IO_STATUS_BLOCK iosb;
        const auto nts = NtQueryInformationFile(m_hFile, &iosb,
            pBuf, cbBuf, eCls);
        if (pcbRet) *pcbRet = (ULONG)iosb.Information;
        return nts;
    }
};

class CFileSectionMap
{
private:
    HANDLE m_hSection{};
    void* m_pMap{};
public:
    CFileSectionMap() = default;
    CFileSectionMap(const CFileSectionMap&) = delete;
    CFileSectionMap& operator=(const CFileSectionMap&) = delete;
    CFileSectionMap(CFileSectionMap&& e) noexcept
    {
        std::swap(m_hSection, e.m_hSection);
        std::swap(m_pMap, e.m_pMap);
    }
    CFileSectionMap& operator=(CFileSectionMap&& e) noexcept
    {
        std::swap(m_hSection, e.m_hSection);
        std::swap(m_pMap, e.m_pMap);
        return *this;
    }
    ~CFileSectionMap() { Close(); }

    HANDLE Create(HANDLE hFile,
        DWORD dwProtect = PAGE_READONLY,
        DWORD dwSectionAttr = SEC_COMMIT,
        _Out_opt_ NTSTATUS* pnts = nullptr) noexcept
    {
        NTSTATUS nts;
        Close();
        nts = NtCreateSection(&m_hSection, SECTION_ALL_ACCESS, nullptr,
            nullptr, dwProtect, dwSectionAttr, hFile);
        if (pnts) *pnts = nts;
        return m_hSection;
    }

    void Close() noexcept
    {
        if (m_hSection)
        {
            NtClose(m_hSection);
            m_hSection = nullptr;
        }
        UnMap();
    }

    void* Map(DWORD dwProtect = PAGE_READONLY,
        _Out_opt_ NTSTATUS* pnts = nullptr) noexcept
    {
        UnMap();
        SIZE_T cbView{};
        const auto nts = NtMapViewOfSection(m_hSection, NtCurrentProcess(),
            &m_pMap, 0, 0, nullptr, &cbView, ViewShare, 0, dwProtect);
        if (pnts) *pnts = nts;
        return m_pMap;
    }

    NTSTATUS UnMap() noexcept
    {
        if (!m_pMap)
            return STATUS_SUCCESS;
        const auto nts = NtUnmapViewOfSection(NtCurrentProcess(), m_pMap);
        m_pMap = nullptr;
        return nts;
    }
};
ECK_NAMESPACE_END