#include "CWnd.h"
#include "Utility.h"

ECK_NAMESPACE_BEGIN
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
ECK_NAMESPACE_END