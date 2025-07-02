#pragma once
#include "SystemHelper.h"

#include <DbgHelp.h>

ECK_NAMESPACE_BEGIN
// 微软符号服务器
constexpr inline std::wstring_view SymSrvMicrosoft{ L"http://msdl.microsoft.com/download/symbols"sv };

struct CV_INFO_PDB70
{
	DWORD CvSignature;
	GUID Signature;
	DWORD Age;
	char PdbFileName[1];
};

struct PDBInfo
{
	CRefStrA rsPdbFile{};
	CV_INFO_PDB70 Cv{};
};

inline HRESULT DshQueryPePdb(PDBInfo& PdbInfo, _In_ PCWSTR pszPeFile) noexcept
{
	NTSTATUS nts;
	UniquePtr<DelHNtObj> hFile{
		NaOpenFile(pszPeFile, GENERIC_READ | SYNCHRONIZE, FILE_SHARE_READ,
			FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE, &nts)
	};
	if (!NT_SUCCESS(nts))
		return HRESULT_FROM_NT(nts);
	HANDLE hSection{};
	UniquePtr<DelHNtObj> _{ hSection };
	OBJECT_ATTRIBUTES oa{ sizeof(oa) };
	nts = NtCreateSection(&hSection, SECTION_MAP_READ, &oa, nullptr,
		PAGE_READONLY, SEC_COMMIT, hFile.get());
	if (!NT_SUCCESS(nts))
		return HRESULT_FROM_NT(nts);
	void* pViewBase{};
	SIZE_T cbView{};
	nts = NtMapViewOfSection(hSection, NtCurrentProcess(), &pViewBase, 0, 0,
		nullptr, &cbView, ViewShare, 0, PAGE_READONLY);
	if (!NT_SUCCESS(nts))
		return HRESULT_FROM_NT(nts);
	HRESULT hr;
	const auto pDosHeader = (IMAGE_DOS_HEADER*)pViewBase;
	const auto pNtHeaders = (IMAGE_NT_HEADERS*)((BYTE*)pViewBase + pDosHeader->e_lfanew);
	const IMAGE_DATA_DIRECTORY* pDbgDataDir;
	if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE ||
		pNtHeaders->Signature != IMAGE_NT_SIGNATURE)
	{
		hr = HRESULT_FROM_WIN32(ERROR_BAD_EXE_FORMAT);
		goto Exit;
	}

	switch (pNtHeaders->FileHeader.Machine)
	{
	case IMAGE_FILE_MACHINE_I386:
	{
		const auto pOptHdr = (IMAGE_OPTIONAL_HEADER32*)(&pNtHeaders->OptionalHeader);
		pDbgDataDir = &pOptHdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG];
	}
	break;
	case IMAGE_FILE_MACHINE_AMD64:
	{
		const auto pOptHdr = (IMAGE_OPTIONAL_HEADER64*)(&pNtHeaders->OptionalHeader);
		pDbgDataDir = &pOptHdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG];
	}
	break;
	default:
		hr = HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
		goto Exit;
	}
	if (pDbgDataDir->VirtualAddress == 0 || pDbgDataDir->Size == 0)
	{
		hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
		goto Exit;
	}

	for (DWORD i = 0; i < pDbgDataDir->Size / sizeof(IMAGE_DEBUG_DIRECTORY); ++i)
	{
		const auto pDbgData = (IMAGE_DEBUG_DIRECTORY*)
			((BYTE*)pViewBase + pDbgDataDir->VirtualAddress);
		if (pDbgData[i].Type == IMAGE_DEBUG_TYPE_CODEVIEW)
		{
			const auto pCvInfo = (CV_INFO_PDB70*)(
				(BYTE*)pViewBase + pDbgData[i].PointerToRawData);
			if (pCvInfo->CvSignature == 'SDSR')
			{
				PdbInfo.rsPdbFile.DupString(pCvInfo->PdbFileName);
				PdbInfo.Cv = *pCvInfo;
				hr = S_OK;
				goto Exit;
			}
		}
	}
	hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
Exit:;
	NtUnmapViewOfSection(NtCurrentProcess(), pViewBase);
	return hr;
}

inline HRESULT DshMakeSymbolUrl(CRefStrW& rsSymbolUrl, const PDBInfo& PdbInfo,
	std::wstring_view svSymbolSrv = SymSrvMicrosoft) noexcept
{
	const auto rsPdbW = StrX2W(PdbInfo.rsPdbFile, CP_ACP);
	rsSymbolUrl.Clear();
	if (PdbInfo.rsPdbFile.IsEmpty())
		return E_INVALIDARG;
	rsSymbolUrl.PushBack(svSymbolSrv);
	if (rsSymbolUrl.Back() != L'/')
		rsSymbolUrl.PushBackChar(L'/');
	rsSymbolUrl.PushBack(rsPdbW);
	rsSymbolUrl.PushBackChar(L'/');
	GuidToStringUpper(PdbInfo.Cv.Signature, rsSymbolUrl.PushBack(32));
	rsSymbolUrl.AppendFormat(L"%u/", PdbInfo.Cv.Age);
	rsSymbolUrl.PushBack(rsPdbW);
	return S_OK;
}

inline HRESULT DshInit(_Out_ HANDLE& hProcess,
	DWORD dwOptions = SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_ANYTHING,
	PCWSTR pszUserSearchPath = nullptr, BOOL bInvadeProcess = FALSE) noexcept
{
	NTSTATUS nts;
	hProcess = NaOpenProcess(SYNCHRONIZE, FALSE, NtCurrentProcessId32(), &nts);
	if (!NT_SUCCESS(nts))
		return HRESULT_FROM_NT(nts);
	if (!SymInitializeW(hProcess, nullptr, bInvadeProcess))
		return HRESULT_FROM_WIN32(NaGetLastError());
	SymSetOptions(dwOptions);
	return S_OK;
}

inline HRESULT DshUnInit(HANDLE hProcess) noexcept
{
	if (!hProcess || hProcess == INVALID_HANDLE_VALUE)
		return E_INVALIDARG;
	HRESULT hr;
	if (SymCleanup(hProcess))
		hr = S_OK;
	else
		hr = HRESULT_FROM_WIN32(NaGetLastError());
	NtClose(hProcess);
	return hr;
}

inline HRESULT DshLoadPdb(HANDLE hProcess, _In_ PCWSTR pszPdbFile,
	DWORD64 DllBase = 0x00401000) noexcept
{
	NTSTATUS nts;
	const auto cbPdb = (DWORD)GetFileSizeWithPath(pszPdbFile, &nts);
	if (!NT_SUCCESS(nts))
		return HRESULT_FROM_NT(nts);
	if (!SymLoadModuleExW(hProcess, nullptr, pszPdbFile,
		nullptr, DllBase, cbPdb, nullptr, 0))
		return HRESULT_FROM_WIN32(NaGetLastError());
	return S_OK;
}
ECK_NAMESPACE_END