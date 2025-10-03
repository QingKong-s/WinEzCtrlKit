#pragma once
#include "DuiBase.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
class CButton :public CElem
{
private:
	ID2D1SolidColorBrush* m_pBrush{};
	IDWriteTextLayout* m_pLayout{};

	ID2D1Bitmap* m_pImg{};
	D2D1_SIZE_F m_sizeImg{};
	float m_cxText{};
	float m_cyText{};

	BITBOOL m_bHot : 1{};
	BITBOOL m_bLBtnDown : 1{};

	BITBOOL m_bAutoScale : 1{ TRUE };

	constexpr float CalcImageWidth() const
	{
		const float Padding = GetTheme()->GetMetrics(Metrics::Padding);
		if (m_pImg)
			if (m_bAutoScale)
				return (GetHeightF() - Padding * 2) * m_sizeImg.width / m_sizeImg.height;
			else
				return m_sizeImg.width;
		else
			return 0.f;
	}

	void UpdateTextLayout(PCWSTR pszText, int cchText)
	{
		const float Padding = GetTheme()->GetMetrics(Metrics::Padding);
		const float Padding2 = GetTheme()->GetMetrics(Metrics::SmallPadding);

		SafeRelease(m_pLayout);
		g_pDwFactory->CreateTextLayout(pszText, cchText,
			GetTextFormat(),
			GetWidthF() - CalcImageWidth() - Padding * 2 - Padding2,
			GetHeightF() - Padding * 2,
			&m_pLayout);
		if (m_pLayout)
		{
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

	void UpdateTextLayout()
	{
		UpdateTextLayout(GetText().Data(), GetText().Size());
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

			State eState;
			if (m_bLBtnDown)
				eState = State::Selected;
			else if (m_bHot)
				eState = State::Hot;
			else
				eState = State::Normal;

			GetTheme()->DrawBackground(Part::Button, eState, GetViewRectF(), nullptr);

			const float Padding = GetTheme()->GetMetrics(Metrics::Padding);
			const float Padding2 = GetTheme()->GetMetrics(Metrics::SmallPadding);
			if (m_pImg)
			{
				D2D1_RECT_F rcImg;
				if (m_bAutoScale)
				{
					const auto cy = GetHeightF() - Padding * 2;
					const auto cx = cy * m_sizeImg.width / m_sizeImg.height;
					rcImg.left = Padding;
					rcImg.top = Padding;
					rcImg.right = rcImg.left + cx;
					rcImg.bottom = rcImg.top + cy;
				}
				else
				{
					rcImg.left = (GetWidthF() - Padding * 2 - Padding2 -
						m_sizeImg.width - m_cxText) / 2.f;
					rcImg.top = (GetHeightF() - m_sizeImg.height) / 2.f;
					rcImg.right = rcImg.left + m_sizeImg.width;
					rcImg.bottom = rcImg.top + m_sizeImg.height;
				}
				m_pDC->DrawBitmap(m_pImg, rcImg);
			}

			if (m_pLayout)
			{
				D2D1_COLOR_F cr;
				GetTheme()->GetSysColor(SysColor::Text, cr);
				m_pBrush->SetColor(cr);
				m_pDC->DrawTextLayout({ Padding + CalcImageWidth(),Padding },
					m_pLayout, m_pBrush, DrawTextLayoutFlags);
			}

			ECK_DUI_DBG_DRAW_FRAME;
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
				const POINT pt ECK_GET_PT_LPARAM(lParam);
				if (PtInRect(GetViewRectF(), pt))
				{
					DUINMHDR nm{ EE_COMMAND };
					GenElemNotify(&nm);
				}
			}
		}
		return 0;

		case WM_SIZE:
			UpdateTextLayout();
			return 0;

		case WM_SETTEXT:
			UpdateTextLayout((PCWSTR)lParam, (int)wParam);
			InvalidateRect();
			return 0;

		case WM_SETFONT:
			UpdateTextLayout();
			if (lParam)
				InvalidateRect();
			return 0;

		case WM_CREATE:
			m_pDC->CreateSolidColorBrush({}, &m_pBrush);
			return 0;

		case WM_DESTROY:
		{
			SafeRelease(m_pBrush);
			SafeRelease(m_pLayout);
			SafeRelease(m_pImg);
			m_bHot = FALSE;
			m_bLBtnDown = FALSE;
		}
		return 0;
		}
		return CElem::OnEvent(uMsg, wParam, lParam);
	}

	void SetBitmap(ID2D1Bitmap* pImg)
	{
		ECK_DUILOCK;
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