﻿#pragma once
#include "DuiBase.h"
#include "CInertialScrollView.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
class CScrollBar :public CElem
{
public:
	enum class Part
	{
		Button1,
		Button2,
		Track,
		Thumb,
	};
private:
	CInertialScrollView* m_psv{};
	CEasingCurve* m_pec{};
	ID2D1SolidColorBrush* m_pBr{};
	BITBOOL m_bHot : 1 = FALSE;
	BITBOOL m_bLBtnDown : 1 = FALSE;
	BITBOOL m_bHorz : 1 = FALSE;
	BITBOOL m_bDragThumb : 1 = FALSE;
	BITBOOL m_bTransparentTrack : 1 = TRUE;
public:
	LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_NCHITTEST:
		{
			if (m_bHot)
				return HTCLIENT;
			else
			{
				if (m_bTransparentTrack)
				{
					D2D1_POINT_2F pt{ MakeD2dPtF(ECK_GET_PT_LPARAM(lParam)) };
					ClientToElem(pt);
					D2D1_RECT_F rc;
					GetPartRect(rc, Part::Thumb);
					if (!PtInRect(rc, pt))
						return HTTRANSPARENT;
				}
				return HTCLIENT;
			}
		}
		break;
		case WM_MOUSEMOVE:
		{
			if (!m_bHot)
			{
				m_bHot = TRUE;
				m_pec->SetReverse(FALSE);
				m_pec->Begin(ECBF_CONTINUE);
				GetWnd()->WakeRenderThread();
			}
			if (m_bDragThumb)
			{
				POINT pt ECK_GET_PT_LPARAM(lParam);
				ClientToElem(pt);
				m_psv->OnMouseMove((int)(m_bHorz ? pt.x - GetViewHeightF() : pt.y - GetViewWidthF()));
				DUINMHDR nm{ m_bHorz ? EE_HSCROLL : EE_VSCROLL };
				if (!GenElemNotify(&nm))
					InvalidateRect();
			}
		}
		return 0;
		case WM_MOUSELEAVE:
		{
			if (m_bHot && !m_bDragThumb)
			{
				m_bHot = FALSE;
				m_pec->SetReverse(TRUE);
				m_pec->Begin(ECBF_CONTINUE);
				GetWnd()->WakeRenderThread();
			}
		}
		return 0;
		case WM_LBUTTONDBLCLK:
		case WM_LBUTTONDOWN:
		{
			SetCapture();
			m_bLBtnDown = TRUE;
			POINT pt ECK_GET_PT_LPARAM(lParam);
			ClientToElem(pt);

			const D2D1_POINT_2F ptf{ MakeD2dPtF(pt) };
			D2D1_RECT_F rcf;
			GetPartRect(rcf, Part::Thumb);
			if (PtInRect(rcf, ptf))
			{
				m_bDragThumb = TRUE;
				m_psv->OnLButtonDown((int)(m_bHorz ? pt.x - GetViewHeightF() : pt.y - GetViewWidthF()));
			}
			else
			{
				if (m_bHorz)
					m_psv->SetPos(ptf.x < rcf.left ?
						m_psv->GetPos() - m_psv->GetPage() :
						m_psv->GetPos() + m_psv->GetPage());
				else
					m_psv->SetPos(ptf.y < rcf.top ?
						m_psv->GetPos() - m_psv->GetPage() :
						m_psv->GetPos() + m_psv->GetPage());
				InvalidateRect();
			}
		}
		return 0;
		case WM_CAPTURECHANGED:
		case WM_LBUTTONUP:
		{
			if (m_bLBtnDown)
			{
				m_bLBtnDown = FALSE;
				ReleaseCapture();
				if (m_bDragThumb)
				{
					m_psv->OnLButtonUp();
					m_bDragThumb = FALSE;
					DUINMHDR nm{ m_bHorz ? EE_HSCROLL : EE_VSCROLL };
					if (!GenElemNotify(&nm))
						InvalidateRect();
				}
			}
		}
		return 0;
		case WM_SIZE:
		{
			const auto cx = GetViewWidth(),
				cy = GetViewHeight();
			if (m_bHorz)
				m_psv->SetViewSize(cx - 2 * cy);
			else
				m_psv->SetViewSize(cy - 2 * cx);
		}
		return 0;

