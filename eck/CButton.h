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

// ��ť���ࡣ
// ����ֱ��ʵ��������
class CButton :public CWnd
{
protected:
	BITBOOL m_bShowTextAndImage : 1 = FALSE;
public:
	CButton() {}

	~CButton() {}

	/// <summary>
	/// ��ͼƬ�ı�ͬʱ��ʾ
	/// </summary>
	/// <param name="bShowTextAndImage">�Ƿ�ͬʱ��ʾ</param>
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
	/// ȡͼƬ�ı�ͬʱ��ʾ
	/// </summary>
	/// <returns>�Ƿ�ͬʱ��ʾ</returns>
	EckInline BOOL GetTextImageShowing() const
	{
		return m_bShowTextAndImage;
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

// ��ͨ��ť
class CPushButton :public CButton
{
public:
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData = NULL) override;

	CRefBin SerializeData(SIZE_T cbExtra = 0, SIZE_T* pcbSize = NULL) override;

	/// <summary>
	/// ������
	/// </summary>
	/// <param name="iType">���ͣ�0 - ��ͨ  1 - ���</param>
	void SetType(int iType);

	/// <summary>
	/// ȡ����
	/// </summary>
	/// <returns>���ͣ�0 - ��ͨ  1 - ���</returns>
	int GetType();

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
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData = NULL) override;

	CRefBin SerializeData(SIZE_T cbExtra = 0, SIZE_T* pcbSize = NULL) override;

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
	BITBOOL m_bShieldIcon : 1;
public:
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData = NULL) override;

	CRefBin SerializeData(SIZE_T cbExtra = 0, SIZE_T* pcbSize = NULL) override;

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
	CRefStrW GetNote();

	/// <summary>
	/// �ö���ͼ��
	/// </summary>
	/// <param name="bShieldIcon">�Ƿ�Ϊ����ͼ��</param>
	EckInline void SetShieldIcon(BOOL bShieldIcon)
	{
		m_bShieldIcon = bShieldIcon;
		SendMessageW(m_hWnd, BCM_SETSHIELD, 0, bShieldIcon);
	}

	/// <summary>
	/// ȡ����ͼ��
	/// </summary>
	/// <returns></returns>
	EckInline BOOL GetShieldIcon()
	{
		return m_bShieldIcon;// �������ֻ���ò���ȡ.....�Ѽ�¼��ֵ���ػ�ȥ��
	}

	/// <summary>
	/// ���Ƿ�Ĭ��
	/// </summary>
	/// <param name="bDef">�Ƿ�Ĭ��</param>
	void SetDef(BOOL bDef);

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