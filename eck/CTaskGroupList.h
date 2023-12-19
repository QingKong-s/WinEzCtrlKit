#pragma once
#include "CListView.h"
#include "CSubclassMgr.h"

ECK_NAMESPACE_BEGIN
struct TGLSUBTASK
{
	PCWSTR pszText;
	UINT uFlags;
};

struct TGLCLICKINFO
{
	int idxItem;
	UINT uPart;
	int idxSubTask;
};

enum
{
	TGLP_NONE,
	TGLP_ICON,
	TGLP_SECTIONTITLE,
	TGLP_SUBTASK
};

enum
{
	// 链接被单击，lParam = *TGLCLICKINFO
	TGLNM_CLICK,
};
class CTaskGroupList :public CListView
{
private:
	struct SUBTASK
	{
		CRefStrW rsText;
		UINT uFlags;
		int cx;
	};

	struct TASKITEM
	{
		CRefStrW rsText;
		int idxImage;
		std::vector<SUBTASK> SubTasks;
		int cx;
	};

	SUBCLASS_MGR_DECL(CTaskGroupList);
	SUBCLASS_REF_MGR_DECL(CTaskGroupList, ObjRecorderRefPlaceholder);

	HWND m_hParent = NULL;
	UINT m_uNotifyMsg = 0;

	std::vector<TASKITEM> m_Items{};

	HTHEME m_hthControlPanel = NULL;
	HTHEME m_hthListView = NULL;
	HDC m_hCDCAuxiliary = NULL;
	HIMAGELIST m_hImageList = NULL;

	int m_iIdealWidth = 0;
	int m_cxClient = 0;

	int m_cxSubTaskPadding = 0;
	int m_cxPadding = 0;
	
	int m_cxIcon = 0, 
		m_cyIcon = 0;
	int m_cySectionTitle = 0;
	int m_cySubTask = 0;

	int m_idxHot = -1;
	BOOL m_bSectionTitleHot = FALSE;
	int m_idxHotSubTask = -1;

	int m_idxPressed = -1;
	BOOL m_bSectionTitlePressed = FALSE;
	int m_idxPressedSubTask = -1;

	BOOL m_bLBtnDown = FALSE;

	static LRESULT CALLBACK SubclassProc_Parent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
		UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

	static LRESULT CALLBACK SubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
		UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

	void RefreshThemeRes();
public:
	
	ECK_CWND_CREATE;

	EckInline void SetNotifyMsg(UINT uMsg)
	{
		m_uNotifyMsg = uMsg;
	}

	int InsertTask(PCWSTR pszText, int idx = -1, int idxImage = -1, TGLSUBTASK* pSubTask = NULL, int cSubTask = 0);

	int RecalcColumnIdealWidth();

	EckInline int GetIdealWidth()
	{
		return m_iIdealWidth;
	}

	HIMAGELIST SetTaskImageList(HIMAGELIST hImageList)
	{
		auto hOld = m_hImageList;
		m_hImageList = hImageList;
		ImageList_GetIconSize(hImageList, &m_cxIcon, &m_cyIcon);
		Redraw();
		return hOld;
	}

	UINT HitTestTask(RECT* prcItem, int idxItem, POINT pt, int* pidxSubTask = NULL);
};

ECK_NAMESPACE_END