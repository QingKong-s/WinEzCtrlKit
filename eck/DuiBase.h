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

#include <dcomp.h>
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

	EE_PRIVATE_BEGIN = 0x4000
};

struct DRAGDROPINFO
{
	IDataObject* pDataObj;
	DWORD grfKeyState;
	POINTL ptScreen;
	DWORD* pdwEffect;
	HRESULT hr;
};

enum
{
	WM_DRAGENTER = WM_USER_SAFE,
	WM_DRAGOVER,
	WM_DRAGLEAVE,
	WM_DROP,
	WM_COLORSCHEMECHANGED,

	WM_PRIVBEGIN = 5000,
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
	CElem* m_pNext = NULL;
	CElem* m_pPrev = NULL;
	CElem* m_pParent = NULL;
	CElem* m_pFirstChild = NULL;
	CElem* m_pLastChild = NULL;
	CDuiWnd* m_pWnd = NULL;
	ID2D1DeviceContext* m_pDC = NULL;
	CColorTheme* m_pColorTheme = NULL;

	ID2D1Bitmap1* m_pBitmapComp = NULL;

	RECT m_rc{};
	D2D1_RECT_F m_rcf{};

	RECT m_rcInvalid{};		// 客户坐标

	RECT m_rcInClient{};
	D2D1_RECT_F m_rcfInClient{};

	RECT m_rcPostComposited{};	// 混合到主图面的矩形，至少完全包含元素矩形
	RECT m_rcPostCompositedInClient{};	// 混合到主图面的矩形，相对于客户区

	CRefStrW m_rsText{};

	DWORD m_dwStyle = 0;
	DWORD m_dwExStyle = 0;

	int m_iId = 0;

	BITBOOL m_bAllowRedraw : 1 = TRUE;

	BOOL IntCreate(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, CElem* pParent, CDuiWnd* pWnd, int iId = 0, PCVOID pData = NULL);

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

	CElem* HitTestChildUncheck(POINT pt, LRESULT* pResult = NULL)
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
		return NULL;
	}

	void IRUnionTransparentElemRect(CElem* pLast, RECT& rcInClient)
	{
		while (pLast)
		{
			if (pLast->GetStyle() & DES_VISIBLE)
			{
				if (IsBitSet(pLast->GetStyle(), DES_CONTENT_EXPAND))
				{
					const auto& rcElem = ((
						(pLast->GetStyle() & DES_COMPOSITED) && !(pLast->GetStyle() & DES_INPLACE_COMP)) ?
						pLast->GetPostCompositedRectInClient() :
						pLast->GetRectInClient());
					if (IsRectsIntersect(rcInClient, rcElem))
					{
						pLast->m_rcInvalid = rcElem;
						UnionRect(rcInClient, rcInClient, rcElem);
					}
				}
				else
					IRUnionTransparentElemRect(pLast->GetLastChildElem(), rcInClient);
			}
			pLast = pLast->GetPrevElem();
		}
	}

	void IRCheckAndInvalid();

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
			EckAssert(!m_pBitmapComp);
			ReCreateCompBitmap();
		}

		if ((m_dwStyle & DES_COMPOSITED) && !(dwStyle & DES_COMPOSITED))
		{
			EckAssert(m_pBitmapComp);
			m_pBitmapComp->Release();
			m_pBitmapComp = NULL;
		}
		m_dwStyle = dwStyle;
	}

	void SwitchDefColorTheme(int idxTheme, WPARAM bDark);

	void ReCreateCompBitmap();
