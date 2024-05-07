/*
* WinEzCtrlKit Library
*
* CPictureBox.h ： 图片框
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "CWnd.h"
#include "GraphicsHelper.h"

ECK_NAMESPACE_BEGIN
struct NMPKBOWNERDRAW
{
	NMHDR nmhdr;
	PAINTSTRUCT* pps;
	RECT rcSrc;
};

class CPictureBox :public CWnd
{
private:
	HBITMAP m_hBitmap{};
	GpImage* m_pGpImage{};

	RECT m_rcClient{};

	int m_cxImage{},
		m_cyImage{};
	int m_iScale{ 100 };
	int m_iViewMode{ DBGIF_TOPLEFT };

	COLORREF m_crBk{ CLR_DEFAULT };

	POINT m_ptViewOrg{};// 当前显示区域的左上角对应原图的位置

	BITBOOL m_bScalable : 1 = FALSE;
	BITBOOL m_b32BppHBitmap : 1 = FALSE;
	BITBOOL m_bOwnerDraw : 1 = FALSE;
	BITBOOL m_bFullRgnImage : 1 = TRUE;
	BITBOOL m_bAutoScaleToFit : 1 = TRUE;
	BITBOOL m_bLBtnDown : 1 = TRUE;
	BITBOOL m_bCurrFit : 1 = FALSE;
public:
	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = NULL) override
	{
		return IntCreate(dwExStyle, WCN_PICTUREBOX, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, g_hInstance, this);
	}

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);
			SetDCBrushColor(ps.hdc, m_crBk == CLR_DEFAULT ? GetThreadCtx()->crDefBkg : m_crBk);
			FillRect(ps.hdc, &ps.rcPaint, GetStockBrush(DC_BRUSH));

			if (m_bScalable)
			{
				if (m_bOwnerDraw)
				{
					// TODO:所有者绘制
				}
				else
				{

				}
			}
			else
			{
				if (m_bOwnerDraw)
				{

				}
				else if (m_pGpImage)
				{
					GpGraphics* pGraphics;
					GdipCreateFromHDC(ps.hdc, &pGraphics);
					DrawBackgroundImage(pGraphics, m_pGpImage, m_rcClient, m_cxImage, m_cyImage,
						m_iViewMode, m_bFullRgnImage);
					GdipDeleteGraphics(pGraphics);
				}
				else if(m_hBitmap)
				{
					const auto hCDC = CreateCompatibleDC(ps.hdc);
					SelectObject(hCDC, m_hBitmap);
					if (m_b32BppHBitmap)
						DrawBackgroundImage32(ps.hdc, hCDC, m_rcClient, m_cxImage, m_cyImage, 
							m_iViewMode, m_bFullRgnImage);
					else
						DrawBackgroundImage(ps.hdc, hCDC, m_rcClient, m_cxImage, m_cyImage,
							m_iViewMode, m_bFullRgnImage);
					DeleteDC(hCDC);
				}
			}
			EndPaint(hWnd, &ps);
		}
		return 0;
		case WM_SIZE:
		{
			ECK_GET_SIZE_LPARAM(m_rcClient.right, m_rcClient.bottom, lParam);

		}
		return 0;
		}
		return CWnd::OnMsg(hWnd, uMsg, wParam, lParam);
	}

	BOOL SetImage(HBITMAP hbm)
	{
		BITMAP bmp;
		if (!GetObjectW(hbm, sizeof(bmp), &bmp))
			return FALSE;
		m_hBitmap = hbm;
		m_cxImage = bmp.bmWidth;
		m_cyImage = bmp.bmHeight;
		m_b32BppHBitmap = (bmp.bmBitsPixel == 32);
		return TRUE;
	}

	GpStatus SetImage(GpImage* p)
	{
		UINT cx, cy;
		GpStatus gps;
		if ((gps = GdipGetImageWidth(p, &cx)) != Gdiplus::Ok)
			return gps;
		if ((gps = GdipGetImageHeight(p, &cy)) != Gdiplus::Ok)
			return gps;
		m_pGpImage = p;
		m_cxImage = (int)cx;
		m_cyImage = (int)cy;
		return Gdiplus::Ok;
	}

	EckInline void SetViewMode(int i)
	{
		m_iViewMode = i;
	}
};
ECK_NAMESPACE_END