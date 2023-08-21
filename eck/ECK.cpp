#include "ECK.h"
#include "CLabel.h"
#include "CColorPicker.h"
#include "CBk.h"

#include <Shlwapi.h>

ECK_NAMESPACE_BEGIN
CRefStrW g_rsCurrDir;
ULONG_PTR g_uGpToken = 0u;
HINSTANCE g_hInstance = NULL;
InitStatus Init(HINSTANCE hInstance)
{
	g_hInstance = hInstance;
	GdiplusStartupInput gpsi{};
	gpsi.GdiplusVersion = 1;
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