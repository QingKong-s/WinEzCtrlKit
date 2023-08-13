#include "WndHelper.h"

ECK_NAMESPACE_BEGIN
struct DPICHANGEDCTX
{
	int iDpiOld;
	int iDpiNew;
	HDWP hDwp;
	HWND hParent;
};

BOOL CALLBACK OnDpiChanged_Parent_PreMonV2_EnumChildProc(HWND hWnd, LPARAM lParam)
{
	auto pCtx = (DPICHANGEDCTX*)lParam;
	RECT rc;
	GetWindowRect(hWnd, &rc);
	ScreenToClient(pCtx->hParent, &rc);

	rc.left = DpiScale(rc.left, pCtx->iDpiNew, pCtx->iDpiOld);
	rc.top = DpiScale(rc.top, pCtx->iDpiNew, pCtx->iDpiOld);
	rc.right = DpiScale(rc.right, pCtx->iDpiNew, pCtx->iDpiOld);
	rc.bottom = DpiScale(rc.bottom, pCtx->iDpiNew, pCtx->iDpiOld);

	pCtx->hDwp = DeferWindowPos(pCtx->hDwp, hWnd, NULL,
		rc.left,
		rc.top,
		rc.right - rc.left,
		rc.bottom - rc.top, SWP_NOZORDER | SWP_NOACTIVATE);

	OnDpiChanged_Parent_PreMonV2(hWnd, pCtx->iDpiOld, pCtx->iDpiNew);
	return TRUE;
}

BOOL OnDpiChanged_Parent_PreMonV2(HWND hWnd, int iDpiNew, int iDpiOld)
{
	DPICHANGEDCTX Ctx{ iDpiOld,iDpiNew,BeginDeferWindowPos(30),hWnd };
	if (!Ctx.hDwp)
		return FALSE;
	if (!EnumChildWindows(hWnd, OnDpiChanged_Parent_PreMonV2_EnumChildProc, (LPARAM)&Ctx))
	{
		EndDeferWindowPos(Ctx.hDwp);
		return FALSE;
	}

	return EndDeferWindowPos(Ctx.hDwp);
}

BOOL GetWindowClientRect(HWND hWnd, HWND hParent, RECT* prc)
{
	if (!GetWindowRect(hWnd, prc))
		return FALSE;
	int cx = prc->right - prc->left,
		cy = prc->bottom - prc->top;
	if (!ScreenToClient(hParent, (POINT*)prc))
		return FALSE;
	prc->right = prc->left + cx;
	prc->bottom = prc->top + cy;
	return TRUE;
}

LRESULT OnNcCalcSize(WPARAM wParam, LPARAM lParam, MARGINS* pMargins)
{
	if (wParam)
	{
		auto pnccsp = (NCCALCSIZE_PARAMS*)lParam;
		pnccsp->rgrc[0].left += pMargins->cxLeftWidth;
		pnccsp->rgrc[0].top += pMargins->cyTopHeight;
		pnccsp->rgrc[0].right -= pMargins->cxRightWidth;
		pnccsp->rgrc[0].bottom -= pMargins->cyBottomHeight;
	}
	else
	{
		auto prc = (RECT*)lParam;
		prc->left += pMargins->cxLeftWidth;
		prc->top += pMargins->cyTopHeight;
		prc->right -= pMargins->cxRightWidth;
		prc->bottom -= pMargins->cyBottomHeight;
	}
	return 0;
}
ECK_NAMESPACE_END