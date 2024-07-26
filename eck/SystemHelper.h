﻿/*
* WinEzCtrlKit Library
*
* SystemHelper.h ： 系统相关帮助函数
*
* Copyright(C) 2023-2024 QingKong
*/
#pragma once
#include "Utility.h"
#include "Utility2.h"
#include "CException.h"
#include "CFile.h"
#include "CRefBin.h"

#include <intrin.h>

#include <wbemcli.h>
#include <comdef.h>

ECK_NAMESPACE_BEGIN
inline COLORREF GetCursorPosColor()
{
	POINT pt;
	GetCursorPos(&pt);
	const HDC hDC = GetDC(NULL);
	const auto cr = GetPixel(hDC, pt.x, pt.y);
	ReleaseDC(NULL, hDC);
	return cr;
}

/// <summary>
/// 读入文件
/// </summary>
/// <param name="pszFile">文件路径</param>
/// <returns>返回字节集</returns>
inline CRefBin ReadInFile(PCWSTR pszFile)
{
	const HANDLE hFile = CreateFileW(pszFile, GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		EckDbgPrintFormatMessage(GetLastError());
		return {};
	}

	LARGE_INTEGER i64{};
	GetFileSizeEx(hFile, &i64);
	if (i64.QuadPart > 1'073'741'824i64)// 大于1G，不读
	{
		EckDbgPrintFmt(L"文件太大! 尺寸 = %I64d", i64.QuadPart);
		CloseHandle(hFile);
		return {};
	}

	CRefBin rb(i64.LowPart);
	DWORD dwRead;
	if (!ReadFile(hFile, rb.Data(), i64.LowPart, &dwRead, NULL))
	{
		EckDbgPrintFormatMessage(GetLastError());
		CloseHandle(hFile);
		return {};
	}

	CloseHandle(hFile);
	return rb;
}

/// <summary>
/// 写到文件
/// </summary>
/// <param name="pszFile">文件路径</param>
/// <param name="pData">字节流</param>
/// <param name="cb">字节流长度</param>
inline BOOL WriteToFile(PCWSTR pszFile, PCVOID pData, DWORD cb)
{
	const HANDLE hFile = CreateFileW(pszFile, GENERIC_WRITE, FILE_SHARE_READ, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;

	DWORD dwRead;
	const BOOL b = WriteFile(hFile, pData, cb, &dwRead, NULL);
	CloseHandle(hFile);
	return b;
}

/// <summary>
/// 写到文件
/// </summary>
/// <param name="pszFile">文件路径</param>
/// <param name="rb">字节集</param>
EckInline BOOL WriteToFile(PCWSTR pszFile, const CRefBin& rb)
{
	return WriteToFile(pszFile, rb.Data(), (DWORD)rb.Size());
}

/// <summary>
/// WMI连接命名空间
/// </summary>
/// <param name="pWbemSrv">接收IWbemServices指针的变量，该变量的值将被覆盖</param>
/// <param name="pWbemLoc">接收IWbemLocator指针的变量，该变量的值将被覆盖</param>
/// <returns>错误代码</returns>
inline HRESULT WmiConnectNamespace(IWbemServices*& pWbemSrv, IWbemLocator*& pWbemLoc)
{
	pWbemSrv = NULL;
	pWbemLoc = NULL;
	HRESULT hr;
	if (FAILED(hr = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT,
		RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL)))
		return hr;
	if (FAILED(hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pWbemLoc))))
		return hr;
	if (FAILED(hr = pWbemLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"),
		NULL, NULL, 0, NULL, 0, 0, &pWbemSrv)))
	{
		pWbemLoc->Release();
		pWbemLoc = NULL;
		return hr;
	}
	if (FAILED(hr = CoSetProxyBlanket(pWbemSrv, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
		RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE)))
	{
		pWbemSrv->Release();
		pWbemSrv = NULL;
		pWbemLoc->Release();
		pWbemLoc = NULL;
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
/// <returns>错误代码</returns>
inline HRESULT WmiQueryClassProp(PCWSTR pszWql, PCWSTR pszProp, VARIANT& Var, IWbemServices* pWbemSrv)
{
	HRESULT hr;
	ComPtr<IEnumWbemClassObject> pEnum;
	if (FAILED(hr = pWbemSrv->ExecQuery(_bstr_t(L"WQL"), _bstr_t(pszWql),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnum)))
		return hr;
	ComPtr<IWbemClassObject> pClsObj;
	ULONG ulReturned;
	hr = pEnum->Next(WBEM_INFINITE, 1, &pClsObj, &ulReturned);
	if (FAILED(hr) || ulReturned != 1)
		return hr;
	return pClsObj->Get(pszProp, 0, &Var, NULL, NULL);
}

/// <summary>
/// WMI查询类属性
/// </summary>
/// <param name="pszWql">WQL语句</param>
/// <param name="pszProp">属性</param>
/// <param name="Var">查询结果</param>
/// <returns>错误代码</returns>
inline HRESULT WmiQueryClassProp(PCWSTR pszWql, PCWSTR pszProp, VARIANT& Var)
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

/// <summary>
/// 取CPU信息
/// </summary>
/// <param name="ci">接收信息变量</param>
/// <returns>错误代码</returns>
inline HRESULT GetCpuInfo(CPUINFO& ci)
{
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
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnum)))
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
		if (SUCCEEDED(pClsObj->Get(L"Description", 0, &Var, NULL, NULL)))
		{
			ci.rsDescription.DupBSTR(Var.bstrVal);
			VariantClear(&Var);
		}
		// 取二级缓存
		if (SUCCEEDED(pClsObj->Get(L"L2CacheSize", 0, &Var, NULL, NULL)))
			ci.uL2Cache = Var.uintVal;
		// 取三级缓存
		if (SUCCEEDED(pClsObj->Get(L"L3CacheSize", 0, &Var, NULL, NULL)))
			ci.uL3Cache = Var.uintVal;
		// 取数据宽度
		if (SUCCEEDED(pClsObj->Get(L"DataWidth", 0, &Var, NULL, NULL)))
			ci.uDataWidth = Var.uiVal;
		// 取核心数
		if (SUCCEEDED(pClsObj->Get(L"NumberOfCores", 0, &Var, NULL, NULL)))
			ci.cCore = Var.uintVal;
		// 取线程数
		if (SUCCEEDED(pClsObj->Get(L"ThreadCount", 0, &Var, NULL, NULL)))
			ci.cThread = Var.uintVal;
		// 取最大时钟速度
		if (SUCCEEDED(pClsObj->Get(L"MaxClockSpeed", 0, &Var, NULL, NULL)))
			ci.uMaxClockSpeed = Var.uintVal;
		pClsObj->Release();
	}
	pEnum->Release();
	pWbemSrv->Release();
	pWbemLoc->Release();
	return S_OK;
}

