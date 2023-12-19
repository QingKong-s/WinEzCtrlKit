/*
* WinEzCtrlKit Library
*
* CLabel.h ： 标签
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "CWnd.h"
#include "CSubclassMgr.h"
#include "Utility.h"
#include "GraphicsHelper.h"

#include <windowsx.h>

ECK_NAMESPACE_BEGIN
struct ECKLABELDATA
{
	int iBKPicMode;			// 底图模式，0 - 居左上  1 - 平铺  2 - 居中  3 - 缩放
	int iAlignH;			// 横向对齐
	int iAlignV;			// 纵向对齐
	COLORREF crText;		// 文本颜色
	COLORREF crTextBK;		// 文本背景颜色
	COLORREF crBK;			// 背景颜色
	/*
	* 渐变背景模式，可选值：
	* 0 - 无  1 - 从上到下  2 - 从下到上  3 - 从左到右  4 - 从右到左
	* 5 - 从左上到右下  6 - 从右下到左上  7 - 从左下到右上  8 - 从右上到左下
	*/
	int iGradientMode;
	COLORREF crGradient[3];	// 渐变背景颜色
	int iEllipsisMode;		// 省略号模式，0 - 无  1 - 末尾省略  2 - 路径省略  3 - 省略单词
	int iPrefixMode;		// 前缀模式，0 - 常规  1 - 不解释前缀  2 - 隐藏下划线  3 - 只显示下划线
	int iMousePassingThrough;	// 鼠标穿透，0 - 无  1 - 穿透空白区域  2 - 穿透整个控件
	BITBOOL bAutoWrap : 1;		// 自动折行
	BITBOOL bFullWndPic : 1;	// 底图尽量充满控件
	BITBOOL bTransparent : 1;	// 透明标签
	BITBOOL bUxThemeText : 1;
};

class CLabel :public CWnd
{
private:
	ECKLABELDATA m_Info{};

	int m_cxClient = 0,
		m_cyClient = 0;			// 客户区大小
	HDC m_hCDC = NULL;			// 后台兼容DC
	HDC m_hcdcHelper = NULL;	// 画位图使用的辅助DC
	HBITMAP m_hBitmap = NULL;	// 后台兼容位图
	HGDIOBJ m_hOld1 = NULL;		// 后台DC旧位图句柄
	HGDIOBJ m_hOld2 = NULL;		// 辅助DC旧位图句柄

	HBITMAP m_hbmBK = NULL;		// 底图
	HBITMAP m_hbmPic = NULL;	// 图片
	int m_cxBKPic = 0,
		m_cyBKPic = 0;			// 底图大小

	int m_cxPic = 0,
		m_cyPic = 0;			// 图片大小
	RECT m_rcPartPic{};			// 缓存的图片矩形
	RECT m_rcPartText{};		// 缓存的文本矩形

	HFONT m_hFont = NULL;
	CRefStrW m_rsText{};

	static ATOM m_atomLabel;	// 标签类原子

	/// <summary>
	/// 绘制标签。
	/// 若m_Info.bTransparent为TRUE，则自动设置剪辑DC至控件矩形
	/// </summary>
	/// <param name="hDC">目标DC，调用之前属性必须设置完毕</param>
	void Paint(HDC hDC)
	{
		BLENDFUNCTION bf;
		bf.BlendOp = AC_SRC_OVER;
		bf.BlendFlags = 0;
		bf.SourceConstantAlpha = 255;
		bf.AlphaFormat = AC_SRC_ALPHA;
		//
		// 画背景
		//

		// 画纯色背景
		RECT rc{ 0,0,m_cxClient,m_cyClient };
		if (!m_Info.bTransparent)
			FillRect(hDC, &rc, (HBRUSH)GetStockObject(DC_BRUSH));
		else
			IntersectClipRect(hDC, 0, 0, m_cxClient, m_cyClient);
		// 类样式带CS_PARENTDC速度会快一点，并且按理来说应该手动剪辑子窗口，但是为什么不剪辑也没事。。。反正这里就这么写了吧，按规定来

		// 画渐变背景或底图
		if (m_Info.iGradientMode != 0)
			FillGradientRect(hDC, rc, m_Info.crGradient, m_Info.iGradientMode);
		else if (m_hbmBK)
		{
			SelectObject(m_hcdcHelper, m_hbmBK);
			DrawBackgroundImage32(hDC, m_hcdcHelper, rc, m_cxBKPic, m_cyBKPic,
				m_Info.iBKPicMode, m_Info.bFullWndPic);
		}
		//
		// 画文本
		//
		UINT uDTFlags = DT_NOCLIP | (m_Info.bAutoWrap ? DT_WORDBREAK : DT_SINGLELINE);
		switch (m_Info.iEllipsisMode)
		{
		case 0:uDTFlags |= DT_END_ELLIPSIS; break;
		case 1:uDTFlags |= DT_PATH_ELLIPSIS; break;
		case 2:uDTFlags |= DT_WORD_ELLIPSIS; break;
		default:assert(FALSE);
		}
		switch (m_Info.iPrefixMode)
		{
		case 0:uDTFlags |= DT_NOPREFIX; break;
		case 1:uDTFlags |= DT_HIDEPREFIX; break;
		case 2:uDTFlags |= DT_PREFIXONLY; break;
		default:assert(FALSE);
		}

		SelectObject(m_hcdcHelper, m_hbmPic);
		AlphaBlend(hDC, m_rcPartPic.left, m_rcPartPic.top, m_cxPic, m_cyPic, m_hcdcHelper, 0, 0, m_cxPic, m_cyPic, bf);
		rc = m_rcPartText;
		DrawTextW(hDC, m_rsText.Data(), -1, &rc, uDTFlags);
	}

