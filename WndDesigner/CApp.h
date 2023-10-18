#pragma once
#include <Windows.h>

#include "..\eck\CWnd.h"

using eck::PCVOID;
using eck::PCBYTE;

class CApp;
extern CApp* App;

class CApp
{
private:
	friend class CWndMain;

	HINSTANCE m_hInstance;

	UINT m_cfmtCtrlInfo;

	
public:
	void Init(HINSTANCE hInstance);

	EckInline HINSTANCE GetHInstance() { return m_hInstance; }
};

