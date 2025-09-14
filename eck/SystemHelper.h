#pragma once
#include "NativeWrapper.h"
#include "ComPtr.h"
#include "AutoPtrDef.h"
#include "CRefStr.h"
#include "CRefBin.h"

#include <intrin.h>
#include <wbemcli.h>
#include <comdef.h>

ECK_NAMESPACE_BEGIN
inline LONGLONG GetFileSizeWithPath(PCWSTR pszFile, NTSTATUS* pnts = nullptr)
{
	const auto hFile = NaOpenFile(pszFile, FILE_READ_ATTRIBUTES, FILE_SHARE_READ, 0, pnts);
	if (hFile == INVALID_HANDLE_VALUE)
		return 0ll;
	IO_STATUS_BLOCK iosb;
	FILE_STANDARD_INFORMATION fsi;
	fsi.EndOfFile.QuadPart = 0ll;
	const auto nts = NtQueryInformationFile(hFile, &iosb,
		&fsi, sizeof(fsi), FileStandardInformation);
	if (pnts)
		*pnts = nts;
	NtClose(hFile);
	return fsi.EndOfFile.QuadPart;
}

inline CRefBin ReadInFile(PCWSTR pszFile, NTSTATUS* pnts = nullptr)
{
	const auto hFile = NaOpenFile(pszFile,
		FILE_READ_DATA | SYNCHRONIZE,
		FILE_SHARE_READ,
		FILE_NON_DIRECTORY_FILE | FILE_SEQUENTIAL_ONLY | FILE_SYNCHRONOUS_IO_NONALERT,
		pnts);
	if (hFile == INVALID_HANDLE_VALUE)
		return {};
	NTSTATUS nts;
	if (!pnts) pnts = &nts;
	IO_STATUS_BLOCK iosb;
	FILE_STANDARD_INFORMATION fsi;
	fsi.EndOfFile.QuadPart = 0ll;
	*pnts = NtQueryInformationFile(hFile, &iosb,
		&fsi, sizeof(fsi), FileStandardInformation);
	if (!NT_SUCCESS(*pnts))
	{
		NtClose(hFile);
		return {};
	}
	const auto cbFile = (size_t)fsi.EndOfFile.QuadPart;
	if (cbFile > (size_t)1'073'741'824u)// 大于1G，不读
	{
		*pnts = STATUS_BUFFER_TOO_SMALL;
		NtClose(hFile);
		return {};
	}

	CRefBin rb{};
	rb.Reserve(cbFile + 4/*给调用方预留，例如添加结尾NULL等*/);
	rb.ReSize(cbFile);
	*pnts = NtReadFile(hFile, nullptr, nullptr, nullptr, &iosb,
		rb.Data(), (ULONG)rb.Size(), nullptr, nullptr);
	NtClose(hFile);
	return rb;
}

inline NTSTATUS WriteToFile(PCWSTR pszFile, PCVOID pData, DWORD cb)
{
	NTSTATUS nts;
	const HANDLE hFile = NaCreateFile(pszFile,
		FILE_WRITE_DATA | SYNCHRONIZE,
		FILE_SHARE_READ,
		FILE_NON_DIRECTORY_FILE | FILE_SEQUENTIAL_ONLY | FILE_SYNCHRONOUS_IO_NONALERT,
		FILE_OVERWRITE_IF, &nts);
	if (hFile == INVALID_HANDLE_VALUE)
		return nts;
	IO_STATUS_BLOCK iosb;
	nts = NtWriteFile(hFile, nullptr, nullptr, nullptr, &iosb,
		(void*)pData, cb, nullptr, nullptr);
	NtClose(hFile);
	return nts;
}

EckInline NTSTATUS WriteToFile(PCWSTR pszFile, const CRefBin& rb)
{
	return WriteToFile(pszFile, rb.Data(), (DWORD)rb.Size());
}

namespace Priv
{
	inline void IntEnumFileRecurse(CRefStrW& rs, int cchDir, PCWSTR pszPat,
		std::vector<CRefStrW>& vResult, WIN32_FIND_DATAW* pwfd)
	{
		auto hFind = FindFirstFileExW(rs.Data(), FindExInfoBasic, pwfd,
			FindExSearchNameMatch, nullptr, FIND_FIRST_EX_LARGE_FETCH);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			do
			{
				if ((pwfd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					if (*pwfd->cFileName != L'.')
					{
						rs.PopBack(rs.Size() - cchDir);
						rs.PushBack(pwfd->cFileName);
						rs.PushBackChar(L'\\');
						const int cchNewDir = rs.Size();
						rs.PushBack(pszPat);
						IntEnumFileRecurse(rs, cchNewDir, pszPat, vResult, pwfd);
					}
				}
				else
				{
					if (!pszPat || *pszPat == L'\0' || *pszPat == L'*')
					{
						rs.PopBack(rs.Size() - cchDir);
						vResult.emplace_back(rs + pwfd->cFileName);
					}
					else
					{
						const auto pszExt = PathFindExtensionW(pwfd->cFileName);
						if (_wcsicmp(pszExt, pszPat) == 0)
							vResult.emplace_back(rs + pwfd->cFileName);
					}
				}
			} while (FindNextFileW(hFind, pwfd));
			FindClose(hFind);
		}

		rs.PopBack(rs.Size() - cchDir);
		rs += L'*';
		hFind = FindFirstFileExW(rs.Data(), FindExInfoBasic, pwfd,
			FindExSearchNameMatch, nullptr, FIND_FIRST_EX_LARGE_FETCH);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			do
			{
				if ((pwfd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					if (*pwfd->cFileName != L'.')
					{
						rs.PopBack(rs.Size() - cchDir);
						rs.PushBack(pwfd->cFileName);
						rs.PushBackChar(L'\\');
						const int cchNewDir = rs.Size();
						rs.PushBack(pszPat);
						IntEnumFileRecurse(rs, cchNewDir, pszPat, vResult, pwfd);
					}
				}
			} while (FindNextFileW(hFind, pwfd));
			FindClose(hFind);
		}
	}
}

/// <summary>
/// 递归枚举文件
/// </summary>
/// <param name="pszPath">目录</param>
/// <param name="pszFilePat">文件名或通配符</param>
/// <param name="vResult">枚举结果，不会清空该容器</param>
/// <returns>成功返回TRUE，失败返回FALSE</returns>
inline BOOL EnumFileRecurse(PCWSTR pszPath, PCWSTR pszFilePat, std::vector<CRefStrW>& vResult)
{
	if (!PathIsDirectoryW(pszPath))
		return FALSE;
	CRefStrW rs(pszPath);
	if (rs.Back() != L'\\')
		rs += L'\\';
	const int cchDir = rs.Size();
	rs += pszFilePat;
	WIN32_FIND_DATAW wfd{};
	Priv::IntEnumFileRecurse(rs, cchDir, pszFilePat, vResult, &wfd);
	return TRUE;
}

/// <summary>
/// 递归枚举文件
/// </summary>
/// <param name="pszPath">完整文件名，可含通配符</param>
/// <param name="vResult">枚举结果，不会清空该容器</param>
/// <returns>成功返回TRUE，失败返回FALSE</returns>
EckInline BOOL EnumFileRecurse(PCWSTR pszPath, std::vector<CRefStrW>& vResult)
{
	const auto pszFileName = PathFindFileNameW(pszPath);
	if (pszFileName == pszPath)
		return FALSE;
	const CRefStrW rs(pszPath, int(pszFileName - pszPath));
	EckAssert(rs.Back() == L'\\');
	return EnumFileRecurse(rs.Data(), pszFileName, vResult);
}

inline COLORREF GetCursorPosColor()
{
	POINT pt;
	GetCursorPos(&pt);
	const HDC hDC = GetDC(nullptr);
	const auto cr = GetPixel(hDC, pt.x, pt.y);
	ReleaseDC(nullptr, hDC);
	return cr;
}

inline HRESULT WmiConnectNamespace(_Out_ IWbemServices*& pWbemSrv,
	_Out_ IWbemLocator*& pWbemLoc)
{
	pWbemSrv = nullptr;
	pWbemLoc = nullptr;
	HRESULT hr;
	if (FAILED(hr = CoInitializeSecurity(nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_DEFAULT,
		RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE, nullptr)))
		return hr;
	if (FAILED(hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pWbemLoc))))
		return hr;
	if (FAILED(hr = pWbemLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"),
		nullptr, nullptr, 0, 0, 0, 0, &pWbemSrv)))
	{
		pWbemLoc->Release();
		pWbemLoc = nullptr;
		return hr;
	}
	if (FAILED(hr = CoSetProxyBlanket(pWbemSrv, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr,
		RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE)))
	{
		pWbemSrv->Release();
		pWbemSrv = nullptr;
		pWbemLoc->Release();
		pWbemLoc = nullptr;
		return hr;
	}
	return S_OK;
}

