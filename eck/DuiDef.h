#pragma once
#include "ECK.h"

#if !ECKCXX20
#error "EckDui requires C++20"
#endif// !ECKCXX20

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
class CElem;

// 元素样式
enum
{
	DES_BLURBKG = (1u << 0),	// 模糊背景
	DES_DISABLE = (1u << 1),	// 已禁用
	DES_VISIBLE = (1u << 2),	// 可见
	DES_TRANSPARENT = (1u << 3),// 保留，当前无效
	// 元素的内容受周边其他内容影响，若无效区域与元素相交，
	// 则必须更新整个元素，设置时DES_BLURBKG强制设置此样式
	DES_CONTENT_EXPAND = (1u << 4),	
	// 元素的内容受周边其他内容影响，确定更新区域时DUI系统发送
	// EWM_QUERY_EXPAND_RECT以获取扩展矩形
	DES_CONTENT_EXPAND_RECT = (1u << 5),// TODO
	// DUI系统应当检查当前元素的祖元素，因为它们可能设置了混合器
	DES_PARENT_COMP = (1u << 6),
	// 使用某外部图面作为当前元素的DComp视觉对象的内容
	DES_EXTERNAL_CONTENT = (1u << 7),// TODO
	DES_NO_REDRAW = (1u << 8),	// 不允许重绘
	// 对于手动混合元素，DUI不应自行分配后台缓存，而应按下列顺序请求缓存：
	// 调用CCompositor::CreateCacheBitmap，若失败，向元素的父级发送
	// EWM_CREATE_CACHE_BITMAP
	DES_OWNER_COMP_CACHE = (1u << 9),
	// 指示当前手动混合元素不使用后台缓存，
	// 设置后DUI系统适时调用CCompositor::PreRender
	DES_COMP_NO_REDIRECTION = (1u << 10),// TODO
	// GenElemNotify产生的通知不会发送到父级，而是发送到窗口
	DES_NOTIFY_TO_WND = (1u << 11),
};

// 元素产生的通知
enum :UINT
{
	EE_COMMAND = 1,
	EE_KILLFOCUS,
	EE_SETFOCUS,
	EE_CLICK,
	EE_RCLICK,
	EE_HSCROLL,
	EE_VSCROLL,
	EE_CUSTOMDRAW,	// CUSTOM_DRAW*，返回CDRF_*

	// TrackBar
	TBE_POSCHANGED,	// 位置改变
	// List
	LEE_GETDISPINFO,// 【渲染线程】获取显示信息
	// ListTemplate
	LTE_ITEMCLICK,	// 项选中(LTN_ITEM*)
	// TabList
	TBLE_GETDISPINFO,// 【渲染线程】获取显示信息
	// GroupList
	GLE_GETDISPINFO,// 【渲染线程】获取显示信息

	EE_PRIVATE_BEGIN = 0x0400
};

// DUI图形系统呈现模式
enum class PresentMode
{
	// WS_EX_NRB = WS_EX_NOREDIRECTIONBITMAP
	//						|最小等待	|	   透明混合		|	 备注	|
	BitBltSwapChain,	//	|	0	|支持，必须无WS_EX_NRB|  性能极差	|
	FlipSwapChain,		//	|	1	|	   不支持		|  性能极好	|
	DCompositionSurface,//	|	0	|		支持			|  建议使用	|
	WindowRenderTarget,	//  |	1	|支持，必须无WS_EX_NRB|  兼容性好	|
	AllDComp,			//	|	0	|		支持			|			|
	DCompositionVisual,	//  | 不适用	|		支持			|			|
	DxgiSurface,		//	| 不适用	|		支持			|  D3D互操作	|
	GdiRenderTarget,	//	| 不适用	|		支持			|  GDI互操作	|
	UpdateLayeredWindow,//  |   0	|		支持			|  Win2K分层	|
};

// 渲染事件代码
enum
{
	RE_PRERENDER,		// 即将开始渲染
	RE_POSTRENDER,		// 渲染完毕
	RE_COMMIT,			// DUI系统认为有必要冲洗一切挂起的工作
};

// 渲染事件结构
struct RENDER_EVENT
{

};

// 拖放信息
struct DRAGDROPINFO
{
	IDataObject* pDataObj;
	DWORD grfKeyState;
	POINTL ptScreen;
	DWORD* pdwEffect;
	HRESULT hr;
};

class CCompCacheSurface;
struct CREATE_CACHE_BITMAP_INFO
{
	int cxPhy;
	int cyPhy;
	CCompCacheSurface* pCacheSurface;
	HRESULT hr;
};
// 事件
enum
{
	ECKPRIV_EWM_PLACEHOLDER = WM_USER_SAFE,
	WM_DRAGENTER,
	WM_DRAGOVER,
	WM_DRAGLEAVE,
	WM_DROP,

	EWM_COLORSCHEMECHANGED,	// 颜色主题改变 (BOOL 是否为暗色, 0)
	EWM_QUERY_EXPAND_RECT,	// 查询扩展矩形 (_Out_ RECT*, 0)
	// 创建缓存位图 (_Inout_ CREATE_CACHE_BITMAP_INFO*, 0)
	// 若成功则应返回TRUE
	EWM_CREATE_CACHE_BITMAP,

	EWM_PRIVBEGIN,
};

// BeginPaint选项
enum
{
	EBPF_DO_NOT_FILLBK = (1u << 0),
};

// PaintStruct
struct ELEMPAINTSTRU
{
	const RECT* prcClip;		// 剪裁矩形，相对客户区
	D2D1_RECT_F rcfClip;		// 剪裁矩形，相对客户区
	D2D1_RECT_F rcfClipInElem;	// 剪裁矩形，相对元素
};

// 所有通知结构的头
struct DUINMHDR
{
	UINT uCode;
};

// 通用自定义绘制结构
struct CUSTOM_DRAW : DUINMHDR
{
	DWORD dwStage;
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END