		case WM_PAINT:
		{
			ELEMPAINTSTRU ps;
			BeginPaint(ps, wParam, lParam);

			if (m_bHot)
			{
				m_pBr->SetColor(m_pColorTheme->Get().crBkNormal);
				m_pDC->FillRectangle(ps.rcfClipInElem, m_pBr);
			}

			D2D1_RECT_F rc;
			GetPartRect(rc, Part::Thumb);
			float cxyLeave, cxyMin;
			if (m_bHorz)
			{
				cxyLeave = (rc.bottom - rc.top) / 3 * 2;
				cxyMin = (rc.bottom - rc.top) - cxyLeave;
			}
			else
			{
				cxyLeave = (rc.right - rc.left) / 3 * 2;
				cxyMin = (rc.right - rc.left) - cxyLeave;
			}
			if (m_pec->IsActive())
			{
				if (m_bHorz)
					rc.top = rc.bottom - cxyMin - cxyLeave * m_pec->GetCurrValue();
				else
					rc.left = rc.right - cxyMin - cxyLeave * m_pec->GetCurrValue();
			}
			else if (!m_bHot)
				if (m_bHorz)
					rc.top += cxyLeave;
				else
					rc.left += cxyLeave;
			m_pBr->SetColor(m_pColorTheme->Get().crTextNormal);
			m_pDC->FillRectangle(rc, m_pBr);

			m_pBr->SetColor(D2D1::ColorF(D2D1::ColorF::Red));
			m_pDC->DrawRectangle(GetViewRectF(), m_pBr);
			
			EndPaint(ps);
		}
		return 0;

		case WM_CREATE:
		{
			m_pColorTheme = GetWnd()->GetDefColorTheme()[CTI_SCROLLBAR];
			m_pec = new CEasingCurve{};
			InitEasingCurve(m_pec);
			m_pec->SetCallBack([](float fOld, float f, LPARAM lParam)
				{
					auto p = (CScrollBar*)lParam;
					p->InvalidateRect();
				});
			m_pec->SetAnProc(Easing::OutSine);
			m_pec->SetDuration(160);
			m_pec->SetRange(0.f, 1.f);

			m_psv = new CInertialScrollView{};
			GetWnd()->RegisterTimeLine(m_psv);

			m_pDC->CreateSolidColorBrush({}, &m_pBr);
		}
		return 0;
		}
		return CElem::OnEvent(uMsg, wParam, lParam);
	}

	EckInline auto GetScrollView() { return m_psv; }

	void GetPartRect(D2D1_RECT_F& rc, Part eType)
	{
		const auto cx = GetViewWidthF(),
			cy = GetViewHeightF();
		if (m_bHorz)
			switch (eType)
			{
			case Part::Button1:
				rc = { 0,0,cy,cy };
				break;
			case Part::Button2:
				rc = { cx - cy,0,cx,cy };
				break;
			case Part::Track:
				rc = { cy,0,cx - cy,cy };
				break;
			case Part::Thumb:
			{
				const int cxyThumb = m_psv->GetThumbSize();
				rc.left = cy + m_psv->GetThumbPos(cxyThumb);
				rc.top = GetWnd()->GetDs().SBPadding;
				rc.right = rc.left + cxyThumb;
				rc.bottom = cy - GetWnd()->GetDs().SBPadding;
			}
			break;
			}
		else
			switch (eType)
			{
			case Part::Button1:
				rc = { 0,0,cx,cx };
				break;
			case Part::Button2:
				rc = { 0,cy - cx,cx,cy };
				break;
			case Part::Track:
				rc = { 0,cx,cx,cy - cx };
				break;
			case Part::Thumb:
			{
				const int cxyThumb = m_psv->GetThumbSize();
				rc.left = GetWnd()->GetDs().SBPadding;;
				rc.top = cx + m_psv->GetThumbPos(cxyThumb);
				rc.right = cx - GetWnd()->GetDs().SBPadding;
				rc.bottom = rc.top + cxyThumb;
			}
			break;
			}
		ECK_UNREACHABLE;
	}
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END