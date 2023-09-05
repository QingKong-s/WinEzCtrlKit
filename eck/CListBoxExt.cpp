#include "CListBoxExt.h"

ECK_NAMESPACE_BEGIN
SUBCLASS_MGR_INIT(CListBoxExt, SCID_LBEXT, SubclassProc)
SUBCLASS_REF_MGR_INIT(CListBoxExt, ObjRecorderRefPlaceholder, SCID_LBEXTPARENT, SubclassProc_Parent, ObjRecordRefStdDeleter)

void CListBoxExt::UpdateThemeInfo()
{
	CloseThemeData(m_hTheme);
	m_hTheme = OpenThemeData(m_hWnd, L"Button");
	HDC hDC = GetDC(m_hWnd);
	SIZE size;
	GetThemePartSize(m_hTheme, hDC, BP_CHECKBOX, CBS_CHECKEDNORMAL, NULL, TS_DRAW, &size);
	m_cxCheckBox = size.cx;
	ReleaseDC(m_hWnd, hDC);
}

void CListBoxExt::AddFile()
{
	if (!m_rsDir.Size())
		return;
	DeleteString(-1);

	PWSTR pszPath = (PWSTR)_malloca((
		m_rsDir.Size() +
		m_rsFilePattern.Size() +
		5/*一个反斜杠，三个*.*（通配符为空时用），一个结尾NULL*/) * sizeof(WCHAR));
	assert(pszPath);// 消除警告
	wcscpy(pszPath, m_rsDir);
	PWSTR pszTemp = pszPath + m_rsDir.Size();// 指针指到目录的后面，方便替换通配符

	PWSTR pszFilePattern;
	if (m_rsFilePattern.Size() && m_rsFilePattern)
		pszFilePattern = m_rsFilePattern;
	else
	{
#pragma warning(push)
#pragma warning(disable:6255)// 禁用警告：考虑改用_malloca
		pszFilePattern = (PWSTR)_alloca(4 * sizeof(WCHAR));
#pragma warning(pop)
		assert(pszFilePattern);// 消除警告
		wcscpy(pszFilePattern, L"*.*");
	}

	WIN32_FIND_DATAW wfd;
	HANDLE hFind;
	PWSTR pszDivPos, pszOld = pszFilePattern;
	while (TRUE)
	{
		pszDivPos = wcsstr(pszFilePattern, L"|");
		if (pszDivPos != pszOld && pszDivPos)// 常规情况
		{
			int cch = (int)(pszDivPos - pszOld - 1);
			wcscpy(pszTemp, L"\\");
			wcsncat(pszTemp, pszOld, cch);
			*(pszTemp + cch + 1) = L'\0';
		}
		else if (!pszDivPos)// 找不到下一个分隔符
		{
			wcscpy(pszTemp, L"\\");
			wcscat(pszTemp, pszOld);
		}
		else// 尾部（pszDivPos==pszOld）
			break;

		LBITEMCOMMINFO CommInfo{};
		pszOld = pszDivPos + 1;
		hFind = FindFirstFileW(pszPath, &wfd);
		if (hFind == INVALID_HANDLE_VALUE)
			if (!pszDivPos)
				break;
			else
				continue;
		do
		{
			if (memcmp(wfd.cFileName, L".", 2 * sizeof(WCHAR)) == 0 ||
				memcmp(wfd.cFileName, L"..", 3 * sizeof(WCHAR)) == 0)
				continue;
			if (!m_InfoEx.uFileAttr || wfd.dwFileAttributes & m_InfoEx.uFileAttr)
			{
				if (m_InfoEx.ftMaxTime == m_InfoEx.ftMinTime || IsFILETIMEZero(&m_InfoEx.ftMaxTime))
					AddString(wfd.cFileName, NULL, CommInfo);
				else if (wfd.ftCreationTime > m_InfoEx.ftMinTime && wfd.ftCreationTime < m_InfoEx.ftMaxTime)
					AddString(wfd.cFileName, NULL, CommInfo);
			}
		} while (FindNextFileW(hFind, &wfd));
		FindClose(hFind);
		if (!pszDivPos)
			break;
	}

	_freea(pszPath);
}

int CListBoxExt::HitTestCheckBox(POINT pt, int* pidxItem)
{
	POINT ptScr = pt;
	ClientToScreen(m_hWnd, &ptScr);
	int idx = LBItemFromPt(m_hWnd, ptScr, FALSE);
	if (pidxItem)
		*pidxItem = idx;
	if (idx < 0)
		return -1;

	RECT rcItem;
	if (SendMessageW(m_hWnd, LB_GETITEMRECT, idx, (LPARAM)&rcItem) == LB_ERR)
		return -1;
	rcItem.left += c_LBPadding;
	rcItem.right = rcItem.left + m_cxCheckBox;
	if (PtInRect(&rcItem, pt))
		return idx;
	else
		return -1;
}

