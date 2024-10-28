/*
* WinEzCtrlKit Library
*
* CLabel.h ： 标签
*
* Copyright(C) 2023-2024 QingKong
*/
#pragma once
#include "CWnd.h"
#include "GraphicsHelper.h"

ECK_NAMESPACE_BEGIN
struct LA_HITTEST
{
	BITBOOL bHit : 1;
	BITBOOL bHitText : 1;
	BITBOOL bHitImg : 1;
};

constexpr inline std::array<COLORREF, 3> DefLabelGradient{ 0x808080,0xFFFFFF,0x808080 };

class CLabel :public CWnd
{
public:
	ECK_RTTI(CLabel);
	ECK_CWND_SINGLEOWNER(CLabel);
	ECK_CWND_CREATE_CLS_HINST(WCN_LABEL, g_hInstance);

	enum class Ellipsis :BYTE
	{
		None,
		End,
		Path,
		Word
	};

	enum class Prefix :BYTE
	{
		Normal,
		NoPrefix,
		HidePrefix,
		PrefixOnly,
	};

	enum class MouseOpt :BYTE
	{
		None,
		TransparentSpace,
		Transparent,
	};
private:
	CEzCDC m_DC{};			// 兼容DC
	HBITMAP m_hbmImg{};		// 图片，无需销毁
	HBITMAP m_hbmBkImg{};	// 底图，无需销毁
	int m_cxBkImg{},
		m_cyBkImg{};		// 底图大小
	HFONT m_hFont{};		// 字体，无需销毁
	CRefStrW m_rsText{};

	BkImgMode m_eBkImgMode{ BkImgMode::TopLeft };		// 底图模式
	GradientMode m_eGradientMode{ GradientMode::None };	// 渐变模式
	Align m_eAlignH{};	// 横向对齐
	Align m_eAlignV{};	// 纵向对齐

	Ellipsis m_eEllipsisMode{};		// 省略号模式
	Prefix m_ePrefixMode{};			// 前缀模式
	MouseOpt m_eMouseOption{};		// 鼠标穿透
	BITBOOL m_bAutoWrap : 1{};		// 自动换行
	BITBOOL m_bFullWndImg : 1{};	// 底图尽量充满控件
	BITBOOL m_bTransparent : 1{};	// 透明标签
	BITBOOL m_bImgAlpha : 1{};		// 图片需要Alpha混合
	BITBOOL m_bBkImgAlpha : 1{};	// 底图需要Alpha混合

	BITBOOL m_bPartMetricsDirty : 1{ TRUE };// 部件矩形需要重新计算

	COLORREF m_crText{ CLR_DEFAULT };	// 文本颜色
	COLORREF m_crTextBK{ CLR_DEFAULT };	// 文本背景颜色
	COLORREF m_crBK{ CLR_DEFAULT };		// 背景颜色
	std::array<COLORREF, 3> m_crGradient{ DefLabelGradient };

	int m_cxClient{},
		m_cyClient{};

	RECT m_rcPartImg{};	// 缓存的图片矩形
	RECT m_rcPartText{};// 缓存的文本矩形

	constexpr UINT DtEllipsis() const
	{
		switch (m_eEllipsisMode)
		{
		case Ellipsis::End:	return DT_END_ELLIPSIS; break;
		case Ellipsis::Path:return DT_PATH_ELLIPSIS; break;
		case Ellipsis::Word:return DT_WORD_ELLIPSIS; break;
		}
		return 0u;
	}

	constexpr UINT DtPrefix() const
	{
		switch (m_ePrefixMode)
		{
		case Prefix::NoPrefix:	return DT_NOPREFIX; break;
		case Prefix::HidePrefix:return DT_HIDEPREFIX; break;
		case Prefix::PrefixOnly:return DT_PREFIXONLY; break;
		}
		return 0u;
	}

	constexpr UINT DtAlignH() const
	{
		switch (m_eAlignH)
		{
		case Align::Center:	return DT_CENTER; break;
		case Align::Far:	return DT_RIGHT; break;
		}
		return DT_LEFT;
	}

