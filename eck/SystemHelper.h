/*
* WinEzCtrlKit Library
*
* SystemHelper.h ： 系统相关帮助函数
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "Utility.h"
#include "CException.h"
#include "CFile.h"
#include "CRefBin.h"

#include <intrin.h>

#include <wbemcli.h>
#include <comdef.h>

ECK_NAMESPACE_BEGIN
inline int CalcDbcsStringCharCount(PCSTR pszText, int cchText, UINT uCp = CP_ACP)
{
	int c = 0;
	for (auto p = pszText; p < pszText + cchText; ++c)
	{
		if (IsDBCSLeadByteEx(uCp, *p))
			p += 2;
		else
			p += 1;
	}
	return c;
}

inline COLORREF GetCursorPosColor()
{
	POINT pt;
	GetCursorPos(&pt);
	const HDC hDC = GetDC(NULL);
	const auto cr = GetPixel(hDC, pt.x, pt.y);
	ReleaseDC(NULL, hDC);
	return cr;
}

inline CRefStrW GetClipboardString(HWND hWnd = NULL)
{
	if (!OpenClipboard(hWnd))
	{
		EckDbgPrintFmt(L"剪贴板打开失败，当前所有者 = %p", GetClipboardOwner());
		return {};
	}
	if (IsClipboardFormatAvailable(CF_UNICODETEXT))
	{
		const HANDLE hData = GetClipboardData(CF_UNICODETEXT);
		if (hData)
		{
			const void* pData = GlobalLock(hData);
			if (pData)
			{
				const auto cb = GlobalSize(hData);
				const int cch = (int)(cb / sizeof(WCHAR));
				CRefStrW rs(cch);
				memcpy(rs.Data(), pData, cch * sizeof(WCHAR));
				*(rs.Data() + cch) = L'\0';
				GlobalUnlock(hData);
				CloseClipboard();
				return rs;
			}
		}
	}
	CloseClipboard();
	return {};
}

inline BOOL SetClipboardString(PCWSTR pszText, int cch = -1, HWND hWnd = NULL)
{
	if (!OpenClipboard(hWnd))
	{
		EckDbgPrintFmt(L"剪贴板打开失败，当前所有者 = %p", GetClipboardOwner());
		return FALSE;
	}
	EmptyClipboard();
	if (cch < 0)
		cch = (int)wcslen(pszText);
	const SIZE_T cb = Cch2Cb(cch);

	HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, cb);
	if (hGlobal)
	{
		void* pData = GlobalLock(hGlobal);
		if (pData)
		{
			memcpy(pData, pszText, cb);
			GlobalUnlock(hGlobal);
			SetClipboardData(CF_UNICODETEXT, hGlobal);
			CloseClipboard();
			return TRUE;
		}
	}
	CloseClipboard();
	return FALSE;
}

EckInline void DoEvents()
{
	MSG msg;
	if (GetInputState())
		while (PeekMessageW(&msg, NULL, 0u, 0u, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				PostQuitMessage((int)msg.wParam);
				return;
			}
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
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
		EckDbgPrintFmt(L"文件太大! 尺寸 = %i64", i64.QuadPart);
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
		goto Exit1;
	if (FAILED(hr = CoSetProxyBlanket(pWbemSrv, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
		RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE)))
		goto Exit2;
	return S_OK;
Exit2:
	pWbemSrv->Release();
Exit1:
	pWbemLoc->Release();
	return hr;
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
	IEnumWbemClassObject* pEnum;
	if (FAILED(hr = pWbemSrv->ExecQuery(_bstr_t(L"WQL"), _bstr_t(pszWql),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnum)))
		goto Exit1;
	IWbemClassObject* pClsObj;
	ULONG ulReturned;
	hr = pEnum->Next(WBEM_INFINITE, 1, &pClsObj, &ulReturned);
	if (ulReturned != 1)
		goto Exit1;
	hr = pClsObj->Get(pszProp, 0, &Var, NULL, NULL);
	pClsObj->Release();
Exit1:
	pEnum->Release();
	return hr;
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
	if (ci.rsVendor == L"GenuineIntel")
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
	else
		ci.rsVendor = L"未知制造商";
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
	ci.rsSerialNum.ReSize(c_cchI32ToStrBuf * 2);
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
	EckAssert(pBuf);
	if (!GetFileVersionInfoW(pszFile, 0, cbBuf, pBuf))
	{
		free(pBuf);
		return FALSE;
	}
	struct
	{
		WORD wLanguage;
		WORD wCodePage;
	}*pLangCp;
	UINT cbLangCp;
	if (!VerQueryValueW(pBuf, LR"(\VarFileInfo\Translation)", (void**)&pLangCp, &cbLangCp))
	{
		free(pBuf);
		return FALSE;
	}
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

	free(pBuf);
	return TRUE;
}

namespace EckPriv___
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
	INPUT Args[]{ EckPriv___::KeyboardEventGetArg(wVk)... };
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
[[nodiscard("Thread handle has been leaked")]]
EckInline HANDLE CrtCreateThread(_beginthreadex_proc_type pStartAddress,
	void* pParameter = NULL, UINT* pThreadId = NULL, UINT dwCreationFlags = 0)
{
	return (HANDLE)_beginthreadex(0, 0, pStartAddress, pParameter, dwCreationFlags, pThreadId);
}

inline HICON GetWindowSmallIcon(HWND hWnd)
{
	HICON hIcon = (HICON)SendMessageW(hWnd, WM_GETICON, ICON_SMALL, 0);
	if (!hIcon)
	{
		hIcon = (HICON)GetClassLongPtrW(hWnd, GCLP_HICONSM);
		if (!hIcon)
			hIcon = (HICON)SendMessageW(hWnd, WM_GETICON, ICON_SMALL2, 0);
	}
	return hIcon;
}

EckInline HICON GetWindowLargeIcon(HWND hWnd)
{
	HICON hIcon = (HICON)SendMessageW(hWnd, WM_GETICON, ICON_BIG, 0);
	if (!hIcon)
		hIcon = (HICON)GetClassLongPtrW(hWnd, GCLP_HICON);
	return hIcon;
}

inline HICON GetWindowIcon(HWND hWnd, BOOL& bNeedDestroy, BOOL bSmall = FALSE)
{
	bNeedDestroy = FALSE;
	const HICON hIcon = (bSmall ? GetWindowSmallIcon(hWnd) : GetWindowLargeIcon(hWnd));
	if (hIcon)
		return hIcon;

	DWORD dwPid;
	GetWindowThreadProcessId(hWnd, &dwPid);
	if (!dwPid)
		return NULL;
	const HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, dwPid);
	if (!hProcess)
		return NULL;

	WCHAR szPath[MAX_PATH];
	DWORD cchBuf = MAX_PATH;
	QueryFullProcessImageNameW(hProcess, 0, szPath, &cchBuf);
	CloseHandle(hProcess);

	SHFILEINFOW sfi;
	const UINT uFlags = (bSmall ? (SHGFI_ICON | SHGFI_SMALLICON) : SHGFI_ICON);
#pragma warning(suppress:6001)
	if (!SHGetFileInfoW(szPath, 0, &sfi, sizeof(sfi), uFlags))
		SHGetFileInfoW(szPath, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(sfi),
			uFlags | SHGFI_USEFILEATTRIBUTES);
	bNeedDestroy = (sfi.hIcon != NULL);
	return sfi.hIcon;
}

EckInline HFONT CreateDefFont()
{
	LOGFONTW lf;
	SystemParametersInfoW(SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, 0);
	return CreateFontIndirectW(&lf);
}
ECK_NAMESPACE_END