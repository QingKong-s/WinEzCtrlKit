﻿#pragma once
#include "Utility.h"
#include "Utility2.h"
#include "CFile.h"
#include "CRefBin.h"
#include "ComPtr.h"
#include "DpiApi.h"
#include "AutoPtrDef.h"

#include <intrin.h>

#include <wbemcli.h>
#include <comdef.h>

ECK_NAMESPACE_BEGIN
inline COLORREF GetCursorPosColor()
{
	POINT pt;
	GetCursorPos(&pt);
	const HDC hDC = GetDC(nullptr);
	const auto cr = GetPixel(hDC, pt.x, pt.y);
	ReleaseDC(nullptr, hDC);
	return cr;
}

/// <summary>
/// 读入文件
/// </summary>
/// <param name="pszFile">文件路径</param>
/// <returns>返回字节集</returns>
inline CRefBin ReadInFile(PCWSTR pszFile)
{
	const HANDLE hFile = CreateFileW(pszFile, GENERIC_READ, FILE_SHARE_READ, nullptr,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
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

	CRefBin rb{};
	rb.Reserve(i64.LowPart + 4/*给调用方预留，例如添加结尾NULL等*/);
	rb.ReSize(i64.LowPart);
	DWORD dwRead;
	if (!ReadFile(hFile, rb.Data(), i64.LowPart, &dwRead, nullptr))
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
	const HANDLE hFile = CreateFileW(pszFile, GENERIC_WRITE, FILE_SHARE_READ, nullptr,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;

	DWORD dwRead;
	const BOOL b = WriteFile(hFile, pData, cb, &dwRead, nullptr);
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
/// <returns>错误代码</returns>
inline HRESULT WmiQueryClassProp(PCWSTR pszWql, PCWSTR pszProp, VARIANT& Var, IWbemServices* pWbemSrv)
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
}

/// <summary>
/// 显示/隐藏桌面
/// </summary>
/// <param name="bShow">是否显示</param>
/// <returns>桌面ListView句柄</returns>
inline HWND ShowDesktop(BOOL bShow)
{
	HWND hPm = FindWindowW(L"Progman", L"Program Manager");
	HWND hDefView = FindWindowExW(hPm, nullptr, L"SHELLDLL_DefView", nullptr);
	HWND hLV = FindWindowExW(hDefView, nullptr, L"SysListView32", nullptr);
	if (!hLV)
	{
		EckCounterNV(1000)
		{
			hPm = FindWindowExW(0, hPm, L"WorkerW", nullptr);
			hDefView = FindWindowExW(hPm, nullptr, L"SHELLDLL_DefView", nullptr);
			hLV = FindWindowExW(hDefView, nullptr, L"SysListView32", nullptr);
			if (hLV)
				goto Succeeded;
		}
		EckDbgPrintWithPos(L"寻找桌面窗口失败");
		return nullptr;
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
	const HWND hTb = FindWindowW(L"Shell_TrayWnd", nullptr);
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

template<class...T>
/// <summary>
/// 模拟按键。
/// 函数顺序按下参数中提供的键，然后倒序将它们放开
/// </summary>
/// <param name="wVk">虚拟键代码</param>
/// <returns>SendInput的返回值</returns>
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
	void* pParameter = nullptr, UINT* pThreadId = nullptr, UINT dwCreationFlags = 0)
{
	return (HANDLE)_beginthreadex(0, 0, pStartAddress, pParameter, dwCreationFlags, pThreadId);
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

[[nodiscard]] EckInline HFONT CreateDefFont(int iDpi = USER_DEFAULT_SCREEN_DPI)
{
	LOGFONTW lf;
	DaSystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, 0, iDpi);
	return CreateFontIndirectW(&lf);
}

EckInline BOOL GetDefFontInfo(LOGFONTW& lf, int iDpi = USER_DEFAULT_SCREEN_DPI)
{
	return DaSystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, 0, iDpi);
}

[[nodiscard]] EckInline IDWriteTextFormat* CreateDefTextFormat(
	int iDpi = USER_DEFAULT_SCREEN_DPI, HRESULT* phr = nullptr)
{
	LOGFONTW lf;
	if (!GetDefFontInfo(lf, iDpi))
		return nullptr;
	IDWriteTextFormat* pTextFormat;
	const auto hr = g_pDwFactory->CreateTextFormat(
		lf.lfFaceName,
		nullptr,
		(DWRITE_FONT_WEIGHT)lf.lfWeight,
		lf.lfItalic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		fabsf((float)lf.lfHeight),
		L"zh-cn",
		&pTextFormat);
	if (phr)
		*phr = hr;
	return pTextFormat;
}

[[nodiscard]] EckInline IDWriteTextFormat* CreateDefTextFormatWithSize(
	float cy, int iDpi = USER_DEFAULT_SCREEN_DPI, HRESULT* phr = nullptr)
{
	LOGFONTW lf;
	if (!GetDefFontInfo(lf, iDpi))
		return nullptr;
	IDWriteTextFormat* pTextFormat;
	const auto hr = g_pDwFactory->CreateTextFormat(
		lf.lfFaceName,
		nullptr,
		(DWRITE_FONT_WEIGHT)lf.lfWeight,
		lf.lfItalic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		cy,
		L"zh-cn",
		&pTextFormat);
	if (phr)
		*phr = hr;
	return pTextFormat;
}

[[nodiscard]] EckInline HMONITOR MonitorFromRectByWorkArea(const RECT& rc,
	HMONITOR* phMonMain = nullptr, HMONITOR* phMonNearest = nullptr)
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

	EnumDisplayMonitors(nullptr, nullptr, [](HMONITOR hMonitor, HDC, RECT*, LPARAM lParam)->BOOL
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

namespace Priv
{
	inline UINT __stdcall OpenInExplorerThread(void* pParam)
	{
		const auto pvPath = (std::vector<CRefStrW>*)pParam;
		if (FAILED(CoInitialize(nullptr)))
		{
			delete pvPath;
			return 0;
		}

		std::unordered_map<std::wstring_view, int> hmPaths{};// 文件夹路径->vPIDL索引
		std::vector<std::pair<PIDLIST_ABSOLUTE, std::vector<PIDLIST_ABSOLUTE>>> vPIDL{};// { 文件夹PIDL,{文件PIDL} }
		PIDLIST_ABSOLUTE pIDL;

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
					if (FAILED(SHParseDisplayName(pszPath, nullptr, &pIDL, 0, nullptr)))// 文件夹转PIDL
					{
						*(pszFileName - 1) = ch;
						continue;
					}
					*(pszFileName - 1) = ch;

					it = hmPaths.insert(std::make_pair(svTemp, idxCurr)).first;
					++idxCurr;

					auto& x = vPIDL.emplace_back(pIDL, std::vector<PIDLIST_ABSOLUTE>{});
					if (FAILED(SHParseDisplayName(pszPath, nullptr, &pIDL, 0, nullptr)))// 文件转PIDL
						continue;
					x.second.emplace_back(pIDL);
				}
				else
				{
					SHParseDisplayName(pszPath, nullptr, &pIDL, 0, nullptr);// 文件转PIDL
					vPIDL[it->second].second.emplace_back(pIDL);
				}
			}
		}

		for (const auto& x : vPIDL)
		{
			SHOpenFolderAndSelectItems(x.first, (UINT)x.second.size(),
				(PCUITEMID_CHILD*)x.second.data(), 0);
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
	NtClose(CrtCreateThread(Priv::OpenInExplorerThread, new std::vector{ vPath }));
}

/// <summary>
/// 在资源管理器中打开。
/// 可一次性传递多个文件，且父目录可以不同
/// </summary>
/// <param name="pvPath">路径vector指针，必须使用new分配且传递后不可再使用</param>
EckInline void OpenInExplorer(std::vector<CRefStrW>* pvPath)
{
	NtClose(CrtCreateThread(Priv::OpenInExplorerThread, pvPath));
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
	PIDLIST_ABSOLUTE pIDL;
	if (FAILED(hr = SHParseDisplayName(pszFolder, nullptr, &pIDL, 0, nullptr)))
		return hr;
	std::vector<PIDLIST_ABSOLUTE> vPIDL(vFile.size());
	for (auto& e : vFile)
	{
		if (FAILED(hr = SHParseDisplayName(e.Data(), nullptr, &vPIDL.emplace_back(), 0, nullptr)))
			goto CleanupAndRet;
	}
	hr = SHOpenFolderAndSelectItems(pIDL,
		(UINT)vPIDL.size(), (PCUITEMID_CHILD*)vPIDL.data(), 0);
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
	PIDLIST_ABSOLUTE pIdlFolder, pIdlFile;
	HRESULT hr;
	if (FAILED(hr = SHParseDisplayName(pszFolder, nullptr, &pIdlFolder, 0, nullptr)))
		goto CleanupAndRet;
	if (FAILED(hr = SHParseDisplayName(psz, nullptr, &pIdlFile, 0, nullptr)))
		goto CleanupAndRet1;
	hr = SHOpenFolderAndSelectItems(pIdlFolder,
		1, (PCUITEMID_CHILD*)&pIdlFile, 0);
CleanupAndRet1:
	CoTaskMemFree(pIdlFile);
CleanupAndRet:
	CoTaskMemFree(pIdlFolder);
	_freea(pszFolder);
	return hr;
}

inline HANDLE NaOpenProcess(DWORD dwDesiredAccess, BOOL bInheritHandle, UINT uProcessId)
{
	OBJECT_ATTRIBUTES oa;
	InitializeObjectAttributes(&oa, nullptr, bInheritHandle ? OBJ_INHERIT : 0, nullptr, nullptr);
	CLIENT_ID cid{ pToI32<HANDLE>(uProcessId) };
	HANDLE hProcess;
	if (NT_SUCCESS(NtOpenProcess(&hProcess, dwDesiredAccess, &oa, &cid)))
		return hProcess;
	return nullptr;
}

/// <summary>
/// 打开文件
/// </summary>
/// <param name="pusFile">文件的NT路径</param>
/// <param name="dwAccess">CreateFileW --> dwAccess，注意CreateFileW总是追加SYNCHRONIZE | FILE_READ_ATTRIBUTES，考虑使用FILE_GENERIC_*</param>
/// <param name="dwShareMode">CreateFileW --> dwShareMode</param>
/// <param name="dwOptions">NtOpenFile --> OpenOptions</param>
/// <param name="pnts">错误码</param>
/// <param name="piost">IO状态</param>
/// <param name="bInheritHandle">句柄是否可继承</param>
/// <param name="hRootDirectory">根目录句柄</param>
/// <returns>成功返回文件句柄，失败返回INVALID_HANDLE_VALUE</returns>
inline HANDLE NaOpenFile(UNICODE_STRING* pusFile, DWORD dwAccess, DWORD dwShareMode, DWORD dwOptions = 0u,
	NTSTATUS* pnts = nullptr, IO_STATUS_BLOCK* piost = nullptr, BOOL bInheritHandle = FALSE, HANDLE hRootDirectory = nullptr)
{
	OBJECT_ATTRIBUTES oa;
	InitializeObjectAttributes(&oa, pusFile, OBJ_CASE_INSENSITIVE, hRootDirectory, nullptr);
	HANDLE hFile;
	NTSTATUS nts;
	IO_STATUS_BLOCK iost;
	nts = NtOpenFile(&hFile, dwAccess, &oa, piost ? piost : &iost, dwShareMode, dwOptions);
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
/// <param name="dwOptions">NtOpenFile --> OpenOptions</param>
/// <param name="pnts">错误码</param>
/// <param name="piost">IO状态</param>
/// <param name="bInheritHandle">句柄是否可继承</param>
/// <returns>成功返回文件句柄，失败返回INVALID_HANDLE_VALUE</returns>
inline HANDLE NaOpenFile(PCWSTR pszFile, DWORD dwAccess, DWORD dwShareMode, DWORD dwOptions = 0u,
	NTSTATUS* pnts = nullptr, IO_STATUS_BLOCK* piost = nullptr, BOOL bInheritHandle = FALSE)
{
	UNICODE_STRING usFile;
	RtlInitUnicodeString(&usFile, pszFile);
	if (!RtlDosPathNameToNtPathName_U(pszFile, &usFile, nullptr, nullptr))
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
	DWORD dwCreationDisposition, NTSTATUS* pnts = nullptr, IO_STATUS_BLOCK* piost = nullptr,
	DWORD dwAttributes = FILE_ATTRIBUTE_NORMAL, ULONGLONG cbInit = 0ll,
	BOOL bInheritHandle = FALSE, HANDLE hRootDirectory = nullptr)
{
	OBJECT_ATTRIBUTES oa;
	InitializeObjectAttributes(&oa, pusFile, OBJ_CASE_INSENSITIVE, hRootDirectory, nullptr);
	HANDLE hFile;
	NTSTATUS nts;
	IO_STATUS_BLOCK iost;
	LARGE_INTEGER li{ .QuadPart = (LONGLONG)cbInit };
	nts = NtCreateFile(&hFile, dwAccess, &oa, piost ? piost : &iost, &li,
		dwAttributes, dwShareMode, dwCreationDisposition, dwOptions, nullptr, 0);
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
	DWORD dwCreationDisposition, NTSTATUS* pnts = nullptr, IO_STATUS_BLOCK* piost = nullptr,
	DWORD dwAttributes = FILE_ATTRIBUTE_NORMAL, ULONGLONG cbInit = 0ll,
	BOOL bInheritHandle = FALSE)
{
	UNICODE_STRING usFile;
	RtlInitUnicodeString(&usFile, pszFile);
	if (!RtlDosPathNameToNtPathName_U(pszFile, &usFile, nullptr, nullptr))
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
	PVOID pOutBuf, DWORD cbOutBuf, PDWORD pcbReturned = nullptr)
{
	NTSTATUS nts;
	IO_STATUS_BLOCK iosb;
	if ((dwIoControlCode >> 16) == FILE_DEVICE_FILE_SYSTEM)
	{
		nts = NtFsControlFile(hDevice, nullptr, nullptr, nullptr, &iosb,
			dwIoControlCode, pInBuf, cbInBuf, pOutBuf, cbOutBuf);
	}
	else
	{
		nts = NtDeviceIoControlFile(hDevice, nullptr, nullptr, nullptr, &iosb,
			dwIoControlCode, pInBuf, cbInBuf, pOutBuf, cbOutBuf);
	}

	if (nts == STATUS_PENDING)
	{
		NtWaitForSingleObject(hDevice, FALSE, nullptr);
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

namespace Priv
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

using FGetDiskID = NTSTATUS(*)(PCVOID pData, size_t cbData, BOOL bIdentifyData);

/// <summary>
/// 取硬盘特征字
/// </summary>
/// <param name="fnProcessData">处理数据回调</param>
/// <param name="idxDrive">物理硬盘索引</param>
/// <returns>NTSTATUS</returns>
template<class F>
inline NTSTATUS IntGetPhysicalDriveIdentifier(F fnProcessData, int idxDrive)
{
	NTSTATUS nts;
	WCHAR szDevice[48];
	swprintf(szDevice, LR"(\\.\PhysicalDrive%d)", idxDrive);
	HANDLE hDevice = NaOpenFile(szDevice, FILE_GENERIC_READ | FILE_GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE);
	if (hDevice != INVALID_HANDLE_VALUE)
	{
		GETVERSIONOUTPARAMS gvop{};
		if (NT_SUCCESS(nts = NaDeviceIoControl(hDevice, SMART_GET_VERSION,
			nullptr, 0, &gvop, sizeof(gvop))))
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

			BYTE byDummy2[sizeof(SENDCMDOUTPARAMS) - 1/*bBuffer*/ + 512]{};
			const auto pscop = (SENDCMDOUTPARAMS*)byDummy2;

			if (NT_SUCCESS(nts = NaDeviceIoControl(hDevice, SMART_RCV_DRIVE_DATA,
				&scip, sizeof(scip) - 1/*bBuffer*/, pscop, sizeof(SENDCMDOUTPARAMS))))
			{
				NtClose(hDevice);
				return fnProcessData(pscop->bBuffer, sizeof(IDENTIFY_DATA), TRUE);
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

		if (NT_SUCCESS(nts = NaDeviceIoControl(hDevice, IOCTL_SCSI_MINIPORT,
			psrbic, cbIn, byDummy2, cbOut)))
		{
			NtClose(hDevice);
			return fnProcessData(pscop->bBuffer, sizeof(IDENTIFY_DATA), FALSE);
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
		UniquePtr<DelVA<void>> pBuf(VAlloc(cbBuf));
		DWORD cbRet{};
		if (NT_SUCCESS(nts = NaDeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
			&spq, sizeof(spq), pBuf.get(), cbBuf, &cbRet)))
		{
			if (cbRet)
			{
				NtClose(hDevice);
				return fnProcessData(pBuf.get(), cbRet, FALSE);
			}
		}
		NtClose(hDevice);
	}
	return STATUS_ACCESS_DENIED;
}

/// <summary>
/// 取硬盘特征字
/// </summary>
/// <param name="idxDrive">物理硬盘索引</param>
/// <param name="uCrc32">返回CRC32</param>
/// <returns>NTSTATUS</returns>
EckInline NTSTATUS GetPhysicalDriveIdentifierCrc32(int idxDrive, UINT& uCrc32)
{
	return IntGetPhysicalDriveIdentifier([&uCrc32](PCVOID pData, size_t cbData, BOOL)->NTSTATUS
		{
			uCrc32 = CalcCrc32(pData, cbData);
			return STATUS_SUCCESS;
		}, idxDrive);
}

/// <summary>
/// 取硬盘特征字
/// </summary>
/// <param name="idxDrive">物理硬盘索引</param>
/// <param name="pMd5">返回MD5，至少16字节</param>
/// <returns>NTSTATUS</returns>
EckInline NTSTATUS GetPhysicalDriveIdentifierMd5(int idxDrive, _Out_writes_bytes_(16) void* pMd5)
{
	return IntGetPhysicalDriveIdentifier([pMd5](PCVOID pData, size_t cbData, BOOL)->NTSTATUS
		{
			return CalcMd5(pData, cbData, pMd5);
		}, idxDrive);
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

struct SNP_OUTPUT
{
	UINT idxOutput;
	ComPtr<IDXGIOutput1> pOutput;
	RCWH rc;
};

struct SNP_ADAPTER
{
	UINT idxAdapter;
	ComPtr<IDXGIAdapter> pAdapter;
	std::vector<SNP_OUTPUT> vOutput;
};

struct SNP_CURSOR
{
	CRefBin rbCursor;
	DXGI_OUTDUPL_POINTER_SHAPE_INFO ShapeInfo;
	DXGI_OUTDUPL_POINTER_POSITION Position;
};

using FSnapshotPreFetch = HRESULT(*)(size_t cOutput, const RCWH& rcBound);
using FSnapshotFetch = HRESULT(*)(size_t cOutput, const RCWH& rcBound, const SNP_OUTPUT& Output,
	const D3D11_MAPPED_SUBRESOURCE& MappedRes, const SNP_CURSOR& Cursor);

/// <summary>
/// 快照
/// </summary>
/// <typeparam name="F1"></typeparam>
/// <typeparam name="F2"></typeparam>
/// <param name="fnPreFetch">捕获前预处理回调</param>
/// <param name="fnFetch">获取纹理数据回调</param>
/// <param name="rc">矩形，为空则捕获所有输出</param>
/// <param name="bCursor">是否捕获鼠标指针</param>
/// <param name="msTimeout">帧超时</param>
/// <returns>HRESULT</returns>
template<class F1, class F2>
inline HRESULT IntSnapshot(F1 fnPreFetch, F2 fnFetch, const RCWH& rc, BOOL bCursor, UINT msTimeout = 500)
{
	HRESULT hr;

	std::vector<SNP_ADAPTER> vAdapter{};
	UINT idxAdapter = 0u;
	UINT idxOutput = 0u;

	IDXGIAdapter* pAdapter;
	IDXGIOutput* pOutput;

	RCWH rcBound{};
	size_t cOutput{};// Optimize for single output

	const BOOL bEmpty = IsRectEmpty(rc);
	while (SUCCEEDED(g_pDxgiFactory->EnumAdapters(idxAdapter, &pAdapter)))
	{
		auto& e = vAdapter.emplace_back(idxAdapter, pAdapter);
		while (SUCCEEDED(pAdapter->EnumOutputs(idxOutput, &pOutput)))
		{
			DXGI_OUTPUT_DESC Desc;
			if (SUCCEEDED(pOutput->GetDesc(&Desc)))
			{
				if (bEmpty)
				{
					++cOutput;
					UnionRect(rcBound, rcBound, ToRCWH(Desc.DesktopCoordinates));
					IDXGIOutput1* pOutput1;
					pOutput->QueryInterface(&pOutput1);
					e.vOutput.emplace_back(idxOutput, pOutput1, ToRCWH(Desc.DesktopCoordinates));
				}
				else if (RCWH rcTemp; IntersectRect(rcTemp, rc, ToRCWH(Desc.DesktopCoordinates)))
				{
					++cOutput;
					UnionRect(rcBound, rcBound, rcTemp);
					IDXGIOutput1* pOutput1;
					pOutput->QueryInterface(&pOutput1);
					e.vOutput.emplace_back(idxOutput, pOutput1, rcTemp);
				}
			}
			pOutput->Release();
			++idxOutput;
		}
		if (e.vOutput.empty())
			vAdapter.pop_back();
		++idxAdapter;
	}
	if (vAdapter.empty())
		return E_FAIL;

	if (FAILED(hr = fnPreFetch(cOutput, rcBound)))
		return hr;

	for (const auto& e : vAdapter)
	{
		for (const auto& f : e.vOutput)
		{
			ComPtr<ID3D11Device> pDevice;
			ComPtr<ID3D11DeviceContext> pDC;

			hr = D3D11CreateDevice(e.pAdapter.Get(), D3D_DRIVER_TYPE_UNKNOWN,
				nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT
#ifdef _DEBUG
				| D3D11_CREATE_DEVICE_DEBUG
#endif // _DEBUG
				, nullptr, 0,
				D3D11_SDK_VERSION, &pDevice, nullptr, &pDC);
			if (FAILED(hr))
				return hr;

			ComPtr<IDXGIOutputDuplication> pDup;
			if (FAILED(hr = f.pOutput->DuplicateOutput(pDevice.Get(), &pDup)))
				return hr;

			ComPtr<IDXGIResource> pResource;
			DXGI_OUTDUPL_FRAME_INFO FrameInfo;
			pDup->ReleaseFrame();
			SNP_CURSOR Cursor{};
			EckCounterNV(100)
			{
				if (FAILED(hr = pDup->AcquireNextFrame(msTimeout, &FrameInfo, &pResource)))
					return hr;
				if (bCursor && FrameInfo.PointerShapeBufferSize
					&& FrameInfo.PointerPosition.Visible)
				{
					Cursor.rbCursor.ReSize(FrameInfo.PointerShapeBufferSize);
					UINT Dummy;
					pDup->GetFramePointerShape(FrameInfo.PointerShapeBufferSize,
						Cursor.rbCursor.Data(), &Dummy, &Cursor.ShapeInfo);
					Cursor.Position = FrameInfo.PointerPosition;
				}
				if (!FrameInfo.TotalMetadataBufferSize)
				{
					pResource->Release();
					pDup->ReleaseFrame();
				}
				else
					goto ResourceOk;
			}
			return ERROR_TIMEOUT;
		ResourceOk:
			ComPtr<ID3D11Texture2D> pTexture;
			pResource.As(pTexture);

			D3D11_TEXTURE2D_DESC Desc;
			pTexture->GetDesc(&Desc);
			Desc.BindFlags = 0;
			Desc.MiscFlags = 0;
			Desc.Usage = D3D11_USAGE_STAGING;// 回读
			Desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

			ComPtr<ID3D11Texture2D> pTex;
			if (FAILED(hr = pDevice->CreateTexture2D(&Desc, nullptr, &pTex)))
				return hr;
			pDC->CopyResource(pTex.Get(), pTexture.Get());
			pDC->Flush();

			D3D11_MAPPED_SUBRESOURCE MappedRes;
			if (FAILED(hr = pDC->Map(pTex.Get(), 0, D3D11_MAP_READ, 0, &MappedRes)))
				return hr;

			hr = fnFetch(cOutput, rcBound, f, MappedRes, Cursor);

			pDC->Unmap(pTex.Get(), 0);
			pDup->ReleaseFrame();
		}
	}

	return hr;
}

/// <summary>
/// 快照
/// </summary>
/// <param name="pBmp">返回WIC位图</param>
/// <param name="rc">矩形，为空则捕获所有输出</param>
/// <param name="bCursor">是否捕获鼠标指针</param>
/// <param name="msTimeout">帧超时</param>
/// <returns>HRESULT</returns>
inline HRESULT Snapshot(IWICBitmap*& pBmp, const RCWH& rc, BOOL bCursor = FALSE, UINT msTimeout = 500)
{
	pBmp = nullptr;
	ComPtr<IWICBitmap> pBitmap;
	ComPtr<IWICBitmapLock> pLock;
	BYTE* pBits{};
	UINT cbStride{};

	const auto hr = IntSnapshot([&](size_t cOutput, const RCWH& rcBound) -> HRESULT
		{
			if (cOutput != 1 || bCursor)
			{
				auto hr = g_pWicFactory->CreateBitmap(rcBound.cx, rcBound.cy,
					GUID_WICPixelFormat32bppBGRA, WICBitmapCacheOnDemand, &pBitmap);
				if (FAILED(hr))
					return hr;
				if (FAILED(hr = pBitmap->Lock(nullptr, WICBitmapLockWrite, &pLock)))
					return hr;
				// TODO: 处理多线程套间
				pLock->GetDataPointer(&cbStride, &pBits);
				pLock->GetStride(&cbStride);
			}
			return S_OK;
		},
		[&](size_t cOutput, const RCWH& rcBound, const SNP_OUTPUT& Output,
			const D3D11_MAPPED_SUBRESOURCE& MappedRes, const SNP_CURSOR& Cursor) -> HRESULT
		{
			if (cOutput != 1 || bCursor)
			{
				RCWH rcDst = Output.rc;
				OffsetRect(rcDst, -rcBound.x, -rcBound.y);

				BYTE* pDst = pBits + rcDst.y * cbStride + rcDst.x * 4;
				for (int y = Output.rc.y; y < Output.rc.y + Output.rc.cy; ++y)
				{
					memcpy(pDst,
						(BYTE*)MappedRes.pData + Output.rc.x * 4 + y * MappedRes.RowPitch,
						Output.rc.cx * 4);
					pDst = pDst + cbStride;
				}

				if (!Cursor.rbCursor.IsEmpty())
				{
					POINT ptCursor{ Cursor.Position.Position };
					ptCursor.x += Output.rc.x;
					ptCursor.y += Output.rc.y;
					ptCursor.x -= rcBound.x;
					ptCursor.y -= rcBound.y;
#pragma warning(suppress: 26813)// 使用位与检查标志
					const int cyReal = ((Cursor.ShapeInfo.Type == DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MONOCHROME) ?
						Cursor.ShapeInfo.Height / 2 : Cursor.ShapeInfo.Height);
					const int yStart = (ptCursor.y < 0 ? -ptCursor.y : 0);
					const int yEnd = (ptCursor.y + cyReal > rcBound.cy ?
						rcBound.cy - ptCursor.y : cyReal);
					const int xStart = (ptCursor.x < 0 ? -ptCursor.x : 0);
					const int xEnd = (ptCursor.x + (int)Cursor.ShapeInfo.Width > rcBound.cx ?
						rcBound.cx - ptCursor.x : Cursor.ShapeInfo.Width);
					switch (Cursor.ShapeInfo.Type)
					{
					case DXGI_OUTDUPL_POINTER_SHAPE_TYPE_COLOR:
					{
						for (int i = yStart; i < yEnd; ++i)
						{
							for (int j = xStart; j < xEnd; ++j)
							{
								const auto dwSrc = *(DWORD*)(Cursor.rbCursor.Data() +
									i * Cursor.ShapeInfo.Pitch + j * 4);
								const auto pdwDst = (DWORD*)(pBits +
									(ptCursor.y + i) * cbStride + (ptCursor.x + j) * 4);
								*pdwDst = ArgbAlphaBlend(dwSrc, *pdwDst);
							}
						}
					}
					break;
					case DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MONOCHROME:
					{
						BYTE byMask = 0b1000'0000;
						for (int i = yStart; i < yEnd; ++i)
						{
							for (int j = xStart; j < xEnd; ++j)
							{
								const auto bySrcAnd = (*(Cursor.rbCursor.Data() +
									i * Cursor.ShapeInfo.Pitch + j / 8)) & byMask;
								const auto bySrcXor = (*(Cursor.rbCursor.Data() +
									(i + Cursor.ShapeInfo.Height / 2) * Cursor.ShapeInfo.Pitch +
									j / 8)) & byMask;
								const auto pdwDst = (DWORD*)(pBits +
									(ptCursor.y + i) * cbStride + (ptCursor.x + j) * 4);

								*pdwDst = (*pdwDst & (bySrcAnd ? 0xFFFFFFFF : 0xFF000000)) ^
									(bySrcXor ? 0x00FFFFFF : 0x00000000);

								if (byMask == 1)
									byMask = 0b1000'0000;
								else
									byMask >>= 1;
							}
						}
					}
					break;
					case DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MASKED_COLOR:
					{
						for (int i = yStart; i < yEnd; ++i)
						{
							for (int j = xStart; j < xEnd; ++j)
							{
								const auto dwSrc = *(DWORD*)(Cursor.rbCursor.Data() +
									i * Cursor.ShapeInfo.Pitch + j * 4);
								const auto pdwDst = (DWORD*)(pBits +
									(ptCursor.y + i) * cbStride + (ptCursor.x + j) * 4);
								if (dwSrc & 0xFF000000)
									*pdwDst = (*pdwDst ^ dwSrc) | 0xFF000000;
								else
									*pdwDst = dwSrc | 0xFF000000;
							}
						}
					}
					break;
					}
				}
			}
			else
				return g_pWicFactory->CreateBitmapFromMemory(
					Output.rc.cx, Output.rc.cy,
					GUID_WICPixelFormat32bppBGRA,
					MappedRes.RowPitch,
					MappedRes.RowPitch * Output.rc.cy,
					(BYTE*)MappedRes.pData + Output.rc.x * 4 + Output.rc.y * MappedRes.RowPitch,
					&pBmp);
			return S_OK;
		}, rc, bCursor, msTimeout);

	if (FAILED(hr))
		return hr;
	if (pBitmap.Get())
		pBmp = pBitmap.Detach();
	return S_OK;
}

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
ECK_NAMESPACE_END