public:
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

	virtual LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_NCHITTEST:
			return HTCLIENT;
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

		case WM_SIZE:
		{
			if (GetStyle() & DES_COMPOSITED)
			{
				ReCreateCompBitmap();
			}
		}
		return 0;
		}
		return 0;
	}

	/// <summary>
	/// 元素内容渲染完毕。
	/// 此方法简单调用DrawBitmap，若要自定义混合方案应覆写本方法
	/// </summary>
	/// <param name="rcClip">剪辑矩形</param>
	/// <param name="ox">X偏移</param>
	/// <param name="oy">Y偏移</param>
	/// <returns>保留，必须返回0</returns>
	virtual LRESULT OnComposite(const RECT& rcClip, float ox, float oy)
	{
		EckAssert(GetStyle() & DES_COMPOSITED);
		m_pDC->DrawBitmap(m_pBitmapComp, { ox,oy,ox + GetWidthF(),oy + GetHeightF() },
			1.f, D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
		return 0;
	}

	EckInline LRESULT CallEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		return OnEvent(uMsg, wParam, lParam);
	}

	EckInline const D2D1_RECT_F& GetRectF() const { return m_rcf; }

	EckInline const RECT& GetRect() const { return m_rc; }

	void SetRect(const RECT& rc);

	void SetPos(int x, int y);

	void SetSize(int cx, int cy);

	virtual BOOL Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, CElem* pParent, CDuiWnd* pWnd, int iId = 0, PCVOID pData = NULL)
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

	EckInline DWORD GetStyle() const { return m_dwStyle; }

	EckInline DWORD GetExStyle() const { return m_dwExStyle; }

	void SetStyle(DWORD dwStyle);

	EckInline void SetExStyle(DWORD dwExStyle) { m_dwExStyle = dwExStyle; }

	EckInline CElem* GetNextElem() const { return m_pNext; }
	EckInline CElem* GetPrevElem() const { return m_pPrev; }

	/// <summary>
	/// 是否可见。
	/// 函数自底向上遍历父元素，如果存在不可见父元素，则返回FALSE，否则返回TRUE。
	/// </summary>
	EckInline BOOL IsVisible() const
	{
		auto pParent = this;
		while (pParent)
		{
			if (!(pParent->m_dwStyle & DES_VISIBLE))
				return FALSE;
			pParent = pParent->m_pParent;
		}
		return TRUE;
	}

	EckInline CElem* HitTestChild(POINT pt)
	{
		if (!IsVisible())
			return NULL;
		return HitTestChildUncheck(pt);
	}

	EckInline void ClientToElem(RECT& rc)
	{
		OffsetRect(rc, -m_rcInClient.left, -m_rcInClient.top);
	}

	EckInline void ClientToElem(D2D1_RECT_F& rc)
	{
		OffsetRect(rc, -m_rcfInClient.left, -m_rcfInClient.top);
	}

	EckInline void ClientToElem(POINT& pt)
	{
		pt.x -= m_rcInClient.left;
		pt.y -= m_rcInClient.top;
	}

	EckInline void ClientToElem(D2D1_POINT_2F& pt)
	{
		pt.x -= m_rcfInClient.left;
		pt.y -= m_rcfInClient.top;
	}

	EckInline void ElemToClient(RECT& rc)
	{
		OffsetRect(rc, m_rcInClient.left, m_rcInClient.top);
	}

	EckInline void ElemToClient(D2D1_RECT_F& rc)
	{
		OffsetRect(rc, m_rcfInClient.left, m_rcfInClient.top);
	}

	EckInline void ElemToClient(POINT& pt)
	{
		pt.x += m_rcInClient.left;
		pt.y += m_rcInClient.top;
	}

	EckInline void ElemToClient(D2D1_POINT_2F& pt)
	{
		pt.x += m_rcfInClient.left;
		pt.y += m_rcfInClient.top;
	}

	EckInline int GetWidth() const { return m_rc.right - m_rc.left; }

	EckInline int GetHeight() const { return m_rc.bottom - m_rc.top; }

	EckInline float GetWidthF() const { return m_rcf.right - m_rcf.left; }

	EckInline float GetHeightF() const { return m_rcf.bottom - m_rcf.top; }

	EckInline int GetViewWidth() const { return GetWidth(); }

	EckInline int GetViewHeight() const { return GetHeight(); }

	EckInline float GetViewWidthF() const { return GetWidthF(); }

	EckInline float GetViewHeightF() const { return GetHeightF(); }

	void SetZOrder(CElem* pElemAfter);

	EckInline LRESULT GenElemNotify(void* pnm);

	EckInline LRESULT GenElemNotifyParent(void* pnm)
	{
		if (GetParentElem())
			return GetParentElem()->CallEvent(WM_NOTIFY, (WPARAM)this, (LPARAM)pnm);
		else
			return 0;
	}

	EckInline void SetRedraw(BOOL bRedraw);

	EckInline BOOL GetRedraw() const { return m_bAllowRedraw; }

	void InvalidateRect(const RECT* prc = NULL);

	void InvalidateRectF(const D2D1_RECT_F* prcf = NULL)
	{
		RECT rc;
		if (prcf)
		{
			rc = MakeRect(*prcf);
			InvalidateRect(&rc);
		}
		else
			InvalidateRect(NULL);
	}

	const RECT& GetRectInClient() const { return m_rcInClient; }

	const D2D1_RECT_F& GetRectInClientF() const { return m_rcfInClient; }

	void BeginPaint(ELEMPAINTSTRU& eps, WPARAM wParam, LPARAM lParam, UINT uFlags = 0u);

	EckInline void EndPaint(const ELEMPAINTSTRU& eps)
	{
		m_pDC->PopAxisAlignedClip();
	}

	EckInline CElem* SetCapture();

	EckInline void ReleaseCapture();

	EckInline void SetFocus();

	EckInline D2D1_RECT_F GetViewRectF() const
	{
		return { 0.f,0.f,GetViewWidthF(),GetViewHeightF() };
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

	EckInline void SetID(int iId) { m_iId = iId; }

	EckInline int GetID() const { return m_iId; }

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

	EckInline const RECT& GetWholeRect() const
	{
		return ((
			(GetStyle() & DES_COMPOSITED) && !(GetStyle() & DES_INPLACE_COMP)) ?
			GetPostCompositedRectInClient() :
			GetRectInClient());
	}
};

