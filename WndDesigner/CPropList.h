#pragma once
#include "CApp.h"

#include <variant>

#include "..\eck\CRefStr.h"
#include "..\eck\CBk.h"
#include "..\eck\CListView.h"
#include "..\eck\DesignerDef.h"
#include "..\eck\CButton.h"
#include "..\eck\CEditExt.h"
#include "..\eck\CComboBox.h"
#include "..\eck\DesignerDef.h"

enum
{
	PLN_PROPCHANGED = 1
};

class CPropList :public eck::CListView
{
private:
	struct ITEM
	{
		int iID;
		eck::CRefStrW rsText;
		eck::EckCtrlPropType uType;
		std::vector<eck::CRefStrW> aPickStr;
		std::variant<eck::EckCtrlPropValue, eck::CRefStrW, eck::CRefBin> Val;
	};


	std::vector<ITEM> m_Items{};

	int m_cxPadding = 0;
	static constexpr int c_cxPadding = 4;
	static constexpr int
		IDC_EDIT = 101,
		IDC_COMBOBOX = 102,
		IDC_BUTTON = 103;

	eck::CEditExt m_Edit{};
	eck::CComboBox m_ComboBox{};
	eck::CPushButton m_Button{};

	int m_idxCurrEdit = -1;

	UINT m_uNotifyMsg = 0u;

	int m_idxInfo = -1;

	eck::CWnd* m_pWnd = nullptr;

	void UpdateDpiSize(int iDpi = 0)
	{
		if (iDpi == 0)
			iDpi = eck::GetDpi(m_hWnd);
		m_cxPadding = eck::DpiScale(c_cxPadding, iDpi);
	}

	void SaveEditingInfo(int idx)
	{
		if (m_idxInfo < 0)
			return;
		auto& Item = m_Items[idx];

		eck::EckCtrlPropValue Val;
		switch (Item.uType)
		{
		case eck::ECPT::Text:
			Item.Val = m_Edit.GetText();
			break;
		case eck::ECPT::Int:
			Val.Vi = _wtoi(m_Edit.GetText().Data());
			Item.Val = Val;
			break;
		case eck::ECPT::Float:
			Val.Vf = (float)_wtof(m_Edit.GetText().Data());
			Item.Val = Val;
			break;
		case eck::ECPT::Double:
			Val.Vlf = _wtof(m_Edit.GetText().Data());
			Item.Val = Val;
			break;
		case eck::ECPT::Bool:
			Val.Vb = m_ComboBox.GetCurSel();
			Item.Val = Val;
			break;
		case eck::ECPT::DateTime:
			break;
		case eck::ECPT::PickInt:
			Val.Vi = m_ComboBox.GetCurSel();
			Item.Val = Val;
			break;
		case eck::ECPT::PickText:
			break;
		case eck::ECPT::Image:
		case eck::ECPT::Customize:
		case eck::ECPT::Color:
		case eck::ECPT::Font:
		case eck::ECPT::ImageList:
			break;
		}

		if (Item.Val.index() == 1)
			Val.Vpsz = std::get<1>(Item.Val).Data();
		else if (Item.Val.index() == 2)
		{
			Val.Vbin.pData = std::get<2>(Item.Val).Data();
			Val.Vbin.cbSize = std::get<2>(Item.Val).Size();
		}
		eck::s_EckDesignAllCtrl[m_idxInfo].pfnSetProp(m_pWnd, Item.iID, &Val);
		if (m_uNotifyMsg)
			SendMessageW(GetParent(m_hWnd), m_uNotifyMsg, PLN_PROPCHANGED, m_idxCurrEdit);
		m_pWnd->Redraw();
	}

	EckInline int InsertItem(eck::EckCtrlPropEntry& Info, int idx = -1, eck::EckCtrlPropValue Val = {})
	{
		ITEM Item{ Info.iID,Info.pszChsName,Info.Type };

		PCWSTR pszPick = Info.pszPickStr;
		if (pszPick)
			while (*pszPick)
			{
				Item.aPickStr.push_back(pszPick);
				pszPick += (Item.aPickStr.back().Size() + 1);
			}

		switch (Info.Type)
		{
		case eck::ECPT::Text:
			Item.Val = Val.Vpsz;
			break;
		case eck::ECPT::Customize:
		case eck::ECPT::Image:
			Item.Val = eck::CRefBin(Val.Vbin.pData, Val.Vbin.cbSize);
			break;
		default:
			Item.Val = Val;
			break;
		}

		if (idx < 0)
		{
			m_Items.push_back(std::move(Item));
			idx = (int)m_Items.size() - 1;
		}
		else
			m_Items.insert(m_Items.begin() + idx, std::move(Item));

		return idx;
	}

