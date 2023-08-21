/*
* WinEzCtrlKit Library
*
* CLabel.h �� ��ǩ
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "CWnd.h"
#include "CSubclassMgr.h"
#include "Utility.h"

#include <windowsx.h>

ECK_NAMESPACE_BEGIN
struct ECKLABELDATA
{
	int iBKPicMode;			// ��ͼģʽ��0 - ������  1 - ƽ��  2 - ����  3 - ����
	int iAlignH;			// �������
	int iAlignV;			// �������
	COLORREF crText;		// �ı���ɫ
	COLORREF crTextBK;		// �ı�������ɫ
	COLORREF crBK;			// ������ɫ
	/*
	* ���䱳��ģʽ����ѡֵ��
	* 0 - ��  1 - ���ϵ���  2 - ���µ���  3 - ������  4 - ���ҵ���
	* 5 - �����ϵ�����  6 - �����µ�����  7 - �����µ�����  8 - �����ϵ�����
	*/
	int iGradientMode;
	COLORREF crGradient[3];	// ���䱳����ɫ
	int iEllipsisMode;		// ʡ�Ժ�ģʽ��0 - ��  1 - ĩβʡ��  2 - ·��ʡ��  3 - ʡ�Ե���
	int iPrefixMode;		// ǰ׺ģʽ��0 - ����  1 - ������ǰ׺  2 - �����»���  3 - ֻ��ʾ�»���
	int iMousePassingThrough;	// ��괩͸��0 - ��  1 - ��͸�հ�����  2 - ��͸�����ؼ�
	BITBOOL bAutoWrap : 1;		// �Զ�����
	BITBOOL bFullWndPic : 1;	// ��ͼ���������ؼ�
	BITBOOL bTransparent : 1;	// ͸����ǩ
	BITBOOL bUxThemeText : 1;
};

class CLabel :public COwnWnd
{
private:
	WND_RECORDER_DECL(CLabel)

	ECKLABELDATA m_Info{};

	int m_cxClient = 0,
		m_cyClient = 0;			// �ͻ�����С
	HDC m_hCDC = NULL;			// ��̨����DC
	HDC m_hcdcHelper = NULL;	// ��λͼʹ�õĸ���DC
	HBITMAP m_hBitmap = NULL;	// ��̨����λͼ
	HGDIOBJ m_hOld1 = NULL;		// ��̨DC��λͼ���
	HGDIOBJ m_hOld2 = NULL;		// ����DC��λͼ���

	HBITMAP m_hbmBK = NULL;		// ��ͼ
	HBITMAP m_hbmPic = NULL;	// ͼƬ
	int m_cxBKPic = 0,
		m_cyBKPic = 0;			// ��ͼ��С

	int m_cxPic = 0,
		m_cyPic = 0;			// ͼƬ��С
	RECT m_rcPartPic{};			// �����ͼƬ����
	RECT m_rcPartText{};		// ������ı�����

	static ATOM m_atomLabel;	// ��ǩ��ԭ��

	/// <summary>
	/// ���Ʊ�ǩ��
	/// ��m_Info.bTransparentΪTRUE�����Զ����ü���DC���ؼ�����
	/// </summary>
	/// <param name="hDC">Ŀ��DC������֮ǰ���Ա����������</param>
	void Paint(HDC hDC);

	/// <summary>
	/// ���㲿������
	/// </summary>
	void CalcPartsRect();

	/// <summary>
	/// ���ⲿDC���ԡ�
	/// ������ʹ��SaveDC����DC״̬��Ȼ����ݿؼ���������DC
	/// </summary>
	/// <param name="hDC">Ŀ��DC</param>
	void SetDCAttr(HDC hDC);

	/// <summary>
	/// �ؼ����ڹ���
	/// </summary>
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
	static ATOM RegisterWndClass(HINSTANCE hInstance);