/// <summary>
/// 显示/隐藏桌面
/// </summary>
/// <param name="bShow">是否显示</param>
/// <returns>桌面ListView句柄</returns>
inline HWND ShowDesktop(BOOL bShow)
{
	HWND hPm = FindWindowW(L"Progman", L"Program Manager");
	HWND hDefView = FindWindowExW(hPm, NULL, L"SHELLDLL_DefView", NULL);
	HWND hLV = FindWindowExW(hDefView, NULL, L"SysListView32", NULL);
	if (!hLV)
	{
		EckCounterNV(1000)
		{
			hPm = FindWindowExW(0, hPm, L"WorkerW", NULL);
			hDefView = FindWindowExW(hPm, NULL, L"SHELLDLL_DefView", NULL);
			hLV = FindWindowExW(hDefView, NULL, L"SysListView32", NULL);
			if (hLV)
				goto Succeeded;
		}
		EckDbgPrintWithPos(L"寻找桌面窗口失败");
		return NULL;
	Succeeded:;
	}
	const int iSw = (bShow ? SW_SHOWNOACTIVATE : SW_HIDE);
	ShowWindowAsync(hLV, iSw);
	ShowWindowAsync(hDefView, iSw);
	ShowWindowAsync(hPm, iSw);
	return hLV;
}

/// <summary>
/// 显示/隐藏任务栏
/// </summary>
/// <param name="bShow">是否显示</param>
/// <returns>任务栏句柄</returns>
EckInline HWND ShowTaskBar(BOOL bShow)
{
	const HWND hTb = FindWindowW(L"Shell_TrayWnd", NULL);
	ShowWindowAsync(hTb, (bShow ? SW_SHOWNOACTIVATE : SW_HIDE));
	return hTb;
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

/// <summary>
/// 取文件版本信息
/// </summary>
/// <param name="pszFile">文件名</param>
/// <param name="fvi">版本信息</param>
/// <returns>成功返回TRUE，失败返回FALSE</returns>
inline BOOL GetFileVerInfo(PCWSTR pszFile, FILEVERINFO& fvi)
{
	const DWORD cbBuf = GetFileVersionInfoSizeW(pszFile, NULL);
	if (!cbBuf)
		return FALSE;
	void* pBuf = malloc(cbBuf);
	EckCheckMem(pBuf);
	UniquePtrCrtMA<void> _(pBuf);
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

	void* pStr;
	UINT cchStr;
	VerQueryValueW(pBuf, (rsSub + L"Comment").Data(), &pStr, &cchStr);
	fvi.Comment.DupString((PCWSTR)pStr, (int)cchStr);
	VerQueryValueW(pBuf, (rsSub + L"InternalName").Data(), &pStr, &cchStr);
	fvi.InternalName.DupString((PCWSTR)pStr, (int)cchStr);
	VerQueryValueW(pBuf, (rsSub + L"ProductName").Data(), &pStr, &cchStr);
	fvi.ProductName.DupString((PCWSTR)pStr, (int)cchStr);
	VerQueryValueW(pBuf, (rsSub + L"CompanyName").Data(), &pStr, &cchStr);
	fvi.CompanyName.DupString((PCWSTR)pStr, (int)cchStr);
	VerQueryValueW(pBuf, (rsSub + L"LegalCopyright").Data(), &pStr, &cchStr);
	fvi.LegalCopyright.DupString((PCWSTR)pStr, (int)cchStr);
	VerQueryValueW(pBuf, (rsSub + L"ProductVersion").Data(), &pStr, &cchStr);
	fvi.ProductVersion.DupString((PCWSTR)pStr, (int)cchStr);
	VerQueryValueW(pBuf, (rsSub + L"FileDescription").Data(), &pStr, &cchStr);
	fvi.FileDescription.DupString((PCWSTR)pStr, (int)cchStr);
	VerQueryValueW(pBuf, (rsSub + L"LegalTrademarks").Data(), &pStr, &cchStr);
	fvi.LegalTrademarks.DupString((PCWSTR)pStr, (int)cchStr);
	VerQueryValueW(pBuf, (rsSub + L"PrivateBuild").Data(), &pStr, &cchStr);
	fvi.PrivateBuild.DupString((PCWSTR)pStr, (int)cchStr);
	VerQueryValueW(pBuf, (rsSub + L"FileVersion").Data(), &pStr, &cchStr);
	fvi.FileVersion.DupString((PCWSTR)pStr, (int)cchStr);
	VerQueryValueW(pBuf, (rsSub + L"OriginalFilename").Data(), &pStr, &cchStr);
	fvi.OriginalFilename.DupString((PCWSTR)pStr, (int)cchStr);
	VerQueryValueW(pBuf, (rsSub + L"SpecialBuild").Data(), &pStr, &cchStr);
	fvi.SpecialBuild.DupString((PCWSTR)pStr, (int)cchStr);
	return TRUE;
}

namespace EckPriv
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

template<class...T>
/// <summary>
/// 模拟按键。
/// 函数顺序按下参数中提供的键，然后倒序将它们放开
/// </summary>
/// <param name="wVk">虚拟键代码</param>
/// <returns>SendInput的返回值</returns>
inline UINT KeyboardEvent(T...wVk)
{
	INPUT Args[]{ EckPriv::KeyboardEventGetArg(wVk)... };
	INPUT input[ARRAYSIZE(Args) * 2];
	memcpy(input, Args, sizeof(Args));
	for (auto& e : Args)
		e.ki.dwFlags = KEYEVENTF_KEYUP;
	std::reverse(Args, Args + ARRAYSIZE(Args));
	memcpy(input + ARRAYSIZE(Args), Args, sizeof(Args));
	return SendInput(ARRAYSIZE(input), input, sizeof(INPUT));
}

const CRefStrW& GetRunningPath();

/// <summary>
/// CRT创建线程。
/// （_beginthreadex wrapper）
/// </summary>
/// <param name="pStartAddress">起始地址</param>
/// <param name="pParameter">参数</param>
/// <param name="pThreadId">线程ID变量指针</param>
/// <param name="dwCreationFlags">标志</param>
/// <returns>线程句柄</returns>
[[nodiscard]] EckInline HANDLE CrtCreateThread(_beginthreadex_proc_type pStartAddress,
	void* pParameter = NULL, UINT* pThreadId = NULL, UINT dwCreationFlags = 0)
{
	return (HANDLE)_beginthreadex(0, 0, pStartAddress, pParameter, dwCreationFlags, pThreadId);
}

inline NTSTATUS NtPathToDosPath(CRefStrW& rsBuf)
{
	RTL_UNICODE_STRING_BUFFER Buf{ rsBuf.ToNtStringBuf() };
	NTSTATUS nts = RtlNtPathNameToDosPathName(0, &Buf, NULL, NULL);
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
		nts = RtlNtPathNameToDosPathName(0, &Buf, NULL, NULL);
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
					if (rsBuf.IsStartOfI(szLinkTarget, cch))
					{
						rsBuf.Replace(0, cch, p, 2);
						break;
					}
				}
			}
	}
	return STATUS_SUCCESS;
}

