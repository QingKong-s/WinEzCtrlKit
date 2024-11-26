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
	IDWriteTextLayout* m_pLayout{};
	ID2D1SolidColorBrush* m_pBrush{};

	D2D1_COLOR_F m_crText = D2D1::ColorF(0);
	D2D1_COLOR_F m_crBk = D2D1::ColorF(0xFFFFFF);

	BITBOOL m_bUseThemeColor : 1{ TRUE };
	BITBOOL m_bTransparent : 1{ TRUE };
public:
	LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_PAINT:
		{
			ELEMPAINTSTRU ps;
			BeginPaint(ps, wParam, lParam);

			if (m_bUseThemeColor)
			{
				D2D1_COLOR_F cr;
				GetTheme()->GetSysColor(SysColor::Text, cr);
				m_pBrush->SetColor(cr);
			}
			else
				m_pBrush->SetColor(m_crText);

			m_pDC->DrawTextLayout({}, m_pLayout, m_pBrush,
				D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT);

			EndPaint(ps);
		}
		return 0;

		case WM_SETTEXT:
		{
			CElem::OnEvent(uMsg, wParam, lParam);
			SafeRelease(m_pLayout);
			g_pDwFactory->CreateTextLayout(GetText().Data(), GetText().Size(),
				GetTextFormat(), GetWidthF(), GetHeightF(), &m_pLayout);
		}
		return 0;

		case WM_ERASEBKGND:
		{
			const auto* const pps = (ELEMPAINTSTRU*)lParam;
			if (!m_bTransparent)
			{
				if (m_bUseThemeColor)
				{
					D2D1_COLOR_F cr;
					GetTheme()->GetSysColor(SysColor::Bk, cr);
					m_pBrush->SetColor(cr);
				}
				else
					m_pBrush->SetColor(m_crBk);
				m_pDC->FillRectangle(pps->rcfClip, m_pBrush);
			}
		}
		return 0;

		case WM_CREATE:
			m_pDC->CreateSolidColorBrush({}, &m_pBrush);
			break;

		case WM_DESTROY:
			SafeRelease(m_pLayout);
			SafeRelease(m_pBrush);
			break;
		}
		return CElem::OnEvent(uMsg, wParam, lParam);
	}
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END