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
#include <assert.h>
/*��*/

#define ECK_NAMESPACE_BEGIN namespace eck {
#define ECK_NAMESPACE_END }

#define EckInline __forceinline

#define ECK_CTRLDATA_ALIGN 4

#define ECKWIDE2___(x)          L##x
// ANSI�ַ��������ַ���
#define ECKWIDE___(x)           ECKWIDE2___(x)

#define ECKTOSTR2___(x)         #x
// ��ANSI�ַ���
#define ECKTOSTR___(x)          ECKTOSTR2___(x)
// �����ַ���
#define ECKTOSTRW___(x)         ECKWIDE___(ECKTOSTR2___(x))

// [Ԥ����] ��ǰ������W
#define ECK_FUNCTIONW           ECKWIDE___(__FUNCTION__)
// [Ԥ����] �к�W
#define ECK_LINEW               ECKTOSTRW___(__LINE__)
// [Ԥ����] ��ǰ�ļ�W
#define ECK_FILEW               __FILEW__


#define EckProp(getter, setter) __declspec(property(get = getter, put = setter)))

/*����*/
ECK_NAMESPACE_BEGIN

using SCHAR = signed char;
using BITBOOL = UINT;
using PCBYTE = const BYTE*;
using PCVOID = const void*;
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

enum class Align
{
	Near,
	Center,
	Far
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

/*�����ַ���*/

#define PROP_DPIINFO		L"Eck.Prop.DpiInfo"
#define PROP_PREVDPI		L"Eck.Prop.PrevDpi"
#define PROP_PROPSHEETCTX	L"Eck.Prop.PropertySheet.Ctx"


/*һЩ�����õ��Ŀؼ�ID*/



#include "DbgHelper.h"
#include "GdiplusFlatDef.h"
ECK_NAMESPACE_BEGIN
extern HINSTANCE g_hInstance;

/*��������*/

constexpr inline PCWSTR WCN_LABEL = L"Eck.WndClass.Label";
constexpr inline PCWSTR WCN_COLORPICKER = L"Eck.WndClass.ColorPicker";
constexpr inline PCWSTR WCN_BK = L"Eck.WndClass.BK";
constexpr inline PCWSTR WCN_LUNARCALENDAR = L"Eck.WndClass.LunarCalendar";
constexpr inline PCWSTR WCN_CHARTPIE = L"Eck.WndClass.ChartPie";
constexpr inline PCWSTR WCN_FORM = L"Eck.WndClass.Form";
constexpr inline PCWSTR WCN_TABHEADER = L"Eck.WndClass.TabHeader";
constexpr inline PCWSTR WCN_DLG = L"Eck.WndClass.CommDlg";

enum class InitStatus
{
	Ok = 0,
	RegWndClassError = 1,
	GdiplusInitError = 2
};

/// <summary>
/// ��ʼ��ECK Lib
/// </summary>
/// <param name="hInstance">ʵ������������Զ��崰���ཫ�ڴ�ʵ����ע��</param>
/// <returns>�������</returns>
InitStatus Init(HINSTANCE hInstance);
ECK_NAMESPACE_END