enum class PresentMode
{
	//						|  等待	|				透明混合					|	 备注	|
	BitBltSwapChain,	//	|	0	|支持，必须无WS_EX_NOREDIRECTIONBITMAP	|  性能极差	|
	FlipSwapChain,		//	|	1	|不支持									|  性能极好	|
	DCompositionSurface,//	|	0	|支持，建议加入WS_EX_NOREDIRECTIONBITMAP	|  建议使用	|
	WindowRenderTarget,	//  |	1	|支持，必须无WS_EX_NOREDIRECTIONBITMAP	|  兼容性好	|
};

/// <summary>
/// DUI窗体类
/// </summary>
class CDuiWnd :public CWnd
{
	friend class CElem;
	friend class CDuiDropTarget;
private:
	//------元素树------
	CElem* m_pFirstChild = NULL;		// 第一个子元素
	CElem* m_pLastChild = NULL;			// 最后一个子元素

	CElem* m_pFocusElem = NULL;			// 当前焦点元素
	CElem* m_pCurrNcHitTestElem = NULL;	// 当前非客户区命中元素
	CElem* m_pMouseCaptureElem = NULL;	// 当前鼠标捕获元素
	CElem* m_pHoverElem = NULL;			// 当前鼠标悬停元素，for WM_MOUSELEAVE
	//------拖放------
	CElem* m_pDragDropElem = NULL;		// 当前拖放元素
	IDataObject* m_pDataObj = NULL;
	CDuiDropTarget* m_pDropTarget = NULL;
	//------图形------
	CEzD2D m_D2d{};
	ID2D1Bitmap* m_pBmpBkg = NULL;
	ID2D1SolidColorBrush* m_pBrBkg = NULL;

	IDWriteTextFormat* m_pDefTextFormat = NULL;

