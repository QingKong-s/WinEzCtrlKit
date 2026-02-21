#pragma once
#include "CWindow.h"

ECK_NAMESPACE_BEGIN
class CPropertySheet : public CWindow
{
public:
    ECK_RTTI(CPropertySheet, CWindow);

    EckInline INT_PTR Create(const PROPSHEETHEADERW* ppsh) noexcept
    {
        INT_PTR lRet = PropertySheetW(ppsh);
        if (IsBitSet(ppsh->dwFlags, PSH_MODELESS))
            Attach((HWND)lRet);
        return lRet;
    }

    EckInline BOOL AddPage(HPROPSHEETPAGE hPage) const noexcept
    {
        return (BOOL)SendMsg(PSM_ADDPAGE, 0, (LPARAM)hPage);
    }

    /// <summary>
    /// 模拟应用操作
    /// </summary>
    /// <returns>若所有页面均已应用，则返回TRUE，否则返回FALSE</returns>
    EckInline BOOL Apply() const noexcept
    {
        return (BOOL)SendMsg(PSM_APPLY, 0, 0);
    }

    /// <summary>
    /// 指示操作不可逆
    /// </summary>
    EckInline void CancelToClose() const noexcept
    {
        SendMsg(PSM_CANCELTOCLOSE, 0, 0);
    }

    EckInline void Changed(HPROPSHEETPAGE hPage) const noexcept
    {
        SendMsg(PSM_CHANGED, (WPARAM)hPage, 0);
    }

    /// <summary>
    /// 启用/禁用向导按钮
    /// </summary>
    /// <param name="uButtons">按钮，PSBTN_常量</param>
    /// <param name="uMask">掩码，PSBTN_常量</param>
    EckInline void EnableWizardButton(UINT uButtons, UINT uMask) const noexcept
    {
        SendMsg(PSM_ENABLEWIZBUTTONS, uButtons, uMask);
    }

    /// <summary>
    /// 启用/禁用向导按钮
    /// </summary>
    /// <param name="uButtons">按钮，PSBTN_常量</param>
    /// <param name="bEnable">是否启用</param>
    EckInline void EnableWizardButton2(UINT uButtons, BOOL bEnable) const noexcept
    {
        SendMsg(PSM_ENABLEWIZBUTTONS, bEnable ? uButtons : 0, uButtons);
    }

    EckInline HWND GetCurrentPageWindow() const noexcept
    {
        return (HWND)SendMsg(PSM_GETCURRENTPAGEHWND, 0, 0);
    }

    EckInline int GetResult() const noexcept
    {
        return (int)SendMsg(PSM_GETRESULT, 0, 0);
    }

    EckInline HWND GetTabControl() const noexcept
    {
        return (HWND)SendMsg(PSM_GETTABCONTROL, 0, 0);
    }

    EckInline int HWndToIndex(HWND hDlg) const noexcept
    {
        return (int)SendMsg(PSM_HWNDTOINDEX, (WPARAM)hDlg, 0);
    }

    EckInline int IdToIndex(PCWSTR idRes) const noexcept
    {
        return (int)SendMsg(PSM_IDTOINDEX, 0, (LPARAM)idRes);
    }

    EckInline HWND IndexToHWnd(int idx) const noexcept
    {
        return (HWND)SendMsg(PSM_INDEXTOHWND, idx, 0);
    }

    EckInline PCWSTR IndexToId(int idx) const noexcept
    {
        return (PCWSTR)SendMsg(PSM_INDEXTOID, idx, 0);
    }

    EckInline HPROPSHEETPAGE IndexToHPage(int idx) const noexcept
    {
        return (HPROPSHEETPAGE)SendMsg(PSM_INDEXTOPAGE, idx, 0);
    }

    EckInline BOOL InsertPage(HPROPSHEETPAGE hPage,
        HPROPSHEETPAGE hInsertAfter = nullptr) const noexcept
    {
        return (BOOL)SendMsg(PSM_INSERTPAGE, (WPARAM)hInsertAfter, (LPARAM)hPage);
    }

    EckInline BOOL InsertPage(HPROPSHEETPAGE hPage, int idx) const noexcept
    {
        return (BOOL)SendMsg(PSM_INSERTPAGE, idx, (LPARAM)hPage);
    }

    EckInline BOOL IsPsDialogMessage(MSG* pmsg) const noexcept
    {
        return (BOOL)SendMsg(PSM_ISDIALOGMESSAGE, 0, (LPARAM)pmsg);
    }

    EckInline int HPageToIndex(HPROPSHEETPAGE hPage) const noexcept
    {
        return (int)SendMsg(PSM_PAGETOINDEX, 0, (LPARAM)hPage);
    }

    EckInline void PressButton(UINT uButton) const noexcept
    {
        SendMsg(PSM_PRESSBUTTON, uButton, 0);
    }

