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
#include <wincodec.h>
#include <dwrite.h>
#include <d2d1_1.h>
#include <dxgi1_2.h>

#include <assert.h>

#include <unordered_map>
/*宏*/

#define ECK_NAMESPACE_BEGIN namespace eck {
#define ECK_NAMESPACE_END }

#define EckInline __forceinline
#define ECK_COMDAT __declspec(selectany)

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


#define ECKPROP(getter, setter) __declspec(property(get = getter, put = setter))

#define ECKPROP_R(getter) __declspec(property(get = getter))

#define ECKPROP_W(setter) __declspec(property(put = setter))

// 计次循环
#define EckCounter(c, Var) for(std::remove_cvref_t<decltype(c)> Var = 0; Var < (c); ++Var)

#define EckCounterNVMakeVarName2(Name) ECKPRIV_COUNT_##Name##___
#define EckCounterNVMakeVarName(Name) EckCounterNVMakeVarName2(Name)

// 计次循环，无变量名参数
#define EckCounterNV(c) EckCounter((c), EckCounterNVMakeVarName(__LINE__))

#define EckOpt(Type, Name) std::optional<Type> Name
#define EckOptNul(Type, Name) std::optional<Type> Name = std::nullopt

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

constexpr inline UINT SCID_PUSHBUTTON = 20230603'01u;
constexpr inline UINT SCID_CHECKBUTTON = 20230603'02u;
constexpr inline UINT SCID_COMMANDLINK = 20230603'03u;
constexpr inline UINT SCID_EDIT = 20230603'04u;
constexpr inline UINT SCID_EDITPARENT = 20230603'05u;
constexpr inline UINT SCID_COLORPICKERPARENT = 20230604'01u;
constexpr inline UINT SCID_LBEXT = 20230606'01u;
constexpr inline UINT SCID_LBEXTPARENT = 20230606'02u;
constexpr inline UINT SCID_DIRBOX = 20230607'01u;
constexpr inline UINT SCID_DIRBOXPARENT = 20230607'02u;
constexpr inline UINT SCID_TRAY = 20230611'01u;
constexpr inline UINT SCID_DESIGN = 20230621'01u;
constexpr inline UINT SCID_TASKGROUPLIST = 20230725'01u;
constexpr inline UINT SCID_TASKGROUPLISTPARENT = 20230725'02u;
constexpr inline UINT SCID_INERTIALSCROLLVIEW = 20231103'01u;

#pragma warning(suppress:26454)// 算术溢出
constexpr inline UINT NM_FIRST_ECK = (0u - 0x514B);
constexpr inline UINT NM_CLP_CLRCHANGED = NM_FIRST_ECK;

/*属性字符串*/

constexpr inline PCWSTR PROP_DPIINFO = L"Eck.Prop.DpiInfo";
constexpr inline PCWSTR PROP_PREVDPI = L"Eck.Prop.PrevDpi";
constexpr inline PCWSTR PROP_PROPSHEETCTX = L"Eck.Prop.PropertySheet.Ctx";

#include "DbgHelper.h"
#include "GdiplusFlatDef.h"
ECK_NAMESPACE_BEGIN
extern HINSTANCE g_hInstance;
extern IWICImagingFactory* g_pWicFactory;
extern ID2D1Factory1* g_pD2dFactory;
extern IDWriteFactory* g_pDwFactory;
extern ID2D1Device* g_pD2dDevice;
extern IDXGIDevice1* g_pDxgiDevice;
extern IDXGIFactory2* g_pDxgiFactory;

/*窗口类名*/

constexpr inline PCWSTR WCN_LABEL = L"Eck.WndClass.Label";
constexpr inline PCWSTR WCN_COLORPICKER = L"Eck.WndClass.ColorPicker";
constexpr inline PCWSTR WCN_BK = L"Eck.WndClass.BK";
constexpr inline PCWSTR WCN_LUNARCALENDAR = L"Eck.WndClass.LunarCalendar";
constexpr inline PCWSTR WCN_CHARTPIE = L"Eck.WndClass.ChartPie";
constexpr inline PCWSTR WCN_FORM = L"Eck.WndClass.Form";
constexpr inline PCWSTR WCN_TABHEADER = L"Eck.WndClass.TabHeader";
constexpr inline PCWSTR WCN_DLG = L"Eck.WndClass.CommDlg";
constexpr inline PCWSTR WCN_SPLITBAR = L"Eck.WndClass.SplitBar";
constexpr inline PCWSTR WCN_DRAWPANEL = L"Eck.WndClass.DrawPanel";
constexpr inline PCWSTR WCN_DRAWPANELD2D = L"Eck.WndClass.DrawPanelD2D";
constexpr inline PCWSTR WCN_LISTBOXNEW = L"Eck.WndClass.ListBoxNew";
constexpr inline PCWSTR WCN_ANIMATIONBOX = L"Eck.WndClass.AnimationBox";

constexpr inline PCWSTR MSG_INERTIALSV = L"Eck.Message.InertialScrollView";