	CColorTheme* m_pStdColorTheme[CTI_COUNT]{};
	CColorTheme* m_pStdColorThemeDark[CTI_COUNT]{};

	CCriticalSection m_cs{};

	HANDLE m_hthRender = NULL;
	CEvent m_evtRender{};
	std::vector<ITimeLine*> m_vTimeLine{};

	RECT m_rcTotalInvalid{};
	//------其他------
	int m_cxClient = 0;
	int m_cyClient = 0;

	BITBOOL m_bMouseCaptured : 1 = FALSE;	// 鼠标是否被捕获
	BITBOOL m_bTransparent : 1 = FALSE;		// 窗口是透明的
	BITBOOL m_bRenderThreadShouldExit : 1 = FALSE;	// 渲染线程应当退出
	BITBOOL m_bSizeChanged : 1 = FALSE;				// 渲染线程应当重设图面大小

	D2D1_ALPHA_MODE m_eAlphaMode{ D2D1_ALPHA_MODE_IGNORE };
	DXGI_ALPHA_MODE m_eDxgiAlphaMode{ DXGI_ALPHA_MODE_IGNORE };

	IDCompositionDevice* m_pDcDevice{};
	IDCompositionTarget* m_pDcTarget{};
	IDCompositionVisual* m_pDcVisual{};
	IDCompositionSurface* m_pDcSurface{};

	ID2D1HwndRenderTarget* m_pHwndRenderTarget{};

	PresentMode m_ePresentMode = PresentMode::FlipSwapChain;

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
			m_pFocusElem = NULL;
		if (m_pCurrNcHitTestElem == pElem)
			m_pCurrNcHitTestElem = NULL;
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

	void RedrawElem(CElem* pElem, const RECT& rc, float ox, float oy)
	{
		const auto pDC = m_D2d.GetDC();
		RECT rcClip;
		while (pElem)
		{
			if ((pElem->GetStyle() & DES_VISIBLE) && pElem->GetRedraw())
			{
				const auto& rcElem = pElem->GetWholeRect();
				if (IntersectRect(rcClip, rcElem, rc))
				{
					if (IsRectEmpty(pElem->m_rcInvalid))
						pElem->m_rcInvalid = pElem->GetRectInClient();
					ID2D1Image* pOldTarget{};
					BOOL bNeedComposite = FALSE;
					if (pElem->GetStyle() & DES_COMPOSITED)
					{
						pDC->Flush();
						pDC->GetTarget(&pOldTarget);
						pDC->SetTarget(pElem->m_pBitmapComp);
						pDC->SetTransform(D2D1::Matrix3x2F::Identity());
						bNeedComposite = TRUE;
					}
					else
					{
						pDC->SetTransform(D2D1::Matrix3x2F::Translation(
							pElem->GetRectInClientF().left + ox,
							pElem->GetRectInClientF().top + oy));
					}
					pElem->CallEvent(WM_PAINT, 0, (LPARAM)&rcClip);
					if (bNeedComposite)
					{
						RedrawElem(pElem->GetFirstChildElem(), rcClip,
							-pElem->GetRectInClientF().left,
							-pElem->GetRectInClientF().top);
						pDC->Flush();
						pDC->SetTarget(pOldTarget);
						pOldTarget->Release();
						pDC->SetTransform(D2D1::Matrix3x2F::Translation(
							pElem->GetRectInClientF().left + ox,
							pElem->GetRectInClientF().top + oy));
						pElem->OnComposite(rcClip, ox, oy);
					}
					else
						RedrawElem(pElem->GetFirstChildElem(), rcClip, ox, oy);
				}
			}
			pElem = pElem->GetNextElem();
		}
	}