    /// <summary>
    /// 广播消息。
    /// 向每个页面发送PSM_QUERYSIBLINGS消息。
    /// 若某个页面返回TRUE，则停止向后续页面发送消息
    /// </summary>
    /// <param name="wParam"></param>
    /// <param name="lParam"></param>
    /// <returns>若所有页面均已接收到消息，则返回TRUE，否则返回FALSE</returns>
    EckInline BOOL QuerySiblings(WPARAM wParam, LPARAM lParam) const noexcept
    {
        return (BOOL)SendMsg(PSM_QUERYSIBLINGS, wParam, lParam);
    }

    EckInline void RebootSystem() const noexcept
    {
        SendMsg(PSM_REBOOTSYSTEM, 0, 0);
    }

    EckInline BOOL ReCalculatePageSizes() const noexcept
    {
        return (BOOL)SendMsg(PSM_RECALCPAGESIZES, 0, 0);
    }

    EckInline void DeletePage(int idx) const noexcept
    {
        SendMsg(PSM_REMOVEPAGE, idx, 0);
    }

    EckInline void DeletePage(HPROPSHEETPAGE hPage) const noexcept
    {
        SendMsg(PSM_REMOVEPAGE, 0, (LPARAM)hPage);
    }

    EckInline void RestartWindows() const noexcept
    {
        SendMsg(PSM_RESTARTWINDOWS, 0, 0);
    }

    EckInline void SetButtonText(PCWSTR pszText, UINT uButton) const noexcept
    {
        SendMsg(PSM_SETBUTTONTEXTW, uButton, (LPARAM)pszText);
    }

    EckInline BOOL SetCurrentSelection(int idx) const noexcept
    {
        return (BOOL)SendMsg(PSM_SETCURSEL, idx, 0);
    }

    EckInline BOOL SetCurrentSelection(HPROPSHEETPAGE hPage) const noexcept
    {
        return (BOOL)SendMsg(PSM_SETCURSEL, 0, (LPARAM)hPage);
    }

    EckInline BOOL SetCurrentSelection(PCWSTR idRes) const noexcept
    {
        return (BOOL)SendMsg(PSM_SETCURSELID, 0, (LPARAM)idRes);
    }

    EckInline void SetFinishText(int idx, PCWSTR pszText) const noexcept
    {
        SendMsg(PSM_SETFINISHTEXTW, idx, (LPARAM)pszText);
    }

    EckInline void SetHeaderSubTitle(int idx, PCWSTR pszText) const noexcept
    {
        SendMsg(PSM_SETHEADERSUBTITLEW, idx, (LPARAM)pszText);
    }

    EckInline void SetHeaderTitle(int idx, PCWSTR pszText) const noexcept
    {
        SendMsg(PSM_SETHEADERTITLEW, idx, (LPARAM)pszText);
    }

    EckInline void SetNextText(PCWSTR pszText) const noexcept
    {
        SendMsg(PSM_SETNEXTTEXTW, 0, (LPARAM)pszText);
    }

    /// <summary>
    /// 置标题
    /// </summary>
    /// <param name="pszText">标题文本，可传入MAKEINRESOURCE(资源ID)</param>
    /// <param name="bPropOf">是否显示为某某的属性</param>
    EckInline void SetTitle(PCWSTR pszText, BOOL bPropOf = FALSE) const noexcept
    {
        SendMsg(PSM_SETTITLEW, bPropOf ? PSH_PROPTITLE : 0, (LPARAM)pszText);
    }

    /// <summary>
    /// 置向导按钮
    /// </summary>
    /// <param name="uButtons">按钮，PSBTN_常量</param>
    /// <param name="bShieldIcon">是否显示盾牌图标，仅在Aero向导中有效</param>
    EckInline void SetWizardButtons(UINT uButtons, BOOL bShieldIcon = FALSE) const noexcept
    {
        SendMsg(PSM_SETWIZBUTTONS, bShieldIcon ? PSWIZBF_ELEVATIONREQUIRED : 0, uButtons);
    }

    /// <summary>
    /// 显示/隐藏向导按钮
    /// </summary>
    /// <param name="uButtons">按钮，PSBTN_常量</param>
    /// <param name="uMask">掩码，PSBTN_常量</param>
    EckInline void ShowWizardButton(UINT uButtons, UINT uMask) const noexcept
    {
        SendMsg(PSM_SHOWWIZBUTTONS, uButtons, uMask);
    }

    /// <summary>
    /// 显示/隐藏向导按钮
    /// </summary>
    /// <param name="uButtons">按钮，PSBTN_常量</param>
    /// <param name="bShow">是否显示</param>
    EckInline void ShowWizardButton2(UINT uButtons, BOOL bShow) const noexcept
    {
        SendMsg(PSM_SHOWWIZBUTTONS, bShow ? uButtons : 0, uButtons);
    }

    EckInline void Unchanged(HPROPSHEETPAGE hPage) const noexcept
    {
        SendMsg(PSM_UNCHANGED, (WPARAM)hPage, 0);
    }
};
ECK_NAMESPACE_END