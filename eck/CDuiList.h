#pragma once
#include "CDuiListTemplate.h"


ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
struct NMLEDISPINFO : DUINMHDR
{
	DispInfoMask uMask;
	BOOL bItem;
	union
	{
		struct ITEM
		{
			int idx;
			int idxSub;
			int idxGroup;
			PCWSTR pszText;
			int cchText;
			int idxImg;
			ID2D1Bitmap* pImg;
		} Item;
		struct GROUP
		{
			int idx;
			PCWSTR pszText;
			int cchText;
			int idxImg;
			ID2D1Bitmap* pImg;
		} Group;
	};
};


class CList : public CListTemplate
{
private:
	D2D1_SIZE_F GetImageSize(const NMLEDISPINFO& es)
	{
		if (es.Item.pImg)
			return es.Item.pImg->GetSize();
		else if (m_pImgList && es.Item.idxImg >= 0)
		{
			int cx, cy;
			m_pImgList->GetImageSize(cx, cy);
			return { (float)cx, (float)cy };
		}
		return {};
	}

	void GRPaintGroup(const D2D1_RECT_F& rcPaint, NMLTCUSTOMDRAW& nm, LRESULT r) override
	{
		const auto Padding = GetTheme()->GetMetrics(Metrics::Padding);
		auto& e = m_Group[nm.idxGroup];

		D2D1_RECT_F rcText, rcGroupImg;
		GetGroupPartRect(ListPart::GroupHeader, -1, nm.idxGroup, rcText);
		GetGroupPartRect(ListPart::GroupImg, -1, nm.idxGroup, rcGroupImg);

		const BOOL bText = !(rcText.bottom <= rcPaint.top ||
			rcText.top >= rcPaint.bottom);
		const BOOL bGroupImg = IsRectsIntersect(rcGroupImg, rcPaint);
		if (!bText && !bGroupImg)
			return;

		NMLEDISPINFO sldi{};
		if (bText)
			sldi.uMask |= DIM_TEXT;
		if (bGroupImg)
			sldi.uMask |= DIM_IMAGE;
		sldi.uCode = LEE_GETDISPINFO;
		sldi.bItem = FALSE;
		sldi.Group.cchText = -1;
		sldi.Group.idx = nm.idxGroup;
		GenElemNotify(&sldi);

		if (bText)
		{
			if (!e.pLayout.Get())
			{
				g_pDwFactory->CreateTextLayout(sldi.Group.pszText,
					sldi.Group.cchText, m_pTfGroup, GetWidthF() - Padding * 2.f,
					(float)m_cyGroupHeader, &e.pLayout);
			}
			if (e.pLayout.Get())
			{
				const auto Padding2 = GetTheme()->GetMetrics(Metrics::LargePadding);
				if (nm.bColorText)
					m_pBrush->SetColor(nm.crText);
				else
				{
					D2D1_COLOR_F cr;
					GetTheme()->GetSysColor(Dui::SysColor::MainTitle, cr);
					m_pBrush->SetColor(cr);
				}
				m_pDC->DrawTextLayout({ rcText.left + Padding,rcText.top }, e.pLayout.Get(),
					m_pBrush, DrawTextLayoutFlags);
				DWRITE_TEXT_METRICS tm;
				e.pLayout->GetMetrics(&tm);
				const float yLine = rcText.top + (float)(m_cyGroupHeader / 2);
				D2D1_POINT_2F pt1{ rcText.left + tm.width + Padding2 * 2.f ,yLine };
				D2D1_POINT_2F pt2{ GetWidthF() - Padding2,yLine };
				if (pt1.x < pt2.x)
					m_pDC->DrawLine(pt1, pt2, m_pBrush, (float)CyGroupLine);
			}
		}

		if (bGroupImg && sldi.Group.pImg)
			m_pDC->DrawBitmap(sldi.Group.pImg, &rcGroupImg);
	}