	void RedrawDui(const RECT& rc)
	{
		switch (m_ePresentMode)
		{
		case PresentMode::BitBltSwapChain:
		case PresentMode::FlipSwapChain:
		case PresentMode::WindowRenderTarget:
		{
			const auto pDC = m_D2d.GetDC();
			pDC->BeginDraw();
			pDC->SetTransform(D2D1::Matrix3x2F::Identity());
			const auto rcF = MakeD2DRcF(rc);
			if (m_bTransparent)
			{
				pDC->PushAxisAlignedClip(rcF, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
				pDC->Clear({});
				pDC->PopAxisAlignedClip();
			}
			FillBackground(rcF);
			RedrawElem(GetFirstChildElem(), rc, 0.f, 0.f);
			pDC->EndDraw();
		}
		return;
		case PresentMode::DCompositionSurface:
		{
			const auto pDC = m_D2d.GetDC();
			IDXGISurface1* pDxgiSurface = NULL;
			POINT ptOffset;
			m_pDcSurface->BeginDraw(&rc, IID_PPV_ARGS(&pDxgiSurface), &ptOffset);
			const D2D1_BITMAP_PROPERTIES1 D2dBmpProp
			{
				{ DXGI_FORMAT_B8G8R8A8_UNORM,m_eAlphaMode },
				96,
				96,
				D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
				NULL
			};

			ID2D1Bitmap1* pBitmap = NULL;
			pDC->CreateBitmapFromDxgiSurface(pDxgiSurface, &D2dBmpProp, &pBitmap);

			pDC->SetTarget(pBitmap);
			pDC->BeginDraw();
			if (m_bTransparent)
				pDC->Clear({});
			ptOffset.x -= rc.left;
			ptOffset.y -= rc.top;
			RECT rcReal{ rc };
			OffsetRect(rcReal, ptOffset.x, ptOffset.y);
			pDC->SetTransform(D2D1::Matrix3x2F::Identity());
			FillBackground(MakeD2DRcF(rcReal));
			RedrawElem(GetFirstChildElem(), rc, (float)ptOffset.x, (float)ptOffset.y);
			pDC->EndDraw();
			m_pDcSurface->EndDraw();

			pBitmap->Release();
			pDxgiSurface->Release();

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
				const HANDLE hTimer = CreateWaitableTimerW(NULL, FALSE, NULL);
				EckAssert(hTimer);
				BOOL bThereAreActiveTimeLine;

				WaitForSingleObject(m_evtRender.GetHEvent(), INFINITE);
				ULONGLONG ullTime = GetTickCount64() - c_iMinGap;
				for (;;)
				{
					bThereAreActiveTimeLine = FALSE;
					m_cs.Enter();

					if (m_bRenderThreadShouldExit)
					{
						m_cs.Leave();
						break;
					}

					const auto ullCurrTime = GetTickCount64();
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
						m_rcTotalInvalid = {};

						switch (m_ePresentMode)
						{
						case PresentMode::BitBltSwapChain:
							m_D2d.ReSize(1, m_cxClient, m_cyClient, 0, m_eAlphaMode);
							break;
						case PresentMode::FlipSwapChain:
							m_D2d.ReSize(2, m_cxClient, m_cyClient, 0, m_eAlphaMode);
							break;
						case PresentMode::DCompositionSurface:
						{
							IDCompositionSurface* pDcSurface = NULL;
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
							RedrawDui(rcClient);
							m_cs.Leave();
							if (m_ePresentMode == PresentMode::BitBltSwapChain ||
								m_ePresentMode == PresentMode::FlipSwapChain)
								m_D2d.GetSwapChain()->Present(0, 0);
						}
						else
							m_cs.Leave();
					}
					else ECKLIKELY
					{
						// 更新脏矩形
						RECT rc{ m_rcTotalInvalid };
						IntersectRect(rc, rc, rcClient);
						if (!IsRectEmpty(rc))
						{
							m_rcTotalInvalid = {};
							RedrawDui(rc);
							m_cs.Leave();
							// 呈现
							switch (m_ePresentMode)
							{
							case PresentMode::BitBltSwapChain:
								m_D2d.GetSwapChain()->Present(0, 0);
								break;
							case PresentMode::FlipSwapChain:
							{
								DXGI_PRESENT_PARAMETERS pp{ 1,&rc };
								m_D2d.GetSwapChain()->Present1(0, 0, &pp);
							}
							break;
							}
						}
						else
							m_cs.Leave();
					}

					iDeltaTime = (int)(GetTickCount64() - ullCurrTime);
					ullTime = ullCurrTime;
					if (bThereAreActiveTimeLine)
					{
						if (iDeltaTime < c_iMinGap)// 延时
						{
							const LARGE_INTEGER llDueTime
							{ .QuadPart = -10 * (c_iMinGap - iDeltaTime) * 1000LL };
							SetWaitableTimer(hTimer, &llDueTime, 0, NULL, NULL, 0);
							WaitForSingleObject(hTimer, INFINITE);
						}
					}
					else
					{
						WaitForSingleObject(m_evtRender.GetHEvent(), INFINITE);
						ullTime = GetTickCount64() - c_iMinGap;
					}
				}

				CancelWaitableTimer(hTimer);
				CloseHandle(hTimer);
				ThreadUnInit();
			});
		NtDuplicateObject(NtCurrentProcess(), t.native_handle(),
			NtCurrentProcess(), &m_hthRender, 0, 0,
			DUPLICATE_SAME_ACCESS | DUPLICATE_SAME_ATTRIBUTES);
		t.detach();
	}
public:
	ID2D1Bitmap* m_pBmpBlurCache = NULL;

	// 一般不覆写此方法
	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = NULL) override
	{
		if (m_ePresentMode == PresentMode::DCompositionSurface ||
			m_ePresentMode == PresentMode::FlipSwapChain)
			dwExStyle |= WS_EX_NOREDIRECTIONBITMAP;
		return IntCreate(dwExStyle, WCN_DUIHOST, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, g_hInstance, NULL);
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

		if (uMsg >= WM_KEYFIRST && uMsg <= WM_IME_KEYLAST)
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
				ValidateRect(hWnd, NULL);
				UnionInvalidRect(rcInvalid);
				WakeRenderThread();
			}
			else
				ValidateRect(hWnd, NULL);
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
				m_pHoverElem = NULL;
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
						m_pHoverElem = NULL;
					}
					m_pMouseCaptureElem = NULL;
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
				BroadcastEvent(WM_COLORSCHEMECHANGED, ShouldAppsUseDarkMode(), 0);
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
				{
					g_pD2dDevice->CreateDeviceContext(
						EZD2D_PARAM::MakeFlip(0, NULL, NULL, NULL, 0, 0).uDcOptions, &m_D2d.m_pDC);

					DCompositionCreateDevice2(g_pDxgiDevice, IID_PPV_ARGS(&m_pDcDevice));
					m_pDcDevice->CreateTargetForHwnd(hWnd, TRUE, &m_pDcTarget);
					m_pDcDevice->CreateVisual(&m_pDcVisual);
					auto hr = m_pDcDevice->CreateSurface(rc.right, rc.bottom,
						DXGI_FORMAT_B8G8R8A8_UNORM,
						m_eDxgiAlphaMode,
						&m_pDcSurface);
					m_pDcVisual->SetContent(m_D2d.GetSwapChain());
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

				m_rcTotalInvalid = { 0,0,m_cxClient,m_cyClient };

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
			SetWindowPos(hWnd, NULL,
				prc->left, prc->top, prc->right - prc->left, prc->bottom - prc->top,
				SWP_NOZORDER | SWP_NOACTIVATE);
		}
		return 0;

		case WM_DPICHANGED_AFTERPARENT:// for child window
		{
			UpdateDpi(GetDpi(hWnd));
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
			CloseHandle(m_hthRender);
			m_hthRender = NULL;
			// 销毁所有元素
			auto pElem = m_pFirstChild;
			while (pElem)
			{
				auto pNext = pElem->GetNextElem();
				pElem->Destroy();
				pElem = pNext;
			}
			m_pFirstChild = m_pLastChild = NULL;
			m_pFocusElem = m_pCurrNcHitTestElem = NULL;
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
				p = NULL;
			}
			for (auto& p : m_pStdColorThemeDark)
			{
				p->DeRef();
				p = NULL;
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

	EckInline const CEzD2D& GetD2D() const { return m_D2d; }

	EckInline ID2D1Bitmap* GetBkgBitmap() const { return m_pBmpBkg; }

	EckInline ID2D1SolidColorBrush* GetBkgBrush() const {
		return m_pBrBkg;
	}

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

	CElem* HitTest(POINT pt, LRESULT* pResult = NULL)
	{
		auto pElem = m_pLastChild;
		LRESULT l;
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
		return NULL;
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
		UnionRect(m_rcTotalInvalid, m_rcTotalInvalid, rc);
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
		m_rcTotalInvalid = { 0,0,m_cxClient,m_cyClient };
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
};

inline void CElem::IRCheckAndInvalid()
{
	RECT rcReal = m_rcInvalid;
	CElem* pElem =
		((GetStyle() & DES_CONTENT_EXPAND) ? GetWnd()->GetLastChildElem() : this);
	IRUnionTransparentElemRect(pElem, rcReal);

	GetWnd()->UnionInvalidRect(rcReal);
	GetWnd()->WakeRenderThread();
}

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
	m_dwExStyle = dwExStyle;

	ECK_DUILOCK;
#ifdef _DEBUG
	if (pParent)
		EckAssert(pParent->m_pWnd == pWnd);
#endif
	auto& pParentLastChild = (pParent ? pParent->m_pLastChild : pWnd->m_pLastChild);
	auto& pParentFirstChild = (pParent ? pParent->m_pFirstChild : pWnd->m_pFirstChild);
	m_pParent = pParent;

	if (pParentLastChild)
	{
		m_pPrev = pParentLastChild;
		m_pNext = NULL;
		m_pPrev->m_pNext = this;
		pParentLastChild = this;
	}
	else
	{
		m_pPrev = m_pNext = NULL;
		pParentFirstChild = this;
		pParentLastChild = this;
	}

	CallEvent(WM_NCCREATE, 0, (LPARAM)pData);

	m_rsText = pszText;
	IntSetRect({ x,y,x + cx,y + cy });

	if (CallEvent(WM_CREATE, 0, (LPARAM)pData))
	{
		Destroy();
		return FALSE;
	}
	else
	{
		CallEvent(WM_MOVE, 0, 0);
		CallEvent(WM_SIZE, 0, 0);
		return TRUE;
	}
}

