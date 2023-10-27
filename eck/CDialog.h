#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
class CDialog :public CWnd
{
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
	static ATOM RegisterWndClass(HINSTANCE hInstance);

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

	virtual BOOL EndDialog(INT_PTR nResult)
	{
		BOOL b = ::EndDialog(m_hWnd, nResult);
		Manage(CWnd::ManageOp::Detach, NULL);
		return b;
	}
};
ECK_NAMESPACE_END