#include "CDirBox.h"

#include <utility>

ECK_NAMESPACE_BEGIN
SUBCLASS_MGR_INIT(CDirBox, SCID_DIRBOX, CDirBox::SubclassProc)
SUBCLASS_REF_MGR_INIT(CDirBox, CDirBox::PARENTSCCTX*, SCID_DIRBOXPARENT, CDirBox::SubclassProc_Parent, ObjRecordRefStdDeleter)


HTREEITEM CDirBox::InsertItem(PCWSTR pszText, PWSTR pszNextLevelPath, int cchNextLevelPath,
	HTREEITEM hParentItem, HTREEITEM hItemAfter, BOOL* pbHasChildPath)
{
	if (pbHasChildPath)
		*pbHasChildPath = FALSE;
	PWSTR pTemp = pszNextLevelPath + cchNextLevelPath;

	TVINSERTSTRUCTW tis;
	SHFILEINFOW sfi{};
	WIN32_FIND_DATAW wfd;
	HANDLE hFind;
	HTREEITEM hNewItem = NULL;

	if (!SHGetFileInfoW(pszNextLevelPath, 0, &sfi, sizeof(SHFILEINFOW), SHGFI_SYSICONINDEX | SHGFI_SMALLICON))
		return NULL;
	tis.hParent = hParentItem;
	tis.hInsertAfter = hItemAfter;
	tis.itemex.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
	tis.itemex.iSelectedImage = tis.itemex.iImage = sfi.iIcon;
	tis.itemex.pszText = (PWSTR)pszText;
	hNewItem = (HTREEITEM)SendMessageW(m_hWnd, TVM_INSERTITEMW, 0, (LPARAM)&tis);

	wcscat(pTemp, L"\\*");
	hFind = FindFirstFileW(pszNextLevelPath, &wfd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (memcmp(wfd.cFileName, L".", 2 * sizeof(WCHAR)) == 0 ||
				memcmp(wfd.cFileName, L"..", 3 * sizeof(WCHAR)) == 0)
				continue;

			tis.hParent = hNewItem;
			tis.hInsertAfter = TVI_FIRST;
			tis.itemex.mask = TVIF_TEXT | TVIF_PARAM;
			tis.itemex.pszText = (PWSTR)L"0";
			tis.itemex.lParam = ECKDBITEMFLAG_ISHIDEITEM;
			SendMessageW(m_hWnd, TVM_INSERTITEMW, 0, (LPARAM)&tis);

			if (pbHasChildPath)
				*pbHasChildPath = TRUE;
			break;
		} while (FindNextFileW(hFind, &wfd));
		FindClose(hFind);
	}

	return hNewItem;
}

void CDirBox::EnumFile(PCWSTR pszFile, HTREEITEM hParentItem)
{
	int cchFile = (int)wcslen(pszFile);
	if (!cchFile)
		return;
	WCHAR pszPath[MAX_PATH];
	wcscpy(pszPath, pszFile);
	PWSTR pszPathTemp = pszPath + cchFile;
	if (*(pszPathTemp - 1) != L'\\')
	{
		wcscpy(pszPathTemp, L"\\*");
		++cchFile;
	}
	else
		wcscpy(pszPathTemp, L"*");

	WCHAR pszNextLevelPath[MAX_PATH];
	PWSTR pszTemp = pszNextLevelPath + cchFile;
	wcscpy(pszNextLevelPath, pszPath);
	*pszTemp = L'\0';

	WIN32_FIND_DATAW wfd;
	HANDLE hFind;
	HTREEITEM hNewItem = NULL, hItemAfterDir = TVI_FIRST, hItemAfterFile = TVI_LAST;

	hFind = FindFirstFileW(pszPath, &wfd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (memcmp(wfd.cFileName, L".", 2 * sizeof(WCHAR)) == 0 ||
				memcmp(wfd.cFileName, L"..", 3 * sizeof(WCHAR)) == 0)
				continue;
			wcscpy(pszTemp, wfd.cFileName);
			if (IsBitSet(wfd.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY))
				hItemAfterDir = InsertItem(wfd.cFileName, pszNextLevelPath, cchFile, hParentItem, hItemAfterDir);
			else if (m_Info.bFile)
				hItemAfterFile = InsertItem(wfd.cFileName, pszNextLevelPath, cchFile, hParentItem, hItemAfterFile);
		} while (FindNextFileW(hFind, &wfd));
		FindClose(hFind);
	}
}

