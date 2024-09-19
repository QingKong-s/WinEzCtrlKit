/*
* WinEzCtrlKit Library
*
* ProcThrdHelper.h �� �����߳�
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "SystemHelper.h"

ECK_NAMESPACE_BEGIN
inline NTSTATUS GetProcessPath(UINT uPid, CRefStrW& rsPath, BOOL bDosPath = TRUE)
{
	SYSTEM_PROCESS_ID_INFORMATION spii{ .ProcessId = i32ToP<HANDLE>(uPid) };
	NTSTATUS nts = NtQuerySystemInformation(SystemProcessIdInformation, &spii, sizeof(spii), nullptr);
	if (spii.ImageName.MaximumLength && nts == STATUS_INFO_LENGTH_MISMATCH)
	{
		rsPath.ReSize(spii.ImageName.MaximumLength);
		spii.ImageName.Buffer = rsPath.Data();
		spii.ImageName.Length = 0;
		nts = NtQuerySystemInformation(SystemProcessIdInformation, &spii, sizeof(spii), nullptr);
		if (NT_SUCCESS(nts))
		{
			if (bDosPath)
				return NtPathToDosPath(rsPath);
			else
				return STATUS_SUCCESS;
		}
	}
	rsPath.Clear();
	return nts;
}

inline BOOL GetProcessPath(HANDLE hProcess, CRefStrW& rsPath, BOOL bDosPath = TRUE)
{
	DWORD cch{ MAX_PATH };
	rsPath.ReSize(cch);
	if (QueryFullProcessImageNameW(hProcess, bDosPath ? 0 : PROCESS_NAME_NATIVE,
		rsPath.Data(), &cch))
	{
		rsPath.ReSize(cch);
		return TRUE;
	}
	else
	{
		rsPath.Clear();
		return FALSE;
	}
}

inline NTSTATUS GetPidByProcessName(PCWSTR pszImageName, UINT& uPid)
{
	uPid = 0u;
	ULONG cb;
	NTSTATUS nts = NtQuerySystemInformation(SystemProcessInformation, nullptr, 0, &cb);
	if (!cb)
		return nts;
	BYTE* pBuf = (BYTE*)VAlloc(cb);
	UniquePtrVA<BYTE> _(pBuf);
	if (!NT_SUCCESS(nts = NtQuerySystemInformation(SystemProcessInformation, pBuf, cb, &cb)))
		return nts;
	SYSTEM_PROCESS_INFORMATION* pspi = (SYSTEM_PROCESS_INFORMATION*)pBuf;
	EckLoop()
	{
		if (wcsnicmp(pszImageName, pspi->ImageName.Buffer, pspi->ImageName.Length) == 0)
		{
			uPid = pToI32<UINT>(pspi->UniqueProcessId);
			return STATUS_SUCCESS;
		}
		if (pspi->NextEntryOffset == 0)
			break;
		pspi = PtrStepCb(pspi, pspi->NextEntryOffset);
	}
	return STATUS_NOT_FOUND;
}

inline NTSTATUS GetPidByProcessName(PCWSTR pszImageName, std::vector<UINT>& vPid)
{
	ULONG cb;
	NTSTATUS nts = NtQuerySystemInformation(SystemProcessInformation, nullptr, 0, &cb);
	if (!cb)
		return nts;
	BYTE* pBuf = (BYTE*)VAlloc(cb);
	UniquePtrVA<BYTE> _(pBuf);
	if (!NT_SUCCESS(nts = NtQuerySystemInformation(SystemProcessInformation, pBuf, cb, &cb)))
		return nts;
	SYSTEM_PROCESS_INFORMATION* pspi = (SYSTEM_PROCESS_INFORMATION*)pBuf;
	EckLoop()
	{
		if (wcsnicmp(pszImageName, pspi->ImageName.Buffer, pspi->ImageName.Length) == 0)
		{
			vPid.push_back(pToI32<UINT>(pspi->UniqueProcessId));
			return STATUS_SUCCESS;
		}
		if (pspi->NextEntryOffset == 0)
			break;
		pspi = PtrStepCb(pspi, pspi->NextEntryOffset);
	}
	return STATUS_NOT_FOUND;
}

EckInline void* GetProcessPeb(HANDLE hProcess, NTSTATUS* pnts = nullptr)
{
	PROCESS_BASIC_INFORMATION pbi;
	NTSTATUS nts;
	if (!NT_SUCCESS(nts = NtQueryInformationProcess(hProcess, ProcessBasicInformation, &pbi, sizeof(pbi), nullptr)))
	{
		if (pnts)
			*pnts = nts;
		return nullptr;
	}
	if (pnts)
		*pnts = STATUS_SUCCESS;
	return pbi.PebBaseAddress;
}

struct MODULE_INFO
{
	CRefStrW rsModuleName;
	CRefStrW rsModulePath;
	void* BaseAddress;
	SIZE_T cbImage;
};

/// <summary>
/// ö�ٽ���ģ��
/// </summary>
/// <param name="hProcess">���̾�����������PROCESS_QUERY_INFORMATION | PROCESS_VM_READȨ��</param>
/// <param name="vResult">ö�ٽ����������ո�����</param>
/// <returns>NTSTATUS</returns>
inline NTSTATUS EnumProcessModules(HANDLE hProcess, std::vector<MODULE_INFO>& vResult)
{
	NTSTATUS nts;
	// ȡPEB
	UINT_PTR pPeb = (UINT_PTR)GetProcessPeb(hProcess, &nts);
	if (!pPeb)
		return nts;
	// ȡPEB_LDR_DATA
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

	LDR_DATA_TABLE_ENTRY Entry;// ��Ҫʹ��Win7֮����ӵ��ֶ�
	UINT_PTR pBegin = pLdr + offsetof(PEB_LDR_DATA, InLoadOrderModuleList);
	for (UINT_PTR p = (UINT_PTR)LdrData.InLoadOrderModuleList.Flink; p != pBegin; )
	{
		if (!NT_SUCCESS(nts = NtReadVirtualMemory(hProcess, (void*)p,
			&Entry, LDR_DATA_TABLE_ENTRY_SIZE_WIN7, nullptr)))
			return nts;
		if (Entry.DllBase)
		{
			auto& e = vResult.emplace_back();
			if (Entry.BaseDllName.Length)
			{
				const int cch = Entry.BaseDllName.Length / sizeof(WCHAR);
				e.rsModuleName.ReSize(cch);
				if (!NT_SUCCESS(nts = NtReadVirtualMemory(hProcess, (void*)Entry.BaseDllName.Buffer,
					e.rsModuleName.Data(), cch * sizeof(WCHAR), nullptr)))
					return nts;
			}
			if (Entry.FullDllName.Length)
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

struct THREAD_INFO
{
	UINT uTID;
	UINT_PTR AddrStart;
	KPRIORITY Priority;
	KPRIORITY BasePriority;
};

struct PROCESS_INFO
{
	CRefStrW rsImageName;	// ������
	ULONG uPid;				// ����ID
	ULONG uParentPid;		// ������ID
	ULONG cThreads;			// �߳���
	ULONG uSessionID;		// �ỰID
	ULONG cHandles;			// �����
	ULONG cPageFaults;		// ҳ�������
	SIZE_T cbPrivateWorkingSet;			// ר�ù�����
	SIZE_T cbWorkingSet;				// ������
	SIZE_T cbPeakWorkingSet;			// ��ֵ������
	SIZE_T cbQuotaPagedPoolUsage;		// ҳ�滺���
	SIZE_T cbPeakQuotaPagedPoolUsage;	// ��ֵҳ�滺���
	SIZE_T cbQuotaNonPagedPoolUsage;	// ��ҳ�滺���
	SIZE_T cbPeakQuotaNonPagedPoolUsage;// ��ֵ��ҳ�滺���
	SIZE_T cbPageFileUsage;				// ���ύ
	SIZE_T cbPeakPageFileUsage;			// ��ֵ���ύ
	CRefStrW rsFilePath;	// ����·��
	std::vector<THREAD_INFO> vThreads;	// �߳���Ϣ
	std::vector<MODULE_INFO> vModules;	// ģ����Ϣ
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
/// ö�ٽ���
/// </summary>
/// <param name="vResult">ö�ٽ����������ո�����</param>
/// <param name="uFlags">EPF_����</param>
/// <returns>NTSTATUS</returns>
inline NTSTATUS EnumProcess(std::vector<PROCESS_INFO>& vResult, EPFLAGS uFlags = EPF_NONE)
{
	ULONG cb;
	NTSTATUS nts = NtQuerySystemInformation(SystemProcessInformation, nullptr, 0, &cb);
	if (!cb)
		return nts;
	BYTE* pBuf = (BYTE*)VAlloc(cb);
	UniquePtrVA<BYTE> _(pBuf);
	if (!NT_SUCCESS(nts = NtQuerySystemInformation(SystemProcessInformation, pBuf, cb, &cb)))
		return nts;
	vResult.reserve(150u);
	auto pspi = (SYSTEM_PROCESS_INFORMATION*)pBuf;
	EckLoop()
	{
		auto& e = vResult.emplace_back(
			CRefStrW(pspi->ImageName),
			pToI32<ULONG>(pspi->UniqueProcessId),
			pToI32<ULONG>(pspi->InheritedFromUniqueProcessId),
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
				t.uTID = pToI32<ULONG>(p->ClientId.UniqueThread);
				t.AddrStart = p->StartAddress;
				t.Priority = p->Priority;
				t.BasePriority = p->BasePriority;
			}
		}

		if (uFlags & EPF_MODULE_INFO)
		{
			const auto hProcess = NaOpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, e.uPid);
			EnumProcessModules(hProcess, e.vModules);
			NtClose(hProcess);
		}

		if (pspi->NextEntryOffset == 0)
			break;
		pspi = PtrStepCb(pspi, pspi->NextEntryOffset);
	}
	return STATUS_SUCCESS;
}

/// <summary>
/// ��������������Ȩ
/// </summary>
/// <param name="hProcess">���̾�����������PROCESS_QUERY_LIMITED_INFORMATIONȨ��</param>
/// <param name="pszPrivilege">��Ȩ��</param>
/// <param name="bEnable">�Ƿ�����</param>
/// <returns>NTSTATUS</returns>
inline NTSTATUS AdjustProcessPrivilege(HANDLE hProcess, PCWSTR pszPrivilege, BOOL bEnable)
{
	HANDLE hToken;
	NTSTATUS nts = NtOpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES, &hToken);
	if (!NT_SUCCESS(nts))
		return nts;
	UNICODE_STRING usPrivilege;
	RtlInitUnicodeString(&usPrivilege, pszPrivilege);

	TOKEN_PRIVILEGES tp;
	if (!NT_SUCCESS(nts = LsaLookupPrivilegeValue(hToken, &usPrivilege, &tp.Privileges[0].Luid)))
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

[[nodiscard]] inline HICON GetWindowSmallIcon(HWND hWnd, int msTimeOut = 300)
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

[[nodiscard]] EckInline HICON GetWindowLargeIcon(HWND hWnd, int msTimeOut = 300)
{
	HICON hIcon;
	if (!SendMessageTimeoutW(hWnd, WM_GETICON, ICON_BIG, 0,
		SMTO_ABORTIFHUNG | SMTO_BLOCK | SMTO_ERRORONEXIT, msTimeOut, (DWORD_PTR*)&hIcon))
		return nullptr;
	if (!hIcon)
		hIcon = (HICON)GetClassLongPtrW(hWnd, GCLP_HICON);
	return hIcon;
}

[[nodiscard]] inline HICON GetWindowIcon(HWND hWnd, BOOL& bNeedDestroy, BOOL bSmall = FALSE, int msTimeOut = 300)
{
	bNeedDestroy = FALSE;
	const HICON hIcon =
		(bSmall ? GetWindowSmallIcon(hWnd, msTimeOut) : GetWindowLargeIcon(hWnd, msTimeOut));
	if (hIcon)
		return hIcon;

	DWORD dwPid;
	GetWindowThreadProcessId(hWnd, &dwPid);
	if (!dwPid)
		return nullptr;
	CRefStrW rsPath{};
	if (!NT_SUCCESS(GetProcessPath(dwPid, rsPath)))
		return nullptr;

	SHFILEINFOW sfi;
	const UINT uFlags = (bSmall ? (SHGFI_ICON | SHGFI_SMALLICON) : SHGFI_ICON);
#pragma warning(suppress:6001)
	if (!SHGetFileInfoW(rsPath.Data(), 0, &sfi, sizeof(sfi), uFlags))
		SHGetFileInfoW(rsPath.Data(), FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(sfi),
			uFlags | SHGFI_USEFILEATTRIBUTES);
	bNeedDestroy = (sfi.hIcon != nullptr);
	return sfi.hIcon;
}
ECK_NAMESPACE_END