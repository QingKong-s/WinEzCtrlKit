/*
* WinEzCtrlKit Library
*
* ECK.h ： 公共头文件
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include <Windows.h>
#include <Uxtheme.h>
#include <vsstyle.h>
#include <dwmapi.h>
#include <assert.h>
/*宏*/

#define ECK_NAMESPACE_BEGIN namespace eck {
#define ECK_NAMESPACE_END }

#define EckInline __forceinline

#define ECK_CTRLDATA_ALIGN 4

#define ECKWIDE2___(x)          L##x
// ANSI字符串到宽字符串
#define ECKWIDE___(x)           ECKWIDE2___(x)

#define ECKTOSTR2___(x)         #x
// 到ANSI字符串
#define ECKTOSTR___(x)          ECKTOSTR2___(x)
// 到宽字符串
#define ECKTOSTRW___(x)         ECKWIDE___(ECKTOSTR2___(x))

// [预定义] 当前函数名W
#define ECK_FUNCTIONW           ECKWIDE___(__FUNCTION__)
// [预定义] 行号W
#define ECK_LINEW               ECKTOSTRW___(__LINE__)
// [预定义] 当前文件W
#define ECK_FILEW               __FILEW__


#define EckProp(getter, setter) __declspec(property(get = getter, put = setter)))

/*类型*/
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


/*子类化ID*/

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

/*属性字符串*/

#define PROP_DPIINFO		L"Eck.Prop.DpiInfo"
#define PROP_PREVDPI		L"Eck.Prop.PrevDpi"
#define PROP_PROPSHEETCTX	L"Eck.Prop.PropertySheet.Ctx"


/*一些可能用到的控件ID*/



#include "DbgHelper.h"
#include "GdiplusFlatDef.h"
ECK_NAMESPACE_BEGIN
extern HINSTANCE g_hInstance;

/*窗口类名*/

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
/// 初始化ECK Lib
/// </summary>
/// <param name="hInstance">实例句柄，所有自定义窗口类将在此实例上注册</param>
/// <returns>错误代码</returns>
InitStatus Init(HINSTANCE hInstance);
ECK_NAMESPACE_END