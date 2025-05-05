#include "pch.h"
#include "CWndMain.h"
#include "CApp.h"


void CWndMain::OnCreate(HWND hWnd)
{
	eck::GetThreadCtx()->UpdateDefColor();
	m_iDpi = eck::GetDpi(hWnd);
	m_hFontComm = eck::CreateDefFont(m_iDpi);
	eck::UpdateDpiSize(m_Ds, m_iDpi);
	UpdateDpi();

	InitMenu();
	eck::UxfMenuInit(this);

	ComPtr<IStream> pStream;
	SHCreateStreamOnFileEx(LR"(E:\Desktop\1.il)",
		STGM_READ, 0, FALSE, nullptr, &pStream);
	ImageList_ReadEx(ILP_NORMAL, pStream.Get(), IID_PPV_ARGS(&m_pilMain));
	m_pilMain->GetIconSize(&m_cxIlMain, &m_cyIlMain);

	constexpr DWORD dwCommStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP;
	const MARGINS Mar{ .cxRightWidth = m_Ds.IntPad };
	{
		const MARGINS Mar{ .cyBottomHeight = m_Ds.IntPad };
		m_CBBCtrl.Create(nullptr, dwCommStyle, 0,
			0, 0, 0, m_Ds.cyComboBox, hWnd, 0);
		m_LytProp.Add(&m_CBBCtrl, Mar, eck::LF_FIX_HEIGHT | eck::LF_FILL_WIDTH);

		m_PLProp.Create(nullptr, dwCommStyle | WS_BORDER, 0,
			0, 0, m_Ds.cxDefProp, 0, hWnd, 0);
		m_LytProp.Add(&m_PLProp, Mar, eck::LF_FILL, 1u);

		m_EDDesc.SetMultiLine(TRUE);
		m_EDDesc.Create(nullptr, dwCommStyle | ES_AUTOVSCROLL, 0,
			0, 0, 0, m_Ds.cyDescEdit, hWnd, 0);
		m_EDDesc.SetFrameType(3);
		m_LytProp.Add(&m_EDDesc, {}, eck::LF_FIX_HEIGHT | eck::LF_FILL_WIDTH);
	}
	m_Lyt.Add(&m_LytProp, Mar, eck::LF_FILL_HEIGHT | eck::LF_FIX_WIDTH);

	m_Tab.Create(nullptr, dwCommStyle, 0,
		0, 0, 0, 0, hWnd, 0);
	m_Lyt.Add(&m_Tab, Mar, eck::LF_FILL, 1u);

	m_LBCtrl.Create(nullptr, dwCommStyle | WS_BORDER, 0,
		0, 0, m_Ds.cxDefCtrlBox, 0, hWnd, 0);
	m_LBCtrl.SetItemCount(ARRAYSIZE(BuiltInCtrls) + 1);
	m_Lyt.Add(&m_LBCtrl, Mar, eck::LF_FILL_HEIGHT | eck::LF_FIX_WIDTH);

	m_Lyt.LoInitDpi(m_iDpi);

	eck::SetFontForWndAndCtrl(hWnd, m_hFontComm);
}

void CWndMain::InitMenu()
{
	m_MenuFile = {
		{ L"新建(&N)\tCtrl+N",CWndMain::IDMI_FILE_NEW },
		{ L"打开(&O)\tCtrl+O",CWndMain::IDMI_FILE_OPEN },
		{ L"保存(&S)\tCtrl+S",CWndMain::IDMI_FILE_SAVE },
		{ L"另存为(&A)",CWndMain::IDMI_FILE_SAVEAS },
		{ nullptr,0,MF_SEPARATOR },
		{ L"退出(&X)",CWndMain::IDMI_FILE_EXIT },
	};
	m_MenuEdit = {
		{ L"撤销(&U)\tCtrl+Z",CWndMain::IDMI_EDIT_UNDO },
		{ L"重做(&R)\tCtrl+Y",CWndMain::IDMI_EDIT_REDO },
		{ nullptr,0,MF_SEPARATOR },
		{ L"剪切(&T)\tCtrl+X",CWndMain::IDMI_EDIT_CUT },
		{ L"复制(&C)\tCtrl+C",CWndMain::IDMI_EDIT_COPY },
		{ L"粘贴(&P)\tCtrl+V",CWndMain::IDMI_EDIT_PASTE },
		{ L"删除(&L)\tDel",CWndMain::IDMI_EDIT_DELETE },
		{ nullptr,0,MF_SEPARATOR },
		{ L"全选(&A)\tCtrl+A",CWndMain::IDMI_EDIT_SELECTALL },
	};
	m_MenuInsert = {
		{ L"窗体(&F)",CWndMain::IDMI_INSERT_FORM },
	};
	m_MenuBarMain.Create();
	m_MenuBarMain.AppendItem(L"文件(&F)", m_MenuFile.GetHMenu());
	m_MenuBarMain.AppendItem(L"编辑(&E)", m_MenuEdit.GetHMenu());
	m_MenuBarMain.AppendItem(L"插入(&I)", m_MenuInsert.GetHMenu());
	SetMenu(HWnd, m_MenuBarMain.GetHMenu());
}

