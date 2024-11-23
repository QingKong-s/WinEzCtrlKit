/*
* WinEzCtrlKit Library
*
* CDuiLabel.h ： DUI标签
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "DuiBase.h"

#if !ECKCXX20
#error "EckDui requires C++20"
#endif// !ECKCXX20

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN

class CLabel : public CElem
{
private:
	IDWriteTextFormat* m_pTf = nullptr;// 外部传入
	ID2D1SolidColorBrush* m_pBrush = nullptr;

	D2D1_COLOR_F m_crText = D2D1::ColorF(0);
	D2D1_COLOR_F m_crBk = D2D1::ColorF(0xFFFFFF);
public:
	LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_PAINT:
		{
			ELEMPAINTSTRU ps;
			BeginPaint(ps, wParam, lParam);

			if (m_pTf && !m_rsText.IsEmpty())
			{
				m_pBrush->SetColor(m_crText);
				m_pDC->DrawTextW(m_rsText.Data(), m_rsText.Size(), m_pTf, GetViewRectF(), m_pBrush,
					D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT);
			}

			EndPaint(ps);
		}
		return 0;
		case WM_ERASEBKGND:
		{
			const auto* const pps = (ELEMPAINTSTRU*)lParam;

			m_pBrush->SetColor(m_crBk);
			m_pDC->FillRectangle(pps->rcfClip, m_pBrush);
		}
		return 0;
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