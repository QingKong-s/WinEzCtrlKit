﻿#pragma once
#include "CRefStr.h"
#include "ImageHelper.h"

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
ECK_NAMESPACE_END