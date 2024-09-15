#include "pch.h"

LRESULT CPropList::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DPICHANGED_BEFOREPARENT:
		UpdateDpiSize(LOWORD(wParam));
		break;

	case WM_COMMAND:
	{
		if (lParam == (LPARAM)m_Button.GetHWND() && HIWORD(wParam) == BN_CLICKED)
		{
			if (m_idxCurrEdit < 0)
				return 0;
			auto& Item = m_Items[m_idxCurrEdit];
			//switch (Item.uType)
			//{
			//case eck::ECPT::Image:
			//case eck::ECPT::Customize:
			//case eck::ECPT::Color:
			//case eck::ECPT::Font:
			//case eck::ECPT::ImageList:
			//}
		}
		else if (lParam == (LPARAM)m_Edit.GetHWND() && HIWORD(wParam) == NM_RETURN)
		{
			if (m_idxCurrEdit >= 0)
				SaveEditingInfo(m_idxCurrEdit);
		}
		else if (lParam == (LPARAM)m_ComboBox.GetHWND() && HIWORD(wParam) == CBN_SELCHANGE)
		{
			if (m_idxCurrEdit >= 0)
				SaveEditingInfo(m_idxCurrEdit);
		}
	}
	break;

	case WM_LBUTTONDOWN:
	{
		LRESULT lResult = __super::OnMsg(hWnd, uMsg, wParam, lParam);
		LVHITTESTINFO lvhti;
		lvhti.pt = ECK_GET_PT_LPARAM(lParam);
		HitTest(&lvhti);

		if (lvhti.iItem == m_idxCurrEdit)
			return lResult;

		m_Edit.Show(SW_HIDE);
		m_ComboBox.Show(SW_HIDE);
		m_Button.Show(SW_HIDE);
		if (lvhti.iItem < 0)
		{
			m_idxCurrEdit = -1;
			return lResult;
		}
		if (m_idxCurrEdit >= 0)
			SaveEditingInfo(m_idxCurrEdit);
		m_idxCurrEdit = lvhti.iItem;
		RECT rcItem;
		GetSubItemRect(lvhti.iItem, 1, &rcItem);
		rcItem.left += 1;
		rcItem.right -= rcItem.left;
		rcItem.bottom -= rcItem.top;

		auto& Item = m_Items[lvhti.iItem];
		switch (Item.uType)
		{
		case eck::ECPT::Text:
			m_Edit.Move(rcItem.left, rcItem.top, rcItem.right, rcItem.bottom);
			m_Edit.SetInputMode(eck::CEditExt::InputMode::Normal);
			m_Edit.SetText(std::get<1>(Item.Val).Data());
			m_Edit.Show(SW_SHOWNOACTIVATE);
			break;
		case eck::ECPT::Int:
			m_Edit.Move(rcItem.left, rcItem.top, rcItem.right, rcItem.bottom);
			m_Edit.SetInputMode(eck::CEditExt::InputMode::Int);
			m_Edit.SetText(std::to_wstring(std::get<0>(Item.Val).Vi).c_str());
			m_Edit.Show(SW_SHOWNOACTIVATE);
			break;
		case eck::ECPT::Float:
			m_Edit.Move(rcItem.left, rcItem.top, rcItem.right, rcItem.bottom);
			m_Edit.SetInputMode(eck::CEditExt::InputMode::Float);
			m_Edit.SetText(std::to_wstring(std::get<0>(Item.Val).Vf).c_str());
			m_Edit.Show(SW_SHOWNOACTIVATE);
			break;
		case eck::ECPT::Double:
			m_Edit.Move(rcItem.left, rcItem.top, rcItem.right, rcItem.bottom);
			m_Edit.SetInputMode(eck::CEditExt::InputMode::Double);
			m_Edit.SetText(std::to_wstring(std::get<0>(Item.Val).Vlf).c_str());
			m_Edit.Show(SW_SHOWNOACTIVATE);
			break;
		case eck::ECPT::Bool:
			m_ComboBox.Move(rcItem.left, rcItem.top, rcItem.right, rcItem.bottom);
			m_ComboBox.ResetContent();
			m_ComboBox.AddString(L"假");
			m_ComboBox.AddString(L"真");
			m_ComboBox.SetItemHeight(rcItem.bottom);
			m_ComboBox.SetCurSel(!!std::get<0>(Item.Val).Vb);
			m_ComboBox.Show(SW_SHOWNOACTIVATE);
			break;
		case eck::ECPT::DateTime:
			break;
		case eck::ECPT::PickInt:
			m_ComboBox.Move(rcItem.left, rcItem.top, rcItem.right, rcItem.bottom);
			m_ComboBox.ResetContent();
			EckCounter(Item.aPickStr.size(), i)
				m_ComboBox.AddString((eck::ToStr(i) + L". " + Item.aPickStr[i]).Data());
			m_ComboBox.SetItemHeight(rcItem.bottom);
			m_ComboBox.SetCurSel(std::get<0>(Item.Val).Vi);
			m_ComboBox.Show(SW_SHOWNOACTIVATE);
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
			m_Button.Move(rcItem.left, rcItem.top, rcItem.right, rcItem.bottom);
			m_Button.Show(SW_SHOWNOACTIVATE);
			break;
		}
		return lResult;
	}

	case WM_KILLFOCUS:
	{
		LRESULT lResult = __super::OnMsg(hWnd, uMsg, wParam, lParam);
		if (m_idxCurrEdit >= 0 &&
			wParam != (WPARAM)m_Button.GetHWND() &&
			wParam != (WPARAM)m_Edit.GetHWND() &&
			wParam != (WPARAM)m_ComboBox.GetHWND())
		{
			SaveEditingInfo(m_idxCurrEdit);
			ExitEditing();
		}
		return lResult;
	}

	}

	return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}

