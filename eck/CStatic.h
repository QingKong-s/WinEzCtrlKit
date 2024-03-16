/*
* WinEzCtrlKit Library
*
* CStatic.h ： 标准静态
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
class CStatic :public CWnd
{
public:
	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = NULL) override
	{
		return IntCreate(dwExStyle, WC_STATICW, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, NULL, NULL);
	}

	EckInline HICON GetIcon() const
	{
		return (HICON)SendMsg(STM_GETICON, 0, 0);
	}

	/// <summary>
	/// 取图像
	/// </summary>
	/// <param name="uType">图像类型，IMAGE_常量</param>
	/// <returns></returns>
	EckInline HANDLE GetImage(UINT uType = IMAGE_BITMAP) const
	{
		return (HANDLE)SendMsg(STM_GETIMAGE, uType, 0);
	}

	EckInline HICON SetIcon(HICON hIcon) const
	{
		return (HICON)SendMsg(STM_SETICON, (WPARAM)hIcon, 0);
	}

	/// <summary>
	/// 置图像
	/// </summary>
	/// <param name="h">图像句柄，含义由uType决定</param>
	/// <param name="uType">图像类型，IMAGE_常量</param>
	/// <returns>先前的图像句柄</returns>
	EckInline HANDLE SetImage(HANDLE h, UINT uType = IMAGE_BITMAP) const
	{
		return (HANDLE)SendMsg(STM_SETIMAGE, uType, (LPARAM)h);
	}
};
ECK_NAMESPACE_END