/// <summary>
/// WMI查询类属性
/// </summary>
/// <param name="pszWql">WQL语句</param>
/// <param name="pszProp">属性</param>
/// <param name="Var">查询结果，调用方必须对其调用VariantClear以解分配</param>
/// <param name="pWbemSrv">IWbemServices指针，使用此接口执行查询</param>
/// <returns>HRESULT</returns>
inline HRESULT WmiQueryClassProp(_In_ PCWSTR pszWql, _In_ PCWSTR pszProp,
	_Inout_ VARIANT& Var, _In_ IWbemServices* pWbemSrv)
{
	HRESULT hr;
	ComPtr<IEnumWbemClassObject> pEnum;
	if (FAILED(hr = pWbemSrv->ExecQuery(_bstr_t(L"WQL"), _bstr_t(pszWql),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &pEnum)))
		return hr;
	ComPtr<IWbemClassObject> pClsObj;
	ULONG ulReturned;
	hr = pEnum->Next(WBEM_INFINITE, 1, &pClsObj, &ulReturned);
	if (FAILED(hr) || ulReturned != 1)
		return hr;
	return pClsObj->Get(pszProp, 0, &Var, nullptr, nullptr);
}

/// <summary>
/// WMI查询类属性
/// </summary>
/// <param name="pszWql">WQL语句</param>
/// <param name="pszProp">属性</param>
/// <param name="Var">查询结果</param>
/// <returns>HRESULT</returns>
inline HRESULT WmiQueryClassProp(_In_ PCWSTR pszWql,
	_In_ PCWSTR pszProp, _Inout_  VARIANT& Var)
{
	IWbemServices* pWbemSrv;
	IWbemLocator* pWbemLoc;
	HRESULT hr;
	if (FAILED(hr = WmiConnectNamespace(pWbemSrv, pWbemLoc)))
		return hr;
	hr = WmiQueryClassProp(pszWql, pszProp, Var, pWbemSrv);
	pWbemSrv->Release();
	pWbemLoc->Release();
	return hr;
}