LRESULT CALLBACK CListBoxExt::SubclassProc_Parent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
	UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	switch (uMsg)
	{
	case WM_MEASUREITEM:
	{
		HWND hCtrl = GetDlgItem(hWnd, (int)wParam);
		auto it = m_WndRecord.find(hCtrl);
		if (it == m_WndRecord.end())
			break;
		auto p = it->second;
		if (p->m_InfoEx.cyItem <= 0)
			break;
		auto pmis = (MEASUREITEMSTRUCT*)lParam;
		pmis->itemHeight = p->m_InfoEx.cyItem;
		return TRUE;
	}
	break;

	case WM_DRAWITEM:
	{
		auto pdis = (DRAWITEMSTRUCT*)lParam;
		auto it = m_WndRecord.find(pdis->hwndItem);
		if (it == m_WndRecord.end())
			break;
		auto p = it->second;
		if (pdis->itemID == -1)
			break;
		auto& Item = p->m_ItemsInfo[pdis->itemID];

		HDC hDC = pdis->hDC;
		// 画背景
		if (IsBitSet(pdis->itemState, ODS_SELECTED)/* && !Item.Info.bDisabled*/)
		{
			if (Item.hbrSelBK)
				FillRect(hDC, &pdis->rcItem, Item.hbrSelBK);
			else
				FillRect(hDC, &pdis->rcItem, p->m_hbrSelBK);

			if (Item.Info.crSelText != CLR_DEFAULT)
				SetTextColor(hDC, Item.Info.crSelText);
			else if (p->m_InfoEx.crSelText != CLR_DEFAULT)
				SetTextColor(hDC, p->m_InfoEx.crSelText);
			else
				SetTextColor(hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
		}
		else
		{
			if (Item.hbrBK)
				FillRect(hDC, &pdis->rcItem, Item.hbrBK);
			else
				FillRect(hDC, &pdis->rcItem, p->m_hbrBK);

			if (Item.Info.bDisabled)
				SetTextColor(hDC, GetSysColor(COLOR_GRAYTEXT));
			else if (Item.Info.crText != CLR_DEFAULT)
				SetTextColor(hDC, Item.Info.crText);
			else if (p->m_InfoEx.crText != CLR_DEFAULT)
				SetTextColor(hDC, p->m_InfoEx.crText);
			else
				SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));
		}
		// 画选择框
		RECT rc = pdis->rcItem;
		if (p->m_InfoEx.iCheckBoxMode)
		{
			rc.left += c_LBPadding;
			rc.right = rc.left + p->m_cxCheckBox;

			int iPartID;
			int iStateID;
			if (p->m_InfoEx.iCheckBoxMode == 1)
			{
				iPartID = BP_RADIOBUTTON;
				if (p->m_idxChecked == pdis->itemID)
					iStateID = Item.Info.bDisabled ? RBS_CHECKEDDISABLED : RBS_CHECKEDNORMAL;
				else
					iStateID = Item.Info.bDisabled ? RBS_UNCHECKEDDISABLED : RBS_UNCHECKEDNORMAL;
			}
			else
			{
				iPartID = BP_CHECKBOX;
				if (Item.Info.bChecked)
					iStateID = Item.Info.bDisabled ? CBS_CHECKEDDISABLED : CBS_CHECKEDNORMAL;
				else
					iStateID = Item.Info.bDisabled ? CBS_UNCHECKEDDISABLED : CBS_UNCHECKEDNORMAL;
			}
			DrawThemeBackground(p->m_hTheme, hDC, iPartID, iStateID, &rc, NULL);
			rc.right += c_LBPadding;
			rc.left = rc.right;
		}
		else
		{
			rc.left += c_LBPadding;
			rc.right = rc.left;
		}
		// 画图片
		if (p->m_cxImage)
		{
			if (p->m_hImageList && Item.Info.idxImage >= 0)
			{
				ImageList_Draw(p->m_hImageList, Item.Info.idxImage, hDC,
					rc.right,
					rc.top + ((rc.bottom - rc.top) - p->m_cyImage) / 2,
					ILD_NORMAL | (IsBitSet(pdis->itemState, ODS_SELECTED) ? ILD_SELECTED : 0));
			}
			rc.left += (p->m_cxImage + c_LBPadding);
		}
		rc.right = pdis->rcItem.right;
		// 画文本
		UINT uDTFlags = DT_NOCLIP | DT_SINGLELINE | (p->m_InfoEx.bEllipsis ? DT_END_ELLIPSIS : 0);
		switch (p->m_InfoEx.iAlignH)
		{
		case 0:uDTFlags |= DT_LEFT; break;
		case 1:uDTFlags |= DT_CENTER; break;
		case 2:uDTFlags |= DT_VCENTER; break;
		default:assert(FALSE); break;
		}

		switch (p->m_InfoEx.iAlignV)
		{
		case 0:uDTFlags |= DT_TOP; break;
		case 1:uDTFlags |= DT_VCENTER; break;
		case 2:uDTFlags |= DT_BOTTOM; break;
		default:assert(FALSE); break;
		}

		SetBkMode(hDC, TRANSPARENT);
		DrawTextW(hDC, Item.rsCaption, -1, &rc, uDTFlags);
		return TRUE;
	}
	break;

	case WM_DESTROY:
		m_SMRef.DeleteRecord(hWnd);
		break;
	}

	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK CListBoxExt::SubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
	UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	auto p = (CListBoxExt*)dwRefData;
	switch (uMsg)
	{
	case WM_LBUTTONDOWN:// 更新检查框
	{
		if (!p->m_InfoEx.iCheckBoxMode)
			break;

		POINT pt{ GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam) };
		int idxItem;
		int idx = p->HitTestCheckBox(pt, &idxItem);
		// 过滤禁止的项目
		if (idxItem < 0)
			break;
		if (p->m_ItemsInfo[idxItem].Info.bDisabled)
			return 0;
		// 更新检查状态
		if (idx < 0)
			break;
		if (idx >= 0)
			if (p->m_InfoEx.iCheckBoxMode == 1)
			{
				if (p->m_idxChecked != idx)
				{
					if (p->m_idxChecked >= 0)
						p->RedrawItem(p->m_idxChecked);
					p->m_idxChecked = idx;
					p->RedrawItem(idx);
				}
			}
			else
			{
				EckBoolNot(p->m_ItemsInfo[idx].Info.bChecked);
				p->RedrawItem(idx);
			}
	}
	break;

	case WM_KEYDOWN:
	{

	}
	break;

	case WM_LBUTTONDBLCLK:// 连击修复
	{
		POINT pt{ GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam) };
		int idx = p->HitTestCheckBox(pt);
		if (idx >= 0)
			PostMessageW(hWnd, WM_LBUTTONDOWN, wParam, lParam);
	}
	break;

	case WM_MOUSEMOVE:
	{
		if (!p->m_InfoEx.bToolTip)
			break;
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_HOVER | TME_LEAVE;
		tme.dwHoverTime = 400;
		tme.hwndTrack = hWnd;
		TrackMouseEvent(&tme);
	}
	break;

	case WM_MOUSEHOVER:
	{
		if (!p->m_InfoEx.bToolTip)
			break;
		SendMessageW(p->m_hToolTip, TTM_TRACKACTIVATE, FALSE, (LPARAM)&p->m_ti);
		POINT ptScr{ GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam) };
		ClientToScreen(hWnd, &ptScr);
		int idx = LBItemFromPt(hWnd, ptScr, FALSE);
		if (idx < 0)
			break;
		SendMessageW(p->m_hToolTip, TTM_GETTOOLINFOW, 0, (LPARAM)&p->m_ti);
		p->m_ti.lpszText = p->m_ItemsInfo[idx].rsTip;
		SendMessageW(p->m_hToolTip, TTM_SETTOOLINFOW, 0, (LPARAM)&p->m_ti);
		SendMessageW(p->m_hToolTip, TTM_TRACKPOSITION, 0, MAKELPARAM(ptScr.x, ptScr.y));
		SendMessageW(p->m_hToolTip, TTM_TRACKACTIVATE, TRUE, (LPARAM)&p->m_ti);
	}
	break;

	case WM_MOUSELEAVE:
		if (!p->m_InfoEx.bToolTip)
			break;
		SendMessageW(p->m_hToolTip, TTM_TRACKACTIVATE, FALSE, (LPARAM)&p->m_ti);
		break;

	case WM_THEMECHANGED:
		p->UpdateThemeInfo();
		break;

	case WM_DESTROY:
	{
		m_SMRef.DeleteRecord(p->m_hParent);
		m_SM.RemoveSubclass(hWnd);
	}
	break;
	}

	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

