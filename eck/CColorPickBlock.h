#pragma once
#include "CStatic.h"

ECK_NAMESPACE_BEGIN
class CColorPickBlock :public CStatic
{
private:
	COLORREF m_crCust[16]{};
	COLORREF m_cr{ CLR_INVALID };
	DWORD m_dwCCFlags{ CC_FULLOPEN };
public:
	void AttachNew(HWND hWnd) override
	{
		CWnd::AttachNew(hWnd);
		SetText(NULL);
		Redraw();
	}

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_CREATE:
			((CREATESTRUCTW*)lParam)->lpszName = NULL;
			break;

		case WM_LBUTTONDBLCLK:
		{
			CHOOSECOLORW cc{ sizeof(CHOOSECOLORW) };
			cc.hwndOwner = hWnd;
			cc.Flags = m_dwCCFlags;
			cc.lpCustColors = m_crCust;
			if (ChooseColorW(&cc))
			{
				m_cr = cc.rgbResult;
				Redraw();
			}
		}
		return 0;

		case WM_SETTEXT:
			return TRUE;
		}
		return CStatic::OnMsg(hWnd, uMsg, wParam, lParam);
	}

	LRESULT OnNotifyMsg(HWND hParent, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bProcessed) override
	{
		switch (uMsg)
		{
		case WM_CTLCOLORSTATIC:
		{
			bProcessed = TRUE;
			SetDCBrushColor((HDC)wParam, m_cr);
			return (LRESULT)GetStockBrush(DC_BRUSH);
		}
		break;
		}
		return CStatic::OnNotifyMsg(hParent, uMsg, wParam, lParam, bProcessed);
	}

	EckInline void SetColor(COLORREF cr)
	{
		m_cr = cr;
		Redraw();
	}

	EckInline COLORREF GetColor() const
	{
		return m_cr;
	}

	EckInline const COLORREF* GetCustColor() const { return m_crCust; }

	EckInline COLORREF* GetCustColor() { return m_crCust; }

	EckInline DWORD GetChooseColorFlags() const { return m_dwCCFlags; }

	EckInline void SetChooseColorFlags(DWORD dwFlags)
	{
		m_dwCCFlags = dwFlags;
	}
};
ECK_NAMESPACE_END