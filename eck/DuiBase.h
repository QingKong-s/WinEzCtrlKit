﻿/*
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

#define ECK_DUI_NAMESPACE_BEGIN namespace Dui {
#define ECK_DUI_NAMESPACE_END }

#ifdef _DEBUG
#	if 0
#		define ECK_DUI_DBG_DRAW_FRAME								\
			{														\
				ID2D1SolidColorBrush* ECKPRIV_pBr___;				\
				m_pDC->CreateSolidColorBrush(D2D1::ColorF(1.f, 0.f, 0.f, 1.f), &ECKPRIV_pBr___); \
				if (ECKPRIV_pBr___) {								\
					m_pDC->DrawRectangle(rc, ECKPRIV_pBr___, 1.f);	\
					ECKPRIV_pBr___->Release();						\
				}													\
			}
#	else
#		define ECK_DUI_DBG_DRAW_FRAME ;
#	endif
#else
#		define ECK_DUI_DBG_DRAW_FRAME ;
#endif

#define ECK_ELEMTOP			((CElem*)HWND_TOP)
#define ECK_ELEMBOTTOM		((CElem*)HWND_BOTTOM)


ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
// 元素样式
enum
{
	DES_BLURBKG = (1u << 0),
	DES_DISABLE = (1u << 1),
	DES_VISIBLE = (1u << 2),
	DES_PRIV_BITMAP = (1u << 3),
	DES_TRANSPARENT = (1u << 4),

};

// 元素产生的通知
enum
{
	EE_COMMAND = 1,

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

// 仅用于元素
enum
{
	WM_DRAGENTER = WM_USER_SAFE,
	WM_DRAGOVER,
	WM_DRAGLEAVE,
	WM_DROP,
};


enum
{
	EBPF_DO_NOT_FILLBK = (1u << 0),
};
struct ELEMPAINTSTRU
{
	D2D1_RECT_F rcfClip;
	const RECT* prcClip;
	RECT rcInvalid;
};

/// <summary>
/// DUI元素基类
/// </summary>
class CElem
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
	ID2D1Bitmap* m_pPrivateBmp = NULL;

	RECT m_rc{};
	D2D1_RECT_F m_rcf{};

	RECT m_rcInvalid{};// 客户坐标

	RECT m_rcInClient{};
	D2D1_RECT_F m_rcfInClient{};

	CRefStrW m_rsText{};

	DWORD m_dwStyle = 0;
	DWORD m_dwExStyle = 0;

	BITBOOL m_bAllowRedraw : 1 = FALSE;

	BOOL IntCreate(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, CElem* pParent, CDuiWnd* pWnd, PCVOID pData = NULL);

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
		auto pElem = GetFirstChildElem();
		while (pElem)
		{
			if (pElem->GetStyle() & DES_VISIBLE)
			{
				if (PtInRect(pElem->GetRectInClient(), pt) &&
					pElem->CallEvent(WM_NCHITTEST, 0, MAKELPARAM(pt.x, pt.y)) != HTTRANSPARENT)
				{
					const auto pHit = pElem->HitTestChildUncheck(pt);
					if (pHit)
						return pHit;
					return pElem;
				}
			}
			pElem = pElem->GetNextElem();
		}
		return NULL;
	}

	void IRUnionTransparentElemRect(CElem* pLast, RECT& rcInClient)
	{
		while (pLast)
		{
			const auto& rcElem = pLast->GetRectInClient();
			if (pLast->GetStyle() & DES_VISIBLE)
			{
				if (IsBitSet(pLast->GetStyle(), DES_TRANSPARENT))
				{
					if (IsRectsIntersect(rcInClient, rcElem))
					{
						pLast->m_rcInvalid = rcElem;// invalidate
						UnionRect(rcInClient, rcInClient, rcElem);
					}
				}
				else
					IRUnionTransparentElemRect(pLast->GetLastChildElem(), rcInClient);
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
			pElem->SRCorrectChildrenRectInClient();
			pElem = pElem->GetNextElem();
		}
	}

	EckInline void RedrawWnd(const RECT& rc)
	{
		RECT rcReal = rc;

		CElem* pElem;
		if (GetStyle() & DES_TRANSPARENT)
			pElem = GetWnd()->GetLastChildElem();
		else
			pElem = this;

		IRUnionTransparentElemRect(pElem, rcReal);
		GetWnd()->RedrawDui(rcReal);
	}

	void DestroyPrivateBitmap()
	{
		SafeRelease(m_pPrivateBmp);
	}

	void UpdatePrivateBitmap()
	{
		DestroyPrivateBitmap();
		m_pDC->CreateBitmap(
			D2D1::SizeU(GetWidth(), GetHeight()),
			NULL,
			0,
			D2D1::BitmapProperties(m_pDC->GetPixelFormat()),
			&m_pPrivateBmp);
	}
public:
	virtual LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_NCHITTEST:
			return HTCLIENT;
		}
		return 0;
	}

	EckInline LRESULT CallEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		return OnEvent(uMsg, wParam, lParam);
	}

	EckInline const D2D1_RECT_F& GetRectF() const { return m_rcf; }

	EckInline const RECT& GetRect() const { return m_rc; }

	EckInline void SetRect(const RECT& rc)
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
		CallEvent(WM_SIZE, 0, 0);
		InvalidateRect();
	}

	virtual BOOL Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, CElem* pParent, CDuiWnd* pWnd, PCVOID pData = NULL)
	{
		return IntCreate(pszText, dwStyle, dwExStyle, x, y, cx, cy, pParent, pWnd, pData);
	}

	void Destroy();

	EckInline CElem* GetFirstChildElem() const { return m_pFirstChild; }
	EckInline CElem* GetLastChildElem() const { return m_pLastChild; }
	EckInline CElem* GetParentElem() const { return m_pParent; }
	EckInline CDuiWnd* GetWnd() const { return m_pWnd; }
	EckInline ID2D1DeviceContext* GetD2DDC() const { return m_pDC; }

	EckInline const CRefStrW& GetText() const { return m_rsText; }

	EckInline void SetText(PCWSTR pszText)
	{
		m_rsText = pszText;
		CallEvent(WM_SETTEXT, 0, 0);
	}

	EckInline DWORD GetStyle() const { return m_dwStyle; }

	EckInline DWORD GetExStyle() const { return m_dwExStyle; }

	EckInline void SetStyle(DWORD dwStyle)
	{ 
		if (dwStyle & DES_BLURBKG)
			dwStyle |= DES_TRANSPARENT;

		const auto dwOldStyle = m_dwStyle;
		m_dwStyle = dwStyle;

		if (!(dwOldStyle & DES_PRIV_BITMAP) && (dwStyle & DES_PRIV_BITMAP))
			UpdatePrivateBitmap();
		else if ((dwOldStyle & DES_PRIV_BITMAP) && !(dwStyle & DES_PRIV_BITMAP))
			DestroyPrivateBitmap();

		CallEvent(WM_STYLECHANGED, dwOldStyle, dwStyle);
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

	EckInline void ParentToElem(POINT& pt, CElem* pParentEnd = NULL)
	{
		auto pParent = this;
		do
		{
			pt.x -= pParent->GetRect().left;
			pt.y -= pParent->GetRect().top;
			pParent = pParent->GetParentElem();
		} while (pParent != pParentEnd);
	}

	EckInline void ParentToElem(RECT& rc, CElem* pParentEnd = NULL)
	{
		auto pParent = this;
		do
		{
			OffsetRect(rc, -pParent->GetRect().left, -pParent->GetRect().top);
			pParent = pParent->GetParentElem();
		} while (pParent != pParentEnd);
	}

	EckInline void ElemToParent(POINT& pt, CElem* pParentEnd = NULL)
	{
		auto pParent = this;
		while (pParent != pParentEnd)
		{
			pt.x += pParent->GetRect().left;
			pt.y += pParent->GetRect().top;
			pParent = pParent->GetParentElem();
		}
	}

	EckInline void ElemToParent(RECT& rc, CElem* pParentEnd = NULL)
	{
		auto pParent = this;
		while (pParent != pParentEnd)
		{
			OffsetRect(rc, pParent->GetRect().left, pParent->GetRect().top);
			pParent = pParent->GetParentElem();
		}
	}

	EckInline void ElemToParent(D2D1_RECT_F& rc, CElem* pParentEnd = NULL)
	{
		auto pParent = this;
		while (pParent != pParentEnd)
		{
			OffsetRect(rc, pParent->GetRect().left, pParent->GetRect().top);
			pParent = pParent->GetParentElem();
		}
	}

	EckInline void ClientToElem(RECT& rc)
	{
		OffsetRect(rc, -m_rcInClient.left, -m_rcInClient.top);
	}

	EckInline void ClientToElem(POINT& pt)
	{
		pt.x -= m_rcInClient.left;
		pt.y -= m_rcInClient.top;
	}

	EckInline void ElemToClient(RECT& rc)
	{
		OffsetRect(rc, m_rcInClient.left, m_rcInClient.top);
	}

	EckInline void ElemToClient(D2D1_RECT_F& rc)
	{
		OffsetRect(rc, m_rcfInClient.left, m_rcfInClient.top);
	}

	EckInline int GetWidth() const { return m_rc.right - m_rc.left; }

	EckInline int GetHeight() const { return m_rc.bottom - m_rc.top; }

	EckInline float GetWidthF() const { return m_rcf.right - m_rcf.left; }

	EckInline float GetHeightF() const { return m_rcf.bottom - m_rcf.top; }

	void SetZOrder(CElem* pElemAfter);

	EckInline void GetRectFAbs(D2D1_RECT_F& rc) const
	{
		rc = m_rcf;
		if (m_pParent)
			m_pParent->ElemToParent(rc);
	}

	EckInline LRESULT GenElemNotify(UINT uMsg, WPARAM wParam, LPARAM lParam);

	EckInline void SetRedraw(BOOL bRedraw)
	{
		if (bRedraw)
		{
			m_bAllowRedraw = TRUE;
			if (!IsRectEmpty(m_rcInvalid))
				RedrawWnd(m_rcInvalid);
		}
		else
			m_bAllowRedraw = FALSE;
	}

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
};

/// <summary>
/// DUI窗体类
/// </summary>
class CDuiWnd :public CWnd
{
	friend class CElem;
	friend class CDuiDropTarget;
private:
	CElem* m_pFirstChild = NULL;		// 第一个子元素
	CElem* m_pLastChild = NULL;			// 最后一个子元素

	CElem* m_pFocusElem = NULL;			// 当前焦点元素
	CElem* m_pCurrNcHitTestElem = NULL;	// 当前非客户区命中元素
	CElem* m_pMouseCaptureElem = NULL;	// 当前鼠标捕获元素
	CElem* m_pHoverElem = NULL;			// 当前鼠标悬停元素，for WM_MOUSELEAVE
	CElem* m_pDragDropElem = NULL;		// 当前拖放元素

	IDataObject* m_pDataObj = NULL;		// 
	CDuiDropTarget* m_pDropTarget = NULL;

	CEzD2D m_D2d{};
	ID2D1Bitmap* m_pBmpBkg = NULL;
	ID2D1SolidColorBrush* m_pBrBkg = NULL;

	IDWriteTextFormat* m_pDefTextFormat = NULL;

	UINT m_uLastError = S_OK;

	int m_cxClient = 0;
	int m_cyClient = 0;

	int m_iDpi = USER_DEFAULT_SCREEN_DPI;
	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY_F(CommEdge, 1.f)
		ECK_DS_ENTRY_F(CommRrcRadius, 4.f)
		ECK_DS_ENTRY_F(CommMargin, 4.f)
		;
	ECK_DS_END_VAR(m_Ds);

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
						if (pElem->GetStyle() & DES_PRIV_BITMAP)
						{
							pElem->m_pDC->SetTarget(pElem->m_pPrivateBmp);
							pElem->m_pDC->SetTransform(D2D1::Matrix3x2F::Identity());
						}
						else
							pElem->m_pDC->SetTransform(D2D1::Matrix3x2F::Translation(
								pElem->GetRectInClientF().left,
								pElem->GetRectInClientF().top));
						pElem->CallEvent(WM_PAINT, 0, (WPARAM)&rcClip);
						RedrawElem(pElem->GetLastChildElem(), rcClip);
						if (pElem->GetStyle() & DES_PRIV_BITMAP)
							pElem->m_pDC->SetTarget(m_D2d.GetBitmap());
					}
				}
			}
			pElem = pElem->GetPrevElem();
		}
	}

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

	EckInline void RedrawDui(const RECT& rc)
	{
		m_D2d.GetDC()->BeginDraw();
		m_D2d.GetDC()->SetTransform(D2D1::Matrix3x2F::Identity());
		FillBackground(MakeD2DRcF(rc));
		RedrawElem(GetLastChildElem(), rc);
		m_D2d.GetDC()->EndDraw();
		m_D2d.GetSwapChain()->Present(0, 0);
	}

	EckInline void RedrawDui()
	{
		const RECT rcClient{ 0,0,m_cxClient,m_cyClient };
		RedrawDui(rcClient);
	}
public:
	ID2D1Bitmap* m_pBmpBlurCache = NULL;

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		if (uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST)
		{
			auto pElem = (m_pMouseCaptureElem ? m_pMouseCaptureElem : m_pCurrNcHitTestElem);
			if (pElem)
				pElem->CallEvent(uMsg, wParam, lParam);

			if (uMsg == WM_MOUSEMOVE)
			{
				if (m_pHoverElem != pElem)
				{
					if (m_pHoverElem)
						m_pHoverElem->CallEvent(WM_MOUSELEAVE, 0, 0);
					m_pHoverElem = pElem;
				}
				TRACKMOUSEEVENT tme;
				tme.cbSize = sizeof(tme);
				tme.dwFlags = TME_LEAVE;
				tme.hwndTrack = hWnd;
				TrackMouseEvent(&tme);
			}
			else if (uMsg == WM_LBUTTONDBLCLK)
			{
				PostMsg(WM_LBUTTONDOWN, wParam, lParam);
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
		{
			RECT rc;
			GetUpdateRect(hWnd, &rc, FALSE);
			ValidateRect(hWnd, NULL);
			RedrawDui(rc);
		}
		return 0;

		case WM_SIZE:
			ECK_GET_SIZE_LPARAM(m_cxClient, m_cyClient, lParam);
			m_D2d.ReSize(1, m_cxClient, m_cyClient, 0);
			break;

		case WM_SETCURSOR:
			if (m_pCurrNcHitTestElem)
				m_pCurrNcHitTestElem->CallEvent(uMsg, wParam, lParam);
			break;

		case WM_MOUSELEAVE:
			if (m_pHoverElem)
			{
				m_pHoverElem->CallEvent(WM_MOUSELEAVE, 0, 0);
				m_pHoverElem = NULL;
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

				m_pDefTextFormat = CreateDefTextFormat(96);
				m_pDefTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

				m_D2d.Create(EZD2D_PARAM::MakeBitblt(hWnd,
					g_pDxgiFactory, g_pDxgiDevice, g_pD2dDevice, rc.right, rc.bottom));
				m_D2d.GetDC()->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);

				m_D2d.GetDC()->CreateSolidColorBrush(D2D1::ColorF(1.f, 1.f, 1.f, 1.f), &m_pBrBkg);
				m_cxClient = rc.right;
				m_cyClient = rc.bottom;
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

	EckInline ID2D1SolidColorBrush* GetBkgBrush() const { return m_pBrBkg; }

	EckInline void SetBkgBitmap(ID2D1Bitmap* pBmp)
	{
		std::swap(m_pBmpBkg, pBmp);
		if (m_pBmpBkg)
			m_pBmpBkg->AddRef();
		if (pBmp)
			pBmp->Release();
		RedrawDui();
	}

	void FillBackground(const D2D1_RECT_F& rc)
	{
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
		auto pElem = m_pFirstChild;
		while (pElem)
		{
			if (pElem->GetStyle() & DES_VISIBLE)
			{
				if (PtInRect(pElem->GetRectInClient(), pt) &&
					pElem->CallEvent(WM_NCHITTEST, 0, MAKELPARAM(pt.x, pt.y)) != HTTRANSPARENT)
				{
					auto pHit = pElem->HitTestChildUncheck(pt);
					if (pHit)
						return pHit;
					else
						return pElem;
				}
			}
			pElem = pElem->GetNextElem();
		}
		return NULL;
	}

	CElem* SetFocusElem(CElem* pElem)
	{
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
			SetCapture(m_hWnd);
		if (pOld)
			pOld->CallEvent(WM_CAPTURECHANGED, 0, (LPARAM)pElem);
		return pOld;
	}

	void ReleaseCaptureElem()
	{
		if (m_pMouseCaptureElem)
		{
			ReleaseCapture();
			m_pMouseCaptureElem->CallEvent(WM_CAPTURECHANGED, 0, (LPARAM)m_pMouseCaptureElem);
			m_pMouseCaptureElem = NULL;
		}
	}

	EckInline CElem* GetCaptureElem() const { return m_pMouseCaptureElem; }

	EckInline CElem* GetFirstChildElem() const { return m_pFirstChild; }

	EckInline CElem* GetLastChildElem() const { return m_pLastChild; }

	EckInline const DPIS& GetDs() const { return m_Ds; }

	EckInline int GetDpiValue() const { return m_iDpi; }

	EckInline HRESULT EnableDragDrop(BOOL bEnable);

	EckInline auto GetDefTextFormat() const { return m_pDefTextFormat; }
};



BOOL CElem::IntCreate(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
	int x, int y, int cx, int cy, CElem* pParent, CDuiWnd* pWnd, PCVOID pData)
{
	EckAssert(!m_pWnd && !m_pDC);

	SetRedraw(FALSE);
	if (dwStyle & DES_BLURBKG)
		dwStyle |= DES_TRANSPARENT;
	SetStyle(dwStyle);
	m_dwExStyle = dwExStyle;

	m_pWnd = pWnd;
	m_pDC = pWnd->m_D2d.GetDC();

#ifdef _DEBUG
	if (pParent)
		EckAssert(pParent->m_pWnd == pWnd);
#endif
	auto& pParentLastChild = (pParent ? pParent->m_pLastChild : pWnd->m_pLastChild);
	auto& pParentFirstChild = (pParent ? pParent->m_pFirstChild : pWnd->m_pFirstChild);
	m_pParent = pParent;

	if (pParentFirstChild)
	{
		m_pPrev = NULL;
		m_pNext = pParentFirstChild;
		m_pNext->m_pPrev = this;
		pParentFirstChild = this;
	}
	else
	{
		m_pPrev = m_pNext = NULL;
		pParentFirstChild = this;
		pParentLastChild = this;
	}

	SetText(pszText);
	SetRect({ x,y,x + cx,y + cy });
	SetRedraw(TRUE);

	if (CallEvent(WM_CREATE, 0, (LPARAM)pData))
	{
		Destroy();
		return FALSE;
	}
	else
		return TRUE;
}

void CElem::Destroy()
{
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

void CElem::SetZOrder(CElem* pElemAfter)
{
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

LRESULT CElem::GenElemNotify(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return m_pWnd->OnElemEvent(this, uMsg, wParam, lParam);
}

void CElem::InvalidateRect(const RECT* prc)
{
	if ((GetStyle() & DES_TRANSPARENT) || !prc)
		m_rcInvalid = GetRectInClient();
	else
	{
		UnionRect(m_rcInvalid, m_rcInvalid, *prc);
		IntersectRect(m_rcInvalid, m_rcInvalid, GetRectInClient());// 裁剪到元素矩形
	}
	if (!GetRedraw())
		return;
	RedrawWnd(m_rcInvalid);
}

void CElem::BeginPaint(ELEMPAINTSTRU& eps, WPARAM wParam, LPARAM lParam, UINT uFlags)
{
	eps.prcClip = (const RECT*)lParam;
	eps.rcfClip = MakeD2DRcF(*eps.prcClip);
	eps.rcInvalid = m_rcInvalid;
	m_rcInvalid = {};

	if (uFlags & EBPF_DO_NOT_FILLBK)
		m_pDC->PushAxisAlignedClip(eps.rcfClip, D2D1_ANTIALIAS_MODE_ALIASED);
	else
		if (m_dwStyle & DES_BLURBKG)
		{
			m_pDC->Flush();
			m_pDC->PushAxisAlignedClip(eps.rcfClip, D2D1_ANTIALIAS_MODE_ALIASED);
			BlurD2dDC(m_pDC, m_pWnd->GetD2D().GetBitmap(), m_pWnd->m_pBmpBlurCache, eps.rcfClip, 10.0f);
		}
		else
		{
			m_pDC->PushAxisAlignedClip(eps.rcfClip, D2D1_ANTIALIAS_MODE_ALIASED);
			CallEvent(WM_ERASEBKGND, 0, (LPARAM)&eps);
		}
}

CElem* CElem::SetCapture() { return m_pWnd->SetCaptureElem(this); }

void CElem::ReleaseCapture() { m_pWnd->ReleaseCaptureElem(); }



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
			return pElem->CallEvent(WM_DRAGENTER, (WPARAM)&ddi, MAKELPARAM(pt0.x, pt0.y));
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
				return pElem->CallEvent(WM_DRAGENTER, (WPARAM)&ddi, MAKELPARAM(pt0.x, pt0.y));
			}
		}
		else if (pElem)
			return pElem->CallEvent(WM_DRAGOVER, (WPARAM)&ddi, MAKELPARAM(pt0.x, pt0.y));
		else
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
			return pElem->CallEvent(WM_DRAGLEAVE, 0, 0);
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
			return pElem->CallEvent(WM_DROP, (WPARAM)&ddi, MAKELPARAM(pt0.x, pt0.y));
		else
			return S_OK;
	}
};

EckInline HRESULT CDuiWnd::EnableDragDrop(BOOL bEnable)
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