struct CPUINFO
{
	CRefStrW rsVendor;
	CRefStrW rsBrand;
	CRefStrW rsSerialNum;
	CRefStrW rsDescription;
	UINT uL2Cache;// 千字节
	UINT uL3Cache;// 千字节
	UINT uDataWidth;
	UINT cCore;
	UINT cThread;
	UINT uMaxClockSpeed;// 兆赫兹
};

inline HRESULT GetCpuInfo(CPUINFO& ci)
{
#if !defined(_M_ARM64) && !defined(_M_ARM)
	int Register[4];
	// 取制造商
	__cpuid(Register, 0);
	std::swap(Register[2], Register[3]);
	ci.rsVendor = StrX2W((PCSTR)&Register[1], 12);
	/* */if (ci.rsVendor == L"GenuineIntel")
		ci.rsVendor = L"Intel Corporation.";
	else if (ci.rsVendor == L"AuthenticAMD" || ci.rsVendor == L"AMD ISBETTER")
		ci.rsVendor = L"Advanced Micro Devices.";
	else if (ci.rsVendor == L"Geode By NSC")
		ci.rsVendor = L"National Semiconductor.";
	else if (ci.rsVendor == L"CyrixInstead")
		ci.rsVendor = L"Cyrix Corp., VIA Inc.";
	else if (ci.rsVendor == L"NexGenDriven")
		ci.rsVendor = L"NexGen Inc., Advanced Micro Devices.";
	else if (ci.rsVendor == L"CentaurHauls")
		ci.rsVendor = L"IDT\\Centaur, Via Inc.";
	else if (ci.rsVendor == L"UMC UMC UMC ")
		ci.rsVendor = L"United Microelectronics Corp.";
	else if (ci.rsVendor == L"RiseRiseRise")
		ci.rsVendor = L"Rise.";
	else if (ci.rsVendor == L"GenuineTMx86" || ci.rsVendor == L"TransmetaCPU")
		ci.rsVendor = L"Transmeta.";
	// 取商标
	char szBrand[49];
	for (int i = 0x80000002; i <= 0x80000004; ++i)
	{
		__cpuid(Register, i);
		memcpy(szBrand + (i - 0x80000002) * 16, Register, sizeof(Register));
	}
	szBrand[48] = '\0';
	ci.rsBrand = StrX2W(szBrand);
	// 取序列号
	ci.rsSerialNum.ReSize(CchI32ToStrBuf * 2);
	__cpuid(Register, 1);
	int cch = swprintf(ci.rsSerialNum.Data(), L"%08X%08X", Register[3], Register[0]);
	ci.rsSerialNum.ReSize(cch);

	IWbemServices* pWbemSrv;
	IWbemLocator* pWbemLoc;
	HRESULT hr;
	if (FAILED(hr = WmiConnectNamespace(pWbemSrv, pWbemLoc)))
		return hr;

	VARIANT Var{};
	IEnumWbemClassObject* pEnum;
	if (FAILED(hr = pWbemSrv->ExecQuery(_bstr_t(L"WQL"), _bstr_t(L"Select * From Win32_Processor"),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &pEnum)))
	{
		pWbemSrv->Release();
		pWbemLoc->Release();
		return hr;
	}
	IWbemClassObject* pClsObj;
	ULONG ulReturned;
	hr = pEnum->Next(WBEM_INFINITE, 1, &pClsObj, &ulReturned);
	if (ulReturned == 1)
	{
		// 取描述
		if (SUCCEEDED(pClsObj->Get(L"Description", 0, &Var, nullptr, nullptr)))
		{
			ci.rsDescription.DupBSTR(Var.bstrVal);
			VariantClear(&Var);
		}
		// 取二级缓存
		if (SUCCEEDED(pClsObj->Get(L"L2CacheSize", 0, &Var, nullptr, nullptr)))
			ci.uL2Cache = Var.uintVal;
		// 取三级缓存
		if (SUCCEEDED(pClsObj->Get(L"L3CacheSize", 0, &Var, nullptr, nullptr)))
			ci.uL3Cache = Var.uintVal;
		// 取数据宽度
		if (SUCCEEDED(pClsObj->Get(L"DataWidth", 0, &Var, nullptr, nullptr)))
			ci.uDataWidth = Var.uiVal;
		// 取核心数
		if (SUCCEEDED(pClsObj->Get(L"NumberOfCores", 0, &Var, nullptr, nullptr)))
			ci.cCore = Var.uintVal;
		// 取线程数
		if (SUCCEEDED(pClsObj->Get(L"ThreadCount", 0, &Var, nullptr, nullptr)))
			ci.cThread = Var.uintVal;
		// 取最大时钟速度
		if (SUCCEEDED(pClsObj->Get(L"MaxClockSpeed", 0, &Var, nullptr, nullptr)))
			ci.uMaxClockSpeed = Var.uintVal;
		pClsObj->Release();
	}
	pEnum->Release();
	pWbemSrv->Release();
	pWbemLoc->Release();
	return S_OK;
#else
	return E_NOTIMPL;
#endif// __arm__
}

