/*
* WinEzCtrlKit Library
*
* CColorPickBlock.h : 颜色选择器
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "CStatic.h"

ECK_NAMESPACE_BEGIN
inline constexpr int CDV_COLOR_PICK_BLOCK_1 = 1;

#pragma pack(push, ECK_CTRLDATA_ALIGN)
struct CTRLDATA_COLOR_PICK_BLOCK
{
	int iVer;
	COLORREF crCust[16];
	COLORREF cr;
	DWORD dwCCFlags;
};
#pragma pack(pop)

class CColorPickBlock :public CStatic
{
private:
	COLORREF m_crCust[16]{};
	COLORREF m_cr{ CLR_INVALID };
	DWORD m_dwCCFlags{ CC_FULLOPEN };

	void CleanupForDestroyWindow()
	{
		ZeroMemory(m_crCust, sizeof(m_crCust));
		m_cr = CLR_INVALID;
		m_dwCCFlags = CC_FULLOPEN;
	}
public:
	ECK_RTTI(CColorPickBlock);

	[[nodiscard]] EckInline constexpr static PCVOID SkipBaseData(PCVOID p)
	{
		return PtrStepCb(CStatic::SkipBaseData(p), sizeof(CTRLDATA_COLOR_PICK_BLOCK));
	}

	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = nullptr) override
	{
		if (pData)
		{
			const auto* const pBase = (CTRLDATA_WND*)pData;
			const auto* const p = (CTRLDATA_COLOR_PICK_BLOCK*)CStatic::SkipBaseData(pData);
			if (pBase->iVer < CDV_WND_1)
			{
				EckDbgBreak();
				return nullptr;
			}

			CStatic::Create(pszText, dwStyle, dwExStyle,
				x, y, cx, cy, hParent, hMenu, pData);
			memcpy(m_crCust, p->crCust, sizeof(m_crCust));
			m_cr = p->cr;
			m_dwCCFlags = p->dwCCFlags;
		}
		else
		{
			CStatic::Create(pszText, dwStyle, dwExStyle,
				x, y, cx, cy, hParent, hMenu);
		}
		return m_hWnd;
	}

	void SerializeData(CRefBin& rb)
	{
		CStatic::SerializeData(rb);
		constexpr auto cbSize = sizeof(CTRLDATA_COLOR_PICK_BLOCK);
		CMemWriter w(rb.PushBack(cbSize), cbSize);
		CTRLDATA_COLOR_PICK_BLOCK* p;
		w.SkipPointer(p);
		p->iVer = CDV_COLOR_PICK_BLOCK_1;
		memcpy(p->crCust, m_crCust, sizeof(m_crCust));
		p->cr = m_cr;
		p->dwCCFlags = m_dwCCFlags;
	}

	void AttachNew(HWND hWnd) override
	{
		CStatic::AttachNew(hWnd);
		SetText(nullptr);
		Redraw();
	}

	void DetachNew() override
	{
		CStatic::DetachNew();
		CleanupForDestroyWindow();
	}

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
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

		case WM_CREATE:
			((CREATESTRUCTW*)lParam)->lpszName = nullptr;
			break;
		case WM_DESTROY:
			CleanupForDestroyWindow();
			break;
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

	EckInline const COLORREF* GetCustomColor() const { return m_crCust; }

	EckInline COLORREF* GetCustomColor() { return m_crCust; }

	EckInline DWORD GetChooseColorFlags() const { return m_dwCCFlags; }

	EckInline void SetChooseColorFlags(DWORD dwFlags)
	{
		m_dwCCFlags = dwFlags;
	}
};
ECK_RTTI_IMPL_BASE_INLINE(CColorPickBlock, CStatic);
ECK_NAMESPACE_END