	/// <summary>
	/// 计算部件矩形
	/// </summary>
	void CalcPartsRect()
	{
		RECT rc{ 0,0,m_cxClient - m_cxPic,m_cyClient };
		UINT uDTFlags = DT_NOCLIP | DT_CALCRECT;
		switch (m_Info.iEllipsisMode)
		{
		case 0:uDTFlags |= DT_END_ELLIPSIS; break;
		case 1:uDTFlags |= DT_PATH_ELLIPSIS; break;
		case 2:uDTFlags |= DT_WORD_ELLIPSIS; break;
		default:assert(FALSE);
		}
		switch (m_Info.iPrefixMode)
		{
		case 0:uDTFlags |= DT_NOPREFIX; break;
		case 1:uDTFlags |= DT_HIDEPREFIX; break;
		case 2:uDTFlags |= DT_PREFIXONLY; break;
		default:assert(FALSE);
		}

		int xPic, yPic;

		if (m_Info.bAutoWrap)
		{
			uDTFlags |= DT_WORDBREAK;
			DrawTextW(m_hCDC, m_rsText.Data(), -1, &rc, uDTFlags);

			int cyText = rc.bottom - rc.top;
			switch (m_Info.iAlignV)
			{
			case 0:// 上边
				rc.top = 0;
				rc.bottom = rc.top + cyText;
				yPic = rc.top;
				break;
			case 1:// 中间
				rc.top = (m_cyClient - cyText) / 2;
				rc.bottom = rc.top + cyText;
				yPic = (m_cyClient - m_cyPic) / 2;
				break;
			case 2:// 下边
				rc.bottom = m_cyClient;
				rc.top = rc.bottom - cyText;
				yPic = m_cyClient - m_cyPic;
				break;
			default:
				assert(FALSE);
			}
		}
		else
		{
			uDTFlags |= DT_SINGLELINE;
			DrawTextW(m_hCDC, m_rsText.Data(), -1, &rc, uDTFlags);

			int cyText = rc.bottom - rc.top;
			switch (m_Info.iAlignV)
			{
			case 0:// 上边
				rc.top = 0;
				rc.bottom = rc.top + cyText;
				yPic = rc.top;
				break;
			case 1:// 中间
				rc.top = (m_cyClient - cyText) / 2;
				rc.bottom = rc.top + cyText;
				yPic = (m_cyClient - m_cyPic) / 2;
				break;
			case 2:// 下边
				rc.bottom = m_cyClient;
				rc.top = rc.bottom - cyText;
				yPic = m_cyClient - m_cyPic;
				break;
			default:
				assert(FALSE);
			}
		}
		uDTFlags &= (~DT_CALCRECT);
		int cxText = rc.right - rc.left;
		int cxTotal = cxText + m_cxPic;
		switch (m_Info.iAlignH)
		{
		case 0:// 左边
			uDTFlags |= DT_LEFT;
			rc.left = m_cxPic;
			rc.right = rc.left + cxText;
			xPic = 0;
			break;
		case 1:// 中间
			rc.left = (m_cxClient - cxTotal) / 2 + m_cxPic;
			rc.right = rc.left + cxText;
			xPic = rc.left - m_cxPic;
			break;
		case 2:// 右边
			uDTFlags |= DT_RIGHT;
			rc.right = m_cxClient - m_cxPic;
			rc.left = rc.right - cxText;
			xPic = rc.right;
			break;
		default:
			assert(FALSE);
		}

		m_rcPartPic.left = xPic;
		m_rcPartPic.top = yPic;
		m_rcPartPic.right = m_rcPartPic.left + m_cxPic;
		m_rcPartPic.bottom = m_rcPartPic.top + m_cyPic;

		m_rcPartText = rc;
	}