	void Paint(HDC hDC, const RECT& rcPaint)
	{
		if (m_bPartMetricsDirty)
		{
			m_bPartMetricsDirty = FALSE;
			CalcPartsRect();
		}
		const auto hCDC = (m_hbmImg || m_hbmBkImg) ?
			CreateCompatibleDC(hDC) : nullptr;
		RECT rc{ 0,0,m_cxClient,m_cyClient };
		// 画纯色背景
		if (!m_bTransparent)
		{
			FillRect(hDC, &rc, GetStockBrush(DC_BRUSH));
		}
		// 画渐变背景或底图
		if (m_eGradientMode != GradientMode::None)
			FillGradientRect(hDC, rc, m_crGradient.data(), m_eGradientMode);
		else if (m_hbmBkImg)
		{
			SelectObject(hCDC, m_hbmBkImg);
			if (m_bBkImgAlpha)
				DrawBackgroundImage32(hDC, hCDC, rc, m_cxBkImg, m_cyBkImg,
					m_eBkImgMode, m_bFullWndImg);
			else
				DrawBackgroundImage(hDC, hCDC, rc, m_cxBkImg, m_cyBkImg,
					m_eBkImgMode, m_bFullWndImg);
		}
		// 画图片
		if (m_hbmImg)
		{
			SelectObject(hCDC, m_hbmImg);
			if (m_bImgAlpha)
				AlphaBlend(hDC,
					m_rcPartImg.left,
					m_rcPartImg.top,
					m_rcPartImg.right - m_rcPartImg.left,
					m_rcPartImg.bottom - m_rcPartImg.top,
					hCDC, 0, 0,
					m_rcPartImg.right - m_rcPartImg.left,
					m_rcPartImg.bottom - m_rcPartImg.top, BlendFuncAlpha);
			else
				BitBlt(hDC,
					m_rcPartImg.left,
					m_rcPartImg.top,
					m_rcPartImg.right - m_rcPartImg.left,
					m_rcPartImg.bottom - m_rcPartImg.top, hCDC, 0, 0, SRCCOPY);
		}
		// 画文本
		const UINT uDTFlags = DT_NOCLIP | DtEllipsis() | DtPrefix() | DtAlignH() |
			(m_bAutoWrap ? DT_WORDBREAK : DT_SINGLELINE);
		DrawTextW(hDC, m_rsText.Data(), m_rsText.Size(), &m_rcPartText, uDTFlags);
		if (hCDC)
			DeleteDC(hCDC);
	}

