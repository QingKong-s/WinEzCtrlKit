#include "ECK.h"
#include "CLabel.h"
#include "CColorPicker.h"
#include "CBk.h"
#include "Utility.h"

#include <Shlwapi.h>

ECK_NAMESPACE_BEGIN
CRefStrW g_rsCurrDir;
ULONG_PTR g_uGpToken = 0u;
HINSTANCE g_hInstance = NULL;


#ifdef _DEBUG
void CALLBACK GdiplusDebug(GpDebugEventLevel dwLevel, CHAR* pszMsg)
{
	PWSTR pszMsgW = StrX2W(pszMsg);
	DbgPrint(pszMsgW);
	CAllocator<WCHAR>::Free(pszMsgW);
	if (dwLevel == DebugEventLevelFatal)
		DebugBreak();
}
#endif,

InitStatus Init(HINSTANCE hInstance)
{
	g_hInstance = hInstance;
	GdiplusStartupInput gpsi{};
	gpsi.GdiplusVersion = 1;
#ifdef _DEBUG
	gpsi.DebugEventCallback = GdiplusDebug;
#endif
	if (GdiplusStartup(&g_uGpToken, &gpsi, NULL) != Ok)
		return InitStatus::GdiplusInitError;

	if (!CLabel::RegisterWndClass(hInstance))
		return InitStatus::RegWndClassError;

	if (!CColorPicker::RegisterWndClass(hInstance))
		return InitStatus::RegWndClassError;

	if (!CBk::RegisterWndClass(hInstance))
		return InitStatus::RegWndClassError;

	WCHAR szPath[MAX_PATH];
	GetModuleFileNameW(NULL, szPath, MAX_PATH - 1);
	PathRemoveFileSpecW(szPath);
	g_rsCurrDir = szPath;

	return InitStatus::Ok;
}
ECK_NAMESPACE_END