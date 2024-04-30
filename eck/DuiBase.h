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

#define ECK_DUI_NAMESPACE_BEGIN namespace Dui {
#define ECK_DUI_NAMESPACE_END }

#ifdef _DEBUG
#	if 0
#		define ECK_DUI_DBG_DRAW_FRAME					\
			{											\
				ID2D1SolidColorBrush* ECKPRIV_pBr___;	\
				m_pDC->CreateSolidColorBrush(D2D1::ColorF(1.f, 0.f, 0.f, 1.f), &ECKPRIV_pBr___); \
				if (ECKPRIV_pBr___) {					\
					m_pDC->DrawRectangle(GetViewRectF(), ECKPRIV_pBr___, 1.f);	\
					ECKPRIV_pBr___->Release();			\
				}										\
			}
#	else
#		define ECK_DUI_DBG_DRAW_FRAME ;
#	endif
#else
#		define ECK_DUI_DBG_DRAW_FRAME ;
#endif

#define ECK_ELEMTOP			((::eck::Dui::CElem*)HWND_TOP)
#define ECK_ELEMBOTTOM		((::eck::Dui::CElem*)HWND_BOTTOM)


ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
// 元素样式
enum
{
	DES_BLURBKG = (1u << 0),	// 元素具有模糊的背景
	DES_DISABLE = (1u << 1),	// 元素已禁用
	DES_VISIBLE = (1u << 2),	// 元素是可见的
	DES_TRANSPARENT = (1u << 3),// 元素的背景透明，该样式无效果，保留供将来使用
	DES_CONTENT_EXPAND = (1u << 4),	// 更新时必须更新整个元素，若设置了DES_BLURBKG，则强制设置此标志
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

enum
{
	CTI_BUTTON,
	CTI_LIST,
	CTI_LABEL,
	CTI_TRACKBAR,
	CTI_CIRCLEBUTTON,
	CTI_SCROLLBAR,

	CTI_COUNT
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

	RECT m_rc{};
	D2D1_RECT_F m_rcf{};

	RECT m_rcInvalid{};// 客户坐标

	RECT m_rcInClient{};
	D2D1_RECT_F m_rcfInClient{};

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