	void CalcPartsRect()
	{
		BITMAP bm;
		int cxImg, cyImg;
		if (GetObjectW(m_hbmImg, sizeof(bm), &bm))
		{
			cxImg = bm.bmWidth;
			cyImg = bm.bmHeight;
		}
		else
			cxImg = cyImg = 0;

		RECT rc{ 0,0,m_cxClient - cxImg,m_cyClient };
		UINT uDtFlags = DT_NOCLIP | DT_CALCRECT | DtEllipsis() | DtPrefix();
		int xPic, yPic;

		if (m_bAutoWrap)
		{
			uDtFlags |= DT_WORDBREAK;
			DrawTextW(m_DC.GetDC(), m_rsText.Data(), m_rsText.Size(), &rc, uDtFlags);

			int cyText = rc.bottom - rc.top;
			switch (m_eAlignV)
			{
			case Align::Near:// 上边
				rc.top = 0;
				rc.bottom = rc.top + cyText;
				yPic = rc.top;
				break;
			case Align::Center:// 中间
				rc.top = (m_cyClient - cyText) / 2;
				rc.bottom = rc.top + cyText;
				yPic = (m_cyClient - cyImg) / 2;
				break;
			case Align::Far:// 下边
				rc.bottom = m_cyClient;
				rc.top = rc.bottom - cyText;
				yPic = m_cyClient - cyImg;
				break;
			default: ECK_UNREACHABLE;
			}
		}
		else
		{
			uDtFlags |= DT_SINGLELINE;
			DrawTextW(m_DC.GetDC(), m_rsText.Data(), m_rsText.Size(), &rc, uDtFlags);

			int cyText = rc.bottom - rc.top;
			switch (m_eAlignV)
			{
			case Align::Near:// 上边
				rc.top = 0;
				rc.bottom = rc.top + cyText;
				yPic = rc.top;
				break;
			case Align::Center:// 中间
				rc.top = (m_cyClient - cyText) / 2;
				rc.bottom = rc.top + cyText;
				yPic = (m_cyClient - cyImg) / 2;
				break;
			case Align::Far:// 下边
				rc.bottom = m_cyClient;
				rc.top = rc.bottom - cyText;
				yPic = m_cyClient - cyImg;
				break;
			default: ECK_UNREACHABLE;
			}
		}
		uDtFlags &= (~DT_CALCRECT);

		const int cxText = rc.right - rc.left;
		switch (m_eAlignH)
		{
		case Align::Near:// 左边
			uDtFlags |= DT_LEFT;
			rc.left = cxImg;
			rc.right = rc.left + cxText;
			xPic = 0;
			break;
		case Align::Center:// 中间
			rc.left = (m_cxClient - (cxText + cxImg)) / 2 + cxImg;
			rc.right = rc.left + cxText;
			xPic = rc.left - cxImg;
			break;
		case Align::Far:// 右边
			uDtFlags |= DT_RIGHT;
			rc.right = m_cxClient - cxImg;
			rc.left = rc.right - cxText;
			xPic = rc.right;
			break;
		default: ECK_UNREACHABLE;
		}

		m_rcPartImg.left = xPic;
		m_rcPartImg.top = yPic;
		m_rcPartImg.right = m_rcPartImg.left + cxImg;
		m_rcPartImg.bottom = m_rcPartImg.top + cyImg;

		m_rcPartText = rc;
	}

