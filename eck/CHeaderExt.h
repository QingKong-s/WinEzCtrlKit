/*
* WinEzCtrlKit Library
*
* CHeaderExt.h ： 表头扩展
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "CHeader.h"
#include "CEditExt.h"

ECK_NAMESPACE_BEGIN
class CHeaderExt : public CHeader
{
public:
	ECK_RTTI(CHeaderExt);
private:
	struct ITEM
	{
		eck::CRefStrW rsMainText{};
		eck::CRefStrW rsSubText{};
		COLORREF crText{ CLR_DEFAULT };
		COLORREF crTextBk{ CLR_DEFAULT };
		COLORREF crTextSubText{ CLR_DEFAULT };
		COLORREF crTextBkSubText{ CLR_DEFAULT };
		COLORREF crTextFilter{ CLR_DEFAULT };
		COLORREF crTextBkFilter{ CLR_DEFAULT };
		COLORREF crBk{ CLR_DEFAULT };
		UINT uFmt{};// 缓存供命中测试修复用
	};

	// 信息
	int m_idxHotItem{ -1 };
	int m_cxIL{};
	int m_cyIL{};
	int m_idxFilterEditing{ -1 };
	HIMAGELIST m_hIL{};
	BITBOOL m_bCheckBoxes : 1{};
	BITBOOL m_bFilterBar : 1{};
	// 选项
	BITBOOL m_bSplitBtnHot : 1{};				// [内部标志]拆分按钮已点燃
	BITBOOL m_bFilterBtnHot : 1{};				// [内部标志]筛选按钮已点燃
	BITBOOL m_bAutoDarkMode : 1{ TRUE };		// 自动处理暗色
	BITBOOL m_bUseExtText : 1{};				// 使用扩展文本
	BITBOOL m_bRepairDbClick : 1{ TRUE };		// 连击修正
	BITBOOL m_bShowEmptyMainExtText : 1{ TRUE };// 即使主文本为空也不忽略其高度
	BYTE m_byColorAlpha{ 100 };					// 背景色透明度
	Align m_eExtAlignV{ Align::Center };		// 扩展文本垂直对齐
	COLORREF m_crText{ CLR_DEFAULT };			// 文本颜色
	COLORREF m_crBk{ CLR_DEFAULT };				// 背景色
	COLORREF m_crTextBk{ CLR_DEFAULT };			// 文本背景色
	COLORREF m_crTextSubText{ CLR_DEFAULT };	// [扩展文本]子文本颜色
	COLORREF m_crTextBkSubText{ CLR_DEFAULT };	// [扩展文本]子文本背景色
	COLORREF m_crTextFilter{ CLR_DEFAULT };		// 过滤器文本颜色
	COLORREF m_crTextBkFilter{ CLR_DEFAULT };	// 过滤器文本背景色
	// 图形
	CEzCDC m_DcAlpha{};
	HFONT m_hFontMainText{};
	HTHEME m_hTheme{};
	HTHEME m_hThemeCB{};
	int m_cxCheckBox{};
	int m_cxSplitBtn{};
	SIZE m_sizeSortUp{};
	SIZE m_sizeSortDown{};
	int m_cyMainText{};
	int m_cySubText{};
	COLORREF m_crSortUp{ CLR_DEFAULT };
	//
	std::vector<ITEM> m_vData{};
	const THREADCTX* m_pThrCtx{};
	int m_cxClient{};
	int m_cyClient{};

	CRefStrW m_rsTextBuf{ MAX_PATH };

	int m_iDpi{ USER_DEFAULT_SCREEN_DPI };
	int m_cxEdge{};
	int m_cyMenuCheck{};
	int m_cxBorder{};

	CEditExt* m_pEDFilter{};

	HMODULE m_hModComctl32{};
	CEzCDC m_DcFilterBmp{};
	SIZE m_sizeFilter{};
	CRefStrW m_rsEnterTextHere{};

	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY(cxFilterGlyph, 13)
		;
	ECK_DS_END_VAR(m_Ds);

	void PrepareTextColor(HDC hDC, const ITEM& e)
	{
		if (e.crTextBk != CLR_DEFAULT)
		{
			SetBkMode(hDC, OPAQUE);
			SetBkColor(hDC, e.crTextBk);
		}
		else if (m_crTextBk != CLR_DEFAULT)
		{
			SetBkMode(hDC, OPAQUE);
			SetBkColor(hDC, m_crTextBk);
		}
		else
			SetBkMode(hDC, TRANSPARENT);

		if (e.crText != CLR_DEFAULT)
			SetTextColor(hDC, e.crText);
		else if (m_crText != CLR_DEFAULT)
			SetTextColor(hDC, m_crText);
		else
			SetTextColor(hDC, m_pThrCtx->crDefText);
	}

	void PrepareTextColorSubText(HDC hDC, const ITEM& e)
	{
		if (e.crTextBkSubText != CLR_DEFAULT)
		{
			SetBkMode(hDC, OPAQUE);
			SetBkColor(hDC, e.crTextBkSubText);
		}
		else if (m_crTextBkSubText != CLR_DEFAULT)
		{
			SetBkMode(hDC, OPAQUE);
			SetBkColor(hDC, m_crTextBkSubText);
		}
		else
			SetBkMode(hDC, TRANSPARENT);

		if (e.crTextSubText != CLR_DEFAULT)
			SetTextColor(hDC, e.crTextSubText);
		else if (m_crTextSubText != CLR_DEFAULT)
			SetTextColor(hDC, m_crTextSubText);
		else
			SetTextColor(hDC, m_pThrCtx->crTip1);
	}

	void PrepareTextColorFilter(HDC hDC, const ITEM& e)
	{
		if (e.crTextBkFilter != CLR_DEFAULT)
		{
			SetBkMode(hDC, OPAQUE);
			SetBkColor(hDC, e.crTextBkFilter);
		}
		else if (m_crTextBkFilter != CLR_DEFAULT)
		{
			SetBkMode(hDC, OPAQUE);
			SetBkColor(hDC, m_crTextBkFilter);
		}
		else
			SetBkMode(hDC, TRANSPARENT);

		if (e.crTextFilter != CLR_DEFAULT)
			SetTextColor(hDC, e.crText);
		else if (m_crTextFilter != CLR_DEFAULT)
			SetTextColor(hDC, m_crTextFilter);
		else
			SetTextColor(hDC, m_pThrCtx->crDefText);
	}

	LRESULT OnItemPrePaint(NMCUSTOMDRAW* pnmcd)
	{
		//return CDRF_DODEFAULT;
		const int idx = (int)pnmcd->dwItemSpec;
		if (idx < 0 || idx >(int)m_vData.size())
			return CDRF_DODEFAULT;
		const auto& e = m_vData[idx];
		const auto hDC = pnmcd->hdc;
		RECT rc{ pnmcd->rc };
		if (m_bFilterBar)
			rc.bottom -= ((rc.bottom - rc.top) / 2);
		int xAvailable;

		HDITEMW hdi;
		hdi.mask = HDI_BITMAP | HDI_FORMAT | HDI_IMAGE | HDI_LPARAM |
			HDI_ORDER | HDI_WIDTH;
		if (!m_bUseExtText)
		{
			hdi.mask |= HDI_TEXT;
			hdi.pszText = m_rsTextBuf.Data();
			hdi.cchTextMax = m_rsTextBuf.Size();
		}
		GetItem(idx, &hdi);

		rc.left += m_cxEdge;
		// 画背景
		int iState;
		if (pnmcd->uItemState & CDIS_SELECTED)
			iState = HIS_PRESSED;
		else if (idx == m_idxHotItem)
			iState = HIS_HOT;
		else
			iState = HIS_NORMAL;
		DrawThemeBackground(m_hTheme, hDC, HP_HEADERITEM, iState, &pnmcd->rc, nullptr);
		COLORREF crBk;
		if (e.crBk != CLR_DEFAULT)
			crBk = e.crBk;
		else if (m_crBk != CLR_DEFAULT)
			crBk = m_crBk;
		else
			goto SkipColorBk;
		SetDCBrushColor(m_DcAlpha.GetDC(), crBk);
		constexpr static RECT rcColorAlpha{ 0,0,1,1 };
		FillRect(m_DcAlpha.GetDC(), &rcColorAlpha, GetStockBrush(DC_BRUSH));
		AlphaBlend(hDC, pnmcd->rc.left, pnmcd->rc.top,
			pnmcd->rc.right - pnmcd->rc.left, pnmcd->rc.bottom - pnmcd->rc.top,
			m_DcAlpha.GetDC(), 0, 0, 1, 1, { AC_SRC_OVER,0,m_byColorAlpha,0 });
	SkipColorBk:;
		// 画拆分按钮
		if (hdi.fmt & HDF_SPLITBUTTON)
		{
			if (idx == m_idxHotItem)
				if (m_bSplitBtnHot)
					iState = HDDS_HOT;
				else
					iState = HDDS_SOFTHOT;
			else
				iState = HDDS_NORMAL;
			RECT rc{ pnmcd->rc };
			if (m_bFilterBar)
				rc.bottom -= ((rc.bottom - rc.top) / 2);
			rc.left = rc.right - m_cxSplitBtn;
			xAvailable = rc.left;
			DrawThemeBackground(m_hTheme, hDC, HP_HEADERDROPDOWN, iState, &rc, nullptr);
		}
		else
			xAvailable = pnmcd->rc.right;
		// 画检查框
		if (m_bCheckBoxes && (hdi.fmt & HDF_CHECKBOX))
		{
			rc.right = rc.left + m_cxCheckBox;
			DrawThemeBackground(m_hThemeCB, hDC, BP_CHECKBOX,
				(hdi.fmt & HDF_CHECKED) ? CBS_CHECKEDNORMAL : CBS_UNCHECKEDNORMAL, &rc, nullptr);
			rc.left = rc.right + m_cxEdge;
		}
		// 画图像
		BITMAP bm{};
		int xRight{ xAvailable - m_cxEdge };
		BOOL bBmp = (hdi.fmt & HDF_BITMAP) && hdi.hbm;
		BOOL bImg = (hdi.fmt & HDF_IMAGE) && hdi.iImage >= 0 && m_hIL;
		if (bBmp || bImg)
		{
			const BOOL bRight = (hdi.fmt & HDF_BITMAP_ON_RIGHT);
			if (bRight)
				if (bBmp && bImg)
				{
					GetObjectW(hdi.hbm, sizeof(bm), &bm);
					xRight -= (m_cxIL + bm.bmWidth + m_cxEdge * 2);
				}
				else if (bBmp)
				{
					GetObjectW(hdi.hbm, sizeof(bm), &bm);
					xRight -= (bm.bmWidth + m_cxEdge);
				}
				else/* if (bImg)*/
					xRight -= (m_cxIL + m_cxEdge);
			else
				if (bBmp && bImg)// hbm绘制在右边，bRight对iImage生效
				{
					bImg = FALSE;
					rc.left += m_cxEdge;
					ImageList_Draw(m_hIL, hdi.iImage, hDC,
						rc.left, rc.top + (rc.bottom - rc.top - m_cyIL) / 2, ILD_NORMAL);
					rc.left += (m_cxIL + m_cxEdge);
					// 右边剩下hbm，此处应对称
					GetObjectW(hdi.hbm, sizeof(bm), &bm);
					//xRight -= (bm.bmWidth + m_cxEdge);
					xRight -= (rc.left - pnmcd->rc.left - m_cxEdge/*补回边界*/);
				}
				else if (bBmp)// bRight对hbm生效
				{
					bBmp = FALSE;
					rc.left += m_cxEdge;
					GetObjectW(hdi.hbm, sizeof(bm), &bm);
					const int y = rc.top + (rc.bottom - rc.top - bm.bmHeight) / 2;
					const auto hCDC = CreateCompatibleDC(hDC);
					SelectObject(hCDC, hdi.hbm);
					if (bm.bmBitsPixel == 32)
						AlphaBlend(hDC, rc.left, y, bm.bmWidth, bm.bmHeight,
							hCDC, 0, 0, bm.bmWidth, bm.bmHeight, BlendFuncAlpha);
					else
						BitBlt(hDC, rc.left, y, bm.bmWidth, bm.bmHeight,
							hCDC, 0, 0, SRCCOPY);
					DeleteDC(hCDC);
					rc.left += (bm.bmWidth + m_cxEdge);
				}
				else/* if (bImg)*/// bRight对iImage生效
				{
					bImg = FALSE;
					rc.left += m_cxEdge;
					ImageList_Draw(m_hIL, hdi.iImage, hDC,
						rc.left, rc.top + (rc.bottom - rc.top - m_cyIL) / 2, ILD_NORMAL);
					rc.left += (m_cxIL + m_cxEdge);
				}
		}
		// 画文本
		if (hdi.fmt & HDF_STRING)
		{
			UINT uDtFlags = DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_END_ELLIPSIS;
			if (hdi.fmt & HDF_RIGHT)
				uDtFlags |= DT_RIGHT;
			else if (hdi.fmt & HDF_CENTER)
				uDtFlags |= DT_CENTER;

			if (m_bUseExtText)
			{
				rc.right = xRight - m_cxEdge * 2;
				rc.left += (m_cxEdge * 2);
				if (!m_bShowEmptyMainExtText)
				{
					if (e.rsMainText.IsEmpty())
					{
						PrepareTextColorSubText(hDC, e);
						DrawTextW(hDC, e.rsSubText.Data(), e.rsSubText.Size(), &rc, uDtFlags);
					}
					else if (e.rsSubText.IsEmpty())
					{
						const auto hOld = SelectObject(hDC, m_hFontMainText);
						PrepareTextColor(hDC, e);
						DrawTextW(hDC, e.rsMainText.Data(), e.rsMainText.Size(), &rc, uDtFlags);
						SelectObject(hDC, hOld);
					}
					else
						goto NormalDraw2Row;
				}
				else
				{
				NormalDraw2Row:;
					const int y0Bak = rc.top;
					const int y1Bak = rc.bottom;
					int y;
					switch (m_eExtAlignV)
					{
					case Align::Near:
						y = rc.top + m_cyMainText + m_cxEdge;
						break;
					case Align::Center:
						y = rc.top + (rc.bottom - rc.top - (m_cyMainText + m_cxEdge + m_cySubText)) / 2;
						y += (m_cyMainText + m_cxEdge);
						break;
					case Align::Far:
						y = rc.bottom - m_cxEdge - m_cySubText;
						break;
					default: ECK_UNREACHABLE;
					}
					rc.top = y;
					rc.bottom = rc.top + m_cySubText;
					PrepareTextColorSubText(hDC, e);
					DrawTextW(hDC, e.rsSubText.Data(), e.rsSubText.Size(), &rc, uDtFlags);

					y -= (m_cyMainText + m_cxEdge);
					rc.top = y;
					rc.bottom = rc.top + m_cyMainText;
					const auto hOld = SelectObject(hDC, m_hFontMainText);
					PrepareTextColor(hDC, e);
					DrawTextW(hDC, e.rsMainText.Data(), e.rsMainText.Size(), &rc, uDtFlags);
					SelectObject(hDC, hOld);

					rc.top = y0Bak;
					rc.bottom = y1Bak;
				}
			}
			else
			{
				rc.right = xRight;
				PrepareTextColor(hDC, e);
				DrawTextW(hDC, hdi.pszText, -1, &rc, uDtFlags);
			}
			rc.left = rc.right + m_cxEdge;
		}
		// 画右边图像
		if (bImg)
		{
			ImageList_Draw(m_hIL, hdi.iImage, hDC,
				rc.left, rc.top + (rc.bottom - rc.top - m_cyIL) / 2, ILD_NORMAL);
			rc.left += (m_cxIL + m_cxEdge);
		}
		if (bBmp)
		{
			const int y = rc.top + (rc.bottom - rc.top - bm.bmHeight) / 2;
			const auto hCDC = CreateCompatibleDC(hDC);
			SelectObject(hCDC, hdi.hbm);
			if (bm.bmBitsPixel == 32)
				AlphaBlend(hDC, rc.left, y, bm.bmWidth, bm.bmHeight,
					hCDC, 0, 0, bm.bmWidth, bm.bmHeight, BlendFuncAlpha);
			else
				BitBlt(hDC, rc.left, y, bm.bmWidth, bm.bmHeight,
					hCDC, 0, 0, SRCCOPY);
			DeleteDC(hCDC);
			rc.left += (bm.bmWidth + m_cxEdge);
		}
		// 画排序标记
		if (hdi.fmt & HDF_SORTUP)
		{
			rc.left = ((pnmcd->rc.right - pnmcd->rc.left) - m_sizeSortUp.cx) / 2;
			rc.top = 0;
			rc.right = rc.left + m_sizeSortUp.cx;
			rc.bottom = rc.top + m_sizeSortUp.cy;
			DrawThemeBackground(m_hTheme, hDC, HP_HEADERSORTARROW, HSAS_SORTEDUP, &rc, nullptr);
		}
		else if (hdi.fmt & HDF_SORTDOWN)
		{
			rc.left = ((pnmcd->rc.right - pnmcd->rc.left) - m_sizeSortDown.cx) / 2;
			rc.top = 0;
			rc.right = rc.left + m_sizeSortDown.cx;
			rc.bottom = rc.top + m_sizeSortDown.cy;
			DrawThemeBackground(m_hTheme, hDC, HP_HEADERSORTARROW, HSAS_SORTEDDOWN, &rc, NULL);
		}
		// 画过滤器
		if (m_bFilterBar)
		{
			rc.left = pnmcd->rc.left + m_cxEdge * 3;
			rc.right = xAvailable - m_Ds.cxFilterGlyph - m_cxBorder * 4;
			OffsetRect(rc, 0, rc.bottom - rc.top);
			if (m_idxFilterEditing != idx)
			{
				HEADER_FILTER Val;
				Val.str.pszText = m_rsTextBuf.Data();
				Val.str.cchTextMax = m_rsTextBuf.Size();
				hdi.pvFilter = nullptr;
				hdi.mask = HDI_FILTER;
				hdi.type = HDFT_HASNOVALUE;
				GetItem(idx, &hdi);
				if (hdi.type == HDFT_ISSTRING)
				{
					hdi.pvFilter = &Val.str;
					GetItem(idx, &hdi);
					PrepareTextColorFilter(hDC, e);
					DrawTextW(hDC, Val.str.pszText, -1, &rc,
						DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_END_ELLIPSIS);
					goto SkipFilterText;
				}
				else if (hdi.type == HDFT_ISDATE)
				{
					hdi.pvFilter = &Val.st;
					GetItem(idx, &hdi);
					const int cchDate = GetDateFormatEx(LOCALE_NAME_USER_DEFAULT, 0, &Val.st,
						nullptr, nullptr, 0, nullptr);
					if (m_rsTextBuf.Size() < cchDate)
						m_rsTextBuf.ReSize(cchDate);
					GetDateFormatEx(LOCALE_NAME_USER_DEFAULT, 0, &Val.st,
						nullptr, m_rsTextBuf.Data(), cchDate, nullptr);
					PrepareTextColorFilter(hDC, e);
					DrawTextW(hDC, m_rsTextBuf.Data(), cchDate, &rc,
						DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_END_ELLIPSIS);
					goto SkipFilterText;
				}
				else if (hdi.type == HDFT_ISNUMBER)
				{
					hdi.pvFilter = &Val.i;
					GetItem(idx, &hdi);
					if (m_rsTextBuf.Size() < CchI32ToStrBufNoRadix2)
						m_rsTextBuf.ReSize(CchI32ToStrBufNoRadix2);
					const auto cch = _swprintf(m_rsTextBuf.Data(), L"%d", Val.i);
					PrepareTextColorFilter(hDC, e);
					DrawTextW(hDC, m_rsTextBuf.Data(), cch, &rc,
						DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_END_ELLIPSIS);
					goto SkipFilterText;
				}
				SetTextColor(hDC, m_pThrCtx->crGray1);
				SetBkMode(hDC, TRANSPARENT);
				DrawTextW(hDC, m_rsEnterTextHere.Data(), m_rsEnterTextHere.Size(), &rc,
					DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_END_ELLIPSIS);
			SkipFilterText:;
			}
			rc.right = pnmcd->rc.right;
			rc.left = rc.right - ((hdi.fmt & HDF_SPLITBUTTON) ?
				m_cxSplitBtn : m_Ds.cxFilterGlyph);// 还是对齐吧...
			if (m_idxFilterEditing == idx)
			{
				if (m_idxHotItem == idx)
					if (m_bFilterBtnHot)
						iState = HDDFS_HOT;
					else
						iState = HDDFS_SOFTHOT;
				else
					iState = HDDFS_NORMAL;
				DrawThemeBackground(m_hTheme, hDC,
					HP_HEADERDROPDOWNFILTER, iState, &rc, nullptr);
			}
			else
			{
				rc.left += ((rc.right - rc.left - m_sizeFilter.cx) / 2);
				rc.top += ((rc.bottom - rc.top - m_sizeFilter.cy) / 2);
				TransparentBlt(hDC, rc.left, rc.top, m_sizeFilter.cx, m_sizeFilter.cy,
					m_DcFilterBmp.GetDC(), 0, 0, m_sizeFilter.cx, m_sizeFilter.cy, RGB(128, 0, 0));
			}
		}
		return CDRF_SKIPDEFAULT;
	}

	void UpdateStyleOptions(DWORD dwStyle)
	{
		m_bCheckBoxes = IsBitSet(dwStyle, HDS_CHECKBOXES);
		m_bFilterBar = IsBitSet(dwStyle, HDS_FILTERBAR);
		if (m_bFilterBar && !m_hModComctl32)
		{
			m_hModComctl32 = GetModuleHandleW(L"comctl32.dll");
			m_DcFilterBmp.m_hBmp = (HBITMAP)LoadImageW(m_hModComctl32, MAKEINTRESOURCEW(140),
				IMAGE_BITMAP, 0, 0, LR_SHARED);
			m_DcFilterBmp.m_hCDC = CreateCompatibleDC(nullptr);
			m_DcFilterBmp.m_hOld = SelectObject(m_DcFilterBmp.m_hCDC, m_DcFilterBmp.m_hBmp);
			BITMAP bm;
			GetObjectW(m_DcFilterBmp.m_hBmp, sizeof(bm), &bm);
			m_sizeFilter = { bm.bmWidth,bm.bmHeight };

			const auto hRes = FindResourceExW(m_hModComctl32, RT_STRING,
				MAKEINTRESOURCEW(1 + 0x1050 / 16), LANGIDFROMLCID(GetThreadLocale()));
			const auto hgStr = LoadResource(m_hModComctl32, hRes);
			auto pszBegin = (PCWSTR)LockResource(hgStr);
			for (size_t i = 0; i < 0x1050 % 16; ++i)
				pszBegin += (*pszBegin + 1);
			const int cch = *pszBegin++;
			m_rsEnterTextHere.DupString(pszBegin, cch);
		}
	}

	void UpdateThemeMetrics()
	{
		SIZE size;
		const HDC hDC = GetDC(HWnd);
		GetThemePartSize(m_hThemeCB, hDC, BP_CHECKBOX, CBS_UNCHECKEDNORMAL,
			nullptr, TS_DRAW, &size);
		m_cxCheckBox = size.cx;
		GetThemePartSize(m_hTheme, hDC, HP_HEADERSORTARROW, HSAS_SORTEDUP,
			nullptr, TS_DRAW, &m_sizeSortUp);
		GetThemePartSize(m_hTheme, hDC, HP_HEADERSORTARROW, HSAS_SORTEDDOWN,
			nullptr, TS_DRAW, &m_sizeSortUp);
		GetThemePartSize(m_hTheme, hDC, HP_HEADERDROPDOWN, 0,
			nullptr, TS_TRUE, &size);
		if (size.cx > m_cyMenuCheck)// Windows uses it.
			m_cxSplitBtn = size.cx;
		else
			m_cxSplitBtn = m_cyMenuCheck;
		ReleaseDC(HWnd, hDC);
	}

	void InitForNewWindow(HWND hWnd)
	{
		m_iDpi = GetDpi(hWnd);
		UpdateDpiSize(m_Ds, m_iDpi);
		m_cxEdge = DaGetSystemMetrics(SM_CXEDGE, m_iDpi);
		m_cyMenuCheck = DaGetSystemMetrics(SM_CYMENUCHECK, m_iDpi);
		m_cxBorder = DaGetSystemMetrics(SM_CXBORDER, m_iDpi);

		m_DcAlpha.Create32(hWnd, 1, 1);
		m_pThrCtx = GetThreadCtx();
		m_hTheme = OpenThemeData(hWnd, L"Header");
		m_hThemeCB = OpenThemeData(hWnd, L"Button");
		UpdateThemeMetrics();

		UpdateStyleOptions(Style);

		m_vData.clear();
		if (const int cCol = GetItemCount(); cCol)
			m_vData.resize(cCol);
	}

	void CleanupForDestroyWindow()
	{
		CloseThemeData(m_hTheme);
		m_hTheme = nullptr;
		CloseThemeData(m_hThemeCB);
		m_hThemeCB = nullptr;
		m_crBk = m_crText = m_crTextBk = CLR_DEFAULT;
		m_hIL = nullptr;
		m_cxIL = m_cyIL = 0;
		m_idxHotItem = m_idxFilterEditing = -1;
		m_bAutoDarkMode = m_bRepairDbClick = m_bShowEmptyMainExtText = TRUE;
		m_bUseExtText = FALSE;
		m_eExtAlignV = Align::Center;
		m_crText = m_crBk = m_crTextBk = m_crTextSubText = m_crTextBkSubText =
			m_crTextFilter = m_crTextBkFilter = CLR_DEFAULT;
		m_hFontMainText = nullptr;
	}