LRESULT CWndMain::OnCustomDrawCtrlListBox(const eck::NMCUSTOMDRAWEXT* p)
{
	switch (p->dwDrawStage)
	{
	case CDDS_PREPAINT:
		return CDRF_NOTIFYITEMDRAW;
	case CDDS_ITEMPREPAINT:
		return CDRF_NOTIFYPOSTPAINT;
	case CDDS_ITEMPOSTPAINT:
	{
		std::wstring_view svText;
		IMAGELISTDRAWPARAMS ildp{ sizeof(IMAGELISTDRAWPARAMS) };
		if (p->dwItemSpec)
		{
			ildp.i = BuiltInCtrls[p->dwItemSpec - 1].idxIcon;
			svText = BuiltInCtrls[p->dwItemSpec - 1].svDisplayName;
		}
		else
		{
			ildp.i = 60;
			svText = L"指针"sv;
		}
		ildp.hdcDst = p->hdc;
		const auto cxPad = eck::DaGetSystemMetrics(SM_CXEDGE, m_iDpi);
		ildp.x = cxPad;
		ildp.y = p->rc.top + (p->rc.bottom - p->rc.top - m_cyIlMain) / 2;
		ildp.rgbBk = CLR_NONE;
		m_pilMain->Draw(&ildp);

		RECT rcText{ p->rc };
		rcText.left += (ildp.x + m_cxIlMain + cxPad);
		rcText.right -= cxPad;
		SetTextColor(p->hdc, p->crText);
		DrawTextW(p->hdc, svText.data(), (int)svText.size(),
			&rcText, DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
	}
	return CDRF_DODEFAULT;
	}
	return CDRF_DODEFAULT;
}

LRESULT CWndMain::OnCommand(HWND hWnd, int nId, HWND hCtrl, UINT uNotifyCode)
{
	if (!hCtrl && !uNotifyCode)// 菜单
		switch (nId)
		{
		case IDMI_FILE_NEW:
			break;
		case IDMI_FILE_OPEN:
			break;
		case IDMI_FILE_SAVE:
			break;
		case IDMI_FILE_SAVEAS:
			break;
		case IDMI_FILE_EXIT:
			PostQuitMessage(0);
			break;
		case IDMI_EDIT_UNDO:
			break;
		case IDMI_EDIT_REDO:
			break;
		case IDMI_EDIT_CUT:
			break;
		case IDMI_EDIT_COPY:
			break;
		case IDMI_EDIT_PASTE:
			break;
		case IDMI_EDIT_DELETE:
			break;
		case IDMI_EDIT_SELECTALL:
			break;
		case IDMI_INSERT_FORM:
			break;
		}
}

LRESULT CWndMain::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_SIZE:
	{
		const auto cx = LOWORD(lParam);
		const auto cy = HIWORD(lParam);
		m_Lyt.Arrange(m_Ds.OutPad, m_Ds.OutPad,
			cx - m_Ds.OutPad * 2, cy - m_Ds.OutPad * 2);
	}
	return 0;
	case WM_CREATE:
		OnCreate(hWnd);
		return 0;

	case WM_NOTIFY:
	{
		const auto* const pnmhdr = (NMHDR*)lParam;
		if (pnmhdr->hwndFrom == m_LBCtrl.HWnd)
			switch (pnmhdr->code)
			{
			case NM_CUSTOMDRAW:
			{
				const auto* const p = (eck::NMCUSTOMDRAWEXT*)lParam;
				return OnCustomDrawCtrlListBox(p);
			}
			return CDRF_DODEFAULT;
			}
		else if (pnmhdr->hwndFrom == m_Tab.HWnd)
			switch (pnmhdr->code)
			{
			case TCN_SELCHANGE:
			{
				//	if (pnmhdr->hwndFrom == m_Tab.HWnd)
				//	{
				//		for (auto x : m_vTabs)
				//			ShowWindow(x->pBK->GetHWND(), SW_HIDE);
				//		ShowWindow(m_vTabs[m_Tab.GetCurSel()]->pBK->GetHWND(), SW_SHOWNOACTIVATE);
				//return 0;
				//	}
			}
			break;
			}
	}
	break;
	case WM_COMMAND:
		return OnCommand(hWnd, LOWORD(wParam), (HWND)lParam, HIWORD(wParam));

	case WM_DPICHANGED:
		eck::MsgOnDpiChanged(hWnd, lParam);
		return 0;
	case WM_SETTINGCHANGE:
		if (eck::MsgOnSettingChangeMainWnd(hWnd, wParam, lParam))
			return 0;
		else
			eck::BroadcastChildrenMessage(hWnd, uMsg, wParam, lParam);
		return 0;
	}

	return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}