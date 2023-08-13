/*
* WinEzCtrlKit Library
*
* DlgHelper.h �� �Ի�����ذ�������
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "ECK.h"
#include "CRefStr.h"
#include "CRefBin.h"
#include "Utility.h"

#include <vector>
#include <variant>
#include <optional>

ECK_NAMESPACE_BEGIN

#pragma pack(push, 2)
struct DLGTHEADER
{
	WORD      wVer;// ����Ϊ1;
	WORD      wSignature;// ����Ϊ0xFFFF;
	DWORD     idHelp;
	DWORD     dwExStyle;
	DWORD     dwStyle;
	WORD      cDlgItems;
	short     x;
	short     y;
	short     cx;
	short     cy;
};

/*
DLGTHEADER������½ṹ
struct DLGTMENU
{
	WCHAR ch[];// ch[0] == 0 - �޲˵���ch[1]  0xFFFF - ��ԴID��ch[2]  ���� - ��NULL��β����Դ���ƣ�ch[n]
};

struct DLGTWCLS
{
	WCHAR ch[];// ch[0] == 0 - ��׼�Ի����࣬ch[1]  0xFFFF - Ԥ������ԭ�ӣ�ch[2]  ���� - ��NULL��β�Ĵ���������ch[n]
};

struct DLGTCAPTION
{
	WCHAR ch[];// ch[0] == 0 - �ޱ��⣬ch[1]  ���� - ��NULL��β�ı��⣬ch[n]
};
�����������������WORD�߽��϶���

struct DLGTFONT// ����ָ����DS_SETFONT��DS_SHELLFONTʱ����
{
	WORD      wPoint;
	WORD      wWeight;
	BYTE      bItalic;
	BYTE      byCharSet;
	WCHAR     szName[];// ��NULL��β����������
};
*/


// ÿ��DLGITEMTHEADER��������DWORD�߽��϶���

struct DLGITEMTHEADER
{
	DWORD     idHelp;
	DWORD     dwExStyle;
	DWORD     dwStyle;
	short     x;
	short     y;
	short     cx;
	short     cy;
	DWORD     id;
};

/*
DLGITEMTHEADER������½ṹ
struct DLGITEMTWCLS
{
	WCHAR ch[];// ch[0] == 0xFFFF - Ԥ������ԭ�ӣ�ch[2]  ���� - ��NULL��β�Ĵ���������ch[n]
};

struct DLGITEMTCAPTION
{
	WCHAR ch[];// ch[0] == 0xFFFF - ��ԴID��ch[2]  ���� - ��NULL��β�ı��⣬ch[n]
};

struct DLGITEMTEXTRA
{
	WORD cbExtra;
};// ��Ӹ�������
*/

struct DLGTDLG :public DLGTHEADER
{
	std::optional<std::variant<WORD, CRefStrW>> Menu;
	std::optional<std::variant<ATOM, CRefStrW>> Class;
	std::optional<CRefStrW> Caption;
	struct
	{
		WORD      wPoint;
		WORD      wWeight;
		BYTE      bItalic;
		BYTE      byCharSet;
	} Font;
	CRefStrW rsFontName;
};

struct DLGTITEM :public DLGITEMTHEADER
{
	std::variant<ATOM, CRefStrW> Class;
	std::variant<WORD, CRefStrW> Caption;
	CRefBin rbExtra;
};
#pragma pack(pop)

enum
{
	// ���ߴ絥λ��Ϊ���أ�������DLU
	CWFDT_USEPIXELUNIT = 1u << 0
};

/// <summary>
/// ȡ�׸���ͣ������ؼ���
/// ����ö�ٴ��ڵ��Ӵ��ڲ����ص�һ���ɼ����ǽ����Ҿ���WS_TABSTOP�Ŀؼ����
/// </summary>
/// <param name="hWnd">������</param>
/// <returns>�ؼ����</returns>
HWND GetFirstTabStopCtrl(HWND hWnd);

/// <summary>
/// ���л��Ի���ģ��
/// </summary>
/// <param name="Dlg">�Ի�����Ϣ</param>
/// <param name="Items">�Ի�����Ŀ��Ϣ</param>
/// <returns>���л���ĶԻ���ģ���ֽڼ�</returns>
CRefBin SerializeDialogTemplate(const DLGTDLG& Dlg, const std::vector<DLGTITEM>& Items);

/// <summary>
/// �����л��Ի���ģ��
/// </summary>
/// <param name="pTemplate">�Ի���ģ��ָ��</param>
/// <param name="Dlg">���նԻ�����Ϣ������������ʧ�ܣ���˲��������޷�ȷ��</param>
/// <param name="Items">���նԻ�����Ŀ��Ϣ����</param>
/// <returns>�ɹ�����TRUE��ʧ�ܷ���FALSE</returns>
BOOL DeserializeDialogTemplate(PCVOID pTemplate, DLGTDLG& Dlg, std::vector<DLGTITEM>& Items);

/// <summary>
/// ���������ԶԻ���ģ�塣
/// �����ӶԻ���ģ��ķ����л����ģ�ⴴ��һ���Ի��򴰿ڡ�
/// �޷����մ��ڵ�WM_CREATE��WM_NCCREATE��Ϣ������ָ��������Ĵ��ڹ������Զ���ġ�
/// �ؼ���������Ҵ�����ʾ֮ǰ�򴰿ڹ��̷���WM_INITDIALOG��Ϣ
/// </summary>
/// <param name="Dlg">�Ի�����Ϣ</param>
/// <param name="Items">�Ի�����Ŀ��Ϣ</param>
/// <param name="pfnWndProc">���ڹ��̡�����ΪNULL���򴰿ڴ�������ҿؼ�����֮ǰ�����ڵĴ��ڹ�����Ϊ�˲���ָ���Ĵ��ڹ���</param>
/// <param name="hParent">������</param>
/// <param name="hInstance">ʵ��������Ӵ�ʵ��������Դ����ʹ���ڸ�ʵ����ע��Ĵ�����</param>
/// <param name="pParam">�Զ�����Ϣ������WM_CREATE��WM_NCCREATE��WM_INITDIALOG����Ϣ��ʹ��</param>
/// <param name="phMenu">���ղ˵��������ָ�룬��ΪNULL�����÷�����ά���˾������������</param>
/// <param name="phFont">��������������ָ�룬��ΪNULL�����÷�����ά���˾������������</param>
/// <param name="uFlags">��־��CWFDT_����</param>
/// <returns>�ɹ����ش��ھ����ʧ�ܷ���NULL</returns>
HWND CreateWindowFromDialogTemplate(DLGTDLG& Dlg, std::vector<DLGTITEM>& Items,
	WNDPROC pfnWndProc, HWND hParent, HINSTANCE hInstance,
	void* pParam = NULL, HMENU* phMenu = NULL, HFONT* phFont = NULL, UINT uFlags = 0);
ECK_NAMESPACE_END