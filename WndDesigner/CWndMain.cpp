#include "pch.h"
#include "CWndMain.h"
#include "BuiltinControl.h"

enum : int
{
    CyComboBox = 24,
    CyDescEdit = 100,
    CxDefProp = 200,
    CxDefCtrlBox = 200,
    CyDefProjTree = 220,
    CxIlMain = 16,
    CyIlMain = 16,
    CxIntPadding = 4,
    CxExtPadding = 6,
};

void CWndMain::OnCreate(HWND hWnd)
{
    eck::GetThreadCtx()->UpdateDefaultColor();
    m_iDpi = eck::GetDpi(hWnd);
    m_hFontComm = eck::DftCreate(m_iDpi);

    InitMenu();
    eck::UxfMenuInit(this);

    /*ComPtr<IStream> pStream;
    SHCreateStreamOnFileEx(LR"(1.il)",
        STGM_READ, 0, FALSE, nullptr, &pStream);
    ImageList_ReadEx(ILP_NORMAL, pStream.Get(), IID_PPV_ARGS(&m_pilMain));
    m_pilMain->GetIconSize(&m_cxIlMain, &m_cyIlMain);*/

    const auto cxIntPad = eck::DpiScale((eck::TLytCoord)CxIntPadding, m_iDpi);

    constexpr DWORD dwCommStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP;
    const eck::LYTMARGINS Mar{ .r = cxIntPad };
    {
        const eck::LYTMARGINS Mar{ .b = cxIntPad };
        m_CBBCtrl.Create(nullptr, dwCommStyle, 0,
            0, 0, 0, eck::DpiScale(CyComboBox, m_iDpi), hWnd, 0);
        m_LytProp.Add(&m_CBBCtrl, Mar, eck::LF_FIX_HEIGHT | eck::LF_FILL_WIDTH);

        m_PLProp.Create(nullptr, dwCommStyle | WS_BORDER, 0,
            0, 0, eck::DpiScale(CxDefProp, m_iDpi), 0, hWnd, 0);
        m_LytProp.Add(&m_PLProp, Mar, eck::LF_FILL, 1u);

        m_EDDesc.SetMultiLine(TRUE);
        m_EDDesc.Create(nullptr, dwCommStyle | ES_AUTOVSCROLL, 0,
            0, 0, 0, eck::DpiScale(CyDescEdit, m_iDpi), hWnd, 0);
        m_EDDesc.SetFrameType(3);
        m_LytProp.Add(&m_EDDesc, {}, eck::LF_FIX_HEIGHT | eck::LF_FILL_WIDTH);
    }
    m_Lyt.Add(&m_LytProp, Mar, eck::LF_FILL_HEIGHT | eck::LF_FIX_WIDTH);

    m_Tab.Create(nullptr, dwCommStyle, WS_EX_COMPOSITED,
        0, 0, 0, 0, hWnd, 0);
    m_SelOverlay.Create(nullptr, WS_CHILD | WS_VISIBLE, WS_EX_TRANSPARENT,
        0, 0, 0, 0, m_Tab.HWnd, 0);
    m_Lyt.Add(&m_Tab, Mar, eck::LF_FILL, 1u);

    {
        constexpr auto dwTVStyle = TVS_LINESATROOT | TVS_HASBUTTONS |
            TVS_SHOWSELALWAYS | TVS_TRACKSELECT |
            TVS_DISABLEDRAGDROP | TVS_FULLROWSELECT;
        m_TVProject.Create(nullptr, dwCommStyle | WS_BORDER | dwTVStyle, 0,
            0, 0, eck::DpiScale(CxDefCtrlBox, m_iDpi),
            eck::DpiScale(CyDefProjTree, m_iDpi), hWnd, 0);
        m_TVProject.SetTVExtendStyle(TVS_EX_DOUBLEBUFFER | TVS_EX_FADEINOUTEXPANDOS);
        TVINSERTSTRUCTW tvis;
        tvis.hParent = TVI_ROOT;
        tvis.hInsertAfter = TVI_LAST;
        tvis.itemex.mask = TVIF_TEXT | TVIF_STATE;
        tvis.itemex.state = tvis.itemex.stateMask = TVIS_EXPANDED;
        tvis.itemex.pszText = (PWSTR)L"工程";
        tvis.hParent = m_htiRoot = m_TVProject.InsertItem(&tvis);

        tvis.itemex.pszText = (PWSTR)L"窗体";
        m_htiForms = m_TVProject.InsertItem(&tvis);
        m_LytProjAndCtrl.Add(&m_TVProject, Mar, eck::LF_FIX);

        m_LBCtrl.Create(nullptr, dwCommStyle | WS_BORDER, 0,
            0, 0, eck::DpiScale(CxDefCtrlBox, m_iDpi), 0, hWnd, 0);
        m_LBCtrl.SetItemCount(ARRAYSIZE(BuiltInCtrls) + 1);
        m_LytProjAndCtrl.Add(&m_LBCtrl, Mar, eck::LF_FILL, 1u);
    }
    m_Lyt.Add(&m_LytProjAndCtrl, Mar, eck::LF_FILL_HEIGHT | eck::LF_FIX_WIDTH);

    m_Lyt.LoInitializeDpi(m_iDpi);

    eck::ApplyWindowFont(hWnd, m_hFontComm);
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
        //m_pilMain->Draw(&ildp);

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
            DmAddForm();
            break;
        }
    return 0;
}

