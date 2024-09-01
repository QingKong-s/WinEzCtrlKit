/*
* WinEzCtrlKit Library
*
* ShellHelper.h ： 外壳操作辅助
*
* Copyright(C) 2023-2024 QingKong
*/
#pragma once
#include "CRefStr.h"
#include "ComPtr.h"

ECK_NAMESPACE_BEGIN
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

inline HRESULT CreateShortcut(PCWSTR pszLinkFile, PCWSTR pszTarget, int iCmdShow = SW_SHOW,
	PCWSTR pszArguments = nullptr, PCWSTR pszDescription = nullptr,
	PCWSTR pszIcon = nullptr, int idxIcon = 0, PCWSTR pszWorkingDir = nullptr)
{
	HRESULT hr;
	ComPtr<IShellLinkW> pShellLink;
	if (FAILED(hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pShellLink))))
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
ECK_NAMESPACE_END