int CListBoxExt::InsertString(PCWSTR pszString, PCWSTR pszTip, const LBITEMCOMMINFO& CommInfo, int iPos)
{
	int idx = (int)SendMessageW(m_hWnd, LB_INSERTSTRING, iPos, (LPARAM)L"");
	if (idx == LB_ERR)
		return -1;
	LBITEMINFO Item{ pszString,pszTip,NULL,NULL,CommInfo };
	if (CommInfo.crBK != CLR_DEFAULT)
		Item.hbrBK = CreateSolidBrush(CommInfo.crBK);
	if (CommInfo.crSelBK != CLR_DEFAULT)
		Item.hbrSelBK = CreateSolidBrush(CommInfo.crSelBK);
	if (!m_InfoEx.bAutoSort)
		if (iPos < 0)
			m_ItemsInfo.push_back(std::move(Item));
		else
			m_ItemsInfo.insert(m_ItemsInfo.begin() + idx, std::move(Item));
	else
	{
		for (int i = 0; i < (int)m_ItemsInfo.size(); ++i)
		{
			if (wcscmp(m_ItemsInfo[i].rsCaption, pszString) > 0)
			{
				m_ItemsInfo.insert(m_ItemsInfo.begin() + i, std::move(Item));
				goto Ret;
			}
		}
		m_ItemsInfo.push_back(std::move(Item));
	}
Ret:
	return idx;
}

