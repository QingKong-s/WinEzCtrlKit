/*
* WinEzCtrlKit Library
*
* CButton.h ： 标准按钮
* 包含按钮的所有变体
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "CWnd.h"
#include "CSubclassMgr.h"
#include "CRefStr.h"
#include "Utility.h"

ECK_NAMESPACE_BEGIN
// 基础数据
struct CTRLINFO_BUTTON
{
	BOOL bShowTextAndImage;	// 是否同时显示图片和文本
};

struct CTRLINFO_COMMANDLINK
{
	BOOL bShieldIcon;
};
#pragma pack(push, ECK_CTRLDATA_ALIGN)

inline constexpr int
DATAVER_BUTTON_1 = 1,
DATAVER_PUSHBUTTON_1 = 1,
DATAVER_CHECKBUTTON_1 = 1,
DATAVER_COMMANDLINK_1 = 1;

struct CREATEDATA_BUTTON
{
	int iVer;
	BOOL bShowTextAndImage;
	ECKENUM iAlignH;
	ECKENUM iAlignV;
	int cchText;
};

struct CREATEDATA_PUSHBUTTON :CREATEDATA_BUTTON
{
	int iVer;
};

struct CREATEDATA_CHECKBUTTON :CREATEDATA_BUTTON
{
	int iVer;
};

struct CREATEDATA_COMMANDLINK :CREATEDATA_BUTTON
{
	int iVer;
};

#ifdef ECK_CTRL_DESIGN_INTERFACE
struct DESIGNDATA_PUSHBUTTON
{

};

#endif
#pragma pack(pop)

// 按钮基类。
// 请勿直接实例化此类
class CButton :public CWnd
{
protected:
	CTRLINFO_BUTTON m_Info{};

public:
	CButton() {}

	~CButton() {}

	/// <summary>
	/// 置图片文本同时显示
	/// </summary>
	/// <param name="bShowTextAndImage">是否同时显示</param>
	void SetTextImageShowing(BOOL bShowTextAndImage)
	{
		m_Info.bShowTextAndImage = bShowTextAndImage;
		DWORD dwStyle = GetStyle();
		if (bShowTextAndImage)
			dwStyle &= ~(BS_BITMAP);
		else if (SendMsg(BM_GETIMAGE, IMAGE_BITMAP, 0) || SendMsg(BM_GETIMAGE, IMAGE_ICON, 0))
			dwStyle |= BS_BITMAP;
		SetStyle(dwStyle);
	}

	/// <summary>
	/// 取图片文本同时显示
	/// </summary>
	/// <returns>是否同时显示</returns>
	EckInline BOOL GetTextImageShowing() const
	{
		return m_Info.bShowTextAndImage;
	}

	/// <summary>
	/// 置对齐
	/// </summary>
	/// <param name="bHAlign">是否水平对齐</param>
	/// <param name="iAlign">对齐，参见属性定义</param>
	void SetAlign(BOOL bHAlign, int iAlign);

	/// <summary>
	/// 取对齐
	/// </summary>
	/// <param name="bHAlign">是否水平对齐</param>
	/// <returns>对齐，参见属性定义</returns>
	int GetAlign(BOOL bHAlign);

	EckInline void SetImage(HANDLE hImage, UINT uType)
	{
		if (hImage)
			if (m_Info.bShowTextAndImage)
				ModifyStyle(0, BS_BITMAP);
			else
				ModifyStyle(BS_BITMAP, BS_BITMAP);
		else
			ModifyStyle(0, BS_BITMAP);

		SendMsg(BM_SETIMAGE, uType, (LPARAM)hImage);
	}
};

// 普通按钮
class CPushButton :public CButton
{
public:
	EckInline HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, UINT nID)
	{
		dwStyle &= ~(BS_CHECKBOX | BS_COMMANDLINK | BS_DEFCOMMANDLINK);
		dwStyle |= WS_CHILD;
		if (!IsBitSet(dwStyle, BS_PUSHBUTTON | BS_DEFPUSHBUTTON | BS_SPLITBUTTON | BS_DEFSPLITBUTTON))
			dwStyle |= BS_PUSHBUTTON;
		m_hWnd = CreateWindowExW(dwExStyle, WC_BUTTONW, pszText, dwStyle,
			x, y, cx, cy, hParent, i32ToP<HMENU>(nID), NULL, NULL);
		return m_hWnd;
	}

	/// <summary>
	/// 置类型
	/// </summary>
	/// <param name="iType">类型，0 - 普通  1 - 拆分</param>
	void SetType(int iType);

	/// <summary>
	/// 取类型
	/// </summary>
	/// <returns>类型，0 - 普通  1 - 拆分</returns>
	EckInline int GetType()
	{
		DWORD dwStyle = GetStyle();
		if (IsBitSet(dwStyle, BS_PUSHBUTTON) || IsBitSet(dwStyle, BS_DEFPUSHBUTTON))
			return 0;
		else if (IsBitSet(dwStyle, BS_SPLITBUTTON) || IsBitSet(dwStyle, BS_DEFSPLITBUTTON))
			return 1;
		return -1;
	}

	/// <summary>
	/// 置是否默认
	/// </summary>
	/// <param name="iDef">是否默认</param>
	void SetDef(BOOL bDef);

	/// <summary>
	/// 取是否默认
	/// </summary>
	EckInline BOOL GetDef()
	{
		DWORD dwStyle = GetStyle();
		return (IsBitSet(dwStyle, BS_DEFSPLITBUTTON) || IsBitSet(dwStyle, BS_DEFPUSHBUTTON));
	}
};

// 选择框
class CCheckButton :public CButton
{
public:
	EckInline HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, UINT nID)
	{
		dwStyle &= ~(BS_PUSHBUTTON | BS_DEFPUSHBUTTON | BS_SPLITBUTTON | BS_DEFSPLITBUTTON | BS_COMMANDLINK | BS_DEFCOMMANDLINK);
		dwStyle |= WS_CHILD;
		if (!IsBitSet(dwStyle, BS_RADIOBUTTON | BS_AUTORADIOBUTTON |
			BS_CHECKBOX | BS_AUTOCHECKBOX | BS_3STATE | BS_AUTO3STATE))
			dwStyle |= BS_AUTORADIOBUTTON;
		m_hWnd = CreateWindowExW(dwExStyle, WC_BUTTONW, pszText, dwStyle,
			x, y, cx, cy, hParent, i32ToP<HMENU>(nID), NULL, NULL);
		/*
		* 有一个专用于单选按钮的状态叫做BST_DONTCLICK，
		* 如果未设置这个状态，那么按钮每次获得焦点都会产生BN_CLICKED，
		* 发送BM_SETDONTCLICK设置它防止事件错误生成
		*/
		SendMsg(BM_SETDONTCLICK, TRUE, 0);
		return m_hWnd;
	}

	/// <summary>
	/// 置类型
	/// </summary>
	/// <param name="iType">类型，0 - 单选框  1 - 复选框  2 - 三态复选框</param>
	void SetType(int iType);

	/// <summary>
	/// 取类型
	/// </summary>
	int GetType();

	/// <summary>
	/// 置检查框状态
	/// </summary>
	/// <param name="iState">状态，0 - 未选中  1 - 选中  2 - 半选中</param>
	void SetCheckState(int iState);

	/// <summary>
	/// 取检查框状态
	/// </summary>
	int GetCheckState();

	/// <summary>
	/// 置按钮形式
	/// </summary>
	/// <param name="bPushLike">是否为按钮形式</param>
	EckInline void SetPushLike(BOOL bPushLike)
	{
		ModifyStyle(bPushLike ? BS_PUSHLIKE : 0, BS_PUSHLIKE);
		Redraw();
	}

	/// <summary>
	/// 取按钮形式
	/// </summary>
	/// <returns></returns>
	EckInline BOOL GetPushLike()
	{
		return IsBitSet(GetStyle(), BS_PUSHLIKE);
	}

	/// <summary>
	/// 置平面形式
	/// </summary>
	/// <param name="bFlat">是否为平面形式</param>
	EckInline void SetFlat(BOOL bFlat)
	{
		ModifyStyle(bFlat ? BS_FLAT : 0, BS_FLAT);
		Redraw();
	}

	/// <summary>
	/// 取平面形式
	/// </summary>
	EckInline BOOL GetFlat()
	{
		return IsBitSet(GetStyle(), BS_FLAT);
	}

	/// <summary>
	/// 置文本居左
	/// </summary>
	/// <param name="bLeftText">是否文本居左</param>
	EckInline void SetLeftText(BOOL bLeftText)
	{
		ModifyStyle(bLeftText ? BS_LEFTTEXT : 0, BS_LEFTTEXT);
		Redraw();
	}

	/// <summary>
	/// 取文本居左
	/// </summary>
	/// <returns></returns>
	EckInline BOOL GetLeftText()
	{
		return IsBitSet(GetStyle(), BS_LEFTTEXT);
	}
};