void CWndMain::UpdateComboBox()
{
    if (m_Tab.GetCurSel() < 0)
        m_CBBCtrl.GetListBox().SetItemCount(0);
    else
        m_CBBCtrl.GetListBox().SetItemCount(
            (int)GetCurrentTab().pForm->vChild.size());
    m_CBBCtrl.Redraw();
}

LRESULT CWndMain::OnGetDisplayInfoComboBox(eck::NMLBNGETDISPINFO* p)
{
    if (m_Tab.GetCurSel() < 0)
        return 0;
    const auto& rs = GetCurrentTab().pForm->vChild[p->Item.idxItem]->rsName;
    p->Item.pszText = rs.Data();
    p->Item.cchText = rs.Size();
    return TRUE;
}

LRESULT CWndMain::OnControlMsg(DsForm* pDsForm, DsWnd* pDsCtrlWnd,
    HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, eck::SlotCtx& Ctx) noexcept
{
    switch (uMsg)
    {
    case WM_LBUTTONDOWN:
    case WM_NCLBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
    case WM_NCLBUTTONDBLCLK:
        Ctx.Processed();
        SrvDrawOverlayRect(pDsForm->pWnd->HWnd, &pDsCtrlWnd->rc, 1);
        return 0;
    case WM_LBUTTONUP:
    case WM_NCLBUTTONUP:
        Ctx.Processed();
        return 0;
    case WM_NCMOUSEMOVE:
    case WM_MOUSEMOVE:
        Ctx.Processed();
        return 0;
    case WM_SETCURSOR:
        Ctx.Processed();
        return 0;
    case WM_WINDOWPOSCHANGED:
    {
        const auto* const pwp = (WINDOWPOS*)lParam;
        pDsCtrlWnd->rc = eck::MakeRect(pwp->x, pwp->y, pwp->cx, pwp->cy);
    }
    break;
    }
    return 0;
}

LRESULT CWndMain::OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_SIZE:
    {
        const auto cx = LOWORD(lParam);
        const auto cy = HIWORD(lParam);
        const auto cxExtPad = eck::DpiScale(CxExtPadding, m_iDpi);
        m_Lyt.Arrange(cxExtPad, cxExtPad,
            cx - cxExtPad * 2, cy - cxExtPad * 2);

        RECT rcTab;
        GetClientRect(m_Tab.HWnd, &rcTab);
        SetWindowPos(m_SelOverlay.HWnd, nullptr,
            0, 0, rcTab.right, rcTab.bottom,
            SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
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
                return OnCustomDrawCtrlListBox((eck::NMCUSTOMDRAWEXT*)lParam);
            }
        else if (pnmhdr->hwndFrom == m_Tab.HWnd)
            switch (pnmhdr->code)
            {
            case TCN_SELCHANGE:
                if (pnmhdr->hwndFrom == m_Tab.HWnd)
                {
                    DmShowForm(m_Tab.GetCurSel());
                    UpdateComboBox();
                    return 0;
                }
                break;
            }
        else if (pnmhdr->hwndFrom == m_CBBCtrl.HWnd)
            switch (pnmhdr->code)
            {
            case eck::NM_LBN_GETDISPINFO:
                return OnGetDisplayInfoComboBox((eck::NMLBNGETDISPINFO*)lParam);
            }
    }
    break;
    case WM_COMMAND:
        return OnCommand(hWnd, LOWORD(wParam), (HWND)lParam, HIWORD(wParam));

    case WM_DPICHANGED:
        eck::MsgOnDpiChanged(hWnd, lParam);
        m_iDpi = eck::GetDpi(hWnd);
        m_Lyt.LoOnDpiChanged(m_iDpi);
        return 0;
    case WM_SETTINGCHANGE:
        if (!eck::MsgOnSettingChangeMainWnd(hWnd, wParam, lParam))
            eck::BroadcastChildrenMessage(hWnd, uMsg, wParam, lParam);
        return 0;
    }
    return __super::OnMsg(hWnd, uMsg, wParam, lParam);
}