inline BOOL GetDesktopPartHWnd(_Out_ HWND& hPmOrWorkerW, _Out_ HWND& hDefView, _Out_ HWND& hLV)
{
	hDefView = hLV = nullptr;
	if (!(hPmOrWorkerW = FindWindowW(L"Progman", L"Program Manager")))
		return FALSE;
	if (hDefView = FindWindowExW(hPmOrWorkerW, nullptr, L"SHELLDLL_DefView", nullptr))
		if (hLV = FindWindowExW(hDefView, nullptr, L"SysListView32", nullptr))
			return TRUE;
	hPmOrWorkerW = nullptr;
	EckCounterNV(1024)
	{
		if (!(hPmOrWorkerW = FindWindowExW(0, hPmOrWorkerW, L"WorkerW", nullptr)))
			break;
		if (!(hDefView = FindWindowExW(hPmOrWorkerW, nullptr, L"SHELLDLL_DefView", nullptr)))
			continue;
		if (hLV = FindWindowExW(hDefView, nullptr, L"SysListView32", nullptr))
			return TRUE;
	}
	return FALSE;
}

inline BOOL ShowDesktop(BOOL bShow, BOOL bIgnoreProgman = TRUE)
{
	HWND hPmOrWorkerW, hDefView, hLV;
	if (!GetDesktopPartHWnd(hPmOrWorkerW, hDefView, hLV))
		return FALSE;
	const int iSw = (bShow ? SW_SHOWNOACTIVATE : SW_HIDE);
	ShowWindowAsync(hLV, iSw);
	ShowWindowAsync(hDefView, iSw);
	if (!bIgnoreProgman)
		ShowWindowAsync(hPmOrWorkerW, iSw);
	return TRUE;
}

