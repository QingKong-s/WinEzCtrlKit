#include "CLabel.h"

ECK_NAMESPACE_BEGIN
WND_RECORDER_INIT(CLabel)
ATOM CLabel::m_atomLabel = 0;


void CLabel::Paint(HDC hDC)
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
	{
		if (m_Info.iGradientMode <= 4 && m_Info.iGradientMode >= 1)
		{
			TRIVERTEX tv[4];
			COLORREF cr1, cr2, cr3;
			ULONG uMode;
			switch (m_Info.iGradientMode)
			{
			case 1:// 从上到下
			case 2:// 从下到上
				cr2 = m_Info.crGradient[1];
				tv[0].x = 0;
				tv[0].y = 0;
				tv[1].x = m_cxClient;
				tv[1].y = m_cyClient / 2;
				tv[2].x = 0;
				tv[2].y = m_cyClient / 2;
				tv[3].x = m_cxClient;
				tv[3].y = m_cyClient;
				uMode = GRADIENT_FILL_RECT_V;
				if (m_Info.iGradientMode == 1)
				{
					cr1 = m_Info.crGradient[0];
					cr3 = m_Info.crGradient[2];
				}
				else
				{
					cr1 = m_Info.crGradient[2];
					cr3 = m_Info.crGradient[0];
				}
				break;
			case 3:// 从左到右
			case 4:// 从右到左
				cr2 = m_Info.crGradient[1];
				tv[0].x = 0;
				tv[0].y = 0;
				tv[1].x = m_cxClient / 2;
				tv[1].y = m_cyClient;
				tv[2].x = m_cxClient / 2;
				tv[2].y = 0;
				tv[3].x = m_cxClient;
				tv[3].y = m_cyClient;
				uMode = GRADIENT_FILL_RECT_H;
				if (m_Info.iGradientMode == 3)
				{
					cr1 = m_Info.crGradient[0];
					cr3 = m_Info.crGradient[2];
				}
				else
				{
					cr1 = m_Info.crGradient[2];
					cr3 = m_Info.crGradient[0];
				}
				break;
			}

			tv[0].Red = GetRValue(cr1) << 8;
			tv[0].Green = GetGValue(cr1) << 8;
			tv[0].Blue = GetBValue(cr1) << 8;
			tv[0].Alpha = 0xFF << 8;

			tv[1].Red = GetRValue(cr2) << 8;
			tv[1].Green = GetGValue(cr2) << 8;
			tv[1].Blue = GetBValue(cr2) << 8;
			tv[1].Alpha = 0xFF << 8;

			tv[2].Red = tv[1].Red;
			tv[2].Green = tv[1].Green;
			tv[2].Blue = tv[1].Blue;
			tv[2].Alpha = 0xFF << 8;

			tv[3].Red = GetRValue(cr3) << 8;
			tv[3].Green = GetGValue(cr3) << 8;
			tv[3].Blue = GetBValue(cr3) << 8;
			tv[3].Alpha = 0xFF << 8;

			GRADIENT_RECT gr[2];
			gr[0].UpperLeft = 0;
			gr[0].LowerRight = 1;
			gr[1].UpperLeft = 2;
			gr[1].LowerRight = 3;
			GdiGradientFill(hDC, tv, ARRAYSIZE(tv), &gr, ARRAYSIZE(gr), uMode);
		}
		else
		{
			TRIVERTEX tv[4];
			// 左上
			tv[0].x = 0;
			tv[0].y = 0;
			// 左下
			tv[1].x = 0;
			tv[1].y = m_cyClient;
			// 右上
			tv[2].x = m_cxClient;
			tv[2].y = 0;
			// 右下
			tv[3].x = m_cxClient;
			tv[3].y = m_cyClient;
			COLORREF cr1, cr2, cr3;

			GRADIENT_TRIANGLE gt[2];
			switch (m_Info.iGradientMode)
			{
			case 5:// 左上到右下↘
			case 6:// 右下到左上↖
				gt[0].Vertex1 = 0;
				gt[0].Vertex2 = 1;
				gt[0].Vertex3 = 2;
				gt[1].Vertex1 = 3;
				gt[1].Vertex2 = 1;
				gt[1].Vertex3 = 2;
				cr2 = m_Info.crGradient[1];
				if (m_Info.iGradientMode == 5)
				{
					cr1 = m_Info.crGradient[0];
					cr3 = m_Info.crGradient[2];
				}
				else
				{
					cr1 = m_Info.crGradient[2];
					cr3 = m_Info.crGradient[0];
				}

				tv[0].Red = GetRValue(cr1) << 8;
				tv[0].Green = GetGValue(cr1) << 8;
				tv[0].Blue = GetBValue(cr1) << 8;
				tv[0].Alpha = 0xFF << 8;

				tv[1].Red = GetRValue(cr2) << 8;
				tv[1].Green = GetGValue(cr2) << 8;
				tv[1].Blue = GetBValue(cr2) << 8;
				tv[1].Alpha = 0xFF << 8;

				tv[2].Red = tv[1].Red;
				tv[2].Green = tv[1].Green;
				tv[2].Blue = tv[1].Blue;
				tv[2].Alpha = 0xFF << 8;

				tv[3].Red = GetRValue(cr3) << 8;
				tv[3].Green = GetGValue(cr3) << 8;
				tv[3].Blue = GetBValue(cr3) << 8;
				tv[3].Alpha = 0xFF << 8;
				break;
			case 7:// 左下到右上↗
			case 8:// 右上到左下↙
				gt[0].Vertex1 = 1;
				gt[0].Vertex2 = 0;
				gt[0].Vertex3 = 3;
				gt[1].Vertex1 = 2;
				gt[1].Vertex2 = 0;
				gt[1].Vertex3 = 3;
				cr2 = m_Info.crGradient[1];
				if (m_Info.iGradientMode == 7)
				{
					cr1 = m_Info.crGradient[0];
					cr3 = m_Info.crGradient[2];
				}
				else
				{
					cr1 = m_Info.crGradient[2];
					cr3 = m_Info.crGradient[0];
				}

				tv[0].Red = GetRValue(cr2) << 8;
				tv[0].Green = GetGValue(cr2) << 8;
				tv[0].Blue = GetBValue(cr2) << 8;
				tv[0].Alpha = 0xFF << 8;

				tv[1].Red = GetRValue(cr1) << 8;
				tv[1].Green = GetGValue(cr1) << 8;
				tv[1].Blue = GetBValue(cr1) << 8;
				tv[1].Alpha = 0xFF << 8;

				tv[3].Red = tv[0].Red;
				tv[3].Green = tv[0].Green;
				tv[3].Blue = tv[0].Blue;
				tv[3].Alpha = 0xFF << 8;

				tv[2].Red = GetRValue(cr3) << 8;
				tv[2].Green = GetGValue(cr3) << 8;
				tv[2].Blue = GetBValue(cr3) << 8;
				tv[2].Alpha = 0xFF << 8;
				break;
			}

			GdiGradientFill(hDC, tv, ARRAYSIZE(tv), gt, ARRAYSIZE(gt), GRADIENT_FILL_TRIANGLE);
		}
	}
	else if (m_hbmBK)
	{
		SelectObject(m_hcdcHelper, m_hbmBK);
		switch (m_Info.iBKPicMode)
		{
		case 0:// 居左上
			if (m_Info.bFullWndPic)
			{
				if (!m_cyBKPic || !m_cxBKPic)
					break;
				int cxRgn, cyRgn;

				cxRgn = m_cyClient * m_cxBKPic / m_cyBKPic;
				if (cxRgn < m_cxClient)// 先尝试y对齐，看x方向是否充满窗口
				{
					cxRgn = m_cxClient;
					cyRgn = m_cxClient * m_cyBKPic / m_cxBKPic;
				}
				else
					cyRgn = m_cyClient;

				GdiAlphaBlend(hDC, 0, 0, cxRgn, cyRgn, m_hcdcHelper, 0, 0, m_cxBKPic, m_cyBKPic, bf);
			}
			else
				GdiAlphaBlend(hDC, 0, 0, m_cxBKPic, m_cyBKPic, m_hcdcHelper, 0, 0, m_cxBKPic, m_cyBKPic, bf);
			break;
		case 1:// 平铺
			for (int i = 0; i < (m_cxClient - 1) / m_cxBKPic + 1; ++i)
				for (int j = 0; j < (m_cyClient - 1) / m_cyBKPic + 1; ++j)
					GdiAlphaBlend(hDC, i * m_cxBKPic, j * m_cyBKPic, m_cxBKPic, m_cyBKPic,
						m_hcdcHelper, 0, 0, m_cxBKPic, m_cyBKPic, bf);
			break;
		case 2:// 居中
			if (m_Info.bFullWndPic)
			{
				if (!m_cyBKPic || !m_cxBKPic)
					break;
				int cxRgn, cyRgn;
				int x, y;

				cxRgn = m_cyClient * m_cxBKPic / m_cyBKPic;
				if (cxRgn < m_cxClient)// 先尝试y对齐，看x方向是否充满窗口
				{
					cxRgn = m_cxClient;
					cyRgn = m_cxClient * m_cyBKPic / m_cxBKPic;
					x = 0;
					y = (m_cyClient - cyRgn) / 2;
				}
				else
				{
					cyRgn = m_cyClient;
					x = (m_cxClient - cxRgn) / 2;
					y = 0;
				}

				GdiAlphaBlend(hDC, x, y, cxRgn, cyRgn, m_hcdcHelper, 0, 0, m_cxBKPic, m_cyBKPic, bf);
			}
			else
				GdiAlphaBlend(hDC, (m_cxClient - m_cxBKPic) / 2, (m_cyClient - m_cyBKPic) / 2, m_cxBKPic, m_cyBKPic,
					m_hcdcHelper, 0, 0, m_cxBKPic, m_cyBKPic, bf);
			break;
		case 3:// 缩放
			GdiAlphaBlend(hDC, 0, 0, m_cxClient, m_cyClient, m_hcdcHelper, 0, 0, m_cxBKPic, m_cyBKPic, bf);
			break;
		}
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
	GdiAlphaBlend(hDC, m_rcPartPic.left, m_rcPartPic.top, m_cxPic, m_cyPic, m_hcdcHelper, 0, 0, m_cxPic, m_cyPic, bf);
	rc = m_rcPartText;
	DrawTextW(hDC, m_rsText, -1, &rc, uDTFlags);
}

