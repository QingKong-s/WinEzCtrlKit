#pragma once
#include "CProject.h"
#include "CWndWork.h"


class CWndMain :public eck::CForm
{
	friend class CWndWork;
public:
	enum
	{
		IDMI_FILE_NEW,
		IDMI_FILE_OPEN,
		IDMI_FILE_SAVE,
		IDMI_FILE_SAVEAS,
		IDMI_FILE_EXIT,

		IDMI_EDIT_UNDO,
		IDMI_EDIT_REDO,
		IDMI_EDIT_CUT,
		IDMI_EDIT_COPY,
		IDMI_EDIT_PASTE,
		IDMI_EDIT_SELECTALL,
		IDMI_EDIT_DELETE,

		IDMI_INSERT_FORM,
	};
private:
	struct ITEMTAB
	{
		DsForm* pForm{};		// 窗体，不持有所有权
		CWndWork* pWorkWnd{};	// 工作窗口，销毁标签页时解分配
	};

	enum
	{
		IDMI_WW_COPY = 101,
		IDMI_WW_CUT,
		IDMI_WW_PASTE,
		IDMI_WW_DEL,
	};

	eck::CComboBoxNew m_CBBCtrl{};
	eck::CListView m_PLProp{};
	eck::CEditExt m_EDDesc{};
	eck::CLinearLayoutV m_LytProp{};

	eck::CTab m_Tab{};

	eck::CTreeView m_TVProject{};
	eck::CListBoxNew m_LBCtrl{};
	eck::CLinearLayoutV m_LytProjAndCtrl{};

	eck::CLinearLayoutH m_Lyt{};

	eck::CMenu m_MenuBarMain{};
	eck::CMenu m_MenuFile{};
	eck::CMenu m_MenuEdit{};
	eck::CMenu m_MenuInsert{};

	ComPtr<IImageList2> m_pilMain{};
	int m_cxIlMain{}, m_cyIlMain{};

	HTREEITEM m_htiRoot{};
	HTREEITEM m_htiForms{};

	std::vector<ITEMTAB> m_vTabs{};
	int m_idxCurrTab = -1;

	HFONT m_hFontComm = nullptr;
	HFONT m_hFontCtrlBox = nullptr;

	BOOL m_bPlacingCtrl = FALSE;

	CProject m_Project{};

	int m_iDpi = USER_DEFAULT_SCREEN_DPI;
	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY(IntPad, 4)
		ECK_DS_ENTRY(ExtPad, 6)
		ECK_DS_ENTRY(sizeBlock, 8)
		;
	ECK_DS_END_VAR(m_Ds)
private:
	void UpdateDpi()
	{
		eck::UpdateDpiSize(m_Ds, m_iDpi);
	}

	void OnCreate(HWND hWnd);

	void InitMenu();

	LRESULT OnCustomDrawCtrlListBox(const eck::NMCUSTOMDRAWEXT* p);

	LRESULT OnCommand(HWND hWnd, int nId, HWND hCtrl, UINT uNotifyCode);

	void DsShowForm();

	void DsAddForm();
public:
	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
};