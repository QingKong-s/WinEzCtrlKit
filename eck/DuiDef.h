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
	DES_COMP_NO_REDIRECTION = (1u << 10),
	// GenElemNotify产生的通知不会发送到父级，而是发送到窗口
	DES_NOTIFY_TO_WND = (1u << 11),
	// 【仅供内部使用】元素的后台缓存内容需要更新。
	// 设置该标志的意图是尽量减少手动混合元素的重渲染（特别是在播放动画时）。
	// 普通元素需要每次重渲染是因为其内容没有保存，但未设置DES_COMP_NO_REDIRECTION
	// 的手动混合标志都具有一副后台位图，内容为上一次渲染结果。
	// DUI系统直接在m_dwStyle字段上操作该位，该位永远不会传递到SetStyle
	DESP_COMP_CONTENT_INVALID = (1u << 12),
	// 指示基类事件处理函数应调用BeginPaint/EndPaint对
	DES_BASE_BEGIN_END_PAINT = (1u << 13),
};

// 元素产生的通知
enum :UINT
{
	EE_COMMAND = 1,
	EE_KILLFOCUS,
	EE_SETFOCUS,
	EE_CLICK,
	EE_RCLICK,
	EE_DBLCLICK,
	EE_SCROLL,
	EE_HSCROLL = EE_SCROLL,
	EE_VSCROLL = EE_SCROLL,
	EE_CUSTOMDRAW,		// NMECUSTOMDRAW*，返回CDRF_*

	// TrackBar
	TBE_POSCHANGED,		// 位置改变
	// List
	LEE_GETDISPINFO,	// 【渲染线程】获取显示信息(NMLEDISPINFO*)
	// ListTemplate
	LTE_ITEMCHANED,		// 项改变(NMLTITEMCHEANGED*)，返回TRUE禁止修改
	LTE_HOTITEMCHANED,	// 热点项改变(NMLTHOTITEMCHEANGED*)，返回TRUE禁止修改
	LTE_SCROLLED,		// 滚动结束(NMLTSCROLLED*)
	// TabList
	TBLE_GETDISPINFO,	// 【渲染线程】获取显示信息(NMTBLDISPINFO*)
	TBLE_SELCHANGED,	// 选项卡改变(NMTBLITEMINDEX*)
	// Header
	HEE_GETDISPINFO,	// 【渲染线程】获取显示信息(NMHEDISPINFO*)
	HEE_BEGINDRAG,		// 开始拖拽(NMHEDRAG*)
	HEE_ENDDRAG,		// 结束拖拽(NMHEDRAG*)
	HEE_WIDTHCHANGED,	// 宽度改变(NMHEITEMNOTIFY*)
	HEE_ORDERCHANGED,	// 顺序改变(NMHEITEMNOTIFY*)

	EE_PRIVATE_BEGIN = 0x0400
};

// DUI图形系统呈现模式
enum class PresentMode : BYTE
{
	// WS_EX_NRB = WS_EX_NOREDIRECTIONBITMAP
	//						|	   透明混合		|
	BitBltSwapChain,	//	|支持，必须无WS_EX_NRB|
	FlipSwapChain,		//	|	   不支持		|
	DCompositionSurface,//	|		支持			|
	WindowRenderTarget,	//  |支持，必须无WS_EX_NRB|
	AllDComp,			//	|		支持			|FIXME
	DCompositionVisual,	//  |		支持			|
	DxgiSurface,		//	|		支持			|TODO
	GdiRenderTarget,	//	|		支持			|TODO
	UpdateLayeredWindow,//  |		支持			|TODO
};

// 渲染事件代码
enum
{
	// 即将开始渲染
	// 下列字段有效：PreRender
	// 下列返回值有效：
	// RER_NONE = 执行默认操作
	// RER_REDIRECTION = 应用程序重定向渲染，DUI系统应使用pSfcNewDst和prcNewDirtyPhy
	RE_PRERENDER,
	RE_POSTRENDER,		// 渲染完毕，仅当RE_PRERENDER返回RER_REDIRECTION时产生
	RE_COMMIT,			// DUI系统认为有必要冲洗一切挂起的工作
	RE_FILLBACKGROUND,	// 正在填充背景
};
// 渲染事件返回值
enum : LRESULT
{
	RER_NONE = 0,
	RER_REDIRECTION = 1 << 0,
	RER_NO_ERASE = 1 << 1,
};
// 渲染事件结构
struct RENDER_EVENT
{
	union
	{
		struct
		{
			IDXGISurface* pSfcFinalDst;
			POINT ptOffsetPhy;
			const RECT* prcDirtyPhy;
			IDXGISurface* pSfcNewDst;
			RECT* prcNewDirtyPhy;
		} PreRender;
		struct
		{
			D2D1_RECT_F rc;
		} FillBkg;
	};
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

namespace Priv
{
	struct PAINT_EXTRA
	{
		float ox;
		float oy;
	};
}

// PaintStruct
struct ELEMPAINTSTRU
{
	const RECT* prcClip;		// 剪裁矩形，相对客户区
	D2D1_RECT_F rcfClip;		// 剪裁矩形，相对客户区
	D2D1_RECT_F rcfClipInElem;	// 剪裁矩形，相对元素
	float ox;
	float oy;
};

// 所有通知结构的头
struct DUINMHDR
{
	UINT uCode;
};

// 通用自定义绘制结构
struct NMECUSTOMDRAW : DUINMHDR
{
	int idx;
	DWORD dwStage;
	Part ePart;
	State eState;
	D2D1_RECT_F rc;
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END