inline BOOL GetTaskBarPartHWnd(_Out_ HWND& hTaskBar,
	_Out_writes_opt_(*pcSecondary) HWND* phSecondary = nullptr,
	_Inout_opt_ size_t* pcSecondary = nullptr)
{
	hTaskBar = FindWindowW(L"Shell_TrayWnd", nullptr);
	if (!hTaskBar)
	{
		if (pcSecondary) *pcSecondary = 0u;
		return FALSE;
	}
	if (phSecondary && pcSecondary)
	{
		HWND hLast{};
		size_t c{};
		while (hLast = FindWindowExW(nullptr, hLast,
			L"Shell_SecondaryTrayWnd", nullptr))
		{
			*phSecondary++ = hLast;
			if (++c == *pcSecondary)
				break;
		}
		*pcSecondary = c;
	}
	return TRUE;
}

inline BOOL ShowTaskBar(BOOL bShow, BOOL bIgnoreSecondary = FALSE)
{
	HWND hTaskBar, hSecondary[16];
	size_t cSecondary = ARRAYSIZE(hSecondary);
	if (!GetTaskBarPartHWnd(hTaskBar, hSecondary,
		bIgnoreSecondary ? nullptr : &cSecondary))
		return FALSE;
	const int iSw = (bShow ? SW_SHOWNOACTIVATE : SW_HIDE);
	if (bIgnoreSecondary)
		ShowWindowAsync(hTaskBar, iSw);
	else// 使用ShowWindowAsync会导致副屏任务栏隐藏后再次显示
	{
		ShowWindow(hTaskBar, iSw);
		EckCounter(cSecondary, i)
			ShowWindow(hSecondary[i], iSw);
	}
	return TRUE;
}

struct FILEVERINFO
{
	CRefStrW Comment;
	CRefStrW InternalName;
	CRefStrW ProductName;
	CRefStrW CompanyName;
	CRefStrW LegalCopyright;
	CRefStrW ProductVersion;
	CRefStrW FileDescription;
	CRefStrW LegalTrademarks;
	CRefStrW PrivateBuild;
	CRefStrW FileVersion;
	CRefStrW OriginalFilename;
	CRefStrW SpecialBuild;
};

