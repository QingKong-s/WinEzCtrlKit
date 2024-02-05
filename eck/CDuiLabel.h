#pragma once
#include "DuiBase.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN

class CLabel : public CElem
{
private:
	IDWriteTextFormat* m_pTf = NULL;// 外部传入
	ID2D1SolidColorBrush* m_pBrush = NULL;

	D2D1_COLOR_F m_clrText = D2D1::ColorF(0);
	D2D1_COLOR_F m_clrBk = D2D1::ColorF(0xFFFFFF);
public:
	LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_CREATE:
			m_pDC->CreateSolidColorBrush(D2D1::ColorF(0), &m_pBrush);
			return 0;
		case WM_DESTROY:
			SafeRelease(m_pTf);
			SafeRelease(m_pBrush);
			return 0;
		}
		return CElem::OnEvent(uMsg, wParam, lParam);
	}

	void OnRedraw(const D2D1_RECT_F& rcClip, float ox, float oy) override
	{
		ECK_DUI_BEGINREDRAW;

		if (!(GetStyle() & DES_BLURBKG))
		{
			m_pBrush->SetColor(m_clrBk);
			m_pDC->FillRectangle(rcClip, m_pBrush);
		}

		if (m_pTf && !m_rsText.IsEmpty())
		{
			m_pBrush->SetColor(m_clrText);
			m_pDC->DrawTextW(m_rsText.Data(), m_rsText.Size(), m_pTf, rc, m_pBrush,
				D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT);
		}

		ECK_DUI_ENDREDRAW;
	}

	void SetTextFormat(IDWriteTextFormat* pTf)
	{
		std::swap(m_pTf, pTf);
		if (m_pTf)
			m_pTf->AddRef();
		if (pTf)
			pTf->Release();
	}
};

ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END