int CListBoxExt::AddString(PCWSTR pszString, PCWSTR pszTip, const LBITEMCOMMINFO& CommInfo)
{
	int idx = (int)SendMessageW(m_hWnd, LB_ADDSTRING, 0, (LPARAM)L"");
	if (idx == LB_ERR)
		return -1;
	LBITEMINFO Item{ pszString,pszTip,NULL,NULL,CommInfo };
	if (CommInfo.crBK != CLR_DEFAULT)
		Item.hbrBK = CreateSolidBrush(CommInfo.crBK);
	if (CommInfo.crSelBK != CLR_DEFAULT)
		Item.hbrSelBK = CreateSolidBrush(CommInfo.crSelBK);
	if (!m_InfoEx.bAutoSort)
		m_ItemsInfo.push_back(std::move(Item));
	else
	{
		for (int i = 0; i < (int)m_ItemsInfo.size(); ++i)
		{
			if (wcscmp(m_ItemsInfo[i].rsCaption, pszString) > 0)
			{
				m_ItemsInfo.insert(m_ItemsInfo.begin() + i, std::move(Item));
				goto Ret;
			}
		}
		m_ItemsInfo.push_back(std::move(Item));
	}
Ret:
	return idx;
}

void CListBoxExt::SetItem(int idx, PCWSTR pszString, PCWSTR pszTip, const LBITEMCOMMINFO& CommInfo)
{
	LBITEMINFO& Item = m_ItemsInfo[idx];
	Item = { pszString,pszTip,NULL,NULL,CommInfo };
	if (CommInfo.crBK != CLR_DEFAULT)
		Item.hbrBK = CreateSolidBrush(CommInfo.crBK);
	if (CommInfo.crSelBK != CLR_DEFAULT)
		Item.hbrSelBK = CreateSolidBrush(CommInfo.crSelBK);
}

BOOL CListBoxExt::DeleteString(int iPos)
{
	if (iPos < 0)
	{
		SendMessageW(m_hWnd, LB_RESETCONTENT, 0, 0);
		for (auto& x : m_ItemsInfo)
		{
			DeleteObject(x.hbrBK);
			DeleteObject(x.hbrSelBK);
		}
		m_ItemsInfo.resize(0u);
	}
	else if (iPos >= 0 && iPos < (int)m_ItemsInfo.size())
	{
		if (SendMessageW(m_hWnd, LB_DELETESTRING, iPos, 0) == LB_ERR)
			return FALSE;
		auto& Item = m_ItemsInfo[iPos];
		DeleteObject(Item.hbrBK);
		DeleteObject(Item.hbrSelBK);
		m_ItemsInfo.erase(m_ItemsInfo.begin() + iPos);
	}
	Redraw();
	return TRUE;
}

BOOL CListBoxExt::InitStorage(int cItems)
{
	if (SendMessageW(m_hWnd, LB_INITSTORAGE, cItems, 0) == LB_ERRSPACE)
		return FALSE;
	m_ItemsInfo.reserve(cItems);
	return TRUE;
}
ECK_NAMESPACE_END