	/// <summary>
	/// 置外部DC属性。
	/// 函数先使用SaveDC保存DC状态，然后根据控件属性设置DC
	/// </summary>
	/// <param name="hDC">目标DC</param>
	void SetDCAttr(HDC hDC)
	{
		SaveDC(hDC);
		///////////
		SelectObject(hDC, m_hFont);
		///////////
		SetTextColor(hDC, m_Info.crText);
		///////////
		if (m_Info.crBK == CLR_DEFAULT)
			m_Info.crBK = GetSysColor(COLOR_BTNFACE);
		SetDCBrushColor(hDC, m_Info.crBK);
		///////////
		if (m_Info.crTextBK == CLR_DEFAULT)
			SetBkMode(hDC, TRANSPARENT);
		else
		{
			SetBkMode(hDC, OPAQUE);
			SetBkColor(hDC, m_Info.crTextBK);
		}
	}
public:
	LRESULT CALLBACK OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_NCHITTEST:
		{
			if (m_Info.bTransparent)
				switch (m_Info.iMousePassingThrough)
				{
				case 0:// 无
					break;
				case 1:// 穿透空白区域
				{
					POINT pt{ GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam) };
					ScreenToClient(hWnd, &pt);
					if (!PtInRect(&m_rcPartPic, pt) && !PtInRect(&m_rcPartText, pt))
						return HTTRANSPARENT;
				}
				break;
				case 2:// 穿透整个控件
					return HTTRANSPARENT;
				}
		}
		break;

		case WM_WINDOWPOSCHANGED:
		{
			LRESULT lResult = DefWindowProcW(hWnd, uMsg, wParam, lParam);
			if (m_Info.bTransparent)
				Redraw();
			return lResult;
		}

		case WM_SIZE:
		{
			m_cxClient = LOWORD(lParam);
			m_cyClient = HIWORD(lParam);
			SelectObject(m_hCDC, m_hOld1);
			DeleteObject(m_hBitmap);
			HDC hDC = GetDC(hWnd);
			m_hBitmap = CreateCompatibleBitmap(hDC, m_cxClient, m_cyClient);
			SelectObject(m_hCDC, m_hBitmap);
			ReleaseDC(hWnd, hDC);
			CalcPartsRect();
			if (!m_Info.bTransparent)
				Redraw();
		}
		return 0;

		case WM_NCCREATE:
			SetWindowLongPtrW(hWnd, 0, (LONG_PTR)((CREATESTRUCTW*)lParam)->lpCreateParams);
			return TRUE;

		case WM_CREATE:
		{
			auto pcs = (CREATESTRUCTW*)lParam;
			m_rsText = pcs->lpszName;
			HDC hDC = GetDC(hWnd);
			m_hCDC = CreateCompatibleDC(hDC);
			m_hcdcHelper = CreateCompatibleDC(hDC);
			ReleaseDC(hWnd, hDC);
			m_hOld1 = GetCurrentObject(m_hCDC, OBJ_BITMAP);
			m_hOld2 = GetCurrentObject(m_hcdcHelper, OBJ_BITMAP);
			SetDCBrushColor(m_hCDC, GetSysColor(COLOR_BTNFACE));
		}
		return 0;

		case WM_DESTROY:
		{
			SelectObject(m_hCDC, m_hOld1);
			SelectObject(m_hcdcHelper, m_hOld2);
			DeleteDC(m_hCDC);
			DeleteDC(m_hcdcHelper);
			DeleteObject(m_hBitmap);
		}
		return 0;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);
			if (m_Info.bTransparent)
			{
				SetDCAttr(ps.hdc);
				Paint(ps.hdc);
				RestoreDC(ps.hdc, -1);
			}
			else
			{
				Paint(m_hCDC);
				BitBlt(ps.hdc,
					ps.rcPaint.left,
					ps.rcPaint.top,
					ps.rcPaint.right - ps.rcPaint.left,
					ps.rcPaint.bottom - ps.rcPaint.top,
					m_hCDC,
					ps.rcPaint.left,
					ps.rcPaint.top,
					SRCCOPY);
			}
			EndPaint(hWnd, &ps);
		}
		return 0;

		case WM_SETFONT:
		{
			m_hFont = (HFONT)wParam;
			SelectObject(m_hCDC, m_hFont);
			CalcPartsRect();
			Redraw();
		}
		break;

		case WM_SETTEXT:
			m_rsText = (PWSTR)lParam;
			return TRUE;
		case WM_GETTEXTLENGTH:
			return m_rsText.Size();
		case WM_GETTEXT:
			if (wParam > 0)
				return m_rsText.CopyTo((PWSTR)lParam, (int)wParam - 1);
			else
				return 0;
		case WM_GETDLGCODE:
			return DLGC_STATIC;
		}

		return CWnd::OnMsg(hWnd, uMsg, wParam, lParam);
	}

	static ATOM RegisterWndClass()
	{
		WNDCLASSW wc{};
		wc.cbWndExtra = sizeof(void*);
		wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
		wc.hInstance = eck::g_hInstance;
		wc.lpfnWndProc = DefWindowProcW;
		wc.lpszClassName = WCN_LABEL;
		wc.style = CS_DBLCLKS | CS_PARENTDC;
		return RegisterClassW(&wc);
	}

	ECK_CWND_CREATE
	{
		dwStyle |= WS_CHILD;
		m_hWnd = IntCreate(dwExStyle, WCN_LABEL, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, g_hInstance, this);
		SetClr(2, CLR_DEFAULT);
		return m_hWnd;
	}

	CLabel()
	{
		m_Info.iAlignV = 1;
		m_Info.crBK = CLR_DEFAULT;
		m_Info.crTextBK = CLR_DEFAULT;
		m_Info.crGradient[0] = 0x808080;
		m_Info.crGradient[1] = 0xFFFFFF;
		m_Info.crGradient[2] = 0x808080;
	}

	~CLabel()
	{
		DeleteObject(m_hbmBK);
	}

	void Redraw()
	{
		if (!m_hWnd)
			return;
		if (m_Info.bTransparent)
		{
			RECT rc{ 0,0,m_cxClient,m_cyClient };
			HWND hParent = GetParent(m_hWnd);
			MapWindowPoints(m_hWnd, hParent, (POINT*)&rc, 2);
			RedrawWindow(hParent, &rc, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW);
		}
		else
		{
			InvalidateRect(m_hWnd, NULL, FALSE);
			UpdateWindow(m_hWnd);
		}
	}

	/// <summary>
	/// 置背景图片
	/// </summary>
	/// <param name="hBitmap">位图句柄</param>
	HBITMAP SetBKPic(HBITMAP hBitmap)
	{
		HBITMAP hOld = m_hbmBK;
		m_hbmBK = hBitmap;
		BITMAP bitmap{};
		GetObjectW(m_hbmBK, sizeof(bitmap), &bitmap);
		m_cxBKPic = bitmap.bmWidth;
		m_cyBKPic = bitmap.bmHeight;
		Redraw();
		return hOld;
	}

	/// <summary>
	/// 取背景图片
	/// </summary>
	/// <returns>图片句柄</returns>
	EckInline HBITMAP GetBKPic()
	{
		return m_hbmBK;
	}

	/// <summary>
	/// 置图片
	/// </summary>
	/// <param name="hBitmap">位图句柄</param>
	HBITMAP SetPic(HBITMAP hBitmap)
	{
		HBITMAP hOld = m_hbmPic;
		m_hbmPic = hBitmap;
		BITMAP bitmap;
		GetObjectW(m_hbmPic, sizeof(bitmap), &bitmap);
		m_cxPic = bitmap.bmWidth;
		m_cyPic = bitmap.bmHeight;

		CalcPartsRect();
		Redraw();
		return hOld;
	}

	EckInline HBITMAP GetPic()
	{
		return m_hbmPic;
	}

	/// <summary>
	/// 置底图方式
	/// </summary>
	EckInline void SetBKPicMode(int iBKPicMode)
	{
		m_Info.iBKPicMode = iBKPicMode;
		Redraw();
	}

	EckInline int GetBKPicMode()
	{
		return m_Info.iBKPicMode;
	}

	/// <summary>
	/// 置对齐
	/// </summary>
	/// <param name="bHAlign">是否为横向对齐</param>
	/// <param name="iAlign">对齐方式</param>
	EckInline void SetAlign(BOOL bHAlign, int iAlign)
	{
		if (bHAlign)
			m_Info.iAlignH = iAlign;
		else
			m_Info.iAlignV = iAlign;
		CalcPartsRect();
		Redraw();
	}

	EckInline int GetAlign(BOOL bHAlign) const
	{
		if (bHAlign)
			return m_Info.iAlignH;
		else
			return m_Info.iAlignV;
	}

	/// <summary>
	/// 置自动换行
	/// </summary>
	/// <param name="bAutoWrap"></param>
	/// <returns></returns>
	EckInline void SetAutoWrap(BOOL bAutoWrap)
	{
		m_Info.bAutoWrap = bAutoWrap;
		CalcPartsRect();
		Redraw();
	}

	EckInline BOOL GetAutoWrap() const
	{
		return m_Info.bAutoWrap;
	}

	/// <summary>
	/// 置颜色
	/// </summary>
	/// <param name="idx">0 = 文本颜色  1 = 背景  2 = 文本背景</param>
	/// <param name="cr">颜色</param>
	void SetClr(int idx, COLORREF cr)
	{
		switch (idx)
		{
		case 0:
			m_Info.crText = cr;
			SetTextColor(m_hCDC, cr);
			break;
		case 1:
			m_Info.crBK = cr;
			if (cr == CLR_DEFAULT)
				cr = GetSysColor(COLOR_BTNFACE);
			SetDCBrushColor(m_hCDC, cr);
			break;
		case 2:
			m_Info.crTextBK = cr;
			if (cr == CLR_DEFAULT)
				SetBkMode(m_hCDC, TRANSPARENT);
			else
			{
				SetBkMode(m_hCDC, OPAQUE);
				SetBkColor(m_hCDC, cr);
			}
		}

		Redraw();
	}

	EckInline COLORREF GetClr(int idx) const
	{
		switch (idx)
		{
		case 0:return m_Info.crText;
		case 1:return m_Info.crBK;
		case 2:return m_Info.crTextBK;
		}
		assert(FALSE);
		return 0;
	}

	/// <summary>
	/// 置渐变方式
	/// </summary>
	EckInline void SetGradientMode(int iGradientMode)
	{
		m_Info.iGradientMode = iGradientMode;
		Redraw();
	}

	EckInline int GetGradientMode() const
	{
		return m_Info.iGradientMode;
	}

	/// <summary>
	/// 置渐变颜色
	/// </summary>
	EckInline void SetGradientClr(int idx, COLORREF cr)
	{
		m_Info.crGradient[idx] = cr;
		Redraw();
	}

	EckInline COLORREF GetGradientClr(int idx) const
	{
		return m_Info.crGradient[idx];
	}

	/// <summary>
	/// 置省略号模式
	/// </summary>
	/// <param name="iEllipsisMode"></param>
	/// <returns></returns>
	EckInline void SetEllipsisMode(int iEllipsisMode)
	{
		m_Info.iEllipsisMode = iEllipsisMode;
		CalcPartsRect();
		Redraw();
	}

	EckInline int GetEllipsisMode() const
	{
		return m_Info.iEllipsisMode;
	}

	/// <summary>
	/// 置前缀解释模式
	/// </summary>
	/// <param name="iPrefixMode"></param>
	/// <returns></returns>
	EckInline void SetPrefixMode(int iPrefixMode)
	{
		m_Info.iPrefixMode = iPrefixMode;
		CalcPartsRect();
		Redraw();
	}

	EckInline int GetPrefixMode() const
	{
		return m_Info.iPrefixMode;
	}

	/// <summary>
	/// 置底图充满窗口
	/// </summary>
	EckInline void SetFullWndPic(BOOL bFullWndPic)
	{
		m_Info.bFullWndPic = bFullWndPic;
		Redraw();
	}

	EckInline BOOL GetFullWndPic() const
	{
		return m_Info.bFullWndPic;
	}

	/// <summary>
	/// 置透明标签
	/// </summary>
	EckInline void SetTransparent(BOOL bTransparent)
	{
		m_Info.bTransparent = bTransparent;
		ModifyStyle(bTransparent ? WS_EX_TRANSPARENT : 0, WS_EX_TRANSPARENT, GWL_EXSTYLE);
		Redraw();
	}

	EckInline BOOL GetTransparent() const
	{
		return m_Info.bTransparent;
	}

	/// <summary>
	/// 置鼠标穿透
	/// </summary>
	/// <param name="iMousePassingThrough"></param>
	/// <returns></returns>
	EckInline void SetMousePassingThrough(int iMousePassingThrough)
	{
		m_Info.iMousePassingThrough = iMousePassingThrough;
		Redraw();
	}

	EckInline int GetMousePassingThrough() const
	{
		return m_Info.iMousePassingThrough;
	}
};
ECK_NAMESPACE_END