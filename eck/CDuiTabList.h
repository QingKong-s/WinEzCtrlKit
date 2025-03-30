#pragma once
#include "CDuiListTemplate.h"
#include "CD2DImageList.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
struct TBL_DISPINFO :DUINMHDR
{
	int idx;
	DispInfoMask uMask;
	IDWriteTextLayout** ppTextLayout;
	PCWSTR pszText;
	int cchText;
	int idxImage;
	ID2D1Bitmap* pImage;
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
	struct ITEM
	{
		IDWriteTextLayout* pTextLayout;
	};

	CD2DImageList* m_pImageList{};
	ID2D1SolidColorBrush* m_pBrush{};
	CEasingCurve* m_pec1{}, * m_pec2{};
	int m_idxTo{ -1 }, m_idxFrom{ -1 };

	void PaintItem(int idx, const D2D1_RECT_F& rcItem, const D2D1_RECT_F& rcClip) override
	{
		__super::PaintItem(idx, rcItem, rcClip);
		TBL_DISPINFO di{};
		di.uCode = TBLE_GETDISPINFO;
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
		float x = rcItem.left + (float)CxIndicatorPadding + (float)CxIndicator;
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
			m_pImageList->GetImageSize(cx, cy);
			sizeImg.width = cyImg / cy * cx;
			sizeImg.height = cyImg;
			x += (sizeImg.width + Padding);
		}
		else
			goto SkipDrawImg;
		rc.left = rcItem.left + (float)CxIndicatorPadding + (float)CxIndicator;
		rc.top = rcItem.top + (m_cyItem - sizeImg.height) / 2.f;
		rc.right = rc.left + sizeImg.width;
		rc.bottom = rc.top + sizeImg.height;
		if (IsRectsIntersect(rc, rcClip))
			if (di.pImage)
				m_pDC->DrawBitmap(di.pImage, rc, 1.f, D2D1_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC);
			else if (di.idxImage >= 0)
				m_pImageList->Draw(di.idxImage, rc);
	SkipDrawImg:
		if (di.ppTextLayout)
		{
			if (di.cchText && !*di.ppTextLayout)
			{
				const auto cx = rcItem.right - rcItem.left - x;
				g_pDwFactory->CreateTextLayout(di.pszText, di.cchText,
					GetTextFormat(), cx, (float)m_cyItem, di.ppTextLayout);
			}

			if (*di.ppTextLayout)
			{
				if (rcClip.left < x && rcClip.right > x)
				{
					D2D1_COLOR_F cr;
					GetTheme()->GetSysColor(SysColor::Text, cr);
					m_pBrush->SetColor(cr);
					m_pDC->DrawTextLayout({ x,rcItem.top }, *di.ppTextLayout,
						m_pBrush, DrawTextLayoutFlags);
				}
			}
		}
	}

	static void EasingProc(float fCurrValue, float fOldValue, LPARAM lParam)
	{
		const auto p = (CTabList*)lParam;
		RECT rc;
		if (p->m_idxTo > p->m_idxFrom)
			p->GetItemRect(p->m_idxFrom, p->m_idxTo, rc);
		else
			p->GetItemRect(p->m_idxTo, p->m_idxFrom, rc);
		rc.left = CxIndicatorPadding;
		rc.right = CxIndicatorPadding + CxIndicator;
		p->ElemToClient(rc);
		p->InvalidateRect(rc);
	}
public:
	LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_NOTIFY:
			if (wParam == (WPARAM)&m_SB)
				if (((DUINMHDR*)lParam)->uCode == EE_VSCROLL)
					InvalidateRect();
			break;

		case WM_PAINT:
		{
			ELEMPAINTSTRU ps;
			BeginPaint(ps, wParam, lParam);

			const int idxBegin = HitTestY((int)ps.rcfClipInElem.top);
			const int idxEnd = std::min(HitTestY((int)ps.rcfClipInElem.bottom), m_cItem - 1);
			if (idxBegin >= 0 && idxBegin <= idxEnd)
			{
				D2D1_RECT_F rcItem;
				rcItem.left = 0.f;
				rcItem.top = float(idxBegin * (m_cyItem + m_cyPadding)) - m_psv->GetPos();
				rcItem.right = GetWidthF();
				rcItem.bottom = rcItem.top + (float)m_cyItem;
				for (int idx = idxBegin; idx <= idxEnd; ++idx)
				{
					PaintItem(idx, rcItem, ps.rcfClipInElem);
					OffsetRect(rcItem, 0.f, float(m_cyItem + m_cyPadding));
				}
			}

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
						float(m_idxFrom * (m_cyItem + m_cyPadding) - m_psv->GetPos()));
				}
				else// 向上
				{
					const auto d = (m_idxFrom - m_idxTo) * (m_cyItem + m_cyPadding);
					rc.top = -(m_pec1->GetCurrValue() * d - (float)CyIndicatorPadding);
					rc.bottom = -(m_pec2->GetCurrValue() * d - (m_cyItem - CyIndicatorPadding));
					OffsetRect(rc, 0.f,
						float(m_idxFrom * (m_cyItem + m_cyPadding) - m_psv->GetPos()));
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

			ECK_DUI_DBG_DRAW_FRAME;
			EndPaint(ps);
		}
		return 0;

		case WM_CREATE:
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
			break;
		case WM_DESTROY:
			SafeRelease(m_pBrush);
			break;
		}
		return __super::OnEvent(uMsg, wParam, lParam);
	}

	LRESULT OnNotify(DUINMHDR* pnm, BOOL& bProcessed) override
	{
		if (pnm->uCode == LTE_ITEMCLICK)
		{
			const auto p = (LTN_ITEM*)pnm;
			if (p->idxItem == m_idxSel || p->idxItem < 0 || m_idxSel < 0)
				return 0;
			m_idxFrom = m_idxSel;
			m_idxTo = p->idxItem;
			m_pec1->Begin(ECBF_CONTINUE);
			m_pec2->Begin(ECBF_CONTINUE);
			m_pec1->SetCurrTime(0.f);
			m_pec2->SetCurrTime(0.f);
			GetWnd()->WakeRenderThread();
		}
		return 0;
	}

	void SetImageList(CD2DImageList* pImageList)
	{
		m_pImageList = pImageList;
	}
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END