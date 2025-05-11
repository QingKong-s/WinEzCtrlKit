#pragma once
#include "CWnd.h"

#include "DuiDef.h"
#include "DuiStdTheme.h"
#include "DuiCompositor.h"

#include "GraphicsHelper.h"
#include "OleDragDropHelper.h"
#include "SystemHelper.h"
#include "CDwmWndPartMgr.h"

#include "EasingCurve.h"

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
constexpr inline auto DrawTextLayoutFlags =
D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT | D2D1_DRAW_TEXT_OPTIONS_NO_SNAP;

enum : int
{
	CxyMinScrollThumb = 16,
};

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
	ID2D1DeviceContext* m_pDC{};		// DC，未加引用
	union// DComp和手动混合只能选其一
	{
		void* PRIV_Dummy[4]{};
		struct
		{
			IDCompositionVisual* m_pDcVisual;	// DComp视觉对象
			IDCompositionSurface* m_pDcSurface;	// DComp表面
			IUnknown* m_pDcContent;				// DComp内容
		};
		struct
		{
			RECT m_rcCompositedInClient;// 缓存已混合的元素矩形，至少完全包含原始元素矩形相对客户区
			CCompositor* m_pCompositor;	// 混合操作
			union
			{
				ID2D1Bitmap1* m_pCompCachedBitmap;		// 内容渲染到的位图
				// 内容渲染到的缓存表面，设置DES_OWNER_COMP_CACHE时有效
				CCompCacheSurface* m_pCompCacheSurface;
			};
		};
	};
	//------位置，均为DPI虚拟化坐标------
	RECT m_rc{};						// 元素矩形，相对父元素
	D2D1_RECT_F m_rcf{};				// 元素矩形，相对父元素
	RECT m_rcInClient{};				// 元素矩形，相对客户区
	D2D1_RECT_F m_rcfInClient{};		// 元素矩形，相对客户区
	//------属性------
	CRefStrW m_rsText{};				// 标题
	INT_PTR m_iId{};					// 元素ID
	ITheme* m_pTheme{};		// 主题
	IDWriteTextFormat* m_pTextFormat{};	// 文本格式
	DWORD m_dwStyle{};					// 样式
	int m_cChildren{};					// 子元素数量
#ifdef _DEBUG
	RECT m_rcRealCompositedInClient{};	// 实际计算得到的混合矩形
#endif


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

	constexpr void tcIrpUnionContentExpandElemRect(_Inout_ RECT& rcInClient);

	constexpr void tcSrpCorrectChildrenRectInClient() const
	{
		auto pElem = GetFirstChildElem();
		while (pElem)
		{
			pElem->m_rcInClient = pElem->m_rc;
			pElem->m_rcfInClient = pElem->m_rcf;
			OffsetRect(pElem->m_rcInClient, m_rcInClient.left, m_rcInClient.top);
			OffsetRect(pElem->m_rcfInClient, m_rcfInClient.left, m_rcfInClient.top);
			pElem->tcSrpCorrectChildrenRectInClient();
			pElem = pElem->GetNextElem();
		}
	}

	constexpr void tcSetRectWorker(const RECT& rc)
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

	constexpr void tcSetStyleWorker(DWORD dwStyle);

	void utcReCreateDCompVisual();

	void tcReSizeDCompVisual();

	void tcPostMoveSize(BOOL bSize, BOOL bMove, const RECT& rcOld);