inline void CElem::Destroy()
{
	ECK_DUILOCK;
	m_pWnd->CorrectSingleElemMember(this);
	CallEvent(WM_DESTROY, 0, 0);
	DestroyChild(this);

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

	m_pNext = NULL;
	m_pPrev = NULL;
	m_pParent = NULL;
	m_pFirstChild = NULL;
	m_pLastChild = NULL;
	m_pWnd = NULL;
	m_pDC = NULL;

	if (m_pColorTheme)
		m_pColorTheme->DeRef();

	m_rc = {};
	m_rcf = {};
	m_rsText.Clear();
	m_dwStyle = 0;
	m_dwExStyle = 0;
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

			m_pPrev = NULL;
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

			m_pNext = NULL;
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

inline void CElem::InvalidateRect(const RECT* prc)
{
	ECK_DUILOCK;
	if ((GetStyle() & DES_CONTENT_EXPAND) || !prc)
		m_rcInvalid = GetWholeRect();
	else
	{
		UnionRect(m_rcInvalid, m_rcInvalid, *prc);
		IntersectRect(m_rcInvalid, m_rcInvalid, GetWholeRect());// 裁剪到元素矩形
	}
	IRCheckAndInvalid();
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
			BlurD2dDC(m_pDC, m_pWnd->GetD2D().GetBitmap(), m_pWnd->m_pBmpBlurCache,
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

inline void CElem::SetRect(const RECT& rc)
{
	ECK_DUILOCK;
	RECT rcOld = GetWholeRect();
	IntSetRect(rc);
	CallEvent(WM_MOVE, 0, 0);
	CallEvent(WM_SIZE, 0, 0);
	UnionRect(rcOld, rcOld, GetWholeRect());
	m_rcInvalid = m_rcInClient;
	IRCheckAndInvalid();
}

inline void CElem::SetPos(int x, int y)
{
	ECK_DUILOCK;
	RECT rcOld = GetWholeRect();
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
	CallEvent(WM_MOVE, 0, 0);
	UnionRect(rcOld, rcOld, GetWholeRect());
	m_rcInvalid = m_rcInClient;
	IRCheckAndInvalid();
}

inline void CElem::SetSize(int cx, int cy)
{
	ECK_DUILOCK;
	RECT rcOld = GetWholeRect();
	m_rc.right = m_rc.left + cx;
	m_rc.bottom = m_rc.top + cy;
	m_rcf.right = m_rcf.left + cx;
	m_rcf.bottom = m_rcf.top + cy;
	m_rcInClient.right = m_rcInClient.left + cx;
	m_rcInClient.bottom = m_rcInClient.top + cy;
	m_rcfInClient.right = m_rcfInClient.left + cx;
	m_rcfInClient.bottom = m_rcfInClient.top + cy;
	UnionRect(rcOld, rcOld, GetWholeRect());
	m_rcInvalid = m_rcInClient;
	IRCheckAndInvalid();
}

EckInline void CElem::SetText(PCWSTR pszText)
{
	ECK_DUILOCK;
	m_rsText = pszText;
	CallEvent(WM_SETTEXT, 0, 0);
}

EckInline void CElem::SetRedraw(BOOL bRedraw)
{
	ECK_DUILOCK;
	if (bRedraw)
	{
		if (m_bAllowRedraw)
			return;
		m_bAllowRedraw = TRUE;
	}
	else
		m_bAllowRedraw = FALSE;
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

inline void CElem::ReCreateCompBitmap()
{
	ECK_DUILOCK;
	if (m_pBitmapComp)
		m_pBitmapComp->Release();
	D2D1_BITMAP_PROPERTIES1 BmpProp;
	BmpProp.pixelFormat = { DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED };
	BmpProp.dpiX = 96.f;
	BmpProp.dpiY = 96.f;
	BmpProp.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET;
	BmpProp.colorContext = NULL;
	m_pDC->CreateBitmap(D2D1::SizeU(std::max(1, GetWidth()), std::max(1, GetHeight())), NULL, 0,
		BmpProp, &m_pBitmapComp);
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
		dwStyle |= DES_VISIBLE;
	else
		dwStyle &= ~DES_VISIBLE;
	SetStyle(dwStyle);
	InvalidateRect();
}



class CDuiDropTarget :public CDropTarget
{
private:
	CDuiWnd* m_pWnd = NULL;
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
		DRAGDROPINFO ddi{ NULL,grfKeyState, pt, pdwEffect };

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
			m_pWnd->m_pDataObj = NULL;
			return S_OK;
		}
		m_pWnd->m_pDataObj = NULL;

		const auto pElem = m_pWnd->m_pDragDropElem;
		if (pElem)
		{
			m_pWnd->m_pDragDropElem = NULL;
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
			m_pWnd->m_pDataObj = NULL;
			return S_OK;
		}
		m_pWnd->m_pDataObj = NULL;

		auto pElem = m_pWnd->HitTest(pt0);
		m_pWnd->m_pDragDropElem = NULL;
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
			m_pDropTarget = NULL;
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