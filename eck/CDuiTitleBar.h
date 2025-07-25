﻿#pragma once
#include "DuiBase.h"

#if !ECKCXX20
#error "EckDui requires C++20"
#endif// !ECKCXX20

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
class CTitleBar :public CElem
{
protected:
	CDwmWndPartMgr m_DwmPartMgr{};
	ID2D1Bitmap1* m_pBmpDwmWndAtlas{};
	int m_cxClose{};
	int m_cxMaxMin{};
	int m_cyBtn{};
	DwmWndPart m_idxHot{ DwmWndPart::Invalid };
	DwmWndPart m_idxPressed{ DwmWndPart::Invalid };
	BITBOOL m_bMaximized{};

	DwmWPartState GetPartState(DwmWndPart idx) const
	{
		if (m_idxPressed == idx)
			return DwmWPartState::Pressed;
		else if (m_idxHot == idx)
			return DwmWPartState::Hot;
		return DwmWPartState::Normal;
	}

	LRESULT OnWndMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, SlotCtx&)
	{
		if (uMsg == WM_WINDOWPOSCHANGED)
			m_bMaximized = IsZoomed(hWnd);
		return 0;
	}
public:
	LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_PAINT:
		{
			ELEMPAINTSTRU eps;
			BeginPaint(eps, wParam, lParam);

			D2D1_RECT_F rcDst
			{
				GetWidthF() - m_cxClose,
				0,
				GetWidthF(),
				(float)m_cyBtn
			};
			D2D1_RECT_F rcTemp;

			RECT rc, rcBkg;
			DWMW_GET_PART_EXTRA Extra;
			const auto bDarkMode = ShouldAppsUseDarkMode();
			const auto iUserDpi = GetWnd()->GetUserDpiValue();
			const auto dMargin = DpiScaleF(0.5f, 96, iUserDpi);

			m_pDC->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
			if (m_DwmPartMgr.GetPartRect(rc, rcBkg, DwmWndPart::Close,
				GetPartState(DwmWndPart::Close), bDarkMode, TRUE, iUserDpi, &Extra))
			{
				rcDst.left += dMargin;
				rcDst.right -= dMargin;
				DrawImageFromGrid(m_pDC, m_pBmpDwmWndAtlas, rcDst,
					MakeD2DRcF(rcBkg), MarginsToD2dRcF(Extra.pBkg->mgSizing));

				rcTemp = MakeD2DRcF(rc);
				GetWnd()->Phy2Log(rcTemp);
				CenterRect(rcTemp, rcDst);

				m_pDC->DrawBitmap(m_pBmpDwmWndAtlas, rcTemp, 1.f,
					D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR, MakeD2DRcF(rc));

				rcDst.left -= dMargin;
			}

			rcDst.left -= m_cxMaxMin;
			rcDst.right = rcDst.left + m_cxMaxMin;

			if (m_DwmPartMgr.GetPartRect(rc, rcBkg,
				m_bMaximized ? DwmWndPart::Restore : DwmWndPart::Max,
				GetPartState(DwmWndPart::Max), bDarkMode, TRUE, iUserDpi, &Extra))
			{
				rcDst.left += dMargin;
				rcDst.right -= dMargin;
				DrawImageFromGrid(m_pDC, m_pBmpDwmWndAtlas, rcDst,
					MakeD2DRcF(rcBkg), MarginsToD2dRcF(Extra.pBkg->mgSizing));

				//ID2D1SolidColorBrush* pBrush = nullptr;
				//m_pDC->CreateSolidColorBrush(bDarkMode ? D2D1::ColorF(0.f, 0.f, 0.f, 1.f) : D2D1::ColorF(1.f, 1.f), &pBrush);
				//m_pDC->FillRectangle(rcDst, pBrush);
				//pBrush->Release();

				rcTemp = MakeD2DRcF(rc);
				GetWnd()->Phy2Log(rcTemp);
				CenterRect(rcTemp, rcDst);

				m_pDC->DrawBitmap(m_pBmpDwmWndAtlas, rcTemp, 1.f,
					D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR, MakeD2DRcF(rc));

				rcDst.left -= dMargin;
			}

			rcDst.left -= m_cxMaxMin;
			rcDst.right -= m_cxMaxMin;

			if (m_DwmPartMgr.GetPartRect(rc, rcBkg, DwmWndPart::Min,
				GetPartState(DwmWndPart::Min), bDarkMode, TRUE, iUserDpi, &Extra))
			{
				rcDst.left += dMargin;
				rcDst.right -= dMargin;
				DrawImageFromGrid(m_pDC, m_pBmpDwmWndAtlas, rcDst,
					MakeD2DRcF(rcBkg), MarginsToD2dRcF(Extra.pBkg->mgSizing));

				rcTemp = MakeD2DRcF(rc);
				GetWnd()->Phy2Log(rcTemp);
				CenterRect(rcTemp, rcDst);

				m_pDC->DrawBitmap(m_pBmpDwmWndAtlas, rcTemp, 1.0f,
					D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR, MakeD2DRcF(rc));

				rcDst.left -= dMargin;
			}
			m_pDC->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

			ECK_DUI_DBG_DRAW_FRAME;

			EndPaint(eps);
		}
		return 0;

		case WM_NCHITTEST:
		{
			POINT pt ECK_GET_PT_LPARAM(lParam);
			ClientToElem(pt);
			const DwmWndPart ePart = HitTest(pt);
			switch (ePart)
			{
			case DwmWndPart::Close:
				return HTCLOSE;
			case DwmWndPart::Max:
				return HTMAXBUTTON;
			case DwmWndPart::Min:
				return HTMINBUTTON;
			case DwmWndPart::Extra:
				if (pt.y < DaGetSystemMetrics(SM_CYFRAME, 96))
					return HTTOP;
				else
					return HTCAPTION;
			}
		}
		return HTTRANSPARENT;

		case WM_NCLBUTTONDOWN:
		{
			POINT pt ECK_GET_PT_LPARAM(lParam);
			ScreenToClient(GetWnd()->HWnd, &pt);
			ClientToElem(pt);
			GetWnd()->Phy2Log(pt);
			m_idxPressed = HitTest(pt);
			InvalidateBtnRect();
		}
		return 0;

		case WM_NCLBUTTONUP:
		{
			if (m_idxPressed != DwmWndPart::Invalid)
			{
				POINT pt ECK_GET_PT_LPARAM(lParam);
				ScreenToClient(GetWnd()->HWnd, &pt);
				ClientToElem(pt);
				GetWnd()->Phy2Log(pt);
				const auto idx = HitTest(pt);
				if (m_idxPressed == idx)
					switch (idx)
					{
					case DwmWndPart::Close:
						GetWnd()->PostMsg(WM_SYSCOMMAND, SC_CLOSE, 0);
						break;
					case DwmWndPart::Max:
						if (m_bMaximized)
							GetWnd()->PostMsg(WM_SYSCOMMAND, SC_RESTORE, 0);
						else
							GetWnd()->PostMsg(WM_SYSCOMMAND, SC_MAXIMIZE, 0);
						break;
					case DwmWndPart::Min:
						GetWnd()->PostMsg(WM_SYSCOMMAND, SC_MINIMIZE, 0);
						break;
					}

				m_idxPressed = DwmWndPart::Invalid;

				InvalidateBtnRect();
			}
		}
		return 0;

		case WM_NCMOUSEMOVE:
		{
			POINT pt ECK_GET_PT_LPARAM(lParam);
			ScreenToClient(GetWnd()->HWnd, &pt);
			ClientToElem(pt);
			GetWnd()->Phy2Log(pt);
			const auto idxOld = m_idxHot;
			m_idxHot = HitTest(pt);
			if (m_idxHot != idxOld)
				InvalidateBtnRect();
		}
		return 0;

		case WM_MOUSELEAVE:
		{
			if (m_idxHot != DwmWndPart::Invalid)
			{
				m_idxHot = DwmWndPart::Invalid;
				InvalidateBtnRect();
			}
		}
		return 0;

		case WM_CAPTURECHANGED:
		{
			if (m_idxPressed != DwmWndPart::Invalid)
			{
				m_idxPressed = DwmWndPart::Invalid;
				InvalidateBtnRect();
			}
		}
		return 0;

		case WM_CREATE:
		{
			GetWnd()->GetSignal().Connect(this, &CTitleBar::OnWndMsg, MHI_DUI_TITLEBAR);
			m_bMaximized = IsZoomed(GetWnd()->HWnd);
			UpdateTitleBarInfo(TRUE);
			UpdateMetrics();
		}
		break;

		case WM_DESTROY:
			GetWnd()->GetSignal().Disconnect(MHI_DUI_TITLEBAR);
			SafeRelease(m_pBmpDwmWndAtlas);
			break;
		}
		return CElem::OnEvent(uMsg, wParam, lParam);
	}

	void UpdateTitleBarInfo(BOOL bForceUpdate = FALSE)
	{
		if (bForceUpdate || !m_DwmPartMgr.GetHTheme())
		{
			m_DwmPartMgr.AnalyzeDefaultTheme();
			PCVOID pData;
			DWORD cbData;
			m_DwmPartMgr.GetData(&pData, &cbData);
			const auto pStream = new CStreamView(pData, cbData);
			IWICBitmapDecoder* pDecoder;
			IWICBitmap* pBitmap{};
			CreateWicBitmapDecoder(pStream, pDecoder);
			CreateWicBitmap(pBitmap, pDecoder);
			m_pDC->CreateBitmapFromWicBitmap(pBitmap, &m_pBmpDwmWndAtlas);
			pDecoder->Release();
			pBitmap->Release();
			pStream->LeaveRelease();
		}
	}

	void UpdateMetrics()
	{
		if (g_NtVer.uMajor == 6 && (g_NtVer.uMinor == 2 || g_NtVer.uMinor == 3))
		{
			m_cxClose = 46;
			m_cxMaxMin = m_cxClose * 80 / 150;
			m_cyBtn = 21;
		}
		else
		{
			m_cxClose = 46;
			m_cxMaxMin = m_cxClose;
			m_cyBtn = 31;
		}
	}

	DwmWndPart HitTest(POINT ptClient) const
	{
		const int cx = GetWidth();
		const int cy = GetHeight();
		if (ptClient.x < 0 || ptClient.x > cx || ptClient.y < 0 || ptClient.y > cy ||
			ptClient.y > m_cyBtn)
			return DwmWndPart::Invalid;
		if (ptClient.x > cx - m_cxClose)
			return DwmWndPart::Close;
		else if (ptClient.x > cx - m_cxMaxMin - m_cxClose)
			return DwmWndPart::Max;
		else if (ptClient.x > cx - m_cxMaxMin * 2 - m_cxClose)
			return DwmWndPart::Min;
		else
			return DwmWndPart::Extra;
	}

	void InvalidateBtnRect()
	{
		RECT rc{ GetWidth() - m_cxClose - m_cxMaxMin * 2 , 0, GetWidth(), m_cyBtn };
		ElemToClient(rc);
		InvalidateRect(rc);
	}
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END