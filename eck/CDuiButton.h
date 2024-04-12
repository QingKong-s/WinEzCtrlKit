#pragma once
#include "DuiBase.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
class CButton :public CElem
{
private:
	IDWriteTextFormat* m_pTf = NULL;		// 外部传入
	ID2D1Bitmap* m_pImg = NULL;				// 外部传入

	ID2D1SolidColorBrush* m_pBrush = NULL;
	ID2D1LinearGradientBrush* m_pLgBrush = NULL;
	IDWriteTextLayout* m_pLayout = NULL;

	D2D1_SIZE_F m_sizeImg{};
	float m_cxText = 0.f;
	float m_cyText = 0.f;

	BITBOOL m_bHot : 1 = FALSE;
	BITBOOL m_bLBtnDown : 1 = FALSE;

	void UpdateTextLayout()
	{
		const float cxMargin = m_pWnd->GetDs().CommMargin;
		SafeRelease(m_pLayout);
		g_pDwFactory->CreateTextLayout(m_rsText.Data(), m_rsText.Size(), m_pTf,
			GetWidthF() - m_sizeImg.width - cxMargin * 2,
			GetHeightF() - cxMargin * 2,
			&m_pLayout);
		if (m_pLayout)
		{
			m_pLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
			m_pLayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
			DWRITE_TEXT_METRICS tm;
			m_pLayout->GetMetrics(&tm);
			m_cxText = tm.width;
			m_cyText = tm.height;
		}
		else
		{
			m_cxText = 0.f;
			m_cyText = 0.f;
		}
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
			const float cxEdge = m_pWnd->GetDs().CommEdge;
			D2D1_ROUNDED_RECT rrc
			{ 
				GetViewRectF(),
				m_pWnd->GetDs().CommRrcRadius,
				m_pWnd->GetDs().CommRrcRadius
			};

			auto& ct = GetColorTheme()->Get();
			const D2D1_COLOR_F* pcrText;

			if (m_bHot)
				if (m_bLBtnDown)
					m_pBrush->SetColor(ct.crBkHotSel), pcrText = &ct.crTextSelected;
				else
					m_pBrush->SetColor(ct.crBkHot), pcrText = &ct.crTextHot;
			else if (m_bLBtnDown)
				m_pBrush->SetColor(ct.crBkSelected), pcrText = &ct.crTextSelected;
			else
				m_pBrush->SetColor(ct.crBkNormal), pcrText = &ct.crTextNormal;
			m_pDC->FillRoundedRectangle(rrc, m_pBrush);

			m_pBrush->SetColor(ct.crBorder);
			InflateRect(rrc.rect, -cxEdge / 2.f, -cxEdge / 2.f);
			m_pDC->DrawRoundedRectangle(rrc, m_pBrush, cxEdge);

			if (!m_bLBtnDown)
				m_pDC->DrawRoundedRectangle(rrc, m_pLgBrush, cxEdge);

			float x = (GetViewWidthF() - m_pWnd->GetDs().CommMargin * 2 - m_sizeImg.width - m_cxText) / 2.f;
			if (m_pImg)
			{
				float y = (GetHeightF() - m_sizeImg.height) / 2.f;
				D2D1_RECT_F rcImg;
				rcImg.left = x;
				rcImg.top = y;
				rcImg.right = rcImg.left + m_sizeImg.width;
				rcImg.bottom = rcImg.top + m_sizeImg.height;
				m_pDC->DrawBitmap(m_pImg, rcImg);
				x = m_pWnd->GetDs().CommMargin + m_sizeImg.width;
			}

			if (m_pLayout)
			{
				m_pBrush->SetColor(pcrText);
				m_pDC->DrawTextLayout({ x,m_pWnd->GetDs().CommMargin }, m_pLayout, m_pBrush);
			}

			EndPaint(ps);
		}
		return 0;

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
				if (PtInRect(GetRectInClient(), ECK_GET_PT_LPARAM(lParam)))
				{
					DUINMHDR nm{ EE_COMMAND };
					GenElemNotify(&nm);
				}
			}
		}
		return 0;

		case WM_SIZE:
		{
			ID2D1GradientStopCollection* pGsc = NULL;
			D2D1_GRADIENT_STOP gsc[2]
			{
				{ 0.f },
				{ 1.f }
			};
			gsc[1].color = GetColorTheme()->Get().crShadow;

			gsc[0].color = gsc[1].color;
			gsc[0].color.a = 0.f;

			m_pDC->CreateGradientStopCollection(gsc, 2, &pGsc);
			EckAssert(pGsc);

			SafeRelease(m_pLgBrush);
			D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES Prop
			{
				{ 0,GetViewRectF().bottom - GetWnd()->GetDs().CommRrcRadius },
				{ 0,GetViewRectF().bottom }
			};
			m_pDC->CreateLinearGradientBrush(Prop, pGsc, &m_pLgBrush);
			pGsc->Release();
		}
		return 0;

		case WM_SETTEXT:
			UpdateTextLayout();
			InvalidateRect();
			return 0;

		case WM_NCCREATE:
			SetColorTheme(GetWnd()->GetDefColorTheme()[CTI_BUTTON]);
			return 0;
		case WM_CREATE:
			m_pDC->CreateSolidColorBrush({}, &m_pBrush);
			return 0;

		case WM_DESTROY:
		{
			SafeRelease(m_pBrush);
			SafeRelease(m_pLayout);
			SafeRelease(m_pTf);
			SafeRelease(m_pImg);
			SafeRelease(m_pLgBrush);
			m_bHot = FALSE;
			m_bLBtnDown = FALSE;
		}
		return 0;
		}
		return CElem::OnEvent(uMsg, wParam, lParam);
	}

	void SetTextFormat(IDWriteTextFormat* pTf)
	{
		std::swap(m_pTf, pTf);
		if (m_pTf)
			m_pTf->AddRef();
		UpdateTextLayout();
		if (pTf)
			pTf->Release();
	}

	void SetImage(ID2D1Bitmap* pImg)
	{
		std::swap(m_pImg, pImg);
		if (m_pImg)
		{
			m_pImg->AddRef();
			m_sizeImg = m_pImg->GetSize();
		}
		else
			m_sizeImg = {};
		UpdateTextLayout();
		if (pImg)
			pImg->Release();
	}
};

ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END