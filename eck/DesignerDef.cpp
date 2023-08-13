#include "DesignerDef.h"

ECK_NAMESPACE_BEGIN
#ifdef ECK_CTRL_DESIGN_INTERFACE
EckPropCallBackRet CALLBACK SetProp_Common(CWnd* pWnd, int idProp, EckCtrlPropValue* pProp, BOOL* pbProcessed)
{
	*pbProcessed = FALSE;
	switch (idProp)
	{
	case CPID_LEFT:
	{
		*pbProcessed = TRUE;
		HWND hWnd = pWnd->GetHWND();
		RECT rc;
		GetWindowRect(hWnd, &rc);
		ScreenToClient(GetParent(hWnd), &rc);
		rc.left = pProp->Vi;
		SetWindowPos(hWnd, NULL, rc.left, rc.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	}
	break;
	case CPID_TOP:
	{
		*pbProcessed = TRUE;
		HWND hWnd = pWnd->GetHWND();
		RECT rc;
		GetWindowRect(hWnd, &rc);
		ScreenToClient(GetParent(hWnd), &rc);
		rc.top = pProp->Vi;
		SetWindowPos(hWnd, NULL, rc.left, rc.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	}
	break;
	case CPID_WIDTH:
	{
		*pbProcessed = TRUE;
		HWND hWnd = pWnd->GetHWND();
		RECT rc;
		GetWindowRect(hWnd, &rc);
		ScreenToClient(GetParent(hWnd), &rc);
		rc.right = pProp->Vi;
		rc.bottom -= rc.top;
		SetWindowPos(hWnd, NULL, 0, 0, rc.right, rc.bottom, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
	}
	break;
	case CPID_HEIGHT:
	{
		*pbProcessed = TRUE;
		HWND hWnd = pWnd->GetHWND();
		RECT rc;
		GetWindowRect(hWnd, &rc);
		ScreenToClient(GetParent(hWnd), &rc);
		rc.bottom = pProp->Vi;
		rc.right -= rc.left;
		SetWindowPos(hWnd, NULL, 0, 0, rc.right, rc.bottom, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
	}
	break;
	case CPID_TEXT:
		*pbProcessed = TRUE;
		pWnd->SetText(pProp->Vpsz);
		break;
	case CPID_VISIBLE:
		*pbProcessed = TRUE;
		pWnd->m_DDBase.bVisible = pProp->Vb;
		break;
	case CPID_ENABLE:
		*pbProcessed = TRUE;
		pWnd->m_DDBase.bEnable = pProp->Vb;
		break;
	}
	return ESPR_NONE;
}

EckPropCallBackRet CALLBACK GetProp_Common(CWnd* pWnd, int idProp, EckCtrlPropValue* pProp, BOOL* pbProcessed)
{
	*pbProcessed = FALSE;
	switch (idProp)
	{
	case CPID_LEFT:
	{
		*pbProcessed = TRUE;
		HWND hWnd = pWnd->GetHWND();
		RECT rc;
		GetWindowRect(hWnd, &rc);
		ScreenToClient(GetParent(hWnd), &rc);
		pProp->Vi = rc.left;
	}
	break;
	case CPID_TOP:
	{
		*pbProcessed = TRUE;
		HWND hWnd = pWnd->GetHWND();
		RECT rc;
		GetWindowRect(hWnd, &rc);
		ScreenToClient(GetParent(hWnd), &rc);
		pProp->Vi = rc.top;
	}
	break;
	case CPID_WIDTH:
	{
		*pbProcessed = TRUE;
		HWND hWnd = pWnd->GetHWND();
		RECT rc;
		GetWindowRect(hWnd, &rc);
		ScreenToClient(GetParent(hWnd), &rc);
		pProp->Vi = rc.right - rc.left;
	}
	break;
	case CPID_HEIGHT:
	{
		*pbProcessed = TRUE;
		HWND hWnd = pWnd->GetHWND();
		RECT rc;
		GetWindowRect(hWnd, &rc);
		ScreenToClient(GetParent(hWnd), &rc);
		pProp->Vi = rc.bottom - rc.top;
	}
	break;
	case CPID_TEXT:
		*pbProcessed = TRUE;
		pProp->Vpsz = pWnd->GetText().Reset();
		return ESPR_NEEDFREE;
	case CPID_VISIBLE:
		*pbProcessed = TRUE;
		pProp->Vb = pWnd->m_DDBase.bVisible;
		break;
	case CPID_ENABLE:
		*pbProcessed = TRUE;
		pProp->Vb = pWnd->m_DDBase.bEnable;
		break;
	}
	return ESPR_NONE;
}
#endif // ECK_CTRL_DESIGN_INTERFACE
ECK_NAMESPACE_END