public:
	virtual BOOL Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, CElem* pParent,
		CDuiWnd* pWnd = nullptr, int iId = 0, PCVOID pData = nullptr)
	{
		return IntCreate(pszText, dwStyle, dwExStyle,
			x, y, cx, cy, pParent, pWnd, iId, pData);
	}

	void Destroy();

	// 事件处理函数，一般不直接调用此函数
	virtual LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual LRESULT OnNotify(DUINMHDR* pnm, BOOL& bProcessed) { return 0; }

	// 调用事件处理
	EckInline LRESULT CallEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (GetCompositor() && uMsg == WM_DPICHANGED)
			CompInvalidateCacheBitmap();
		SlotCtx Ctx{};
		const auto r = m_Sig.Emit2(Ctx, uMsg, wParam, lParam);
		if (Ctx.IsProcessed())
			return r;
		return OnEvent(uMsg, wParam, lParam);
	}

	/// <summary>
	/// 生成元素通知
	/// </summary>
	/// <param name="pnm">通知结构，第一个字段必须为DUINMHDR</param>
	/// <returns>处理方的返回值</returns>
	EckInline LRESULT GenElemNotify(void* pnm);

	/// <summary>
	/// 生成元素通知。
	/// 仅向父元素发送通知，若无父元素，返回0
	/// </summary>
	/// <param name="pnm">通知结构，第一个字段必须为DUINMHDR</param>
	/// <returns>处理方的返回值</returns>
	EckInline LRESULT GenElemNotifyParent(void* pnm)
	{
		BOOL bProcessed{};
		const auto lResult = OnNotify((DUINMHDR*)pnm, bProcessed);
		if (bProcessed)
			return lResult;
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
	EckInlineNdCe ID2D1Bitmap1* GetCacheBitmap() const;
#pragma region PosSize
	EckInline constexpr D2D1_RECT_F GetViewRectF() const
	{
		return { 0.f,0.f,m_rcf.right - m_rcf.left,m_rcf.bottom - m_rcf.top };
	}

	EckInline constexpr RECT GetViewRect() const
	{
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

	EckInline constexpr int GetWidth() const { return m_rc.right - m_rc.left; }
	EckInline constexpr int GetHeight() const { return m_rc.bottom - m_rc.top; }
	EckInline constexpr float GetWidthF() const { return m_rcf.right - m_rcf.left; }
	EckInline constexpr float GetHeightF() const { return m_rcf.bottom - m_rcf.top; }

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
		POINT pt0;
		while (pElem)
		{
			if (pElem->GetStyle() & DES_VISIBLE)
			{
				pt0 = pt;
				if (pElem->CompIsNeedCoordinateTransform())
				{
					pElem->ClientToElem(pt0);
					pElem->CompTransformCoordinate(pt0, TRUE);
					pElem->ElemToClient(pt0);
				}
				if (PtInRect(pElem->GetRectInClient(), pt0))
				{
					const auto pHit = pElem->ElemFromPoint(pt, pResult);
					if (pHit)
						return pHit;
					else if (LRESULT lResult;
						(lResult = pElem->CallEvent(WM_NCHITTEST,
							0, MAKELPARAM(pt0.x, pt0.y))) != HTTRANSPARENT)
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

	EckInline constexpr void ClientToElem(_Inout_ RECT& rc) const
	{
		OffsetRect(rc, -m_rcInClient.left, -m_rcInClient.top);
	}
	EckInline constexpr void ClientToElem(_Inout_ D2D1_RECT_F& rc) const
	{
		OffsetRect(rc, -m_rcfInClient.left, -m_rcfInClient.top);
	}
	EckInline constexpr void ClientToElem(_Inout_ POINT& pt) const
	{
		pt.x -= m_rcInClient.left;
		pt.y -= m_rcInClient.top;
	}
	EckInline constexpr void ClientToElem(_Inout_ D2D1_POINT_2F& pt) const
	{
		pt.x -= m_rcfInClient.left;
		pt.y -= m_rcfInClient.top;
	}
	EckInline constexpr void ElemToClient(_Inout_ RECT& rc) const
	{
		OffsetRect(rc, m_rcInClient.left, m_rcInClient.top);
	}
	EckInline constexpr void ElemToClient(_Inout_ D2D1_RECT_F& rc) const
	{
		OffsetRect(rc, m_rcfInClient.left, m_rcfInClient.top);
	}
	EckInline constexpr void ElemToClient(_Inout_ POINT& pt) const
	{
		pt.x += m_rcInClient.left;
		pt.y += m_rcInClient.top;
	}
	EckInline constexpr void ElemToClient(_Inout_ D2D1_POINT_2F& pt) const
	{
		pt.x += m_rcfInClient.left;
		pt.y += m_rcfInClient.top;
	}
#pragma endregion ElemTree
#pragma region OthersProp
	// 置标题【不能在渲染线程调用】
	EckInline void SetText(PCWSTR pszText, int cchText = -1)
	{
		if (cchText < 0)
			cchText = (int)TcsLen(pszText);
		ECK_DUILOCK;
		if (!CallEvent(WM_SETTEXT, cchText, (LPARAM)pszText))
			m_rsText.DupString(pszText, cchText);
	}
	// 取标题
	EckInlineNdCe const CRefStrW& GetText() const { return m_rsText; }

	// 置样式
	void SetStyle(DWORD dwStyle)
	{
		ECK_DUILOCK;
		const auto dwOldStyle = m_dwStyle;
		tcSetStyleWorker(dwStyle);
		CallEvent(WM_STYLECHANGED, dwOldStyle, m_dwStyle);
	}
	// 取样式
	EckInlineNdCe DWORD GetStyle() const { return m_dwStyle; }

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
	EckInlineNdCe BOOL IsVisible() const
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
			SetStyle(GetStyle() & ~DES_NO_REDRAW);
		else
			SetStyle(GetStyle() | DES_NO_REDRAW);
	}
	// 是否允许重绘
	EckInlineNdCe BOOL GetRedraw() const { return !(GetStyle() & DES_NO_REDRAW); }

	// 置文本格式【不能在渲染线程调用】
	EckInline void SetTextFormat(IDWriteTextFormat* pTf)
	{
		ECK_DUILOCK;
		std::swap(m_pTextFormat, pTf);
		if (m_pTextFormat)
			m_pTextFormat->AddRef();
		if (pTf)
			pTf->Release();
		CallEvent(WM_SETFONT, 0, 0);
	}
	// 取文本格式
	EckInlineNdCe IDWriteTextFormat* GetTextFormat() const { return m_pTextFormat; }

	// 置元素ID【不能在渲染线程调用】
	EckInline constexpr void SetID(INT_PTR iId) { m_iId = iId; }
	// 取元素ID
	EckInlineNdCe INT_PTR GetID() const { return m_iId; }

	// 置主题【不能在渲染线程调用】
	EckInline void SetTheme(ITheme* pTheme)
	{
		ECK_DUILOCK;
		std::swap(m_pTheme, pTheme);
		if (m_pTheme)
			m_pTheme->AddRef();
		if (pTheme)
			pTheme->Release();
		CallEvent(WM_THEMECHANGED, 0, 0);
	}
	// 取主题
	EckInlineNdCe auto GetTheme() const { return m_pTheme; }

	// 置混合操作
	void SetCompositor(CCompositor* pCompositor)
	{
		ECK_DUILOCK;
		if (m_pCompositor == pCompositor)
			return;
		CompMarkDirty();
		std::swap(m_pCompositor, pCompositor);
		if (m_pCompositor)
			m_pCompositor->AddRef();
		if (pCompositor)
			pCompositor->Release();
		CompInvalidateCacheBitmap();
		if (m_pCompositor)
			CompReCalcCompositedRect();
	}
	// 取混合操作
	EckInlineNdCe CCompositor* GetCompositor() const { return m_pCompositor; }
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
	void BeginPaint(_Out_ ELEMPAINTSTRU& eps, WPARAM wParam, LPARAM lParam, UINT uFlags = 0u);

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
	// 释放鼠标【不能在渲染线程调用】
	EckInline void ReleaseCapture();

	// 置焦点【不能在渲染线程调用】
	EckInline void SetFocus();

	// 安装定时器【不能在渲染线程调用】
	EckInline BOOL SetTimer(UINT_PTR uId, UINT uElapse);
	// 销毁定时器【不能在渲染线程调用】
	EckInline BOOL KillTimer(UINT_PTR uId);

	EckInlineNdCe BOOL IsValid() const { return !!GetWnd(); }
#pragma endregion ElemFunc
#pragma region Composite
	// 取完全包围元素的矩形，相对客户区
	EckInlineNdCe const RECT& GetWholeRectInClient() const
	{
		return GetCompositor() && !GetCompositor()->IsInPlace() ?
			m_rcCompositedInClient : GetRectInClient();
	}

	EckInlineNdCe CElem* CompGetFirstCompositedAncestor() const
	{
		if (GetStyle() & DES_PARENT_COMP)
		{
			auto pParent = GetParentElem();
			while (pParent && !pParent->GetCompositor())
				pParent = pParent->GetParentElem();
			return pParent;
		}
		return nullptr;
	}

	EckInline void CompReCalcCompositedRect()
	{
		ECK_DUILOCK;
		if (!GetCompositor()->IsInPlace())
		{
			GetCompositor()->CalcCompositedRect(this, m_rcCompositedInClient, TRUE);
#ifdef _DEBUG
			m_rcRealCompositedInClient = m_rcCompositedInClient;
#endif
			UnionRect(m_rcCompositedInClient, m_rcCompositedInClient, m_rcInClient);
		}
	}

	EckInlineNdCe BOOL CompIsNeedCoordinateTransform() const
	{
		return GetCompositor() || (GetStyle() & DES_PARENT_COMP);
	}

	void CompTransformCoordinate(_Inout_ POINT& pt, BOOL bNormalToComposited)
	{
		if (GetCompositor() && !(GetStyle() & DES_PARENT_COMP))// OPTIMIZATION
		{
			if (bNormalToComposited)
				GetCompositor()->PtNormalToComposited(this, pt);
			else
				GetCompositor()->PtCompositedToNormal(this, pt);
			return;
		}
		CElem* pTrans[16]{};
		auto pp = pTrans;
		auto pElem{ this };
		do
		{
			const auto pCompositor = pElem->GetCompositor();
			if (pCompositor)
			{
				*pp++ = pElem;
				if (!(pElem->GetStyle() & DES_PARENT_COMP))
					break;
				pElem = pElem->GetParentElem();
			}
			else if (pElem->GetStyle() & DES_PARENT_COMP)
			{
				const auto pAncestor = pElem->CompGetFirstCompositedAncestor();
				if (pAncestor)
				{
					*pp++ = pAncestor;
					pElem = pAncestor->GetParentElem();
				}
				else
					break;
			}
			else
				break;
		} while (pElem);

		ElemToClient(pt);
		--pp;
		for (; pp >= pTrans; --pp)
		{
			pElem = *pp;
			const auto pCompositor = pElem->GetCompositor();
			if (pCompositor)
			{
				pElem->ClientToElem(pt);
				if (bNormalToComposited)
					pCompositor->PtNormalToComposited(pElem, pt);
				else
					pCompositor->PtCompositedToNormal(pElem, pt);
				pElem->ElemToClient(pt);
			}
			else if (pElem->GetStyle() & DES_PARENT_COMP)
			{
				const auto pAncestor = pElem->CompGetFirstCompositedAncestor();
				pAncestor->ClientToElem(pt);
				pAncestor->CompTransformCoordinate(pt, bNormalToComposited);
				pAncestor->ElemToClient(pt);
			}
		}
		ClientToElem(pt);
	}

	inline HRESULT CompUpdateCacheBitmap(int cx, int cy);

	EckInline void CompInvalidateCacheBitmap()
	{
		SafeRelease(m_pCompCachedBitmap);
	}

	EckInlineNdCe void CompMarkDirty()
	{
		if (GetCompositor())
			m_dwStyle |= DESP_COMP_CONTENT_INVALID;
	}
#pragma endregion Composite
};

class CDuiWnd :public CWnd
{
	friend class CElem;
	friend class CDuiDropTarget;
private:
	struct TIMER
	{
		CElem* pElem;
		UINT_PTR uId;
	};

	//------元素树------
	CElem* m_pFirstChild{};	// 第一个子元素
	CElem* m_pLastChild{};	// 最后一个子元素
	std::vector<CElem*> m_vContentExpandElem{};	// 内容扩展元素列表
	std::vector<RECT> m_vRcContentExpandElem{};	// 合并后的内容扩展元素的矩形，两两不相交
	//------UI系统------
	CElem* m_pFocusElem{};	// 当前焦点元素
	CElem* m_pHoverElem{};	// 当前鼠标悬停元素，WM_MOUSELEAVE使用
	CElem* m_pCurrNcHitTestElem{};	// 当前非客户区命中元素
	CElem* m_pMouseCaptureElem{};	// 当前鼠标捕获元素

	CCriticalSection m_cs{};// 渲染线程同步临界区
	CEvent m_EvtRender{};	// 渲染线程事件对象
	HANDLE m_hthRender{};	// 渲染线程句柄

	std::vector<ITimeLine*> m_vTimeLine{};	// 时间线
	std::vector<TIMER> m_vTimer{};			// 需要定时器的元素
	//------拖放------
	CElem* m_pDragDropElem{};		// 当前拖放元素
	IDataObject* m_pDataObj{};		// 当前拖放的数据对象
	CDuiDropTarget* m_pDropTarget{};// 拖放目标
	//------图形------
	RECT m_rcInvalid{};				// 无效矩形

	ID2D1Bitmap* m_pBmpBkg{};		// 背景位图
	ID2D1SolidColorBrush* m_pBrBkg{};		// 背景画刷

	CEzD2D m_D2D{};
	union
	{
		void* PRIV_Dummy[4];
		// DComp呈现使用
		struct
		{
			IDCompositionDevice* m_pDcDevice;	// DComp设备
			IDCompositionTarget* m_pDcTarget;	// DComp目标，DCompVisual不使用
			IDCompositionVisual* m_pDcVisual;	// 根视觉对象
			IDCompositionSurface* m_pDcSurface;	// 根视觉对象的内容，DCompVisual下仅此字段是由自身创建的
		};
		// 窗口渲染目标呈现使用
		ID2D1HwndRenderTarget* m_pRtHwnd;		// 窗口渲染目标，仅当呈现模式为窗口渲染目标时有效
	};

	ID2D1Bitmap1* m_pBmpCache{};	// 缓存位图
	int m_cxCache{};				// 缓存位图宽度
	int m_cyCache{};				// 缓存位图高度

	std::vector<CThemeRealization*> m_vTheme{};		// 主题列表

	ID2D1Effect* m_pFxBlur{};		// 缓存模糊效果
	ID2D1Effect* m_pFxCrop{};		// 缓存裁剪效果
	//------其他------
	int m_cxClient{};		// 客户区宽度
	int m_cyClient{};		// 客户区高度
	int m_cxClientLog{};
	int m_cyClientLog{};
	int m_cChildren{};		// 子元素数量，UIAccessible使用
	float m_fBlurDeviation = 15.f;			// 高斯模糊标准差

	PresentMode m_ePresentMode{ PresentMode::FlipSwapChain };	// 呈现模式

	BITBOOL m_bMouseCaptured : 1 = FALSE;	// 鼠标是否被捕获
	BITBOOL m_bTransparent : 1 = FALSE;		// 窗口是透明的
	BITBOOL m_bRenderThreadShouldExit : 1 = FALSE;	// 渲染线程应当退出
	BITBOOL m_bSizeChanged : 1 = FALSE;				// 渲染线程应当重设图面大小
	BITBOOL m_bUserDpiChanged : 1 = FALSE;			// 渲染线程应当重设DPI
	BITBOOL m_bFullUpdate : 1 = TRUE;				// 当前是否需要完全重绘
	BITBOOL m_bBlurUseLayer : 1 = FALSE;			// 模糊是否使用图层
#ifdef _DEBUG
	BITBOOL m_bDrawDirtyRect : 1 = FALSE;			// 是否绘制脏矩形
#endif

	BYTE m_eAlphaMode{ D2D1_ALPHA_MODE_IGNORE };		// 缓存D2D透明模式
	BYTE m_eDxgiAlphaMode{ DXGI_ALPHA_MODE_IGNORE };	// 缓存DXGI透明模式

	USHORT m_iUserDpi = USER_DEFAULT_SCREEN_DPI;
	USHORT m_iDpi = USER_DEFAULT_SCREEN_DPI;


	void ElemDestroying(CElem* pElem)
	{
		if (m_pFocusElem == pElem)
			m_pFocusElem = nullptr;
		if (m_pCurrNcHitTestElem == pElem)
			m_pCurrNcHitTestElem = nullptr;
		if (m_pMouseCaptureElem == pElem)
			ElemReleaseCapture();
		if (m_pHoverElem == pElem)
			m_pHoverElem = nullptr;
		for (size_t i{}; i < m_vTimer.size();)
		{
			if (m_vTimer[i].pElem == pElem)
			{
				KillTimer(HWnd, m_vTimer[i].uId);
				m_vTimer.erase(m_vTimer.begin() + i);
			}
			else
				++i;
		}
	}

	/// <summary>
	/// 重画元素树
	/// </summary>
	/// <param name="pElem">起始元素</param>
	/// <param name="rc">重画区域，逻辑坐标</param>
	/// <param name="ox">用于DComp图面的X偏移</param>
	/// <param name="oy">用于DComp图面的Y偏移</param>
	/// <param name="oxComp">用于手动混合元素子级的X偏移</param>
	/// <param name="oyComp">用于手动混合元素子级的Y偏移</param>
	void RedrawElem(CElem* pElem, const RECT& rc, float ox, float oy,
		float oxComp = 0.f, float oyComp = 0.f)
	{
		const auto pDC = GetDeviceContext();
		RECT rcClip;// 相对客户区
		ID2D1Image* pOldTarget{};
		COMP_RENDER_INFO cri;
		IDXGISurface1* pDxgiSurface{};
		ID2D1Bitmap1* pBitmap{};
		Priv::PAINT_EXTRA Extra{ ox,oy };
		while (pElem)
		{
			const auto& rcElem = pElem->GetRectInClient();
			const auto dwStyle = pElem->GetStyle();
			if (!(dwStyle & DES_VISIBLE) ||
				(dwStyle & (DES_NO_REDRAW | DES_EXTERNAL_CONTENT)) ||
				IsRectEmpty(rcElem))
				goto NextElem;
			if ((pElem->GetCompositor() && !(dwStyle & DES_COMP_NO_REDIRECTION) &&
				IsRectsIntersect(pElem->GetWholeRectInClient(), rc)))
				rcClip = rcElem;
			else if (!IntersectRect(rcClip, rcElem, rc))
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
			}
			else if (pElem->GetCompositor())// 手动合成
			{
				cri.pElem = pElem;
				cri.pDC = GetDeviceContext();
				cri.rcDst = pElem->GetViewRectF();
				if (dwStyle & DES_COMP_NO_REDIRECTION)
				{
					pDC->SetTransform(D2D1::Matrix3x2F::Translation(
						pElem->GetRectInClientF().left + ox + oxComp,
						pElem->GetRectInClientF().top + oy + oyComp));
					pElem->GetCompositor()->PreRender(cri);
				}
				else
				{
					if (!(dwStyle & DESP_COMP_CONTENT_INVALID) &&
						pElem->m_pCompCachedBitmap)
						goto SkipCompReRender;
					pDC->Flush();
					pDC->GetTarget(&pOldTarget);
					pElem->CompUpdateCacheBitmap(rcElem.right - rcElem.left,
						rcElem.bottom - rcElem.top);
					if (pElem->GetStyle() & DES_OWNER_COMP_CACHE)
					{
						pDC->SetTarget(pElem->m_pCompCacheSurface->GetBitmap());
						const auto& rcValid = pElem->m_pCompCacheSurface->GetValidRect();
						pDC->SetTransform(D2D1::Matrix3x2F::Translation(
							rcValid.left, rcValid.top));
					}
					else
					{
						pDC->SetTarget(pElem->m_pCompCachedBitmap);
						pDC->SetTransform(D2D1::Matrix3x2F::Identity());
					}
				}
			}
			else// 直接渲染
			{
				pDC->SetTransform(D2D1::Matrix3x2F::Translation(
					pElem->GetRectInClientF().left + ox + oxComp,
					pElem->GetRectInClientF().top + oy + oyComp));
			}
			pElem->CallEvent(WM_PAINT, (WPARAM)&Extra, (LPARAM)&rcClip);
			if (IsElemUseDComp())
			{
				pDC->EndDraw();
				pDC->SetTarget(nullptr);
				pBitmap->Release();
				pDxgiSurface->Release();
				pElem->m_pDcSurface->EndDraw();
				RedrawElem(pElem->GetFirstChildElem(), rcClip, 0.f, 0.f);
			}
			else if (pElem->GetCompositor())
			{
				if (dwStyle & DES_COMP_NO_REDIRECTION)
					RedrawElem(pElem->GetFirstChildElem(), rcClip, ox, oy);
				else
				{
					RedrawElem(pElem->GetFirstChildElem(), rcClip, 0.f, 0.f,
						oxComp - rcElem.left, oyComp - rcElem.top);
					pDC->Flush();
					pDC->SetTarget(pOldTarget);
					pOldTarget->Release();
				}
			SkipCompReRender:;
				pDC->SetTransform(D2D1::Matrix3x2F::Translation(
					pElem->GetRectInClientF().left + ox + oxComp,
					pElem->GetRectInClientF().top + oy + oyComp));
				if (pElem->GetStyle() & DES_OWNER_COMP_CACHE)
				{
					cri.pBitmap = pElem->m_pCompCacheSurface->GetBitmap();
					cri.rcSrc = pElem->m_pCompCacheSurface->GetValidRect();
				}
				else
				{
					cri.pBitmap = pElem->m_pCompCachedBitmap;
					cri.rcSrc = pElem->GetViewRectF();
				}
				auto rcRealClip{ pElem->GetWholeRectInClient() };
				IntersectRect(rcRealClip, rcRealClip, rc);
				pElem->ClientToElem(rcRealClip);
				pDC->PushAxisAlignedClip(MakeD2DRcF(rcRealClip), D2D1_ANTIALIAS_MODE_ALIASED);
				pElem->GetCompositor()->PostRender(cri);
				pDC->PopAxisAlignedClip();
#ifdef _DEBUG
				ID2D1SolidColorBrush* pBr{};
				pDC->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Aqua), &pBr);
				if (pBr)
				{
					auto rcComp{ MakeD2DRcF(pElem->m_rcCompositedInClient) };
					pElem->ClientToElem(rcComp);
					pDC->DrawRectangle(rcComp, pBr);

					pBr->SetColor(D2D1::ColorF(D2D1::ColorF::Green));
					rcComp = MakeD2DRcF(pElem->m_rcRealCompositedInClient);
					pElem->ClientToElem(rcComp);
					pDC->DrawRectangle(rcComp, pBr);

					pBr->SetColor(D2D1::ColorF(D2D1::ColorF::Orange));
					pDC->DrawRectangle(pElem->GetViewRectF(), pBr);
					pBr->Release();
				}
#endif
			}
			else
				RedrawElem(pElem->GetFirstChildElem(), rcClip, ox, oy);
		NextElem:
			pElem = pElem->GetNextElem();
		}
	}

	void RedrawDui_DComp(const RECT& rc, BOOL bFullUpdate = FALSE, RECT* prcPhy = nullptr)
	{
		EckAssert(GetPresentMode() == PresentMode::DCompositionSurface ||
			GetPresentMode() == PresentMode::DCompositionVisual);
		const auto pDC = GetDeviceContext();
		RENDER_EVENT e;
		ComPtr<IDXGISurface1> pDxgiSurface;
		auto rcPhyF = MakeD2DRcF(rc);
		Log2Phy(rcPhyF);
		RECT rcPhy, rcNewPhy;
		CeilRect(rcPhyF, rcPhy);
		// 准备
		e.PreRender.prcDirtyPhy = bFullUpdate ? nullptr : &rcPhy;
		m_pDcSurface->BeginDraw(e.PreRender.prcDirtyPhy,
			IID_PPV_ARGS(&pDxgiSurface), &e.PreRender.ptOffsetPhy);
		e.PreRender.pSfcFinalDst = pDxgiSurface.Get();
		e.PreRender.prcNewDirtyPhy = &rcNewPhy;
		e.PreRender.pSfcNewDst = nullptr;
		const auto rer = OnRenderEvent(RE_PRERENDER, e);
		if (rer == RER_NONE)
		{
			e.PreRender.ptOffsetPhy.x -= rcPhy.left;
			e.PreRender.ptOffsetPhy.y -= rcPhy.top;
		}
		// 准备D2D渲染目标
		const D2D1_BITMAP_PROPERTIES1 D2dBmpProp
		{
			{ DXGI_FORMAT_B8G8R8A8_UNORM,(D2D1_ALPHA_MODE)m_eAlphaMode },
			(float)m_iUserDpi,
			(float)m_iUserDpi,
			D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
			nullptr
		};
		ComPtr<ID2D1Bitmap1> pBitmap;
		pDC->CreateBitmapFromDxgiSurface(
			rer == RER_NONE ? pDxgiSurface.Get() : e.PreRender.pSfcNewDst,
			&D2dBmpProp, &pBitmap);
		pDC->SetTarget(pBitmap.Get());
		pDC->BeginDraw();
		pDC->SetTransform(D2D1::Matrix3x2F::Identity());

		D2D1_RECT_F rcF, rcFinalF;
		rcFinalF = MakeD2DRcF(rcPhy);
		OffsetRect(rcFinalF, (float)e.PreRender.ptOffsetPhy.x,
			(float)e.PreRender.ptOffsetPhy.y);
		Phy2Log(rcFinalF);
		if (rer == RER_NONE)
			rcF = rcFinalF;
		else
		{
			rcF = MakeD2DRcF(rcNewPhy);
			Phy2Log(rcF);
		}
		// 画背景
		if (bFullUpdate)
		{
			++rcF.right;
			++rcF.bottom;
		}
		if (m_bTransparent)
		{
			pDC->PushAxisAlignedClip(rcF, D2D1_ANTIALIAS_MODE_ALIASED);
			pDC->Clear({});
			pDC->PopAxisAlignedClip();
		}
		FillBackground(rcF);
		if (bFullUpdate)
		{
			--rcF.right;
			--rcF.bottom;
		}
		// 画元素树
		const D2D1_POINT_2F ptLogOffsetFinalF
		{
			Phy2LogF((float)e.PreRender.ptOffsetPhy.x),
			Phy2LogF((float)e.PreRender.ptOffsetPhy.y)
		};
		RECT rcReal;
		OffsetRect(rcFinalF, -ptLogOffsetFinalF.x, -ptLogOffsetFinalF.y);
		CeilRect(rcFinalF, rcReal);
		if (rer == RER_NONE)
			RedrawElem(GetFirstChildElem(), rcReal,
				ptLogOffsetFinalF.x, ptLogOffsetFinalF.y);
		else
		{
			const D2D1_POINT_2F ptOrgLogF
			{
				Phy2LogF((float)rcNewPhy.left),
				Phy2LogF((float)rcNewPhy.top)
			};
			RedrawElem(GetFirstChildElem(), rcReal,
				ptOrgLogF.x - rc.left, ptOrgLogF.y - rc.top);
		}

#ifdef _DEBUG
		if (m_bDrawDirtyRect)
		{
			ComPtr<ID2D1SolidColorBrush> pBr;
			InflateRect(rcF, -1.f, -1.f);
			pDC->SetTransform(D2D1::Matrix3x2F::Identity());
			ARGB Cr = Rand(0x255) | Rand(0x255) << 8 | Rand(0x255) << 16 | 0xFF000000;
			pDC->CreateSolidColorBrush(D2D1::ColorF(Cr), &pBr);
			pDC->DrawRectangle(rcF, pBr.Get(), 2.f);
		}
#endif // _DEBUG
		pDC->EndDraw();
		if (rer == RER_REDIRECTION)
			OnRenderEvent(RE_POSTRENDER, e);
		m_pDcSurface->EndDraw();
		pDC->SetTarget(nullptr);

		if (m_ePresentMode == PresentMode::DCompositionVisual)
			OnRenderEvent(RE_COMMIT, e);
		else
			m_pDcDevice->Commit();
	}

	/// <summary>
	/// 重画DUI
	/// </summary>
	/// <param name="rc">重画区域，逻辑坐标</param>
	/// <param name="bFullUpdate">是否全更新，仅用于DComp</param>
	void RedrawDui(const RECT& rc, BOOL bFullUpdate = FALSE, RECT* prcPhy = nullptr)
	{
		const auto pDC = GetDeviceContext();
		switch (m_ePresentMode)
		{
		case PresentMode::BitBltSwapChain:
		case PresentMode::FlipSwapChain:
		case PresentMode::WindowRenderTarget:
		{
			pDC->BeginDraw();
			pDC->SetTransform(D2D1::Matrix3x2F::Identity());
			auto rcF = MakeD2DRcF(rc);
			Log2Phy(rcF);
			CeilRect(rcF);
			if (prcPhy)
				*prcPhy = MakeRect(rcF);
			Phy2Log(rcF);
			CeilRect(rcF);

			if (bFullUpdate) [[unlikely]]
			{
				++rcF.right;
				++rcF.bottom;
			}
			if (m_bTransparent)
			{
				pDC->PushAxisAlignedClip(rcF, D2D1_ANTIALIAS_MODE_ALIASED);
				pDC->Clear({});
				pDC->PopAxisAlignedClip();
			}
			FillBackground(rcF);
			if (bFullUpdate) [[unlikely]]
			{
				--rcF.right;
				--rcF.bottom;
			}

			RedrawElem(GetFirstChildElem(), MakeRect(rcF), 0.f, 0.f);
			pDC->EndDraw();
		}
		return;
		case PresentMode::DCompositionSurface:
		case PresentMode::DCompositionVisual:
			RedrawDui_DComp(rc, bFullUpdate, prcPhy);
			return;
		case PresentMode::AllDComp:
		{

		}
		return;
		}
		ECK_UNREACHABLE;
	}

	void RenderThread()
	{
		constexpr int c_iMinGap = 14;
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
					GetDeviceContext()->SetDpi((float)m_iUserDpi, (float)m_iUserDpi);
					m_bUserDpiChanged = FALSE;
					if (!m_bSizeChanged)
						goto SkipReSize;
				}
				m_bSizeChanged = FALSE;
				switch (m_ePresentMode)
				{
				case PresentMode::BitBltSwapChain:
					m_D2D.ReSize(1, m_cxClient, m_cyClient, 0,
						(D2D1_ALPHA_MODE)m_eAlphaMode, D2dBmpOpt, (float)m_iUserDpi);
					break;
				case PresentMode::FlipSwapChain:
					m_D2D.ReSize(2, m_cxClient, m_cyClient, 0,
						(D2D1_ALPHA_MODE)m_eAlphaMode, D2dBmpOpt, (float)m_iUserDpi);
					break;
				case PresentMode::DCompositionSurface:
				case PresentMode::AllDComp:
				case PresentMode::DCompositionVisual:
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
				if (m_cxClient && m_cyClient) [[likely]]
				{
					m_rcInvalid = rcClient;
					RedrawDui(rcClient, TRUE);
					m_cs.Leave();
					if (m_ePresentMode == PresentMode::BitBltSwapChain ||
						m_ePresentMode == PresentMode::FlipSwapChain)
						m_D2D.GetSwapChain()->Present(0, 0);
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
				if (!IsRectEmpty(m_rcInvalid)) [[likely]]
				{
					RECT rc;
					IntersectRect(rc, m_rcInvalid, rcClient);
					if (IsRectEmpty(rc))
						goto NoRedraw;
					RedrawDui(rc, m_bFullUpdate,
						m_ePresentMode == PresentMode::FlipSwapChain ? &rc : nullptr);
					if (m_bFullUpdate)
						m_bFullUpdate = FALSE;
					m_rcInvalid = {};
					m_cs.Leave();
					// 呈现

					switch (m_ePresentMode)
					{
					case PresentMode::BitBltSwapChain:
					case PresentMode::FlipSwapChain:
					{
						const DXGI_PRESENT_PARAMETERS pp
						{
							.DirtyRectsCount = 1,
							.pDirtyRects = &rc,
						};
						m_D2D.GetSwapChain()->Present1(0, 0, &pp);
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
			m_ePresentMode == PresentMode::AllDComp ||
			m_ePresentMode == PresentMode::DCompositionVisual)
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
				if (pElem->CompIsNeedCoordinateTransform())
				{
					POINT pt0{ pt };
					pElem->ClientToElem(pt0);
					pElem->CompTransformCoordinate(pt0, TRUE);
					pElem->ElemToClient(pt0);
					pElem->CallEvent(uMsg, wParam, MAKELPARAM(pt0.x, pt0.y));
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
			m_cxClientLog = (int)ceilf(Phy2LogF((float)m_cxClient));
			m_cyClientLog = (int)ceilf(Phy2LogF((float)m_cyClient));
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

		case WM_TIMER:
		{
			for (const auto e : m_vTimer)
			{
				if (e.uId == wParam)
				{
					e.pElem->CallEvent(WM_TIMER, wParam, 0);
					return 0;
				}
			}
		}
		return 0;

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
					if (m_pHoverElem && m_pHoverElem != m_pMouseCaptureElem)
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

				rc.right = std::max(rc.right, 8L);
				rc.bottom = std::max(rc.bottom, 8L);
				switch (m_ePresentMode)
				{
				case PresentMode::BitBltSwapChain:
				{
					auto Param = EZD2D_PARAM::MakeBitblt(hWnd, g_pDxgiFactory, g_pDxgiDevice,
						g_pD2dDevice, rc.right, rc.bottom, (float)m_iUserDpi);
					Param.uBmpAlphaMode = (D2D1_ALPHA_MODE)m_eAlphaMode;
					m_D2D.Create(Param);
				}
				break;
				case PresentMode::FlipSwapChain:
				{
					auto Param = EZD2D_PARAM::MakeFlip(hWnd, g_pDxgiFactory, g_pDxgiDevice,
						g_pD2dDevice, rc.right, rc.bottom, (float)m_iUserDpi);
					Param.uBmpAlphaMode = (D2D1_ALPHA_MODE)m_eAlphaMode;
					m_D2D.Create(Param);
				}
				break;
				case PresentMode::DCompositionSurface:
				case PresentMode::AllDComp:
				{
					g_pD2dDevice->CreateDeviceContext(
						EZD2D_PARAM::MakeFlip(0, nullptr, nullptr, nullptr, 0, 0).uDcOptions,
						&m_D2D.m_pDC);

					DCompositionCreateDevice3(g_pDxgiDevice, IID_PPV_ARGS(&m_pDcDevice));
					m_pDcDevice->CreateTargetForHwnd(hWnd, TRUE, &m_pDcTarget);
					m_pDcDevice->CreateVisual(&m_pDcVisual);
					m_pDcDevice->CreateSurface(rc.right, rc.bottom,
						DXGI_FORMAT_B8G8R8A8_UNORM,
						(DXGI_ALPHA_MODE)m_eDxgiAlphaMode,
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
					RtProp.pixelFormat = { DXGI_FORMAT_B8G8R8A8_UNORM,(D2D1_ALPHA_MODE)m_eAlphaMode };
					RtProp.dpiX = RtProp.dpiY = (float)m_iUserDpi;
					RtProp.usage = D2D1_RENDER_TARGET_USAGE_NONE;
					RtProp.minLevel = D2D1_FEATURE_LEVEL_DEFAULT;
					D2D1_HWND_RENDER_TARGET_PROPERTIES HwRtProp;
					HwRtProp.hwnd = hWnd;
					HwRtProp.pixelSize = D2D1::SizeU(rc.right, rc.bottom);
					HwRtProp.presentOptions = D2D1_PRESENT_OPTIONS_NONE;
					g_pD2dFactory->CreateHwndRenderTarget(RtProp, HwRtProp, &m_pRtHwnd);
					const auto hr = m_pRtHwnd->QueryInterface(&m_D2D.m_pDC);
					EckAssert(SUCCEEDED(hr));
				}
				break;
				case PresentMode::DCompositionVisual:
				{
					g_pD2dDevice->CreateDeviceContext(
						EZD2D_PARAM::MakeFlip(0, nullptr, nullptr, nullptr, 0, 0).uDcOptions,
						&m_D2D.m_pDC);

					m_pDcDevice->CreateSurface(rc.right, rc.bottom,
						DXGI_FORMAT_B8G8R8A8_UNORM,
						(DXGI_ALPHA_MODE)m_eDxgiAlphaMode,
						&m_pDcSurface);
					m_pDcVisual->SetContent(m_pDcSurface);
					m_pDcVisual->SetOffsetX(0.f);
					m_pDcVisual->SetOffsetY(0.f);
					RENDER_EVENT e;
					OnRenderEvent(RE_COMMIT, e);
				}
				break;
				default:
					ECK_UNREACHABLE;
				}

				const auto pDC = GetDeviceContext();

				const auto pTheme = new CTheme{};
				pTheme->Open(&StdThemeData, sizeof(StdThemeData), FALSE);

				auto pPal = new CThemePalette{ Palette_StdLight,ARRAYSIZE(Palette_StdLight) };
				pTheme->AddPalette(pPal);
				pTheme->SetPalette(pPal);
				pPal->Release();

				pPal = new CThemePalette{ Palette_StdDark,ARRAYSIZE(Palette_StdDark) };
				pTheme->AddPalette(pPal);
				pPal->Release();

				const auto pTr = new CThemeRealization{ pDC, pTheme };

				m_vTheme.emplace_back(pTr);

				pTheme->Release();

				pDC->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
				if (m_ePresentMode != PresentMode::WindowRenderTarget)
					pDC->SetDpi((float)m_iUserDpi, (float)m_iUserDpi);

				if (!m_bTransparent)
				{
					const auto crBkg = ColorrefToD2dColorF(GetThreadCtx()->crDefBkg);
					pDC->CreateSolidColorBrush(crBkg, &m_pBrBkg);
				}
				m_cxClient = rc.right;
				m_cyClient = rc.bottom;
				m_cxClientLog = (int)ceilf(Phy2LogF((float)m_cxClient));
				m_cyClientLog = (int)ceilf(Phy2LogF((float)m_cyClient));

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
			MsgOnDpiChanged(hWnd, lParam);
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
			WaitObject(CWaitableObject{ m_hthRender });// 等待渲染线程退出
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
			m_D2D.Destroy();
			SafeReleaseAssert0(m_pBmpCache);
			SafeReleaseAssert0(m_pBmpBkg);
			SafeReleaseAssert0(m_pBrBkg);
			switch (m_ePresentMode)
			{
			case PresentMode::WindowRenderTarget:
				SafeReleaseAssert0(m_pRtHwnd);
				break;
			case PresentMode::AllDComp:
			case PresentMode::DCompositionSurface:
				SafeReleaseAssert0(m_pDcTarget);
				SafeReleaseAssert0(m_pDcSurface);
				SafeReleaseAssert0(m_pDcVisual);
				m_pDcDevice->Commit();// 冲洗所有清理操作
				m_pDcDevice->WaitForCommitCompletion();
				SafeReleaseAssert0(m_pDcDevice);
				break;
			case PresentMode::DCompositionVisual:
				SafeRelease(m_pDcSurface);
				SafeRelease(m_pDcVisual);
				m_pDcDevice->Commit();// 冲洗所有清理操作
				m_pDcDevice->WaitForCommitCompletion();
				SafeRelease(m_pDcDevice);// 外部对象可能未清理完成
				break;
			}

			SafeReleaseAssert0(m_pFxBlur);
			SafeReleaseAssert0(m_pFxCrop);
			// 销毁其他接口
			SafeReleaseAssert0(m_pDataObj);
			SafeReleaseAssert0(m_pDropTarget);

			for (auto p : m_vTheme)
				p->Release();
			m_vTheme.clear();

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

	virtual LRESULT OnRenderEvent(UINT uMsg, RENDER_EVENT& e) { return 0; }

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
		if (m_pBrBkg)
			GetDeviceContext()->FillRectangle(rc, m_pBrBkg);
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
			GetDeviceContext()->DrawBitmap(m_pBmpBkg, &rc,
				1.f, D2D1_INTERPOLATION_MODE_LINEAR, &rcSource);
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

	BOOL ElemSetTimer(CElem* pElem, UINT_PTR uId, UINT uElapse)
	{
		if (!SetTimer(HWnd, uId, uElapse, nullptr))
			return FALSE;
		for (auto& e : m_vTimer)
		{
			if (e.pElem == pElem && e.uId == uId)
				return TRUE;
		}
		m_vTimer.emplace_back(pElem, uId);
		return TRUE;
	}

	BOOL ElemKillTimer(CElem* pElem, UINT_PTR uId)
	{
		for (auto it = m_vTimer.begin(); it != m_vTimer.end(); ++it)
		{
			if (it->pElem == pElem && it->uId == uId)
			{
				KillTimer(HWnd, uId);
				m_vTimer.erase(it);
				return TRUE;
			}
		}
		return FALSE;
	}

	EckInline constexpr CElem* GetFirstChildElem() const { return m_pFirstChild; }
	EckInline constexpr CElem* GetLastChildElem() const { return m_pLastChild; }

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

	EckInline auto GetStdTheme()
	{
		ECK_DUILOCKWND;
		return m_vTheme.front();
	}

	EckInline void StSwitchStdThemeMode(BOOL bDark)
	{
		ECK_DUILOCKWND;
		const auto pTheme = m_vTheme.front()->GetTheme();
		pTheme->SetPalette(pTheme->GetPaletteList()[!!bDark]);
		D2D1_COLOR_F cr;
		pTheme->GetSysColor(SysColor::Bk, cr);
		if (m_pBrBkg)
			m_pBrBkg->SetColor(cr);
		BroadcastEvent(WM_THEMECHANGED, 0, 0);
	}
	void StUpdateColorizationColor(const D2D1_COLOR_F& cr)
	{
		m_vTheme.front()->SetColorizationColor(cr);
	}
	void StUpdateColorizationColor()
	{
		DWORD Argb{};
		BOOL Dummy;
		DwmGetColorizationColor(&Argb, &Dummy);
		StUpdateColorizationColor(ARGBToD2dColorF(Argb));
	}

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
		if (it != m_vTimeLine.end())
		{
			(*it)->Release();
			m_vTimeLine.erase(it);
		}
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

	void CacheReserve(int cxPhy, int cyPhy)
	{
		if (cxPhy > m_cxCache || cyPhy > m_cyCache)
		{
			if (m_pBmpCache)
				m_pBmpCache->Release();
			m_cxCache = cxPhy;
			m_cyCache = cyPhy;
			BmpNew(m_cxCache, m_cyCache, m_pBmpCache);
		}
	}

	void CacheReserveLogSize(float cx, float cy)
	{
		CacheReserve((int)ceilf(Log2PhyF(cx)), (int)ceilf(Log2PhyF(cy)));
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

	void BlurInit()
	{
		ECK_DUILOCKWND;
		if (!m_pFxBlur)
		{
			GetDeviceContext()->CreateEffect(CLSID_D2D1GaussianBlur, &m_pFxBlur);
			m_pFxBlur->SetValue(D2D1_GAUSSIANBLUR_PROP_BORDER_MODE,
				D2D1_BORDER_MODE_HARD);
		}
		if (!m_pFxCrop)
			GetDeviceContext()->CreateEffect(CLSID_D2D1Crop, &m_pFxCrop);
	}
private:
	HRESULT BlurpDrawEffect(ID2D1Effect* pFx,
		D2D1_POINT_2F ptDrawing, BOOL bUseLayer)
	{
		const auto pDC = GetDeviceContext();
		const auto iBlend = pDC->GetPrimitiveBlend();
		pDC->SetPrimitiveBlend(D2D1_PRIMITIVE_BLEND_COPY);
		const static D2D1_LAYER_PARAMETERS1 LyParam
		{
			.contentBounds = D2D1::InfiniteRect(),
			.opacity = 1.f,
		};
		if (bUseLayer)
			pDC->PushLayer(LyParam, nullptr);
		pDC->DrawImage(m_pFxBlur, ptDrawing);
		if (bUseLayer)
			pDC->PopLayer();
		pDC->SetPrimitiveBlend(iBlend);
		return S_OK;
	}
public:
	/// <summary>
	/// 模糊当前设备上下文的内容，并画出。
	/// 调用方负责初始化效果与位图缓存，还负责刷新DC上任何挂起的操作
	/// </summary>
	/// <param name="rc">范围，相对当前位图。如果重画时有任何偏移量，必须手动与之相加</param>
	/// <param name="ptDrawing">效果画出点</param>
	/// <param name="fDeviation">标准差</param>
	/// <param name="bUseLayer">是否使用图层</param>
	/// <returns>HRESULT</returns>
	HRESULT BlurDrawDC(const D2D1_RECT_F& rc,
		D2D1_POINT_2F ptDrawing, float fDeviation, BOOL bUseLayer = FALSE)
	{
		ComPtr<ID2D1Bitmap1> pBmp;
		ComPtr<ID2D1Image> pTarget;
		GetDeviceContext()->GetTarget(&pTarget);
		pTarget->QueryInterface(&pBmp);
		HRESULT hr;
		float xDpi, yDpi;
		pBmp->GetDpi(&xDpi, &yDpi);
		const D2D1_RECT_U rcU
		{
			UINT32(rc.left * xDpi / 96.f),
			UINT32(rc.top * yDpi / 96.f),
			UINT32(rc.right * xDpi / 96.f),
			UINT32(rc.bottom * yDpi / 96.f)
		};
		if (FAILED(hr = GetCacheBitmap()->CopyFromBitmap(nullptr, pBmp.Get(), &rcU)))
			return hr;

		m_pFxBlur->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, fDeviation);
		m_pFxCrop->SetValue(D2D1_CROP_PROP_RECT,
			D2D1::RectF(0.f, 0.f, rc.right - rc.left, rc.bottom - rc.top));

		m_pFxCrop->SetInput(0, GetCacheBitmap());
		m_pFxBlur->SetInputEffect(0, m_pFxCrop);
		return BlurpDrawEffect(m_pFxBlur, ptDrawing, bUseLayer);
	}

	/// <summary>
	/// 模糊指定位图的内容，并画出。
	/// </summary>
	/// <param name="pBmp">输入位图，必须可作输入，即不能是“不能画”的</param>
	/// <param name="rc">范围</param>
	/// <param name="ptDrawing">画出点</param>
	/// <param name="fDeviation">标准差</param>
	/// <param name="bUseLayer">是否使用图层</param>
	/// <returns>HRESULT</returns>
	HRESULT BlurDrawDirect(ID2D1Bitmap1* pBmp, const D2D1_RECT_F& rc,
		D2D1_POINT_2F ptDrawing, float fDeviation, BOOL bUseLayer = FALSE)
	{
		m_pFxBlur->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, fDeviation);
		m_pFxCrop->SetValue(D2D1_CROP_PROP_RECT,
			D2D1::RectF(0.f, 0.f, rc.right - rc.left, rc.bottom - rc.top));

		m_pFxCrop->SetInput(0, pBmp);
		m_pFxBlur->SetInputEffect(0, m_pFxCrop);
		return BlurpDrawEffect(m_pFxBlur, ptDrawing, bUseLayer);
	}

	// 模糊指定位图的内容，并画出。
	// 忽略裁剪效果
	HRESULT BlurDrawDirect(ID2D1Bitmap1* pBmp,
		D2D1_POINT_2F ptDrawing, float fDeviation, BOOL bUseLayer = FALSE)
	{
		m_pFxBlur->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, fDeviation);
		m_pFxBlur->SetInput(0, pBmp);
		return BlurpDrawEffect(m_pFxBlur, ptDrawing, bUseLayer);
	}

	EckInlineCe void BlurSetUseLayer(BOOL bUseLayer) noexcept { m_bBlurUseLayer = bUseLayer; }
	EckInlineNdCe BOOL BlurGetUseLayer() const noexcept { return m_bBlurUseLayer; }
	EckInlineCe void BlurSetDeviation(float fDeviation) noexcept { m_fBlurDeviation = fDeviation; }
	EckInlineNdCe float BlurGetDeviation() const noexcept { return m_fBlurDeviation; }

	HRESULT BmpNew(int cxPhy, int cyPhy, _Out_ ID2D1Bitmap1*& pBmp)
	{
		const D2D1_BITMAP_PROPERTIES1 Prop
		{
			{ DXGI_FORMAT_B8G8R8A8_UNORM,D2D1_ALPHA_MODE_PREMULTIPLIED },
			(float)m_iUserDpi,
			(float)m_iUserDpi,
			D2D1_BITMAP_OPTIONS_TARGET
		};
		return GetDeviceContext()->CreateBitmap(D2D1::SizeU(cxPhy, cyPhy),
			nullptr, 0, Prop, &pBmp);
	}

	HRESULT BmpNewLogSize(float cx, float cy, _Out_ ID2D1Bitmap1*& pBmp)
	{
		return BmpNew((int)ceilf(Log2PhyF(cx)), (int)ceilf(Log2PhyF(cy)), pBmp);
	}

	EckInlineNdCe int GetClientWidth() const noexcept { return m_cxClient; }
	EckInlineNdCe int GetClientHeight() const noexcept { return m_cyClient; }
	EckInlineNdCe int GetClientWidthLog() const noexcept { return m_cxClientLog; }
	EckInlineNdCe int GetClientHeightLog() const noexcept { return m_cyClientLog; }

	/// <summary>
	/// 初始化目标
	/// 使用DCompositionVisual呈现时，初始化渲染到的目标视觉对象
	/// </summary>
	/// <param name="pVisual">目标视觉对象</param>
	/// <param name="pDevice">DComp设备</param>
	void DcvInit(IDCompositionVisual* pVisual, IDCompositionDevice* pDevice)
	{
		EckAssert(m_ePresentMode == PresentMode::DCompositionVisual);
		EckAssert(!m_pDcVisual && !m_pDcSurface && !m_pDcDevice);
		m_pDcVisual = pVisual;
		m_pDcVisual->AddRef();
		m_pDcDevice = pDevice;
		m_pDcDevice->AddRef();
	}

	EckInlineNdCe ID2D1DeviceContext* GetDeviceContext() const noexcept { return m_D2D.GetDC(); }

	template<std::invocable<CElem*> F>
	void EtForEachElem(const F& Fn, CElem* pElemBegin = nullptr)
	{
		auto p{ pElemBegin ? pElemBegin : GetFirstChildElem() };
		while (p)
		{
			EckCanCallbackContinue(Fn(p))
				return;
			EtForEachElem(Fn, p->GetFirstChildElem());
			p = p->GetNextElem();
		}
	}

	EckInlineCe void SetDrawDirtyRect(BOOL b)
	{
#ifdef _DEBUG
		m_bDrawDirtyRect = b;
#endif// _DEBUG
	}
};

inline BOOL CElem::IntCreate(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
	int x, int y, int cx, int cy, CElem* pParent, CDuiWnd* pWnd, int iId, PCVOID pData)
{
	EckAssert(!m_pWnd && !m_pDC);

	if (!pWnd)
		pWnd = pParent->GetWnd();
	m_iId = iId;
	m_pWnd = pWnd;
	m_pDC = pWnd->GetDeviceContext();
	m_pParent = pParent;
	m_pTheme = pWnd->m_vTheme.front();
	m_pTheme->AddRef();

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
	m_pWnd->ElemDestroying(this);
	CallEvent(WM_DESTROY, 0, 0);
	utcDestroyChild(this);

	switch (m_pWnd->GetPresentMode())
	{
	case PresentMode::AllDComp:
		SafeRelease(m_pDcVisual);
		SafeRelease(m_pDcSurface);
		SafeRelease(m_pDcContent);
		break;
	default:
	{
		if (GetCompositor())
		{
			CompInvalidateCacheBitmap();
			SafeRelease(m_pCompositor);
		}
	}
	break;
	}
	SafeRelease(m_pTextFormat);
	SafeRelease(m_pTheme);

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

	case WM_PAINT:
	{
		if (GetStyle() & DES_BASE_BEGIN_END_PAINT)
		{
			ELEMPAINTSTRU ps;
			BeginPaint(ps, wParam, lParam);
			EndPaint(ps);
		}
	}
	return 0;

	case WM_ERASEBKGND:
	{
		if (GetWnd()->IsElemUseDComp() ||
			(GetCompositor() && !(GetStyle() & DES_COMP_NO_REDIRECTION)))
			m_pDC->Clear({});
	}
	return TRUE;

	case WM_SIZE:
		if (GetCompositor())
		{
			CompReCalcCompositedRect();
			CompInvalidateCacheBitmap();
		}
		break;
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
	BOOL bProcessed{};
	const auto lResult = OnNotify((DUINMHDR*)pnm, bProcessed);
	if (bProcessed)
		return lResult;
	if (GetParentElem() && !(GetStyle() & DES_NOTIFY_TO_WND))
		return GetParentElem()->CallEvent(WM_NOTIFY, (WPARAM)this, (LPARAM)pnm);
	else
		return GetWnd()->OnElemEvent(this, ((DUINMHDR*)pnm)->uCode, 0, (LPARAM)pnm);
}

inline void CElem::InvalidateRect(const RECT& rc, BOOL bUpdateNow)
{
	ECK_DUILOCK;
	if (!(GetStyle() & DES_VISIBLE))
		return;
	RECT rcTemp;
	if (GetStyle() & DES_CONTENT_EXPAND)
		rcTemp = GetWholeRectInClient();
	else
	{
		rcTemp = rc;
		IntersectRect(rcTemp, rcTemp, GetWholeRectInClient());// 裁剪到元素矩形
	}
	if (IsRectEmpty(rcTemp))
		return;
	if (!(GetStyle() & DES_COMP_NO_REDIRECTION) &&
		((GetStyle() & DES_PARENT_COMP) || GetCompositor()))
	{
		auto pElem{ this };
		do
		{
			pElem->CompMarkDirty();
		} while (pElem = pElem->GetParentElem());
	}
	tcIrpUnionContentExpandElemRect(rcTemp);
	GetWnd()->IrUnion(rcTemp);
	if (bUpdateNow)
		GetWnd()->WakeRenderThread();
}

inline void CElem::BeginPaint(_Out_ ELEMPAINTSTRU& eps, WPARAM wParam, LPARAM lParam, UINT uFlags)
{
	const auto pExtra = (Priv::PAINT_EXTRA*)wParam;
	eps.prcClip = (const RECT*)lParam;
	eps.rcfClip = MakeD2DRcF(*eps.prcClip);
	eps.rcfClipInElem = MakeD2DRcF(*eps.prcClip);
	ClientToElem(eps.rcfClipInElem);
	eps.ox = pExtra->ox;
	eps.oy = pExtra->oy;
	if (uFlags & EBPF_DO_NOT_FILLBK)
		m_pDC->PushAxisAlignedClip(eps.rcfClipInElem, D2D1_ANTIALIAS_MODE_ALIASED);
	else
		if (m_dwStyle & DES_BLURBKG)
		{
			m_pDC->Flush();
			m_pDC->PushAxisAlignedClip(eps.rcfClipInElem, D2D1_ANTIALIAS_MODE_ALIASED);
			GetWnd()->CacheReserveLogSize(
				eps.rcfClip.right - eps.rcfClip.left,
				eps.rcfClip.bottom - eps.rcfClip.top);
			auto rc{ eps.rcfClip };
			OffsetRect(rc, eps.ox, eps.oy);
			GetWnd()->BlurDrawDC(rc,
				{ eps.rcfClipInElem.left, eps.rcfClipInElem.top },
				GetWnd()->BlurGetDeviation(),
				GetWnd()->BlurGetUseLayer());
		}
		else [[likely]]
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
		else if (GetCompositor())
		{
			if (GetStyle() & DES_OWNER_COMP_CACHE)
			{
				const auto& rcValid = m_pCompCacheSurface->GetValidRect();
				if (rcValid.right - rcValid.left < GetWidth() ||
					rcValid.bottom - rcValid.top < GetHeight())
					CompInvalidateCacheBitmap();
			}
			else
			{
				const auto size = m_pCompCachedBitmap->GetSize();
				if (size.width < GetWidthF() || size.height < GetHeightF())
					CompInvalidateCacheBitmap();
			}
		}
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
	if (!IsRectEmpty(rc))
	{
		InvalidateRect(rc);
		GetWnd()->IrUnion(rc);
	}
}

inline constexpr void CElem::tcIrpUnionContentExpandElemRect(_Inout_ RECT& rcInClient)
{
	for (const auto e : GetWnd()->m_vContentExpandElem)
	{
		const auto& rc = e->GetWholeRectInClient();
		if (IsRectsIntersect(rcInClient, rc))
			UnionRect(rcInClient, rcInClient, rc);
	}
}

inline constexpr void CElem::tcSetStyleWorker(DWORD dwStyle)
{
	if (dwStyle & DES_BLURBKG)
		dwStyle |= (DES_TRANSPARENT | DES_CONTENT_EXPAND);
	const auto dwOld = m_dwStyle;
	m_dwStyle = dwStyle;
	// 检查DES_CONTENT_EXPAND变动
	if ((dwOld & DES_CONTENT_EXPAND) && !(m_dwStyle & DES_CONTENT_EXPAND))
	{
		auto& v = GetWnd()->m_vContentExpandElem;
#ifdef _DEBUG
		BOOL bFound = FALSE;
#endif
		for (auto it = v.begin(); it != v.end(); ++it)
		{
			if (*it == this)
			{
#ifdef _DEBUG
				bFound = TRUE;
#endif
				v.erase(it);
				break;
			}
		}
		EckAssert(bFound);
	}
	else if (!(dwOld & DES_CONTENT_EXPAND) && (m_dwStyle & DES_CONTENT_EXPAND))
	{
		auto& v = GetWnd()->m_vContentExpandElem;
		EckAssert(std::find(v.begin(), v.end(), this) == v.end());
		v.emplace_back(this);
	}
	// 检查DES_OWNER_COMP_CACHE变动
	if (((dwOld ^ m_dwStyle) & DES_OWNER_COMP_CACHE) && GetCompositor())
		CompInvalidateCacheBitmap();
}

inline HRESULT CElem::CompUpdateCacheBitmap(int cx, int cy)
{
	if (m_pCompCachedBitmap)
	{
		float cxOld, cyOld;
		if (GetStyle() & DES_OWNER_COMP_CACHE)
		{
			const auto rc = m_pCompCacheSurface->GetValidRect();
			cxOld = rc.right - rc.left;
			cyOld = rc.bottom - rc.top;
		}
		else
		{
			const auto size = m_pCompCachedBitmap->GetSize();
			cxOld = size.width;
			cyOld = size.height;
		}
		if (cxOld >= (float)cx && cyOld >= (float)cy)
			return S_FALSE;
		else
			CompInvalidateCacheBitmap();
	}

	cx = (int)ceilf(Log2PhyF((float)cx));
	cy = (int)ceilf(Log2PhyF((float)cy));
	if (GetStyle() & DES_OWNER_COMP_CACHE)
	{
		if (FAILED(GetCompositor()->CreateCacheBitmap(
			cx, cy, m_pCompCacheSurface)))
		{
			CREATE_CACHE_BITMAP_INFO ccbi;
			ccbi.cxPhy = cx;
			ccbi.cyPhy = cy;
			ccbi.hr = E_NOTIMPL;
			const auto pNotifyElem = GetParentElem() ?
				GetParentElem() : this;
			if (pNotifyElem->CallEvent(
				EWM_CREATE_CACHE_BITMAP, (WPARAM)&ccbi, 0))
				m_pCompCacheSurface = ccbi.pCacheSurface;
			return ccbi.hr;
		}
	}
	else
		GetWnd()->BmpNew(cx, cy, m_pCompCachedBitmap);
	return S_OK;
}

EckInline CElem* CElem::SetCapture() { return GetWnd()->ElemSetCapture(this); }
EckInline void CElem::ReleaseCapture() { GetWnd()->ElemReleaseCapture(); }
EckInline void CElem::SetFocus() { GetWnd()->ElemSetFocus(this); }
EckInline BOOL CElem::SetTimer(UINT_PTR uId, UINT uElapse) { return GetWnd()->ElemSetTimer(this, uId, uElapse); }
EckInline BOOL CElem::KillTimer(UINT_PTR uId) { return GetWnd()->ElemKillTimer(this, uId); }
EckInlineCe CCriticalSection& CElem::GetCriticalSection() const { return GetWnd()->GetCriticalSection(); }
EckInlineCe int CElem::Log2Phy(int i) const { return GetWnd()->Log2Phy(i); }
EckInlineCe float CElem::Log2PhyF(float f) const { return GetWnd()->Log2PhyF(f); }
EckInlineCe int CElem::Phy2Log(int i) const { return GetWnd()->Phy2Log(i); }
EckInlineCe float CElem::Phy2LogF(float f) const { return GetWnd()->Phy2LogF(f); }
EckInlineCe ID2D1Bitmap1* CElem::GetCacheBitmap() const { return GetWnd()->GetCacheBitmap(); }

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
	return S_FALSE;
}
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END