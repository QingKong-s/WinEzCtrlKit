/*
* WinEzCtrlKit Library
*
* DuiBase.h ： 简单DUI基础设施
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "CWnd.h"
#include "GraphicsHelper.h"
#include "OleDragDropHelper.h"
#include "EasingCurve.h"
#include "CDuiColorTheme.h"
#include "ITimeLine.h"
#include "CEvent.h"
#include "CCriticalSection.h"
#include "ILayout.h"
#include "SystemHelper.h"
#include "CDwmWndPartMgr.h"
#include "CSignal.h"

#include <dcomp.h>
#include <oleacc.h>

#if ECKCXX20
#define ECK_DUI_NAMESPACE_BEGIN namespace Dui {
#define ECK_DUI_NAMESPACE_END }

#ifdef _DEBUG
#	define ECK_DUI_DBG_DRAW_FRAME					\
		{											\
			ID2D1SolidColorBrush* ECKPRIV_pBr___;	\
			m_pDC->CreateSolidColorBrush(D2D1::ColorF(1.f, 0.f, 0.f, 1.f), &ECKPRIV_pBr___); \
			if (ECKPRIV_pBr___) {					\
				m_pDC->DrawRectangle(GetViewRectF(), ECKPRIV_pBr___, 1.f);	\
				ECKPRIV_pBr___->Release();			\
			}										\
		}
#else
#	define ECK_DUI_DBG_DRAW_FRAME ;
#endif

#define ECK_ELEMTOP			((::eck::Dui::CElem*)HWND_TOP)
#define ECK_ELEMBOTTOM		((::eck::Dui::CElem*)HWND_BOTTOM)


ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
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

	EE_PRIVATE_BEGIN = 0x0400
};

struct DRAGDROPINFO
{
	IDataObject* pDataObj;
	DWORD grfKeyState;
	POINTL ptScreen;
	DWORD* pdwEffect;
	HRESULT hr;
};

class CElem;
struct COMP_INFO
{
	const D2D1_RECT_F* prc;
	CElem* pElem;
};

enum
{
	ECKPRIV_EWM_PLACEHOLDER = WM_USER_SAFE,
	WM_DRAGENTER,
	WM_DRAGOVER,
	WM_DRAGLEAVE,
	WM_DROP,

	EWM_COLORSCHEMECHANGED,// 颜色主题改变
	EWM_COMPOSITE,	// 指示手动混合元素执行混合，(COMP_INFO*, 0)
	EWM_COMP_POS,	// 指示手动混合元素执行坐标变换，(POINT*待变换坐标, BOOL是否从标准点转换为混合后的点)

	EWM_PRIVBEGIN,
};

enum
{
	EBPF_DO_NOT_FILLBK = (1u << 0),
};

struct ELEMPAINTSTRU
{
	D2D1_RECT_F rcfClip;		// 剪裁矩形，相对客户区
	const RECT* prcClip;		// 剪裁矩形，相对客户区
	D2D1_RECT_F rcfClipInElem;	// 剪裁矩形，相对元素
};

struct DUINMHDR
{
	UINT uCode;
};

#define ECK_DUILOCK		::eck::CCsGuard ECKPRIV_DUI_LOCK_GUARD(*(GetWnd()->GetCriticalSection()))
#define ECK_DUILOCKWND	::eck::CCsGuard ECKPRIV_DUI_LOCK_GUARD(*GetCriticalSection())


/// <summary>
/// DUI元素基类
/// </summary>
class CElem :public ILayout
{
	friend class CDuiWnd;
protected:
	CElem* m_pNext{};		// 下一元素，Z序高于当前
	CElem* m_pPrev{};		// 上一元素，Z序低于当前
	CElem* m_pParent{};		// 父元素
	CElem* m_pFirstChild{};	// 第一个子元素
	CElem* m_pLastChild{};	// 最后一个子元素
	CDuiWnd* m_pWnd{};		// 所属窗口
	ID2D1DeviceContext* m_pDC{};	// DC
	CColorTheme* m_pColorTheme{};	// 颜色主题

	RECT m_rc{};			// 元素矩形，相对父元素
	D2D1_RECT_F m_rcf{};	// 元素矩形，相对父元素
	RECT m_rcInClient{};	// 元素矩形，相对客户区
	D2D1_RECT_F m_rcfInClient{};	// 元素矩形，相对客户区

	RECT m_rcInvalid{};		// 无效矩形，相对客户区

	RECT m_rcPostComposited{};		// 混合到主图面的矩形，至少完全包含元素矩形
	RECT m_rcPostCompositedInClient{};	// 混合到主图面的矩形，相对于客户区

	CRefStrW m_rsText{};	// 标题
	DWORD m_dwStyle{};		// 样式
	INT_PTR m_iId{};		// 元素ID

	int m_cChildren{};		// 子元素数量

	IDCompositionVisual* m_pDcVisual{};		// DComp视觉对象
	IDCompositionSurface* m_pDcSurface{};	// DComp表面
	IUnknown* m_pDcContent{};				// DComp内容

	CSignal<Intercept_T, LRESULT, UINT, WPARAM, LPARAM> m_Sig{};// 信号

	BOOL IntCreate(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, CElem* pParent, CDuiWnd* pWnd, int iId = 0, PCVOID pData = nullptr);

	void DestroyChild(CElem* pElem)
	{
		auto pChild = pElem->GetFirstChildElem();
		while (pChild)
		{
			auto pNext = pChild->GetNextElem();
			pChild->Destroy();
			pChild = pNext;
		}
	}

	CElem* HitTestChildUncheck(POINT pt, LRESULT* pResult = nullptr)
	{
		auto pElem = GetLastChildElem();
		while (pElem)
		{
			if (pElem->GetStyle() & DES_VISIBLE)
			{
				if (PtInRect(pElem->GetRectInClient(), pt))
				{
					const auto pHit = pElem->HitTestChildUncheck(pt, pResult);
					if (pHit)
						return pHit;
					else if (LRESULT lResult;
						(lResult = pElem->CallEvent(WM_NCHITTEST, 0, MAKELPARAM(pt.x, pt.y))) != HTTRANSPARENT)
					{
						if (pResult)
							*pResult = lResult;
						return pElem;
					}
				}
			}
			pElem = pElem->GetPrevElem();
		}
		return nullptr;
	}

	void IRUnionContentExpandElemRect(CElem* pLast, RECT& rcInClient)
	{
		while (pLast)
		{
			if (const auto dwStyle = pLast->GetStyle(); dwStyle & DES_VISIBLE)
			{
				if (dwStyle & DES_CONTENT_EXPAND)
				{
					const auto& rcElem = pLast->GetWholeRectInClient();
					if (IsRectsIntersect(rcInClient, rcElem))
					{
						pLast->m_rcInvalid = rcElem;
						UnionRect(rcInClient, rcInClient, rcElem);
					}
				}
				else
					IRUnionContentExpandElemRect(pLast->GetLastChildElem(), rcInClient);
			}
			pLast = pLast->GetPrevElem();
		}
	}

	void SRCorrectChildrenRectInClient()
	{
		auto pElem = GetFirstChildElem();
		while (pElem)
		{
			pElem->m_rcInClient = pElem->m_rc;
			pElem->m_rcfInClient = pElem->m_rcf;
			OffsetRect(pElem->m_rcInClient, m_rcInClient.left, m_rcInClient.top);
			OffsetRect(pElem->m_rcfInClient, m_rcfInClient.left, m_rcfInClient.top);
			if ((pElem->GetStyle() & DES_COMPOSITED) &&
				!(pElem->GetStyle() & DES_INPLACE_COMP))
			{
				pElem->m_rcPostCompositedInClient = pElem->m_rcPostComposited;
				OffsetRect(pElem->m_rcPostCompositedInClient, m_rcInClient.left, m_rcInClient.top);
			}
			pElem->SRCorrectChildrenRectInClient();
			pElem = pElem->GetNextElem();
		}
	}

	void IntSetRect(const RECT& rc)
	{
		m_rc = rc;
		m_rcf = MakeD2DRcF(rc);
		m_rcInClient = rc;
		m_rcfInClient = m_rcf;
		if (m_pParent)
		{
			OffsetRect(
				m_rcInClient,
				m_pParent->GetRectInClient().left,
				m_pParent->GetRectInClient().top);
			OffsetRect(
				m_rcfInClient,
				m_pParent->GetRectInClientF().left,
				m_pParent->GetRectInClientF().top);
		}

		SRCorrectChildrenRectInClient();
	}

	void IntSetStyle(DWORD dwStyle)
	{
		if (dwStyle & DES_BLURBKG)
			dwStyle |= (DES_TRANSPARENT | DES_CONTENT_EXPAND);
		if (!(m_dwStyle & DES_COMPOSITED) && (dwStyle & DES_COMPOSITED))
		{
			if (!(dwStyle & DES_INPLACE_COMP))
				dwStyle |= DES_CONTENT_EXPAND;
		}
		m_dwStyle = dwStyle;
	}

	void SwitchDefColorTheme(int idxTheme, WPARAM bDark);

	void ReCreateDCompVisual();

	void ReSizeDCompVisual();

	void PostMoveSize(BOOL bSize, BOOL bMove, const RECT& rcOld);

	void InvalidateAllElem()
	{
		auto pElem = GetFirstChildElem();
		while (pElem)
		{
			pElem->m_rcInvalid = pElem->GetWholeRectInClient();
			pElem->InvalidateAllElem();
			pElem = pElem->GetNextElem();
		}
	}
public:
#pragma region ILayout
	void LoGetAppropriateSize(int& cx, int& cy) override
	{
		cx = GetWidth();
		cy = GetHeight();
	}

	void LoSetPos(int x, int y) override
	{
		SetPos(x, y);
	}

	void LoSetSize(int cx, int cy) override
	{
		SetSize(cx, cy);
	}

	void LoSetPosSize(int x, int y, int cx, int cy) override
	{
		SetRect({ x,y,x + cx,y + cy });
	}

	std::pair<int, int> LoGetPos() override
	{
		return { GetRect().left,GetRect().top };
	}

	std::pair<int, int> LoGetSize() override
	{
		return { GetWidth(),GetHeight() };
	}

	void LoShow(BOOL bShow) override
	{
		SetVisible(bShow);
	}
#pragma endregion ILayout

	virtual LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);

	EckInline LRESULT CallEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		BOOL bProcessed{};
		const auto r = m_Sig.Emit2(bProcessed, uMsg, wParam, lParam);
		if (bProcessed)
			return r;
		return OnEvent(uMsg, wParam, lParam);
	}

	EckInline const D2D1_RECT_F& GetRectF() const { return m_rcf; }

	EckInline const RECT& GetRect() const { return m_rc; }

	void SetRect(const RECT& rc);

	void SetPos(int x, int y);

	void SetSize(int cx, int cy);

	virtual BOOL Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, CElem* pParent, CDuiWnd* pWnd, int iId = 0, PCVOID pData = nullptr)
	{
		return IntCreate(pszText, dwStyle, dwExStyle, x, y, cx, cy, pParent, pWnd, iId, pData);
	}

	void Destroy();

	EckInline CElem* GetFirstChildElem() const { return m_pFirstChild; }
	EckInline CElem* GetLastChildElem() const { return m_pLastChild; }
	EckInline CElem* GetParentElem() const { return m_pParent; }
	EckInline CDuiWnd* GetWnd() const { return m_pWnd; }
	EckInline ID2D1DeviceContext* GetD2DDC() const { return m_pDC; }

	EckInline const CRefStrW& GetText() const { return m_rsText; }

	EckInline void SetText(PCWSTR pszText);

	EckInline constexpr DWORD GetStyle() const { return m_dwStyle; }

	void SetStyle(DWORD dwStyle);

	// 取下一元素，Z序高于当前
	EckInline constexpr CElem* GetNextElem() const { return m_pNext; }

	// 取上一元素，Z序低于当前
	EckInline constexpr CElem* GetPrevElem() const { return m_pPrev; }

	/// <summary>
	/// 是否可见。
	/// 函数自底向上遍历父元素，如果存在不可见父元素，则返回FALSE，否则返回TRUE。
	/// </summary>
	EckInline constexpr BOOL IsVisible() const
	{
		auto pParent = this;
		while (pParent)
		{
			if (!(pParent->GetStyle() & DES_VISIBLE))
				return FALSE;
			pParent = pParent->GetParentElem();
		}
		return TRUE;
	}

	EckInline CElem* HitTestChild(POINT pt)
	{
		if (!IsVisible())
			return nullptr;
		return HitTestChildUncheck(pt);
	}

	EckInline constexpr void ClientToElem(RECT& rc) const
	{
		OffsetRect(rc, -m_rcInClient.left, -m_rcInClient.top);
	}

	EckInline constexpr void ClientToElem(D2D1_RECT_F& rc) const
	{
		OffsetRect(rc, -m_rcfInClient.left, -m_rcfInClient.top);
	}

	EckInline constexpr void ClientToElem(POINT& pt) const
	{
		pt.x -= m_rcInClient.left;
		pt.y -= m_rcInClient.top;
	}

	EckInline constexpr void ClientToElem(D2D1_POINT_2F& pt) const
	{
		pt.x -= m_rcfInClient.left;
		pt.y -= m_rcfInClient.top;
	}

	EckInline constexpr void ElemToClient(RECT& rc) const
	{
		OffsetRect(rc, m_rcInClient.left, m_rcInClient.top);
	}

	EckInline constexpr void ElemToClient(D2D1_RECT_F& rc) const
	{
		OffsetRect(rc, m_rcfInClient.left, m_rcfInClient.top);
	}

	EckInline constexpr void ElemToClient(POINT& pt) const
	{
		pt.x += m_rcInClient.left;
		pt.y += m_rcInClient.top;
	}

	EckInline constexpr void ElemToClient(D2D1_POINT_2F& pt) const
	{
		pt.x += m_rcfInClient.left;
		pt.y += m_rcfInClient.top;
	}

	EckInline constexpr int GetWidth() const { return m_rc.right - m_rc.left; }

	EckInline constexpr int GetHeight() const { return m_rc.bottom - m_rc.top; }

	EckInline constexpr float GetWidthF() const { return m_rcf.right - m_rcf.left; }

	EckInline constexpr float GetHeightF() const { return m_rcf.bottom - m_rcf.top; }

	void SetZOrder(CElem* pElemAfter);

	/// <summary>
	/// 生成元素通知。
	/// 若GenElemNotifyParent返回0，则通知窗口
	/// </summary>
	/// <param name="pnm">通知结构，第一个字段必须为DUINMHDR</param>
	/// <returns>处理方的返回值</returns>
	EckInline LRESULT GenElemNotify(void* pnm);

	/// <summary>
	/// 生成元素通知。
	/// 向父元素发送通知，若无父元素，返回0
	/// </summary>
	/// <param name="pnm">通知结构，第一个字段必须为DUINMHDR</param>
	/// <returns>处理方的返回值</returns>
	EckInline LRESULT GenElemNotifyParent(void* pnm)
	{
		if (GetParentElem())
			return GetParentElem()->CallEvent(WM_NOTIFY, (WPARAM)this, (LPARAM)pnm);
		else
			return 0;
	}

	EckInline void SetRedraw(BOOL bRedraw)
	{
		if (bRedraw)
			SetStyle(GetStyle() & ~DES_DISALLOW_REDRAW);
		else
			SetStyle(GetStyle() | DES_DISALLOW_REDRAW);
	}

	EckInline constexpr BOOL GetRedraw() const { return !(GetStyle() & DES_DISALLOW_REDRAW); }

	/// <summary>
	/// 无效化矩形
	/// </summary>
	/// <param name="rc">无效区域，相对客户区</param>
	/// <param name="bUpdateNow">是否立即唤醒渲染线程</param>
	void InvalidateRect(const RECT& rc, BOOL bUpdateNow = TRUE);

	EckInline void InvalidateRect(BOOL bUpdateNow = TRUE)
	{
		InvalidateRect(GetWholeRectInClient(), bUpdateNow);
	}

	EckInline void InvalidateRect(const D2D1_RECT_F& rc, BOOL bUpdateNow = TRUE)
	{
		InvalidateRect(MakeRect(rc), bUpdateNow);
	}

	EckInline constexpr const RECT& GetRectInClient() const { return m_rcInClient; }

	EckInline constexpr const D2D1_RECT_F& GetRectInClientF() const { return m_rcfInClient; }

	/// <summary>
	/// 开始画图。
	/// 目前所有元素仅能在处理WM_PAINT事件时绘制，处理时必须调用此函数且必须与EndPaint配对
	/// </summary>
	/// <param name="eps">画图信息结构的引用，将返回画图参数</param>
	/// <param name="wParam">事件wParam，目前设为0</param>
	/// <param name="lParam">事件lParam</param>
	/// <param name="uFlags">标志</param>
	void BeginPaint(ELEMPAINTSTRU& eps, WPARAM wParam, LPARAM lParam, UINT uFlags = 0u);

	/// <summary>
	/// 结束画图
	/// </summary>
	/// <param name="eps">BeginPaint返回的结构，目前未用</param>
	EckInline void EndPaint(const ELEMPAINTSTRU& eps)
	{
		m_pDC->PopAxisAlignedClip();
	}

	// 捕获鼠标
	EckInline CElem* SetCapture();

	// 释放鼠标
	EckInline void ReleaseCapture();

	EckInline void SetFocus();

	EckInline D2D1_RECT_F GetViewRectF() const
	{
		return { 0.f,0.f,GetWidthF(),GetHeightF() };
	}

	EckInline void InitEasingCurve(CEasingCurve* pec);

	EckInline void SetColorTheme(CColorTheme* pColorTheme);

	EckInline const CColorTheme* GetColorTheme() const { return m_pColorTheme; }

	EckInline void BroadcastEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		CallEvent(uMsg, wParam, lParam);
		auto pElem = GetFirstChildElem();
		while (pElem)
		{
			pElem->BroadcastEvent(uMsg, wParam, lParam);
			pElem = pElem->GetNextElem();
		}
	}

	EckInline void BroadcastEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT lStop)
	{
		if (CallEvent(uMsg, wParam, lParam) == lStop)
			return;
		auto pElem = GetFirstChildElem();
		while (pElem)
		{
			pElem->BroadcastEvent(uMsg, wParam, lParam, lStop);
			pElem = pElem->GetNextElem();
		}
	}

	EckInline constexpr void SetID(INT_PTR iId) { m_iId = iId; }

	EckInline constexpr INT_PTR GetID() const { return m_iId; }

	void SetVisible(BOOL b);

	EckInline const RECT& GetPostCompositedRect() const
	{
		return m_rcPostComposited;
	}

	EckInline const RECT& GetPostCompositedRectInClient() const
	{
		return m_rcPostCompositedInClient;
	}

	EckInline void SetPostCompositedRect(const RECT& rc)
	{
		m_rcPostComposited = rc;
		m_rcPostCompositedInClient = rc;
		OffsetRect(m_rcPostCompositedInClient,
			GetRectInClient().left,
			GetRectInClient().top);
	}

	EckInline const RECT& GetWholeRectInClient() const
	{
		return ((
			(GetStyle() & DES_COMPOSITED) && !(GetStyle() & DES_INPLACE_COMP)) ?
			GetPostCompositedRectInClient() :
			GetRectInClient());
	}

	EckInline constexpr ID2D1Bitmap1* GetCompBitmap() const;

	EckInline constexpr auto& GetSignal() { return m_Sig; }
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

/// <summary>
/// DUI窗体类
/// </summary>
class CDuiWnd :public CWnd
{
	friend class CElem;
	friend class CDuiDropTarget;
private:
	constexpr static size_t MaxIR = 4;

	//------元素树------
	CElem* m_pFirstChild{};	// 第一个子元素
	CElem* m_pLastChild{};	// 最后一个子元素

	CElem* m_pFocusElem{};	// 当前焦点元素
	CElem* m_pCurrNcHitTestElem{};	// 当前非客户区命中元素
	CElem* m_pMouseCaptureElem{};	// 当前鼠标捕获元素
	CElem* m_pHoverElem{};	// 当前鼠标悬停元素，for WM_MOUSELEAVE
	//------拖放------
	CElem* m_pDragDropElem{};		// 当前拖放元素
	IDataObject* m_pDataObj{};		// 当前拖放的数据对象
	CDuiDropTarget* m_pDropTarget{};// 拖放目标
	//------图形------
	CEzD2D m_D2d{};					// D2D设备上下文相关，若呈现模式不为交换链，则仅DC字段有效
	ID2D1Bitmap* m_pBmpBkg{};		// 背景位图
	ID2D1SolidColorBrush* m_pBrBkg{};		// 背景画刷

	IDWriteTextFormat* m_pDefTextFormat{};	// 默认文本格式

	IDCompositionDevice* m_pDcDevice{};		// DComp设备
	IDCompositionDevice3* m_pDcDevice3{};	// DComp设备3
	IDCompositionTarget* m_pDcTarget{};		// DComp目标
	IDCompositionVisual* m_pDcVisual{};		// 根视觉对象
	IDCompositionSurface* m_pDcSurface{};	// 根视觉对象的内容

	ID2D1HwndRenderTarget* m_pHwndRenderTarget{};	// 窗口渲染目标，仅当呈现模式为窗口渲染目标时有效

	ID2D1Bitmap1* m_pBmpCache{};	// 缓存位图，模糊或独立混合等可在其上进行
	int m_cxCache{};// 缓存位图宽度
	int m_cyCache{};// 缓存位图高度

	CColorTheme* m_pStdColorTheme[CTI_COUNT]{};		// 默认亮色主题
	CColorTheme* m_pStdColorThemeDark[CTI_COUNT]{};	// 默认暗色主题

	CCriticalSection m_cs{};// 渲染线程同步临界区

	HANDLE m_hthRender{};	// 渲染线程句柄
	CEvent m_evtRender{};	// 渲染线程事件对象
	std::vector<ITimeLine*> m_vTimeLine{};	// 所有时间线

	size_t m_cInvalidRect{};	// 无效矩形数量
	RECT m_InvalidRect[MaxIR]{};// 无效矩形数组
	//------其他------
	int m_cxClient = 0;		// 客户区宽度
	int m_cyClient = 0;		// 客户区高度

	BITBOOL m_bMouseCaptured : 1 = FALSE;	// 鼠标是否被捕获
	BITBOOL m_bTransparent : 1 = FALSE;		// 窗口是透明的
	BITBOOL m_bRenderThreadShouldExit : 1 = FALSE;	// 渲染线程应当退出
	BITBOOL m_bSizeChanged : 1 = FALSE;				// 渲染线程应当重设图面大小

	D2D1_ALPHA_MODE m_eAlphaMode{ D2D1_ALPHA_MODE_IGNORE };		// 缓存D2D透明模式
	DXGI_ALPHA_MODE m_eDxgiAlphaMode{ DXGI_ALPHA_MODE_IGNORE };	// 缓存DXGI透明模式

	PresentMode m_ePresentMode = PresentMode::FlipSwapChain;	// 呈现模式

	IAccessible* m_pStdAcc{};

	int m_cChildren{};

	int m_iUserDpi = USER_DEFAULT_SCREEN_DPI;
	int m_iDpi = USER_DEFAULT_SCREEN_DPI;
	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY_F(CommEdge, 1.f)
		ECK_DS_ENTRY_F(CommRrcRadius, 4.f)
		ECK_DS_ENTRY_F(CommMargin, 4.f)
		ECK_DS_ENTRY_F(SBPadding, 3.f)
		ECK_DS_ENTRY_F(CommSBCxy, 12.f)
		;
	ECK_DS_END_VAR(m_Ds);

	void CorrectSingleElemMember(CElem* pElem)
	{
		if (m_pFocusElem == pElem)
			m_pFocusElem = nullptr;
		if (m_pCurrNcHitTestElem == pElem)
			m_pCurrNcHitTestElem = nullptr;
		if (m_pMouseCaptureElem == pElem)
			ReleaseCaptureElem();
		if (m_pHoverElem == pElem)
		{
			POINT pt;
			GetCursorPos(&pt);
			ScreenToClient(m_hWnd, &pt);
			m_pHoverElem = HitTest(pt);
		}
	}

	void UpdateDpi(int iDpi)
	{
		m_iDpi = iDpi;
		UpdateDpiSizeF(m_Ds, iDpi);
	}

	EckInline constexpr size_t IsIRIntersect(const RECT& rc) const
	{
		__assume(m_cInvalidRect <= MaxIR);
		EckCounter(m_cInvalidRect, i)
		{
			if (IsRectsIntersect(rc, m_InvalidRect[i]))
				return i;
		}
		return SizeTMax;
	}

	void RedrawElem(CElem* pElem, const RECT& rc, float ox, float oy)
	{
		const auto pDC = m_D2d.GetDC();
		RECT rcClip;
		IDXGISurface1* pDxgiSurface{};
		ID2D1Image* pOldTarget{};
		BOOL bNeedComposite{};
		ID2D1Bitmap1* pBitmap{};
		COMP_INFO ci;
		while (pElem)
		{
			const auto& rcElem = pElem->GetRectInClient();
			if (const auto dwStyle = pElem->GetStyle();
				!(dwStyle & DES_VISIBLE) || (dwStyle & (DES_DISALLOW_REDRAW | DES_EXTERNAL_CONTENT)) ||
				IsRectEmpty(rcElem))
				goto NextElem;

			if (const auto iIR = IsIRIntersect(rcElem); iIR == SizeTMax)
				goto NextElem;
			else
				IntersectRect(rcClip, rcElem, m_InvalidRect[iIR]);
			if (IsElemUseDComp())// 使用DComp合成
			{
				RECT rcUpdate{ rcClip };
				pElem->ClientToElem(rcUpdate);
				POINT ptOffset{};
				const auto hr = pElem->m_pDcSurface->BeginDraw(
					&rcUpdate, IID_PPV_ARGS(&pDxgiSurface), &ptOffset);
				if (FAILED(hr))
					pElem->m_pDcSurface->BeginDraw(nullptr, IID_PPV_ARGS(&pDxgiSurface), &ptOffset);
				else
				{
					ptOffset.x -= rcUpdate.left;
					ptOffset.y -= rcUpdate.top;
				}

				const D2D1_BITMAP_PROPERTIES1 D2dBmpProp
				{
					{ DXGI_FORMAT_B8G8R8A8_UNORM,D2D1_ALPHA_MODE_PREMULTIPLIED },
					96,
					96,
					D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
					nullptr
				};
				pDC->CreateBitmapFromDxgiSurface(pDxgiSurface, &D2dBmpProp, &pBitmap);
				pDC->BeginDraw();
				pDC->SetTarget(pBitmap);
				pDC->SetTransform(D2D1::Matrix3x2F::Translation((float)ptOffset.x, (float)ptOffset.y));
				bNeedComposite = FALSE;
			}
			else if (pElem->GetStyle() & DES_COMPOSITED)// 手动合成
			{
				pDC->Flush();
				pDC->GetTarget(&pOldTarget);
				CacheReserve(rcElem.right - rcElem.left, rcElem.bottom - rcElem.top);
				pDC->SetTarget(GetCacheBitmap());
				pDC->SetTransform(D2D1::Matrix3x2F::Identity());
				bNeedComposite = TRUE;
			}
			else// 直接渲染
			{
				pDC->SetTransform(D2D1::Matrix3x2F::Translation(
					pElem->GetRectInClientF().left + ox,
					pElem->GetRectInClientF().top + oy));
				bNeedComposite = FALSE;
			}
			pElem->CallEvent(WM_PAINT, 0, (LPARAM)&rcClip);
			if (IsElemUseDComp())
			{
				pDC->EndDraw();
				pDC->SetTarget(nullptr);
				pBitmap->Release();
				pDxgiSurface->Release();
				pElem->m_pDcSurface->EndDraw();
				RedrawElem(pElem->GetFirstChildElem(), rcClip, 0.f, 0.f);
			}
			else if (bNeedComposite)
			{
				const D2D1_RECT_F rcF{ 0.f,0.f,pElem->GetWidthF(),pElem->GetHeightF() };
				pDC->Flush();
				pDC->SetTarget(pOldTarget);
				pOldTarget->Release();
				pDC->SetTransform(D2D1::Matrix3x2F::Translation(
					pElem->GetRectInClientF().left + ox,
					pElem->GetRectInClientF().top + oy));
				ci.prc = &rcF;
				ci.pElem = pElem;
				pElem->CallEvent(EWM_COMPOSITE, (WPARAM)&ci, 0);
				RedrawElem(pElem->GetFirstChildElem(), rcClip, ox, oy);
			}
			else
				RedrawElem(pElem->GetFirstChildElem(), rcClip, ox, oy);
		NextElem:
			pElem = pElem->GetNextElem();
		}
	}

	void RedrawDui(const RECT& rc)
	{
		__assume(m_cInvalidRect <= MaxIR);
		switch (m_ePresentMode)
		{
		case PresentMode::BitBltSwapChain:
		case PresentMode::FlipSwapChain:
		case PresentMode::WindowRenderTarget:
		{
			const auto pDC = m_D2d.GetDC();
			pDC->BeginDraw();
			pDC->SetTransform(D2D1::Matrix3x2F::Identity());
			EckCounter(m_cInvalidRect, i)
			{
				const auto rcF = MakeD2DRcF(m_InvalidRect[i]);
				if (m_bTransparent)
				{
					pDC->PushAxisAlignedClip(rcF, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
					pDC->Clear({});
					pDC->PopAxisAlignedClip();
				}
				FillBackground(rcF);
			}
			RedrawElem(GetFirstChildElem(), rc, 0.f, 0.f);
			pDC->EndDraw();
		}
		return;
		case PresentMode::DCompositionSurface:
		case PresentMode::AllDComp:
		{
			const auto pDC = m_D2d.GetDC();
			IDXGISurface1* pDxgiSurface = nullptr;
			POINT ptOffset;
			m_pDcSurface->BeginDraw(&rc, IID_PPV_ARGS(&pDxgiSurface), &ptOffset);
			ptOffset.x -= rc.left;
			ptOffset.y -= rc.top;
			const D2D1_BITMAP_PROPERTIES1 D2dBmpProp
			{
				{ DXGI_FORMAT_B8G8R8A8_UNORM,m_eAlphaMode },
				96,
				96,
				D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
				nullptr
			};

			ID2D1Bitmap1* pBitmap = nullptr;
			pDC->CreateBitmapFromDxgiSurface(pDxgiSurface, &D2dBmpProp, &pBitmap);
			pDC->SetTarget(pBitmap);
			pDC->BeginDraw();
			pDC->SetTransform(D2D1::Matrix3x2F::Identity());
			m_D2d.m_pBitmap = pBitmap;

			// 由于DComp图面BeginDraw无法保存矩形内已绘制内容，此处必须填充无效区域的广义并
			RECT rcReal{ rc };
			OffsetRect(rcReal, ptOffset.x, ptOffset.y);
			const auto rcF = MakeD2DRcF(rcReal);
			if (m_bTransparent)
			{
				pDC->PushAxisAlignedClip(rcF, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
				pDC->Clear({});
				pDC->PopAxisAlignedClip();
			}
			FillBackground(rcF);

			if (!IsElemUseDComp())
				RedrawElem(GetFirstChildElem(), rc, (float)ptOffset.x, (float)ptOffset.y);

			m_D2d.m_pBitmap = nullptr;

			pDC->EndDraw();
			pDC->SetTarget(nullptr);
			pBitmap->Release();
			pDxgiSurface->Release();
			m_pDcSurface->EndDraw();

			if (IsElemUseDComp())
				RedrawElem(GetFirstChildElem(), rc, 0.f, 0.f);

			m_pDcDevice->Commit();
		}
		return;
		}
		ECK_UNREACHABLE;
	}

	EckInline void StartupRenderThread()
	{
		std::thread t([this]
			{
				ThreadInit();
				constexpr int c_iMinGap = 16;
				HANDLE hTimer;
				OBJECT_ATTRIBUTES oa;
				InitOA(oa);
				NtCreateTimer(&hTimer, TIMER_ALL_ACCESS, &oa, SynchronizationTimer);
				EckAssert(hTimer);
				BOOL bThereAreActiveTimeLine;

				WaitObject(m_evtRender);
				ULONGLONG ullTime = NtGetTickCount64() - c_iMinGap;
				EckLoop()
				{
					bThereAreActiveTimeLine = FALSE;
					m_cs.Enter();

					if (m_bRenderThreadShouldExit)
					{
						m_cs.Leave();
						break;
					}

					const auto ullCurrTime = NtGetTickCount64();
					// 滴答所有时间线
					int iDeltaTime = (int)(ullCurrTime - ullTime);
					for (const auto e : m_vTimeLine)
					{
						if (e->IsValid())
							e->Tick(iDeltaTime);
						bThereAreActiveTimeLine = bThereAreActiveTimeLine || e->IsValid();
					}

					const RECT rcClient{ 0,0,m_cxClient,m_cyClient };
					if (m_bSizeChanged)
					{
						switch (m_ePresentMode)
						{
						case PresentMode::BitBltSwapChain:
							m_D2d.ReSize(1, m_cxClient, m_cyClient, 0, m_eAlphaMode);
							break;
						case PresentMode::FlipSwapChain:
							m_D2d.ReSize(2, m_cxClient, m_cyClient, 0, m_eAlphaMode);
							break;
						case PresentMode::DCompositionSurface:
						case PresentMode::AllDComp:
						{
							IDCompositionSurface* pDcSurface = nullptr;
							m_pDcDevice->CreateSurface(m_cxClient, m_cyClient,
								DXGI_FORMAT_B8G8R8A8_UNORM,
								m_bTransparent ? DXGI_ALPHA_MODE_PREMULTIPLIED : DXGI_ALPHA_MODE_IGNORE,
								&pDcSurface);
							m_pDcVisual->SetContent(pDcSurface);
							if (m_pDcSurface)
								m_pDcSurface->Release();
							m_pDcSurface = pDcSurface;
							m_pDcVisual->SetContent(m_pDcSurface);
						}
						break;
						case PresentMode::WindowRenderTarget:
							m_pHwndRenderTarget->Resize(D2D1::SizeU(m_cxClient, m_cyClient));
							break;
						default:
							ECK_UNREACHABLE;
						}

						m_bSizeChanged = FALSE;
						if (m_cxClient && m_cyClient)
						{
							m_InvalidRect[0] = rcClient;
							m_cInvalidRect = 1u;
							RedrawDui(rcClient);
							m_cInvalidRect = 0u;
							m_cs.Leave();
							if (m_ePresentMode == PresentMode::BitBltSwapChain ||
								m_ePresentMode == PresentMode::FlipSwapChain)
								m_D2d.GetSwapChain()->Present(0, 0);
						}
						else
						{
							m_cInvalidRect = 0u;
							m_cs.Leave();
						}
					}
					else ECKLIKELY
					{
						// 更新脏矩形
						RECT rc{};
						__assume(m_cInvalidRect <= MaxIR);
						if (m_cInvalidRect)
						{
							const auto cRc = m_cInvalidRect;
							RECT aRc[MaxIR];
							memcpy(aRc, m_InvalidRect, sizeof(RECT) * cRc);

							const RECT rcClient{ 0,0,m_cxClient,m_cyClient };
							EckCounter(cRc, i)
							{
								auto& e = aRc[i];
								IntersectRect(e, e, rcClient);
								UnionRect(rc, rc, e);
							}
							RedrawDui(rc);
							m_cInvalidRect = 0u;
							m_cs.Leave();
							// 呈现
							switch (m_ePresentMode)
							{
							case PresentMode::BitBltSwapChain:
								m_D2d.GetSwapChain()->Present(0, 0);
								break;
							case PresentMode::FlipSwapChain:
							{
								DXGI_PRESENT_PARAMETERS pp
								{
									.DirtyRectsCount = (UINT)cRc,
									.pDirtyRects = aRc
								};
								m_D2d.GetSwapChain()->Present1(0, 0, &pp);
							}
							break;
							}
						}
						else
							m_cs.Leave();
					}

					iDeltaTime = (int)(NtGetTickCount64() - ullCurrTime);
					ullTime = ullCurrTime;
					if (bThereAreActiveTimeLine)
					{
						if (iDeltaTime < c_iMinGap)// 延时
						{
							LARGE_INTEGER llDueTime
							{ .QuadPart = -10 * (c_iMinGap - iDeltaTime) * 1000LL };
							NtSetTimer(hTimer, &llDueTime, nullptr, nullptr, FALSE, 0, nullptr);
							WaitObject(hTimer);
						}
					}
					else
					{
						WaitObject(m_evtRender);
						ullTime = NtGetTickCount64() - c_iMinGap;
					}
				}

				NtCancelTimer(hTimer,nullptr);
				NtClose(hTimer);
				ThreadUnInit();
			});
		m_hthRender = DuplicateStdThreadHandle(t);
		t.detach();
	}
public:
	// 一般不覆写此方法
	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = nullptr) override
	{
		if (m_ePresentMode == PresentMode::FlipSwapChain ||
			m_ePresentMode == PresentMode::DCompositionSurface ||
			m_ePresentMode == PresentMode::AllDComp)
			dwExStyle |= WS_EX_NOREDIRECTIONBITMAP;
		return IntCreate(dwExStyle, WCN_DUIHOST, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, g_hInstance, nullptr);
	}

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		if ((uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST))
		{
			if (m_bMouseCaptured)
			{
				POINT pt ECK_GET_PT_LPARAM(lParam);
				m_pCurrNcHitTestElem = HitTest(pt);
			}
			auto pElem = (m_pMouseCaptureElem ? m_pMouseCaptureElem : m_pCurrNcHitTestElem);
			if (pElem)
				pElem->CallEvent(uMsg, wParam, lParam);

			if (uMsg == WM_MOUSEMOVE)// 移出监听
			{
				if (m_pHoverElem != m_pCurrNcHitTestElem && !m_bMouseCaptured)
				{
					if (m_pHoverElem)
						m_pHoverElem->CallEvent(WM_MOUSELEAVE, 0, 0);
					m_pHoverElem = m_pCurrNcHitTestElem;
				}
				TRACKMOUSEEVENT tme;
				tme.cbSize = sizeof(tme);
				tme.dwFlags = TME_LEAVE;
				tme.hwndTrack = hWnd;
				TrackMouseEvent(&tme);
			}

			return 0;
		}
		else if (uMsg >= WM_NCMOUSEMOVE && uMsg <= WM_NCXBUTTONDBLCLK)
		{
			if (m_bMouseCaptured)
			{
				POINT pt ECK_GET_PT_LPARAM(lParam);
				ScreenToClient(hWnd, &pt);
				m_pCurrNcHitTestElem = HitTest(pt);
			}
			auto pElem = (m_pMouseCaptureElem ? m_pMouseCaptureElem : m_pCurrNcHitTestElem);
			if (pElem)
				pElem->CallEvent(uMsg, wParam, lParam);

			if (uMsg == WM_NCMOUSEMOVE)// 移出监听
			{
				if (m_pHoverElem != m_pCurrNcHitTestElem && !m_bMouseCaptured)
				{
					if (m_pHoverElem)
						m_pHoverElem->CallEvent(WM_MOUSELEAVE, 0, 0);
					m_pHoverElem = m_pCurrNcHitTestElem;
				}
				TRACKMOUSEEVENT tme;
				tme.cbSize = sizeof(tme);
				tme.dwFlags = TME_LEAVE | TME_NONCLIENT;
				tme.hwndTrack = hWnd;
				TrackMouseEvent(&tme);
			}
			else if (uMsg == WM_NCLBUTTONDOWN)// 修正放开事件
			{
				if (pElem)
				{
					const auto lResult = CWnd::OnMsg(hWnd, uMsg, wParam, lParam);
					POINT pt;
					GetCursorPos(&pt);
					pElem->CallEvent(WM_NCLBUTTONUP, wParam, MAKELPARAM(pt.x, pt.y));
					return lResult;
				}
			}
		}
		else if (uMsg >= WM_KEYFIRST && uMsg <= WM_IME_KEYLAST)
		{
			if (m_pFocusElem)
				m_pFocusElem->CallEvent(uMsg, wParam, lParam);
			return 0;
		}

		switch (uMsg)
		{
		case WM_NCHITTEST:
		{
			POINT pt ECK_GET_PT_LPARAM(lParam);
			ScreenToClient(hWnd, &pt);
			LRESULT lResult;
			if (m_pCurrNcHitTestElem = HitTest(pt, &lResult))
				return lResult;
		}
		break;

		case WM_PAINT:
		{
			if (m_ePresentMode == PresentMode::BitBltSwapChain ||
				m_ePresentMode == PresentMode::WindowRenderTarget)
			{
				RECT rcInvalid;
				GetUpdateRect(hWnd, &rcInvalid, FALSE);
				ValidateRect(hWnd, nullptr);
				UnionInvalidRect(rcInvalid);
				WakeRenderThread();
			}
			else
				ValidateRect(hWnd, nullptr);
		}
		return 0;

		case WM_SIZE:
		{
			ECK_DUILOCKWND;
			ECK_GET_SIZE_LPARAM(m_cxClient, m_cyClient, lParam);
			m_bSizeChanged = TRUE;
			WakeRenderThread();
		}
		break;

		case WM_SETCURSOR:
		{
			const auto pElem = (m_pMouseCaptureElem ? m_pMouseCaptureElem : m_pCurrNcHitTestElem);
			if (pElem && pElem->CallEvent(uMsg, wParam, lParam))
				return TRUE;
		}
		break;

		case WM_NCMOUSELEAVE:
		case WM_MOUSELEAVE:
			if (m_pHoverElem)
			{
				m_pHoverElem->CallEvent(WM_MOUSELEAVE, 0, 0);
				m_pHoverElem = nullptr;
			}
			break;

		case WM_CAPTURECHANGED:
		{
			if (m_bMouseCaptured)
			{
				m_bMouseCaptured = FALSE;
				if (m_pMouseCaptureElem)
				{
					m_pMouseCaptureElem->CallEvent(WM_CAPTURECHANGED, 0, NULL);
					if (m_pHoverElem != m_pMouseCaptureElem)
					{
						m_pHoverElem->CallEvent(WM_MOUSELEAVE, 0, 0);
						m_pHoverElem = nullptr;
					}
					m_pMouseCaptureElem = nullptr;
				}
			}
		}
		break;

		case WM_GETOBJECT:
		{

		}
		return 0;

		case WM_COMMAND:// 应用程序可能需要使用菜单
			BroadcastEvent(uMsg, wParam, lParam, TRUE);
			return 0;

		case WM_SETTINGCHANGE:
		{
			if (IsColorSchemeChangeMessage(lParam))
				BroadcastEvent(EWM_COLORSCHEMECHANGED, ShouldAppsUseDarkMode(), 0);
		}
		break;

		case WM_CREATE:
		{
			auto lResult = CWnd::OnMsg(hWnd, uMsg, wParam, lParam);
			if (!lResult)
			{
				UpdateDpi(GetDpi(hWnd));
				RECT rc;
				GetClientRect(hWnd, &rc);

				MakeStdThemeLight(m_pStdColorTheme);
				MakeStdThemeDark(m_pStdColorThemeDark);

				m_pDefTextFormat = CreateDefTextFormat(m_iDpi);
				m_pDefTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

				rc.right = std::max(rc.right, 8L);
				rc.bottom = std::max(rc.bottom, 8L);
				switch (m_ePresentMode)
				{
				case PresentMode::BitBltSwapChain:
				{
					auto Param = EZD2D_PARAM::MakeBitblt(hWnd, g_pDxgiFactory, g_pDxgiDevice,
						g_pD2dDevice, rc.right, rc.bottom);
					Param.uBmpAlphaMode = m_eAlphaMode;
					m_D2d.Create(Param);
				}
				break;
				case PresentMode::FlipSwapChain:
				{
					auto Param = EZD2D_PARAM::MakeFlip(hWnd, g_pDxgiFactory, g_pDxgiDevice,
						g_pD2dDevice, rc.right, rc.bottom);
					Param.uBmpAlphaMode = m_eAlphaMode;
					m_D2d.Create(Param);
				}
				break;
				case PresentMode::DCompositionSurface:
				case PresentMode::AllDComp:
				{
					g_pD2dDevice->CreateDeviceContext(
						EZD2D_PARAM::MakeFlip(0, nullptr, nullptr, nullptr, 0, 0).uDcOptions, &m_D2d.m_pDC);

					DCompositionCreateDevice3(g_pDxgiDevice, IID_PPV_ARGS(&m_pDcDevice));
					m_pDcDevice->QueryInterface(&m_pDcDevice3);
					m_pDcDevice->CreateTargetForHwnd(hWnd, TRUE, &m_pDcTarget);
					m_pDcDevice->CreateVisual(&m_pDcVisual);
					m_pDcDevice->CreateSurface(rc.right, rc.bottom,
						DXGI_FORMAT_B8G8R8A8_UNORM,
						m_eDxgiAlphaMode,
						&m_pDcSurface);
					m_pDcVisual->SetContent(m_pDcSurface);
					m_pDcTarget->SetRoot(m_pDcVisual);
					m_pDcVisual->SetOffsetX(0.f);
					m_pDcVisual->SetOffsetY(0.f);
					m_pDcDevice->Commit();
				}
				break;
				case PresentMode::WindowRenderTarget:
				{
					D2D1_RENDER_TARGET_PROPERTIES RtProp;
					RtProp.type = D2D1_RENDER_TARGET_TYPE_DEFAULT;
					RtProp.pixelFormat = { DXGI_FORMAT_B8G8R8A8_UNORM, m_eAlphaMode };
					RtProp.dpiX = 96.f;
					RtProp.dpiY = 96.f;
					RtProp.usage = D2D1_RENDER_TARGET_USAGE_NONE;
					RtProp.minLevel = D2D1_FEATURE_LEVEL_DEFAULT;
					D2D1_HWND_RENDER_TARGET_PROPERTIES HwRtProp;
					HwRtProp.hwnd = hWnd;
					HwRtProp.pixelSize = D2D1::SizeU(rc.right, rc.bottom);
					HwRtProp.presentOptions = D2D1_PRESENT_OPTIONS_NONE;
					g_pD2dFactory->CreateHwndRenderTarget(RtProp, HwRtProp, &m_pHwndRenderTarget);
					const auto hr = m_pHwndRenderTarget->QueryInterface(&m_D2d.m_pDC);
					EckAssert(SUCCEEDED(hr));
				}
				break;
				default:
					ECK_UNREACHABLE;
				}

				m_D2d.GetDC()->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);

				if (!m_bTransparent)
				{
					const auto crBkg = ColorrefToD2dColorF(GetThreadCtx()->crDefBkg);
					m_D2d.GetDC()->CreateSolidColorBrush(crBkg, &m_pBrBkg);
				}
				m_cxClient = rc.right;
				m_cyClient = rc.bottom;

				m_cInvalidRect = 1u;
				m_InvalidRect[0] = { 0,0,m_cxClient,m_cyClient };

				m_evtRender.NoSignal();
				StartupRenderThread();
			}
			return lResult;
		}
		break;

		case WM_DPICHANGED:// for top-level window
		{
			UpdateDpi(HIWORD(wParam));
			auto prc = (const RECT*)lParam;
			SetWindowPos(hWnd, nullptr,
				prc->left, prc->top, prc->right - prc->left, prc->bottom - prc->top,
				SWP_NOZORDER | SWP_NOACTIVATE);
			BroadcastEvent(WM_DPICHANGED, HIWORD(wParam), 0);
		}
		return 0;

		case WM_DPICHANGED_AFTERPARENT:// for child window
		{
			const int iDpi = GetDpi(hWnd);
			UpdateDpi(iDpi);
			BroadcastEvent(WM_DPICHANGED, iDpi, 0);
		}
		return 0;

		case WM_DESTROY:
		{
			// 终止渲染线程
			m_cs.Enter();
			m_bRenderThreadShouldExit = TRUE;
			WakeRenderThread();
			m_cs.Leave();
			WaitForSingleObject(m_hthRender, INFINITE);// 等待渲染线程退出
			NtClose(m_hthRender);
			m_hthRender = nullptr;
			// 销毁所有元素
			auto pElem = m_pFirstChild;
			while (pElem)
			{
				auto pNext = pElem->GetNextElem();
				pElem->Destroy();
				pElem = pNext;
			}
			m_pFirstChild = m_pLastChild = nullptr;
			m_pFocusElem = m_pCurrNcHitTestElem = nullptr;
			// 销毁图形堆栈
			m_D2d.Destroy();
			SafeRelease(m_pBmpBkg);
			SafeRelease(m_pBrBkg);
			SafeRelease(m_pDefTextFormat);
			SafeRelease(m_pHwndRenderTarget);
			SafeRelease(m_pDcSurface);
			SafeRelease(m_pDcTarget);
			SafeRelease(m_pDcVisual);
			SafeRelease(m_pDcDevice);
			// 销毁其他接口
			SafeRelease(m_pDataObj);
			SafeRelease(m_pDropTarget);

			for (auto& p : m_pStdColorTheme)
			{
				p->DeRef();
				p = nullptr;
			}
			for (auto& p : m_pStdColorThemeDark)
			{
				p->DeRef();
				p = nullptr;
			}

			for (const auto p : m_vTimeLine)
				p->Release();
			m_vTimeLine.clear();

			m_cxClient = m_cyClient = 0;
		}
		break;
		}

		return CWnd::OnMsg(hWnd, uMsg, wParam, lParam);
	}

	virtual LRESULT OnElemEvent(CElem* pElem, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		return 0;
	}

	EckInline constexpr const CEzD2D& GetD2D() const { return m_D2d; }

	EckInline constexpr ID2D1Bitmap* GetBkgBitmap() const { return m_pBmpBkg; }

	EckInline constexpr ID2D1SolidColorBrush* GetBkgBrush() const { return m_pBrBkg; }

	EckInline void SetBkgBitmap(ID2D1Bitmap* pBmp)
	{
		std::swap(m_pBmpBkg, pBmp);
		if (m_pBmpBkg)
			m_pBmpBkg->AddRef();
		if (pBmp)
			pBmp->Release();
	}

	void FillBackground(const D2D1_RECT_F& rc)
	{
		m_D2d.GetDC()->SetTransform(D2D1::Matrix3x2F::Identity());
		if (m_pBrBkg)
			m_D2d.GetDC()->FillRectangle(rc, m_pBrBkg);
		if (m_pBmpBkg)
		{
			auto size = m_pBmpBkg->GetSize();
			const float kx = size.width / m_cxClient;
			const float ky = size.height / m_cyClient;

			D2D1_RECT_F rcSource;
			rcSource.left = rc.left * kx;
			rcSource.top = rc.top * ky;
			rcSource.right = rc.right * kx;
			rcSource.bottom = rc.bottom * ky;
			m_D2d.GetDC()->DrawBitmap(m_pBmpBkg, &rc, 1.f, D2D1_INTERPOLATION_MODE_LINEAR, &rcSource);
		}
	}

	CElem* HitTest(POINT pt, LRESULT* pResult = nullptr)
	{
		auto pElem = m_pLastChild;
		while (pElem)
		{
			if (pElem->GetStyle() & DES_VISIBLE)
			{
				if (PtInRect(pElem->GetRectInClient(), pt))
				{
					auto pHit = pElem->HitTestChildUncheck(pt, pResult);
					if (pHit)
						return pHit;
					else if (LRESULT lResult;
						(lResult = pElem->CallEvent(WM_NCHITTEST, 0, MAKELPARAM(pt.x, pt.y))) != HTTRANSPARENT)
					{
						if (pResult)
							*pResult = lResult;
						return pElem;
					}
				}
			}
			pElem = pElem->GetPrevElem();
		}
		return nullptr;
	}

	CElem* SetFocusElem(CElem* pElem)
	{
		SetFocus(HWnd);
		if (m_pFocusElem == pElem)
			return pElem;
		auto pOld = m_pFocusElem;
		if (pOld)
			pOld->CallEvent(WM_KILLFOCUS, (WPARAM)pElem, 0);
		m_pFocusElem = pElem;
		pElem->CallEvent(WM_SETFOCUS, (WPARAM)pOld, 0);
		return pOld;
	}

	EckInline CElem* GetFocusElem() const { return m_pFocusElem; }

	CElem* SetCaptureElem(CElem* pElem)
	{
		auto pOld = m_pMouseCaptureElem;
		m_pMouseCaptureElem = pElem;
		if (GetCapture() != m_hWnd)
		{
			SetCapture(m_hWnd);
			m_bMouseCaptured = TRUE;
		}
		if (pOld)
			pOld->CallEvent(WM_CAPTURECHANGED, 0, (LPARAM)pElem);
		return pOld;
	}

	EckInline void ReleaseCaptureElem()
	{
		ReleaseCapture();

		// WM_CAPTURECHANGED will process it:
		// m_pMouseCaptureElem->CallEvent(WM_CAPTURECHANGED, 0, NULL);
		// m_pMouseCaptureElem = NULL;
	}

	EckInline CElem* GetCaptureElem() const { return m_pMouseCaptureElem; }

	EckInline CElem* GetFirstChildElem() const { return m_pFirstChild; }

	EckInline CElem* GetLastChildElem() const { return m_pLastChild; }

	EckInline const DPIS& GetDs() const { return m_Ds; }

	EckInline int GetDpiValue() const { return m_iDpi; }

	EckInline HRESULT EnableDragDrop(BOOL bEnable);

	EckInline auto GetDefTextFormat() const { return m_pDefTextFormat; }

	EckInline auto GetDefColorTheme() const { return m_pStdColorTheme; }

	EckInline auto GetDefColorThemeDark() const { return m_pStdColorThemeDark; }

	EckInline void BroadcastEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		auto pElem = GetFirstChildElem();
		while (pElem)
		{
			pElem->BroadcastEvent(uMsg, wParam, lParam);
			pElem = pElem->GetNextElem();
		}
	}

	EckInline void BroadcastEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT lStop)
	{
		auto pElem = GetFirstChildElem();
		while (pElem)
		{
			pElem->BroadcastEvent(uMsg, wParam, lParam, lStop);
			pElem = pElem->GetNextElem();
		}
	}

	EckInline CCriticalSection* GetCriticalSection() { return &m_cs; }

	EckInline void UnionInvalidRect(const RECT& rc)
	{
		__assume(m_cInvalidRect <= MaxIR);
		// 此种情况无法保存无效区域广义并之内、每个无效区域之外的已绘制内容，必须退化为单个无效矩形
		if (m_ePresentMode == PresentMode::DCompositionSurface)
		{
			m_cInvalidRect = 1u;
			UnionRect(m_InvalidRect[0], m_InvalidRect[0], rc);
			return;
		}
		EckCounter(m_cInvalidRect, i)
		{
			if (IsRectsIntersect(m_InvalidRect[i], rc))
			{
				UnionRect(m_InvalidRect[i], m_InvalidRect[i], rc);
				return;
			}
		}

		if (m_cInvalidRect != MaxIR) [[likely]]// 若列表未满，插入到末尾
			m_InvalidRect[m_cInvalidRect++] = rc;
		else// 若列表已满，合并整个列表，然后插入到末尾
		{
			UnionRect(m_InvalidRect[0], m_InvalidRect[0], m_InvalidRect[1]);
			UnionRect(m_InvalidRect[2], m_InvalidRect[2], m_InvalidRect[3]);
			UnionRect(m_InvalidRect[0], m_InvalidRect[0], m_InvalidRect[2]);
			m_InvalidRect[1] = rc;
			m_cInvalidRect = 2u;
		}
	}

	EckInline void WakeRenderThread()
	{
		m_evtRender.Signal();
	}

	EckInline void RegisterTimeLine(ITimeLine* ptl)
	{
		ECK_DUILOCKWND;
		ptl->AddRef();
		m_vTimeLine.emplace_back(ptl);
	}

	EckInline void UnregisterTimeLine(ITimeLine* ptl)
	{
		ECK_DUILOCKWND;
		const auto it = std::find(m_vTimeLine.begin(), m_vTimeLine.end(), ptl);
		EckAssert(it != m_vTimeLine.end());
		const auto p = *it;
		m_vTimeLine.erase(it);
		p->Release();
	}

	EckInline void Redraw()
	{
		ECK_DUILOCKWND;
		m_cInvalidRect = 1u;
		m_InvalidRect[0] = { 0,0,m_cxClient,m_cyClient };
		WakeRenderThread();
	}

	/// <summary>
	/// 置呈现模式。
	/// 必须在创建窗口之前调用
	/// </summary>
	EckInline void SetPresentMode(PresentMode ePresentMode)
	{
		EckAssert(!IsValid());
		m_ePresentMode = ePresentMode;
	}

	EckInline constexpr PresentMode GetPresentMode() const { return m_ePresentMode; }

	/// <summary>
	/// 置透明模式。
	/// 必须在创建窗口之前调用。
	/// 设置透明后不会生成默认背景画刷，渲染开始前总是清除图面，并且在呈现时不忽略Alpha通道。这在渲染到带背景材料的窗口时尤为有用
	/// </summary>
	/// <param name="bTransparent">是否透明</param>
	EckInline constexpr void SetTransparent(BOOL bTransparent)
	{
		m_bTransparent = bTransparent;
		m_eAlphaMode = (bTransparent ? D2D1_ALPHA_MODE_PREMULTIPLIED : D2D1_ALPHA_MODE_IGNORE);
		m_eDxgiAlphaMode = (bTransparent ? DXGI_ALPHA_MODE_PREMULTIPLIED : DXGI_ALPHA_MODE_IGNORE);
	}

	EckInline constexpr BOOL GetTransparent() const { return m_bTransparent; }

	EckInline constexpr int GetChildrenCount() const { return m_cChildren; }

	EckInline constexpr BOOL IsElemUseDComp() const { return m_ePresentMode == PresentMode::AllDComp; }

	EckInline constexpr ID2D1Bitmap1* GetCacheBitmap() const { return m_pBmpCache; }

	void CacheReserve(int cx, int cy)
	{
		if (cx > m_cxCache || cy > m_cyCache)
		{
			if (m_pBmpCache)
				m_pBmpCache->Release();
			m_cxCache = cx;
			m_cyCache = cy;
			const D2D1_BITMAP_PROPERTIES1 Prop
			{
				{ DXGI_FORMAT_B8G8R8A8_UNORM,D2D1_ALPHA_MODE_PREMULTIPLIED },
				(float)m_iUserDpi, (float)m_iUserDpi,
				D2D1_BITMAP_OPTIONS_TARGET
			};
			m_D2d.GetDC()->CreateBitmap(D2D1::SizeU(m_cxCache, m_cyCache),
				nullptr, 0, Prop, &m_pBmpCache);
		}
	}

	void CacheClear()
	{
		if (m_pBmpCache)
			m_pBmpCache->Release();
		m_pBmpCache = nullptr;
		m_cxCache = m_cyCache = 0;
	}
};

inline void CElem::SwitchDefColorTheme(int idxTheme, WPARAM bDark)
{
	auto pctDark = GetWnd()->GetDefColorThemeDark()[CTI_SCROLLBAR];
	auto pctLight = GetWnd()->GetDefColorTheme()[CTI_SCROLLBAR];
	if (m_pColorTheme == pctLight || m_pColorTheme == pctDark || !m_pColorTheme)
		SetColorTheme(bDark ? pctDark : pctLight);
}

inline BOOL CElem::IntCreate(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
	int x, int y, int cx, int cy, CElem* pParent, CDuiWnd* pWnd, int iId, PCVOID pData)
{
	EckAssert(!m_pWnd && !m_pDC);

	m_iId = iId;
	m_pWnd = pWnd;
	m_pDC = pWnd->m_D2d.GetDC();

	IntSetStyle(dwStyle);

	ECK_DUILOCK;
#ifdef _DEBUG
	if (pParent)
		EckAssert(pParent->m_pWnd == pWnd);
#endif
	auto& pParentLastChild = (pParent ? pParent->m_pLastChild : pWnd->m_pLastChild);
	auto& pParentFirstChild = (pParent ? pParent->m_pFirstChild : pWnd->m_pFirstChild);
	m_pParent = pParent;
	if (m_pParent)
		++m_pParent->m_cChildren;
	else
		++m_pWnd->m_cChildren;

	if (pParentLastChild)
	{
		m_pPrev = pParentLastChild;
		m_pNext = nullptr;
		m_pPrev->m_pNext = this;
		pParentLastChild = this;
	}
	else
	{
		m_pPrev = m_pNext = nullptr;
		pParentFirstChild = this;
		pParentLastChild = this;
	}

	CallEvent(WM_NCCREATE, 0, (LPARAM)pData);

	m_rsText = pszText;
	IntSetRect({ x,y,x + cx,y + cy });

	if (GetWnd()->IsElemUseDComp())
		ReCreateDCompVisual();

	if (CallEvent(WM_CREATE, 0, (LPARAM)pData))
	{
		Destroy();
		return FALSE;
	}
	else
	{
		PostMoveSize(TRUE, TRUE, GetWholeRectInClient());
		return TRUE;
	}
}

inline void CElem::Destroy()
{
	ECK_DUILOCK;
	if (m_pParent)
		--m_pParent->m_cChildren;
	else
		--m_pWnd->m_cChildren;
	m_pWnd->CorrectSingleElemMember(this);
	CallEvent(WM_DESTROY, 0, 0);
	DestroyChild(this);

	SafeRelease(m_pDcVisual);
	SafeRelease(m_pDcSurface);
	SafeRelease(m_pDcContent);

	if (m_pPrev)
		m_pPrev->m_pNext = m_pNext;
	else
	{
		if (m_pParent)
			m_pParent->m_pFirstChild = m_pNext;
		else
			m_pWnd->m_pFirstChild = m_pNext;
	}

	if (m_pNext)
		m_pNext->m_pPrev = m_pPrev;
	else
	{
		if (m_pParent)
			m_pParent->m_pLastChild = m_pPrev;
		else
			m_pWnd->m_pLastChild = m_pPrev;
	}

	m_pNext = nullptr;
	m_pPrev = nullptr;
	m_pParent = nullptr;
	m_pFirstChild = nullptr;
	m_pLastChild = nullptr;
	m_pWnd = nullptr;
	m_pDC = nullptr;

	if (m_pColorTheme)
		m_pColorTheme->DeRef();

	m_rc = {};
	m_rcf = {};
	m_rsText.Clear();
	m_dwStyle = 0;
}

inline void CElem::SetZOrder(CElem* pElemAfter)
{
	ECK_DUILOCK;
	auto& pParentLastChild = (m_pParent ? m_pParent->m_pLastChild : m_pWnd->m_pLastChild);
	auto& pParentFirstChild = (m_pParent ? m_pParent->m_pFirstChild : m_pWnd->m_pFirstChild);

	if (pElemAfter == ECK_ELEMTOP)// == NULL
	{
		if (m_pPrev)
		{
			m_pPrev->m_pNext = m_pNext;
			if (m_pNext)
				m_pNext->m_pPrev = m_pPrev;
			else
				pParentLastChild = m_pPrev;

			m_pPrev = nullptr;
			m_pNext = pParentFirstChild;
			if (m_pNext)
				m_pNext->m_pPrev = this;
			pParentFirstChild = this;
		}
		// else: 已在最顶层
	}
	else if (pElemAfter == ECK_ELEMBOTTOM)
	{
		if (m_pNext)
		{
			m_pNext->m_pPrev = m_pPrev;
			if (m_pPrev)
				m_pPrev->m_pNext = m_pNext;
			else
				pParentFirstChild = m_pNext;

			m_pNext = nullptr;
			m_pPrev = pParentLastChild;
			if (m_pPrev)
				m_pPrev->m_pNext = this;
			pParentLastChild = this;
		}
		// else: 已在最底层
	}
	else
	{
		// pElemAfter一定不为NULL
		if (m_pPrev)
			m_pPrev->m_pNext = m_pNext;
		else
			pParentFirstChild = m_pNext;

		if (m_pNext)
			m_pNext->m_pPrev = m_pPrev;
		else
			pParentLastChild = m_pPrev;

		m_pPrev = pElemAfter;
		m_pNext = pElemAfter->m_pNext;

		m_pPrev->m_pNext = this;
		if (m_pNext)
			m_pNext->m_pPrev = this;
		else
			pParentLastChild = this;
	}
}

EckInline LRESULT CElem::GenElemNotify(void* pnm)
{
	if (!GenElemNotifyParent(pnm))
		return m_pWnd->OnElemEvent(this, ((DUINMHDR*)pnm)->uCode, 0, (LPARAM)pnm);
	else
		return 0;
}

inline void CElem::InvalidateRect(const RECT& rc, BOOL bUpdateNow)
{
	ECK_DUILOCK;
	if ((GetStyle() & DES_CONTENT_EXPAND))
		m_rcInvalid = GetWholeRectInClient();
	else
	{
		UnionRect(m_rcInvalid, m_rcInvalid, rc);
		IntersectRect(m_rcInvalid, m_rcInvalid, GetWholeRectInClient());// 裁剪到元素矩形
	}

	if (IsRectEmpty(m_rcInvalid))
		return;
	RECT rcReal{ m_rcInvalid };
	IRUnionContentExpandElemRect(
		(GetStyle() & DES_CONTENT_EXPAND) ? GetWnd()->GetLastChildElem() : this,
		rcReal);

	GetWnd()->UnionInvalidRect(rcReal);
	if (bUpdateNow)
		GetWnd()->WakeRenderThread();
}

inline void CElem::BeginPaint(ELEMPAINTSTRU& eps, WPARAM wParam, LPARAM lParam, UINT uFlags)
{
	eps.prcClip = (const RECT*)lParam;
	eps.rcfClip = MakeD2DRcF(*eps.prcClip);
	m_rcInvalid = {};

	eps.rcfClipInElem = MakeD2DRcF(*eps.prcClip);
	ClientToElem(eps.rcfClipInElem);
	if (uFlags & EBPF_DO_NOT_FILLBK)
		m_pDC->PushAxisAlignedClip(eps.rcfClipInElem, D2D1_ANTIALIAS_MODE_ALIASED);
	else
		if (m_dwStyle & DES_BLURBKG)
		{
			m_pDC->Flush();
			m_pDC->PushAxisAlignedClip(eps.rcfClipInElem, D2D1_ANTIALIAS_MODE_ALIASED);
			BlurD2dDC(m_pDC, GetWnd()->GetD2D().GetBitmap(), GetWnd()->GetCacheBitmap(),
				eps.rcfClip, { eps.rcfClipInElem.left,eps.rcfClipInElem.top }, 10.f);
		}
		else
		{
			m_pDC->PushAxisAlignedClip(eps.rcfClipInElem, D2D1_ANTIALIAS_MODE_ALIASED);
			CallEvent(WM_ERASEBKGND, 0, (LPARAM)&eps);
		}
}

EckInline CElem* CElem::SetCapture() { return m_pWnd->SetCaptureElem(this); }

EckInline void CElem::ReleaseCapture() { m_pWnd->ReleaseCaptureElem(); }

EckInline void CElem::SetFocus() { m_pWnd->SetFocusElem(this); }

EckInline void CElem::InitEasingCurve(CEasingCurve* pec)
{
	pec->SetParam((LPARAM)this);
	GetWnd()->RegisterTimeLine(pec);
}

inline LRESULT CElem::OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_NCHITTEST:
		return HTCLIENT;
	case WM_ERASEBKGND:
	{
		if (GetWnd()->IsElemUseDComp() || (GetStyle() & DES_COMPOSITED))
			m_pDC->Clear({});
	}
	return 1;
	case EWM_COMPOSITE:
	{
		if (GetParentElem())
			return GetParentElem()->CallEvent(uMsg, wParam, lParam);
		else
		{
			EckAssert(GetStyle() & DES_COMPOSITED);
			m_pDC->DrawBitmap(GetCompBitmap(), (D2D1_RECT_F*)wParam,
				1.f, D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
		}
	}
	return 0;
	case WM_KILLFOCUS:
	{
		DUINMHDR nm{ EE_KILLFOCUS };
		GenElemNotifyParent(&nm);
	}
	return 0;
	case WM_SETFOCUS:
	{
		DUINMHDR nm{ EE_SETFOCUS };
		GenElemNotifyParent(&nm);
	}
	return 0;
	case WM_MOVE:
	case WM_SIZE:
	{
		if (GetStyle() & DES_COMPOSITED)
			SetPostCompositedRect(GetRect());
	}
	return 0;
	}
	return 0;
}

inline void CElem::SetRect(const RECT& rc)
{
	ECK_DUILOCK;
	const auto rcOld = GetWholeRectInClient();
	IntSetRect(rc);
	PostMoveSize(TRUE, TRUE, rcOld);
}

inline void CElem::SetPos(int x, int y)
{
	ECK_DUILOCK;
	const auto rcOld = GetWholeRectInClient();
	m_rc = { x,y,x + GetWidth(),y + GetHeight() };
	m_rcf = MakeD2DRcF(m_rc);
	m_rcInClient = m_rc;
	m_rcfInClient = m_rcf;
	if (m_pParent)
	{
		OffsetRect(
			m_rcInClient,
			m_pParent->GetRectInClient().left,
			m_pParent->GetRectInClient().top);
		OffsetRect(
			m_rcfInClient,
			m_pParent->GetRectInClientF().left,
			m_pParent->GetRectInClientF().top);
	}

	SRCorrectChildrenRectInClient();
	PostMoveSize(FALSE, TRUE, rcOld);
}

inline void CElem::SetSize(int cx, int cy)
{
	ECK_DUILOCK;
	const auto rcOld = GetWholeRectInClient();
	m_rc.right = m_rc.left + cx;
	m_rc.bottom = m_rc.top + cy;
	m_rcf.right = m_rcf.left + cx;
	m_rcf.bottom = m_rcf.top + cy;
	m_rcInClient.right = m_rcInClient.left + cx;
	m_rcInClient.bottom = m_rcInClient.top + cy;
	m_rcfInClient.right = m_rcfInClient.left + cx;
	m_rcfInClient.bottom = m_rcfInClient.top + cy;
	PostMoveSize(TRUE, FALSE, rcOld);
}

EckInline void CElem::SetText(PCWSTR pszText)
{
	ECK_DUILOCK;
	m_rsText = pszText;
	CallEvent(WM_SETTEXT, 0, 0);
}

inline void CElem::SetColorTheme(CColorTheme* pColorTheme)
{
	ECK_DUILOCK;
	std::swap(m_pColorTheme, pColorTheme);
	if (m_pColorTheme)
		m_pColorTheme->Ref();
	if (pColorTheme)
		pColorTheme->DeRef();
	CallEvent(WM_THEMECHANGED, 0, 0);
}

inline void CElem::ReCreateDCompVisual()
{
	ECK_DUILOCK;
	EckAssert(GetWnd()->IsElemUseDComp());
	SafeRelease(m_pDcVisual);
	SafeRelease(m_pDcContent);
	SafeRelease(m_pDcSurface);
	const auto pDevice = m_pWnd->m_pDcDevice;
	pDevice->CreateVisual(&m_pDcVisual);
	auto cx = std::max(1, GetWidth()), cy = std::max(1, GetHeight());
	pDevice->CreateSurface(std::max(1, GetWidth()), std::max(1, GetHeight()),
		DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_ALPHA_MODE_PREMULTIPLIED, &m_pDcSurface);
	m_pDcContent = m_pDcSurface;
	m_pDcContent->AddRef();
	m_pDcVisual->SetContent(m_pDcContent);
	const auto pParent = GetParentElem();
	IDCompositionVisual* pRefVisual;
	BOOL bInsertAbove;
	if (GetPrevElem())
	{
		pRefVisual = GetPrevElem()->m_pDcVisual;
		bInsertAbove = TRUE;
	}
	else if (GetNextElem())
	{
		pRefVisual = GetNextElem()->m_pDcVisual;
		bInsertAbove = FALSE;
	}
	else
	{
		pRefVisual = nullptr;
		bInsertAbove = FALSE;
	}
	if (pParent)
		pParent->m_pDcVisual->AddVisual(m_pDcVisual, bInsertAbove, pRefVisual);
	else
		m_pWnd->m_pDcVisual->AddVisual(m_pDcVisual, bInsertAbove, pRefVisual);
}

inline void CElem::ReSizeDCompVisual()
{
	ECK_DUILOCK;
	m_pDcSurface->Release();
	GetWnd()->m_pDcDevice->CreateSurface(std::max(1, GetWidth()), std::max(1, GetHeight()),
		DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_ALPHA_MODE_PREMULTIPLIED, &m_pDcSurface);
	m_pDcVisual->SetContent(m_pDcSurface);
}

inline void CElem::PostMoveSize(BOOL bSize, BOOL bMove, const RECT& rcOld)
{
	if (bSize)
	{
		if (GetWnd()->IsElemUseDComp())
			ReSizeDCompVisual();
		CallEvent(WM_SIZE, 0, 0);
	}
	if (bMove)
	{
		if (GetWnd()->IsElemUseDComp())
		{
			m_pDcVisual->SetOffsetX(GetRectF().left);
			m_pDcVisual->SetOffsetY(GetRectF().top);
		}
		CallEvent(WM_MOVE, 0, 0);
	}
	RECT rc;
	UnionRect(rc, rcOld, GetWholeRectInClient());
}

inline void CElem::SetStyle(DWORD dwStyle)
{
	ECK_DUILOCK;
	const auto dwOldStyle = m_dwStyle;
	IntSetStyle(dwStyle);
	CallEvent(WM_STYLECHANGED, dwOldStyle, m_dwStyle);
}

inline void CElem::SetVisible(BOOL b)
{
	ECK_DUILOCK;
	DWORD dwStyle = GetStyle();
	if (b)
		if (dwStyle & DES_VISIBLE)
			return;
		else
			dwStyle |= DES_VISIBLE;
	else
		if (dwStyle & DES_VISIBLE)
			dwStyle &= ~DES_VISIBLE;
		else
			return;
	SetStyle(dwStyle);
	InvalidateRect();
}

inline constexpr EckInline ID2D1Bitmap1* CElem::GetCompBitmap() const
{
	return GetWnd()->GetCacheBitmap();
}


class CAccServer :public IAccessible
{
private:
	LONG m_cRef{ 1 };
	CDuiWnd& m_Wnd;
	IAccessible* m_pStdAcc{};
public:
	CAccServer(CDuiWnd& wnd) :m_Wnd(wnd)
	{
		CreateStdAccessibleObject(m_Wnd.HWnd, OBJID_CLIENT, IID_IAccessible, (void**)&m_pStdAcc);
	}

	~CAccServer()
	{
		if (m_pStdAcc)
			m_pStdAcc->Release();
	}

	BOOL IsAlive() const { return m_Wnd.IsValid(); }

	// IUnknown
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) override
	{
		QITAB qit[]
		{
			QITABENT(CAccServer, IDispatch),
			QITABENT(CAccServer, IAccessible),
			{ 0 },
		};
		return QISearch(this, qit, riid, ppv);
	}

	ULONG STDMETHODCALLTYPE AddRef(void) override
	{
		return ++m_cRef;
	}

	ULONG STDMETHODCALLTYPE Release(void) override
	{
		if (m_cRef == 1)
		{
			delete this;
			return 0;
		}
		else
			return --m_cRef;
	}

	// IDispatch
	HRESULT STDMETHODCALLTYPE GetTypeInfoCount(UINT* pctinfo) override
	{
		*pctinfo = 0;
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo) override
	{
		*ppTInfo = nullptr;
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) override
	{
		*rgszNames = nullptr;
		*rgDispId = 0;
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags,
		DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) override
	{
		pDispParams = nullptr;
		pVarResult->vt = VT_EMPTY;
		pExcepInfo = nullptr;
		puArgErr = nullptr;
		if (!IsAlive())
			return RPC_E_DISCONNECTED;
		return E_NOTIMPL;
	}

	// IAccessible
	HRESULT STDMETHODCALLTYPE get_accParent(IDispatch** ppdispParent)
	{
		if (!IsAlive())
		{
			*ppdispParent = nullptr;
			return RPC_E_DISCONNECTED;
		}
		return m_pStdAcc->get_accParent(ppdispParent);
	}

	HRESULT STDMETHODCALLTYPE get_accChildCount(long* pcountChildren) override
	{
		if (!IsAlive())
		{
			*pcountChildren = 0;
			return RPC_E_DISCONNECTED;
		}
		*pcountChildren = m_Wnd.GetChildrenCount();
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE get_accChild(VARIANT varChild, IDispatch** ppdispChild) override
	{
		//if (varChild.vt != VT_I4)
		//{
		//	*ppdispChild = NULL;
		//	return E_INVALIDARG;
		//}
		//if (!IsAlive())
		//{
		//	*ppdispChild = NULL;
		//	return RPC_E_DISCONNECTED;
		//}
		//if (varChild.lVal < 1 || varChild.lVal > m_Wnd.GetChildrenCount())
		//{
		//	*ppdispChild = NULL;
		//	return E_INVALIDARG;
		//}

		//auto pElem = m_Wnd.GetFirstChildElem();
		//EckCounterNV(varChild.lVal)
		//	pElem = pElem->GetNextElem();
		//
		//if (!pElem)
		//{
		//	*ppdispChild = NULL;
		//	return E_INVALIDARG;
		//}
		//return pElem->QueryInterface(IID_IDispatch, (void**)ppdispChild);
	}

	HRESULT STDMETHODCALLTYPE get_accName(
		VARIANT varChild,
		BSTR* pszName) override;

	HRESULT STDMETHODCALLTYPE get_accValue(
		VARIANT varChild,
		BSTR* pszValue) override;

	HRESULT STDMETHODCALLTYPE get_accDescription(
		VARIANT varChild,
		BSTR* pszDescription) override;

	HRESULT STDMETHODCALLTYPE get_accRole(
		VARIANT varChild,
		VARIANT* pvarRole) override;

	HRESULT STDMETHODCALLTYPE get_accState(
		VARIANT varChild,
		VARIANT* pvarState) override;

	HRESULT STDMETHODCALLTYPE get_accHelp(
		VARIANT varChild,
		BSTR* pszHelp) override;

	HRESULT STDMETHODCALLTYPE get_accHelpTopic(
		BSTR* pszHelpFile,
		VARIANT varChild,
		long* pidTopic) override;

	HRESULT STDMETHODCALLTYPE get_accKeyboardShortcut(
		VARIANT varChild,
		BSTR* pszKeyboardShortcut) override;

	HRESULT STDMETHODCALLTYPE get_accFocus(
		VARIANT* pvarChild) override;

	HRESULT STDMETHODCALLTYPE get_accSelection(
		VARIANT* pvarChildren) override;

	HRESULT STDMETHODCALLTYPE get_accDefaultAction(
		VARIANT varChild,
		BSTR* pszDefaultAction) override;

	HRESULT STDMETHODCALLTYPE accSelect(
		long flagsSelect,
		VARIANT varChild) override;

	HRESULT STDMETHODCALLTYPE accLocation(
		long* pxLeft,
		long* pyTop,
		long* pcxWidth,
		long* pcyHeight,
		VARIANT varChild) override;

	HRESULT STDMETHODCALLTYPE accNavigate(
		long navDir,
		VARIANT varStart,
		VARIANT* pvarEndUpAt) override;

	HRESULT STDMETHODCALLTYPE accHitTest(
		long xLeft,
		long yTop,
		VARIANT* pvarChild) override;

	HRESULT STDMETHODCALLTYPE accDoDefaultAction(
		VARIANT varChild) override;

	HRESULT STDMETHODCALLTYPE put_accName(
		VARIANT varChild,
		BSTR szName) override;

	HRESULT STDMETHODCALLTYPE put_accValue(
		VARIANT varChild,
		BSTR szValue) override;
};

class CDuiDropTarget :public CDropTarget
{
private:
	CDuiWnd* m_pWnd = nullptr;
public:
	CDuiDropTarget(CDuiWnd* pWnd) :m_pWnd(pWnd) {}

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv)
	{
		const QITAB qit[]
		{
			QITABENT(CDuiDropTarget, IDropTarget),
			{ 0 },
		};
		return QISearch(this, qit, riid, ppv);
	}

	HRESULT STDMETHODCALLTYPE DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
	{
		m_pWnd->m_pDataObj = pDataObj;
		POINT pt0{ pt.x, pt.y };
		ScreenToClient(m_pWnd->HWnd, &pt0);
		DRAGDROPINFO ddi{ pDataObj, grfKeyState, pt, pdwEffect };

		if (m_pWnd->SendMsg(WM_DRAGENTER, (WPARAM)&ddi, MAKELPARAM(pt0.x, pt0.y)))
			return ddi.hr;

		auto pElem = m_pWnd->HitTest(pt0);
		EckAssert(!m_pWnd->m_pDragDropElem);
		m_pWnd->m_pDragDropElem = pElem;

		if (pElem)
			return (HRESULT)pElem->CallEvent(WM_DRAGENTER, (WPARAM)&ddi, MAKELPARAM(pt0.x, pt0.y));
		else
			return S_OK;
	}

	HRESULT STDMETHODCALLTYPE DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
	{
		POINT pt0{ pt.x, pt.y };
		ScreenToClient(m_pWnd->HWnd, &pt0);
		DRAGDROPINFO ddi{ nullptr,grfKeyState, pt, pdwEffect };

		if (m_pWnd->SendMsg(WM_DRAGOVER, (WPARAM)&ddi, MAKELPARAM(pt0.x, pt0.y)))
			return ddi.hr;

		auto pElem = m_pWnd->HitTest(pt0);
		const auto pOldElem = m_pWnd->m_pDragDropElem;
		m_pWnd->m_pDragDropElem = pElem;

		if (pOldElem != pElem)
		{
			if (pOldElem)
				pOldElem->CallEvent(WM_DRAGLEAVE, 0, 0);
			if (pElem)
			{
				ddi.pDataObj = m_pWnd->m_pDataObj;
				return (HRESULT)pElem->CallEvent(WM_DRAGENTER, (WPARAM)&ddi, MAKELPARAM(pt0.x, pt0.y));
			}
		}
		else if (pElem)
			return (HRESULT)pElem->CallEvent(WM_DRAGOVER, (WPARAM)&ddi, MAKELPARAM(pt0.x, pt0.y));
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE DragLeave(void)
	{
		if (m_pWnd->SendMsg(WM_DRAGLEAVE, 0, 0))
		{
			EckAssert(!m_pWnd->m_pDragDropElem);
			m_pWnd->m_pDataObj = nullptr;
			return S_OK;
		}
		m_pWnd->m_pDataObj = nullptr;

		const auto pElem = m_pWnd->m_pDragDropElem;
		if (pElem)
		{
			m_pWnd->m_pDragDropElem = nullptr;
			return (HRESULT)pElem->CallEvent(WM_DRAGLEAVE, 0, 0);
		}
		else
			return S_OK;
	}

	HRESULT STDMETHODCALLTYPE Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
	{
		POINT pt0{ pt.x, pt.y };
		ScreenToClient(m_pWnd->HWnd, &pt0);
		DRAGDROPINFO ddi{ pDataObj, grfKeyState, pt, pdwEffect };

		if (m_pWnd->SendMsg(WM_DROP, (WPARAM)&ddi, MAKELPARAM(pt0.x, pt0.y)))
		{
			EckAssert(!m_pWnd->m_pDragDropElem);
			m_pWnd->m_pDataObj = nullptr;
			return S_OK;
		}
		m_pWnd->m_pDataObj = nullptr;

		auto pElem = m_pWnd->HitTest(pt0);
		m_pWnd->m_pDragDropElem = nullptr;
		if (pElem)
			return (HRESULT)pElem->CallEvent(WM_DROP, (WPARAM)&ddi, MAKELPARAM(pt0.x, pt0.y));
		else
			return S_OK;
	}
};

inline HRESULT CDuiWnd::EnableDragDrop(BOOL bEnable)
{
	if (bEnable)
	{
		if (!m_pDropTarget)
		{
			m_pDropTarget = new CDuiDropTarget(this);
			return RegisterDragDrop(m_hWnd, m_pDropTarget);
		}
	}
	else
	{
		if (m_pDropTarget)
		{
			const auto hr = RevokeDragDrop(m_hWnd);
			m_pDropTarget->Release();
			m_pDropTarget = nullptr;
			return hr;
		}
	}
	return S_FALSE;// indicate do nothing
}

ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END
#else
#error "EckDui requires C++20"
#endif// ECKCXX20