void CLabel::CalcPartsRect()
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
		DrawTextW(m_hCDC, m_rsText, -1, &rc, uDTFlags);

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
		DrawTextW(m_hCDC, m_rsText, -1, &rc, uDTFlags);

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

void CLabel::SetDCAttr(HDC hDC)
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

LRESULT CALLBACK CLabel::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	auto p = (CLabel*)GetWindowLongPtrW(hWnd, 0);

	switch (uMsg)
	{
	case WM_NCHITTEST:
	{
		if (p->m_Info.bTransparent)
			switch (p->m_Info.iMousePassingThrough)
			{
			case 0:// 无
				break;
			case 1:// 穿透空白区域
			{
				POINT pt{ GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam) };
				ScreenToClient(hWnd, &pt);
				if (!PtInRect(&p->m_rcPartPic, pt) && !PtInRect(&p->m_rcPartText, pt))
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
		if (p->m_Info.bTransparent)
			p->Redraw();
		return lResult;
	}

	case WM_SIZE:
	{
		p->m_cxClient = LOWORD(lParam);
		p->m_cyClient = HIWORD(lParam);
		SelectObject(p->m_hCDC, p->m_hOld1);
		DeleteObject(p->m_hBitmap);
		HDC hDC = GetDC(hWnd);
		p->m_hBitmap = CreateCompatibleBitmap(hDC, p->m_cxClient, p->m_cyClient);
		SelectObject(p->m_hCDC, p->m_hBitmap);
		ReleaseDC(hWnd, hDC);
		p->CalcPartsRect();
		if (!p->m_Info.bTransparent)
			p->Redraw();
	}
	return 0;

	case WM_NCCREATE:
		SetWindowLongPtrW(hWnd, 0, (LONG_PTR)((CREATESTRUCTW*)lParam)->lpCreateParams);
		return TRUE;

	case WM_CREATE:
	{
		p->OnOwnWndMsg(hWnd, uMsg, wParam, lParam);
		HDC hDC = GetDC(hWnd);
		p->m_hCDC = CreateCompatibleDC(hDC);
		p->m_hcdcHelper = CreateCompatibleDC(hDC);
		ReleaseDC(hWnd, hDC);
		p->m_hOld1 = GetCurrentObject(p->m_hCDC, OBJ_BITMAP);
		p->m_hOld2 = GetCurrentObject(p->m_hcdcHelper, OBJ_BITMAP);
		SetDCBrushColor(p->m_hCDC, GetSysColor(COLOR_BTNFACE));
	}
	return 0;

	case WM_DESTROY:
	{
		m_Recorder.DeleteRecord(hWnd);
		SelectObject(p->m_hCDC, p->m_hOld1);
		SelectObject(p->m_hcdcHelper, p->m_hOld2);
		DeleteDC(p->m_hCDC);
		DeleteDC(p->m_hcdcHelper);
		DeleteObject(p->m_hBitmap);
	}
	return 0;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		BeginPaint(hWnd, &ps);
		if (p->m_Info.bTransparent)
		{
			p->SetDCAttr(ps.hdc);
			p->Paint(ps.hdc);
			RestoreDC(ps.hdc, -1);
		}
		else
		{
			p->Paint(p->m_hCDC);
			BitBlt(ps.hdc,
				ps.rcPaint.left,
				ps.rcPaint.top,
				ps.rcPaint.right - ps.rcPaint.left,
				ps.rcPaint.bottom - ps.rcPaint.top,
				p->m_hCDC,
				ps.rcPaint.left,
				ps.rcPaint.top,
				SRCCOPY);
		}
		EndPaint(hWnd, &ps);
	}
	return 0;

	case WM_SETFONT:
	{
		p->OnOwnWndMsg(hWnd, uMsg, wParam, lParam);
		SelectObject(p->m_hCDC, (HFONT)wParam);
		p->CalcPartsRect();
		p->Redraw();
	}
	break;

	case WM_SETTEXT:
	{
		p->OnOwnWndMsg(hWnd, uMsg, wParam, lParam);
		LRESULT lResult = DefWindowProcW(hWnd, uMsg, wParam, lParam);

		p->CalcPartsRect();
		p->Redraw();
		return lResult;
	}
	}

	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

ATOM CLabel::RegisterWndClass(HINSTANCE hInstance)
{
	if (m_atomLabel)
		return m_atomLabel;
	WNDCLASSW wc{};
	wc.cbWndExtra = sizeof(void*);
	wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wc.hInstance = hInstance;
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = WCN_LABEL;
	wc.style = CS_DBLCLKS | CS_PARENTDC;
	m_atomLabel = RegisterClassW(&wc);
	return m_atomLabel;
}

void CLabel::Redraw()
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

HBITMAP CLabel::SetBKPic(HBITMAP hBitmap)
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

HBITMAP CLabel::SetPic(HBITMAP hBitmap)
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

void CLabel::SetClr(int idx, COLORREF cr)
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
ECK_NAMESPACE_END