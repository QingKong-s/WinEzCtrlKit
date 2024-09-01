#include "pch.h"

CApp* App = NULL;

void CApp::Init(HINSTANCE hInstance)
{
	m_hInstance = hInstance;
	eck::Init(hInstance);

	m_cfmtCtrlInfo = RegisterClipboardFormatW(L"Eck.Designer.ClipBoardFormat.Ctrl");

}
