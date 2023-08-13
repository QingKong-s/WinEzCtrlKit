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
#include "CSubclassMgr.h"

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
	struct PARENTSCCTX
	{
		int iDpi;
	};
	SUBCLASS_MGR_DECL(CDirBox)
	SUBCLASS_REF_MGR_DECL(CDirBox, PARENTSCCTX*)

	ECKDIRBOXDATA m_Info{};

	CRefStrW m_rsDir{};
	IImageList* m_pIImageList = NULL;
	int m_cyImage = 0;
	std::wstring m_sCurrPath{};
	HWND m_hParent = NULL;
	PARENTSCCTX m_ParentCtx{};

	HTREEITEM InsertItem(PCWSTR pszText, PWSTR pszNextLevelPath, int cchNextLevelPath,
		HTREEITEM hParentItem, HTREEITEM hItemAfter, BOOL* pbHasChildPath = NULL);

	void EnumFile(PCWSTR pszFile, HTREEITEM hParentItem);

	void FillCtrl();

	void GetCheckedItemsHelper(std::vector<CRefStrW>& aText, HTREEITEM hParentItem, TVITEMEXW* ptvi);

	static LRESULT CALLBACK SubclassProc_Parent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
		UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

	static LRESULT CALLBACK SubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
		UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
public:
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, UINT nID)
	{
		dwStyle |= WS_CHILD;

		m_hWnd = CreateWindowExW(dwExStyle, WC_TREEVIEWW, NULL, dwStyle,
			x, y, cx, cy, hParent, i32ToP<HMENU>(nID), NULL, NULL);
		m_SM.AddSubclass(m_hWnd, this);
		m_hParent = hParent;
		m_ParentCtx.iDpi = GetDpi(hParent);
		m_SMRef.AddRef(hParent, &m_ParentCtx);

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

	void ExtendToPath(PCWSTR pszPath);

	std::vector<CRefStrW> GetCheckedItems();

	EckInline void Refresh()
	{
		FillCtrl();
	}
};
ECK_NAMESPACE_END