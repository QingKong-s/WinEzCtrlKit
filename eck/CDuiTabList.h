#pragma once
#include "CDuiListTemplate.h"
#include "CD2DImageList.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
struct TBL_DISPINFO : DUINMHDR
{
	DispInfoMask uMask;
	int idx;
	PCWSTR pszText;
	int cchText;
	int idxImage;
	ID2D1Bitmap* pImage;
};

struct TBL_ITEM : DUINMHDR
{
	int idx;
};

class CTabList : public CListTemplate
{
public:
	enum
	{
		CxIndicator = 4,
		CxIndicatorPadding = 4,
		CyIndicatorPadding = 10
	};
protected:
	CEasingCurve* m_pec1{}, * m_pec2{};
	int m_idxTo{ -1 }, m_idxFrom{ -1 };
	int m_idxLastSel{ -1 };
	RECT m_rcLastRedraw{};

	void LVPaintSubItem(int idx, int idxSub, int idxGroup,
		const D2D1_RECT_F& rcSub, const D2D1_RECT_F& rcPaint) override
	{
		TBL_DISPINFO di{ TBLE_GETDISPINFO };
		di.uMask = DIM_TEXT | DIM_IMAGE;
		di.idxImage = -1;
		di.idx = idx;
		m_rsText.Reserve(MAX_PATH);
		di.pszText = m_rsText.Data();
		di.cchText = MAX_PATH;
		GenElemNotify(&di);

		const float Padding = GetTheme()->GetMetrics(Metrics::SmallPadding);
		const float Padding2 = GetTheme()->GetMetrics(Metrics::LargePadding);
		const float cyImg = m_cyItem - Padding2 * 2.f;
		float x = rcSub.left + (float)CxIndicatorPadding + (float)CxIndicator;
		D2D1_SIZE_F sizeImg;
		D2D1_RECT_F rc;
		if (di.pImage)
		{
			sizeImg = di.pImage->GetSize();
			sizeImg.width = cyImg / sizeImg.height * sizeImg.width;
			sizeImg.height = cyImg;
			x += (sizeImg.width + Padding);
		}
		else if (di.idxImage >= 0)
		{
			int cx, cy;
			m_pImgList->GetImageSize(cx, cy);
			sizeImg.width = cyImg / cy * cx;
			sizeImg.height = cyImg;
			x += (sizeImg.width + Padding);
		}
		else
			goto SkipDrawImg;
		rc.left = rcSub.left + (float)CxIndicatorPadding + (float)CxIndicator;
		rc.top = rcSub.top + (m_cyItem - sizeImg.height) / 2.f;
		rc.right = rc.left + sizeImg.width;
		rc.bottom = rc.top + sizeImg.height;
		if (IsRectsIntersect(rc, rcPaint))
			if (di.pImage)
				m_pDC->DrawBitmap(di.pImage, rc, 1.f, D2D1_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC);
			else if (di.idxImage >= 0)
				m_pImgList->Draw(di.idxImage, rc);
	SkipDrawImg:
		auto& e = m_vItem[idx];
		if (!e.pLayout.Get() && di.pszText && di.cchText > 0)
		{
			const auto cx = rcSub.right - rcSub.left - x;
			g_pDwFactory->CreateTextLayout(di.pszText, di.cchText,
				GetTextFormat(), cx, (float)m_cyItem, &e.pLayout);
		}
		if (e.pLayout.Get())
		{
			if (rcPaint.right >= x)
			{
				D2D1_COLOR_F cr;
				GetTheme()->GetSysColor(SysColor::Text, cr);
				m_pBrush->SetColor(cr);
				m_pDC->DrawTextLayout({ x,rcSub.top }, e.pLayout.Get(),
					m_pBrush, DrawTextLayoutFlags);
			}
		}
	}

