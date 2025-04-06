#pragma once
#include "CRefStr.h"
#include "ImageHelper.h"

#include <taskschd.h>

ECK_NAMESPACE_BEGIN
inline CRefStrW GetClipboardString(HWND hWnd = nullptr)
{
	if (!OpenClipboard(hWnd))
	{
		EckDbgPrintFmt(L"剪贴板打开失败，当前所有者 = %p", GetClipboardOwner());
		return {};
	}
	if (IsClipboardFormatAvailable(CF_UNICODETEXT))
	{
		const HGLOBAL hData = GetClipboardData(CF_UNICODETEXT);
		if (hData)
		{
			int cch = (int)(GlobalSize(hData) / sizeof(WCHAR));
			if (cch)
			{
				const void* pData = GlobalLock(hData);
				if (pData)
				{
					if (*((PCWSTR)pData + cch - 1) == L'\0')
						--cch;
					CRefStrW rs((PCWSTR)pData, cch);
					GlobalUnlock(hData);
					CloseClipboard();
					return rs;
				}
			}
		}
	}
	CloseClipboard();
	return {};
}

inline BOOL SetClipboardString(PCWSTR pszText, int cch = -1, HWND hWnd = nullptr)
{
	if (!OpenClipboard(hWnd))
	{
		EckDbgPrintFmt(L"剪贴板打开失败，当前所有者 = %p", GetClipboardOwner());
		return FALSE;
	}
	EmptyClipboard();
	if (cch < 0)
		cch = (int)wcslen(pszText);
	const SIZE_T cb = Cch2CbW(cch);

	const HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, cb);
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

inline HBITMAP GetClipboardBitmap(HWND hWnd = nullptr)
{
	if (!OpenClipboard(hWnd))
	{
		EckDbgPrintFmt(L"剪贴板打开失败，当前所有者 = %p", GetClipboardOwner());
		return nullptr;
	}
	if (IsClipboardFormatAvailable(CF_DIB))
	{
		const HANDLE hData = GetClipboardData(CF_DIB);
		if (hData)
		{
			const auto cb = GlobalSize(hData);
			if (cb >= sizeof(BITMAPINFOHEADER))
			{
				const void* pData = GlobalLock(hData);
				if (pData)
				{
					CDib dib{};
					dib.Create(pData);
					GlobalUnlock(hData);
					CloseClipboard();
					return dib.Detach();
				}
			}
		}
	}
	CloseClipboard();
	return {};
}

inline CRefBin GetClipboardDib(HWND hWnd = nullptr)
{
	if (!OpenClipboard(hWnd))
	{
		EckDbgPrintFmt(L"剪贴板打开失败，当前所有者 = %p", GetClipboardOwner());
		return {};
	}
	if (IsClipboardFormatAvailable(CF_DIB))
	{
		const HGLOBAL hData = GetClipboardData(CF_DIB);
		if (hData)
		{
			CRefBin rb(GlobalLock(hData), GlobalSize(hData));
			GlobalUnlock(hData);
			CloseClipboard();
			return rb;
		}
	}
	CloseClipboard();
	return {};
}

inline HRESULT CreateShortcut(PCWSTR pszLinkFile, PCWSTR pszTarget, int iCmdShow = SW_SHOW,
	PCWSTR pszArguments = nullptr, PCWSTR pszDescription = nullptr,
	PCWSTR pszIcon = nullptr, int idxIcon = 0, PCWSTR pszWorkingDir = nullptr)
{
	HRESULT hr;
	ComPtr<IShellLinkW> pShellLink;
	if (FAILED(hr = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pShellLink))))
		return hr;
	if (FAILED(hr = pShellLink->SetPath(pszTarget)))
		return hr;
	if (FAILED(hr = pShellLink->SetShowCmd(iCmdShow)))
		return hr;
	if (pszArguments)
		if (FAILED(hr = pShellLink->SetArguments(pszArguments)))
			return hr;
	if (pszDescription)
		if (FAILED(hr = pShellLink->SetDescription(pszDescription)))
			return hr;
	if (pszIcon)
		if (FAILED(hr = pShellLink->SetIconLocation(pszIcon, idxIcon)))
			return hr;
	if (pszWorkingDir)
		if (FAILED(hr = pShellLink->SetWorkingDirectory(pszWorkingDir)))
			return hr;
	ComPtr<IPersistFile> pPersistFile;
	if (FAILED(hr = pShellLink.As(pPersistFile)))
		return hr;
	return pPersistFile->Save(pszLinkFile, TRUE);
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

enum class AutoRunType : UINT
{
	None,
	LocalMachine,
	CurrentUser,
	TaskScheduler,

