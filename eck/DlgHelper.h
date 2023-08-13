/*
* WinEzCtrlKit Library
*
* DlgHelper.h ： 对话框相关帮助函数
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
	WORD      wVer;// 必须为1;
	WORD      wSignature;// 必须为0xFFFF;
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
DLGTHEADER后跟以下结构
struct DLGTMENU
{
	WCHAR ch[];// ch[0] == 0 - 无菜单，ch[1]  0xFFFF - 资源ID，ch[2]  其他 - 以NULL结尾的资源名称，ch[n]
};

struct DLGTWCLS
{
	WCHAR ch[];// ch[0] == 0 - 标准对话框类，ch[1]  0xFFFF - 预定义类原子，ch[2]  其他 - 以NULL结尾的窗口类名，ch[n]
};

struct DLGTCAPTION
{
	WCHAR ch[];// ch[0] == 0 - 无标题，ch[1]  其他 - 以NULL结尾的标题，ch[n]
};
上述三个数组必须在WORD边界上对齐

struct DLGTFONT// 仅当指定了DS_SETFONT或DS_SHELLFONT时存在
{
	WORD      wPoint;
	WORD      wWeight;
	BYTE      bItalic;
	BYTE      byCharSet;
	WCHAR     szName[];// 以NULL结尾的字体名称
};
*/


// 每个DLGITEMTHEADER都必须在DWORD边界上对齐

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
DLGITEMTHEADER后跟以下结构
struct DLGITEMTWCLS
{
	WCHAR ch[];// ch[0] == 0xFFFF - 预定义类原子，ch[2]  其他 - 以NULL结尾的窗口类名，ch[n]
};

struct DLGITEMTCAPTION
{
	WCHAR ch[];// ch[0] == 0xFFFF - 资源ID，ch[2]  其他 - 以NULL结尾的标题，ch[n]
};

struct DLGITEMTEXTRA
{
	WORD cbExtra;
};// 后接附加数据
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
	// 将尺寸单位视为像素，而不是DLU
	CWFDT_USEPIXELUNIT = 1u << 0
};

/// <summary>
/// 取首个可停留焦点控件。
/// 函数枚举窗口的子窗口并返回第一个可见、非禁用且具有WS_TABSTOP的控件句柄
/// </summary>
/// <param name="hWnd">父窗口</param>
/// <returns>控件句柄</returns>
HWND GetFirstTabStopCtrl(HWND hWnd);

/// <summary>
/// 序列化对话框模板
/// </summary>
/// <param name="Dlg">对话框信息</param>
/// <param name="Items">对话框项目信息</param>
/// <returns>序列化后的对话框模板字节集</returns>
CRefBin SerializeDialogTemplate(const DLGTDLG& Dlg, const std::vector<DLGTITEM>& Items);

/// <summary>
/// 反序列化对话框模板
/// </summary>
/// <param name="pTemplate">对话框模板指针</param>
/// <param name="Dlg">接收对话框信息变量，若函数失败，则此参数内容无法确定</param>
/// <param name="Items">接收对话框项目信息变量</param>
/// <returns>成功返回TRUE，失败返回FALSE</returns>
BOOL DeserializeDialogTemplate(PCVOID pTemplate, DLGTDLG& Dlg, std::vector<DLGTITEM>& Items);

/// <summary>
/// 创建窗口自对话框模板。
/// 函数从对话框模板的反序列化结果模拟创建一个对话框窗口。
/// 无法接收窗口的WM_CREATE和WM_NCCREATE消息，除非指定窗口类的窗口过程是自定义的。
/// 控件创建完毕且窗口显示之前向窗口过程发送WM_INITDIALOG消息
/// </summary>
/// <param name="Dlg">对话框信息</param>
/// <param name="Items">对话框项目信息</param>
/// <param name="pfnWndProc">窗口过程。若不为NULL，则窗口创建完毕且控件创建之前将窗口的窗口过程设为此参数指定的窗口过程</param>
/// <param name="hParent">父窗口</param>
/// <param name="hInstance">实例句柄。从此实例加载资源，并使用在该实例上注册的窗口类</param>
/// <param name="pParam">自定义信息，将在WM_CREATE、WM_NCCREATE和WM_INITDIALOG等消息中使用</param>
/// <param name="phMenu">接收菜单句柄变量指针，可为NULL。调用方负责维护此句柄的生命周期</param>
/// <param name="phFont">接收字体句柄变量指针，可为NULL。调用方负责维护此句柄的生命周期</param>
/// <param name="uFlags">标志，CWFDT_常量</param>
/// <returns>成功返回窗口句柄，失败返回NULL</returns>
HWND CreateWindowFromDialogTemplate(DLGTDLG& Dlg, std::vector<DLGTITEM>& Items,
	WNDPROC pfnWndProc, HWND hParent, HINSTANCE hInstance,
	void* pParam = NULL, HMENU* phMenu = NULL, HFONT* phFont = NULL, UINT uFlags = 0);
ECK_NAMESPACE_END