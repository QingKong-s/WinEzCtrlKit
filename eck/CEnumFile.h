#pragma once
#include "NativeWrapper.h"
#include "AutoPtrDef.h"

ECK_NAMESPACE_BEGIN
class CEnumFile
{
public:
	using TDefInfo = FILE_FULL_DIR_INFORMATION;
private:
	HANDLE m_hDir{};
public:
	ECK_DISABLE_COPY_MOVE_DEF_CONS(CEnumFile);
	~CEnumFile() { Close(); }
	CEnumFile(_In_z_ PCWSTR pszPath) { Open(pszPath); }

	NTSTATUS Open(_In_z_ PCWSTR pszPath)
	{
		Close();
		NTSTATUS nts;
		m_hDir = NaOpenFile(
			pszPath,
			FILE_LIST_DIRECTORY | SYNCHRONIZE,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT,
			&nts);
		return nts;
	}

	template<
		class TInfoStruct = TDefInfo,
		std::invocable<TInfoStruct&> F
	>
	NTSTATUS Enumerate(_In_reads_or_z_(cchPatten) PCWCH pszPatten,
		int cchPatten, F&& Fn, size_t cbBuf = 4096u)
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
		NTSTATUS nts;
		IO_STATUS_BLOCK iosb;

		const auto pBuf = (BYTE*)VAlloc(cbBuf);
		UniquePtr<DelVA<BYTE>> _{ pBuf };

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
		nts = NtQueryDirectoryFileEx(m_hDir, nullptr, nullptr, nullptr, &iosb,
			pBuf, (ULONG)cbBuf, eCls, SL_RESTART_SCAN, pusPatten);
#else
		nts = NtQueryDirectoryFile(m_hDir, nullptr, nullptr, nullptr, &iosb,
			pBuf, (ULONG)cbBuf, eCls, FALSE, pusPatten, TRUE);
#endif
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
					return STATUS_SUCCESS;

				if (pInfo->NextEntryOffset == 0)
					break;
				pInfo = (TInfoStruct*)((BYTE*)pInfo + pInfo->NextEntryOffset);
			}
#if PHNT_VERSION >= PHNT_REDSTONE3
			nts = NtQueryDirectoryFileEx(m_hDir, nullptr, nullptr, nullptr, &iosb,
				pBuf, (ULONG)cbBuf, eCls, 0u, pusPatten);
#else
			nts = NtQueryDirectoryFile(m_hDir, nullptr, nullptr, nullptr, &iosb,
				pBuf, (ULONG)cbBuf, eCls, FALSE, pusPatten, FALSE);
#endif
			if (nts == STATUS_NO_MORE_FILES)
				return STATUS_SUCCESS;
			if (!NT_SUCCESS(nts))
				return nts;
		}
		return STATUS_SUCCESS;
	}

	EckInline void Close()
	{
		if (m_hDir)
		{
			NtClose(m_hDir);
			m_hDir = nullptr;
		}
	}

	EckInlineNdCe HANDLE GetHDir() const { return m_hDir; }
};
ECK_NAMESPACE_END