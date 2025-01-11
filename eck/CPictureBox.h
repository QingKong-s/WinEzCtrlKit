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
public:
	ECK_RTTI(CPictureBox);
private:
	HBITMAP m_hBitmap{};
	GpImage* m_pGpImage{};
	int m_cxImage{},
		m_cyImage{};

	int m_cxClient{},
		m_cyClient{};

	//int m_iScale{ 100 };
	//POINT m_ptViewOrg{};// 当前显示区域的左上角对应原图的位置

	COLORREF m_crBk{ CLR_DEFAULT };

	BkImgMode m_iViewMode{ BkImgMode::TopLeft };

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
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = nullptr) override
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

			if (m_bScalable)// TODO:缩放
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
					const RECT rcClient{ 0,0,m_cxClient,m_cyClient };
					GpGraphics* pGraphics;
					GdipCreateFromHDC(ps.hdc, &pGraphics);
					DrawBackgroundImage(pGraphics, m_pGpImage, rcClient, m_cxImage, m_cyImage,
						m_iViewMode, m_bFullRgnImage);
					GdipDeleteGraphics(pGraphics);
				}
				else if (m_hBitmap)
				{
					const RECT rcClient{ 0,0,m_cxClient,m_cyClient };
					const auto hCDC = CreateCompatibleDC(ps.hdc);
					SelectObject(hCDC, m_hBitmap);
					if (m_b32BppHBitmap)
						DrawBackgroundImage32(ps.hdc, hCDC, rcClient, m_cxImage, m_cyImage,
							m_iViewMode, m_bFullRgnImage);
					else
						DrawBackgroundImage32(ps.hdc, hCDC, rcClient, m_cxImage, m_cyImage,
							m_iViewMode, m_bFullRgnImage);
					DeleteDC(hCDC);
				}
			}
			EndPaint(hWnd, &ps);
		}
		return 0;
		case WM_SIZE:
			ECK_GET_SIZE_LPARAM(m_cxClient, m_cyClient, lParam);
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

	EckInline constexpr void SetViewMode(BkImgMode i){m_iViewMode = i;}
	EckInline constexpr BkImgMode GetViewMode() const{return m_iViewMode;}

	EckInline constexpr void SetFullRgnImage(BOOL b){m_bFullRgnImage = b;}
	EckInline constexpr BOOL GetFullRgnImage() const{return m_bFullRgnImage;}

	EckInline constexpr void SetBkgColor(COLORREF cr){m_crBk = cr;}
	EckInline constexpr COLORREF GetBkgColor() const{return m_crBk;}
};
ECK_RTTI_IMPL_BASE_INLINE(CPictureBox, CWnd);
ECK_NAMESPACE_END