	TypeMask = 0xFFFF,
	RunAdmin = 1u << 31,
};
ECK_ENUM_BIT_FLAGS(AutoRunType);

inline HRESULT SetAutoRun(PCWSTR pszId, BOOL bEnable,
	AutoRunType eType = AutoRunType::LocalMachine,
	PCWCH pszFile = nullptr, int cchFile = -1)
{
	if (!pszFile)
	{
		const auto usSelf = NtCurrentPeb()->ProcessParameters->ImagePathName;
		pszFile = usSelf.Buffer;
		cchFile = (int)usSelf.Length / sizeof(WCHAR);
	}
	else if (cchFile < 0)
		cchFile = (int)wcslen(pszFile);

	switch (eType & AutoRunType::TypeMask)
	{
	case AutoRunType::LocalMachine:
	case AutoRunType::CurrentUser:
	{
		LSTATUS ls;
		const auto hRootKey = eType == AutoRunType::LocalMachine ?
			HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
		HKEY hKey;
		ls = RegOpenKeyExW(hRootKey, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
			0, KEY_SET_VALUE, &hKey);
		if (ls != ERROR_SUCCESS)
			return HRESULT_FROM_WIN32(ls);
		if (bEnable)
		{
			if (cchFile < 0)
				cchFile = (int)wcslen(pszFile);
			ls = RegSetValueExW(hKey, pszId, 0, REG_SZ,
				(BYTE*)pszFile, cchFile * sizeof(WCHAR));
		}
		else
			ls = RegDeleteValueW(hKey, pszId);
		RegCloseKey(hKey);
		return HRESULT_FROM_WIN32(ls);
	}
	break;
	case AutoRunType::TaskScheduler:
	{
		HRESULT hr;
		ComPtr<ITaskService> pService;
		if (FAILED(hr = CoCreateInstance(CLSID_TaskScheduler, nullptr,
			CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pService))))
			return hr;
		if (FAILED(hr = pService->Connect({}, {}, {}, {})))
			return hr;
		ComPtr<ITaskFolder> pFolder;
		if (FAILED(hr = pService->GetFolder(_bstr_t(L"\\"), &pFolder)))
			return hr;
		_bstr_t bsId{ pszId };
		hr = pFolder->DeleteTask(bsId, 0);
		if (!bEnable)
			return hr;
		ComPtr<ITaskDefinition> pDef;
		if (FAILED(hr = pService->NewTask(0, &pDef)))
			return hr;
		if ((eType & AutoRunType::RunAdmin) != AutoRunType::None)
		{
			ComPtr<IPrincipal> pPrincipal;
			if (FAILED(hr = pDef->get_Principal(&pPrincipal)))
				return hr;
			if (FAILED(hr = pPrincipal->put_RunLevel(TASK_RUNLEVEL_HIGHEST)))
				return hr;
		}
		ComPtr<ITaskSettings> pSettings;
		if (FAILED(hr = pDef->get_Settings(&pSettings)))
			return hr;
		if (FAILED(hr = pSettings->put_DisallowStartIfOnBatteries(VARIANT_FALSE)))
			return hr;
		if (FAILED(hr = pSettings->put_StopIfGoingOnBatteries(VARIANT_FALSE)))
			return hr;
		if (FAILED(hr = pSettings->put_RunOnlyIfIdle(VARIANT_FALSE)))
			return hr;
		ComPtr<ITriggerCollection> pTriggerCollection;
		if (FAILED(hr = pDef->get_Triggers(&pTriggerCollection)))
			return hr;
		ComPtr<ITrigger> pTrigger;
		if (FAILED(hr = pTriggerCollection->Create(TASK_TRIGGER_LOGON, &pTrigger)))
			return hr;
		ComPtr<IActionCollection> pActionCollection;
		if (FAILED(hr = pDef->get_Actions(&pActionCollection)))
			return hr;
		ComPtr<IAction> pAction;
		if (FAILED(hr = pActionCollection->Create(TASK_ACTION_EXEC, &pAction)))
			return hr;
		ComPtr<IExecAction> pExecAction;
		if (FAILED(hr = pAction.As(pExecAction)))
			return hr;
		const auto bsFile = SysAllocStringLen(pszFile, cchFile);
		hr = pExecAction->put_Path(bsFile);
		SysFreeString(bsFile);
		if (FAILED(hr))
			return hr;
		ComPtr<IRegisteredTask> pRegisteredTask;
		hr = pFolder->RegisterTaskDefinition(bsId, pDef.Get(),
			TASK_CREATE_OR_UPDATE, {}, {}, TASK_LOGON_INTERACTIVE_TOKEN,
			_variant_t(L""), &pRegisteredTask);
		return hr;
	}
	break;
	}
	return E_INVALIDARG;
}
ECK_NAMESPACE_END