#include "CCommDlg.h"

ECK_NAMESPACE_BEGIN
HRESULT CALLBACK CTaskDialog::TDCallBack(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LONG_PTR lRefData)
{
	auto p = (ECKTDCTX*)lRefData;
	switch (uMsg)
	{
	case TDN_DIALOG_CONSTRUCTED:
		p->ptdc->pfCallback = p->pProc;
		p->ptdc->lpCallbackData = p->lRefData;
		p->pThis->m_hDlg = hWnd;
		break;
	case TDN_DESTROYED:
		p->pThis->m_hDlg = NULL;
		break;
	}

	if (p->pProc)
		return p->pProc(hWnd, uMsg, wParam, lParam, p->lRefData);
	else
		return S_OK;
}

int CTaskDialog::Show(TASKDIALOGCONFIG* ptdc, int* piRadioButton, BOOL* pbChecked, HRESULT* phr)
{
	ptdc->cButtons = (int)m_aBtn.size();
	if (ptdc->cButtons)
		ptdc->pButtons = m_aBtn.data();
	else
		ptdc->pButtons = NULL;

	ptdc->cRadioButtons = (int)m_aRadioBtn.size();
	if (ptdc->cRadioButtons)
		ptdc->pRadioButtons = m_aRadioBtn.data();
	else
		ptdc->pRadioButtons = NULL;

	ECKTDCTX Ctx{ ptdc->pfCallback,ptdc->lpCallbackData,this };
	ptdc->pfCallback = TDCallBack;
	ptdc->lpCallbackData = (LONG_PTR)&Ctx;

	int iButton = 0;
	int iRadioButton = 0;
	BOOL bChecked = FALSE;
	HRESULT hr = TaskDialogIndirect(
		ptdc,
		&iButton,
		piRadioButton ? piRadioButton : &iRadioButton,
		pbChecked ? pbChecked : &bChecked);
	if (phr)
		*phr = hr;
	return iButton;
}

ECK_NAMESPACE_END