	EckInline void RefreshCount()
	{
		SetItemCount((int)m_Items.size());
	}

	EckInline void DeleteAllItems()
	{
		m_Items.clear();
		RefreshCount();
	}
public:
	void GetPropFromCtrl(int idxItem)
	{
		if (m_idxInfo < 0)
			return;
		auto& Item = m_Items[idxItem];
		eck::EckCtrlPropValue Val{};
		auto iRet = eck::s_EckDesignAllCtrl[m_idxInfo].pfnGetProp(m_pWnd, Item.iID, &Val);

		switch (Item.uType)
		{
		case eck::ECPT::Text:
			Item.Val = Val.Vpsz;
			if (eck::IsBitSet(iRet, eck::ESPR_NEEDFREE))
				eck::TDesignAlloc::Free((BYTE*)Val.Vpsz);
			break;
		case eck::ECPT::Customize:
		case eck::ECPT::Image:
			Item.Val = eck::CRefBin(Val.Vbin.pData, Val.Vbin.cbSize);
			if (eck::IsBitSet(iRet, eck::ESPR_NEEDFREE))
				eck::TDesignAlloc::Free(Val.Vbin.pData);
			break;
		default:
			Item.Val = Val;
			break;
		}
	}

	void ExitEditing(BOOL bSave = FALSE)
	{
		if (m_idxCurrEdit < 0)
			return;
		if (bSave)
		{
			SaveEditingInfo(m_idxCurrEdit);
		}
		m_Edit.Show(SW_HIDE);
		m_ComboBox.Show(SW_HIDE);
		m_Button.Show(SW_HIDE);
		m_idxCurrEdit = -1;
	}

	EckInline void SetPropClass(int idxInfo = -1, eck::CWnd* pWnd = nullptr)
	{
		if (m_idxCurrEdit >= 0)
			SaveEditingInfo(m_idxCurrEdit);
		ExitEditing();
		m_Items.clear();
		m_idxInfo = idxInfo;
		m_pWnd = pWnd;

		int idx;

		EckCounter(ARRAYSIZE(eck::s_CommProp), i)
		{
			idx = InsertItem(eck::s_CommProp[i], (int)i);
			GetPropFromCtrl(idx);
		}

		if (idxInfo >= 0)
		{
			assert(pWnd);
			auto& ClassInfo = eck::s_EckDesignAllCtrl[idxInfo];
			EckCounter(ClassInfo.cProp, i)
			{
				idx = InsertItem(ClassInfo.pProp[i], (int)(ARRAYSIZE(eck::s_CommProp) + i));
				GetPropFromCtrl(idx);
			}
		}

		RefreshCount();
	}

	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = nullptr)
	{
		dwStyle |= (WS_CHILD | LVS_OWNERDATA | LVS_SINGLESEL | LVS_SHOWSELALWAYS);
		IntCreate(dwExStyle, WC_LISTVIEWW, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, nullptr, nullptr);

		UpdateDpiSize();

		m_Edit.Create(nullptr, ES_WANTRETURN | WS_CHILD, 0, 0, 0, 0, 0, m_hWnd, IDC_EDIT);
		m_ComboBox.Create(nullptr, CBS_DROPDOWNLIST | WS_CHILD, 0, 0, 0, 0, 0, m_hWnd, IDC_COMBOBOX);
		m_Button.Create(L"...", WS_CHILD, 0, 0, 0, 0, 0, m_hWnd, IDC_BUTTON);

		return m_hWnd;
	}

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	LRESULT OnNotifyMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bProcessed) override;

	EckInline void SetNotifyMsg(UINT uMsg)
	{
		m_uNotifyMsg = uMsg;
	}

	EckInline void SetValue(int idx, eck::EckCtrlPropValue Val)
	{
		if (idx < 0 || idx >= (int)m_Items.size())
			return;
		m_Items[idx].Val = Val;
		RedrawItems(idx, idx);
	}

	EckInline eck::EckCtrlPropValue GetValue(int idx)
	{
		eck::EckCtrlPropValue Val{};
		auto& Item = m_Items[idx];

		switch (Item.uType)
		{
		case eck::ECPT::Text:
			Val.Vpsz = std::get<1>(Item.Val).Data();
			return Val;
		case eck::ECPT::Customize:
		case eck::ECPT::Image:
			Val.Vbin = std::get<2>(Item.Val);
			return Val;
		default:
			return std::get<0>(Item.Val);
		}
	}
};