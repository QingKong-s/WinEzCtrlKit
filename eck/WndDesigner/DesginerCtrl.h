#pragma once
#include "../CBk.h"
#include "../CListView.h"
#include "../CSubclassMgr.h"
#include "../DesignerDef.h"
#include "../CButton.h"
#include "../CEdit.h"
#include "../CComboBox.h"
#include "../DesignerDef.h"

class CSizer;

enum class SizerHTCode
{
	None,
	LeftTop,
	RightTop,
	LeftBottom,
	RightBottom,
	Top,
	Bottom,
	Left,
	Right
};

class CSizerBlock :public eck::CBk
{
private:
	friend class CSizer;
	HBRUSH m_hbrBlock = NULL;
	RECT m_rcClient{};


	CSizer* m_pSizer = NULL;
	SizerHTCode m_uType = SizerHTCode::None;
	HCURSOR m_hCursor = NULL;

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
public:
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, UINT nID) override;

	EckInline void SetWindowProc(WNDPROC pfnWndProc)
	{
		EckDbgPrint(L"CSizerBlock::SetWindowProc ²»Ö§³Ö");
		return;
	}

	void BindSizer(CSizer* pSizer, SizerHTCode uType);
};

class CSizer
{
private:
	friend class CSizerBlock;
	HWND m_hBK = NULL;
	HWND m_hBottomWorkWnd = NULL;
	HWND m_hWorkWnd = NULL;
	HWND m_hWorkWndParent = NULL;
	int m_sizeBlock = 0;
	CSizerBlock m_Block[8]{};

	HDC m_hDC = NULL;
	HPEN m_hPen = NULL;
	HGDIOBJ m_hOld = NULL;
	RECT m_rcOrg{};
	RECT m_rcLast{};
	BOOL m_bLBtnDown = FALSE;
public:
	~CSizer()
	{
		for (auto& x : m_Block)
			DestroyWindow(x);
		DeleteObject(m_hPen);
	}

	void Create(HWND hBK, HWND hBottomWorkWnd);

	EckInline void SetBlockSize(int i)
	{
		m_sizeBlock = i;
	}

	HWND SetTargetWindow(HWND hWnd);

	EckInline HWND GetTargetWindow()
	{
		return m_hWorkWnd;
	}

	SizerHTCode HitTest(POINT pt);

	void MoveToTargetWindow();

	EckInline void Show(BOOL bShow)
	{
		int iShow = (bShow ? SW_SHOWNOACTIVATE : SW_HIDE);
		for (auto& x : m_Block)
			ShowWindow(x, iShow);
	}
private:
	RECT SizerMakeRect(POINT ptCursor, SizerHTCode uType);

	void OnBlockLButtonDown(CSizerBlock* pBlock, LPARAM lParam);

	void OnBlockLButtonUp(CSizerBlock* pBlock, LPARAM lParam);

	void OnBlockMouseMove(CSizerBlock* pBlock, LPARAM lParam);
};

#include <variant>

#define SCID_PROPLISTPARENT 20230808'01

class CPropList :public eck::CListView
{
	SUBCLASS_MGR_DECL(CPropList)
	SUBCLASS_REF_MGR_DECL(CPropList, eck::ObjRecorderRefPlaceholder)
private:
	struct ITEM
	{
		int iID;
		eck::CRefStrW rsText;
		eck::EckCtrlPropType uType;
		std::vector<eck::CRefStrW> aPickStr;
		std::variant<eck::EckCtrlPropValue, eck::CRefStrW, eck::CRefBin> Val;
	};


	std::vector<ITEM> m_Items;

	int m_cxPadding = 0;
	static constexpr int c_cxPadding = 4;
	static constexpr int
		IDC_EDIT = 101,
		IDC_COMBOBOX = 102,
		IDC_BUTTON = 103;

	eck::CEdit m_Edit{};
	eck::CComboBox m_ComboBox{};
	eck::CPushButton m_Button{};

	int m_idxCurrEdit = -1;

	UINT m_uNotifyMsg = 0u;

	int m_idxInfo = -1;

	eck::CWnd* m_pWnd = NULL;

	void UpdateDpiSize(int iDpi = 0)
	{
		if (iDpi == 0)
			iDpi = eck::GetDpi(m_hWnd);
		m_cxPadding = eck::DpiScale(c_cxPadding, iDpi);
	}

