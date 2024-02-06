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
			const auto& rcfInClient = GetRectInClientF();
			D2D1_ROUNDED_RECT rrc
			{ 
				rcfInClient,
				m_pWnd->GetDs().CommRrcRadius,
				m_pWnd->GetDs().CommRrcRadius
			};

			m_pBrush->SetColor(D2D1::ColorF((m_bHot || m_bLBtnDown) ? 0xfafbfa : 0xfefefd));
			m_pDC->FillRoundedRectangle(rrc, m_pBrush);
			m_pBrush->SetColor(D2D1::ColorF(0xecedeb));
			InflateRect(rrc.rect, -cxEdge / 2.f, -cxEdge / 2.f);
			m_pDC->DrawRoundedRectangle(rrc, m_pBrush, cxEdge);

			if (!m_bLBtnDown)
			{
				D2D1_RECT_F rcBottom{ rcfInClient };
				rcBottom.top = rcfInClient.bottom - rrc.radiusY;

				m_pDC->PushAxisAlignedClip(rcBottom, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
				m_pBrush->SetColor(D2D1::ColorF(0xd3d3d2));
				m_pDC->DrawRoundedRectangle(rrc, m_pBrush, cxEdge);
				m_pDC->PopAxisAlignedClip();
			}

			float x = (GetWidthF() - m_pWnd->GetDs().CommMargin * 2 - m_sizeImg.width - m_cxText) / 2.f;
			if (m_pImg)
			{
				float y = (GetHeightF() - m_sizeImg.height) / 2.f;
				D2D1_RECT_F rcImg;
				rcImg.left = rcfInClient.left + x;
				rcImg.top = rcfInClient.top + y;
				rcImg.right = rcImg.left + m_sizeImg.width;
				rcImg.bottom = rcImg.top + m_sizeImg.height;
				m_pDC->DrawBitmap(m_pImg, rcImg);
				x += (m_sizeImg.width);
			}

			if (m_pLayout)
			{
				m_pBrush->SetColor(D2D1::ColorF(m_bLBtnDown ? 0x707070 : 0));
				m_pDC->DrawTextLayout({ rcfInClient.left + x,rcfInClient.top + m_pWnd->GetDs().CommMargin }, m_pLayout, m_pBrush);
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

		case WM_LBUTTONDOWN:
		{
			m_bLBtnDown = TRUE;
			m_pWnd->SetCaptureElem(this);
			InvalidateRect();
		}
		return 0;

		case WM_LBUTTONUP:
		{
			if (m_bLBtnDown)
			{
				m_bLBtnDown = FALSE;
				m_pWnd->ReleaseCaptureElem();
				InvalidateRect();
				GenElemNotify(EE_COMMAND, 0, 0);
			}
		}
		return 0;

		case WM_SETTEXT:
			UpdateTextLayout();
			InvalidateRect();
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