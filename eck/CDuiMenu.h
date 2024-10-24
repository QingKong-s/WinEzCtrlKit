/*
* WinEzCtrlKit Library
*
* CDuiMenu.h £º DUI²Ëµ¥
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#if ECKCXX20
#include "DuiBase.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
class CMenu;
namespace Priv
{
	class CMenuCtrl :public CElem
	{
		friend class CMenu;
	private:
		CMenu& m_Menu;
		CEasingCurve* m_pec{};
		ID2D1SolidColorBrush* m_pBr{}, * m_pBrText{};

	public:
		CMenuCtrl(CMenu& x) :m_Menu{ x } {}

		LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

		EckInline void BeginAnimation(UINT uFlags)
		{
			m_pec->Begin();
			GetWnd()->WakeRenderThread();
		}
	};
}

class CMenu :public CDuiWnd
{
	friend class Priv::CMenuCtrl;
private:
	struct ITEM
	{
		CRefStrW rsText;
		UINT_PTR uId;
		UINT uState;
		CMenu* pSubMenu;
		int cy;
		int cx;
		int y;
		IDWriteTextLayout* pTextLayout;
	};

	enum :UINT
	{
		MIS_NONE = 0,
		MIS_DISABLED = 1u << 0,
		MIS_HOT = 1u << 1,
		MIS_SELECTED = 1u << 2,
		MIS_CHECKED = 1u << 3,
	};

	std::vector<ITEM> m_vItem{};

	BITBOOL m_bMetricsDirty : 1{ TRUE };
	BITBOOL m_bPopup : 1{ FALSE };

	int m_cxMenu{}, m_cyMenu{};
	IDWriteTextFormat* m_pTextFormat{};
	Priv::CMenuCtrl m_MenuCtrl{ *this };

	int m_iDpi{ USER_DEFAULT_SCREEN_DPI };
	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY(cyItemMargin, 10)
		ECK_DS_ENTRY(rMenuMargin, 5)
		ECK_DS_ENTRY(lMenuMargin, 20)
		;
	ECK_DS_END_VAR(m_Ds);

	void PrePopup(int& xScreen, int& yScreen)
	{
		const auto hMon = MonitorFromPoint({ xScreen, yScreen }, MONITOR_DEFAULTTONEAREST);
		MONITORINFO mi{ sizeof(MONITORINFO) };
		GetMonitorInfoW(hMon, &mi);
		const auto rLeave = xScreen - mi.rcWork.left;
		const auto lLeave = mi.rcWork.right - xScreen;
		const auto tLeave = yScreen - mi.rcWork.top;
		const auto bLeave = mi.rcWork.bottom - yScreen;

		const auto cxWork = std::max(rLeave, lLeave);
		const auto cyWork = std::max(tLeave, bLeave);
		if (m_bMetricsDirty)
		{
			m_bMetricsDirty = FALSE;
			m_cxMenu = m_cyMenu = 0;
			int y{};
			DWRITE_TEXT_METRICS tm{};
			for (auto& e : m_vItem)
			{
				e.y = y;
				if (e.pTextLayout)
					e.pTextLayout->Release();
				g_pDwFactory->CreateTextLayout(e.rsText.Data(), (UINT32)e.rsText.Size(),
					m_pTextFormat, (float)cxWork, (float)cyWork, &e.pTextLayout);
				e.pTextLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
				e.pTextLayout->GetMetrics(&tm);
				e.cx = (int)tm.width + m_Ds.rMenuMargin + m_Ds.lMenuMargin;
				e.cy = (int)tm.height + m_Ds.cyItemMargin * 2;
				y += e.cy;
				m_cxMenu = std::max(m_cxMenu, e.cx);
				m_cyMenu += e.cy;
			}
		}

		if (m_cxMenu > lLeave)
			xScreen = xScreen - m_cxMenu + lLeave;
		if (m_cyMenu > bLeave)
			yScreen = yScreen - m_cyMenu + bLeave;
	}
public:
	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_CREATE:
		{
			CDuiWnd::OnMsg(hWnd, uMsg, wParam, lParam);
			m_MenuCtrl.Create(NULL, DES_TRANSPARENT|DES_VISIBLE, 0,
				0, 0, m_cxMenu, m_cyMenu, NULL, this);
		}
		return 0;

		case WM_CAPTURECHANGED:
		{
			CDuiWnd::OnMsg(hWnd, uMsg, wParam, lParam);
			Destroy();
		}
		break;

		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		{
			CDuiWnd::OnMsg(hWnd, uMsg, wParam, lParam);
			ReleaseCapture();
		}
		return 0;

		case WM_DESTROY:
		{
			m_bMetricsDirty = TRUE;
			m_bPopup = FALSE;

		}
		break;
		}
		return CDuiWnd::OnMsg(hWnd, uMsg, wParam, lParam);
	}

	INT_PTR Track(int xScreen, int yScreen)
	{
		PrePopup(xScreen, yScreen);// ×ø±êÐÞÕý

		IntCreate(WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE,
			WCN_DUIMENU, NULL, WS_POPUP | WS_VISIBLE,
			xScreen, yScreen, m_cxMenu+700, m_cyMenu+300, NULL, NULL, g_hInstance, NULL);

		m_bPopup = TRUE;
		SetCapture(HWnd);
		Redraw();
		m_MenuCtrl.BeginAnimation(0);
		//while(GetMessageW())
		return 0;
	}

	int Append(UINT uFlags, UINT_PTR uId, PCWSTR pszText)
	{
		EckAssert(!m_bPopup);
		m_bMetricsDirty = TRUE;
		m_vItem.emplace_back(pszText, uId);
		return (int)m_vItem.size() - 1;
	}

	void SetTextFormat(IDWriteTextFormat* pTextFormat)
	{
		EckAssert(!m_bPopup);
		std::swap(m_pTextFormat, pTextFormat);
		if(m_pTextFormat)
			m_pTextFormat->AddRef();
		if (pTextFormat)
			pTextFormat->Release();
		m_bMetricsDirty = TRUE;
	}
};

LRESULT Priv::CMenuCtrl::OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_PAINT:
	{
		ELEMPAINTSTRU ps;
		BeginPaint(ps, wParam, lParam);

		const auto& cr = m_pColorTheme->Get();
		D2D1_RECT_F rcItem;
		rcItem.left = 0;
		rcItem.right = (float)m_Menu.m_cxMenu;
		for (auto& e : m_Menu.m_vItem)
		{
			/*if (e.y >= m_pec->GetCurrValue())
				break;*/
			if (e.uState & CMenu::MIS_DISABLED)
			{
				m_pBr->SetColor(cr.crBkDisabled);
				m_pBrText->SetColor(cr.crTextDisabled);
			}
			else if (e.uState & CMenu::MIS_HOT)
				if (e.uState & CMenu::MIS_SELECTED)
				{
					m_pBr->SetColor(cr.crBkHotSel);
					m_pBrText->SetColor(cr.crTextSelected);
				}
				else
				{
					m_pBr->SetColor(cr.crBkHot);
					m_pBrText->SetColor(cr.crTextHot);
				}
			else if (e.uState & CMenu::MIS_SELECTED)
			{
				m_pBr->SetColor(cr.crBkSelected);
				m_pBrText->SetColor(cr.crTextSelected);
			}
			else
			{
				m_pBr->SetColor(cr.crBkNormal);
				m_pBrText->SetColor(cr.crTextNormal);
			}
			m_pBrText->SetColor(D2D1::ColorF(D2D1::ColorF::White));
			m_pBr->SetColor(D2D1::ColorF(D2D1::ColorF::Red));

			rcItem.top = e.y;
			rcItem.bottom = (float)(e.y + e.cy);
			m_pDC->FillRectangle(rcItem, m_pBr);
			e.pTextLayout->Release();
			g_pDwFactory->CreateTextLayout(e.rsText.Data(), (UINT32)e.rsText.Size(),
				m_Menu.m_pTextFormat, (float)m_Menu.m_cxMenu, (float)e.cy, &e.pTextLayout);
			m_pDC->DrawTextLayout(D2D1::Point2F(rcItem.left,rcItem.top),e.pTextLayout, m_pBrText);
		}

		EndPaint(ps);
	}
	return 0;

	case WM_CREATE:
	{
		SetColorTheme(GetWnd()->GetDefColorTheme()[CTI_BUTTON]);
		m_pec = new CEasingCurve{};
		InitEasingCurve(m_pec);
		m_pec->SetRange(0.f, (float)m_Menu.m_cyMenu);
		m_pec->SetDuration(300);
		m_pec->SetAnProc(Easing::OutSine);
		m_pec->SetCallBack([](float fCurrValue, float fOldValue, LPARAM lParam)
			{
				auto pElem = (CElem*)lParam;
				pElem->InvalidateRect();
			});
		m_pDC->CreateSolidColorBrush({}, &m_pBr);
		m_pDC->CreateSolidColorBrush({}, &m_pBrText);
	}
	break;

	case WM_DESTROY:
	{
		GetWnd()->UnregisterTimeLine(m_pec);
		m_pec->Release();
		m_pBr->Release();
		m_pBrText->Release();
	}
	break;
	}
	return CElem::OnEvent(uMsg, wParam, lParam);
}
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END
#else
#error "EckDui requires C++20"
#endif// ECKCXX20