void CDirBox::FillCtrl()
{
	SetRedraw(FALSE);
	SendMessageW(m_hWnd, TVM_DELETEITEM, 0, NULL);
	if (!PathFileExistsW(m_rsDir))
	{
		SetRedraw(TRUE);
		return;
	}

	int cchFile = m_rsDir.Size();
	WCHAR pszPath[MAX_PATH];
	wcscpy(pszPath, m_rsDir);
	if (*(pszPath + cchFile - 1) == L'\\')
		*(pszPath + cchFile - 1) = L'\0';

	WCHAR pszNextLevelPath[MAX_PATH];
	PWSTR pszTemp = pszNextLevelPath + cchFile;
	wcscpy(pszNextLevelPath, m_rsDir);
	if (*(pszTemp - 1) != L'\\')
	{
		wcscpy(pszTemp, L"\\");
		++cchFile;
	}

	BOOL bHasChildPath;
	auto hNewItem = InsertItem(pszPath, pszNextLevelPath, cchFile, TVI_ROOT, TVI_LAST, &bHasChildPath);
	if (bHasChildPath)
	{
		SendMessageW(m_hWnd, TVM_DELETEITEM, 0, SendMessageW(m_hWnd, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)hNewItem));
		EnumFile(m_rsDir, hNewItem);
	}
	SendMessageW(m_hWnd, TVM_EXPAND, TVE_EXPAND, (LPARAM)hNewItem);
	SetRedraw(TRUE);
}

void CDirBox::GetCheckedItemsHelper(std::vector<CRefStrW>& aText, HTREEITEM hParentItem, TVITEMEXW* ptvi)
{
	HTREEITEM hItem = GetFirstChildItem(hParentItem);
	while (hItem)
	{
		if ((GetItemState(hItem, TVIS_STATEIMAGEMASK) >> 12) == 2)
		{
			ptvi->hItem = hItem;
			ptvi->cchTextMax = MAX_PATH - 1;
			SendMessageW(m_hWnd, TVM_GETITEMW, 0, (LPARAM)ptvi);
			if (ptvi->lParam != ECKDBITEMFLAG_ISHIDEITEM)
				aText.push_back(CRefStrW(ptvi->pszText));
		}
		GetCheckedItemsHelper(aText, hItem, ptvi);
		hItem = GetNextSiblingItem(hItem);
	}
}

LRESULT CALLBACK CDirBox::SubclassProc_Parent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, 
	UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	switch (uMsg)
	{
	case WM_NOTIFY:
	{
		auto it = m_WndRecord.find(((NMHDR*)lParam)->hwndFrom);
		if (it == m_WndRecord.end())
			break;
		auto p = it->second;
		auto pnmtv = (NMTREEVIEWW*)lParam;
		HWND hTV = ((NMHDR*)lParam)->hwndFrom;
		TVITEMEXW tvi;
		switch (((NMHDR*)lParam)->code)
		{
		case TVN_ITEMEXPANDEDW:
		{
			if (pnmtv->action == TVE_EXPAND)
			{
				p->SetRedraw(FALSE);
				std::wstring sPath{};
				WCHAR szBuf[MAX_PATH];
				int cch;
				HTREEITEM hItem = (HTREEITEM)SendMessageW(hTV, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)pnmtv->itemNew.hItem);
				if (!hItem)
					break;
				tvi.mask = TVIF_PARAM | TVIF_TEXT;
				tvi.hItem = hItem;
				tvi.cchTextMax = MAX_PATH - 1;
				tvi.pszText = szBuf;
				SendMessageW(hTV, TVM_GETITEMW, 0, (LPARAM)&tvi);

				if (tvi.lParam == ECKDBITEMFLAG_ISHIDEITEM)
				{
					tvi.mask = TVIF_TEXT;
					tvi.cchTextMax = MAX_PATH - 1;
					tvi.pszText = szBuf;
					tvi.hItem = pnmtv->itemNew.hItem;
					SendMessageW(hTV, TVM_GETITEMW, 0, (LPARAM)&tvi);
					sPath = tvi.pszText;
					SendMessageW(hTV, TVM_DELETEITEM, 0, (LPARAM)hItem);
					hItem = (HTREEITEM)SendMessageW(hTV, TVM_GETNEXTITEM, TVGN_PARENT, (LPARAM)pnmtv->itemNew.hItem);
					while (hItem)
					{
						tvi.hItem = hItem;
						tvi.cchTextMax = MAX_PATH - 1;
						SendMessageW(hTV, TVM_GETITEMW, 0, (LPARAM)&tvi);
						cch = (int)wcslen(tvi.pszText);

						if (*(tvi.pszText + cch - 1) == L'\\')
							sPath = std::wstring(tvi.pszText) + sPath;
						else
							sPath = std::wstring(tvi.pszText) + L"\\" + sPath;
						hItem = (HTREEITEM)SendMessageW(hTV, TVM_GETNEXTITEM, TVGN_PARENT, (LPARAM)hItem);
					}
					p->EnumFile(sPath.c_str(), pnmtv->itemNew.hItem);
				}
				p->SetRedraw(TRUE);
			}
			else if (pnmtv->action == TVE_COLLAPSE)
			{
				if (!p->m_Info.bNoCache)
					break;
				HTREEITEM hItemTemp;
				HTREEITEM hItem = (HTREEITEM)SendMessageW(hTV, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)pnmtv->itemNew.hItem);
				if (!hItem)
					break;

				p->SetRedraw(FALSE);
				tvi.mask = TVIF_PARAM;
				tvi.hItem = hItem;
				tvi.lParam = ECKDBITEMFLAG_ISHIDEITEM;
				SendMessageW(hTV, TVM_SETITEMW, 0, (LPARAM)&tvi);

				hItem = (HTREEITEM)SendMessageW(hTV, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)hItem);
				while (hItem)
				{
					hItemTemp = (HTREEITEM)SendMessageW(hTV, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)hItem);
					SendMessageW(hTV, TVM_DELETEITEM, 0, (LPARAM)hItem);
					hItem = hItemTemp;
				}
				p->SetRedraw(TRUE);
			}
		}
		break;

		case TVN_SELCHANGEDW:
		{
			std::wstring sPath{};
			WCHAR szBuf[MAX_PATH];
			int cch;
			HTREEITEM hItem;

			tvi.mask = TVIF_TEXT;
			tvi.cchTextMax = MAX_PATH - 1;
			tvi.pszText = szBuf;
			tvi.hItem = pnmtv->itemNew.hItem;
			SendMessageW(hTV, TVM_GETITEMW, 0, (LPARAM)&tvi);
			sPath = tvi.pszText;
			hItem = (HTREEITEM)SendMessageW(hTV, TVM_GETNEXTITEM, TVGN_PARENT, (LPARAM)pnmtv->itemNew.hItem);
			while (hItem)
			{
				tvi.hItem = hItem;
				tvi.cchTextMax = MAX_PATH - 1;
				SendMessageW(hTV, TVM_GETITEMW, 0, (LPARAM)&tvi);
				cch = (int)wcslen(tvi.pszText);

				if (*(tvi.pszText + cch - 1) == L'\\')
					sPath = std::wstring(tvi.pszText) + sPath;
				else
					sPath = std::wstring(tvi.pszText) + L"\\" + sPath;
				hItem = (HTREEITEM)SendMessageW(hTV, TVM_GETNEXTITEM, TVGN_PARENT, (LPARAM)hItem);
			}

			p->m_sCurrPath = std::move(sPath);
		}
		break;
		}
	}
	break;

	case WM_DESTROY:
		m_SMRef.DeleteRecord(hWnd);
		break;
	}

	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK CDirBox::SubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, 
	UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	auto p = (CDirBox*)dwRefData;
	switch (uMsg)
	{
	case WM_DESTROY:
	{
		m_SMRef.Release(p->m_hParent);
		m_SM.RemoveSubclass(hWnd);
	}
	break;

	case WM_SETFONT:
	{
		LOGFONTA lf;
		GetObjectA((HFONT)wParam, sizeof(lf), &lf);
		int cy = std::max(p->m_cyImage, (int)lf.lfHeight);
		int iDpi = GetDpi(hWnd);
		if (cy < DpiScale(16, iDpi))
			cy = -1;
		else
			cy += DpiScale(4, iDpi);
		DefSubclassProc(hWnd, TVM_SETITEMHEIGHT, cy, lParam);
	}
	break;
	}

	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

