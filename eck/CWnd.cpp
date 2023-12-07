#include "CWnd.h"
#include "Utility.h"
#include "CRefBin.h"

ECK_NAMESPACE_BEGIN
HWND CWnd::ReCreate(EckOpt(DWORD, dwNewStyle), EckOpt(DWORD, dwNewExStyle), EckOpt(RECT, rcPos))
{
	CRefBin rb{};
	SerializeData(rb);
	HWND hParent = GetParent(m_hWnd);
	int iID = GetDlgCtrlID(m_hWnd);

	if (!rcPos.has_value())
	{
		RECT rc;
		GetWindowRect(m_hWnd, &rc);
		ScreenToClient(hParent, &rc);
		rcPos = rc;
	}

	auto pData = (CREATEDATA_STD*)rb.Data();
	if (dwNewStyle.has_value())
		pData->dwStyle = dwNewStyle.value();
	if (dwNewExStyle.has_value())
		pData->dwExStyle = dwNewExStyle.value();

	DestroyWindow(m_hWnd);
	return Create(NULL, 0, 0,
		rcPos.value().left, rcPos.value().top, rcPos.value().right, rcPos.value().bottom,
		hParent, iID, rb.Data());
}

void CWnd::SetFrameType(int iFrame) const
{
	DWORD dwStyle = GetStyle() & ~WS_BORDER;
	DWORD dwExStyle = GetExStyle()
		& ~(WS_EX_WINDOWEDGE | WS_EX_STATICEDGE | WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE);

	switch (iFrame)
	{
	case 0: break;// 无边框
	case 1: dwExStyle |= WS_EX_CLIENTEDGE; break;// 凹入式
	case 2: dwExStyle |= (WS_EX_WINDOWEDGE | WS_EX_DLGMODALFRAME); break;// 凸出式
	case 3: dwExStyle |= WS_EX_STATICEDGE; break;// 浅凹入式
	case 4: dwExStyle |= (WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE); break;// 镜框式
	case 5: dwStyle |= WS_BORDER; break;// 单线边框式
	}

	SetStyle(dwStyle);
	SetExStyle(dwExStyle);
}

int CWnd::GetFrameType() const
{
	DWORD dwStyle = GetStyle();
	DWORD dwExStyle = GetExStyle();
	if (IsBitSet(dwExStyle, WS_EX_DLGMODALFRAME))
	{
		if (IsBitSet(dwExStyle, WS_EX_CLIENTEDGE))
			return 4;// 镜框式
		if (IsBitSet(dwExStyle, WS_EX_WINDOWEDGE))
			return 2;// 凸出式
	}

	if (IsBitSet(dwExStyle, WS_EX_CLIENTEDGE))
		return 1;// 凹入式
	if (IsBitSet(dwExStyle, WS_EX_STATICEDGE))
		return 3;// 浅凹入式
	if (IsBitSet(dwStyle, WS_BORDER))
		return 5;// 单线边框式

	return 0;// 无边框
}

void CWnd::SetScrollBar(int i) const
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

int CWnd::GetScrollBar() const
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