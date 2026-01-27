#include "pch.h"
#include "CApp.h"

CApp* App = nullptr;

void CApp::Initialize()
{
	m_cfCtrl = RegisterClipboardFormatW(ClipboardFormat);
}