LRESULT CPropList::OnNotifyMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bProcessed)
{
	switch (uMsg)
	{
	case WM_NOTIFY:
	{
		const auto pnmhdr = (NMHDR*)lParam;
		switch (pnmhdr->code)
		{
		case NM_CUSTOMDRAW:
		{
			bProcessed = TRUE;
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
				auto Header = GetHeaderCtrl();
				auto& Item = m_Items[idx];

				Header.GetItemRect(0, &rcHItem);
				rcHItem.top = pnmlvcd->nmcd.rc.top;
				rcHItem.bottom = pnmlvcd->nmcd.rc.bottom;

				if (GetItemState(idx, LVIS_SELECTED) == LVIS_SELECTED)
				{
					FillRect(hDC, &rcHItem, GetSysColorBrush(COLOR_HIGHLIGHT));
					::SetTextColor(hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
				}
				else
				{
					FillRect(hDC, &rcHItem, GetSysColorBrush(COLOR_WINDOW));
					::SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));
				}

				rcHItem.left += m_cxPadding;
				DrawTextW(hDC, Item.rsText.Data(), Item.rsText.Size(), &rcHItem,
					DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);


				Header.GetItemRect(1, &rcHItem);
				rcHItem.top = pnmlvcd->nmcd.rc.top;
				rcHItem.bottom = pnmlvcd->nmcd.rc.bottom;

				FillRect(hDC, &rcHItem, GetSysColorBrush(COLOR_WINDOW));

				PCWSTR pszVal = nullptr;

				std::wstring sVal;
				switch (Item.uType)
				{
				case eck::ECPT::Text:
					pszVal = std::get<1>(Item.Val).Data();
					break;
				case eck::ECPT::Image:
				case eck::ECPT::Customize:
					if (std::get<2>(Item.Val).Size())
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
				rcHItem.left += m_cxPadding;
				::SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));
				DrawTextW(hDC, pszVal, -1, &rcHItem, DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
			}
			return CDRF_SKIPDEFAULT;
			}
		}
		return CDRF_SKIPDEFAULT;

		case LVN_ITEMCHANGED:
		{
			auto pnmlv = (NMLISTVIEW*)lParam;


		}
		break;
		}
	}
	break;
	}

	return __super::OnNotifyMsg(hWnd, uMsg, wParam, lParam, bProcessed);
}