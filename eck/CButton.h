/*
* WinEzCtrlKit Library
*
* CButton.h �� ��׼��ť
* ������ť�����б���
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "CWnd.h"
#include "CSubclassMgr.h"
#include "CRefStr.h"
#include "Utility.h"

ECK_NAMESPACE_BEGIN
// ��������
struct CTRLINFO_BUTTON
{
	BOOL bShowTextAndImage;	// �Ƿ�ͬʱ��ʾͼƬ���ı�
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

// ��ť���ࡣ
// ����ֱ��ʵ��������
class CButton :public CWnd
{
protected:
	CTRLINFO_BUTTON m_Info{};

public:
	CButton() {}

	~CButton() {}

	/// <summary>
	/// ��ͼƬ�ı�ͬʱ��ʾ
	/// </summary>
	/// <param name="bShowTextAndImage">�Ƿ�ͬʱ��ʾ</param>
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
	/// ȡͼƬ�ı�ͬʱ��ʾ
	/// </summary>
	/// <returns>�Ƿ�ͬʱ��ʾ</returns>
	EckInline BOOL GetTextImageShowing() const
	{
		return m_Info.bShowTextAndImage;
	}

	/// <summary>
	/// �ö���
	/// </summary>
	/// <param name="bHAlign">�Ƿ�ˮƽ����</param>
	/// <param name="iAlign">���룬�μ����Զ���</param>
	void SetAlign(BOOL bHAlign, int iAlign);

	/// <summary>
	/// ȡ����
	/// </summary>
	/// <param name="bHAlign">�Ƿ�ˮƽ����</param>
	/// <returns>���룬�μ����Զ���</returns>
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

// ��ͨ��ť
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
	/// ������
	/// </summary>
	/// <param name="iType">���ͣ�0 - ��ͨ  1 - ���</param>
	void SetType(int iType);

	/// <summary>
	/// ȡ����
	/// </summary>
	/// <returns>���ͣ�0 - ��ͨ  1 - ���</returns>
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
	/// ���Ƿ�Ĭ��
	/// </summary>
	/// <param name="iDef">�Ƿ�Ĭ��</param>
	void SetDef(BOOL bDef);

	/// <summary>
	/// ȡ�Ƿ�Ĭ��
	/// </summary>
	EckInline BOOL GetDef()
	{
		DWORD dwStyle = GetStyle();
		return (IsBitSet(dwStyle, BS_DEFSPLITBUTTON) || IsBitSet(dwStyle, BS_DEFPUSHBUTTON));
	}
};

// ѡ���
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
		* ��һ��ר���ڵ�ѡ��ť��״̬����BST_DONTCLICK��
		* ���δ�������״̬����ô��ťÿ�λ�ý��㶼�����BN_CLICKED��
		* ����BM_SETDONTCLICK��������ֹ�¼���������
		*/
		SendMsg(BM_SETDONTCLICK, TRUE, 0);
		return m_hWnd;
	}

	/// <summary>
	/// ������
	/// </summary>
	/// <param name="iType">���ͣ�0 - ��ѡ��  1 - ��ѡ��  2 - ��̬��ѡ��</param>
	void SetType(int iType);

	/// <summary>
	/// ȡ����
	/// </summary>
	int GetType();

	/// <summary>
	/// �ü���״̬
	/// </summary>
	/// <param name="iState">״̬��0 - δѡ��  1 - ѡ��  2 - ��ѡ��</param>
	void SetCheckState(int iState);

	/// <summary>
	/// ȡ����״̬
	/// </summary>
	int GetCheckState();

	/// <summary>
	/// �ð�ť��ʽ
	/// </summary>
	/// <param name="bPushLike">�Ƿ�Ϊ��ť��ʽ</param>
	EckInline void SetPushLike(BOOL bPushLike)
	{
		ModifyStyle(bPushLike ? BS_PUSHLIKE : 0, BS_PUSHLIKE);
		Redraw();
	}

	/// <summary>
	/// ȡ��ť��ʽ
	/// </summary>
	/// <returns></returns>
	EckInline BOOL GetPushLike()
	{
		return IsBitSet(GetStyle(), BS_PUSHLIKE);
	}

	/// <summary>
	/// ��ƽ����ʽ
	/// </summary>
	/// <param name="bFlat">�Ƿ�Ϊƽ����ʽ</param>
	EckInline void SetFlat(BOOL bFlat)
	{
		ModifyStyle(bFlat ? BS_FLAT : 0, BS_FLAT);
		Redraw();
	}

	/// <summary>
	/// ȡƽ����ʽ
	/// </summary>
	EckInline BOOL GetFlat()
	{
		return IsBitSet(GetStyle(), BS_FLAT);
	}

	/// <summary>
	/// ���ı�����
	/// </summary>
	/// <param name="bLeftText">�Ƿ��ı�����</param>
	EckInline void SetLeftText(BOOL bLeftText)
	{
		ModifyStyle(bLeftText ? BS_LEFTTEXT : 0, BS_LEFTTEXT);
		Redraw();
	}

	/// <summary>
	/// ȡ�ı�����
	/// </summary>
	/// <returns></returns>
	EckInline BOOL GetLeftText()
	{
		return IsBitSet(GetStyle(), BS_LEFTTEXT);
	}
};

// ��������
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
	/// ��ע���ı�
	/// </summary>
	/// <param name="pszText">�ı�ָ��</param>
	/// <returns>�ɹ�����TRUE��ʧ�ܷ���FALSE</returns>
	EckInline BOOL SetNote(PCWSTR pszText)
	{
		return (BOOL)SendMsg(BCM_SETNOTE, 0, (LPARAM)pszText);
	}

	/// <summary>
	/// ȡע���ı���
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
	/// �ö���ͼ��
	/// </summary>
	/// <param name="bShieldIcon">�Ƿ�Ϊ����ͼ��</param>
	EckInline void SetShieldIcon(BOOL bShieldIcon)
	{
		m_InfoEx.bShieldIcon = bShieldIcon;
		SendMessageW(m_hWnd, BCM_SETSHIELD, 0, bShieldIcon);
	}

	/// <summary>
	/// ȡ����ͼ��
	/// </summary>
	/// <returns></returns>
	EckInline BOOL GetShieldIcon()
	{
		return m_InfoEx.bShieldIcon;// �������ֻ���ò���ȡ.....�Ѽ�¼��ֵ���ػ�ȥ��
	}

	/// <summary>
	/// ���Ƿ�Ĭ��
	/// </summary>
	/// <param name="bDef">�Ƿ�Ĭ��</param>
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
	/// ȡ�Ƿ�Ĭ��
	/// </summary>
	/// <returns>�Ƿ�Ĭ��</returns>
	EckInline BOOL GetDef()
	{
		DWORD dwStyle = GetStyle();
		return IsBitSet(dwStyle, BS_DEFCOMMANDLINK);
	}
};
ECK_NAMESPACE_END