inline BOOL GetFileVerInfo(PCWSTR pszFile, FILEVERINFO& fvi)
{
	const DWORD cbBuf = GetFileVersionInfoSizeW(pszFile, nullptr);
	if (!cbBuf)
		return FALSE;
	void* pBuf = malloc(cbBuf);
	EckCheckMem(pBuf);
	UniquePtr<DelMA<void>> _(pBuf);
	if (!GetFileVersionInfoW(pszFile, 0, cbBuf, pBuf))
		return FALSE;

	struct
	{
		WORD wLanguage;
		WORD wCodePage;
	}*pLangCp;
	UINT cbLangCp;
	if (!VerQueryValueW(pBuf, LR"(\VarFileInfo\Translation)", (void**)&pLangCp, &cbLangCp))
		return FALSE;

	WCHAR szLangCp[9];
	_swprintf(szLangCp, L"%04X%04X", pLangCp[0].wLanguage, pLangCp[0].wCodePage);
	CRefStrW rsSub = CRefStrW(LR"(\StringFileInfo\)") + szLangCp + L"\\";
	const auto pszName = rsSub.PushBackNoExtra(20);

	void* pStr;
	UINT cchStr;
	EckCopyConstStringW(pszName, L"Comment");
	VerQueryValueW(pBuf, rsSub.Data(), &pStr, &cchStr);
	fvi.Comment.DupString((PCWSTR)pStr, (int)cchStr);

	EckCopyConstStringW(pszName, L"InternalName");
	VerQueryValueW(pBuf, rsSub.Data(), &pStr, &cchStr);
	fvi.InternalName.DupString((PCWSTR)pStr, (int)cchStr);

	EckCopyConstStringW(pszName, L"ProductName");
	VerQueryValueW(pBuf, rsSub.Data(), &pStr, &cchStr);
	fvi.ProductName.DupString((PCWSTR)pStr, (int)cchStr);

	EckCopyConstStringW(pszName, L"CompanyName");
	VerQueryValueW(pBuf, rsSub.Data(), &pStr, &cchStr);
	fvi.CompanyName.DupString((PCWSTR)pStr, (int)cchStr);

	EckCopyConstStringW(pszName, L"LegalCopyright");
	VerQueryValueW(pBuf, rsSub.Data(), &pStr, &cchStr);
	fvi.LegalCopyright.DupString((PCWSTR)pStr, (int)cchStr);

	EckCopyConstStringW(pszName, L"ProductVersion");
	VerQueryValueW(pBuf, rsSub.Data(), &pStr, &cchStr);
	fvi.ProductVersion.DupString((PCWSTR)pStr, (int)cchStr);

	EckCopyConstStringW(pszName, L"FileDescription");
	VerQueryValueW(pBuf, rsSub.Data(), &pStr, &cchStr);
	fvi.FileDescription.DupString((PCWSTR)pStr, (int)cchStr);

	EckCopyConstStringW(pszName, L"LegalTrademarks");
	VerQueryValueW(pBuf, rsSub.Data(), &pStr, &cchStr);
	fvi.LegalTrademarks.DupString((PCWSTR)pStr, (int)cchStr);

	EckCopyConstStringW(pszName, L"PrivateBuild");
	VerQueryValueW(pBuf, rsSub.Data(), &pStr, &cchStr);
	fvi.PrivateBuild.DupString((PCWSTR)pStr, (int)cchStr);

	EckCopyConstStringW(pszName, L"FileVersion");
	VerQueryValueW(pBuf, rsSub.Data(), &pStr, &cchStr);
	fvi.FileVersion.DupString((PCWSTR)pStr, (int)cchStr);

	EckCopyConstStringW(pszName, L"OriginalFilename");
	VerQueryValueW(pBuf, rsSub.Data(), &pStr, &cchStr);
	fvi.OriginalFilename.DupString((PCWSTR)pStr, (int)cchStr);

	EckCopyConstStringW(pszName, L"SpecialBuild");
	VerQueryValueW(pBuf, rsSub.Data(), &pStr, &cchStr);
	fvi.SpecialBuild.DupString((PCWSTR)pStr, (int)cchStr);
	return TRUE;
}