	void LVPaintSubItem(const D2D1_RECT_F& rcPaint, NMLTCUSTOMDRAW& nm, LRESULT r) override
	{
		NMLEDISPINFO es{ LEE_GETDISPINFO };
		es.bItem = TRUE;
		es.Item.idx = nm.idx;
		es.Item.idxSub = nm.idxSub;
		es.Item.idxGroup = nm.idxGroup;
		es.uMask = DIM_TEXT | DIM_IMAGE;
		GenElemNotify(&es);

		const auto sizeImg = GetImageSize(es);
		const auto Padding = GetTheme()->GetMetrics(Metrics::SmallPadding);
		auto& e = nm.idxGroup < 0 ? m_vItem[nm.idx] : m_Group[nm.idxGroup].Item[nm.idx];
		auto& pTl = (nm.idxSub ? e.vSubItem[nm.idxSub - 1].pLayout : e.pLayout);
		if (!pTl.Get() && es.Item.pszText && es.Item.cchText > 0)
		{
			g_pDwFactory->CreateTextLayout(es.Item.pszText, es.Item.cchText,
				GetTextFormat(),
				nm.rc.right - nm.rc.left - Padding * 3 - sizeImg.width,
				(float)m_cyItem, &pTl);
		}

		const float xImage = Padding;
		const float yImage = (float)((m_cyItem - sizeImg.height) / 2);
		float xText;
		if (es.Item.pImg || (m_pImgList && es.Item.idxImg >= 0))
		{
			auto rc{ nm.rc };
			rc.left += xImage;
			rc.right = rc.left + sizeImg.width;
			rc.top += yImage;
			rc.bottom = rc.top + sizeImg.height;
			xText = rc.right + Padding;
			if (!(rc.right <= rcPaint.left || rc.left >= rcPaint.right))
				if (es.Item.pImg)
					m_pDC->DrawBitmap(es.Item.pImg, rc);
				else/* if (m_pImgList && es.idxImg >= 0)*/
					m_pImgList->Draw(es.Item.idxImg, rc);
		}
		else
			xText = nm.rc.left + Padding;
		if (pTl.Get())
		{
			DWRITE_TEXT_METRICS tm;
			pTl->GetMetrics(&tm);
			if (!(xText + tm.width <= rcPaint.left || xText >= rcPaint.right))
			{
				if (nm.bColorText)
					m_pBrush->SetColor(nm.crText);
				else
				{
					D2D1_COLOR_F cr;
					GetTheme()->GetSysColor(SysColor::Text, cr);
					m_pBrush->SetColor(cr);
				}
				m_pDC->DrawTextLayout({ xText, nm.rc.top }, pTl.Get(), m_pBrush,
					DrawTextLayoutFlags);
			}
		}
	}

	void IVPaintItem(const D2D1_RECT_F& rcPaint, NMLTCUSTOMDRAW& nm, LRESULT r) override
	{
		NMLEDISPINFO es{ LEE_GETDISPINFO };
		es.bItem = TRUE;
		es.Item.idx = nm.idx;
		es.uMask = DIM_TEXT | DIM_IMAGE;
		GenElemNotify(&es);

		D2D1_SIZE_F sizeImg = GetImageSize(es);
		auto& e = m_vItem[nm.idx];

		RECT rcTmp;
		GetItemRect(nm.idx, rcTmp);
		auto rc{ MakeD2DRcF(rcTmp) };

		State eState;
		if ((e.uFlags & LEIF_SELECTED) || (m_bSingleSel && m_idxSel == nm.idx))
			if (m_idxHot == nm.idx)
				eState = State::HotSelected;
			else
				eState = State::Selected;
		else if (m_idxHot == nm.idx)
			eState = State::Hot;
		else
			eState = State::None;

		if (eState != State::None)
			GetTheme()->DrawBackground(Part::ListItem, eState, rc, nullptr);

		const float Padding = GetTheme()->GetMetrics(Metrics::SmallPadding);
		D2D1_RECT_F rcImg;
		rcImg.left = rc.left + (m_cxItem - sizeImg.width) / 2.f;
		rcImg.top = rc.top + Padding;
		rcImg.right = rcImg.left + sizeImg.width;
		rcImg.bottom = rcImg.top + sizeImg.height;

		if (!(rcImg.right <= rcPaint.left || rcImg.left >= rcPaint.right))
			if (es.Item.pImg)
				m_pDC->DrawBitmap(es.Item.pImg, rcImg, 1.f, D2D1_INTERPOLATION_MODE_LINEAR);
			else if (m_pImgList && es.Item.idxImg >= 0)
				m_pImgList->Draw(es.Item.idxImg, rcImg);

		if (!e.pLayout.Get() && es.Item.pszText)
		{
			EckAssert(es.Item.cchText > 0);
			g_pDwFactory->CreateTextLayout(es.Item.pszText, es.Item.cchText, GetTextFormat(),
				(float)m_cxItem, float(rc.bottom - rcImg.bottom), &e.pLayout);

			if (e.pLayout.Get())
			{
				e.pLayout->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
				e.pLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
				e.pLayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
			}
		}

		if (e.pLayout.Get())
		{
			if (nm.bColorText)
				m_pBrush->SetColor(nm.crText);
			else
			{
				D2D1_COLOR_F cr;
				GetTheme()->GetSysColor(SysColor::Text, cr);
				m_pBrush->SetColor(cr);
			}
			m_pDC->DrawTextLayout({ rc.left, rcImg.bottom + Padding },
				e.pLayout.Get(), m_pBrush, DrawTextLayoutFlags);
		}
	}
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END