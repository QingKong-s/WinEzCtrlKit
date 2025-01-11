#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
class CStatusBar : public CWnd
{
public:
	ECK_RTTI(CStatusBar);
	ECK_CWND_NOSINGLEOWNER(CStatusBar);
	ECK_CWND_CREATE_CLS(STATUSCLASSNAMEW);

	ECK_CWNDPROP_STYLE(SizeGrip, SBARS_SIZEGRIP);
	ECK_CWNDPROP_STYLE(ToolTips, SBARS_TOOLTIPS);

	/// <summary>
	/// 取边框尺寸
	/// </summary>
	/// <param name="pi">水平边框、垂直边框、项目间距</param>
	/// <returns>成功返回TRUE，失败返回FALSE</returns>
	EckInline BOOL GetBorders(_Out_writes_(3) int* pi) const
	{
		return (BOOL)SendMsg(SB_GETBORDERS, 0, (LPARAM)pi);
	}

	/// <summary>
	/// 取项目图标
	/// </summary>
	/// <param name="idx">项目索引，-1 = 简单状态栏的项目</param>
	/// <returns>图标句柄</returns>
	EckInline HICON GetIcon(int idx) const
	{
		return (HICON)SendMsg(SB_GETICON, idx, 0);
	}

	/// <summary>
	/// 取部件
	/// </summary>
	/// <param name="cParts">欲检索的部件数，若大于现有部件数，则返回实际部件数</param>
	/// <param name="pRights">接收部件右边客户区坐标的数组，若某元素设为-1，则表示该部件右边等于窗口右边</param>
	/// <returns>返回实际部件数</returns>
	EckInline int GetParts(int cParts, _Out_writes_opt_(cParts) int* pRights) const
	{
		return (int)SendMsg(SB_GETPARTS, cParts, (LPARAM)pRights);
	}

	EckInline BOOL GetRect(int idx, _Out_ RECT* prc) const
	{
		return (BOOL)SendMsg(SB_GETRECT, idx, (LPARAM)prc);
	}

#pragma warning(push)
#pragma warning(disable: 6054)// 可能未添加终止NULL
	/// <summary>
	/// 取部件文本
	/// </summary>
	/// <param name="idx">索引</param>
	/// <param name="pszBuf">缓冲区</param>
	/// <param name="pDrawFlags">返回绘制标志，SBT_常量</param>
	/// <returns>文本长度</returns>
	EckInline int GetPartText(int idx, _Out_ PWSTR pszBuf,
		_Out_opt_ int* pDrawFlags = nullptr) const
	{
		const auto lRet = SendMsg(SB_GETTEXTW, idx, (LPARAM)pszBuf);
		if (pDrawFlags)
			*pDrawFlags = HIWORD(lRet);
		return LOWORD(lRet);
	}
#pragma warning(pop)

	/// <summary>
	/// 取部件文本长度
	/// </summary>
	/// <param name="idx">索引</param>
	/// <param name="pDrawFlags">返回绘制标志，SBT_常量</param>
	/// <returns>文本长度</returns>
	EckInline int GetPartTextLength(int idx,
		_Out_opt_ int* pDrawFlags = nullptr) const
	{
		const auto lRet = SendMsg(SB_GETTEXTLENGTHW, idx, 0);
		if (pDrawFlags)
			*pDrawFlags = HIWORD(lRet);
		return LOWORD(lRet);
	}

	ECK_SUPPRESS_MISSING_ZERO_TERMINATION;
	EckInline void GetTipText(int idx, _Out_writes_(cchBuf) PWSTR pszBuf, int cchBuf) const
	{
		SendMsg(SB_GETTIPTEXTW, MAKEWPARAM(idx, cchBuf), (LPARAM)pszBuf);
	}

	EckInline BOOL IsSimple() const
	{
		return (BOOL)SendMsg(SB_ISSIMPLE, 0, 0);
	}

	EckInline COLORREF SetBkgColor(COLORREF cr) const
	{
		return (COLORREF)SendMsg(SB_SETBKCOLOR, 0, (LPARAM)cr);
	}

	/// <summary>
	/// 取项目图标
	/// </summary>
	/// <param name="idx">项目索引，-1 = 简单状态栏的项目</param>
	/// <param name="hIcon">图标句柄</param>
	/// <returns>成功返回TRUE，失败返回FALSE</returns>
	EckInline BOOL SetIcon(int idx, HICON hIcon) const
	{
		return (BOOL)SendMsg(SB_SETICON, idx, (LPARAM)hIcon);
	}

	EckInline void SetMinHeight(int cy) const
	{
		SendMsg(SB_SETMINHEIGHT, cy, 0);
	}

	/// <summary>
	/// 置部件
	/// </summary>
	/// <param name="cParts">部件数，不能大于256</param>
	/// <param name="pWidths">宽度数组，若某元素设为-1，则表示该部件宽度等于窗口宽度</param>
	/// <returns>成功返回TRUE，失败返回FALSE</returns>
	EckInline BOOL SetParts(int cParts, _In_reads_(cParts) int* pWidths) const
	{
		return (BOOL)SendMsg(SB_SETPARTS, cParts, (LPARAM)pWidths);
	}

	EckInline BOOL SetPartText(int idx, _In_z_ PCWSTR pszText, int iDrawFlags = 0) const
	{
		return (BOOL)SendMsg(SB_SETTEXTW, MAKEWORD(idx, iDrawFlags), (LPARAM)pszText);
	}

	EckInline BOOL SetPartData(int idx, LPARAM lParam) const
	{
		return (BOOL)SendMsg(SB_SETTEXTW, MAKEWORD(idx, SBT_OWNERDRAW), lParam);
	}

	EckInline void SetTipText(int idx, _In_z_ PCWSTR pszText) const
	{
		SendMsg(SB_SETTIPTEXTW, idx, (LPARAM)pszText);
	}

	EckInline void SetSimple(BOOL bSimple) const
	{
		SendMsg(SB_SIMPLE, bSimple, 0);
	}
};
ECK_RTTI_IMPL_BASE_INLINE(CStatusBar, CWnd);
ECK_NAMESPACE_END