namespace Priv
{
	template<class T>
	EckInline constexpr INPUT KeyboardEventGetArg(T wVk)
	{
#if ECKCXX20
		return INPUT{ .type = INPUT_KEYBOARD ,.ki = { static_cast<WORD>(wVk) } };
#else
		INPUT input{};
		input.type = INPUT_KEYBOARD;
		input.ki.wVk = static_cast<WORD>(wVk);
		return input;
#endif
	}
}

/// <summary>
/// 模拟按键。
/// 函数顺序按下参数中提供的键，然后倒序将它们放开
/// </summary>
/// <param name="wVk">虚拟键代码</param>
/// <returns>SendInput的返回值</returns>
template<class...T>
inline UINT KeyboardEvent(T...wVk)
{
	INPUT Args[]{ Priv::KeyboardEventGetArg(wVk)... };
	INPUT input[ARRAYSIZE(Args) * 2];
	memcpy(input, Args, sizeof(Args));
	for (auto& e : Args)
		e.ki.dwFlags = KEYEVENTF_KEYUP;
	std::reverse(Args, Args + ARRAYSIZE(Args));
	memcpy(input + ARRAYSIZE(Args), Args, sizeof(Args));
	return SendInput(ARRAYSIZE(input), input, sizeof(INPUT));
}

const CRefStrW& GetRunningPath();

EckInline BOOL SystemTimeToULongLong(const SYSTEMTIME& st, ULONGLONG& ull)
{
	return SystemTimeToFileTime(&st, (FILETIME*)&ull);
}

inline CRefStrW FormatDate(const SYSTEMTIME& st, PCWSTR pszFmt = nullptr, DWORD dwFlags = 0u,
	PCWSTR pszLocale = LOCALE_NAME_USER_DEFAULT)
{
	const int cchDate = GetDateFormatEx(pszLocale, dwFlags, &st, pszFmt, nullptr, 0, nullptr);
	if (!cchDate)
		return {};
	CRefStrW rs(cchDate);
	GetDateFormatEx(pszLocale, dwFlags, &st, pszFmt, rs.Data(), cchDate, nullptr);
	return rs;
}

inline CRefStrW FormatTime(const SYSTEMTIME& st, PCWSTR pszFmt = nullptr, DWORD dwFlags = 0u,
	PCWSTR pszLocale = LOCALE_NAME_USER_DEFAULT)
{
	const int cchTime = GetTimeFormatEx(pszLocale, dwFlags, &st, pszFmt, nullptr, 0);
	if (!cchTime)
		return {};
	CRefStrW rs(cchTime);
	GetTimeFormatEx(pszLocale, dwFlags, &st, pszFmt, rs.Data(), cchTime);
	return rs;
}

inline CRefStrW FormatDateTime(const SYSTEMTIME& st,
	PCWSTR pszFmtDate = nullptr, PCWSTR pszFmtTime = nullptr, DWORD dwFlags = 0u,
	PCWSTR pszLocale = LOCALE_NAME_USER_DEFAULT)
{
	const int cchDate = GetDateFormatEx(pszLocale, dwFlags, &st, pszFmtDate, nullptr, 0, nullptr);
	const int cchTime = GetTimeFormatEx(pszLocale, dwFlags, &st, pszFmtTime, nullptr, 0);
	if (!cchDate || !cchTime)
		return {};
	CRefStrW rs(cchTime + cchDate - 2 + 1);
	GetDateFormatEx(pszLocale, dwFlags, &st, pszFmtDate, rs.Data(), cchDate, nullptr);
	GetTimeFormatEx(pszLocale, dwFlags, &st, pszFmtTime, rs.Data() + cchDate, cchTime);
	rs[cchDate - 1] = L' ';
	return rs;
}

inline void InputChar(WCHAR ch, BOOL bExtended = FALSE, BOOL bReplaceEndOfLine = TRUE)
{
	INPUT input[2]{ {.type = INPUT_KEYBOARD } };
	if (bReplaceEndOfLine && (ch == L'\r' || ch == L'\n'))
		input[0].ki.wVk = VK_RETURN;
	else
	{
		input[0].ki.wScan = ch;
		input[0].ki.dwFlags = KEYEVENTF_UNICODE;
	}

	input[1] = input[0];
	input[1].ki.dwFlags |= KEYEVENTF_KEYUP;
	SendInput(ARRAYSIZE(input), input, sizeof(INPUT));
}

