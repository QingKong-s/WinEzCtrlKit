#pragma once
#include "DuiBase.h"
#include "EasingCurve.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
class CTrackBar :public CElem
{
private:
	ID2D1SolidColorBrush* m_pBrush = NULL;

	BITBOOL m_bVertical : 1 = FALSE;
	BITBOOL m_bLBtnDown : 1 = FALSE;
	BITBOOL m_bHover : 1 = FALSE;

	float m_fPos = 0.0f;
	float m_fMin = 0.0f;
	float m_fMax = 100.0f;

	float m_cxyTrack = 0.0f;

	CEasingCurve<Easing::FOutSine> m_Ec{};

	float GetCxyTrack()
	{
		if (m_Ec.IsActive())
			return  m_cxyTrack / 2.f + m_cxyTrack / 2.f * m_Ec.GetCurrValue();
		else
			return (m_bHover ? m_cxyTrack : m_cxyTrack / 2.f);
	}

	float GetTrackRect(D2D1_RECT_F& rc)
	{
		rc = GetViewRectF();
		const float cxyTrack = GetCxyTrack();
		const float fRadius = m_cxyTrack * 3.f / 4.f;
		if (m_bVertical)
		{
			rc.left += (rc.right - rc.left - cxyTrack) / 2.f;
			rc.right = rc.left + cxyTrack;
			rc.top += fRadius;
			rc.bottom -= fRadius;
		}
		else
		{
			rc.top += (rc.bottom - rc.top - cxyTrack) / 2.f;
			rc.bottom = rc.top + cxyTrack;
			rc.left += fRadius;
			rc.right -= fRadius;
		}
		return cxyTrack;
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

			D2D1_ROUNDED_RECT rrc;
			const float cxyTrack = GetTrackRect(rrc.rect);
			rrc.radiusX = rrc.radiusY = cxyTrack / 2.f;

			m_pBrush->SetColor(D2D1::ColorF(0xc6bfbe));
			m_pDC->FillRoundedRectangle(rrc, m_pBrush);

			const float fScale = (m_fPos - m_fMin) / (m_fMax - m_fMin);
			if (m_bVertical)
				rrc.rect.bottom = rrc.rect.top + (rrc.rect.bottom - rrc.rect.top) * fScale;
			else
				rrc.rect.right = rrc.rect.left + (rrc.rect.right - rrc.rect.left) * fScale;
			m_pBrush->SetColor(D2D1::ColorF(0x227988));
			m_pDC->FillRoundedRectangle(rrc, m_pBrush);

			if (m_bHover || m_Ec.IsActive())
			{
				D2D1_ELLIPSE ellipse;
				const float cxy = cxyTrack * 3.f / 4.f * m_Ec.GetCurrValue();
				ellipse.radiusX = ellipse.radiusY = cxy;// cxyTrack * 3.f / 4.f;
				if (m_bVertical)
				{
					ellipse.point.x = (rrc.rect.left + rrc.rect.right) / 2.f;
					ellipse.point.y = rrc.rect.bottom;
				}
				else
				{
					ellipse.point.x = rrc.rect.right;
					ellipse.point.y = (rrc.rect.top + rrc.rect.bottom) / 2.f;
				}
				m_pBrush->SetColor(D2D1::ColorF(0xFFFFFF));
				m_pDC->FillEllipse(ellipse, m_pBrush);
				m_pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Gray, 0.7));
				m_pDC->DrawEllipse(ellipse, m_pBrush, 1.0f);

				ellipse.radiusX = ellipse.radiusY = cxy * 3 / 6;// cxyTrack / 2.f;
				m_pBrush->SetColor(D2D1::ColorF(0x227988));
				m_pDC->FillEllipse(ellipse, m_pBrush);
			}

			EndPaint(ps);
		}
		return 0;

		case WM_NCHITTEST:
		{
			POINT pt ECK_GET_PT_LPARAM(lParam);
			ClientToElem(pt);

			D2D1_RECT_F rcTrack;
			GetTrackRect(rcTrack);
			if (!PtInRect(rcTrack, pt))
				return HTTRANSPARENT;
		}
		return HTCLIENT;

		case WM_MOUSEMOVE:
		{
			if (m_bLBtnDown)
			{
				POINT pt ECK_GET_PT_LPARAM(lParam);
				ClientToElem(pt);

				D2D1_RECT_F rcTrack;
				GetTrackRect(rcTrack);

				if (m_bVertical)
				{
					const float fScale = (pt.y - rcTrack.top) / (rcTrack.bottom - rcTrack.top);
					SetPos(m_fMin + (m_fMax - m_fMin) * fScale);
				}
				else
				{
					const float fScale = (pt.x - rcTrack.left) / (rcTrack.right - rcTrack.left);
					SetPos(m_fMin + (m_fMax - m_fMin) * fScale);
				}
				InvalidateRect();
			}
			else if (!m_bHover)
			{
				m_bHover = TRUE;
				m_Ec.Begin(m_Ec.GetCurrValue(), 1.0f, 100, 20);
				//InvalidateRect();
			}
		}
		return 0;

		case WM_MOUSELEAVE:
		{
			if (!m_bLBtnDown && m_bHover)
			{
				m_bHover = FALSE;
				m_Ec.Begin(m_Ec.GetCurrValue(), -m_Ec.GetCurrValue(), 100, 20);
				//InvalidateRect();
			}
		}
		return 0;

		case WM_LBUTTONDOWN:
		{
			m_bLBtnDown = TRUE;
			SetCapture();
		}
		return 0;

		case WM_LBUTTONUP:
		{
			if (m_bLBtnDown)
			{
				m_bLBtnDown = FALSE;
				ReleaseCapture();
			}
		}
		return 0;

		case WM_CREATE:
		{
			InitEasingCurve(m_Ec);
			m_Ec.SetCallBack([](float fCurrValue, float fOldValue, LPARAM lParam)
				{
					auto pElem = (CElem*)lParam;
					pElem->InvalidateRect();
				});
			m_pDC->CreateSolidColorBrush({}, &m_pBrush);
		}
		return 0;

		case WM_DESTROY:
		{
			SafeRelease(m_pBrush);
		}
		return 0;
		}
		return CElem::OnEvent(uMsg, wParam, lParam);
	}

	void SetRange(float fMin, float fMax)
	{
		m_fMin = fMin;
		m_fMax = fMax;
		if (m_fPos < m_fMin)
			m_fPos = m_fMin;
		else if (m_fPos > m_fMax)
			m_fPos = m_fMax;
	}

	void SetPos(float fPos)
	{
		m_fPos = fPos;
		if (m_fPos < m_fMin)
			m_fPos = m_fMin;
		else if (m_fPos > m_fMax)
			m_fPos = m_fMax;
	}

	float GetPos() const
	{
		return m_fPos;
	}

	void SetVertical(BOOL bVertical)
	{
		m_bVertical = bVertical;
	}

	BOOL IsVertical() const
	{
		return m_bVertical;
	}

	void SetTrackSize(float cxyTrack)
	{
		m_cxyTrack = cxyTrack;
	}

	float GetTrackSize() const
	{
		return m_cxyTrack;
	}
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END