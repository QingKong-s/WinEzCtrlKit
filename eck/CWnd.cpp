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

int CWnd::GetFrameType()
{
	DWORD dwStyle = GetStyle();
	DWORD dwExStyle = GetExStyle();
	if (dwExStyle & WS_EX_DLGMODALFRAME)
	{
		if (dwExStyle & WS_EX_WINDOWEDGE)
			return 2;// 凸出式
		if (dwExStyle & WS_EX_CLIENTEDGE)
			return 4;// 镜框式
	}

	if (dwExStyle & WS_EX_CLIENTEDGE)
		return 1;// 凹入式
	if (dwExStyle & WS_EX_STATICEDGE)
		return 3;// 浅凹入式
	if (dwStyle & WS_BORDER)
		return 5;// 单线边框式

	return 0;// 无边框
}
ECK_NAMESPACE_END