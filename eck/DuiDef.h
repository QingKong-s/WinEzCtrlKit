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
	DES_BLURBKG = (1u << 0),		// 模糊背景
	DES_DISABLE = (1u << 1),		// 已禁用
	DES_VISIBLE = (1u << 2),		// 可见
	DES_TRANSPARENT = (1u << 3),	// 背景透明，该样式无效果，保留供将来使用，但若元素透明则必须设置
	DES_CONTENT_EXPAND = (1u << 4),	// 更新时必须更新整个元素，若设置了DES_BLURBKG，则强制设置此标志
	DES_COMPOSITED = (1u << 5),		// 渲染到独立的图面，然后与主图面混合
	DES_INPLACE_COMP = (1u << 6),	// 混合矩形完全包含在元素矩形中
	DES_EXTERNAL_CONTENT = (1u << 7),// 使用某外部图面作为DComp视觉对象的内容
	DES_DISALLOW_REDRAW = (1u << 8),// 不允许重绘
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
	//						|  等待	|				透明混合					|	 备注	|
	BitBltSwapChain,	//	|	0	|支持，必须无WS_EX_NOREDIRECTIONBITMAP	|  性能极差	|
	FlipSwapChain,		//	|	1	|不支持									|  性能极好	|
	DCompositionSurface,//	|	0	|支持，建议加入WS_EX_NOREDIRECTIONBITMAP	|  建议使用	|
	WindowRenderTarget,	//  |	1	|支持，必须无WS_EX_NOREDIRECTIONBITMAP	|  兼容性好	|
	AllDComp,			//	|	   与DCompositionSurface相同，但所有元素都使用DComp合成		|
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

// 元素混合信息
struct COMP_INFO
{
	CElem* pElem;			// 正操作的元素
	const D2D1_RECT_F* prc;	// 混合到的矩形，相对pElem
	ID2D1Bitmap1* pBitmap;	// 已渲染完毕的位图
};

// 元素混合坐标变换信息
struct COMP_POS
{
	CElem* pElem;
	POINT pt;		// 相对pElem
	BOOL bNormalToComp;
};

// 事件
enum
{
	ECKPRIV_EWM_PLACEHOLDER = WM_USER_SAFE,
	WM_DRAGENTER,
	WM_DRAGOVER,
	WM_DRAGLEAVE,
	WM_DROP,

	EWM_COLORSCHEMECHANGED,// 颜色主题改变 (BOOL 是否为暗色, 0)
	EWM_COMPOSITE,	// 指示手动混合元素执行混合，(COMP_INFO*, 0)
	EWM_COMP_POS,	// 指示手动混合元素执行坐标变换，(COMP_POS*, 0)

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
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END