void CDirBox::ExtendToPath(PCWSTR pszPath)
{
	if (!pszPath)
		return;

	std::wstring_view svPath(pszPath), svSubStr;
	TVITEMEXW tvi;
	WCHAR szBuf[MAX_PATH];

	HTREEITEM hTopItem = GetRootItem();
	HTREEITEM hItem;
	BOOL bExpand;
	size_t uPos = svPath.find(L"\\");
	size_t uOldPos = 0u;
	SetRedraw(FALSE);
	while (uPos != std::wstring_view::npos && hTopItem)
	{
		svSubStr = svPath.substr(uOldPos, uPos - uOldPos);
		hItem = hTopItem;
		bExpand = FALSE;
		do
		{
			tvi.cchTextMax = MAX_PATH - 1;
			tvi.pszText = szBuf;
			GetItem(hItem, TVIF_TEXT, &tvi);

			if (_wcsnicmp(svSubStr.data(), szBuf, svSubStr.size()) == 0)
			{
				bExpand = TRUE;
				Expand(hItem, TVE_EXPAND);
				break;
			}
			else
			{
				hItem = GetNextSiblingItem(hItem);
			}
		} while (hItem);

		if (!bExpand)
			break;
		uOldPos = uPos + 1;
		hTopItem = GetFirstChildItem(hItem);
		uPos = svPath.find(L"\\", uOldPos);
		if (uPos == std::wstring_view::npos && uOldPos < svPath.size())
			uPos = svPath.size();
	}
	SetRedraw(TRUE);
}

std::vector<CRefStrW> CDirBox::GetCheckedItems()
{
	WCHAR szBuf[MAX_PATH];
	TVITEMEXW tvi;
	tvi.mask = TVIF_TEXT | TVIF_PARAM;
	tvi.pszText = szBuf;
	std::vector<CRefStrW> aText{};

	GetCheckedItemsHelper(aText, GetRootItem(), &tvi);
	return aText;
}
ECK_NAMESPACE_END