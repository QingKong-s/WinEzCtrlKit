/*
* WinEzCtrlKit Library
*
* ECK.h �� ����ͷ�ļ�
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include <Windows.h>
#include <Uxtheme.h>
#include <vsstyle.h>
#include <dwmapi.h>
/*��*/

#define ECK_NAMESPACE_BEGIN namespace eck {
#define ECK_NAMESPACE_END }

#define EckInline __forceinline

#define ECK_CTRLDATA_ALIGN 4

/*����*/
ECK_NAMESPACE_BEGIN

using SCHAR = signed char;
using BITBOOL = UINT;
using PCBYTE = LPCBYTE;
using PCVOID = LPCVOID;
using ECKENUM = BYTE;

namespace Literals
{
	EckInline constexpr WORD operator""_us(ULONGLONG x)
	{
		return (WORD)x;
	}

	EckInline constexpr short operator""_ss(ULONGLONG x)
	{
		return (short)x;
	}
}

enum CtrlAlignH
{
	CALeft,
	CACenter,
	CARight
};

enum CtrlAlignV
{
	CATop,
	CAVCenter,
	CABottom
};
ECK_NAMESPACE_END


/*���໯ID*/

#define SCID_PUSHBUTTON				20230603'01u
#define SCID_CHECKBUTTON			20230603'02u
#define SCID_COMMANDLINK			20230603'03u
#define SCID_EDIT					20230603'04u
#define SCID_EDITPARENT				20230603'05u
#define SCID_COLORPICKERPARENT		20230604'01u
#define SCID_LBEXT					20230606'01u
#define SCID_LBEXTPARENT			20230606'02u
#define SCID_DIRBOX					20230607'01u
#define SCID_DIRBOXPARENT			20230607'02u
#define SCID_TRAY					20230611'01u
#define SCID_DESIGN					20230621'01u
#define SCID_TASKGROUPLIST			20230725'01u
#define SCID_TASKGROUPLISTPARENT	20230725'02u

/*��������*/

#define WCN_LABEL			L"Eck.WndClass.Label"
#define WCN_COLORPICKER		L"Eck.WndClass.ColorPicker"
#define WCN_BK				L"Eck.WndClass.BK"
#define WCN_LUNARCALENDAR	L"Eck.WndClass.LunarCalendar"
#define WCN_CHARTPIE		L"Eck.WndClass.ChartPie"


/*�����ַ���*/

#define PROP_DPIINFO		L"Eck.Prop.DpiInfo"
#define PROP_PREVDPI		L"Eck.Prop.PrevDpi"
#define PROP_PROPSHEETCTX	L"Eck.Prop.PropertySheet.Ctx"


/*һЩ�����õ��Ŀؼ�ID*/



#include "DbgHelper.h"
#include "GdiplusFlatDef.h"
ECK_NAMESPACE_BEGIN
extern HINSTANCE g_hInstance;

enum ECKLibInitStatus
{
	ECKOk = 0,
	RegWndClassError = 1,
	GdiplusInitError = 2
};

/// <summary>
/// ��ʼ��ECK Lib
/// </summary>
/// <param name="hInstance">ʵ������������Զ��崰���ཫ�ڴ�ʵ����ע��</param>
/// <returns>�������</returns>
ECKLibInitStatus Init(HINSTANCE hInstance);
ECK_NAMESPACE_END