enum class InitStatus
{
	Ok,
	RegWndClassError,
	GdiplusInitError,
	WicFactoryError,
	D2dFactoryError,
	DxgiDeviceError,
	DWriteFactoryError,
	D3dDeviceError
};

enum
{
	// 不要调用ThreadInit
	ECKINIT_NOINITTHREAD = 1u << 0
};

constexpr inline D3D_FEATURE_LEVEL c_uDefD3dFeatureLevel[]
{
	D3D_FEATURE_LEVEL_11_1,
	D3D_FEATURE_LEVEL_11_0,
	D3D_FEATURE_LEVEL_10_1,
	D3D_FEATURE_LEVEL_10_0,
	D3D_FEATURE_LEVEL_9_3,
	D3D_FEATURE_LEVEL_9_2,
	D3D_FEATURE_LEVEL_9_1
};

struct INITPARAM
{
	D2D1_FACTORY_TYPE uD2dFactoryType = D2D1_FACTORY_TYPE_SINGLE_THREADED;
	DWRITE_FACTORY_TYPE uDWriteFactoryType = DWRITE_FACTORY_TYPE_SHARED;
	const D3D_FEATURE_LEVEL* pD3dFeatureLevel = c_uDefD3dFeatureLevel;
	UINT cD3dFeatureLevel = ARRAYSIZE(c_uDefD3dFeatureLevel);
	UINT uFlags = 0;
};

/// <summary>
/// 初始化ECK Lib。
/// 使用任何ECK功能之前需调用该函数。仅允许调用一次。
/// 函数将在内部调用eck::ThreadInit，除非设置了ECKINIT_NOINITTHREAD
/// </summary>
/// <param name="hInstance">实例句柄，所有自定义窗口类将在此实例上注册</param>
/// <param name="pInitParam">指向初始化参数的可选指针</param>
/// <param name="pdwErrCode">指向接收错误码变量的可选指针</param>
/// <returns>错误代码</returns>
InitStatus Init(HINSTANCE hInstance, const INITPARAM* pInitParam = NULL, DWORD* pdwErrCode = NULL);

void UnInit();

class CWnd;
struct ECKTHREADCTX;

using FWndCreating = void(*)(HWND hWnd, CBT_CREATEWNDW* pcs, ECKTHREADCTX* pThreadCtx);
struct ECKTHREADCTX
{
	std::unordered_map<HWND, CWnd*> hmWnd{};// HWND->CWnd*
	HHOOK hhkTempCBT = NULL;// CBT钩子句柄
	CWnd* pCurrWnd = NULL;// 当前正在创建窗口所属的CWnd指针
	FWndCreating pfnWndCreatingProc = NULL;// 当前创建窗口时要调用的过程

	EckInline void WmAdd(HWND hWnd, CWnd* pWnd)
	{
		hmWnd.insert(std::make_pair(hWnd, pWnd));
	}

	EckInline void WmRemove(HWND hWnd)
	{
		auto it = hmWnd.find(hWnd);
		if (it != hmWnd.end())
			hmWnd.erase(it);
	}

	EckInline CWnd* WmAt(HWND hWnd) const
	{
		auto it = hmWnd.find(hWnd);
		if (it != hmWnd.end())
			return it->second;
		else
			return NULL;
	}
};

/// <summary>
/// 取线程上下文TLS槽
/// </summary>
[[nodiscard]] DWORD GetThreadCtxTlsSlot();

/// <summary>
/// 初始化线程上下文。
/// 在调用线程上初始化线程上下文，如果线程使用了ECK的窗口对象，则必须调用此函数
/// </summary>
void ThreadInit();

/// <summary>
/// 反初始化线程上下文。
/// 调用此函数后不允许使用任何ECK窗口对象
/// </summary>
void ThreadUnInit();

/// <summary>
/// 取线程上下文
/// </summary>
[[nodiscard]] EckInline ECKTHREADCTX* GetThreadCtx()
{
	return (ECKTHREADCTX*)TlsGetValue(GetThreadCtxTlsSlot());
}

/// <summary>
/// 窗口句柄到CWnd指针
/// </summary>
[[nodiscard]] EckInline CWnd* CWndFromHWND(HWND hWnd)
{
	return GetThreadCtx()->WmAt(hWnd);
}

HHOOK BeginCbtHook(CWnd* pCurrWnd, FWndCreating pfnCreatingProc = NULL);

void EndCbtHook();

// 全局消息过滤器，若要拦截消息则应返回TRUE，否则应返回FALSE
using FMsgFilter = BOOL(*)(const MSG& Msg);

/// <summary>
/// 过滤消息。
/// 若使用了任何ECK窗口对象，则必须在翻译按键和派发消息之前调用此函数
/// </summary>
/// <param name="pMsg">即将处理的消息</param>
/// <returns>若返回值为TRUE，则不应继续处理消息；否则应正常进行剩余步骤</returns>
BOOL PreTranslateMessage(const MSG& Msg);

/// <summary>
/// 置消息过滤器
/// </summary>
/// <param name="pfnFilter">应用程序定义的过滤器函数指针</param>
void SetMsgFilter(FMsgFilter pfnFilter);

void DbgPrintWndMap();

ECK_NAMESPACE_END