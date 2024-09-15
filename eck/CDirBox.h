/*
* WinEzCtrlKit Library
*
* CDirBox.h ： 目录框
* 使用TreeView实现文件和目录的树形显示
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "CTreeView.h"

#include <string>
#include <string_view>

#include <Shlwapi.h>
#include <commoncontrols.h>

ECK_NAMESPACE_BEGIN

#define ECKDBITEMFLAG_ISHIDEITEM 1

struct ECKDIRBOXDATA
{
	BITBOOL bFile : 1;   // 显示文件
	BITBOOL bNoCache : 1;// 禁止缓存内容
};

class CDirBox :public CTreeView
{
private:
	ECKDIRBOXDATA m_Info{};

	CRefStrW m_rsDir{};
	IImageList* m_pIImageList = nullptr;
	int m_cyImage = 0;
	std::wstring m_sCurrPath{};
	HWND m_hParent = nullptr;

	void EnumFile(PCWSTR pszFile, HTREEITEM hParentItem)
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
		HTREEITEM hNewItem = nullptr, hItemAfterDir = TVI_FIRST, hItemAfterFile = TVI_LAST;

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

	void FillCtrl()
	{
		SetRedraw(FALSE);
		SendMessageW(m_hWnd, TVM_DELETEITEM, 0, NULL);
		if (!PathFileExistsW(m_rsDir.Data()))
		{
			SetRedraw(TRUE);
			return;
		}

		int cchFile = m_rsDir.Size();
		WCHAR pszPath[MAX_PATH];
		wcscpy(pszPath, m_rsDir.Data());
		if (*(pszPath + cchFile - 1) == L'\\')
			*(pszPath + cchFile - 1) = L'\0';

		WCHAR pszNextLevelPath[MAX_PATH];
		PWSTR pszTemp = pszNextLevelPath + cchFile;
		wcscpy(pszNextLevelPath, m_rsDir.Data());
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
			EnumFile(m_rsDir.Data(), hNewItem);
		}
		SendMessageW(m_hWnd, TVM_EXPAND, TVE_EXPAND, (LPARAM)hNewItem);
		SetRedraw(TRUE);
	}

	void GetCheckedItemsHelper(std::vector<CRefStrW>& aText, HTREEITEM hParentItem, TVITEMEXW* ptvi)
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
public:
	HTREEITEM InsertItem(PCWSTR pszText, PWSTR pszNextLevelPath, int cchNextLevelPath,
		HTREEITEM hParentItem, HTREEITEM hItemAfter, BOOL* pbHasChildPath = nullptr)
	{
		if (pbHasChildPath)
			*pbHasChildPath = FALSE;
		PWSTR pTemp = pszNextLevelPath + cchNextLevelPath;

		TVINSERTSTRUCTW tis;
		SHFILEINFOW sfi{};
		WIN32_FIND_DATAW wfd;
		HANDLE hFind;
		HTREEITEM hNewItem = nullptr;

		if (!SHGetFileInfoW(pszNextLevelPath, 0, &sfi, sizeof(SHFILEINFOW), SHGFI_SYSICONINDEX | SHGFI_SMALLICON))
			return nullptr;
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

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_SETFONT:
		{
			LOGFONTA lf;
			GetObjectA((HFONT)wParam, sizeof(lf), &lf);
			int cy = std::max(m_cyImage, (int)lf.lfHeight);
			int iDpi = GetDpi(hWnd);
			if (cy < DpiScale(16, iDpi))
				cy = -1;
			else
				cy += DpiScale(4, iDpi);
			CWnd::OnMsg(hWnd, TVM_SETITEMHEIGHT, cy, lParam);
		}
		break;
		}

		return CTreeView::OnMsg(hWnd, uMsg, wParam, lParam);
	}

	LRESULT OnNotifyMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bProcessed) override
	{
		switch (uMsg)
		{
		case WM_NOTIFY:
		{
			TVITEMEXW tvi;
			switch (((NMHDR*)lParam)->code)
			{
			case TVN_ITEMEXPANDEDW:
			{
				bProcessed = TRUE;
				auto pnmtv = (NMTREEVIEWW*)lParam;
				if (pnmtv->action == TVE_EXPAND)
				{
					SetRedraw(FALSE);
					std::wstring sPath{};
					WCHAR szBuf[MAX_PATH];
					int cch;
					HTREEITEM hItem = GetFirstChildItem(pnmtv->itemNew.hItem);
					if (!hItem)
						break;
					tvi.mask = TVIF_PARAM | TVIF_TEXT;
					tvi.hItem = hItem;
					tvi.cchTextMax = MAX_PATH - 1;
					tvi.pszText = szBuf;
					GetItem(&tvi);

					if (tvi.lParam == ECKDBITEMFLAG_ISHIDEITEM)
					{
						tvi.mask = TVIF_TEXT;
						tvi.cchTextMax = MAX_PATH - 1;
						tvi.pszText = szBuf;
						tvi.hItem = pnmtv->itemNew.hItem;
						GetItem(&tvi);
						sPath = tvi.pszText;
						DeleteItem(hItem);
						hItem = GetParentItem(pnmtv->itemNew.hItem);
						while (hItem)
						{
							tvi.hItem = hItem;
							tvi.cchTextMax = MAX_PATH - 1;
							GetItem(&tvi);
							cch = (int)wcslen(tvi.pszText);

							if (*(tvi.pszText + cch - 1) == L'\\')
								sPath = std::wstring(tvi.pszText) + sPath;
							else
								sPath = std::wstring(tvi.pszText) + L"\\" + sPath;
							hItem = hItem = GetParentItem(pnmtv->itemNew.hItem);
						}
						EnumFile(sPath.c_str(), pnmtv->itemNew.hItem);
					}
					SetRedraw(TRUE);
				}
				else if (pnmtv->action == TVE_COLLAPSE)
				{
					if (!m_Info.bNoCache)
						break;
					HTREEITEM hItemTemp;
					HTREEITEM hItem = GetFirstChildItem(pnmtv->itemNew.hItem);
					if (!hItem)
						break;

					SetRedraw(FALSE);
					tvi.mask = TVIF_PARAM;
					tvi.hItem = hItem;
					tvi.lParam = ECKDBITEMFLAG_ISHIDEITEM;
					SetItem(&tvi);

					hItem = GetNextSiblingItem(hItem);
					while (hItem)
					{
						hItemTemp = GetNextSiblingItem(hItem);
						DeleteItem(hItem);
						hItem = hItemTemp;
					}
					SetRedraw(TRUE);
				}
			}
			return 0;
			case TVN_SELCHANGEDW:
			{
				bProcessed = TRUE;
				auto pnmtv = (NMTREEVIEWW*)lParam;
				std::wstring sPath{};
				WCHAR szBuf[MAX_PATH];
				int cch;
				HTREEITEM hItem;

				tvi.mask = TVIF_TEXT;
				tvi.cchTextMax = MAX_PATH - 1;
				tvi.pszText = szBuf;
				tvi.hItem = pnmtv->itemNew.hItem;
				GetItem(&tvi);
				sPath = tvi.pszText;
				hItem = GetParentItem(pnmtv->itemNew.hItem);
				while (hItem)
				{
					tvi.hItem = hItem;
					tvi.cchTextMax = MAX_PATH - 1;
					GetItem(&tvi);
					cch = (int)wcslen(tvi.pszText);

					if (*(tvi.pszText + cch - 1) == L'\\')
						sPath = std::wstring(tvi.pszText) + sPath;
					else
						sPath = std::wstring(tvi.pszText) + L"\\" + sPath;
					hItem = GetParentItem(pnmtv->itemNew.hItem);
				}

				m_sCurrPath = std::move(sPath);
			}
			return 0;
			}
		}
		break;
		}
		return CTreeView::OnNotifyMsg(hWnd, uMsg, wParam, lParam, bProcessed);
	}

	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = nullptr) override
	{
		dwStyle |= WS_CHILD;

		m_hWnd = CreateWindowExW(dwExStyle, WC_TREEVIEWW, nullptr, dwStyle,
			x, y, cx, cy, hParent, hMenu, nullptr, nullptr);

		int cxImage;
		SHGetImageList(SHIL_SMALL, IID_PPV_ARGS(&m_pIImageList));
		SetImageList((HIMAGELIST)m_pIImageList);
		m_pIImageList->GetIconSize(&cxImage, &m_cyImage);

		FillCtrl();
		return m_hWnd;
	}

	EckInline void SetDir(PCWSTR pszDir)
	{
		m_rsDir = pszDir;
		FillCtrl();
	}

	EckInline const CRefStrW& GetDir(int* pcb)
	{
		return m_rsDir;
	}

	EckInline void SetFileShowing(BOOL bFileShowing)
	{
		if (m_Info.bFile != bFileShowing)
		{
			m_Info.bFile = bFileShowing;
			FillCtrl();
		}
	}

	EckInline BOOL GetFileShowing()
	{
		return m_Info.bFile;
	}

	EckInline void SetNoCache(BOOL bNoCache)
	{
		m_Info.bNoCache = bNoCache;
	}

	EckInline BOOL GetNoCache()
	{
		return m_Info.bNoCache;
	}

	EckInline const std::wstring& GetCurrPath()
	{
		return m_sCurrPath;
	}

	void ExtendToPath(PCWSTR pszPath)
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
				GetItem(&tvi);

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

	std::vector<CRefStrW> GetCheckedItems()
	{
		WCHAR szBuf[MAX_PATH];
		TVITEMEXW tvi;
		tvi.mask = TVIF_TEXT | TVIF_PARAM;
		tvi.pszText = szBuf;
		std::vector<CRefStrW> aText{};

		GetCheckedItemsHelper(aText, GetRootItem(), &tvi);
		return aText;
	}

	EckInline void Refresh()
	{
		FillCtrl();
	}
};
ECK_NAMESPACE_END