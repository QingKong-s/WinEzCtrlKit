#pragma once
#include "NativeWrapper.h"
#include "AutoPtrDef.h"

ECK_NAMESPACE_BEGIN
class CEnumFile2
{
public:
	using TDefInfo = FILE_FULL_DIR_INFORMATION;
private:
	HANDLE m_hDir{};
	UniquePtr<DelVA<BYTE>> m_pBuf{};
	size_t m_cbBuf{};
	// 若第一次枚举还未进行，则为nullptr
	// 若当前缓冲区遍历完成，则为INVALID_HANDLE_VALUE
	void* m_pCurrItem{};
public:
	ECK_DISABLE_COPY_MOVE(CEnumFile2);

	CEnumFile2(size_t cbBuf = 4096)
		: m_pBuf{ (BYTE*)VAlloc(cbBuf) }, m_cbBuf{ cbBuf }
	{
	}
	CEnumFile2(_In_z_ PCWSTR pszPath, size_t cbBuf = 4096) : CEnumFile2{ cbBuf }
	{
		Open(pszPath);
	}

	~CEnumFile2() { Close(); }

	NTSTATUS Open(_In_z_ PCWSTR pszPath, _In_opt_ HANDLE hRoot = nullptr)
	{
		Close();
		NTSTATUS nts;
		m_hDir = NaOpenFile(
			pszPath,
			FILE_LIST_DIRECTORY | SYNCHRONIZE,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT,
			&nts,
			nullptr,
			FALSE,
			hRoot);
		return nts;
	}

	template<class TInfoStruct = TDefInfo>
	NTSTATUS Next(_In_reads_or_z_opt_(cchPatten) PCWCH pszPatten, int cchPatten,
		_Out_ TInfoStruct*& pInfo)
	{
		FILE_INFORMATION_CLASS eCls;
		if constexpr (std::is_same_v<TInfoStruct, FILE_DIRECTORY_INFORMATION>)
			eCls = FileDirectoryInformation;
		else if constexpr (std::is_same_v<TInfoStruct, FILE_FULL_DIR_INFORMATION>)
			eCls = FileFullDirectoryInformation;
		else if constexpr (std::is_same_v<TInfoStruct, FILE_BOTH_DIR_INFORMATION>)
			eCls = FileBothDirectoryInformation;
		else if constexpr (std::is_same_v<TInfoStruct, FILE_QUOTA_INFORMATION>)
			eCls = FileQuotaInformation;
		else if constexpr (std::is_same_v<TInfoStruct, FILE_ID_BOTH_DIR_INFORMATION>)
			eCls = FileIdBothDirectoryInformation;
		else if constexpr (std::is_same_v<TInfoStruct, FILE_ID_FULL_DIR_INFORMATION>)
			eCls = FileIdFullDirectoryInformation;
		else if constexpr (std::is_same_v<TInfoStruct, FILE_ID_GLOBAL_TX_DIR_INFORMATION>)
			eCls = FileIdGlobalTxDirectoryInformation;
		else if constexpr (std::is_same_v<TInfoStruct, FILE_ID_EXTD_DIR_INFORMATION>)
			eCls = FileIdExtdDirectoryInformation;
		else if constexpr (std::is_same_v<TInfoStruct, FILE_ID_EXTD_BOTH_DIR_INFORMATION>)
			eCls = FileIdExtdBothDirectoryInformation;
		else
			return STATUS_NOT_SUPPORTED;

		if (m_pCurrItem && m_pCurrItem != INVALID_HANDLE_VALUE)
		{
			pInfo = (TInfoStruct*)m_pCurrItem;
			if (pInfo->NextEntryOffset == 0)
				m_pCurrItem = INVALID_HANDLE_VALUE;
			else
				m_pCurrItem = (BYTE*)m_pCurrItem + pInfo->NextEntryOffset;
			return STATUS_SUCCESS;
		}

		NTSTATUS nts;
		IO_STATUS_BLOCK iosb;

		if (!m_pCurrItem)
		{
			UNICODE_STRING usPatten;
			if (pszPatten)
			{
				usPatten.Buffer = (PWSTR)pszPatten;
				if (cchPatten < 0)
					cchPatten = (int)wcslen(pszPatten);
				usPatten.Length = usPatten.MaximumLength = (USHORT)cchPatten * 2;
			}
			UNICODE_STRING* pusPatten{ pszPatten ? &usPatten : nullptr };
#if PHNT_VERSION >= PHNT_REDSTONE3
			nts NtQueryDirectoryFileEx(m_hDir, nullptr, nullptr, nullptr, &iosb,
				m_pBuf.get(), (ULONG)m_cbBuf, eCls, SL_RESTART_SCAN, pusPatten);
#else
			nts = NtQueryDirectoryFile(m_hDir, nullptr, nullptr, nullptr, &iosb,
				m_pBuf.get(), (ULONG)m_cbBuf, eCls, FALSE, pusPatten, TRUE);
#endif
			if (!NT_SUCCESS(nts))
				return nts;
			m_pCurrItem = m_pBuf.get();
			return Next(nullptr, 0, pInfo);
		}
		else
		{
#if PHNT_VERSION >= PHNT_REDSTONE3
			nts = NtQueryDirectoryFileEx(m_hDir, nullptr, nullptr, nullptr, &iosb,
				m_pBuf.get(), (ULONG)m_cbBuf, eCls, 0u, nullptr);
#else
			nts = NtQueryDirectoryFile(m_hDir, nullptr, nullptr, nullptr, &iosb,
				m_pBuf.get(), (ULONG)m_cbBuf, eCls, FALSE, nullptr, FALSE);
#endif
			if (!NT_SUCCESS(nts))
				return nts;
			m_pCurrItem = m_pBuf.get();
			return Next(nullptr, 0, pInfo);
		}
		return STATUS_SUCCESS;
	}

	EckInline void Close()
	{
		if (m_hDir)
		{
			NtClose(m_hDir);
			m_hDir = nullptr;
			m_pCurrItem = nullptr;
		}
	}

	EckInlineNdCe HANDLE GetHDir() const { return m_hDir; }
};
ECK_NAMESPACE_END