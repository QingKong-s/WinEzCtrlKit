#include "DesginerCtrl.h"
LRESULT CALLBACK CSizerBlock::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	auto p = (CSizerBlock*)GetWindowLongPtrW(hWnd, 0);
	switch (uMsg)
	{
	case WM_SIZE:
		p->m_rcClient = { 0,0,LOWORD(lParam),HIWORD(lParam) };
		return 0;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		BeginPaint(hWnd, &ps);
		HGDIOBJ hOldBrush = SelectObject(ps.hdc, p->m_hbrBlock);
		Rectangle(ps.hdc, 0, 0, p->m_rcClient.right, p->m_rcClient.bottom);
		SelectObject(ps.hdc, hOldBrush);
		EndPaint(hWnd, &ps);
	}
	return 0;

	case WM_SETCURSOR:
		SetCursor(p->m_hCursor);
		return 0;

	case WM_MOUSEMOVE:
		p->m_pSizer->OnBlockMouseMove(p, lParam);
		return 0;

	case WM_LBUTTONDOWN:
		p->m_pSizer->OnBlockLButtonDown(p, lParam);
		return 0;

	case WM_LBUTTONUP:
		p->m_pSizer->OnBlockLButtonUp(p, lParam);
		return 0;
	}

	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

HWND CSizerBlock::Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
	int x, int y, int cx, int cy, HWND hParent, UINT nID)
{
	m_hWnd = CreateWindowExW(dwExStyle, WCN_BK, pszText, dwStyle,
		x, y, cx, cy, hParent, eck::i32ToP<HMENU>(nID), eck::g_hInstance, NULL);

	m_hbrBlock = CreateSolidBrush(eck::Colorref::Teal);

	m_rcClient = { 0,0,cx,cy };

	SetWindowLongPtrW(m_hWnd, 0, (LONG_PTR)this);
	SetWindowLongPtrW(m_hWnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
	return m_hWnd;
}

void CSizerBlock::BindSizer(CSizer* pSizer, SizerHTCode uType)
{
	m_pSizer = pSizer;
	m_uType = uType;
	switch (uType)
	{
	case SizerHTCode::LeftTop:
	case SizerHTCode::RightBottom:
		m_hCursor = LoadCursorW(NULL, IDC_SIZENWSE);
		break;
	case SizerHTCode::RightTop:
	case SizerHTCode::LeftBottom:
		m_hCursor = LoadCursorW(NULL, IDC_SIZENESW);
		break;
	case SizerHTCode::Left:
	case SizerHTCode::Right:
		m_hCursor = LoadCursorW(NULL, IDC_SIZEWE);
		break;
	case SizerHTCode::Top:
	case SizerHTCode::Bottom:
		m_hCursor = LoadCursorW(NULL, IDC_SIZENS);
		break;
	}
}



void CSizer::Create(HWND hBK, HWND hBottomWorkWnd)
{
	m_hBK = hBK;
	m_hBottomWorkWnd = hBottomWorkWnd;
	constexpr DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	constexpr DWORD dwExStyle = WS_EX_TOPMOST;
	m_Block[0].Create(NULL, dwStyle, dwExStyle, 0, 0, m_sizeBlock, m_sizeBlock, hBK, 30000);
	m_Block[0].BindSizer(this, SizerHTCode::LeftTop);
	m_Block[1].Create(NULL, dwStyle, dwExStyle, 0, 0, m_sizeBlock, m_sizeBlock, hBK, 30001);
	m_Block[1].BindSizer(this, SizerHTCode::Top);
	m_Block[2].Create(NULL, dwStyle, dwExStyle, 0, 0, m_sizeBlock, m_sizeBlock, hBK, 30002);
	m_Block[2].BindSizer(this, SizerHTCode::RightTop);
	m_Block[3].Create(NULL, dwStyle, dwExStyle, 0, 0, m_sizeBlock, m_sizeBlock, hBK, 30003);
	m_Block[3].BindSizer(this, SizerHTCode::Left);
	m_Block[4].Create(NULL, dwStyle, dwExStyle, 0, 0, m_sizeBlock, m_sizeBlock, hBK, 30004);
	m_Block[4].BindSizer(this, SizerHTCode::Right);
	m_Block[5].Create(NULL, dwStyle, dwExStyle, 0, 0, m_sizeBlock, m_sizeBlock, hBK, 30005);
	m_Block[5].BindSizer(this, SizerHTCode::LeftBottom);
	m_Block[6].Create(NULL, dwStyle, dwExStyle, 0, 0, m_sizeBlock, m_sizeBlock, hBK, 30006);
	m_Block[6].BindSizer(this, SizerHTCode::Bottom);
	m_Block[7].Create(NULL, dwStyle, dwExStyle, 0, 0, m_sizeBlock, m_sizeBlock, hBK, 30007);
	m_Block[7].BindSizer(this, SizerHTCode::RightBottom);

	m_hPen = CreatePen(PS_SOLID, 2, eck::Colorref::Red);
}

HWND CSizer::SetTargetWindow(HWND hWnd)
{
	HWND hOld = m_hWorkWnd;
	m_hWorkWnd = hWnd;
	m_hWorkWndParent = GetParent(hWnd);
	MoveToTargetWindow();
	return hOld;
}

SizerHTCode CSizer::HitTest(POINT pt)
{
	HWND hWnd = ChildWindowFromPoint(m_hBK, pt);
	for (auto& x : m_Block)
	{
		if (x.m_hWnd == hWnd)
			return x.m_uType;
	}
	return SizerHTCode::None;
}

void CSizer::MoveToTargetWindow()
{
	RECT rc;
	GetWindowRect(m_hWorkWnd, &rc);
	eck::ScreenToClient(m_hBK, &rc);

	SetWindowPos(m_Block[0], NULL,
		rc.left - m_sizeBlock,
		rc.top - m_sizeBlock,
		0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	SetWindowPos(m_Block[1], NULL,
		rc.left + (rc.right - rc.left - m_sizeBlock) / 2,
		rc.top - m_sizeBlock,
		0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	SetWindowPos(m_Block[2], NULL,
		rc.right,
		rc.top - m_sizeBlock,
		0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	SetWindowPos(m_Block[3], NULL,
		rc.left - m_sizeBlock,
		rc.top + (rc.bottom - rc.top - m_sizeBlock) / 2,
		0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	SetWindowPos(m_Block[4], NULL,
		rc.right,
		rc.top + (rc.bottom - rc.top - m_sizeBlock) / 2,
		0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	SetWindowPos(m_Block[5], NULL,
		rc.left - m_sizeBlock,
		rc.bottom,
		0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	SetWindowPos(m_Block[6], NULL,
		rc.left + (rc.right - rc.left - m_sizeBlock) / 2,
		rc.bottom,
		0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	SetWindowPos(m_Block[7], NULL,
		rc.right,
		rc.bottom,
		0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);

	for (auto& x : m_Block)
		SetWindowPos(x, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	SetWindowPos(m_hBottomWorkWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
}

RECT CSizer::SizerMakeRect(POINT ptCursor, SizerHTCode uType)
{
	RECT rc;
	switch (uType)
	{
	case SizerHTCode::LeftTop:
		rc.left = ptCursor.x;
		rc.top = ptCursor.y;
		rc.right = m_rcOrg.right;
		rc.bottom = m_rcOrg.bottom;
		break;
	case SizerHTCode::RightTop:
		rc.left = m_rcOrg.left;
		rc.top = ptCursor.y;
		rc.right = ptCursor.x;
		rc.bottom = m_rcOrg.bottom;
		break;
	case SizerHTCode::LeftBottom:
		rc.left = ptCursor.x;
		rc.top = m_rcOrg.top;
		rc.right = m_rcOrg.right;
		rc.bottom = ptCursor.y;
		break;
	case SizerHTCode::RightBottom:
		rc.left = m_rcOrg.left;
		rc.top = m_rcOrg.top;
		rc.right = ptCursor.x;
		rc.bottom = ptCursor.y;
		break;
	case SizerHTCode::Top:
		rc.left = m_rcOrg.left;
		rc.top = ptCursor.y;
		rc.right = m_rcOrg.right;
		rc.bottom = m_rcOrg.bottom;
		break;
	case SizerHTCode::Bottom:
		rc.left = m_rcOrg.left;
		rc.top = m_rcOrg.top;
		rc.right = m_rcOrg.right;
		rc.bottom = ptCursor.y;
		break;
	case SizerHTCode::Left:
		rc.left = ptCursor.x;
		rc.top = m_rcOrg.top;
		rc.right = m_rcOrg.right;
		rc.bottom = m_rcOrg.bottom;
		break;
	case SizerHTCode::Right:
		rc.left = m_rcOrg.left;
		rc.top = m_rcOrg.top;
		rc.right = ptCursor.x;
		rc.bottom = m_rcOrg.bottom;
		break;
	}

	return rc;
}

void CSizer::OnBlockLButtonDown(CSizerBlock* pBlock, LPARAM lParam)
{
	SetCapture(pBlock->m_hWnd);
	m_bLBtnDown = TRUE;
	m_hDC = GetDC(m_hBK);
	SetROP2(m_hDC, R2_NOTXORPEN);
	m_hOld = SelectObject(m_hDC, m_hPen);

	GetWindowRect(m_hWorkWnd, &m_rcOrg);
	eck::ScreenToClient(m_hBK, &m_rcOrg);
}

void CSizer::OnBlockLButtonUp(CSizerBlock* pBlock, LPARAM lParam)
{
	if (!m_bLBtnDown)
		return;
	ReleaseCapture();
	m_bLBtnDown = FALSE;
	m_rcLast = {};
	SelectObject(m_hDC, m_hOld);
	ReleaseDC(pBlock->m_hWnd, m_hDC);

	POINT pt = GET_PT_LPARAM(lParam);
	MapWindowPoints(pBlock->m_hWnd, m_hBK, &pt, 1);
	RECT rc = SizerMakeRect(pt, pBlock->m_uType);
	MapWindowRect(m_hBK, m_hWorkWndParent, &rc);
	rc.right -= rc.left;
	rc.bottom -= rc.top;
	SetWindowPos(m_hWorkWnd, NULL, rc.left, rc.top, rc.right, rc.bottom, SWP_NOZORDER | SWP_NOACTIVATE);
	MoveToTargetWindow();

	InvalidateRect(m_hBK, NULL, FALSE);
	UpdateWindow(m_hBK);
}

void CSizer::OnBlockMouseMove(CSizerBlock* pBlock, LPARAM lParam)
{
	if (!m_bLBtnDown)
		return;
	POINT pt = GET_PT_LPARAM(lParam);
	MapWindowPoints(pBlock->m_hWnd, m_hBK, &pt, 1);

	RECT rc = SizerMakeRect(pt, pBlock->m_uType);

	if (!IsRectEmpty(&m_rcLast))
		Rectangle(m_hDC, m_rcLast.left, m_rcLast.top, m_rcLast.right, m_rcLast.bottom);
	m_rcLast = rc;
	Rectangle(m_hDC, rc.left, rc.top, rc.right, rc.bottom);
}






#define SCID_PROPLIST 20230808'02

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
				DrawTextW(hDC, Item.rsText, Item.rsText.m_cchText, &rcHItem,
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
					pszVal = std::get<1>(Item.Val);
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
						sVal = std::to_wstring(Union.Vi) + L". " + Item.aPickStr[Union.Vi].m_pszText;
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

			ShowWindow(p->m_Edit, SW_HIDE);
			ShowWindow(p->m_ComboBox, SW_HIDE);
			ShowWindow(p->m_Button, SW_HIDE);
			if (pnmlv->iItem < 0 &&
				eck::IsBitSet(pnmlv->uOldState, LVIS_SELECTED) && 
				!eck::IsBitSet(pnmlv->uNewState, LVIS_SELECTED))
			{
				p->m_idxCurrEdit = -1;
				break;
			}
			if (p->m_idxCurrEdit >= 0)
				p->SaveEditingInfo(p->m_idxCurrEdit);
			p->m_idxCurrEdit = pnmlv->iItem;
			RECT rcItem;
			p->GetSubItemRect(pnmlv->iItem, 1, &rcItem);
			rcItem.left += 1;
			rcItem.right -= rcItem.left;
			rcItem.bottom -= rcItem.top;

			auto& Item = p->m_Items[pnmlv->iItem];
			switch (Item.uType)
			{
			case eck::ECPT::Text:
				p->m_Edit.Move(rcItem.left, rcItem.top, rcItem.right, rcItem.bottom);
				p->m_Edit.SetInputMode(eck::CEdit::InputMode::Normal);
				p->m_Edit.SetText(std::get<1>(Item.Val).m_pszText);
				ShowWindow(p->m_Edit, SW_SHOWNOACTIVATE);
				break;
			case eck::ECPT::Int:
				p->m_Edit.Move(rcItem.left, rcItem.top, rcItem.right, rcItem.bottom);
				p->m_Edit.SetInputMode(eck::CEdit::InputMode::Int);
				p->m_Edit.SetText(std::to_wstring(std::get<0>(Item.Val).Vi).c_str());
				ShowWindow(p->m_Edit, SW_SHOWNOACTIVATE);
				break;
			case eck::ECPT::Float:
				p->m_Edit.Move(rcItem.left, rcItem.top, rcItem.right, rcItem.bottom);
				p->m_Edit.SetInputMode(eck::CEdit::InputMode::Float);
				p->m_Edit.SetText(std::to_wstring(std::get<0>(Item.Val).Vf).c_str());
				ShowWindow(p->m_Edit, SW_SHOWNOACTIVATE);
				break;
			case eck::ECPT::Double:
				p->m_Edit.Move(rcItem.left, rcItem.top, rcItem.right, rcItem.bottom);
				p->m_Edit.SetInputMode(eck::CEdit::InputMode::Double);
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
					p->m_ComboBox.AddString(eck::ToStr(i) + L". " + Item.aPickStr[i]);
				p->m_ComboBox.SetItemHeight(rcItem.bottom);
				p->m_ComboBox.SetCurSel(!!std::get<0>(Item.Val).Vi);
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
	}
	break;

	case WM_KILLFOCUS:
	{
		LRESULT lResult = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		if (p->m_idxCurrEdit >= 0)
			p->SaveEditingInfo(p->m_idxCurrEdit);
		return lResult;
	}

	}

	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}