	EckInline HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData = NULL)
	{
		dwStyle |= WS_CHILD;
		m_hWnd = CreateWindowExW(dwExStyle, WCN_LABEL, pszText, dwStyle,
			x, y, cx, cy, hParent, i32ToP<HMENU>(nID), g_hInstance, this);
		m_Recorder.AddRecord(m_hWnd, this);
		return m_hWnd;
	}

	CLabel()
	{
		m_Info.iAlignV = 1;
		m_Info.crBK = CLR_DEFAULT;
		m_Info.crTextBK = CLR_DEFAULT;
		m_Info.crGradient[0] = 0x808080;
		m_Info.crGradient[1] = 0xFFFFFF;
		m_Info.crGradient[2] = 0x808080;
	}

	~CLabel()
	{
		DeleteObject(m_hbmBK);
	}

	void Redraw();

	/// <summary>
	/// �ñ���ͼƬ
	/// </summary>
	/// <param name="hBitmap">λͼ���</param>
	HBITMAP SetBKPic(HBITMAP hBitmap);

	/// <summary>
	/// ȡ����ͼƬ
	/// </summary>
	/// <returns>ͼƬ���</returns>
	EckInline HBITMAP GetBKPic()
	{
		return m_hbmBK;
	}

	/// <summary>
	/// ��ͼƬ
	/// </summary>
	/// <param name="hBitmap">λͼ���</param>
	HBITMAP SetPic(HBITMAP hBitmap);

	EckInline HBITMAP GetPic()
	{
		return m_hbmPic;
	}

	/// <summary>
	/// �õ�ͼ��ʽ
	/// </summary>
	EckInline void SetBKPicMode(int iBKPicMode)
	{
		m_Info.iBKPicMode = iBKPicMode;
		Redraw();
	}

	EckInline int GetBKPicMode()
	{
		return m_Info.iBKPicMode;
	}

	/// <summary>
	/// �ö���
	/// </summary>
	/// <param name="bHAlign">�Ƿ�Ϊ�������</param>
	/// <param name="iAlign">���뷽ʽ</param>
	EckInline void SetAlign(BOOL bHAlign, int iAlign)
	{
		if (bHAlign)
			m_Info.iAlignH = iAlign;
		else
			m_Info.iAlignV = iAlign;
		CalcPartsRect();
		Redraw();
	}

	EckInline int GetAlign(BOOL bHAlign) const
	{
		if (bHAlign)
			return m_Info.iAlignH;
		else
			return m_Info.iAlignV;
	}

	/// <summary>
	/// ���Զ�����
	/// </summary>
	/// <param name="bAutoWrap"></param>
	/// <returns></returns>
	EckInline void SetAutoWrap(BOOL bAutoWrap)
	{
		m_Info.bAutoWrap = bAutoWrap;
		CalcPartsRect();
		Redraw();
	}

	EckInline BOOL GetAutoWrap() const
	{
		return m_Info.bAutoWrap;
	}

	/// <summary>
	/// ����ɫ
	/// </summary>
	/// <param name="idx">0 = �ı���ɫ  1 = ����  2 = �ı�����</param>
	/// <param name="cr">��ɫ</param>
	void SetClr(int idx, COLORREF cr);

	EckInline COLORREF GetClr(int idx) const
	{
		switch (idx)
		{
		case 0:return m_Info.crText;
		case 1:return m_Info.crBK;
		case 2:return m_Info.crTextBK;
		}
		assert(FALSE);
		return 0;
	}

	/// <summary>
	/// �ý��䷽ʽ
	/// </summary>
	EckInline void SetGradientMode(int iGradientMode)
	{
		m_Info.iGradientMode = iGradientMode;
		Redraw();
	}

	EckInline int GetGradientMode() const
	{
		return m_Info.iGradientMode;
	}

	/// <summary>
	/// �ý�����ɫ
	/// </summary>
	EckInline void SetGradientClr(int idx, COLORREF cr)
	{
		m_Info.crGradient[idx] = cr;
		Redraw();
	}

	EckInline COLORREF GetGradientClr(int idx) const
	{
		return m_Info.crGradient[idx];
	}

	/// <summary>
	/// ��ʡ�Ժ�ģʽ
	/// </summary>
	/// <param name="iEllipsisMode"></param>
	/// <returns></returns>
	EckInline void SetEllipsisMode(int iEllipsisMode)
	{
		m_Info.iEllipsisMode = iEllipsisMode;
		CalcPartsRect();
		Redraw();
	}

	EckInline int GetEllipsisMode() const
	{
		return m_Info.iEllipsisMode;
	}

	/// <summary>
	/// ��ǰ׺����ģʽ
	/// </summary>
	/// <param name="iPrefixMode"></param>
	/// <returns></returns>
	EckInline void SetPrefixMode(int iPrefixMode)
	{
		m_Info.iPrefixMode = iPrefixMode;
		CalcPartsRect();
		Redraw();
	}

	EckInline int GetPrefixMode() const
	{
		return m_Info.iPrefixMode;
	}

	/// <summary>
	/// �õ�ͼ��������
	/// </summary>
	EckInline void SetFullWndPic(BOOL bFullWndPic)
	{
		m_Info.bFullWndPic = bFullWndPic;
		Redraw();
	}

	EckInline BOOL GetFullWndPic() const
	{
		return m_Info.bFullWndPic;
	}

	/// <summary>
	/// ��͸����ǩ
	/// </summary>
	EckInline void SetTransparent(BOOL bTransparent)
	{
		m_Info.bTransparent = bTransparent;
		ModifyStyle(bTransparent ? WS_EX_TRANSPARENT : 0, WS_EX_TRANSPARENT, GWL_EXSTYLE);
		Redraw();
	}

	EckInline BOOL GetTransparent() const
	{
		return m_Info.bTransparent;
	}

	/// <summary>
	/// ����괩͸
	/// </summary>
	/// <param name="iMousePassingThrough"></param>
	/// <returns></returns>
	EckInline void SetMousePassingThrough(int iMousePassingThrough)
	{
		m_Info.iMousePassingThrough = iMousePassingThrough;
		Redraw();
	}

	EckInline int GetMousePassingThrough() const
	{
		return m_Info.iMousePassingThrough;
	}
};
ECK_NAMESPACE_END