#pragma once
#include "CProject.h"
#include "CWndSelectOverlay.h"
#include "CWndWork.h"


class CWndMain :public eck::CForm
{
    // Dm = Designing Managment

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
        std::shared_ptr<DsForm> pForm{};
        std::unique_ptr<CWndWork> pWorkWnd{};
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
    CWndSelectOverlay m_SelOverlay{};

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
    int m_idxCurrTab{ -1 };

    HFONT m_hFontComm{};
    HFONT m_hFontCtrlBox{};

    BOOL m_bPlacingCtrl{};

    CProject m_Project{};

    int m_iDpi{ 96 };
private:
    void OnCreate(HWND hWnd);
    LRESULT OnCustomDrawCtrlListBox(const eck::NMCUSTOMDRAWEXT* p);
    LRESULT OnCommand(HWND hWnd, int nId, HWND hCtrl, UINT uNotifyCode);

    void InitMenu();

    void DmShowForm(int idxTab);
    void DmAddForm();
public:
    LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

    void SrvDrawOverlayRect(HWND hWndRef, const RECT* prc, size_t cRc) noexcept;
    void SrvClearOverlayRect() noexcept;
};