	CElem* HitTestChildUncheck(POINT pt)
	{
		auto pElem = GetLastChildElem();
		while (pElem)
		{
			if (pElem->GetStyle() & DES_VISIBLE)
			{
				if (PtInRect(pElem->GetRectInClient(), pt))
				{
					const auto pHit = pElem->HitTestChildUncheck(pt);
					if (pHit)
						return pHit;
					else if (pElem->CallEvent(WM_NCHITTEST, 0, MAKELPARAM(pt.x, pt.y)) != HTTRANSPARENT)
						return pElem;
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
					const auto& rcElem = pLast->GetRectInClient();
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

	EckInline void IntSetStyle(DWORD dwStyle)
	{
		if (dwStyle & DES_BLURBKG)
			dwStyle |= (DES_TRANSPARENT | DES_CONTENT_EXPAND);
		m_dwStyle = dwStyle;
	}
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
		}
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

	EckInline void SetStyle(DWORD dwStyle)
	{
		const auto dwOldStyle = m_dwStyle;
		IntSetStyle(dwStyle);
		CallEvent(WM_STYLECHANGED, dwOldStyle, m_dwStyle);
	}

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

	// TODO: 增加非客户区支持
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
		ECK_DUI_DBG_DRAW_FRAME;
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

	EckInline void SetVisible(BOOL b)
	{
		DWORD dwStyle = GetStyle();
		if (b)
			dwStyle |= DES_VISIBLE;
		else
			dwStyle &= ~DES_VISIBLE;
		SetStyle(dwStyle);
		InvalidateRect();
	}
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

	CCriticalSection m_cs{};

	HANDLE m_hthRender = NULL;
	CEvent m_evtRender{};
	std::vector<ITimeLine*> m_vTimeLine{};

	RECT m_rcTotalInvalid{};
	//------其他------
	int m_cxClient = 0;
	int m_cyClient = 0;

	BITBOOL m_bMouseCaptured : 1 = FALSE;
	BITBOOL m_bHasDirty : 1 = FALSE;
	BITBOOL m_bRenderThreadShouldExit : 1 = FALSE;	// 渲染线程应当退出
	BITBOOL m_bSizeChanged : 1 = FALSE;				// 渲染线程应当重设交换链大小

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

	void RedrawElem(CElem* pElem, const RECT& rc)
	{
		RECT rcClip;
		while (pElem)
		{
			if (pElem->GetStyle() & DES_VISIBLE)
			{
				if (pElem->GetRedraw())
				{
					if (IntersectRect(rcClip, pElem->GetRectInClient(), rc))
					{
						if (IsRectEmpty(pElem->m_rcInvalid))
							pElem->m_rcInvalid = pElem->GetRectInClient();

						pElem->m_pDC->SetTransform(D2D1::Matrix3x2F::Translation(
							pElem->GetRectInClientF().left,
							pElem->GetRectInClientF().top));
						pElem->CallEvent(WM_PAINT, 0, (LPARAM)&rcClip);
						RedrawElem(pElem->GetFirstChildElem(), rcClip);
					}
				}
			}
			pElem = pElem->GetNextElem();
		}
	}

	void RedrawDui(const RECT& rc)
	{
		m_D2d.GetDC()->BeginDraw();
		m_D2d.GetDC()->SetTransform(D2D1::Matrix3x2F::Identity());
		FillBackground(MakeD2DRcF(rc));
		RedrawElem(GetFirstChildElem(), rc);
		m_D2d.GetDC()->SetTransform(D2D1::Matrix3x2F::Identity());

		m_D2d.GetDC()->EndDraw();
	}

	EckInline void StartupRenderThread()
	{
		std::thread t([this]
			{
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
					for (auto e : m_vTimeLine)
					{
						if (e->IsValid())
							e->Tick(iDeltaTime);
						bThereAreActiveTimeLine = bThereAreActiveTimeLine || e->IsValid();
					}

					const RECT rcClient{ 0,0,m_cxClient,m_cyClient };
					if (m_bSizeChanged)
					{
						m_rcTotalInvalid = {};
						m_D2d.ReSize(2, m_cxClient, m_cyClient, 0);
						m_bSizeChanged = FALSE;
						if (m_cxClient && m_cyClient)
						{
							RedrawDui(rcClient);
							m_cs.Leave();
							m_D2d.GetSwapChain()->Present(1, 0);
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
							DXGI_PRESENT_PARAMETERS pp{ 1,&rc };
							m_D2d.GetSwapChain()->Present1(1, 0, &pp);
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
			});
		DuplicateHandle(GetCurrentProcess(), t.native_handle(),
			GetCurrentProcess(), &m_hthRender, 0, NULL, DUPLICATE_SAME_ACCESS);
		t.detach();
	}
public:
	ID2D1Bitmap* m_pBmpBlurCache = NULL;

	CDuiWnd()
	{
		constexpr auto c_Black = ColorrefToD2dColorF(Colorref::Black);
		constexpr auto c_White = ColorrefToD2dColorF(Colorref::White);
		constexpr auto c_Gray = ColorrefToD2dColorF(Colorref::Gray);
		constexpr auto c_DisableText = ColorrefToD2dColorF(Colorref::NeutralGray);
		constexpr auto c_DisableBkg = RgbToD2dColorF(0xfbfbf9);
		constexpr auto c_Unused = RgbToD2dColorF(0);
		m_pStdColorTheme[CTI_BUTTON] = new CColorTheme{};
		m_pStdColorTheme[CTI_BUTTON]->Set(
			{
				c_Black,c_Black,RgbToD2dColorF(0x707070),c_DisableText,
				RgbToD2dColorF(0xfefefd),RgbToD2dColorF(0xfafbfa),RgbToD2dColorF(0xfafbfa),c_DisableBkg,RgbToD2dColorF(0xfafbfa),
				RgbToD2dColorF(0xd3d3d2),RgbToD2dColorF(0xecedeb)
			});

		m_pStdColorTheme[CTI_LIST] = new CColorTheme{};
		m_pStdColorTheme[CTI_LIST]->Set(
			COLORTHEME
			{
				.crTextNormal = c_Black,
				.crBkHot = ColorrefToD2dColorF(Colorref::Black, 0.1f),
				.crBkSelected = ColorrefToD2dColorF(Colorref::Black, 0.2f),
				.crBkHotSel = ColorrefToD2dColorF(Colorref::Black, 0.3f),
			});
		m_pStdColorTheme[CTI_LIST]->Set(
			COLORTHEME
			{
				.crTextNormal = c_White,
				.crBkHot = ColorrefToD2dColorF(Colorref::White, 0.3f),
				.crBkSelected = ColorrefToD2dColorF(Colorref::White, 0.4f),
				.crBkHotSel = ColorrefToD2dColorF(Colorref::White, 0.5f),
			});

		m_pStdColorTheme[CTI_LABEL] = new CColorTheme{};
		m_pStdColorTheme[CTI_LABEL]->Set(
			{
				c_Black,c_Black,c_Black,c_DisableText,
				c_White,c_White,c_White,c_DisableBkg,
				c_Unused,c_Unused
			});

		m_pStdColorTheme[CTI_TRACKBAR] = new CColorTheme{};
		m_pStdColorTheme[CTI_TRACKBAR]->Set(
			{
				c_Unused,c_Unused,c_Unused,c_Unused,
				RgbToD2dColorF(0xc6bfbe),c_White/*滑块空白颜色*/,RgbToD2dColorF(0x227988),c_DisableBkg,c_Unused,
				c_Unused,ColorrefToD2dColorF(Colorref::Gray, 0.5)/*滑块描边颜色*/
			});

		m_pStdColorTheme[CTI_CIRCLEBUTTON] = new CColorTheme{};
		m_pStdColorTheme[CTI_CIRCLEBUTTON]->Set(
			{
				c_Unused,c_Unused,c_Unused,c_Unused,
				RgbToD2dColorF(0xededee, 0.4f),
				RgbToD2dColorF(0xe1e1e2, 0.7f),
				RgbToD2dColorF(0xe1e1e2, 0.9f),
				c_DisableBkg,
				RgbToD2dColorF(0xe1e1e2, 0.9f),
				c_Unused,c_Unused
			});
		m_pStdColorTheme[CTI_SCROLLBAR] = new CColorTheme{};
		m_pStdColorTheme[CTI_SCROLLBAR]->Set(
			{
				RgbToD2dColorF(0x85897e),c_Unused,c_Unused,c_Unused,
				RgbToD2dColorF(0xffffff, 0.6f),c_Unused,c_Unused,c_Unused,c_Unused,
				c_Unused,c_Unused
			});
		m_pStdColorTheme[CTI_SCROLLBAR]->Set(
			{
				RgbToD2dColorF(0x85897e),c_Unused,c_Unused,c_Unused,
				RgbToD2dColorF(0xffffff, 0.2f),c_Unused,c_Unused,c_Unused,c_Unused,
				c_Unused,c_Unused
			});
	}

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		if (uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST)
		{
			if (m_bMouseCaptured)
			{
				POINT pt ECK_GET_PT_LPARAM(lParam);
				m_pCurrNcHitTestElem = HitTest(pt);
			}
			auto pElem = (m_pMouseCaptureElem ? m_pMouseCaptureElem : m_pCurrNcHitTestElem);
			if (pElem)
				pElem->CallEvent(uMsg, wParam, lParam);

			if (uMsg == WM_MOUSEMOVE)
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

		if (uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST)
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
			m_pCurrNcHitTestElem = HitTest(pt); 
		}
		break;

		case WM_PAINT:
			ValidateRect(hWnd, NULL);
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

		case WM_COMMAND:// 应用程序可能需要使用菜单
			BroadcastEvent(uMsg, wParam, lParam, TRUE);
			return 0;

		case WM_CREATE:
		{
			auto lResult = CWnd::OnMsg(hWnd, uMsg, wParam, lParam);
			if (!lResult)
			{
				UpdateDpi(GetDpi(hWnd));
				RECT rc;
				GetClientRect(hWnd, &rc);

				m_pDefTextFormat = CreateDefTextFormat(m_iDpi);
				m_pDefTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

				m_D2d.Create(EZD2D_PARAM::MakeFlip(hWnd,
					g_pDxgiFactory, g_pDxgiDevice, g_pD2dDevice, rc.right, rc.bottom));
				m_D2d.GetDC()->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);

				const auto crBkg = D2D1::ColorF(0x191919);
				m_D2d.GetDC()->CreateSolidColorBrush(crBkg, &m_pBrBkg);
				m_cxClient = rc.right;
				m_cyClient = rc.bottom;

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
			m_cs.Enter();
			m_bRenderThreadShouldExit = TRUE;
			WakeRenderThread();
			m_cs.Leave();
			WaitForSingleObject(m_hthRender, INFINITE);// 等待渲染线程退出
			CloseHandle(m_hthRender); 
			m_hthRender = NULL;

			auto pElem = m_pFirstChild;
			while (pElem)
			{
				auto pNext = pElem->GetNextElem();
				pElem->Destroy();
				pElem = pNext;
			}
			m_pFirstChild = m_pLastChild = NULL;
			m_pFocusElem = m_pCurrNcHitTestElem = NULL;

			m_D2d.Destroy();
			if (m_pBmpBkg)
				m_pBmpBkg->Release();
			if (m_pBrBkg)
				m_pBrBkg->Release();

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

	CElem* HitTest(POINT pt)
	{
		auto pElem = m_pLastChild;
		while (pElem)
		{
			if (pElem->GetStyle() & DES_VISIBLE)
			{
				if (PtInRect(pElem->GetRectInClient(), pt))
				{
					auto pHit = pElem->HitTestChildUncheck(pt);
					if (pHit)
						return pHit;
					else if (pElem->CallEvent(WM_NCHITTEST, 0, MAKELPARAM(pt.x, pt.y)) != HTTRANSPARENT)
						return pElem;
				}
			}
			pElem = pElem->GetPrevElem();
		}
		return NULL;
	}

	CElem* SetFocusElem(CElem* pElem)
	{
		SetFocus(HWnd);
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

inline BOOL CElem::IntCreate(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
	int x, int y, int cx, int cy, CElem* pParent, CDuiWnd* pWnd, int iId, PCVOID pData)
{
	EckAssert(!m_pWnd && !m_pDC);

	IntSetStyle(dwStyle);
	m_dwExStyle = dwExStyle;

	m_iId = iId;
	m_pWnd = pWnd;
	m_pDC = pWnd->m_D2d.GetDC();
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
		m_rcInvalid = GetRectInClient();
	else
	{
		UnionRect(m_rcInvalid, m_rcInvalid, *prc);
		IntersectRect(m_rcInvalid, m_rcInvalid, GetRectInClient());// 裁剪到元素矩形
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
	RECT rcOld = m_rcInClient;
	IntSetRect(rc);
	CallEvent(WM_MOVE, 0, 0);
	CallEvent(WM_SIZE, 0, 0);
	UnionRect(rcOld, rcOld, m_rcInClient);
	m_rcInvalid = m_rcInClient;
	IRCheckAndInvalid();
}

inline void CElem::SetPos(int x, int y)
{
	RECT rcOld = m_rcInClient;
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
	UnionRect(rcOld, rcOld, m_rcInClient);
	m_rcInvalid = m_rcInClient;
	IRCheckAndInvalid();
}

inline void CElem::SetSize(int cx, int cy)
{
	ECK_DUILOCK;
	RECT rcOld = m_rcInClient;
	m_rc.right = m_rc.left + cx;
	m_rc.bottom = m_rc.top + cy;
	m_rcf.right = m_rcf.left + cx;
	m_rcf.bottom = m_rcf.top + cy;
	m_rcInClient.right = m_rcInClient.left + cx;
	m_rcInClient.bottom = m_rcInClient.top + cy;
	m_rcfInClient.right = m_rcfInClient.left + cx;
	m_rcfInClient.bottom = m_rcfInClient.top + cy;
	UnionRect(rcOld, rcOld, m_rcInClient);
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

EckInline void CElem::SetColorTheme(CColorTheme* pColorTheme)
{
	ECK_DUILOCK;
	std::swap(m_pColorTheme, pColorTheme);
	if (m_pColorTheme)
		m_pColorTheme->Ref();
	if (pColorTheme)
		pColorTheme->DeRef();
	CallEvent(WM_THEMECHANGED, 0, 0);
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