	void SaveEditingInfo(int idx)
	{
		auto& Item = m_Items[idx];
		eck::EckCtrlPropValue Val;
		switch (Item.uType)
		{
		case eck::ECPT::Text:
			Item.Val = m_Edit.GetText();
			break;
		case eck::ECPT::Int:
			Val.Vi = _wtoi(m_Edit.GetText());
			Item.Val = Val;
			break;
		case eck::ECPT::Float:
			Val.Vf = (float)_wtof(m_Edit.GetText());
			Item.Val = Val;
			break;
		case eck::ECPT::Double:
			Val.Vlf = _wtof(m_Edit.GetText());
			Item.Val = Val;
			break;
		case eck::ECPT::Bool:
			Val.Vb = m_ComboBox.GetCurrSel();
			Item.Val = Val;
			break;
		case eck::ECPT::DateTime:
			break;
		case eck::ECPT::PickInt:
			Val.Vi = m_ComboBox.GetCurrSel();
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

		if(m_idxInfo>=0)
		{
			if (Item.Val.index() == 1)
				Val.Vpsz = std::get<1>(Item.Val);
			else if (Item.Val.index() == 2)
			{
				Val.Vbin.pData = std::get<2>(Item.Val);
				Val.Vbin.cbSize = std::get<2>(Item.Val).m_cb;
			}
			eck::s_EckDesignAllCtrl[m_idxInfo].pfnSetProp(m_pWnd, Item.iID, &Val);
		}
	}

	EckInline int InsertItem(eck::EckCtrlPropEntry& Info, int idx = -1, eck::EckCtrlPropValue Val = {})
	{
		ITEM Item{ Info.iID,Info.pszChsName,Info.Type };

		PCWSTR pszPick = Info.pszPickStr;
		if (pszPick)
			while (*pszPick)
			{
				Item.aPickStr.push_back(pszPick);
				pszPick += (Item.aPickStr.back().m_cchText + 1);
			}

		switch (Info.Type)
		{
		case eck::ECPT::Text:
			Item.Val = Val.Vpsz;
			break;
		case eck::ECPT::Customize:
		case eck::ECPT::Image:
		{
			eck::CRefBin rb(Val.Vbin.pData, Val.Vbin.cbSize);
			Item.Val = std::move(rb);
		}
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

	static LRESULT CALLBACK SubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
		UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

	static LRESULT CALLBACK SubclassProc_Parent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
		UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
public:
	EckInline void SetPropClass(int idxInfo = -1, eck::CWnd* pWnd = NULL)
	{
		m_Items.clear();
		EckCounter(ARRAYSIZE(eck::s_CommProp), i)
			InsertItem(eck::s_CommProp[i], (int)i);

		if (idxInfo >= 0)
		{
			assert(pWnd);
			auto& ClassInfo = eck::s_EckDesignAllCtrl[idxInfo];
			EckCounter(ClassInfo.cProp, i)
				InsertItem(ClassInfo.pProp[i], (int)(ARRAYSIZE(eck::s_CommProp) + i));
		}

		m_idxInfo = idxInfo;
		m_pWnd = pWnd;
		RefreshCount();
	}

	EckInline HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, UINT nID) override
	{
		dwStyle |= (WS_CHILD | LVS_OWNERDATA | LVS_SINGLESEL | LVS_SHOWSELALWAYS);
		m_hWnd = CreateWindowExW(dwExStyle, WC_LISTVIEWW, pszText, dwStyle,
			x, y, cx, cy, hParent, eck::i32ToP<HMENU>(nID), NULL, NULL);

		UpdateDpiSize();

		m_Edit.Create(NULL, 0, 0, 0, 0, 0, 0, m_hWnd, IDC_EDIT);
		m_ComboBox.Create(NULL, CBS_DROPDOWNLIST, 0, 0, 0, 0, 0, m_hWnd, IDC_COMBOBOX);
		m_Button.Create(L"...", 0, 0, 0, 0, 0, 0, m_hWnd, IDC_BUTTON);

		m_SM.AddSubclass(m_hWnd, this);
		m_SMRef.AddRef(GetParent(m_hWnd), eck::ObjRecorderRefPlaceholderVal);

		return m_hWnd;
	}

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
};