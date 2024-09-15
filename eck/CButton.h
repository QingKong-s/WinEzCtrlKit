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
			return nullptr;
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
	BOOL m_bShowTextAndImage = FALSE;
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
	EckInline BOOL GetTextImageShowing() const { return m_bShowTextAndImage; }

	/// <summary>
	/// 置对齐
	/// </summary>
	/// <param name="bHAlign">是否水平对齐</param>
	/// <param name="iAlign">对齐，参见属性定义</param>
	void SetAlign(BOOL bHAlign, Align iAlign)
	{
		DWORD dwStyle = GetStyle();
		if (bHAlign)
		{
			dwStyle &= (~(BS_LEFT | BS_CENTER | BS_RIGHT));
			switch (iAlign)
			{
			case Align::Near: dwStyle |= BS_LEFT; break;
			case Align::Center: dwStyle |= BS_CENTER; break;
			case Align::Far: dwStyle |= BS_RIGHT; break;
			}
		}
		else
		{
			dwStyle &= (~(BS_TOP | BS_VCENTER | BS_BOTTOM));
			switch (iAlign)
			{
			case Align::Near: dwStyle |= BS_TOP; break;
			case Align::Center: dwStyle |= BS_VCENTER; break;
			case Align::Far: dwStyle |= BS_BOTTOM; break;
			}
		}
		SetStyle(dwStyle);
		Redraw();
	}

	/// <summary>
	/// 取对齐
	/// </summary>
	/// <param name="bHAlign">是否水平对齐</param>
	/// <returns>对齐，参见属性定义</returns>
	Align GetAlign(BOOL bHAlign)
	{
		DWORD dwStyle = GetStyle();
		if (bHAlign)
		{
			if (IsBitSet(dwStyle, BS_CENTER))
				return Align::Center;
			else if (IsBitSet(dwStyle, BS_RIGHT))
				return Align::Far;
			else
				return Align::Near;
		}
		else
		{
			if (IsBitSet(dwStyle, BS_VCENTER))
				return Align::Center;
			else if (IsBitSet(dwStyle, BS_BOTTOM))
				return Align::Far;
			else
				return Align::Near;
		}
	}

	EckInline void SetImage(HANDLE hImage, UINT uType)
	{
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
	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = nullptr) override
	{
		if (pData)
		{
			auto pBase = (const CTRLDATA_WND*)pData;
			auto p = (const CREATEDATA_PUSHBUTTON*)SkipBaseData(pData);
			if (pBase->iVer != CDV_WND_1)
			{
				EckDbgBreak();
				return nullptr;
			}

			m_hWnd = IntCreate(pBase->dwExStyle, WC_BUTTONW, pBase->Text(), pBase->dwStyle,
				x, y, cx, cy, hParent, hMenu, nullptr, nullptr);

			switch (p->iVer)
			{
			case DATAVER_PUSHBUTTON_1:
				SetTextImageShowing(p->bShowTextAndImage);
				break;
			default:
				EckDbgBreak();
				break;
			}
		}
		else
		{
			dwStyle |= WS_CHILD;
			m_hWnd = IntCreate(dwExStyle, WC_BUTTONW, pszText, dwStyle,
				x, y, cx, cy, hParent, hMenu, nullptr, nullptr);
		}

		return m_hWnd;
	}

	void SerializeData(CRefBin& rb) override
	{
		const SIZE_T cbSize = sizeof(CREATEDATA_PUSHBUTTON);
		CWnd::SerializeData(rb);
		CMemWriter w(rb.PushBack(cbSize), cbSize);

		CREATEDATA_PUSHBUTTON* p;
		w.SkipPointer(p);
		p->iVer = DATAVER_PUSHBUTTON_1;
		p->bShowTextAndImage = GetTextImageShowing();
	}

	/// <summary>
	/// 置类型
	/// </summary>
	/// <param name="iType">类型，0 - 普通  1 - 拆分</param>
	void SetType(int iType)
	{
		BOOL bDef = GetDef();
		DWORD dwStyle = GetStyle() & ~(BS_PUSHBUTTON | BS_SPLITBUTTON | BS_DEFPUSHBUTTON | BS_DEFSPLITBUTTON);

		switch (iType)
		{
		case 0:
			dwStyle |= (bDef ? BS_DEFPUSHBUTTON : BS_PUSHBUTTON);
			break;
		case 1:
			dwStyle |= (bDef ? BS_DEFSPLITBUTTON : BS_SPLITBUTTON);
			break;
		}

		SetStyle(dwStyle);
		Redraw();
	}

	/// <summary>
	/// 取类型
	/// </summary>
	/// <returns>类型，0 - 普通  1 - 拆分</returns>
	int GetType()
	{
		DWORD dwStyle = GetStyle();
		if (IsBitSet(dwStyle, BS_SPLITBUTTON) || IsBitSet(dwStyle, BS_DEFSPLITBUTTON))
			return 1;
		else if (IsBitSet(dwStyle, BS_PUSHBUTTON) || IsBitSet(dwStyle, BS_DEFPUSHBUTTON))
			return 0;
		return -1;
	}

	/// <summary>
	/// 置是否默认
	/// </summary>
	/// <param name="iDef">是否默认</param>
	void SetDef(BOOL bDef)
	{
		int iType = GetType();
		DWORD dwStyle = GetStyle() & ~(BS_DEFPUSHBUTTON | BS_DEFSPLITBUTTON | BS_PUSHBUTTON | BS_SPLITBUTTON);
		if (bDef)
			if (iType)
				dwStyle |= BS_DEFSPLITBUTTON;
			else
				dwStyle |= BS_DEFPUSHBUTTON;
		else
			if (iType)
				dwStyle |= BS_SPLITBUTTON;
			else
				dwStyle |= BS_PUSHBUTTON;

		SetStyle(dwStyle);
	}

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
	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = nullptr) override
	{
		if (pData)
		{
			auto pBase = (const CTRLDATA_WND*)pData;
			auto p = (const CREATEDATA_CHECKBUTTON*)SkipBaseData(pData);
			if (pBase->iVer != CDV_WND_1)
			{
				EckDbgBreak();
				return nullptr;
			}

			m_hWnd = IntCreate(pBase->dwExStyle, WC_BUTTONW, pBase->Text(), pBase->dwStyle,
				x, y, cx, cy, hParent, hMenu, nullptr, nullptr);

			switch (p->iVer)
			{
			case DATAVER_CHECKBUTTON_1:
				SetTextImageShowing(p->bShowTextAndImage);
				SetCheckState(p->eCheckState);
				break;
			default:
				EckDbgBreak();
				break;
			}
		}
		else
		{
			dwStyle |= WS_CHILD;
			m_hWnd = IntCreate(dwExStyle, WC_BUTTONW, pszText, dwStyle,
				x, y, cx, cy, hParent, hMenu, nullptr, nullptr);
		}

		/*
		* 有一个专用于单选按钮的状态叫做BST_DONTCLICK，
		* 如果未设置这个状态，那么按钮每次获得焦点都会产生BN_CLICKED，
		* 发送BM_SETDONTCLICK设置它防止事件错误生成
		*/
		SendMsg(BM_SETDONTCLICK, TRUE, 0);
		return m_hWnd;
	}

	void SerializeData(CRefBin& rb) override
	{
		const SIZE_T cbSize = sizeof(CREATEDATA_CHECKBUTTON);
		CWnd::SerializeData(rb);
		CMemWriter w(rb.PushBack(cbSize), cbSize);

		CREATEDATA_CHECKBUTTON* p;
		w.SkipPointer(p);
		p->iVer = DATAVER_CHECKBUTTON_1;
		p->bShowTextAndImage = GetTextImageShowing();
		p->eCheckState = GetCheckState();
	}

	/// <summary>
	/// 置类型
	/// </summary>
	/// <param name="iType">类型，0 - 单选框  1 - 复选框  2 - 三态复选框</param>
	void SetType(int iType)
	{
		DWORD dwStyle = GetStyle() & ~(BS_AUTORADIOBUTTON | BS_AUTOCHECKBOX | BS_AUTO3STATE);
		switch (iType)
		{
		case 0:dwStyle |= BS_AUTORADIOBUTTON; break;
		case 1:dwStyle |= BS_AUTOCHECKBOX; break;
		case 2:dwStyle |= BS_AUTO3STATE; break;
		default:assert(FALSE); break;
		}
		SetStyle(dwStyle);
		Redraw();
	}

	/// <summary>
	/// 取类型
	/// </summary>
	int GetType()
	{
		DWORD dwStyle = GetStyle();
		if (IsBitSet(dwStyle, BS_AUTORADIOBUTTON))
			return 0;
		else if (IsBitSet(dwStyle, BS_AUTOCHECKBOX))
			return 1;
		else if (IsBitSet(dwStyle, BS_AUTO3STATE))
			return 2;
		else
			return -1;
	}

	/// <summary>
	/// 置检查框状态
	/// </summary>
	/// <param name="iState">状态，0 - 未选中  1 - 选中  2 - 半选中</param>
	void SetCheckState(int iState)
	{
		UINT uState;
		switch (iState)
		{
		case 0:uState = BST_UNCHECKED; break;
		case 1:uState = BST_CHECKED; break;
		case 2:uState = BST_INDETERMINATE; break;
		default:assert(FALSE); break;
		}
		SendMsg(BM_SETCHECK, uState, 0);
	}

	/// <summary>
	/// 取检查框状态
	/// </summary>
	int GetCheckState()
	{
		UINT uState = (UINT)SendMsg(BM_GETCHECK, 0, 0);
		if (IsBitSet(uState, BST_CHECKED))
			return 1;
		else if (IsBitSet(uState, BST_INDETERMINATE))
			return 2;
		else
			return 0;
	}

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
	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = nullptr) override
	{
		if (pData)
		{
			auto pBase = (const CTRLDATA_WND*)pData;
			auto p = (const CREATEDATA_COMMANDLINK*)SkipBaseData(pData);
			if (pBase->iVer != CDV_WND_1)
			{
				EckDbgBreak();
				return nullptr;
			}

			m_hWnd = IntCreate(pBase->dwExStyle, WC_BUTTONW, pBase->Text(), pBase->dwStyle,
				x, y, cx, cy, hParent, hMenu, nullptr, nullptr);

			switch (p->iVer)
			{
			case DATAVER_COMMANDLINK_1:
				SetShieldIcon(p->bShieldIcon);
				SetNote(p->Note());
				break;
			default:
				EckDbgBreak();
				break;
			}
		}
		else
		{
			dwStyle &= ~(BS_PUSHBUTTON | BS_DEFPUSHBUTTON | BS_SPLITBUTTON | BS_DEFSPLITBUTTON | BS_CHECKBOX);
			dwStyle |= WS_CHILD;
			if (!IsBitSet(dwStyle, BS_COMMANDLINK | BS_DEFCOMMANDLINK))
				dwStyle |= BS_COMMANDLINK;
			m_hWnd = IntCreate(dwExStyle, WC_BUTTONW, pszText, dwStyle,
				x, y, cx, cy, hParent, hMenu, nullptr, nullptr);
		}

		return m_hWnd;
	}

	void SerializeData(CRefBin&rb) override
	{
		auto rsNote = GetNote();
		const SIZE_T cbSize = sizeof(CREATEDATA_COMMANDLINK) + rsNote.ByteSize();
		CWnd::SerializeData(rb);
		CMemWriter w(rb.PushBack(cbSize), cbSize);
		CREATEDATA_COMMANDLINK* p;
		w.SkipPointer(p);
		p->iVer = DATAVER_COMMANDLINK_1;
		p->cchNote = rsNote.Size();
		p->bShieldIcon = GetShieldIcon();

		w << rsNote;
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
	CRefStrW GetNote()
	{
		CRefStrW rs;
		int cch = (int)SendMsg(BCM_GETNOTELENGTH, 0, 0);
		if (cch)
		{
			rs.ReSize(cch);
			++cch;
			SendMsg(BCM_GETNOTE, (WPARAM)&cch, (LPARAM)rs.Data());
		}
		return rs;
	}

	BOOL GetNote(PWSTR pszBuf, int& cchBuf)
	{
		return (BOOL)SendMsg(BCM_GETNOTE, (WPARAM)&cchBuf, (LPARAM)pszBuf);
	}

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