inline NTSTATUS GetProcessPath(UINT uPid, CRefStrW& rsPath, BOOL bDosPath = TRUE)
{
	SYSTEM_PROCESS_ID_INFORMATION spii{ .ProcessId = i32ToP<HANDLE>(uPid) };
	NTSTATUS nts = NtQuerySystemInformation(SystemProcessIdInformation, &spii, sizeof(spii), NULL);
	if (spii.ImageName.MaximumLength && nts == STATUS_INFO_LENGTH_MISMATCH)
	{
		rsPath.ReSize(spii.ImageName.MaximumLength);
		spii.ImageName.Buffer = rsPath.Data();
		spii.ImageName.Length = 0;
		nts = NtQuerySystemInformation(SystemProcessIdInformation, &spii, sizeof(spii), NULL);
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

[[nodiscard]] inline HICON GetWindowSmallIcon(HWND hWnd, int msTimeOut = 300)
{
	HICON hIcon;
	if (!SendMessageTimeoutW(hWnd, WM_GETICON, ICON_SMALL, 0,
		SMTO_ABORTIFHUNG | SMTO_BLOCK | SMTO_ERRORONEXIT, msTimeOut, (DWORD_PTR*)&hIcon))
		return NULL;
	if (!hIcon)
	{
		hIcon = (HICON)GetClassLongPtrW(hWnd, GCLP_HICONSM);
		if (!hIcon)
		{
			if (!SendMessageTimeoutW(hWnd, WM_GETICON, ICON_SMALL, 0,
				SMTO_ABORTIFHUNG | SMTO_BLOCK | SMTO_ERRORONEXIT, msTimeOut, (DWORD_PTR*)&hIcon))
				return NULL;
		}
	}
	return hIcon;
}

[[nodiscard]] EckInline HICON GetWindowLargeIcon(HWND hWnd, int msTimeOut = 300)
{
	HICON hIcon;
	if (!SendMessageTimeoutW(hWnd, WM_GETICON, ICON_BIG, 0,
		SMTO_ABORTIFHUNG | SMTO_BLOCK | SMTO_ERRORONEXIT, msTimeOut, (DWORD_PTR*)&hIcon))
		return NULL;
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
		return NULL;
	CRefStrW rsPath{};
	if (!NT_SUCCESS(GetProcessPath(dwPid, rsPath)))
		return NULL;

	SHFILEINFOW sfi;
	const UINT uFlags = (bSmall ? (SHGFI_ICON | SHGFI_SMALLICON) : SHGFI_ICON);
#pragma warning(suppress:6001)
	if (!SHGetFileInfoW(rsPath.Data(), 0, &sfi, sizeof(sfi), uFlags))
		SHGetFileInfoW(rsPath.Data(), FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(sfi),
			uFlags | SHGFI_USEFILEATTRIBUTES);
	bNeedDestroy = (sfi.hIcon != NULL);
	return sfi.hIcon;
}

EckInline BOOL SystemParametersInfoDpi(UINT uAction, UINT uParam, PVOID pParam, UINT fWinIni, int iDpi)
{
#if ECKDPIAPI
	return SystemParametersInfoForDpi(uAction, uParam, pParam, fWinIni, iDpi);
#else
	return SystemParametersInfoW(uAction, uParam, pParam, fWinIni);
#endif
}

[[nodiscard]] EckInline HFONT CreateDefFont(int iDpi = USER_DEFAULT_SCREEN_DPI)
{
	LOGFONTW lf;
	SystemParametersInfoDpi(SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, 0, iDpi);
	return CreateFontIndirectW(&lf);
}

EckInline BOOL GetDefFontInfo(LOGFONTW& lf, int iDpi = USER_DEFAULT_SCREEN_DPI)
{
	return SystemParametersInfoDpi(SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, 0, iDpi);
}

[[nodiscard]] EckInline IDWriteTextFormat* CreateDefTextFormat(int iDpi = USER_DEFAULT_SCREEN_DPI, HRESULT* phr = NULL)
{
	LOGFONTW lf;
	if (!GetDefFontInfo(lf, iDpi))
		return NULL;
	IDWriteTextFormat* pTextFormat;
	auto hr = g_pDwFactory->CreateTextFormat(
		lf.lfFaceName,
		NULL,
		(DWRITE_FONT_WEIGHT)lf.lfWeight,
		lf.lfItalic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		(float)Abs(lf.lfHeight),
		L"zh-cn",
		&pTextFormat);
	if (phr)
		*phr = hr;
	return pTextFormat;
}

inline NTSTATUS GetPidByProcessName(PCWSTR pszImageName, UINT& uPid)
{
	uPid = 0u;
	ULONG cb;
	NTSTATUS nts = NtQuerySystemInformation(SystemProcessInformation, NULL, 0, &cb);
	if (!cb)
		return nts;
	BYTE* pBuf = (BYTE*)VAlloc(cb);
	UniquePtrVA<BYTE> _(pBuf);
	if (!NT_SUCCESS(nts = NtQuerySystemInformation(SystemProcessInformation, pBuf, cb, &cb)))
		return nts;
	SYSTEM_PROCESS_INFORMATION* pspi = (SYSTEM_PROCESS_INFORMATION*)pBuf;
	for (;;)
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
	NTSTATUS nts = NtQuerySystemInformation(SystemProcessInformation, NULL, 0, &cb);
	if (!cb)
		return nts;
	BYTE* pBuf = (BYTE*)VAlloc(cb);
	UniquePtrVA<BYTE> _(pBuf);
	if (!NT_SUCCESS(nts = NtQuerySystemInformation(SystemProcessInformation, pBuf, cb, &cb)))
		return nts;
	SYSTEM_PROCESS_INFORMATION* pspi = (SYSTEM_PROCESS_INFORMATION*)pBuf;
	for (;;)
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

[[nodiscard]] EckInline HMONITOR MonitorFromRectByWorkArea(const RECT& rc,
	HMONITOR* phMonMain = NULL, HMONITOR* phMonNearest = NULL)
{
	struct CTX
	{
		const RECT& rc;
		int iMinDistance;
		int iMaxArea;
		HMONITOR hMon;
		HMONITOR hMonMain;
		HMONITOR hMonNearest;
	}
	Ctx{ rc,INT_MAX };

	EnumDisplayMonitors(NULL, NULL, [](HMONITOR hMonitor, HDC, RECT*, LPARAM lParam)->BOOL
		{
			const auto pCtx = (CTX*)lParam;

			MONITORINFO mi;
			mi.cbSize = sizeof(mi);
			GetMonitorInfoW(hMonitor, &mi);
			if (IsBitSet(mi.dwFlags, MONITORINFOF_PRIMARY))
				pCtx->hMonMain = hMonitor;

			RECT rc;
			if (IntersectRect(rc, mi.rcWork, pCtx->rc))
			{
				const int iArea = (rc.right - rc.left) * (rc.bottom - rc.top);
				if (iArea > pCtx->iMaxArea)
				{
					pCtx->iMaxArea = iArea;
					pCtx->hMon = hMonitor;
				}
			}

			const int dx = (pCtx->rc.left + pCtx->rc.right) / 2 -
				(mi.rcWork.left + mi.rcWork.right) / 2;
			const int dy = (pCtx->rc.top + pCtx->rc.bottom) / 2 -
				(mi.rcWork.top + mi.rcWork.bottom) / 2;
			const int d = dx * dx + dy * dy;
			if (d < pCtx->iMinDistance)
			{
				pCtx->iMinDistance = d;
				pCtx->hMonNearest = hMonitor;
			}
			return TRUE;
		}, (LPARAM)&Ctx);

	if (phMonMain)
		*phMonMain = Ctx.hMonMain;
	if (phMonNearest)
		*phMonNearest = Ctx.hMonNearest;
	return Ctx.hMon;
}

inline CRefStrW GetFileNameFromPath(PCWSTR pszPath, int cchPath = -1)
{
	if (cchPath < 0)
		cchPath = (int)wcslen(pszPath);
	const PWSTR pTemp = (PWSTR)_malloca(Cch2CbW(cchPath));
	EckCheckMem(pTemp);
	wmemcpy(pTemp, pszPath, cchPath + 1);

	const auto pszFileName = PathFindFileNameW(pTemp);
	PathRemoveExtensionW(pszFileName);
	const auto rs = CRefStrW(pszFileName);

	_freea(pTemp);
	return rs;
}

EckInline BOOL SystemTimeToULongLong(const SYSTEMTIME& st, ULONGLONG& ull)
{
	return SystemTimeToFileTime(&st, (FILETIME*)&ull);
}

inline CRefStrW FormatDate(const SYSTEMTIME& st, PCWSTR pszFmt, DWORD dwFlags = 0u,
	PCWSTR pszLocale = LOCALE_NAME_USER_DEFAULT)
{
	const int cchDate = GetDateFormatEx(pszLocale, dwFlags, &st, pszFmt, NULL, 0, NULL);
	if (!cchDate)
		return {};
	CRefStrW rs(cchDate);
	GetDateFormatEx(pszLocale, dwFlags, &st, pszFmt, rs.Data(), cchDate, NULL);
	return rs;
}

inline CRefStrW FormatTime(const SYSTEMTIME& st, PCWSTR pszFmt, DWORD dwFlags = 0u,
	PCWSTR pszLocale = LOCALE_NAME_USER_DEFAULT)
{
	const int cchTime = GetTimeFormatEx(pszLocale, dwFlags, &st, pszFmt, NULL, 0);
	if (!cchTime)
		return {};
	CRefStrW rs(cchTime);
	GetTimeFormatEx(pszLocale, dwFlags, &st, pszFmt, rs.Data(), cchTime);
	return rs;
}

inline CRefStrW FormatDateTime(const SYSTEMTIME& st, PCWSTR pszFmt, DWORD dwFlags = 0u,
	PCWSTR pszLocale = LOCALE_NAME_USER_DEFAULT)
{
	const int cchDate = GetDateFormatEx(pszLocale, dwFlags, &st, pszFmt, NULL, 0, NULL);
	const int cchTime = GetTimeFormatEx(pszLocale, dwFlags, &st, pszFmt, NULL, 0);
	if (!cchDate || !cchTime)
		return {};
	CRefStrW rs(cchTime + cchDate - 1);
	GetDateFormatEx(pszLocale, dwFlags, &st, pszFmt, rs.Data(), cchDate, NULL);
	GetTimeFormatEx(pszLocale, dwFlags, &st, pszFmt, rs.Data() + cchDate - 1, cchTime);
	return rs;
}

namespace EckPriv
{
	inline void IntEnumFileRecurse(CRefStrW& rs, int cchDir, PCWSTR pszPat,
		std::vector<CRefStrW>& vResult, WIN32_FIND_DATAW* pwfd)
	{
		auto hFind = FindFirstFileExW(rs.Data(), FindExInfoBasic, pwfd,
			FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH);
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
			FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH);
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
	EckPriv::IntEnumFileRecurse(rs, cchDir, pszFilePat, vResult, &wfd);
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

namespace EckPriv
{
	UINT OpenInExplorerThread(void* pParam)
	{
		const auto pvPath = (std::vector<CRefStrW>*)pParam;
		if (FAILED(CoInitialize(NULL)))
		{
			delete pvPath;
			return 0;
		}

		std::unordered_map<std::wstring_view, int> hmPaths{};// 文件夹路径->vPIDL索引
		std::vector<std::pair<LPITEMIDLIST, std::vector<LPITEMIDLIST>>> vPIDL{};// { 文件夹PIDL,{文件PIDL} }
		LPITEMIDLIST pIDL;

		int idxCurr = 0;

		PWSTR pszFileName;
		PCWSTR pszPath;
		for (const auto& x : *pvPath)
		{
			pszPath = x.Data();
			pszFileName = PathFindFileNameW(pszPath);
			if (pszFileName != pszPath)
			{
				const std::wstring_view svTemp(pszPath, pszFileName - pszPath);
				auto it = hmPaths.find(svTemp);
				if (it == hmPaths.end())
				{
					WCHAR ch = *(pszFileName - 1);
					*(pszFileName - 1) = L'\0';
					if (FAILED(SHParseDisplayName(pszPath, NULL, &pIDL, 0, NULL)))// 文件夹转PIDL
					{
						*(pszFileName - 1) = ch;
						continue;
					}
					*(pszFileName - 1) = ch;

					it = hmPaths.insert(std::make_pair(svTemp, idxCurr)).first;
					++idxCurr;

					auto& x = vPIDL.emplace_back(pIDL, std::vector<LPITEMIDLIST>());
					if (FAILED(SHParseDisplayName(pszPath, NULL, &pIDL, 0, NULL)))// 文件转PIDL
						continue;
					x.second.emplace_back(pIDL);
				}
				else
				{
					SHParseDisplayName(pszPath, NULL, &pIDL, 0, NULL);// 文件转PIDL
					vPIDL[it->second].second.emplace_back(pIDL);
				}
			}
		}

		for (const auto& x : vPIDL)
		{
			SHOpenFolderAndSelectItems(x.first, (UINT)x.second.size(), (LPCITEMIDLIST*)x.second.data(), 0);
			CoTaskMemFree(x.first);
			for (const auto pidl : x.second)
				CoTaskMemFree(pidl);
		}

		delete pvPath;
		CoUninitialize();
		return 0;
	}
}

/// <summary>
/// 在资源管理器中打开。
/// 可一次性传递多个文件，且父目录可以不同
/// </summary>
/// <param name="vPath">路径</param>
EckInline void OpenInExplorer(const std::vector<CRefStrW>& vPath)
{
	CloseHandle(CrtCreateThread(EckPriv::OpenInExplorerThread, new std::vector{ vPath }));
}

/// <summary>
/// 在资源管理器中打开。
/// 可一次性传递多个文件，且父目录可以不同
/// </summary>
/// <param name="pvPath">路径vector指针，必须使用new分配且传递后不可再使用</param>
EckInline void OpenInExplorer(std::vector<CRefStrW>* pvPath)
{
	CloseHandle(CrtCreateThread(EckPriv::OpenInExplorerThread, pvPath));
}

/// <summary>
/// 在资源管理器中打开
/// </summary>
/// <param name="pszFolder">文件夹路径</param>
/// <param name="vFile">文件路径，必须全部在pszFolder指定的文件夹之下</param>
/// <returns>HRESULT</returns>
EckInline HRESULT OpenInExplorer(PCWSTR pszFolder, const std::vector<CRefStrW>& vFile)
{
	HRESULT hr;
	PITEMIDLIST pIDL;
	if (FAILED(hr = SHParseDisplayName(pszFolder, NULL, &pIDL, 0, NULL)))
		return hr;
	std::vector<PITEMIDLIST> vPIDL(vFile.size());
	for (auto& e : vFile)
	{
		if (FAILED(hr = SHParseDisplayName(e.Data(), NULL, &vPIDL.emplace_back(), 0, NULL)))
			goto CleanupAndRet;
	}
	hr = SHOpenFolderAndSelectItems(pIDL, (UINT)vPIDL.size(), (PCITEMIDLIST*)vPIDL.data(), 0);
CleanupAndRet:
	CoTaskMemFree(pIDL);
	for (const auto e : vPIDL)
		CoTaskMemFree(e);
	return hr;
}

inline HRESULT OpenInExplorer(PCWSTR pszFile)
{
	const auto psz = PathFindFileNameW(pszFile);
	if (psz == pszFile)
		return E_INVALIDARG;
	const size_t cbFolder = Cch2CbW(int(psz - pszFile - 1));
	const auto pszFolder = (PWSTR)_malloca(cbFolder);
	EckCheckMem(pszFolder);
	wmemcpy(pszFolder, pszFile, cbFolder);
	*(pszFolder + cbFolder / sizeof(WCHAR) - 1) = L'\0';
	PITEMIDLIST pIdlFolder, pIdlFile;
	HRESULT hr;
	if (FAILED(hr = SHParseDisplayName(pszFolder, NULL, &pIdlFolder, 0, NULL)))
		goto CleanupAndRet;
	if (FAILED(hr = SHParseDisplayName(psz, NULL, &pIdlFile, 0, NULL)))
		goto CleanupAndRet;
	hr = SHOpenFolderAndSelectItems(pIdlFolder, 1, (PCITEMIDLIST*)&pIdlFile, 0);
CleanupAndRet:
	CoTaskMemFree(pIdlFolder);
	CoTaskMemFree(pIdlFile);
	_freea(pszFolder);
	return hr;
}

EckInline void* GetProcessPeb(HANDLE hProcess, NTSTATUS* pnts = NULL)
{
	PROCESS_BASIC_INFORMATION pbi;
	NTSTATUS nts;
	if (!NT_SUCCESS(nts = NtQueryInformationProcess(hProcess, ProcessBasicInformation, &pbi, sizeof(pbi), NULL)))
	{
		if (pnts)
			*pnts = nts;
		return NULL;
	}
	if (pnts)
		*pnts = STATUS_SUCCESS;
	return pbi.PebBaseAddress;
}

inline HANDLE NaOpenProcess(DWORD dwDesiredAccess, BOOL bInheritHandle, UINT uProcessId)
{
	OBJECT_ATTRIBUTES oa;
	InitializeObjectAttributes(&oa, NULL, bInheritHandle ? OBJ_INHERIT : 0, NULL, NULL);
	CLIENT_ID cid{ pToI32<HANDLE>(uProcessId) };
	HANDLE hProcess;
	if (NT_SUCCESS(NtOpenProcess(&hProcess, dwDesiredAccess, &oa, &cid)))
		return hProcess;
	return NULL;
}

struct MODULE_INFO
{
	CRefStrW rsModuleName;
	CRefStrW rsModulePath;
	void* BaseAddress;
	SIZE_T cbImage;
};

/// <summary>
/// 枚举进程模块
/// </summary>
/// <param name="hProcess">进程句柄，必须具有PROCESS_QUERY_INFORMATION | PROCESS_VM_READ权限</param>
/// <param name="vResult">枚举结果，不会清空该容器</param>
/// <returns>NTSTATUS</returns>
inline NTSTATUS EnumProcessModules(HANDLE hProcess, std::vector<MODULE_INFO>& vResult)
{
	NTSTATUS nts;
	// 取PEB
	UINT_PTR pPeb = (UINT_PTR)GetProcessPeb(hProcess, &nts);
	if (!pPeb)
		return nts;
	// 取PEB_LDR_DATA
	PEB_LDR_DATA LdrData;
	UINT_PTR pLdr;
	if (!NT_SUCCESS(nts = NtReadVirtualMemory(hProcess, (void*)(pPeb + offsetof(PEB, Ldr)),
		&pLdr, sizeof(pLdr), NULL)))
		return nts;
	if (!NT_SUCCESS(nts = NtReadVirtualMemory(hProcess, (void*)pLdr,
		&LdrData, sizeof(LdrData), NULL)))
		return nts;
	if (!LdrData.Initialized)
		return STATUS_UNSUCCESSFUL;

	LDR_DATA_TABLE_ENTRY Entry;// 不要使用Win7之后添加的字段
	UINT_PTR pBegin = pLdr + offsetof(PEB_LDR_DATA, InLoadOrderModuleList);
	for (UINT_PTR p = (UINT_PTR)LdrData.InLoadOrderModuleList.Flink; p != pBegin; )
	{
		if (!NT_SUCCESS(nts = NtReadVirtualMemory(hProcess, (void*)p,
			&Entry, LDR_DATA_TABLE_ENTRY_SIZE_WIN7, NULL)))
			return nts;
		if (Entry.DllBase)
		{
			auto& e = vResult.emplace_back();
			if (Entry.BaseDllName.Length)
			{
				const int cch = Entry.BaseDllName.Length / sizeof(WCHAR);
				e.rsModuleName.ReSize(cch);
				if (!NT_SUCCESS(nts = NtReadVirtualMemory(hProcess, (void*)Entry.BaseDllName.Buffer,
					e.rsModuleName.Data(), cch * sizeof(WCHAR), NULL)))
					return nts;
			}
			if (Entry.FullDllName.Length)
			{
				const int cch = Entry.FullDllName.Length / sizeof(WCHAR);
				e.rsModulePath.ReSize(cch);
				if (!NT_SUCCESS(nts = NtReadVirtualMemory(hProcess, (void*)Entry.FullDllName.Buffer,
					e.rsModulePath.Data(), cch * sizeof(WCHAR), NULL)))
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
	CRefStrW rsImageName;	// 进程名
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
	CRefStrW rsFilePath;	// 进程路径
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
/// <param name="vResult">枚举结果，不会清空该容器</param>
/// <param name="uFlags">EPF_常量</param>
/// <returns>NTSTATUS</returns>
inline NTSTATUS EnumProcess(std::vector<PROCESS_INFO>& vResult, EPFLAGS uFlags = EPF_NONE)
{
	ULONG cb;
	NTSTATUS nts = NtQuerySystemInformation(SystemProcessInformation, NULL, 0, &cb);
	if (!cb)
		return nts;
	BYTE* pBuf = (BYTE*)VAlloc(cb);
	UniquePtrVA<BYTE> _(pBuf);
	if (!NT_SUCCESS(nts = NtQuerySystemInformation(SystemProcessInformation, pBuf, cb, &cb)))
		return nts;
	vResult.reserve(150u);
	auto pspi = (SYSTEM_PROCESS_INFORMATION*)pBuf;
	for (;;)
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
/// 调整进程令牌特权
/// </summary>
/// <param name="hProcess">进程句柄，必须具有PROCESS_QUERY_LIMITED_INFORMATION权限</param>
/// <param name="pszPrivilege">特权名</param>
/// <param name="bEnable">是否启用</param>
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
	nts = NtAdjustPrivilegesToken(hToken, FALSE, &tp, 0, NULL, NULL);
	NtClose(hToken);
	return nts;
}

/// <summary>
/// 打开文件
/// </summary>
/// <param name="pusFile">文件的NT路径</param>
/// <param name="dwAccess">CreateFileW --> dwAccess，注意CreateFileW总是追加SYNCHRONIZE | FILE_READ_ATTRIBUTES，考虑使用FILE_GENERIC_*</param>
/// <param name="dwShareMode">CreateFileW --> dwShareMode</param>
/// <param name="dwOptions">NtCreateFile --> CreateOptions</param>
/// <param name="pnts">错误码</param>
/// <param name="piost">IO状态</param>
/// <param name="bInheritHandle">句柄是否可继承</param>
/// <param name="hRootDirectory">根目录句柄</param>
/// <returns>成功返回文件句柄，失败返回INVALID_HANDLE_VALUE</returns>
inline HANDLE NaOpenFile(UNICODE_STRING* pusFile, DWORD dwAccess, DWORD dwShareMode, DWORD dwOptions = 0u,
	NTSTATUS* pnts = NULL, IO_STATUS_BLOCK* piost = NULL, BOOL bInheritHandle = FALSE, HANDLE hRootDirectory = NULL)
{
	OBJECT_ATTRIBUTES oa;
	InitializeObjectAttributes(&oa, pusFile, OBJ_CASE_INSENSITIVE, hRootDirectory, NULL);
	HANDLE hFile;
	NTSTATUS nts;
	IO_STATUS_BLOCK iost;
	nts = NtOpenFile(&hFile, dwAccess, &oa, piost ? piost : &iost, dwShareMode, 0);
	if (pnts)
		*pnts = nts;
	if (NT_SUCCESS(nts))
		return hFile;
	return INVALID_HANDLE_VALUE;
}

/// <summary>
/// 打开文件。
/// 封装与CreateFileW相近的Native API调用，不支持打开控制台
/// </summary>
/// <param name="pszFile">文件路径</param>
/// <param name="dwAccess">CreateFileW --> dwAccess，注意CreateFileW总是追加SYNCHRONIZE | FILE_READ_ATTRIBUTES，考虑使用FILE_GENERIC_*</param>
/// <param name="dwShareMode">CreateFileW --> dwShareMode</param>
/// <param name="dwOptions">NtCreateFile --> CreateOptions</param>
/// <param name="pnts">错误码</param>
/// <param name="piost">IO状态</param>
/// <param name="bInheritHandle">句柄是否可继承</param>
/// <returns>成功返回文件句柄，失败返回INVALID_HANDLE_VALUE</returns>
inline HANDLE NaOpenFile(PCWSTR pszFile, DWORD dwAccess, DWORD dwShareMode, DWORD dwOptions = 0u,
	NTSTATUS* pnts = NULL, IO_STATUS_BLOCK* piost = NULL, BOOL bInheritHandle = FALSE)
{
	UNICODE_STRING usFile;
	RtlInitUnicodeString(&usFile, pszFile);
	if (!RtlDosPathNameToNtPathName_U(pszFile, &usFile, NULL, NULL))
	{
		if (pnts)
			*pnts = STATUS_OBJECT_PATH_NOT_FOUND;
		return INVALID_HANDLE_VALUE;
	}
	const auto hFile = NaOpenFile(&usFile, dwAccess, dwShareMode, dwOptions,
		pnts, piost, bInheritHandle);
	RtlFreeHeap(RtlProcessHeap(), 0, usFile.Buffer);
	return hFile;
}

/// <summary>
/// 打开文件
/// </summary>
/// <param name="pusFile">文件的NT路径</param>
/// <param name="dwAccess">CreateFileW --> dwAccess</param>
/// <param name="dwShareMode">CreateFileW --> dwShareMode</param>
/// <param name="dwOptions">NtCreateFile --> CreateOptions</param>
/// <param name="dwCreationDisposition">NtCreateFile --> CreateDisposition</param>
/// <param name="pnts">错误码</param>
/// <param name="piost">IO状态</param>
/// <param name="dwAttributes">若创建文件，则此参数指定文件属性</param>
/// <param name="cbInit">若创建文件，则此参数指定初始大小</param>
/// <param name="bInheritHandle">句柄是否可继承</param>
/// <param name="hRootDirectory">根目录句柄</param>
/// <returns>成功返回文件句柄，失败返回INVALID_HANDLE_VALUE</returns>
inline HANDLE NaCreateFile(UNICODE_STRING* pusFile, DWORD dwAccess, DWORD dwShareMode, DWORD dwOptions,
	DWORD dwCreationDisposition, NTSTATUS* pnts = NULL, IO_STATUS_BLOCK* piost = NULL,
	DWORD dwAttributes = FILE_ATTRIBUTE_NORMAL, ULONGLONG cbInit = 0ll,
	BOOL bInheritHandle = FALSE, HANDLE hRootDirectory = NULL)
{
	OBJECT_ATTRIBUTES oa;
	InitializeObjectAttributes(&oa, pusFile, OBJ_CASE_INSENSITIVE, hRootDirectory, NULL);
	HANDLE hFile;
	NTSTATUS nts;
	IO_STATUS_BLOCK iost;
	LARGE_INTEGER li{ .QuadPart = (LONGLONG)cbInit };
	nts = NtCreateFile(&hFile, dwAccess, &oa, piost ? piost : &iost, &li,
		dwAttributes, dwShareMode, dwCreationDisposition, dwOptions, NULL, 0);
	if (pnts)
		*pnts = nts;
	if (NT_SUCCESS(nts))
		return hFile;
	return INVALID_HANDLE_VALUE;
}

/// <summary>
/// 打开文件。
/// 封装与CreateFileW相近的Native API调用，不支持打开控制台
/// </summary>
/// <param name="pusFile">文件路径</param>
/// <param name="dwAccess">CreateFileW --> dwAccess</param>
/// <param name="dwShareMode">CreateFileW --> dwShareMode</param>
/// <param name="dwOptions">NtCreateFile --> CreateOptions</param>
/// <param name="dwCreationDisposition">NtCreateFile --> CreateDisposition</param>
/// <param name="pnts">错误码</param>
/// <param name="piost">IO状态</param>
/// <param name="dwAttributes">若创建文件，则此参数指定文件属性</param>
/// <param name="cbInit">若创建文件，则此参数指定初始大小</param>
/// <param name="bInheritHandle">句柄是否可继承</param>
/// <returns>成功返回文件句柄，失败返回INVALID_HANDLE_VALUE</returns>
inline HANDLE NaCreateFile(PCWSTR pszFile, DWORD dwAccess, DWORD dwShareMode, DWORD dwOptions,
	DWORD dwCreationDisposition, NTSTATUS* pnts = NULL, IO_STATUS_BLOCK* piost = NULL,
	DWORD dwAttributes = FILE_ATTRIBUTE_NORMAL, ULONGLONG cbInit = 0ll,
	BOOL bInheritHandle = FALSE)
{
	UNICODE_STRING usFile;
	RtlInitUnicodeString(&usFile, pszFile);
	if (!RtlDosPathNameToNtPathName_U(pszFile, &usFile, NULL, NULL))
	{
		if (pnts)
			*pnts = STATUS_OBJECT_PATH_NOT_FOUND;
		return INVALID_HANDLE_VALUE;
	}
	const auto hFile = NaCreateFile(&usFile, dwAccess, dwShareMode, dwOptions,
		dwCreationDisposition, pnts, piost, dwAttributes, cbInit, bInheritHandle);
	RtlFreeHeap(RtlProcessHeap(), 0, usFile.Buffer);
	return hFile;
}

inline NTSTATUS NaDeviceIoControl(HANDLE hDevice, DWORD dwIoControlCode, PVOID pInBuf, DWORD cbInBuf,
	PVOID pOutBuf, DWORD cbOutBuf, PDWORD pcbReturned = NULL)
{
	NTSTATUS nts;
	IO_STATUS_BLOCK iosb;
	if ((dwIoControlCode >> 16) == FILE_DEVICE_FILE_SYSTEM)
	{
		nts = NtFsControlFile(hDevice, NULL, NULL, NULL, &iosb,
			dwIoControlCode, pInBuf, cbInBuf, pOutBuf, cbOutBuf);
	}
	else
	{
		nts = NtDeviceIoControlFile(hDevice, NULL, NULL, NULL, &iosb,
			dwIoControlCode, pInBuf, cbInBuf, pOutBuf, cbOutBuf);
	}

	if (nts == STATUS_PENDING)
	{
		NtWaitForSingleObject(hDevice, FALSE, NULL);
		nts = iosb.Status;
	}
	if (pcbReturned)
		*pcbReturned = (DWORD)iosb.Information;
	return nts;
}

using GETVERSIONOUTPARAMS = GETVERSIONINPARAMS;

#define IDE_ATAPI_IDENTIFY				0xA1
#define IDE_ATA_IDENTIFY				0xEC

#define FILE_DEVICE_SCSI				0x0000001b
#define IOCTL_SCSI_MINIPORT_IDENTIFY	((FILE_DEVICE_SCSI << 16) + 0x0501)
#define IOCTL_SCSI_MINIPORT				0x0004D008

#pragma pack(push, 1)
typedef struct _IDENTIFY_DATA
{
	USHORT GeneralConfiguration;
	USHORT NumberOfCylinders;
	USHORT Reserved1;
	USHORT NumberOfHeads;
	USHORT UnformattedBytesPerTrack;
	USHORT UnformattedBytesPerSector;
	USHORT SectorsPerTrack;
	USHORT VendorUnique1[3];
	USHORT SerialNumber[10];
	USHORT BufferType;
	USHORT BufferSectorSize;
	USHORT NumberOfEccBytes;
	USHORT FirmwareRevision[4];
	USHORT ModelNumber[20];
	UCHAR  MaximumBlockTransfer;
	UCHAR  VendorUnique2;
	USHORT DoubleWordIo;
	USHORT Capabilities;
	USHORT Reserved2;
	UCHAR  VendorUnique3;
	UCHAR  PioCycleTimingMode;
	UCHAR  VendorUnique4;
	UCHAR  DmaCycleTimingMode;
	USHORT TranslationFieldsValid : 1;
	USHORT Reserved3 : 15;
	USHORT NumberOfCurrentCylinders;
	USHORT NumberOfCurrentHeads;
	USHORT CurrentSectorsPerTrack;
	ULONG  CurrentSectorCapacity;
	USHORT CurrentMultiSectorSetting;
	ULONG  UserAddressableSectors;
	USHORT SingleWordDMASupport : 8;
	USHORT SingleWordDMAActive : 8;
	USHORT MultiWordDMASupport : 8;
	USHORT MultiWordDMAActive : 8;
	USHORT AdvancedPIOModes : 8;
	USHORT Reserved4 : 8;
	USHORT MinimumMWXferCycleTime;
	USHORT RecommendedMWXferCycleTime;
	USHORT MinimumPIOCycleTime;
	USHORT MinimumPIOCycleTimeIORDY;
	USHORT Reserved5[2];
	USHORT ReleaseTimeOverlapped;
	USHORT ReleaseTimeServiceCommand;
	USHORT MajorRevision;
	USHORT MinorRevision;
	USHORT Reserved6[50];
	USHORT SpecialFunctionsEnabled;
	USHORT Reserved7[128];
} IDENTIFY_DATA, * PIDENTIFY_DATA;
#pragma pack(pop)

typedef struct _SRB_IO_CONTROL
{
	ULONG HeaderLength;
	UCHAR Signature[8];
	ULONG Timeout;
	ULONG ControlCode;
	ULONG ReturnCode;
	ULONG Length;
} SRB_IO_CONTROL, * PSRB_IO_CONTROL;

namespace EckPriv
{
	inline constexpr UINT CalcDriveIdentifierFromIdentifyData(const IDENTIFY_DATA* pidd)
	{
		UINT s{};
		for (const auto e : pidd->ModelNumber)
			s += e;
		for (const auto e : pidd->FirmwareRevision)
			s += e;
		for (const auto e : pidd->SerialNumber)
			s += e;
		const UINT t = pidd->BufferSectorSize + pidd->SectorsPerTrack +
			pidd->NumberOfHeads + pidd->NumberOfCylinders;
		const ULONGLONG r = t * 65536ull + s;
		if (r <= 0xFFFF'FFFFull)
			return (UINT)r;
		else
			return ((t - 1) % 65535 + 1) * 65536 + s % 65535;
	}
}

/// <summary>
/// 取硬盘特征字
/// </summary>
/// <param name="idxDrive">物理硬盘索引</param>
/// <param name="pnts">NTSTATUS</param>
/// <returns>成功返回特征字，失败返回0</returns>
inline UINT GetPhysicalDriveIdentifier(int idxDrive, NTSTATUS* pnts = NULL)
{
	NTSTATUS nts;
	if (!pnts)
		pnts = &nts;
	WCHAR szDevice[48];
	swprintf(szDevice, LR"(\\.\PhysicalDrive%d)", idxDrive);
	HANDLE hDevice = NaOpenFile(szDevice, FILE_GENERIC_READ | FILE_GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE);
	if (hDevice != INVALID_HANDLE_VALUE)
	{
		GETVERSIONOUTPARAMS gvop{};
		if (NT_SUCCESS(*pnts = NaDeviceIoControl(hDevice, SMART_GET_VERSION,
			NULL, 0, &gvop, sizeof(gvop))))
		{
			SENDCMDINPARAMS scip
			{
				.cBufferSize = 512,
				.irDriveRegs =
				{
					.bSectorCountReg = 1,
					.bSectorNumberReg = 1,
					.bDriveHeadReg = BYTE(0xA0 | ((idxDrive & 1) << 4)),
					.bCommandReg = BYTE((gvop.bIDEDeviceMap >> idxDrive & 0x10) ?
							IDE_ATAPI_IDENTIFY : IDE_ATA_IDENTIFY)
				},
				.bDriveNumber = (BYTE)idxDrive
			};
			const auto pscop = (SENDCMDOUTPARAMS*)_alloca(
				sizeof(SENDCMDOUTPARAMS) - 1/*bBuffer*/ + 512);

			if (NT_SUCCESS(*pnts = NaDeviceIoControl(hDevice, SMART_RCV_DRIVE_DATA,
				&scip, sizeof(scip) - 1/*bBuffer*/, pscop, sizeof(SENDCMDOUTPARAMS))))
			{
				NtClose(hDevice);
				return EckPriv::CalcDriveIdentifierFromIdentifyData(
					(const IDENTIFY_DATA*)pscop->bBuffer);
			}
		}
		NtClose(hDevice);
	}

	swprintf(szDevice, LR"(\\.\SCSI%d:)", idxDrive);
	hDevice = NaOpenFile(szDevice, FILE_GENERIC_READ | FILE_GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE);
	if (hDevice != INVALID_HANDLE_VALUE)
	{
		constexpr size_t cbIn = sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDINPARAMS) - 1/*bBuffer*/;
		BYTE byDummy1[cbIn]{};
		const auto psrbic = (SRB_IO_CONTROL*)byDummy1;
		const auto pscip = (SENDCMDINPARAMS*)(psrbic + 1);
		ZeroMemory(psrbic, cbIn);

		psrbic->HeaderLength = sizeof(SRB_IO_CONTROL);
		memcpy(psrbic->Signature, "SCSIDISK", 8);
		psrbic->Timeout = 200;
		psrbic->Length = sizeof(SENDCMDOUTPARAMS) - 1/*bBuffer*/ + 512;
		psrbic->ControlCode = IOCTL_SCSI_MINIPORT_IDENTIFY;

		pscip->cBufferSize = 512;
		pscip->irDriveRegs.bSectorCountReg = 1;
		pscip->irDriveRegs.bSectorNumberReg = 1;
		pscip->irDriveRegs.bDriveHeadReg = BYTE(0xA0 | ((idxDrive & 1) << 4)),
			pscip->irDriveRegs.bCommandReg = IDE_ATA_IDENTIFY;
		pscip->bDriveNumber = idxDrive;

		constexpr size_t cbOut = sizeof(SRB_IO_CONTROL) +
			sizeof(SENDCMDOUTPARAMS) - 1/*bBuffer*/ + 512;
		BYTE byDummy2[cbOut]{};

		const auto pscop = ((SENDCMDOUTPARAMS*)(byDummy2 + sizeof(SRB_IO_CONTROL)));

		if (NT_SUCCESS(*pnts = NaDeviceIoControl(hDevice, IOCTL_SCSI_MINIPORT,
			psrbic, cbIn, byDummy2, cbOut)))
		{
			NtClose(hDevice);
			return EckPriv::CalcDriveIdentifierFromIdentifyData(
				(const IDENTIFY_DATA*)pscop->bBuffer);
		}
		NtClose(hDevice);
	}

	swprintf(szDevice, LR"(\\.\PhysicalDrive%d)", idxDrive);
	hDevice = NaOpenFile(szDevice, FILE_GENERIC_READ | FILE_GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE);
	if (hDevice != INVALID_HANDLE_VALUE)
	{
		STORAGE_PROPERTY_QUERY spq
		{
			.PropertyId = StorageDeviceProperty,
			.QueryType = PropertyStandardQuery
		};

		constexpr DWORD cbBuf = 4096;
		void* pBuf = VAlloc(cbBuf);
		UniquePtrVA<void> _(pBuf);
		DWORD cbRet{};
		if (NT_SUCCESS(*pnts = NaDeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
			&spq, sizeof(spq), pBuf, cbBuf, &cbRet)))
		{
			if (cbRet)
			{
				NtClose(hDevice);
				return CalcCrc32(pBuf, cbRet);
			}
		}
		NtClose(hDevice);
	}
	return 0u;
}
ECK_NAMESPACE_END