void CWndMain::SrvDrawOverlayRect(HWND hWndRef, const RECT* prc, size_t cRc) noexcept
{
    m_SelOverlay.SetRect(hWndRef, prc, cRc);
    m_SelOverlay.Redraw();
}

void CWndMain::SrvClearOverlayRect() noexcept
{
    m_SelOverlay.ClearRect();
    m_SelOverlay.Redraw();
}

void CWndMain::SrvEndSelect() noexcept
{
    if (m_SelOverlay.IsEmpty())
        return;
    const auto rc = m_SelOverlay.GetRectInWorkWnd();
    const auto& Tab = GetCurrentTab();
    const auto idxCtrl = m_LBCtrl.GetCurrSel() - 1;
    if (idxCtrl < 0)
    {
        m_SelOverlay.ClearRect();
        for (const auto& e : Tab.pForm->vChild)
        {
            RECT rr;
            if (eck::IntersectRect(rr, rc, e->rc))
                m_SelOverlay.AddRectInWorkWindow(e->rc);
            rr = {};
        }
        m_SelOverlay.EndAddRect(Tab.pWorkWnd->HWnd);
        m_SelOverlay.Redraw();
    }
    else
    {
        const auto& e = BuiltInCtrls[idxCtrl];
        const auto pWnd = eck::DynCast<eck::CWnd*>(eck::RttiNewObject(e.svClsName));
        if (pWnd)
        {
            const auto pDsWnd = Tab.pForm->AddChild();
            pDsWnd->rsName = e.svDisplayName;
            pDsWnd->rsTitle = e.svDisplayName;
            pDsWnd->pWnd = pWnd;
            pDsWnd->rc = rc;
            pDsWnd->dwStyle = e.dwDefStyle | WS_VISIBLE | WS_CHILD;
            pDsWnd->dwExStyle = e.dwDefExStyle;

            pWnd->GetSignal().Connect(
                [this, pDsForm = Tab.pForm, pDsWnd = pDsWnd](HWND hWnd, UINT uMsg,
                    WPARAM wParam, LPARAM lParam, eck::SlotCtx& Ctx)->LRESULT
                {
                    return OnControlMsg(pDsForm.get(), pDsWnd.get(), hWnd, uMsg, wParam, lParam, Ctx);
                });

            pWnd->Create(
                e.svDisplayName.data(),
                pDsWnd->dwStyle,
                pDsWnd->dwExStyle,
                rc.left, rc.top,
                rc.right - rc.left,
                rc.bottom - rc.top,
                Tab.pWorkWnd->HWnd, 0);

            UpdateComboBox();
        }
        SrvClearOverlayRect();
    }
}

void CWndMain::SrvWorkWindowMoved(HWND hWndWork) noexcept
{
    m_SelOverlay.EndAddRect(hWndWork);
    m_SelOverlay.Redraw();
}

void CWndMain::DmShowForm(int idxTab)
{
    for (const auto& x : m_vTabs)
        ShowWindow(x.pWorkWnd->GetHWND(), SW_HIDE);
    ShowWindow(m_vTabs[idxTab].pWorkWnd->GetHWND(), SW_SHOWNOACTIVATE);
}

void CWndMain::DmAddForm()
{
    const auto pForm = m_Project.RsAddForm(L"新窗体");
    m_Tab.InsertItem(pForm->rsName.Data());

    auto& e = m_vTabs.emplace_back(pForm, std::make_unique<CWndWork>(pForm, this));
    pForm->pWnd = e.pWorkWnd.get();
    e.pWorkWnd->Create(pForm->rsName.Data(),
        WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CHILD, 0,
        40, 60, 400, 300, m_Tab.HWnd, 0);

    SetWindowPos(m_SelOverlay.HWnd, HWND_TOP, 0, 0, 0, 0,
        SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
}