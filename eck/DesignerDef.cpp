#include "DesignerDef.h"

ECK_NAMESPACE_BEGIN
#ifdef ECK_CTRL_DESIGN_INTERFACE
EckPropCallBackRet CALLBACK SetProp_Common(CWnd* pWnd, int idProp, EckCtrlPropValue* pProp, BOOL* pbProcessed)
{
	*pbProcessed = FALSE;
	switch (idProp)
	{
	case CPID_NAME:
	{
		*pbProcessed = TRUE;
		pWnd->m_DDBase.rsName = pProp->Vpsz;
	}
	break;
	case CPID_LEFT:
	{
		*pbProcessed = TRUE;
		HWND hWnd = pWnd->GetHWND();
		RECT rc;
		GetWindowRect(hWnd, &rc);
		ScreenToClient(GetParent(hWnd), &rc);
		rc.left = eck::DpiScale(pProp->Vi, eck::GetDpi(pWnd->GetHWND()), USER_DEFAULT_SCREEN_DPI);
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
		rc.top = eck::DpiScale(pProp->Vi, eck::GetDpi(pWnd->GetHWND()), USER_DEFAULT_SCREEN_DPI);
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
		rc.right = eck::DpiScale(pProp->Vi, eck::GetDpi(pWnd->GetHWND()), USER_DEFAULT_SCREEN_DPI);
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
		rc.bottom = eck::DpiScale(pProp->Vi, eck::GetDpi(pWnd->GetHWND()), USER_DEFAULT_SCREEN_DPI);
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
	case CPID_FRAMETYPE:
		*pbProcessed = TRUE;
		pWnd->SetFrameType(pProp->Vi);
		pWnd->FrameChanged();
		break;
	case CPID_SCROLLBAR:
		*pbProcessed = TRUE;
		pWnd->SetScrollBar(pProp->Vi);
		break;
	}
	return ESPR_NONE;
}

EckPropCallBackRet CALLBACK GetProp_Common(CWnd* pWnd, int idProp, EckCtrlPropValue* pProp, BOOL* pbProcessed)
{
	*pbProcessed = FALSE;
	switch (idProp)
	{
	case CPID_NAME:
	{
		auto& rsName = pWnd->m_DDBase.rsName;
		PWSTR p = (PWSTR)TDesignAlloc::Alloc(rsName.ByteSize());
		rsName.CopyTo(p);
		pProp->Vpsz = p;
	}
	return ESPR_NEEDFREE;
	case CPID_LEFT:
	{
		*pbProcessed = TRUE;
		HWND hWnd = pWnd->GetHWND();
		RECT rc;
		GetWindowRect(hWnd, &rc);
		ScreenToClient(GetParent(hWnd), &rc);
		pProp->Vi = eck::DpiScale(rc.left, USER_DEFAULT_SCREEN_DPI, eck::GetDpi(pWnd->GetHWND()));
	}
	break;
	case CPID_TOP:
	{
		*pbProcessed = TRUE;
		HWND hWnd = pWnd->GetHWND();
		RECT rc;
		GetWindowRect(hWnd, &rc);
		ScreenToClient(GetParent(hWnd), &rc);
		pProp->Vi = eck::DpiScale(rc.top, USER_DEFAULT_SCREEN_DPI, eck::GetDpi(pWnd->GetHWND()));
	}
	break;
	case CPID_WIDTH:
	{
		*pbProcessed = TRUE;
		HWND hWnd = pWnd->GetHWND();
		RECT rc;
		GetWindowRect(hWnd, &rc);
		ScreenToClient(GetParent(hWnd), &rc);
		pProp->Vi = eck::DpiScale(rc.right - rc.left, USER_DEFAULT_SCREEN_DPI, eck::GetDpi(pWnd->GetHWND()));
	}
	break;
	case CPID_HEIGHT:
	{
		*pbProcessed = TRUE;
		HWND hWnd = pWnd->GetHWND();
		RECT rc;
		GetWindowRect(hWnd, &rc);
		ScreenToClient(GetParent(hWnd), &rc);
		pProp->Vi = eck::DpiScale(rc.bottom - rc.top, USER_DEFAULT_SCREEN_DPI, eck::GetDpi(pWnd->GetHWND()));
	}
	break;
	case CPID_TEXT:
		*pbProcessed = TRUE;
		pProp->Vpsz = pWnd->GetText().Detach();
		return ESPR_NEEDFREE;
	case CPID_VISIBLE:
		*pbProcessed = TRUE;
		pProp->Vb = pWnd->m_DDBase.bVisible;
		break;
	case CPID_ENABLE:
		*pbProcessed = TRUE;
		pProp->Vb = pWnd->m_DDBase.bEnable;
		break;
	case CPID_FRAMETYPE:
		*pbProcessed = TRUE;
		pProp->Vi = pWnd->GetFrameType();
		break;
	case CPID_SCROLLBAR:
		*pbProcessed = TRUE;
		pProp->Vi = pWnd->GetScrollBar();
		break;
	}
	return ESPR_NONE;
}
#endif // ECK_CTRL_DESIGN_INTERFACE
ECK_NAMESPACE_END