#include "CPropList.h"

#define SCID_PROPLIST		20230808'02
#define SCID_PROPLISTPARENT 20230808'01

SUBCLASS_MGR_INIT(CPropList, SCID_PROPLIST, CPropList::SubclassProc)
SUBCLASS_REF_MGR_INIT(CPropList, eck::ObjRecorderRefPlaceholder, SCID_PROPLISTPARENT, CPropList::SubclassProc_Parent, eck::ObjRecordRefStdDeleter)

LRESULT CALLBACK CPropList::SubclassProc_Parent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
	UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	switch (uMsg)
	{
	case WM_NOTIFY:
	{
		auto pnmhdr = (NMHDR*)lParam;
		auto it = m_WndRecord.find(pnmhdr->hwndFrom);
		if (it == m_WndRecord.end())
			break;
		auto p = it->second;
		switch (pnmhdr->code)
		{
		case NM_CUSTOMDRAW:
		{
			auto pnmlvcd = (NMLVCUSTOMDRAW*)lParam;
			switch (pnmlvcd->nmcd.dwDrawStage)
			{
			case CDDS_PREPAINT:
				return CDRF_NOTIFYITEMDRAW;
			case CDDS_ITEMPREPAINT:
			{
				int idx = (int)pnmlvcd->nmcd.dwItemSpec;
				HDC hDC = pnmlvcd->nmcd.hdc;
				RECT rcHItem;
				auto Header = p->GetHeaderCtrl();
				auto& Item = p->m_Items[idx];

				Header.GetItemRect(0, &rcHItem);
				rcHItem.top = pnmlvcd->nmcd.rc.top;
				rcHItem.bottom = pnmlvcd->nmcd.rc.bottom;

				if (p->GetItemState(idx, LVIS_SELECTED) == LVIS_SELECTED)
				{
					FillRect(hDC, &rcHItem, GetSysColorBrush(COLOR_HIGHLIGHT));
					::SetTextColor(hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
				}
				else
				{
					FillRect(hDC, &rcHItem, GetSysColorBrush(COLOR_WINDOW));
					::SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));
				}

				rcHItem.left += p->m_cxPadding;
				DrawTextW(hDC, Item.rsText.Data(), Item.rsText.Size(), &rcHItem,
					DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);


				Header.GetItemRect(1, &rcHItem);
				rcHItem.top = pnmlvcd->nmcd.rc.top;
				rcHItem.bottom = pnmlvcd->nmcd.rc.bottom;

				FillRect(hDC, &rcHItem, GetSysColorBrush(COLOR_WINDOW));

				PCWSTR pszVal = NULL;

				std::wstring sVal;
				switch (Item.uType)
				{
				case eck::ECPT::Text:
					pszVal = std::get<1>(Item.Val).Data();
					break;
				case eck::ECPT::Image:
				case eck::ECPT::Customize:
					if (std::get<2>(Item.Val).m_cb)
						pszVal = L"有数据";
					break;
				default:
				{
					auto Union = std::get<0>(Item.Val);
					switch (Item.uType)
					{
					case eck::ECPT::Int:
						sVal = std::to_wstring(Union.Vi);
						break;
					case eck::ECPT::Float:
						sVal = std::to_wstring(Union.Vf);
						break;
					case eck::ECPT::Double:
						sVal = std::to_wstring(Union.Vlf);
						break;
					case eck::ECPT::Bool:
						sVal = (Union.Vb ? L"真" : L"假");
						break;
					case eck::ECPT::DateTime:
						break;
					case eck::ECPT::PickInt:
						sVal = std::to_wstring(Union.Vi) + L". " + Item.aPickStr[Union.Vi].Data();
						break;
					case eck::ECPT::PickText:
						break;
					case eck::ECPT::Color:
						break;
					case eck::ECPT::Font:
						break;
					case eck::ECPT::ImageList:
						break;
					}
				}
				break;
				}

				if (!pszVal)
					pszVal = sVal.c_str();
				rcHItem.left += p->m_cxPadding;
				::SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));
				DrawTextW(hDC, pszVal, -1, &rcHItem, DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
			}
			return CDRF_SKIPDEFAULT;
			}
		}
		break;

		case LVN_ITEMCHANGED:
		{
			auto pnmlv = (NMLISTVIEW*)lParam;


		}
		break;
		}
	}
	break;
	}

	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK CPropList::SubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
	UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	auto p = (CPropList*)dwRefData;
	switch (uMsg)
	{
	case WM_DPICHANGED_BEFOREPARENT:
		p->UpdateDpiSize(LOWORD(wParam));
		break;

	case WM_COMMAND:
	{
		if (lParam == (LPARAM)p->m_Button.GetHWND() && HIWORD(wParam) == BN_CLICKED)
		{
			if (p->m_idxCurrEdit < 0)
				return 0;
			auto& Item = p->m_Items[p->m_idxCurrEdit];
			//switch (Item.uType)
			//{
			//case eck::ECPT::Image:
			//case eck::ECPT::Customize:
			//case eck::ECPT::Color:
			//case eck::ECPT::Font:
			//case eck::ECPT::ImageList:
			//}
		}
		else if (lParam == (LPARAM)p->m_Edit.GetHWND() && HIWORD(wParam) == NM_RETURN)
		{
			if (p->m_idxCurrEdit >= 0)
				p->SaveEditingInfo(p->m_idxCurrEdit);
		}
		else if (lParam == (LPARAM)p->m_ComboBox.GetHWND() && HIWORD(wParam) == CBN_SELCHANGE)
		{
			if (p->m_idxCurrEdit >= 0)
				p->SaveEditingInfo(p->m_idxCurrEdit);
		}
	}
	break;

	case WM_LBUTTONDOWN:
	{
		LRESULT lResult = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		LVHITTESTINFO lvhti;
		lvhti.pt = ECK_GET_PT_LPARAM(lParam);
		p->HitTest(&lvhti);

		if (lvhti.iItem == p->m_idxCurrEdit)
			return lResult;

		ShowWindow(p->m_Edit, SW_HIDE);
		ShowWindow(p->m_ComboBox, SW_HIDE);
		ShowWindow(p->m_Button, SW_HIDE);
		if (lvhti.iItem < 0)
		{
			p->m_idxCurrEdit = -1;
			return lResult;
		}
		if (p->m_idxCurrEdit >= 0)
			p->SaveEditingInfo(p->m_idxCurrEdit);
		p->m_idxCurrEdit = lvhti.iItem;
		RECT rcItem;
		p->GetSubItemRect(lvhti.iItem, 1, &rcItem);
		rcItem.left += 1;
		rcItem.right -= rcItem.left;
		rcItem.bottom -= rcItem.top;

		auto& Item = p->m_Items[lvhti.iItem];
		switch (Item.uType)
		{
		case eck::ECPT::Text:
			p->m_Edit.Move(rcItem.left, rcItem.top, rcItem.right, rcItem.bottom);
			p->m_Edit.SetInputMode(eck::CEditExt::InputMode::Normal);
			p->m_Edit.SetText(std::get<1>(Item.Val).Data());
			ShowWindow(p->m_Edit, SW_SHOWNOACTIVATE);
			break;
		case eck::ECPT::Int:
			p->m_Edit.Move(rcItem.left, rcItem.top, rcItem.right, rcItem.bottom);
			p->m_Edit.SetInputMode(eck::CEditExt::InputMode::Int);
			p->m_Edit.SetText(std::to_wstring(std::get<0>(Item.Val).Vi).c_str());
			ShowWindow(p->m_Edit, SW_SHOWNOACTIVATE);
			break;
		case eck::ECPT::Float:
			p->m_Edit.Move(rcItem.left, rcItem.top, rcItem.right, rcItem.bottom);
			p->m_Edit.SetInputMode(eck::CEditExt::InputMode::Float);
			p->m_Edit.SetText(std::to_wstring(std::get<0>(Item.Val).Vf).c_str());
			ShowWindow(p->m_Edit, SW_SHOWNOACTIVATE);
			break;
		case eck::ECPT::Double:
			p->m_Edit.Move(rcItem.left, rcItem.top, rcItem.right, rcItem.bottom);
			p->m_Edit.SetInputMode(eck::CEditExt::InputMode::Double);
			p->m_Edit.SetText(std::to_wstring(std::get<0>(Item.Val).Vlf).c_str());
			ShowWindow(p->m_Edit, SW_SHOWNOACTIVATE);
			break;
		case eck::ECPT::Bool:
			p->m_ComboBox.Move(rcItem.left, rcItem.top, rcItem.right, rcItem.bottom);
			p->m_ComboBox.ResetContent();
			p->m_ComboBox.AddString(L"假");
			p->m_ComboBox.AddString(L"真");
			p->m_ComboBox.SetItemHeight(rcItem.bottom);
			p->m_ComboBox.SetCurSel(!!std::get<0>(Item.Val).Vb);
			ShowWindow(p->m_ComboBox, SW_SHOWNOACTIVATE);
			break;
		case eck::ECPT::DateTime:
			break;
		case eck::ECPT::PickInt:
			p->m_ComboBox.Move(rcItem.left, rcItem.top, rcItem.right, rcItem.bottom);
			p->m_ComboBox.ResetContent();
			EckCounter(Item.aPickStr.size(), i)
				p->m_ComboBox.AddString((eck::ToStr(i) + L". " + Item.aPickStr[i]).Data());
			p->m_ComboBox.SetItemHeight(rcItem.bottom);
			p->m_ComboBox.SetCurSel(std::get<0>(Item.Val).Vi);
			ShowWindow(p->m_ComboBox, SW_SHOWNOACTIVATE);
			break;
		case eck::ECPT::PickText:
			break;
		case eck::ECPT::Image:
		case eck::ECPT::Customize:
		case eck::ECPT::Color:
		case eck::ECPT::Font:
		case eck::ECPT::ImageList:
			rcItem.left = rcItem.left + rcItem.right - rcItem.bottom;
			rcItem.right = rcItem.bottom;
			p->m_Button.Move(rcItem.left, rcItem.top, rcItem.right, rcItem.bottom);
			ShowWindow(p->m_Button, SW_SHOWNOACTIVATE);
			break;
		}
		return lResult;
	}

	case WM_KILLFOCUS:
	{
		LRESULT lResult = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		if (p->m_idxCurrEdit >= 0 &&
			wParam != (WPARAM)p->m_Button.GetHWND() &&
			wParam != (WPARAM)p->m_Edit.GetHWND() &&
			wParam != (WPARAM)p->m_ComboBox.GetHWND())
		{
			p->SaveEditingInfo(p->m_idxCurrEdit);
			p->ExitEditing();
		}
		return lResult;
	}

	}

	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}