#include "CWnd.h"
#include "Utility.h"
#include "CRefBin.h"

ECK_NAMESPACE_BEGIN
HWND CWnd::Manage(ManageOp iType, HWND hWnd)
{
	switch (iType)
	{
	case ManageOp::Attach:
		return DefAttach(hWnd);

	case ManageOp::Detach:
		return DefAttach(NULL);

	case ManageOp::ChangeParent:
		assert(FALSE);
		return NULL;

	case ManageOp::Bind:
		DefAttach(hWnd);
		return (HWND)TRUE;
	}
	assert(FALSE);
	return NULL;
}

CRefBin CWnd::SerializeData(SIZE_T cbExtra, SIZE_T* pcbSize)
{
	CRefStrW rsText = GetText();
	const SIZE_T cbSize = sizeof(CREATEDATA_STD) + rsText.ByteSize();
	if (pcbSize)
		*pcbSize = cbSize;

	CRefBin rb(cbSize + cbExtra);
	CMemWriter w(rb, cbSize);

	CREATEDATA_STD* p;
	w.SkipPointer(p);
	p->iVer_Std = DATAVER_STD_1;
	p->cchText = rsText.Size();
	p->dwStyle = GetStyle();
	p->dwExStyle = GetExStyle();

	w << rsText;
	return rb;
}

void CWnd::SetFrameType(int iFrame)
{
	DWORD dwStyle = GetStyle() & ~WS_BORDER;
	DWORD dwExStyle = GetExStyle()
		& ~(WS_EX_WINDOWEDGE | WS_EX_STATICEDGE | WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE);

	switch (iFrame)
	{
	case 0: break;// �ޱ߿�
	case 1: dwExStyle |= WS_EX_CLIENTEDGE; break;// ����ʽ
	case 2: dwExStyle |= (WS_EX_WINDOWEDGE | WS_EX_DLGMODALFRAME); break;// ͹��ʽ
	case 3: dwExStyle |= WS_EX_STATICEDGE; break;// ǳ����ʽ
	case 4: dwExStyle |= (WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE); break;// ����ʽ
	case 5: dwStyle |= WS_BORDER; break;// ���߱߿�ʽ
	}

	SetStyle(dwStyle);
	SetExStyle(dwExStyle);
}

int CWnd::GetFrameType()
{
	DWORD dwStyle = GetStyle();
	DWORD dwExStyle = GetExStyle();
	if (dwExStyle & WS_EX_DLGMODALFRAME)
	{
		if (dwExStyle & WS_EX_WINDOWEDGE)
			return 2;// ͹��ʽ
		if (dwExStyle & WS_EX_CLIENTEDGE)
			return 4;// ����ʽ
	}

	if (dwExStyle & WS_EX_CLIENTEDGE)
		return 1;// ����ʽ
	if (dwExStyle & WS_EX_STATICEDGE)
		return 3;// ǳ����ʽ
	if (dwStyle & WS_BORDER)
		return 5;// ���߱߿�ʽ

	return 0;// �ޱ߿�
}

void CWnd::SetScrollBar(int i)
{
	switch (i)
	{
	case 0:
		ShowScrollBar(m_hWnd, SB_VERT, FALSE);
		ShowScrollBar(m_hWnd, SB_HORZ, FALSE);
		break;
	case 1:
		ShowScrollBar(m_hWnd, SB_VERT, FALSE);
		ShowScrollBar(m_hWnd, SB_HORZ, TRUE);
		break;
	case 2:
		ShowScrollBar(m_hWnd, SB_VERT, TRUE);
		ShowScrollBar(m_hWnd, SB_HORZ, FALSE);
		break;
	case 3:
		ShowScrollBar(m_hWnd, SB_VERT, TRUE);
		ShowScrollBar(m_hWnd, SB_HORZ, TRUE);
		break;
	}
}

int CWnd::GetScrollBar()
{
	BOOL bVSB = IsBitSet(GetWindowLongPtrW(m_hWnd, GWL_STYLE), WS_VSCROLL);
	BOOL bHSB = IsBitSet(GetWindowLongPtrW(m_hWnd, GWL_STYLE), WS_HSCROLL);
	if (bVSB)
		if (bHSB)
			return 3;
		else
			return 2;
	if (bHSB)
		return 1;

	return 0;
}
ECK_NAMESPACE_END