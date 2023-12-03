#pragma once
#include "CWnd.h"
#include "Utility.h"
#include "GraphicsHelper.h"

#include <dxgi1_2.h>

ECK_NAMESPACE_BEGIN
class CDrawPanel :public CWnd
{
private:
	CEzCDC m_DC{};
	GpGraphics* m_pGraphics = NULL;
	int m_cxClient = 0, m_cyClient = 0;
	HBRUSH m_hbrBK = NULL;

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void OnSize(HWND hWnd, UINT uState, int cx, int cy);
public:
	static ATOM RegisterWndClass();

	EckInline HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData = NULL) override
	{
		m_hWnd = CreateWindowExW(dwExStyle, WCN_DRAWPANEL, pszText, dwStyle,
			x, y, cx, cy, hParent, i32ToP<HMENU>(nID), g_hInstance, this);
		return m_hWnd;
	}

	EckInline HDC GetHDC() const { return m_DC.GetDC(); }

	EckInline GpGraphics* GetGraphics() const { return m_pGraphics; }

	EckInline void SetBkBrush(HBRUSH hbr)
	{
		if (m_hbrBK)
			DeleteObject(m_hbrBK);
		m_hbrBK = hbr;
	}
};

class CDrawPanelD2D :public CWnd
{
private:
	ID2D1DeviceContext* m_pDC = NULL;
	ID2D1Bitmap1* m_pBitmap = NULL;
	IDXGISwapChain1* m_pSwapChain = NULL;

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
public:
	static ATOM RegisterWndClass();

	EckInline HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData = NULL) override
	{
		m_hWnd = CreateWindowExW(dwExStyle, WCN_DRAWPANELD2D, pszText, dwStyle,
			x, y, cx, cy, hParent, i32ToP<HMENU>(nID), g_hInstance, this);
		return m_hWnd;
	}

	EckInline ID2D1DeviceContext* GetDC() const { return m_pDC; }

	EckInline IDXGISwapChain1* GetSwapChain() const { return m_pSwapChain; }
};
ECK_NAMESPACE_END