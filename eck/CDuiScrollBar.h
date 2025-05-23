﻿#pragma once
#include "DuiBase.h"
#include "CInertialScrollView.h"

#if !ECKCXX20
#error "EckDui requires C++20"
#endif// !ECKCXX20

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
class CScrollBar :public CElem
{
public:
	enum class SbPart
	{
		Button1,
		Button2,
		Track,
		Thumb,
	};
private:
	CInertialScrollView* m_psv{};
	CEasingCurve* m_pec{};
	BITBOOL m_bHot : 1{};
	BITBOOL m_bLBtnDown : 1{};
	BITBOOL m_bDragThumb : 1{};

	BITBOOL m_bHorizontal : 1{};
	BITBOOL m_bTransparentTrack : 1{ TRUE };
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
					GetPartRect(rc, SbPart::Thumb);
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
				m_psv->OnMouseMove((int)(m_bHorizontal ? pt.x - GetHeightF() : pt.y - GetWidthF()));
				DUINMHDR nm{ m_bHorizontal ? EE_HSCROLL : EE_VSCROLL };
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
			GetPartRect(rcf, SbPart::Thumb);
			if (PtInRect(rcf, ptf))
			{
				m_bDragThumb = TRUE;
				m_psv->OnLButtonDown((int)(m_bHorizontal ? pt.x - GetHeightF() : pt.y - GetWidthF()));
			}
			else
			{
				if (m_bHorizontal)
					m_psv->SetPos(ptf.x < rcf.left ?
						m_psv->GetPos() - m_psv->GetPage() :
						m_psv->GetPos() + m_psv->GetPage());
				else
					m_psv->SetPos(ptf.y < rcf.top ?
						m_psv->GetPos() - m_psv->GetPage() :
						m_psv->GetPos() + m_psv->GetPage());
				DUINMHDR nm{ m_bHorizontal ? EE_HSCROLL : EE_VSCROLL };
				if (!GenElemNotify(&nm))
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
					DUINMHDR nm{ m_bHorizontal ? EE_HSCROLL : EE_VSCROLL };
					if (!GenElemNotify(&nm))
						InvalidateRect();
				}
			}
		}
		return 0;
		case WM_SIZE:
		{
			const auto cx = GetWidth(),
				cy = GetHeight();
			if (m_bHorizontal)
				m_psv->SetViewSize(cx - 2 * cy);
			else
				m_psv->SetViewSize(cy - 2 * cx);
		}
		break;
		return 0;

		case WM_PAINT:
		{
			ELEMPAINTSTRU ps;
			BeginPaint(ps, wParam, lParam);

			D2D1_RECT_F rc;
			GetPartRect(rc, SbPart::Thumb);
			float cxyLeave, cxyMin;
			if (m_bHorizontal)
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
				DTB_OPT Opt;
				Opt.uFlags = DTBO_NEW_OPACITY;
				Opt.fOpacity = m_pec->GetCurrValue();
				GetTheme()->DrawBackground(Part::ScrollBar, State::Hot,
					GetViewRectF(), &Opt);
				if (m_bHorizontal)
					rc.top = rc.bottom - cxyMin - cxyLeave * m_pec->GetCurrValue();
				else
					rc.left = rc.right - cxyMin - cxyLeave * m_pec->GetCurrValue();
			}
			else
			{
				if (!m_bHot)
				{
					if (m_bHorizontal)
						rc.top += cxyLeave;
					else
						rc.left += cxyLeave;
				}
				else
					GetTheme()->DrawBackground(Part::ScrollBar, State::Hot,
						GetViewRectF(), nullptr);
			}

			GetTheme()->DrawBackground(Part::ScrollThumb,
				State::Normal, rc, nullptr);

			EndPaint(ps);
		}
		return 0;

		case WM_MOUSEWHEEL:
		case WM_MOUSEHWHEEL:
		{
			const auto p = GetParentElem();
			if (p)
				return p->CallEvent(uMsg, wParam, lParam);
		}
		break;

		case WM_CREATE:
		{
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
		}
		return 0;

		case WM_DESTROY:
		{
			GetWnd()->UnregisterTimeLine(m_pec);
			GetWnd()->UnregisterTimeLine(m_psv);
			SafeRelease(m_pec);
			SafeRelease(m_psv);
		}
		return 0;
		}
		return CElem::OnEvent(uMsg, wParam, lParam);
	}

	EckInlineNdCe auto GetScrollView() const noexcept { return m_psv; }

	void GetPartRect(D2D1_RECT_F& rc, SbPart eType)
	{
		const auto cx = GetWidthF(),
			cy = GetHeightF();
		if (m_bHorizontal)
			switch (eType)
			{
			case SbPart::Button1:
				rc = { 0,0,cy,cy };
				return;
			case SbPart::Button2:
				rc = { cx - cy,0,cx,cy };
				return;
			case SbPart::Track:
				rc = { cy,0,cx - cy,cy };
				return;
			case SbPart::Thumb:
			{
				const int cyThumb = (int)GetTheme()->GetMetrics(Metrics::CyHThumb);
				const int cxyThumb = m_psv->GetThumbSize();
				rc.left = cy + m_psv->GetThumbPos(cxyThumb);
				rc.top = (cy - cyThumb) / 2;
				rc.right = rc.left + cxyThumb;
				rc.bottom = rc.top + cyThumb;
			}
			return;
			}
		else
			switch (eType)
			{
			case SbPart::Button1:
				rc = { 0,0,cx,cx };
				return;
			case SbPart::Button2:
				rc = { 0,cy - cx,cx,cy };
				return;
			case SbPart::Track:
				rc = { 0,cx,cx,cy - cx };
				return;
			case SbPart::Thumb:
			{
				const int cxThumb = (int)GetTheme()->GetMetrics(Metrics::CxVThumb);
				const int cxyThumb = m_psv->GetThumbSize();
				rc.left = (cx - cxThumb) / 2;
				rc.top = cx + m_psv->GetThumbPos(cxyThumb);
				rc.right = rc.left + cxThumb;
				rc.bottom = rc.top + cxyThumb;
			}
			return;
			}
		ECK_UNREACHABLE;
	}

	EckInlineCe void SetHorizontal(BOOL b) noexcept { m_bHorizontal = b; }
	EckInlineNdCe BOOL GetHorizontal() const noexcept { return m_bHorizontal; }

	EckInlineCe void SetTransparentTrack(BOOL b) noexcept { m_bTransparentTrack = b; }
	EckInlineNdCe BOOL GetTransparentTrack() const noexcept { return m_bTransparentTrack; }
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END