	void PostPaint(ELEMPAINTSTRU& ps) override
	{
		if (m_pec2->IsActive())
		{
			D2D1_RECT_F rc;
			rc.left = (float)CxIndicatorPadding;
			rc.right = float(CxIndicatorPadding + CxIndicator);
			if (m_idxTo > m_idxFrom)// 向下
			{
				const auto d = (m_idxTo - m_idxFrom) * (m_cyItem + m_cyPadding);
				rc.bottom = m_pec1->GetCurrValue() * d + (m_cyItem - CyIndicatorPadding);
				rc.top = m_pec2->GetCurrValue() * d + (float)CyIndicatorPadding;
				OffsetRect(rc, 0.f,
					float(m_idxFrom * (m_cyItem + m_cyPadding) - m_psvV->GetPos()));
			}
			else// 向上
			{
				const auto d = (m_idxFrom - m_idxTo) * (m_cyItem + m_cyPadding);
				rc.top = -(m_pec1->GetCurrValue() * d - (float)CyIndicatorPadding);
				rc.bottom = -(m_pec2->GetCurrValue() * d - (m_cyItem - CyIndicatorPadding));
				OffsetRect(rc, 0.f,
					float(m_idxFrom * (m_cyItem + m_cyPadding) - m_psvV->GetPos()));
			}
			m_pBrush->SetColor(D2D1::ColorF(0x006FC4));
			m_pDC->FillRectangle(rc, m_pBrush);
		}
		else if (m_idxSel >= 0)
		{
			D2D1_RECT_F rc;
			GetItemRect(m_idxSel, rc);
			rc.left = (float)CxIndicatorPadding;
			rc.right = float(CxIndicatorPadding + CxIndicator);
			rc.top += (float)CyIndicatorPadding;
			rc.bottom -= (float)CyIndicatorPadding;
			m_pBrush->SetColor(D2D1::ColorF(0x006FC4));
			m_pDC->FillRectangle(rc, m_pBrush);
		}
	}

	static void EasingProc(float fCurrValue, float fOldValue, LPARAM lParam)
	{
		const auto p = (CTabList*)lParam;
		RECT rc, rc2;
		p->GetItemRect(p->m_idxFrom, rc);
		p->GetItemRect(p->m_idxTo, rc2);
		UnionRect(rc, rc, rc2);
		rc.left = CxIndicatorPadding;
		rc.right = CxIndicatorPadding + CxIndicator;
		if (p->m_rcLastRedraw.top < rc.top)
			rc.top = p->m_rcLastRedraw.top;
		if (p->m_rcLastRedraw.bottom > rc.bottom)
			rc.bottom = p->m_rcLastRedraw.bottom;
		p->m_rcLastRedraw = rc;

		p->ElemToClient(rc);
		p->InvalidateRect(rc);
	}
public:
	LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_CREATE:
			__super::OnEvent(uMsg, wParam, lParam);
			m_pDC->CreateSolidColorBrush({}, &m_pBrush);
			m_pec1 = new CEasingCurve{};
			m_pec1->SetAnProc(Easing::OutExpo);
			m_pec1->SetCallBack([](float fCurrValue, float fOldValue, LPARAM lParam)
				{
				});
			m_pec1->SetDuration(180);
			m_pec1->SetRange(0.f, 1.f);
			InitEasingCurve(m_pec1);

			m_pec2 = new CEasingCurve{};
			m_pec2->SetAnProc(Easing::OutExpo);
			m_pec2->SetCallBack(EasingProc);
			m_pec2->SetDuration(600);
			m_pec2->SetRange(0.f, 1.f);
			InitEasingCurve(m_pec2);

			SetView(Type::List);
			SetSingleSel(TRUE);
			SetItemNotify(TRUE);
			return 0;
		case WM_DESTROY:
			SafeRelease(m_pec1);
			SafeRelease(m_pec2);
			break;
		}
		return __super::OnEvent(uMsg, wParam, lParam);
	}

	LRESULT OnNotify(DUINMHDR* pnm, BOOL& bProcessed) override
	{
		if (pnm->uCode == EE_CLICK)
		{
			const auto p = (LTN_ITEM*)pnm;
			if (p->idx == m_idxLastSel || p->idx < 0 || m_idxLastSel < 0)
			{
				m_idxLastSel = p->idx;
				return 0;
			}
			m_idxFrom = m_idxLastSel;
			m_idxTo = p->idx;
			m_idxLastSel = p->idx;
			if (!m_pec1->IsActive() || !m_pec2->IsActive())
			{
				GetItemRect(m_idxFrom, m_rcLastRedraw);
				RECT rc2;
				GetItemRect(m_idxTo, rc2);
				UnionRect(m_rcLastRedraw, m_rcLastRedraw, rc2);
			}
			m_pec1->Begin(ECBF_CONTINUE);
			m_pec2->Begin(ECBF_CONTINUE);
			m_pec1->SetCurrTime(0.f);
			m_pec2->SetCurrTime(0.f);
			GetWnd()->WakeRenderThread();
		}
		else if (pnm->uCode == LTE_ITEMCHANED)
		{
			bProcessed = TRUE;
			const auto* const p = (LTN_ITEMCHANGE*)pnm;
			if ((p->uFlagsOld & LEIF_SELECTED) && !(p->uFlagsNew & LEIF_SELECTED))
				return TRUE;
			else
			{
				TBL_ITEM nm{ TBLE_SELCHANGED,p->idx };
				GenElemNotify(&nm);
			}
		}
		return 0;
	}
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END