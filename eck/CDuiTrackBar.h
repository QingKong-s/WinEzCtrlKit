#pragma once
#include "DuiBase.h"

#if !ECKCXX20
#error "EckDui requires C++20"
#endif// !ECKCXX20

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
class CTrackBar :public CElem
{
private:
	BITBOOL m_bVertical : 1{};
	BITBOOL m_bLBtnDown : 1{};
	BITBOOL m_bHover : 1{};

	BITBOOL m_bGenEventWhenDragging : 1{};

	float m_fPos{};
	float m_fMin{};
	float m_fMax{ 100.0f };

	float m_fDragPos{};

	float m_cxyTrack{};

	CEasingCurve* m_pec{};

	float GetCxyTrack()
	{
		if (m_pec->IsActive())
			return  m_cxyTrack / 2.f + m_cxyTrack / 2.f * m_pec->GetCurrValue();
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

	void GetThumbRect(float cxyTrack, const D2D1_RECT_F& rcTrack, D2D1_RECT_F& rc)
	{
		const float cxy = cxyTrack * 3.f / 4.f * m_pec->GetCurrValue();
		if (m_bVertical)
		{
			rc.left = (rcTrack.left + rcTrack.right) / 2.f - cxy;
			rc.top = rcTrack.bottom - cxy;
		}
		else
		{
			rc.left = rcTrack.right - cxy;
			rc.top = (rcTrack.top + rcTrack.bottom) / 2.f - cxy;
		}
		rc.right = rc.left + cxy * 2;
		rc.bottom = rc.top + cxy * 2;
	}

	void GetThumbRect(D2D1_RECT_F& rc)
	{
		const auto cxy = GetTrackRect(rc);
		const float fScale = (GetPos() - m_fMin) / (m_fMax - m_fMin);
		if (m_bVertical)
			rc.bottom = rc.top + (rc.bottom - rc.top) * fScale;
		else
			rc.right = rc.left + (rc.right - rc.left) * fScale;
		GetThumbRect(cxy, rc, rc);
	}

	void SetDragPos(float fPos)
	{
		m_fDragPos = fPos;
		if (m_fDragPos < m_fMin)
			m_fDragPos = m_fMin;
		else if (m_fDragPos > m_fMax)
			m_fDragPos = m_fMax;
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

			DTB_OPT Opt;
			Opt.uFlags = DTBO_NEW_RADX | DTBO_NEW_RADY;
			const float cxyTrack = GetTrackRect(Opt.rcClip);
			Opt.fRadX = Opt.fRadY = cxyTrack / 2.f;

			GetTheme()->DrawBackground(Part::TrackBar, State::Normal,
				Opt.rcClip, &Opt);

			const float fScale = (GetPos() - m_fMin) / (m_fMax - m_fMin);
			if (m_bVertical)
				Opt.rcClip.bottom = Opt.rcClip.top + (Opt.rcClip.bottom - Opt.rcClip.top) * fScale;
			else
				Opt.rcClip.right = Opt.rcClip.left + (Opt.rcClip.right - Opt.rcClip.left) * fScale;
			GetTheme()->DrawBackground(Part::TrackBar, State::Selected,
				Opt.rcClip, &Opt);

			if (m_bHover || m_pec->IsActive())
			{
				const float cxy = cxyTrack * 3.f / 4.f * m_pec->GetCurrValue();
				GetThumbRect(cxyTrack, Opt.rcClip, Opt.rcClip);
				GetTheme()->DrawBackground(Part::TrackBarThumb, State::Hot,
					Opt.rcClip);
			}

			ECK_DUI_DBG_DRAW_FRAME;
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

				SetDragPos(PtToPos(pt));

				if (m_bGenEventWhenDragging)
				{
					DUINMHDR nm{ TBE_POSCHANGED };
					GenElemNotify(&nm);
				}
				InvalidateRect();
			}
			else if (!m_bHover)
			{
				m_bHover = TRUE;
				m_pec->SetReverse(FALSE);
				m_pec->Begin(ECBF_CONTINUE);
				GetWnd()->WakeRenderThread();
			}
		}
		return 0;

		case WM_MOUSELEAVE:
		{
			if (!m_bLBtnDown && m_bHover)
			{
				m_bHover = FALSE;
				m_pec->SetReverse(TRUE);
				m_pec->Begin(ECBF_CONTINUE);
				GetWnd()->WakeRenderThread();
			}
		}
		return 0;

		case WM_LBUTTONDOWN:
		{
			POINT pt ECK_GET_PT_LPARAM(lParam);
			ClientToElem(pt);

			D2D1_RECT_F rcThumb;
			GetThumbRect(rcThumb);
			if (PtInRect(rcThumb, MakeD2dPtF(pt)))
			{
				m_bLBtnDown = TRUE;
				SetCapture();
				m_fDragPos = PtToPos(pt);
			}
			else
			{
				SetPos(PtToPos(pt));
				InvalidateRect();
				DUINMHDR nm{ TBE_POSCHANGED };
				GenElemNotify(&nm);
			}
		}
		return 0;

		case WM_LBUTTONUP:
		{
			if (m_bLBtnDown)
			{
				POINT pt ECK_GET_PT_LPARAM(lParam);
				ClientToElem(pt);

				m_bLBtnDown = FALSE;
				ReleaseCapture();
				SetPos(PtToPos(pt));
				InvalidateRect();
				DUINMHDR nm{ TBE_POSCHANGED };
				GenElemNotify(&nm);

				if (m_bHover)
				{
					m_bHover = FALSE;
					m_pec->SetReverse(TRUE);
					m_pec->Begin(ECBF_CONTINUE);
				}
			}
		}
		return 0;

		case WM_CREATE:
		{
			m_pec = new CEasingCurve{};
			InitEasingCurve(m_pec);
			m_pec->SetRange(0.f, 1.f);
			m_pec->SetDuration(200);
			m_pec->SetAnProc(Easing::OutSine);
			m_pec->SetCallBack([](float fCurrValue, float fOldValue, LPARAM lParam)
				{
					auto pElem = (CElem*)lParam;
					pElem->InvalidateRect();
				});
		}
		return 0;

		case WM_DESTROY:
			SafeRelease(m_pec);
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

	EckInline float GetPos() const
	{
		return m_bLBtnDown ? m_fDragPos : m_fPos;
	}

	EckInline void SetVertical(BOOL bVertical)
	{
		m_bVertical = bVertical;
	}

	EckInline BOOL IsVertical() const
	{
		return m_bVertical;
	}

	EckInline void SetTrackSize(float cxyTrack)
	{
		m_cxyTrack = cxyTrack;
	}

	EckInline float GetTrackSize() const
	{
		return m_cxyTrack;
	}

	EckInline void SetGenEventWhenDragging(BOOL bGenEventWhenDragging)
	{
		m_bGenEventWhenDragging = bGenEventWhenDragging;
	}

	float PtToPos(POINT pt)
	{
		D2D1_RECT_F rcTrack;
		GetTrackRect(rcTrack);

		if (m_bVertical)
		{
			const float fScale = (pt.y - rcTrack.top) / (rcTrack.bottom - rcTrack.top);
			return m_fMin + (m_fMax - m_fMin) * fScale;
		}
		else
		{
			const float fScale = (pt.x - rcTrack.left) / (rcTrack.right - rcTrack.left);
			return m_fMin + (m_fMax - m_fMin) * fScale;
		}
	}
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END