public:
	ECKPROP(HeGetBkColor, HeSetBkColor)						COLORREF BkColor;
	ECKPROP(HeGetTextColor, HeSetTextColor)					COLORREF TextColor;
	ECKPROP(HeGetTextBkColor, HeSetTextBkColor)				COLORREF TextBkColor;
	ECKPROP(HeGetSubTextColor, HeSetSubTextColor)			COLORREF SubTextColor;
	ECKPROP(HeGetSubTextBkColor, HeSetSubTextBkColor)		COLORREF SubTextBkColor;
	ECKPROP(HeGetFilterTextColor, HeSetFilterTextColor)		COLORREF FilterTextColor;
	ECKPROP(HeGetFilterTextBkColor, HeSetFilterTextBkColor) COLORREF FilterTextBkColor;
	ECKPROP(HeGetMainTextFont, HeSetMainTextFont)			HFONT MainTextFont;
	ECKPROP(HeGetExtTextAlignV, HeSetExtTextAlignV)			Align ExtTextAlignV;
	ECKPROP(HeGetAutoDarkMode, HeSetAutoDarkMode)			BOOL AutoDarkMode;
	ECKPROP(HeGetUseExtText, HeSetUseExtText)				BOOL UseExtText;
	ECKPROP(HeGetRepairDoubleClick, HeSetRepairDoubleClick) BOOL RepairDbClick;
	ECKPROP(HeGetShowEmptyMainExtText, HeSetShowEmptyMainExtText) BOOL ShowEmptyMainExtText;
	ECKPROP(HeGetBkColorAlpha, HeSetBkColorAlpha)			BYTE BkColorAlpha;
	ECKPROP_R(HeGetCurrEditingItem)		int CurrEditingItem;
	ECKPROP_R(HeGetFilterEdit)			CEditExt* FilterEdit;

	~CHeaderExt()
	{
		delete m_pEDFilter;
	}

	void AttachNew(HWND hWnd) override
	{
		CWnd::AttachNew(hWnd);
		InitForNewWindow(hWnd);
	}

	void DetachNew() override
	{
		CWnd::DetachNew();
		CleanupForDestroyWindow();
	}

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_LBUTTONDBLCLK:
			if (m_bRepairDbClick)
			{
				HDHITTESTINFO hdhti;
				hdhti.pt = ECK_GET_PT_LPARAM(lParam);
				hdhti.iItem = -1;
				HitTest(&hdhti);
				if (hdhti.iItem >= 0 && !(hdhti.flags & HHT_ONDIVIDER))
					PostMsg(WM_LBUTTONDOWN, wParam, lParam);
			}
			break;

		case WM_MOUSEMOVE:
		{
			HDHITTESTINFO hdhti;
			hdhti.pt = ECK_GET_PT_LPARAM(lParam);
			hdhti.iItem = -1;
			HitTest(&hdhti);

			m_idxHotItem = hdhti.iItem;
			const auto b = !!(hdhti.flags & HHT_ONDROPDOWN);
			const auto b2 = !!(hdhti.flags & HHT_ONFILTERBUTTON);
			if (b != m_bSplitBtnHot)
			{
				m_bSplitBtnHot = b;
				if (m_idxHotItem >= 0)
				{
					RECT rc;
					GetItemRect(m_idxHotItem, &rc);
					InvalidateRect(hWnd, &rc, FALSE);
				}
			}
			if (b2 != m_bFilterBtnHot)
			{
				m_bFilterBtnHot = b2;
				if (m_idxHotItem >= 0)
				{
					RECT rc;
					GetItemRect(m_idxHotItem, &rc);
					InvalidateRect(hWnd, &rc, FALSE);
				}
			}

			//TRACKMOUSEEVENT tme;
			//tme.cbSize = sizeof(tme);
			//tme.dwFlags = TME_LEAVE;
			//tme.hwndTrack = hWnd;
			//TrackMouseEvent(&tme);
		}
		break;

		case WM_MOUSELEAVE:
			m_idxHotItem = -1;
			m_bSplitBtnHot = FALSE;
			break;

		case WM_LBUTTONUP:
		{
			// 先call默认过程，因为控件内部可能要重排列
			LRESULT lResult = CHeader::OnMsg(hWnd, uMsg, wParam, lParam);

			HDHITTESTINFO hdhti;
			hdhti.pt = ECK_GET_PT_LPARAM(lParam);
			hdhti.iItem = -1;
			CHeader::OnMsg(hWnd, HDM_HITTEST, 0, (LPARAM)&hdhti);
			m_idxHotItem = hdhti.iItem;

			return lResult;
		}
		break;

		case WM_SIZE:
			m_cxClient = LOWORD(lParam);
			m_cyClient = HIWORD(lParam);
			break;

		case WM_PARENTNOTIFY:
		{
			if (LOWORD(wParam) == WM_CREATE)
			{
				WCHAR szCls[ARRAYSIZE(WC_HEADERW) + 1];
				szCls[0] = L'\0';
				GetClassNameW(HWND(lParam), szCls, ARRAYSIZE(szCls));
				if (wcscmp(szCls, WC_EDITW) == 0)
				{
					const auto lResult = CHeader::OnMsg(hWnd, uMsg, wParam, lParam);
					if (!m_pEDFilter)
						m_pEDFilter = new CEditExt{};
					m_pEDFilter->AttachNew(HWND(lParam));
					m_pEDFilter->SetFrameType(5);
					RECT rc;
					GetClientRect(m_pEDFilter->HWnd, &rc);
					rc.left = rc.right / 2;
					rc.top = rc.bottom / 2;
					MapWindowPoints(HWND(lParam), hWnd, (POINT*)&rc, 1);
					HDHITTESTINFO hdhti;
					hdhti.pt = *(POINT*)&rc;
					hdhti.iItem = -1;
					HitTest(&hdhti);
					m_idxFilterEditing = hdhti.iItem;
					return lResult;
				}
			}
		}
		break;

		case WM_STYLECHANGED:
		{
			const auto* const p = (STYLESTRUCT*)lParam;
			if (wParam == GWL_STYLE)
				UpdateStyleOptions(p->styleNew);
		}
		break;

		case WM_THEMECHANGED:
		{
			CloseThemeData(m_hTheme);
			m_hTheme = OpenThemeData(hWnd, L"Header");
			CloseThemeData(m_hThemeCB);
			m_hThemeCB = OpenThemeData(hWnd, L"Button");
			UpdateThemeMetrics();
		}
		break;

		case WM_SETFONT:
		{
			const auto hDC = m_DcAlpha.GetDC();
			const auto hOld = SelectObject(hDC, (HFONT)wParam);
			TEXTMETRICW tm;
			GetTextMetricsW(hDC, &tm);
			m_cySubText = tm.tmHeight;
			SelectObject(hDC, hOld);
		}
		break;

		case WM_DPICHANGED_BEFOREPARENT:
		{
			m_iDpi = GetDpi(hWnd);
			UpdateDpiSize(m_Ds, m_iDpi);
			m_cxEdge = DaGetSystemMetrics(SM_CXEDGE, m_iDpi);
			m_cyMenuCheck = DaGetSystemMetrics(SM_CYMENUCHECK, m_iDpi);
			m_cxBorder = DaGetSystemMetrics(SM_CXBORDER, m_iDpi);
		}
		break;

		case WM_CREATE:
		{
			const auto lResult = CHeader::OnMsg(hWnd, uMsg, wParam, lParam);
			InitForNewWindow(hWnd);
			return lResult;
		}
		break;

		case WM_DESTROY:
		{
			CleanupForDestroyWindow();
		}
		break;

		case HDM_HITTEST:
		{
			const auto lResult = CHeader::OnMsg(hWnd, uMsg, wParam, lParam);
			if (lResult >= 0)
			{
				const auto p = (HDHITTESTINFO*)lParam;
				// !!!标准控件Bug：自定义绘制时懒加载数据未正确计算，导致拆分按钮命中测试不正常
				if (m_vData[p->iItem].uFmt & HDF_SPLITBUTTON)
				{
					RECT rc;
					GetItemRect(p->iItem, &rc);
					rc.left = rc.right - m_cxSplitBtn;
					if (m_bSplitBtnHot)
						rc.bottom -= ((rc.bottom - rc.top) / 2);
					if (PtInRect(rc, p->pt))
						p->flags |= HHT_ONDROPDOWN;
				}
			}
			return lResult;
		}
		break;

		case HDM_INSERTITEMW:
		{
			const auto lResult = CHeader::OnMsg(hWnd, uMsg, wParam, lParam);
			if (lResult >= 0)
			{
				const auto it = m_vData.emplace(m_vData.begin() + lResult);
				const auto* const p = (HDITEMW*)lParam;
				if (p->mask & HDI_FORMAT)
					it->uFmt = p->fmt;
			}
			return lResult;
		}
		break;

		case HDM_SETITEMW:
		{
			const auto lResult = CHeader::OnMsg(hWnd, uMsg, wParam, lParam);
			if (lResult >= 0)
			{
				const auto* const p = (HDITEMW*)lParam;
				if (p->mask & HDI_FORMAT)
					m_vData[wParam].uFmt = p->fmt;
			}
			return lResult;
		}
		break;

		case HDM_DELETEITEM:
		{
			const auto lResult = CHeader::OnMsg(hWnd, uMsg, wParam, lParam);
			if (lResult)
				m_vData.erase(m_vData.begin() + wParam);
			return lResult;
		}
		break;

		case HDM_SETIMAGELIST:
		{
			const auto lResult = CHeader::OnMsg(hWnd, uMsg, wParam, lParam);
			if (wParam == HDSIL_NORMAL)
			{
				m_hIL = (HIMAGELIST)lParam;
				ImageList_GetIconSize(m_hIL, &m_cxIL, &m_cyIL);
			}
			return lResult;
		}
		break;
		}
		return CHeader::OnMsg(hWnd, uMsg, wParam, lParam);
	}

	LRESULT OnNotifyMsg(HWND hParent, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bProcessed) override
	{
		switch (uMsg)
		{
		case WM_NOTIFY:
		{
			switch (((NMHDR*)lParam)->code)
			{
			case NM_CUSTOMDRAW:
			{
				bProcessed = TRUE;
				const auto pnmcd = (NMCUSTOMDRAW*)lParam;
				switch (pnmcd->dwDrawStage)
				{
				case CDDS_PREPAINT:
					return CDRF_NOTIFYITEMDRAW;
				case CDDS_ITEMPREPAINT:
					return OnItemPrePaint(pnmcd);
				}
			}
			return CDRF_DODEFAULT;

			// HDN_BEGINFILTEREDIT无法正常使用。。。
			case HDN_ENDFILTEREDIT:
			{
				int idx{ -1 };
				std::swap(idx, m_idxFilterEditing);
				if (idx >= 0)// 非常申必的重画问题
				{
					RECT rc;
					GetItemRect(idx, &rc);
					InvalidateRect(HWnd, &rc, FALSE);
				}
			}
			break;
			}
		}
		break;
		}
		return CHeader::OnNotifyMsg(hParent, uMsg, wParam, lParam, bProcessed);
	}

	EckInline constexpr void HeSetBkColor(COLORREF cr) { m_crBk = cr; }
	EckInline constexpr COLORREF HeGetBkColor() const { return m_crBk; }

	EckInline constexpr void HeSetTextBkColor(COLORREF cr) { m_crTextBk = cr; }
	EckInline constexpr COLORREF HeGetTextBkColor() const { return m_crTextBk; }

	EckInline constexpr void HeSetTextColor(COLORREF cr) { m_crText = cr; }
	EckInline constexpr COLORREF HeGetTextColor() const { return m_crText; }

	EckInline constexpr void HeSetSubTextColor(COLORREF cr) { m_crTextSubText = cr; }
	EckInline constexpr COLORREF HeGetSubTextColor() const { return m_crTextSubText; }

	EckInline constexpr void HeSetSubTextBkColor(COLORREF cr) { m_crTextBkSubText = cr; }
	EckInline constexpr COLORREF HeGetSubTextBkColor() const { return m_crTextBkSubText; }

	EckInline constexpr void HeSetFilterTextColor(COLORREF cr) { m_crTextFilter = cr; }
	EckInline constexpr COLORREF HeGetFilterTextColor() const { return m_crTextFilter; }

	EckInline constexpr void HeSetFilterTextBkColor(COLORREF cr) { m_crTextBkFilter = cr; }
	EckInline constexpr COLORREF HeGetFilterTextBkColor() const { return m_crTextBkFilter; }

	auto& HeGetItemData(int idx) { return m_vData[idx]; }
	const auto& HeGetItemData(int idx) const { return m_vData[idx]; }

	void HeSetMainTextFont(HFONT hFont)
	{
		m_hFontMainText = hFont;
		const auto hDC = m_DcAlpha.GetDC();
		const auto hOld = SelectObject(hDC, hFont);
		TEXTMETRICW tm;
		GetTextMetricsW(hDC, &tm);
		m_cyMainText = tm.tmHeight;
		SelectObject(hDC, hOld);
	}

	EckInline constexpr HFONT HeGetMainTextFont() const { return m_hFontMainText; }

	EckInline constexpr void HeSetAutoDarkMode(BOOL b) { m_bAutoDarkMode = b; }
	EckInline constexpr BOOL HeGetAutoDarkMode() const { return m_bAutoDarkMode; }

	EckInline constexpr void HeSetUseExtText(BOOL b) { m_bUseExtText = b; }
	EckInline constexpr BOOL HeGetUseExtText() const { return m_bUseExtText; }

	EckInline constexpr void HeSetRepairDoubleClick(BOOL b) { m_bRepairDbClick = b; }
	EckInline constexpr BOOL HeGetRepairDoubleClick() const { return m_bRepairDbClick; }

	EckInline constexpr void HeSetShowEmptyMainExtText(BOOL b) { m_bShowEmptyMainExtText = b; }
	EckInline constexpr BOOL HeGetShowEmptyMainExtText() const { return m_bShowEmptyMainExtText; }

	EckInline constexpr void HeSetBkColorAlpha(BYTE by) { m_byColorAlpha = by; }
	EckInline constexpr BYTE HeGetBkColorAlpha() const { return m_byColorAlpha; }

	EckInline constexpr void HeSetExtTextAlignV(Align e) { m_eExtAlignV = e; }
	EckInline constexpr Align HeGetExtTextAlignV() const { return m_eExtAlignV; }

	// 取过滤器当前编辑项
	EckInline constexpr int HeGetCurrEditingItem() const { return m_idxFilterEditing; }

	// 取过滤器编辑控件
	EckInline constexpr CEditExt* HeGetFilterEdit() const { return m_pEDFilter; }

	void HeSetTextBufferSize(int cch) { m_rsTextBuf.ReSize(cch); }
	EckInline constexpr int HeGetTextBufferSize() const { return m_rsTextBuf.Size(); }
};
ECK_RTTI_IMPL_BASE_INLINE(CHeaderExt, CHeader);
ECK_NAMESPACE_END