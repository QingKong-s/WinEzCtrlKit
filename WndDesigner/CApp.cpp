#include "pch.h"
#include "CApp.h"

CApp* App = nullptr;

void CApp::Init()
{
	m_cfCtrl = RegisterClipboardFormatW(ClipbdFmtCtrl);
}
