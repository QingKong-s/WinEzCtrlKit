/*
* WinEzCtrlKit Library
*
* CDialog.h ： 自定义对话框
* 注册的对话框类分配(DLGWINDOWEXTRA + sizeof(void*) * 2)个额外字节，
* 若要存储额外的窗口相关信息，使用CDialog::OcbPtr1和CDialog::OcbPtr2
* 来偏移GetWindowLongPtrW的值
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
class CDialog :public CWnd
{
public:
	constexpr static LONG_PTR OcbPtr1 = DLGWINDOWEXTRA;
	constexpr static LONG_PTR OcbPtr2 = DLGWINDOWEXTRA + sizeof(void*);
protected:
	HWND m_hTop = NULL;
protected:
	EckInline HWND PreModal(HWND hParent)
	{
		return GetSafeOwner(hParent, &m_hTop);
	}

	EckInline void PostModal()
	{
		if (!m_hTop)
			return;
		EnableWindow(m_hTop, TRUE);
		m_hTop = NULL;
	}
public:
	virtual HWND CreateDlg(HWND hParent, void* pData = NULL)
	{
		EckDbgBreak();
		return NULL;
	}

	virtual INT_PTR DlgBox(HWND hParent, void* pData = NULL)
	{
		EckDbgBreak();
		return 0;
	}

	virtual BOOL EndDlg(INT_PTR nResult)
	{
		return EndDialog(m_hWnd, nResult);
	}
};
ECK_NAMESPACE_END