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

inline constexpr int
DATAVER_BUTTON_1 = 1,
DATAVER_PUSHBUTTON_1 = 1,
DATAVER_CHECKBUTTON_1 = 1,
DATAVER_COMMANDLINK_1 = 1;

#pragma pack(push, ECK_CTRLDATA_ALIGN)
struct CREATEDATA_PUSHBUTTON
{
	int iVer;
	BITBOOL bShowTextAndImage : 1;
};

struct CREATEDATA_CHECKBUTTON
{
	int iVer;
	BITBOOL bShowTextAndImage : 1;
	ECKENUM eCheckState;
};

struct CREATEDATA_COMMANDLINK
{
	int iVer;
	int cchNote;
	BITBOOL bShieldIcon : 1;
	// WCHAR szNote[];

	EckInline PCWSTR Note() const
	{
		if (cchNote)
			return (PCWSTR)PtrSkipType(this);
		else
			return NULL;
	}
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
	BITBOOL m_bShowTextAndImage : 1 = FALSE;
public:
	CButton() {}

	~CButton() {}

	/// <summary>
	/// 置图片文本同时显示
	/// </summary>
	/// <param name="bShowTextAndImage">是否同时显示</param>
	void SetTextImageShowing(BOOL bShowTextAndImage)
	{
		m_bShowTextAndImage = bShowTextAndImage;
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
		return m_bShowTextAndImage;
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
			if (m_bShowTextAndImage)
				ModifyStyle(0, BS_BITMAP);
			else
				ModifyStyle(BS_BITMAP, BS_BITMAP);
		else
			ModifyStyle(0, BS_BITMAP);

		SendMsg(BM_SETIMAGE, uType, (LPARAM)hImage);
	}

	EckInline void SetMultiLine(BOOL bMultiLine)
	{
		ModifyStyle(bMultiLine ? BS_MULTILINE : 0, BS_MULTILINE);
	}

	EckInline BOOL GetMultiLine()
	{
		return IsBitSet(GetStyle(), BS_MULTILINE);
	}
};                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            

// 普通按钮
class CPushButton :public CButton
{
public:
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData = NULL) override;

	CRefBin SerializeData(SIZE_T cbExtra = 0, SIZE_T* pcbSize = NULL) override;

	/// <summary>
	/// 置类型
	/// </summary>
	/// <param name="iType">类型，0 - 普通  1 - 拆分</param>
	void SetType(int iType);

	/// <summary>
	/// 取类型
	/// </summary>
	/// <returns>类型，0 - 普通  1 - 拆分</returns>
	int GetType();

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
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData = NULL) override;

	CRefBin SerializeData(SIZE_T cbExtra = 0, SIZE_T* pcbSize = NULL) override;

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
	BITBOOL m_bShieldIcon : 1;
public:
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData = NULL) override;

	CRefBin SerializeData(SIZE_T cbExtra = 0, SIZE_T* pcbSize = NULL) override;

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
	CRefStrW GetNote();

	/// <summary>
	/// 置盾牌图标
	/// </summary>
	/// <param name="bShieldIcon">是否为盾牌图标</param>
	EckInline void SetShieldIcon(BOOL bShieldIcon)
	{
		m_bShieldIcon = bShieldIcon;
		SendMessageW(m_hWnd, BCM_SETSHIELD, 0, bShieldIcon);
	}

	/// <summary>
	/// 取盾牌图标
	/// </summary>
	/// <returns></returns>
	EckInline BOOL GetShieldIcon()
	{
		return m_bShieldIcon;// 这个东西只能置不能取.....把记录的值返回回去吧
	}

	/// <summary>
	/// 置是否默认
	/// </summary>
	/// <param name="bDef">是否默认</param>
	void SetDef(BOOL bDef);

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