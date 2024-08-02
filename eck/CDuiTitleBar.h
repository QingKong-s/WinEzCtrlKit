#pragma once
#include "DuiBase.h"

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

	HWND m_hRefWnd{};

	DwmWPartState GetPartState(DwmWndPart idx) const
	{
		if(m_idxPressed == idx)
			return DwmWPartState::Pressed;
		else if(m_idxHot == idx)
			return DwmWPartState::Hot;
		return DwmWPartState::Normal;
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
			D2D1_RECT_F rcDst2;

			RECT rc, rcBkg;
			D2D1_RECT_F rcMargins;
			DWMW_GET_PART_EXTRA Extra;

			if (m_DwmPartMgr.GetPartRect(rc, rcBkg, DwmWndPart::Close, GetPartState(DwmWndPart::Close),
				ShouldAppsUseDarkMode(), TRUE, GetWnd()->GetDpiValue(), &Extra))
			{
				rcMargins = MarginsToD2dRcF(Extra.pBkg->mgSizing);
				DrawImageFromGrid(m_pDC, m_pBmpDwmWndAtlas, rcDst, MakeD2DRcF(rcBkg), rcMargins);

				rcDst2 = MakeD2DRcF(rc);
				CenterRect(rcDst2, rcDst);

				m_pDC->DrawBitmap(m_pBmpDwmWndAtlas, rcDst2, 1.0f,
					D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR, MakeD2DRcF(rc));
			}

			rcDst.left -= m_cxMaxMin;
			rcDst.right = rcDst.left + m_cxMaxMin;

			if (m_DwmPartMgr.GetPartRect(rc, rcBkg, DwmWndPart::Max, GetPartState(DwmWndPart::Max),
				ShouldAppsUseDarkMode(), TRUE, GetWnd()->GetDpiValue(), &Extra))
			{
				rcMargins = MarginsToD2dRcF(Extra.pBkg->mgSizing);
				DrawImageFromGrid(m_pDC, m_pBmpDwmWndAtlas, rcDst, MakeD2DRcF(rcBkg), rcMargins);

				rcDst2 = MakeD2DRcF(rc);
				CenterRect(rcDst2, rcDst);

				m_pDC->DrawBitmap(m_pBmpDwmWndAtlas, rcDst2, 1.0f,
					D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR, MakeD2DRcF(rc));
			}

			rcDst.left -= m_cxMaxMin;
			rcDst.right -= m_cxMaxMin;

			if (m_DwmPartMgr.GetPartRect(rc, rcBkg, DwmWndPart::Min, GetPartState(DwmWndPart::Min),
				ShouldAppsUseDarkMode(), TRUE, GetWnd()->GetDpiValue(), &Extra))
			{
				rcMargins = MarginsToD2dRcF(Extra.pBkg->mgSizing);
				DrawImageFromGrid(m_pDC, m_pBmpDwmWndAtlas, rcDst, MakeD2DRcF(rcBkg), rcMargins);

				rcDst2 = MakeD2DRcF(rc);
				CenterRect(rcDst2, rcDst);

				m_pDC->DrawBitmap(m_pBmpDwmWndAtlas, rcDst2, 1.0f,
					D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR, MakeD2DRcF(rc));
			}

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
				return HTCAPTION;
			default:
				return HTTRANSPARENT;
			}
		}
		return HTTRANSPARENT;

		case WM_NCLBUTTONDOWN:
		{
			POINT pt ECK_GET_PT_LPARAM(lParam);
			ScreenToClient(GetWnd()->HWnd, &pt);
			ClientToElem(pt);
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
				const auto idx = HitTest(pt);
				if(m_idxPressed == idx)
					switch (idx)
					{
					case DwmWndPart::Close:
						PostMessageW(m_hRefWnd, WM_SYSCOMMAND, SC_CLOSE, 0);
						break;
					case DwmWndPart::Max:
						PostMessageW(m_hRefWnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
						break;
					case DwmWndPart::Min:
						PostMessageW(m_hRefWnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
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
			m_hRefWnd = GetWnd()->GetHWND();
			UpdateTitleBarInfo(TRUE);
			UpdateMetrics();
		}
		break;

		case WM_DESTROY:
			SafeRelease(m_pBmpDwmWndAtlas);
			break;
		}
		return CElem::OnEvent(uMsg, wParam, lParam);
	}

	void UpdateTitleBarInfo(BOOL bForceUpdate = FALSE)
	{
		if (bForceUpdate|| !m_DwmPartMgr.GetHTheme())
		{
			m_DwmPartMgr.AnalyzeDefaultTheme();
			PCVOID pData;
			DWORD cbData;
			m_DwmPartMgr.GetData(&pData, &cbData);
			const auto pStream = new CStreamView(pData, cbData);
			IWICBitmapDecoder* pDecoder;
			IWICBitmap* pBitmap;
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
			m_cxClose = DpiScaleF(46, GetWnd()->GetDpiValue());
			m_cxMaxMin = m_cxClose * 80 / 150;
			m_cyBtn = DpiScaleF(21, GetWnd()->GetDpiValue());
		}
		else
		{
			m_cxClose = DpiScaleF(46, GetWnd()->GetDpiValue());
			m_cxMaxMin = m_cxClose;
			m_cyBtn = DpiScaleF(41, GetWnd()->GetDpiValue());
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
		InvalidateRect(&rc);
	}

	EckInline constexpr void SetRefWnd(HWND hWnd) { m_hRefWnd = hWnd; }

	EckInline constexpr HWND GetRefWnd() const { return m_hRefWnd; }
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END