inline void InputChar(PCWSTR pszText, int cchText = -1,
	BOOL bExtended = FALSE, BOOL bReplaceEndOfLine = TRUE)
{
	if (cchText < 0)
		cchText = (int)wcslen(pszText);
	if (bReplaceEndOfLine)
		for (int i{}; i < cchText;)
		{
			auto ch = pszText[i++];
			if (ch == L'\r' && i < cchText && pszText[i] == L'\n')
			{
				ch = L'\n';
				++i;
			}
			InputChar(ch, bExtended);
		}
	else
	{
		EckCounter(cchText, i)
			InputChar(pszText[i], bExtended);
	}
}

inline NTSTATUS NtPathToDosPath(CRefStrW& rsBuf)
{
	RTL_UNICODE_STRING_BUFFER Buf{ rsBuf.ToNtStringBuf() };
	NTSTATUS nts = RtlNtPathNameToDosPathName(0, &Buf, nullptr, nullptr);
	if (NT_SUCCESS(nts))
	{
		rsBuf.ReSize(Buf.String.Length / sizeof(WCHAR));
		goto Success;
	}
	else if (nts == STATUS_NO_MEMORY ||
		nts == STATUS_BUFFER_TOO_SMALL ||
		nts == STATUS_INFO_LENGTH_MISMATCH)
	{
		rsBuf.Reserve(rsBuf.Size() + MAX_PATH);
		Buf = rsBuf.ToNtStringBuf();
		nts = RtlNtPathNameToDosPathName(0, &Buf, nullptr, nullptr);
		if (NT_SUCCESS(nts))
		{
			rsBuf.ReSize(Buf.String.Length / sizeof(WCHAR));
			rsBuf.ShrinkToFit();
			goto Success;
		}
	}
	rsBuf.Clear();
	return nts;
Success:
	if (rsBuf.IsEmpty())
		return STATUS_SUCCESS;
	// 替换设备名
	// TODO:支持网络位置
	if (rsBuf.Front() == L'\\')
	{
		WCHAR szDriver[53];
		WCHAR szLinkTarget[200];
		if (GetLogicalDriveStringsW(ARRAYSIZE(szDriver), szDriver))
			for (auto p = szDriver; *p; p += 4)
			{
				*(p + 2) = L'\0';
				const int cch = (int)QueryDosDeviceW(p, szLinkTarget, ARRAYSIZE(szLinkTarget)) - 2;
				if (cch > 0)
				{
					szLinkTarget[cch] = L'\\';
					if (rsBuf.IsStartWithI(szLinkTarget, cch))
					{
						rsBuf.Replace(0, cch, p, 2);
						break;
					}
				}
			}
	}
	return STATUS_SUCCESS;
}

inline NTSTATUS RestartExplorer()
{
	const auto hWnd = FindWindowW(L"Shell_TrayWnd", nullptr);
	if (!hWnd)
		return STATUS_NOT_FOUND;
	DWORD dwProcessId;
	GetWindowThreadProcessId(hWnd, &dwProcessId);
	NTSTATUS nts;
	const auto hProcess = NaOpenProcess(
		PROCESS_TERMINATE, FALSE, dwProcessId, &nts);
	if (hProcess)
	{
		NtTerminateProcess(hProcess, 2);
		NtClose(hProcess);
	}
	return nts;
}

inline SIZE GetCursorSize(int iDpi)
{
	HKEY hKey;
	DWORD dwBaseSize{}, cb{ sizeof(DWORD) };
	RegOpenKeyExW(HKEY_CURRENT_USER, LR"(Control Panel\Cursors)",
		0, KEY_READ, &hKey);
	if (hKey)
	{
		RegQueryValueExW(hKey, L"CursorBaseSize", nullptr,
			nullptr, (PBYTE)&dwBaseSize, &cb);
		RegCloseKey(hKey);
	}
	return { (int)dwBaseSize,(int)dwBaseSize };
}
ECK_NAMESPACE_END