	void SetDCAttributes(HDC hDC)
	{
		const auto* const ptc = GetThreadCtx();
		SelectObject(hDC, m_hFont);
		SetTextColor(hDC, m_crText == CLR_DEFAULT ? ptc->crDefText : m_crText);
		SetDCBrushColor(hDC, m_crBK == CLR_DEFAULT ? ptc->crDefBkg : m_crBK);
		if (m_crTextBK == CLR_DEFAULT)
			SetBkMode(hDC, TRANSPARENT);
		else
		{
			SetBkMode(hDC, OPAQUE);
			SetBkColor(hDC, m_crTextBK);
		}
	}
public:
	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_NCHITTEST:
		{
			if (m_bTransparent)
				switch (m_eMouseOption)
				{
				case MouseOpt::TransparentSpace:
				{
					POINT pt{ GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam) };
					ScreenToClient(hWnd, &pt);
					LA_HITTEST laht;
					HitTest(pt, laht);
					if (!laht.bHitImg && !laht.bHitText)
						return HTTRANSPARENT;
				}
				break;
				case MouseOpt::Transparent:
					return HTTRANSPARENT;
				}
		}
		break;

		case WM_WINDOWPOSCHANGING:
		{
			if (m_bTransparent)
			{
				const auto pwp = (WINDOWPOS*)lParam;
				pwp->flags |= SWP_NOCOPYBITS;
			}
		}
		break;

		case WM_PRINTCLIENT:
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hWnd, wParam, ps);
			if (m_bTransparent)
			{
				SaveDC(ps.hdc);
				SetDCAttributes(ps.hdc);
				Paint(ps.hdc, ps.rcPaint);
				RestoreDC(ps.hdc, -1);
			}
			else
			{
				Paint(m_DC.GetDC(), ps.rcPaint);
				BitBltPs(ps, m_DC.GetDC());
			}
			EndPaint(hWnd, wParam, ps);
		}
		return 0;

		case WM_THEMECHANGED:
			SetDCAttributes(m_DC.GetDC());
			break;

		case WM_SETFONT:
		{
			m_hFont = (HFONT)wParam;
			SelectObject(m_DC.GetDC(), m_hFont);
			CalcPartsRect();
			if (LOWORD(lParam))
				Redraw();
		}
		return 0;
		case WM_GETFONT:
			return (LRESULT)m_hFont;

		case WM_SETTEXT:
			m_rsText = (PWSTR)lParam;
			CalcPartsRect();
			Redraw();
			return TRUE;
		case WM_GETTEXTLENGTH:
			return m_rsText.Size();
		case WM_GETTEXT:
			if ((int)wParam > 0)
				return m_rsText.CopyTo((PWSTR)lParam, (int)wParam - 1);
			else
				return 0;

		case WM_GETDLGCODE:
			return DLGC_STATIC;

		case WM_SIZE:
		{
			m_cxClient = LOWORD(lParam);
			m_cyClient = HIWORD(lParam);
			m_DC.ReSize(hWnd, m_cxClient, m_cyClient);
			SetDCAttributes(m_DC.GetDC());
			CalcPartsRect();
		}
		return 0;

		case WM_CREATE:
		{
			m_rsText = ((CREATESTRUCTW*)lParam)->lpszName;
			m_DC.Create(hWnd);
			SetDCAttributes(m_DC.GetDC());
		}
		break;

		case WM_DESTROY:
		{
			m_DC.Destroy();
			m_hbmBkImg = nullptr;
			m_hbmImg = nullptr;
			m_hFont = nullptr;
			m_eBkImgMode = BkImgMode::TopLeft;
			m_eGradientMode = GradientMode::None;
			m_eAlignH = Align::Near;
			m_eAlignV = Align::Near;
			m_eEllipsisMode = Ellipsis::None;
			m_ePrefixMode = Prefix::Normal;
			m_eMouseOption = MouseOpt::None;
			m_bAutoWrap = m_bFullWndImg = m_bTransparent =
				m_bImgAlpha = m_bBkImgAlpha = FALSE;
			m_bPartMetricsDirty = TRUE;
			m_crText = m_crTextBK = m_crBK = CLR_DEFAULT;
			m_crGradient = DefLabelGradient;
		}
		break;
		}

		return CWnd::OnMsg(hWnd, uMsg, wParam, lParam);
	}

	void Redraw()
	{
		if (!m_hWnd)
			return;
		if (m_bTransparent)
		{
			RECT rc{ 0,0,m_cxClient,m_cyClient };
			HWND hParent = GetParent(m_hWnd);
			MapWindowPoints(m_hWnd, hParent, (POINT*)&rc, 2);
			RedrawWindow(hParent, &rc, nullptr,
				RDW_ERASE | RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW);
		}
		else
		{
			InvalidateRect(m_hWnd, nullptr, FALSE);
			UpdateWindow(m_hWnd);
		}
	}

	HBITMAP SetBkImg(HBITMAP hBitmap)
	{
		BITMAP bm;
		if (GetObjectW(hBitmap, sizeof(bm), &bm))
		{
			m_cxBkImg = bm.bmWidth;
			m_cyBkImg = bm.bmHeight;
			std::swap(m_hbmBkImg, hBitmap);
			return hBitmap;
		}
		else
			return nullptr;
	}
	EckInline constexpr HBITMAP GetBkImg() const { return m_hbmBkImg; }

	EckInline constexpr void SetBkImgMode(BkImgMode iBkImgMode) { m_eBkImgMode = iBkImgMode; }
	EckInline constexpr BkImgMode GetBkImgMode() const { return m_eBkImgMode; }

	HBITMAP SetImage(HBITMAP hBitmap)
	{
		m_bPartMetricsDirty = TRUE;
		std::swap(m_hbmImg, hBitmap);
		return hBitmap;
	}
	EckInline constexpr HBITMAP GetImage() const { return m_hbmImg; }

	EckInline constexpr void SetAlign(BOOL bHAlign, Align eAlign)
	{
		(bHAlign ? m_eAlignH : m_eAlignV) = eAlign;
		m_bPartMetricsDirty = TRUE;
	}
	EckInline constexpr Align GetAlign(BOOL bHAlign) const
	{
		return (bHAlign ? m_eAlignH : m_eAlignV);
	}

	EckInline constexpr void SetAutoWrap(BOOL bAutoWrap)
	{
		m_bAutoWrap = bAutoWrap;
		m_bPartMetricsDirty = TRUE;
	}
	EckInline constexpr BOOL GetAutoWrap() const { return m_bAutoWrap; }

	EckInline constexpr void SetEllipsisMode(Ellipsis eEllipsisMode)
	{
		m_eEllipsisMode = eEllipsisMode;
		m_bPartMetricsDirty = TRUE;
	}
	EckInline constexpr Ellipsis GetEllipsisMode() const { return m_eEllipsisMode; }

	EckInline constexpr void SetPrefixMode(Prefix ePrefixMode)
	{
		m_ePrefixMode = ePrefixMode;
		m_bPartMetricsDirty = TRUE;
	}
	EckInline constexpr Prefix GetPrefixMode() const { return m_ePrefixMode; }

	void SetClr(ClrPart ePart, COLORREF cr)
	{
		switch (ePart)
		{
		case ClrPart::Text:
			m_crText = cr;
			SetTextColor(m_DC.GetDC(), cr);
			break;
		case ClrPart::Bk:
			m_crBK = cr;
			if (cr == CLR_DEFAULT)
				cr = GetSysColor(COLOR_BTNFACE);
			SetDCBrushColor(m_DC.GetDC(), cr);
			break;
		case ClrPart::TextBk:
			m_crTextBK = cr;
			if (cr == CLR_DEFAULT)
				SetBkMode(m_DC.GetDC(), TRANSPARENT);
			else
			{
				SetBkMode(m_DC.GetDC(), OPAQUE);
				SetBkColor(m_DC.GetDC(), cr);
			}
			break;
		default: ECK_UNREACHABLE;
		}

		Redraw();
	}

	constexpr COLORREF GetClr(ClrPart ePart) const
	{
		switch (ePart)
		{
		case ClrPart::Text: return m_crText;
		case ClrPart::Bk: return m_crBK;
		case ClrPart::TextBk: return m_crTextBK;
		}
		ECK_UNREACHABLE;
	}

	EckInline constexpr void SetGradientMode(GradientMode eMode) { m_eGradientMode = eMode; }
	EckInline constexpr GradientMode GetGradientMode() const { return m_eGradientMode; }

	EckInline constexpr void SetGradientClr(_In_range_(0, 2) int idx, COLORREF cr)
	{
		m_crGradient[idx] = cr;
	}
	EckInline constexpr COLORREF GetGradientClr(_In_range_(0, 2) int idx) const
	{
		return m_crGradient[idx];
	}

	EckInline void SetTransparent(BOOL bTransparent)
	{
		m_bTransparent = bTransparent;
		ModifyStyle(bTransparent ? WS_EX_TRANSPARENT : 0, WS_EX_TRANSPARENT, GWL_EXSTYLE);
	}
	EckInline constexpr BOOL GetTransparent() const { return m_bTransparent; }

	EckInline constexpr void SetFullWndImg(BOOL bFullWndPic) { m_bFullWndImg = bFullWndPic; }
	EckInline constexpr BOOL GetFullWndImg() const { return m_bFullWndImg; }

	EckInline constexpr void SetMouseOption(MouseOpt eOption) { m_eMouseOption = eOption; }
	EckInline constexpr MouseOpt GetMouseOption() const { return m_eMouseOption; }

	BOOL HitTest(POINT pt, LA_HITTEST& laht)
	{
		laht.bHit = FALSE;
		laht.bHitImg = FALSE;
		laht.bHitText = FALSE;
		if (PtInRect(RECT{ 0,0,m_cxClient,m_cyClient }, pt))
		{
			laht.bHit = TRUE;
			if (m_bPartMetricsDirty)
				CalcPartsRect();
			if (PtInRect(m_rcPartImg, pt))
				laht.bHitImg = TRUE;
			else if (PtInRect(m_rcPartText, pt))
				laht.bHitText = TRUE;
			return TRUE;
		}
		return FALSE;
	}
};
ECK_RTTI_IMPL_BASE_INLINE(CLabel, CWnd);
ECK_NAMESPACE_END