// 命令链接
class CCommandLink :public CButton
{
private:
	CTRLINFO_COMMANDLINK m_InfoEx{};
public:
	EckInline HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, UINT nID)
	{
		dwStyle &= ~(BS_PUSHBUTTON | BS_DEFPUSHBUTTON | BS_SPLITBUTTON | BS_DEFSPLITBUTTON | BS_CHECKBOX);
		dwStyle |= WS_CHILD;
		if (!IsBitSet(dwStyle, BS_COMMANDLINK | BS_DEFCOMMANDLINK))
			dwStyle |= BS_COMMANDLINK;
		m_hWnd = CreateWindowExW(dwExStyle, WC_BUTTONW, pszText, dwStyle,
			x, y, cx, cy, hParent, i32ToP<HMENU>(nID), NULL, NULL);
		return m_hWnd;
	}

	/// <summary>
	/// 置注释文本
	/// </summary>
	/// <param name="pszText">文本指针</param>
	/// <returns>成功返回TRUE，失败返回FALSE</returns>
	EckInline BOOL SetNote(PCWSTR pszText)
	{
		return (BOOL)SendMsg(BCM_SETNOTE, 0, (LPARAM)pszText);
	}

	/// <summary>
	/// 取注释文本。
	/// </summary>
	EckInline CRefStrW GetNote()
	{
		CRefStrW rs;
		int cch = (int)SendMsg(BCM_GETNOTELENGTH, 0, 0);
		if (cch)
		{
			rs.ReSize(cch);
			SendMsg(BCM_GETNOTE, (WPARAM)(cch + 1), (LPARAM)rs.m_pszText);
		}
		return rs;
	}

	/// <summary>
	/// 置盾牌图标
	/// </summary>
	/// <param name="bShieldIcon">是否为盾牌图标</param>
	EckInline void SetShieldIcon(BOOL bShieldIcon)
	{
		m_InfoEx.bShieldIcon = bShieldIcon;
		SendMessageW(m_hWnd, BCM_SETSHIELD, 0, bShieldIcon);
	}

	/// <summary>
	/// 取盾牌图标
	/// </summary>
	/// <returns></returns>
	EckInline BOOL GetShieldIcon()
	{
		return m_InfoEx.bShieldIcon;// 这个东西只能置不能取.....把记录的值返回回去吧
	}

	/// <summary>
	/// 置是否默认
	/// </summary>
	/// <param name="bDef">是否默认</param>
	void SetDef(BOOL bDef)
	{
		DWORD dwStyle = GetStyle() & ~(BS_DEFPUSHBUTTON | BS_PUSHBUTTON | BS_DEFCOMMANDLINK | BS_COMMANDLINK);
		if (bDef)
			dwStyle |= BS_DEFCOMMANDLINK;
		else
			dwStyle |= BS_COMMANDLINK;

		SetStyle(dwStyle);
	}

	/// <summary>
	/// 取是否默认
	/// </summary>
	/// <returns>是否默认</returns>
	EckInline BOOL GetDef()
	{
		DWORD dwStyle = GetStyle();
		return IsBitSet(dwStyle, BS_DEFCOMMANDLINK);
	}
};
ECK_NAMESPACE_END