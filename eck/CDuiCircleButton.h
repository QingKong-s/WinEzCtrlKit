/*
* WinEzCtrlKit Library
*
* CDuiCircleButton.h ： DUI圆形按钮
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#if ECKCXX20
#include "DuiBase.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
class CCircleButton :public CElem
{
private:
	ID2D1Bitmap* m_pImg = nullptr;				// 外部传入

	ID2D1SolidColorBrush* m_pBrush = nullptr;

	D2D1_SIZE_F m_sizeImg{};
	D2D1_INTERPOLATION_MODE m_iInterpolation = D2D1_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC;

	BITBOOL m_bHot : 1 = FALSE;
	BITBOOL m_bLBtnDown : 1 = FALSE;

	BOOL PtInBtn(POINT ptInClient)
	{
		const int iRad = std::min(GetWidth(), GetHeight()) / 2;
		const POINT ptCenter
		{
			GetRectInClient().left + iRad,
			GetRectInClient().top + iRad
		};
		return PtInCircle(ptInClient, ptCenter, iRad);
	}
public:
	LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_PAINT:
		{
			ELEMPAINTSTRU ps;
			BeginPaint(ps, wParam, lParam);

			const float fRadius = std::min(GetWidthF(), GetHeightF()) / 2.f;
			D2D1_ELLIPSE ellipse
			{
				{GetWidthF() / 2.f, GetHeightF() / 2.f},
				fRadius, fRadius
			};

			auto& ct = GetColorTheme()->Get();

			if (m_bHot)
				if (m_bLBtnDown)
					m_pBrush->SetColor(ct.crBkHotSel);
				else
					m_pBrush->SetColor(ct.crBkHot);
			else if (m_bLBtnDown)
				m_pBrush->SetColor(ct.crBkSelected);
			else
				m_pBrush->SetColor(ct.crBkNormal);
			m_pDC->FillEllipse(ellipse, m_pBrush);

			float x = (GetWidthF() - m_sizeImg.width) / 2.f;
			if (m_pImg)
			{
				float y = (GetHeightF() - m_sizeImg.height) / 2.f;
				D2D1_RECT_F rcImg;
				rcImg.left = x;
				rcImg.top = y;
				rcImg.right = rcImg.left + m_sizeImg.width;
				rcImg.bottom = rcImg.top + m_sizeImg.height;
				m_pDC->DrawBitmap(m_pImg, rcImg, 1.f, m_iInterpolation);
				x = m_pWnd->GetDs().CommMargin + m_sizeImg.width;
			}

			EndPaint(ps);
		}
		return 0;

		case WM_NCHITTEST:
			return (PtInBtn(ECK_GET_PT_LPARAM(lParam)) ? HTCLIENT : HTTRANSPARENT);

		case WM_MOUSEMOVE:
		{
			if (!m_bHot)
			{
				m_bHot = TRUE;
				InvalidateRect();
			}
		}
		return 0;

		case WM_MOUSELEAVE:
		{
			if (m_bHot)
			{
				m_bHot = FALSE;
				InvalidateRect();
			}
		}
		return 0;

		case WM_LBUTTONDBLCLK:// 连击修正
		case WM_LBUTTONDOWN:
		{
			SetFocus();
			m_bLBtnDown = TRUE;
			SetCapture();
			InvalidateRect();
		}
		return 0;

		case WM_LBUTTONUP:
		{
			if (m_bLBtnDown)
			{
				m_bLBtnDown = FALSE;
				ReleaseCapture();
				InvalidateRect();
				if (PtInBtn(ECK_GET_PT_LPARAM(lParam)))
				{
					DUINMHDR nm{ EE_COMMAND };
					GenElemNotify(&nm);
				}
			}
		}
		return 0;

		case WM_NCCREATE:
			SetColorTheme(GetWnd()->GetDefColorTheme()[CTI_CIRCLEBUTTON]);
			return 0;

		case WM_CREATE:
			m_pDC->CreateSolidColorBrush({}, &m_pBrush);
			return 0;

		case WM_DESTROY:
		{
			SafeRelease(m_pBrush);
			SafeRelease(m_pImg);
			m_bHot = FALSE;
			m_bLBtnDown = FALSE;
		}
		return 0;
		}
		return CElem::OnEvent(uMsg, wParam, lParam);
	}

	void SetImage(ID2D1Bitmap* pImg)
	{
		std::swap(m_pImg, pImg);
		if (m_pImg)
			m_pImg->AddRef();
		if (pImg)
			pImg->Release();
	}

	void SetImageSize(D2D1_SIZE_F size)
	{
		m_sizeImg = size;
	}

	void SetInterpolationMode(D2D1_INTERPOLATION_MODE iMode)
	{
		m_iInterpolation = iMode;
	}
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END
#else
#error "EckDui requires C++20"
#endif// ECKCXX20