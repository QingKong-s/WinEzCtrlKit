/*
* WinEzCtrlKit Library
*
* DuiBase.h ： 简单DUI基础设施
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "CWnd.h"

#include "CDuiColorTheme.h"
#include "DuiDef.h"

#include "GraphicsHelper.h"
#include "OleDragDropHelper.h"
#include "SystemHelper.h"
#include "CDwmWndPartMgr.h"

#include "EasingCurve.h"
#include "ITimeLine.h"

#include "CEvent.h"
#include "CCriticalSection.h"
#include "CWaitableTimer.h"

#include <dcomp.h>
#include <oleacc.h>

#if !ECKCXX20
#error "EckDui requires C++20"
#endif// !ECKCXX20

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

#define ECK_DUILOCK			::eck::CCsGuard ECKPRIV_DUI_LOCK_GUARD(GetCriticalSection())
#define ECK_DUILOCKWND		ECK_DUILOCK

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
class CElem :public ILayout
{
	friend class CDuiWnd;
protected:
	//------元素树------
	CElem* m_pNext{};		// 下一元素，Z序高于当前
	CElem* m_pPrev{};		// 上一元素，Z序低于当前
	CElem* m_pParent{};		// 父元素
	CElem* m_pFirstChild{};	// 第一个子元素
	CElem* m_pLastChild{};	// 最后一个子元素
	//------UI系统------
	CDuiWnd* m_pWnd{};		// 所属窗口
	CSignal<Intercept_T, LRESULT, UINT, WPARAM, LPARAM> m_Sig{};// 信号
	//------图形，除DC外渲染和UI线程均可读写------
	ID2D1DeviceContext* m_pDC{};			// DC
	IDCompositionVisual* m_pDcVisual{};		// DComp视觉对象
	IDCompositionSurface* m_pDcSurface{};	// DComp表面
	IUnknown* m_pDcContent{};				// DComp内容
	//------位置，均为DPI虚拟化坐标------
	RECT m_rc{};			// 元素矩形，相对父元素
	D2D1_RECT_F m_rcf{};	// 元素矩形，相对父元素
	RECT m_rcInClient{};	// 元素矩形，相对客户区
	D2D1_RECT_F m_rcfInClient{};	// 元素矩形，相对客户区

	RECT m_rcInvalid{};		// 无效矩形，相对客户区

	RECT m_rcComp{};		// 混合到主图面的矩形，至少完全包含元素矩形
	RECT m_rcCompInClient{};// 混合到主图面的矩形，相对于客户区
	//------属性------
	CRefStrW m_rsText{};	// 标题
	INT_PTR m_iId{};		// 元素ID
	CColorTheme* m_pColorTheme{};		// 颜色主题
	IDWriteTextFormat* m_pTextFormat{};	// 文本格式
	DWORD m_dwStyle{};		// 样式
	int m_cChildren{};		// 子元素数量


	BOOL IntCreate(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, CElem* pParent, CDuiWnd* pWnd,
		int iId = 0, PCVOID pData = nullptr);

	void utcDestroyChild(CElem* pElem)
	{
		auto pChild = pElem->GetFirstChildElem();
		while (pChild)
		{
			auto pNext = pChild->GetNextElem();
			pChild->Destroy();
			pChild = pNext;
		}
	}

	// 合并应更新完整区域的元素矩形
	void tcIrpUnionContentExpandElemRect(CElem* pLast, _Inout_ RECT& rcInClient)
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
					tcIrpUnionContentExpandElemRect(pLast->GetLastChildElem(), rcInClient);
			}
			pLast = pLast->GetPrevElem();
		}
	}

	void tcSrpCorrectChildrenRectInClient()
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
				pElem->m_rcCompInClient = pElem->m_rcComp;
				OffsetRect(pElem->m_rcCompInClient, m_rcInClient.left, m_rcInClient.top);
			}
			pElem->tcSrpCorrectChildrenRectInClient();
			pElem = pElem->GetNextElem();
		}
	}

	void tcSetRectWorker(const RECT& rc)
	{
		m_rc = rc;
		m_rcf = MakeD2DRcF(rc);
		m_rcInClient = rc;
		m_rcfInClient = m_rcf;
		if (m_pParent)
		{
			OffsetRect(m_rcInClient,
				m_pParent->GetRectInClient().left,
				m_pParent->GetRectInClient().top);
			OffsetRect(m_rcfInClient,
				m_pParent->GetRectInClientF().left,
				m_pParent->GetRectInClientF().top);
		}

		tcSrpCorrectChildrenRectInClient();
	}

	void tcSetStyleWorker(DWORD dwStyle)
	{
		if (dwStyle & DES_BLURBKG)
			dwStyle |= (DES_TRANSPARENT | DES_CONTENT_EXPAND);
		if (m_pParent && (m_pParent->GetStyle() & DES_COMPOSITED))
			dwStyle |= DES_COMPOSITED;
		if (!(m_dwStyle & DES_COMPOSITED) && (dwStyle & DES_COMPOSITED))
		{
			if (!(dwStyle & DES_INPLACE_COMP))
				dwStyle |= DES_CONTENT_EXPAND;
		}
		m_dwStyle = dwStyle;
	}

	void utcSwitchDefColorTheme(int idxTheme, WPARAM bDark);

	void utcReCreateDCompVisual();

	void tcReSizeDCompVisual();

	void tcPostMoveSize(BOOL bSize, BOOL bMove, const RECT& rcOld);

	void utcOnUserDpiChanged(int iDpiOld)
	{

	}
public:
	virtual BOOL Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, CElem* pParent, CDuiWnd* pWnd, int iId = 0, PCVOID pData = nullptr)
	{
		return IntCreate(pszText, dwStyle, dwExStyle, x, y, cx, cy, pParent, pWnd, iId, pData);
	}

	void Destroy();

	// 事件处理函数，一般不直接调用此函数
	virtual LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);

	// 调用事件处理
	EckInline LRESULT CallEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		BOOL bProcessed{};
		const auto r = m_Sig.Emit2(bProcessed, uMsg, wParam, lParam);
		if (bProcessed)
			return r;
		return OnEvent(uMsg, wParam, lParam);
	}

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

	// 将缓动曲线对象的自定义参数设为this，并注册
	EckInline void InitEasingCurve(CEasingCurve* pEc);

	EckInline constexpr CDuiWnd* GetWnd() const { return m_pWnd; }
	EckInline constexpr ID2D1DeviceContext* GetDC() const { return m_pDC; }
	EckInline constexpr auto& GetSignal() { return m_Sig; }
	EckInline constexpr CCriticalSection& GetCriticalSection() const;
#pragma region PosSize
	EckInline D2D1_RECT_F GetViewRectF() const
	{
		ECK_DUILOCK;
		return { 0.f,0.f,m_rcf.right - m_rcf.left,m_rcf.bottom - m_rcf.top };
	}

	EckInline RECT GetViewRect() const
	{
		ECK_DUILOCK;
		return { 0,0,m_rc.right - m_rc.left,m_rc.bottom - m_rc.top };
	}

	EckInline constexpr const D2D1_RECT_F& GetRectF() const { return m_rcf; }
	EckInline constexpr const RECT& GetRect() const { return m_rc; }
	EckInline constexpr const RECT& GetRectInClient() const { return m_rcInClient; }
	EckInline constexpr const D2D1_RECT_F& GetRectInClientF() const { return m_rcfInClient; }

	void SetRect(const RECT& rc)
	{
		ECK_DUILOCK;
		const auto rcOld = GetWholeRectInClient();
		tcSetRectWorker(rc);
		tcPostMoveSize(TRUE, TRUE, rcOld);
	}

	void SetPos(int x, int y)
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

		tcSrpCorrectChildrenRectInClient();
		tcPostMoveSize(FALSE, TRUE, rcOld);
	}

	void SetSize(int cx, int cy)
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
		tcPostMoveSize(TRUE, FALSE, rcOld);
	}

	EckInline int GetWidth() const
	{
		ECK_DUILOCK;
		return m_rc.right - m_rc.left;
	}
	EckInline int GetHeight() const
	{
		ECK_DUILOCK;
		return m_rc.bottom - m_rc.top;
	}
	EckInline float GetWidthF() const
	{
		ECK_DUILOCK;
		return m_rcf.right - m_rcf.left;
	}
	EckInline float GetHeightF() const
	{
		ECK_DUILOCK;
		return m_rcf.bottom - m_rcf.top;
	}

	EckInline constexpr int Log2Phy(int i) const;
	EckInline constexpr float Log2PhyF(float f) const;
	EckInline constexpr int Phy2Log(int i) const;
	EckInline constexpr float Phy2LogF(float f) const;

	EckInline int GetPhysicalWidth() const { return Log2Phy(GetWidth()); }
	EckInline int GetPhysicalHeight() const { return Log2Phy(GetHeight()); }
	EckInline float GetPhysicalWidthF() const { return Log2PhyF(GetWidthF()); }
	EckInline float GetPhysicalHeightF() const { return Log2PhyF(GetHeightF()); }
#pragma endregion PosSize
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
#pragma region ElemTree
	EckInline constexpr CElem* GetFirstChildElem() const { return m_pFirstChild; }
	EckInline constexpr CElem* GetLastChildElem() const { return m_pLastChild; }
	EckInline constexpr CElem* GetParentElem() const { return m_pParent; }
	// 取下一元素，Z序高于当前
	EckInline constexpr CElem* GetNextElem() const { return m_pNext; }
	// 取上一元素，Z序低于当前
	EckInline constexpr CElem* GetPrevElem() const { return m_pPrev; }

	static CElem* ElemFromPoint(CElem* pElem, POINT pt, _Out_opt_ LRESULT* pResult = nullptr)
	{
		COMP_POS cp;
		cp.bNormalToComp = TRUE;
		while (pElem)
		{
			if (pElem->GetStyle() & DES_VISIBLE)
			{
				cp.pt = pt;
				if (pElem->IsNeedCoordinateTransform())
				{
					cp.pElem = pElem;
					pElem->ClientToElem(cp.pt);
					pElem->CallEvent(EWM_COMP_POS, (WPARAM)&cp, 0);
					pElem->ElemToClient(cp.pt);
				}
				if (PtInRect(pElem->GetRectInClient(), cp.pt))
				{
					const auto pHit = pElem->ElemFromPoint(pt, pResult);
					if (pHit)
						return pHit;
					else if (LRESULT lResult;
						(lResult = pElem->CallEvent(WM_NCHITTEST,
							0, MAKELPARAM(cp.pt.x, cp.pt.y))) != HTTRANSPARENT)
					{
						if (pResult)
							*pResult = lResult;
						return pElem;
					}
				}
			}
			pElem = pElem->GetPrevElem();
		}
		if (pResult)
			*pResult = HTNOWHERE;
		return nullptr;
	}

	EckInline CElem* ElemFromPoint(POINT pt, _Out_opt_ LRESULT* pResult = nullptr)
	{
		return ElemFromPoint(GetLastChildElem(), pt, pResult);
	}

	EckInline void ClientToElem(_Inout_ RECT& rc) const
	{
		ECK_DUILOCK;
		OffsetRect(rc, -m_rcInClient.left, -m_rcInClient.top);
	}

	EckInline void ClientToElem(_Inout_ D2D1_RECT_F& rc) const
	{
		ECK_DUILOCK;
		OffsetRect(rc, -m_rcfInClient.left, -m_rcfInClient.top);
	}

	EckInline void ClientToElem(_Inout_ POINT& pt) const
	{
		ECK_DUILOCK;
		pt.x -= m_rcInClient.left;
		pt.y -= m_rcInClient.top;
	}

	EckInline void ClientToElem(_Inout_ D2D1_POINT_2F& pt) const
	{
		ECK_DUILOCK;
		pt.x -= m_rcfInClient.left;
		pt.y -= m_rcfInClient.top;
	}

	EckInline void ElemToClient(_Inout_ RECT& rc) const
	{
		ECK_DUILOCK;
		OffsetRect(rc, m_rcInClient.left, m_rcInClient.top);
	}

	EckInline void ElemToClient(_Inout_ D2D1_RECT_F& rc) const
	{
		ECK_DUILOCK;
		OffsetRect(rc, m_rcfInClient.left, m_rcfInClient.top);
	}

	EckInline void ElemToClient(_Inout_ POINT& pt) const
	{
		ECK_DUILOCK;
		pt.x += m_rcInClient.left;
		pt.y += m_rcInClient.top;
	}

	EckInline void ElemToClient(_Inout_ D2D1_POINT_2F& pt) const
	{
		ECK_DUILOCK;
		pt.x += m_rcfInClient.left;
		pt.y += m_rcfInClient.top;
	}
#pragma endregion ElemTree
#pragma region OthersProp
	// 取标题
	EckInline const CRefStrW& GetText() const { return m_rsText; }
	// 置标题
	EckInline void SetText(PCWSTR pszText, int cchText = -1)
	{
		ECK_DUILOCK;
		m_rsText.DupString(pszText, cchText);
		CallEvent(WM_SETTEXT, 0, 0);
	}

	// 取样式
	EckInline constexpr DWORD GetStyle() const { return m_dwStyle; }
	// 置样式
	void SetStyle(DWORD dwStyle)
	{
		ECK_DUILOCK;
		const auto dwOldStyle = m_dwStyle;
		tcSetStyleWorker(dwStyle);
		CallEvent(WM_STYLECHANGED, dwOldStyle, m_dwStyle);
	}

	// 置可见性
	void SetVisible(BOOL b)
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
	// 是否可见。函数自底向上遍历父元素，如果存在不可见父元素，则返回FALSE，否则返回TRUE。
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

	// 置Z序
	void SetZOrder(CElem* pElemAfter);

	// 启用/禁用重绘
	EckInline void SetRedraw(BOOL bRedraw)
	{
		if (bRedraw)
			SetStyle(GetStyle() & ~DES_DISALLOW_REDRAW);
		else
			SetStyle(GetStyle() | DES_DISALLOW_REDRAW);
	}
	// 是否允许重绘
	EckInline constexpr BOOL GetRedraw() const { return !(GetStyle() & DES_DISALLOW_REDRAW); }

	// 置文本格式【不能在渲染线程调用】
	EckInline void SetTextFormat(IDWriteTextFormat* pTf)
	{
		ECK_DUILOCK;
		m_pTextFormat = pTf;
		CallEvent(WM_SETFONT, 0, 0);
	}
	// 取文本格式
	EckInline constexpr IDWriteTextFormat* GetTextFormat() const { return m_pTextFormat; }

	// 置元素ID【不能在渲染线程调用】
	EckInline constexpr void SetID(INT_PTR iId) { m_iId = iId; }
	// 取元素ID
	EckInline constexpr INT_PTR GetID() const { return m_iId; }

	// 置颜色主题
	EckInline void SetColorTheme(CColorTheme* pColorTheme)
	{
		ECK_DUILOCK;
		std::swap(m_pColorTheme, pColorTheme);
		if (m_pColorTheme)
			m_pColorTheme->Ref();
		if (pColorTheme)
			pColorTheme->DeRef();
		CallEvent(WM_THEMECHANGED, 0, 0);
	}
	// 取颜色主题
	EckInline constexpr const CColorTheme* GetColorTheme() const { return m_pColorTheme; }
#pragma endregion OthersProp
#pragma region ElemFunc
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

	/// <summary>
	/// 开始画图。
	/// 目前所有元素仅能在处理WM_PAINT事件时绘制，处理时必须调用此函数且必须与EndPaint配对
	/// </summary>
	/// <param name="eps">画图信息结构的引用，将返回画图参数</param>
	/// <param name="wParam">事件wParam，目前未用</param>
	/// <param name="lParam">事件lParam</param>
	/// <param name="uFlags">标志，EBPF_常量</param>
	void BeginPaint(ELEMPAINTSTRU& eps, WPARAM wParam, LPARAM lParam, UINT uFlags = 0u);

	/// <summary>
	/// 结束画图
	/// </summary>
	/// <param name="eps">BeginPaint返回的结构，目前未用</param>
	EckInline void EndPaint(const ELEMPAINTSTRU& eps)
	{
		m_pDC->PopAxisAlignedClip();
	}

	// 捕获鼠标【不能在渲染线程调用】
	EckInline CElem* SetCapture();
	// 释放鼠标
	EckInline void ReleaseCapture();

	// 置焦点【不能在渲染线程调用】
	EckInline void SetFocus();
#pragma endregion ElemFunc
#pragma region Composite
	// 取混合矩形，相对元素
	EckInline constexpr const RECT& GetPostCompositedRect() const { return m_rcComp; }
	// 取混合矩形，相对客户区
	EckInline constexpr const RECT& GetPostCompositedRectInClient() const { return m_rcCompInClient; }
	// 置混合矩形，相对元素
	EckInline void SetPostCompositedRect(const RECT& rc)
	{
		ECK_DUILOCK;
		m_rcComp = rc;
		m_rcCompInClient = rc;
		OffsetRect(m_rcCompInClient, m_rcInClient.left, m_rcInClient.top);
	}

	// 取完全包围元素的矩形，相对客户区
	EckInline const RECT& GetWholeRectInClient() const
	{
		return (((GetStyle() & DES_COMPOSITED) && !(GetStyle() & DES_INPLACE_COMP)) ?
			GetPostCompositedRectInClient() :
			GetRectInClient());
	}

	// 取混合位图
	EckInline constexpr ID2D1Bitmap1* GetCacheBitmap() const;

	// 是否需要坐标变换
	EckInline BOOL IsNeedCoordinateTransform() const
	{
		return !!(GetStyle() & DES_COMPOSITED);
	}
#pragma endregion Composite
};

class CDuiWnd :public CWnd
{
	friend class CElem;
	friend class CDuiDropTarget;
private:
	//------元素树------
	CElem* m_pFirstChild{};	// 第一个子元素
	CElem* m_pLastChild{};	// 最后一个子元素
	int m_cChildren{};		// 子元素数量，UIAccessible使用
	//------UI系统------
	PresentMode m_ePresentMode{ PresentMode::FlipSwapChain };	// 呈现模式
	CElem* m_pFocusElem{};	// 当前焦点元素
	CElem* m_pHoverElem{};	// 当前鼠标悬停元素，WM_MOUSELEAVE使用
	CElem* m_pCurrNcHitTestElem{};	// 当前非客户区命中元素
	CElem* m_pMouseCaptureElem{};	// 当前鼠标捕获元素

	CCriticalSection m_cs{};// 渲染线程同步临界区
	CEvent m_EvtRender{};	// 渲染线程事件对象
	HANDLE m_hthRender{};	// 渲染线程句柄

	std::vector<ITimeLine*> m_vTimeLine{};	// 时间线
	//------拖放------
	CElem* m_pDragDropElem{};		// 当前拖放元素
	IDataObject* m_pDataObj{};		// 当前拖放的数据对象
	CDuiDropTarget* m_pDropTarget{};// 拖放目标
	//------图形------
	RECT m_rcInvalid{};				// 无效矩形

	CEzD2D m_D2d{};					// D2D设备上下文相关，若呈现模式不为交换链，则仅DC字段有效
	ID2D1Bitmap* m_pBmpBkg{};		// 背景位图
	ID2D1SolidColorBrush* m_pBrBkg{};		// 背景画刷

	IDCompositionDevice* m_pDcDevice{};		// DComp设备
	IDCompositionTarget* m_pDcTarget{};		// DComp目标
	IDCompositionVisual* m_pDcVisual{};		// 根视觉对象
	IDCompositionSurface* m_pDcSurface{};	// 根视觉对象的内容

	ID2D1HwndRenderTarget* m_pRtHwnd{};		// 窗口渲染目标，仅当呈现模式为窗口渲染目标时有效

	ID2D1Bitmap1* m_pBmpCache{};	// 缓存位图，模糊或独立混合等可在其上进行
	int m_cxCache{};				// 缓存位图宽度
	int m_cyCache{};				// 缓存位图高度

	CColorTheme* m_pStdColorTheme[CTI_COUNT]{};		// 默认亮色主题
	CColorTheme* m_pStdColorThemeDark[CTI_COUNT]{};	// 默认暗色主题

	//------其他------
	int m_cxClient = 0;		// 客户区宽度
	int m_cyClient = 0;		// 客户区高度

	BITBOOL m_bMouseCaptured : 1 = FALSE;	// 鼠标是否被捕获
	BITBOOL m_bTransparent : 1 = FALSE;		// 窗口是透明的
	BITBOOL m_bRenderThreadShouldExit : 1 = FALSE;	// 渲染线程应当退出
	BITBOOL m_bSizeChanged : 1 = FALSE;				// 渲染线程应当重设图面大小
	BITBOOL m_bUserDpiChanged : 1 = FALSE;			// 渲染线程应当重设DPI
	BITBOOL m_bFullUpdate : 1 = TRUE;

	D2D1_ALPHA_MODE m_eAlphaMode{ D2D1_ALPHA_MODE_IGNORE };		// 缓存D2D透明模式
	DXGI_ALPHA_MODE m_eDxgiAlphaMode{ DXGI_ALPHA_MODE_IGNORE };	// 缓存DXGI透明模式

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
			ElemReleaseCapture();
		if (m_pHoverElem == pElem)
		{
			POINT pt;
			GetCursorPos(&pt);
			ScreenToClient(m_hWnd, &pt);
			Phy2Log(pt);
			m_pHoverElem = ElemFromPoint(pt);
		}
	}

	/// <summary>
	/// 重画元素树
	/// </summary>
	/// <param name="pElem">起始元素</param>
	/// <param name="rc">重画区域，逻辑坐标</param>
	/// <param name="ox">X偏移，仅用于DComp</param>
	/// <param name="oy">Y偏移，仅用于DComp</param>
	void RedrawElem(CElem* pElem, const RECT& rc, float ox, float oy)
	{
		const auto pDC = m_D2d.GetDC();
		RECT rcClip;// 相对客户区
		ID2D1Image* pOldTarget{};

		IDXGISurface1* pDxgiSurface{};
		ID2D1Bitmap1* pBitmap{};

		BOOL bNeedComposite{};
		COMP_INFO ci;
		while (pElem)
		{
			const auto& rcElem = pElem->GetRectInClient();
			if (const auto dwStyle = pElem->GetStyle();
				!(dwStyle & DES_VISIBLE) || (dwStyle & (DES_DISALLOW_REDRAW | DES_EXTERNAL_CONTENT)) ||
				IsRectEmpty(rcElem))
				goto NextElem;
			if (!IntersectRect(rcClip, rcElem, m_rcInvalid))
				goto NextElem;

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
					(float)m_iUserDpi,
					(float)m_iUserDpi,
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

	/// <summary>
	/// 重画DUI
	/// </summary>
	/// <param name="rc">重画区域，逻辑坐标</param>
	void RedrawDui(const RECT& rc, BOOL bFullUpdate = FALSE)
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
			const auto rcF{ MakeD2DRcF(rc) };
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
		case PresentMode::AllDComp:
		{
			const auto pDC = m_D2d.GetDC();
			IDXGISurface1* pDxgiSurface = nullptr;
			POINT ptOffset;
			RECT rcPhy{ rc };
			Log2Phy(rcPhy);

			if (m_iUserDpi != 96)// 误差修正
			{
				auto t = MakeD2DRcF(rcPhy);
				Phy2Log(t);
				if (t.left < rc.left)
					++rcPhy.left;
				if (t.top < rc.top)
					++rcPhy.top;
				if (t.right > rc.right)
					--rcPhy.right;
				if (t.bottom > rc.bottom)
					--rcPhy.bottom;
			}
			m_pDcSurface->BeginDraw(bFullUpdate ? nullptr : &rcPhy,
				IID_PPV_ARGS(&pDxgiSurface), &ptOffset);
			ptOffset.x -= rcPhy.left;
			ptOffset.y -= rcPhy.top;
			const D2D1_POINT_2F ptLogOffsetF{ Phy2LogF((float)ptOffset.x), Phy2LogF((float)ptOffset.y) };

			const D2D1_BITMAP_PROPERTIES1 D2dBmpProp
			{
				{ DXGI_FORMAT_B8G8R8A8_UNORM,m_eAlphaMode },
				(float)m_iUserDpi,
				(float)m_iUserDpi,
				D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
				nullptr
			};

			ID2D1Bitmap1* pBitmap = nullptr;
			pDC->CreateBitmapFromDxgiSurface(pDxgiSurface, &D2dBmpProp, &pBitmap);
			pDC->SetTarget(pBitmap);
			pDC->BeginDraw();
			pDC->SetTransform(D2D1::Matrix3x2F::Identity());

			m_D2d.m_pBitmap = pBitmap;

			auto rcF = MakeD2DRcF(rc);
			OffsetRect(rcF, ptLogOffsetF.x, ptLogOffsetF.y);
			if (m_bTransparent)
			{
				pDC->PushAxisAlignedClip(rcF, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
				pDC->Clear({});
				pDC->PopAxisAlignedClip();
			}
			FillBackground(rcF);
			if (!IsElemUseDComp())
			{
				RedrawElem(GetFirstChildElem(), rc,
					ptLogOffsetF.x, ptLogOffsetF.y);
			}

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

	void RenderThread()
	{
		constexpr int c_iMinGap = 16;
		CWaitableTimer Timer{};
		BOOL bActiveTimeLine;

		WaitObject(m_EvtRender);
		ULONGLONG ullTime = NtGetTickCount64() - c_iMinGap;
		EckLoop()
		{
			bActiveTimeLine = FALSE;
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
				bActiveTimeLine = bActiveTimeLine || e->IsValid();
			}

			RECT rcClient{ 0,0,m_cxClient,m_cyClient };
			Phy2Log(rcClient);
			if (m_bSizeChanged || m_bUserDpiChanged)
			{
				constexpr auto D2dBmpOpt = D2D1_BITMAP_OPTIONS_TARGET |
					D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
				if (m_bUserDpiChanged)
				{
					CacheClear();
					m_D2d.GetDC()->SetDpi((float)m_iUserDpi, (float)m_iUserDpi);
					m_bUserDpiChanged = FALSE;
					if (!m_bSizeChanged)
						goto SkipReSize;
				}
				m_bSizeChanged = FALSE;
				switch (m_ePresentMode)
				{
				case PresentMode::BitBltSwapChain:
					m_D2d.ReSize(1, m_cxClient, m_cyClient, 0,
						m_eAlphaMode, D2dBmpOpt, (float)m_iUserDpi);
					break;
				case PresentMode::FlipSwapChain:
					m_D2d.ReSize(2, m_cxClient, m_cyClient, 0,
						m_eAlphaMode, D2dBmpOpt, (float)m_iUserDpi);
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
				}
				break;
				case PresentMode::WindowRenderTarget:
					m_pRtHwnd->Resize(D2D1::SizeU(m_cxClient, m_cyClient));
					break;
				default:
					ECK_UNREACHABLE;
				}
			SkipReSize:
				if (m_cxClient && m_cyClient)
				{
					m_rcInvalid = rcClient;
					RedrawDui(rcClient, TRUE);
					m_cs.Leave();
					if (m_ePresentMode == PresentMode::BitBltSwapChain ||
						m_ePresentMode == PresentMode::FlipSwapChain)
						m_D2d.GetSwapChain()->Present(0, 0);
				}
				else
				{
					m_rcInvalid = {};
					m_cs.Leave();
				}
			}
			else ECKLIKELY
			{
				// 更新脏矩形
				if (!IsRectEmpty(m_rcInvalid))
				{
					RECT rc;
					IntersectRect(rc, m_rcInvalid, rcClient);
					if (IsRectEmpty(rc))
						goto NoRedraw;
					RedrawDui(rc, m_bFullUpdate);
					if (m_bFullUpdate)
						m_bFullUpdate = FALSE;
					m_rcInvalid = {};
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
							.DirtyRectsCount = 1,
							.pDirtyRects = &rc,
						};
						m_D2d.GetSwapChain()->Present1(0, 0, &pp);
					}
					break;
					}
				}
				else
				{
				NoRedraw:
					m_cs.Leave();
				}
			}

			iDeltaTime = int(NtGetTickCount64() - ullCurrTime);
			ullTime = ullCurrTime;
			if (bActiveTimeLine)
			{
				if (iDeltaTime < c_iMinGap)// 延时
				{
					Timer.SetDueTime(c_iMinGap - iDeltaTime);
					WaitObject(Timer);
				}
			}
			else
			{
				WaitObject(m_EvtRender);
				ullTime = NtGetTickCount64() - c_iMinGap;
			}
		}

		Timer.Cancel();
	}

	EckInline void StartupRenderThread()
	{
		m_hthRender = CrtCreateThread([](void* p)->UINT
			{
				((CDuiWnd*)p)->RenderThread();
				return 0;
			}, this);
	}
public:
	ECK_CWND_CREATE;
	// 一般不覆写此方法
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
			POINT pt ECK_GET_PT_LPARAM(lParam);
			Phy2Log(pt);

			if (m_bMouseCaptured)
				m_pCurrNcHitTestElem = ElemFromPoint(pt);

			auto pElem = (m_pMouseCaptureElem ? m_pMouseCaptureElem : m_pCurrNcHitTestElem);
			if (pElem)
			{
				if (pElem->IsNeedCoordinateTransform())
				{
					COMP_POS cp
					{
						.pElem = pElem,
						.pt = pt,
						.bNormalToComp = TRUE
					};
					pElem->ClientToElem(cp.pt);
					pElem->CallEvent(EWM_COMP_POS, (WPARAM)&cp, 0);
					pElem->ElemToClient(cp.pt);
					pElem->CallEvent(uMsg, wParam, MAKELPARAM(cp.pt.x, cp.pt.y));
				}
				else
					pElem->CallEvent(uMsg, wParam, MAKELPARAM(pt.x, pt.y));
			}

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
				m_pCurrNcHitTestElem = ElemFromPoint(pt);
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
			Phy2Log(pt);
			LRESULT lResult;
			if (m_pCurrNcHitTestElem = ElemFromPoint(pt, &lResult))
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
				IrUnion(rcInvalid);
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
				m_iDpi = GetDpi(hWnd);
				RECT rc;
				GetClientRect(hWnd, &rc);

				MakeStdThemeLight(m_pStdColorTheme);
				MakeStdThemeDark(m_pStdColorThemeDark);

				rc.right = std::max(rc.right, 8L);
				rc.bottom = std::max(rc.bottom, 8L);
				switch (m_ePresentMode)
				{
				case PresentMode::BitBltSwapChain:
				{
					auto Param = EZD2D_PARAM::MakeBitblt(hWnd, g_pDxgiFactory, g_pDxgiDevice,
						g_pD2dDevice, rc.right, rc.bottom, (float)m_iUserDpi);
					Param.uBmpAlphaMode = m_eAlphaMode;
					m_D2d.Create(Param);
				}
				break;
				case PresentMode::FlipSwapChain:
				{
					auto Param = EZD2D_PARAM::MakeFlip(hWnd, g_pDxgiFactory, g_pDxgiDevice,
						g_pD2dDevice, rc.right, rc.bottom, (float)m_iUserDpi);
					Param.uBmpAlphaMode = m_eAlphaMode;
					m_D2d.Create(Param);
				}
				break;
				case PresentMode::DCompositionSurface:
				case PresentMode::AllDComp:
				{
					g_pD2dDevice->CreateDeviceContext(
						EZD2D_PARAM::MakeFlip(0, nullptr, nullptr, nullptr, 0, 0).uDcOptions,
						&m_D2d.m_pDC);

					DCompositionCreateDevice3(g_pDxgiDevice, IID_PPV_ARGS(&m_pDcDevice));
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
					RtProp.dpiX = RtProp.dpiY = (float)m_iUserDpi;
					RtProp.usage = D2D1_RENDER_TARGET_USAGE_NONE;
					RtProp.minLevel = D2D1_FEATURE_LEVEL_DEFAULT;
					D2D1_HWND_RENDER_TARGET_PROPERTIES HwRtProp;
					HwRtProp.hwnd = hWnd;
					HwRtProp.pixelSize = D2D1::SizeU(rc.right, rc.bottom);
					HwRtProp.presentOptions = D2D1_PRESENT_OPTIONS_NONE;
					g_pD2dFactory->CreateHwndRenderTarget(RtProp, HwRtProp, &m_pRtHwnd);
					const auto hr = m_pRtHwnd->QueryInterface(&m_D2d.m_pDC);
					EckAssert(SUCCEEDED(hr));
				}
				break;
				default:
					ECK_UNREACHABLE;
				}

				m_D2d.GetDC()->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
				if (m_ePresentMode != PresentMode::WindowRenderTarget)
					m_D2d.GetDC()->SetDpi((float)m_iUserDpi, (float)m_iUserDpi);

				if (!m_bTransparent)
				{
					const auto crBkg = ColorrefToD2dColorF(GetThreadCtx()->crDefBkg);
					m_D2d.GetDC()->CreateSolidColorBrush(crBkg, &m_pBrBkg);
				}
				m_cxClient = rc.right;
				m_cyClient = rc.bottom;

				m_bFullUpdate = TRUE;
				m_rcInvalid = { 0,0,m_cxClient,m_cyClient };

				m_EvtRender.NoSignal();
				StartupRenderThread();
			}
			return lResult;
		}
		break;

		case WM_DPICHANGED:// For top-level window.
		{
			m_iDpi = HIWORD(wParam);
			auto prc = (const RECT*)lParam;
			SetWindowPos(hWnd, nullptr,
				prc->left, prc->top, prc->right - prc->left, prc->bottom - prc->top,
				SWP_NOZORDER | SWP_NOACTIVATE);
		}
		return 0;

		case WM_DPICHANGED_AFTERPARENT:// For child window.
		{
			m_iDpi = GetDpi(hWnd);
		}
		return 0;

		case WM_DESTROY:
		{
			// 终止渲染线程
			m_cs.Enter();
			m_bRenderThreadShouldExit = TRUE;
			WakeRenderThread();
			m_cs.Leave();
			WaitObject(m_hthRender);// 等待渲染线程退出
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
			SafeRelease(m_pRtHwnd);
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

	EckInline constexpr ID2D1SolidColorBrush* GetBkgBrush() const { return m_pBrBkg; }

	EckInline void SetBkgBitmap(ID2D1Bitmap* pBmp)
	{
		ECK_DUILOCKWND;
		std::swap(m_pBmpBkg, pBmp);
		if (m_pBmpBkg)
			m_pBmpBkg->AddRef();
		if (pBmp)
			pBmp->Release();
	}
	EckInline constexpr ID2D1Bitmap* GetBkgBitmap() const { return m_pBmpBkg; }

	// 【仅在渲染线程调用】
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

	EckInline CElem* ElemFromPoint(POINT pt, _Out_opt_ LRESULT* pResult = nullptr)
	{
		return CElem::ElemFromPoint(GetLastChildElem(), pt, pResult);
	}

	CElem* ElemSetFocus(CElem* pElem)
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
	EckInline constexpr CElem* ElemGetFocus() const { return m_pFocusElem; }

	CElem* ElemSetCapture(CElem* pElem)
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
	EckInline void ElemReleaseCapture()
	{
		ReleaseCapture();

		// WM_CAPTURECHANGED will process it:
		// m_pMouseCaptureElem->CallEvent(WM_CAPTURECHANGED, 0, nullptr);
		// m_pMouseCaptureElem = nullptr;
	}
	EckInline constexpr CElem* ElemGetCapture() const { return m_pMouseCaptureElem; }

	EckInline constexpr CElem* GetFirstChildElem() const { return m_pFirstChild; }
	EckInline constexpr CElem* GetLastChildElem() const { return m_pLastChild; }

	EckInline const DPIS& GetDs() const { return m_Ds; }

	EckInline constexpr int GetDpiValue() const { return m_iDpi; }

	EckInline constexpr int GetUserDpiValue() const { return m_iUserDpi; }

	EckInline constexpr int Phy2Log(int i) const { return i * 96 / m_iUserDpi; }
	EckInline constexpr float Phy2LogF(float i) const { return i * 96.f / m_iUserDpi; }
	EckInline constexpr int Log2Phy(int i) const { return i * m_iUserDpi / 96; }
	EckInline constexpr float Log2PhyF(float i) const { return i * m_iUserDpi / 96.f; }

	EckInline constexpr void Phy2Log(_Inout_ POINT& pt) const
	{
		pt.x = Phy2Log(pt.x);
		pt.y = Phy2Log(pt.y);
	}
	EckInline constexpr void Phy2Log(_Inout_ RECT& rc) const
	{
		rc.left = Phy2Log(rc.left);
		rc.top = Phy2Log(rc.top);
		rc.right = Phy2Log(rc.right);
		rc.bottom = Phy2Log(rc.bottom);
	}
	EckInline constexpr void Phy2Log(_Inout_ D2D1_POINT_2F& pt) const
	{
		pt.x = Phy2LogF(pt.x);
		pt.y = Phy2LogF(pt.y);
	}
	EckInline constexpr void Phy2Log(_Inout_ D2D1_RECT_F& rc) const
	{
		rc.left = Phy2LogF(rc.left);
		rc.top = Phy2LogF(rc.top);
		rc.right = Phy2LogF(rc.right);
		rc.bottom = Phy2LogF(rc.bottom);
	}
	EckInline constexpr void Log2Phy(_Inout_ POINT& pt) const
	{
		pt.x = Log2Phy(pt.x);
		pt.y = Log2Phy(pt.y);
	}
	EckInline constexpr void Log2Phy(_Inout_ RECT& rc) const
	{
		rc.left = Log2Phy(rc.left);
		rc.top = Log2Phy(rc.top);
		rc.right = Log2Phy(rc.right);
		rc.bottom = Log2Phy(rc.bottom);
	}
	EckInline constexpr void Log2Phy(_Inout_ D2D1_POINT_2F& pt) const
	{
		pt.x = Log2PhyF(pt.x);
		pt.y = Log2PhyF(pt.y);
	}
	EckInline constexpr void Log2Phy(_Inout_ D2D1_RECT_F& rc) const
	{
		rc.left = Log2PhyF(rc.left);
		rc.top = Log2PhyF(rc.top);
		rc.right = Log2PhyF(rc.right);
		rc.bottom = Log2PhyF(rc.bottom);
	}

	HRESULT EnableDragDrop(BOOL bEnable);

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

	EckInline constexpr CCriticalSection& GetCriticalSection() { return m_cs; }

	void IrUnion(const RECT& rc)
	{
		EckAssert(!IsRectEmpty(rc));
		UnionRect(m_rcInvalid, m_rcInvalid, rc);
	}

	EckInline void WakeRenderThread()
	{
		m_EvtRender.Signal();
	}

	EckInline void RegisterTimeLine(ITimeLine* pTl)
	{
		ECK_DUILOCKWND;
		pTl->AddRef();
		m_vTimeLine.emplace_back(pTl);
	}

	EckInline void UnregisterTimeLine(ITimeLine* pTl)
	{
		ECK_DUILOCKWND;
		const auto it = std::find(m_vTimeLine.begin(), m_vTimeLine.end(), pTl);
		(*it)->Release();
		m_vTimeLine.erase(it);
	}

	EckInline void Redraw()
	{
		ECK_DUILOCKWND;
		m_rcInvalid = { 0,0,m_cxClient,m_cyClient };
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
				(float)m_iUserDpi,
				(float)m_iUserDpi,
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

	void SetUserDpi(int iDpi)
	{
		ECK_DUILOCKWND;
		m_iUserDpi = iDpi;
		if (IsValid())
		{
			m_bUserDpiChanged = TRUE;
			BroadcastEvent(WM_DPICHANGED, iDpi, 0);
			WakeRenderThread();
		}
	}
};

inline void CElem::utcSwitchDefColorTheme(int idxTheme, WPARAM bDark)
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
	m_pParent = pParent;

	tcSetStyleWorker(dwStyle);

	ECK_DUILOCK;
#ifdef _DEBUG
	if (pParent)
		EckAssert(pParent->m_pWnd == pWnd);
#endif
	auto& pParentLastChild = (pParent ? pParent->m_pLastChild : pWnd->m_pLastChild);
	auto& pParentFirstChild = (pParent ? pParent->m_pFirstChild : pWnd->m_pFirstChild);
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
	tcSetRectWorker({ x,y,x + cx,y + cy });

	if (GetWnd()->IsElemUseDComp())
		utcReCreateDCompVisual();

	if (CallEvent(WM_CREATE, 0, (LPARAM)pData))
	{
		Destroy();
		return FALSE;
	}
	else
	{
		tcPostMoveSize(TRUE, TRUE, GetWholeRectInClient());
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
	utcDestroyChild(this);

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
	return TRUE;

	case EWM_COMP_POS:
	{
		if (GetParentElem())
			return GetParentElem()->CallEvent(uMsg, wParam, lParam);
	}
	return 0;
	case EWM_COMPOSITE:
	{
		if (GetParentElem())
			return GetParentElem()->CallEvent(uMsg, wParam, lParam);
		else
		{
			EckAssert(GetStyle() & DES_COMPOSITED);
			m_pDC->DrawBitmap(GetCacheBitmap(), (D2D1_RECT_F*)wParam,
				1.f, D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
		}
	}
	return 0;
	}
	return 0;
}

inline void CElem::SetZOrder(CElem* pElemAfter)
{
	ECK_DUILOCK;
	auto& pParentLastChild = (m_pParent ? m_pParent->m_pLastChild : m_pWnd->m_pLastChild);
	auto& pParentFirstChild = (m_pParent ? m_pParent->m_pFirstChild : m_pWnd->m_pFirstChild);

	if (pElemAfter == ECK_ELEMTOP)// == nullptr
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
		// pElemAfter一定不为nullptr
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
		return GetWnd()->OnElemEvent(this, ((DUINMHDR*)pnm)->uCode, 0, (LPARAM)pnm);
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
	tcIrpUnionContentExpandElemRect(
		(GetStyle() & DES_CONTENT_EXPAND) ? GetWnd()->GetLastChildElem() : this,
		rcReal);

	GetWnd()->IrUnion(rcReal);
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

EckInline void CElem::InitEasingCurve(CEasingCurve* pEc)
{
	pEc->SetParam((LPARAM)this);
	GetWnd()->RegisterTimeLine(pEc);
}

inline void CElem::utcReCreateDCompVisual()
{
	ECK_DUILOCK;
	EckAssert(GetWnd()->IsElemUseDComp());
	SafeRelease(m_pDcVisual);
	SafeRelease(m_pDcContent);
	SafeRelease(m_pDcSurface);
	const auto pDevice = GetWnd()->m_pDcDevice;
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
		GetWnd()->m_pDcVisual->AddVisual(m_pDcVisual, bInsertAbove, pRefVisual);
}

inline void CElem::tcReSizeDCompVisual()
{
	ECK_DUILOCK;
	m_pDcSurface->Release();
	GetWnd()->m_pDcDevice->CreateSurface(std::max(1, GetWidth()), std::max(1, GetHeight()),
		DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_ALPHA_MODE_PREMULTIPLIED, &m_pDcSurface);
	m_pDcVisual->SetContent(m_pDcSurface);
}

inline void CElem::tcPostMoveSize(BOOL bSize, BOOL bMove, const RECT& rcOld)
{
	if (bSize)
	{
		if (GetWnd()->IsElemUseDComp())
			tcReSizeDCompVisual();
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

EckInline CElem* CElem::SetCapture() { return GetWnd()->ElemSetCapture(this); }
EckInline void CElem::ReleaseCapture() { GetWnd()->ElemReleaseCapture(); }
EckInline void CElem::SetFocus() { GetWnd()->ElemSetFocus(this); }
EckInline constexpr CCriticalSection& CElem::GetCriticalSection() const { return GetWnd()->GetCriticalSection(); }
EckInline constexpr int CElem::Log2Phy(int i) const { return GetWnd()->Log2Phy(i); }
EckInline constexpr float CElem::Log2PhyF(float f) const { return GetWnd()->Log2PhyF(f); }
EckInline constexpr int CElem::Phy2Log(int i) const { return GetWnd()->Phy2Log(i); }
EckInline constexpr float CElem::Phy2LogF(float f) const { return GetWnd()->Phy2LogF(f); }
EckInline constexpr ID2D1Bitmap1* CElem::GetCacheBitmap() const { return GetWnd()->GetCacheBitmap(); }

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

		auto pElem = m_pWnd->ElemFromPoint(pt0);
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

		auto pElem = m_pWnd->ElemFromPoint(pt0);
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

		auto pElem = m_pWnd->ElemFromPoint(pt0);
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
	return S_FALSE;// Indicate do nothing.
}
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END