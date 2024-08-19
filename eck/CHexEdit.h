#pragma once
#include "CWnd.h"
#include "GraphicsHelper.h"

ECK_NAMESPACE_BEGIN
#pragma pack(push, ECK_CTRLDATA_ALIGN)
struct CTRLDATA_HEXEDIT
{
	int cCol;
	int cColPerGroup;

	int cxGap;
	int cyGap;

	BITBOOL bReadOnly : 1;
	BITBOOL bShowAddress : 1;
	BITBOOL bShowChar : 1;
	BITBOOL bShowHeader : 1;
	BITBOOL bHexAddress : 1;
	BITBOOL bCaretInFirstNum : 1;

	COLORREF crOddCol;
	COLORREF crEvenCol;
	COLORREF crAddressCol;

	UINT cCharCol;

	int idxCaretCol{};// 若为负值则显示在数据区，否则指定字符区的某列索引
	size_t posCaret{};
	size_t posSelStart{};
	size_t posSelEnd{};
};
#pragma pack(pop)

struct HEHITTEST
{
	POINT pt;			// 测试点
	union
	{
		struct
		{
			BITBOOL bHitData : 1;		// 命中数据区
			BITBOOL bHitAddress : 1;	// 命中地址
			BITBOOL bHitHeader : 1;		// 命中表头
			BITBOOL bHitChar : 1;		// 命中字符区
			BITBOOL bFirstNumber : 1;	// 是否命中左边一位数字，仅命中数据区时有效
			BITBOOL bHitContent : 1;	// 命中了内容而不是空白
		};
		UINT uFlags;
	};
	int idxCol;			// 命中的列索引，仅命中数据区或字符区时有效
	int idxRowInView;	// 命中的行索引，相对显示的第一行
	int idxCharCol;		// 命中的字符区栏索引，仅命中字符区时有效
};

class CHexEdit : public CWnd
{
private:
	struct CHAR_COL
	{
		UINT uCp;
		COLORREF crCol;
		int cbMaxChar;
		int x;
		CRefStrW rsName;
		BYTE LeadByteRange[MAX_LEADBYTES];
		int xOrg;
		PCBYTE pNeedle;
	};

	PCBYTE m_pData{};
	SIZE_T m_cbData{};
	SIZE_T m_posFirstVisible{};

	std::vector<CHAR_COL> m_vCharCol{};
	CEzCDC m_DC{};
	HFONT m_hFont{};

	int m_cxChar{};
	int m_cyChar{};

	int m_cxAddress{};
	int m_cxData{};
	int m_cxContent{};

	int m_cxClient{},
		m_cyClient{};

	int m_cRow{};

	int m_cScrollLine{ 3 };
	int m_cCharPreScrollH{ 3 };

	BITBOOL m_bLBtnDown : 1 = FALSE;
	BITBOOL m_bFocus : 1 = FALSE;

	int m_iDpi{ USER_DEFAULT_SCREEN_DPI };

	SIZE_T m_posDragSelStart{ SIZETMax };

	CTRLDATA_HEXEDIT D{};

	void UpdateFontMetrics()
	{
		const HDC hDC = m_DC.GetDC();
		SIZE size;
		GetTextExtentPoint32W(hDC, L"0", 1, &size);
		m_cxChar = size.cx;
		m_cyChar = size.cy;
		UpdateDataMetrics();
		UpdateScrollBar();
	}

	void UpdateDataMetrics()
	{
		m_cRow = (int)DivUpper(m_cbData, D.cCol);
		WCHAR szBuf[CchI64ToStrBufNoRadix2];
		m_cxAddress = swprintf(szBuf, (D.bHexAddress ? L"%08Ix" : L"%08Iu"), m_cbData) * m_cxChar + D.cxGap * 2;
		m_cxData = (m_cxChar * 2 + D.cxGap) * D.cCol;
		m_cxContent = m_cxAddress + m_cxData + D.cxGap + int(m_vCharCol.size() * (GetCharColumnWidth() + D.cxGap));
	}

	void UpdateScrollBar()
	{
		if (!m_cyChar)
			return;
		SCROLLINFO si;
		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE | SIF_PAGE;
		si.nMin = 0;
		si.nMax = m_cRow;
		si.nPage = (m_cyClient - m_cyChar - D.cyGap) / (m_cyChar + D.cyGap);
		SetSbInfo(SB_VERT, &si);

		si.fMask = SIF_RANGE | SIF_PAGE;
		si.nMin = 0;
		si.nMax = m_cxContent;
		si.nPage = m_cxClient;
		SetSbInfo(SB_HORZ, &si);
	}

	void ReCalcFirstVisible(int posSb)
	{
		m_posFirstVisible = posSb * D.cCol;
	}

	void UpdateSystemConfig()
	{
		SystemParametersInfoW(SPI_GETWHEELSCROLLLINES, 0, &m_cScrollLine, 0);
		SystemParametersInfoW(SPI_GETWHEELSCROLLCHARS, 0, &m_cCharPreScrollH, 0);
	}

	void UpdateDCAttributes()
	{
		SetBkMode(m_DC.GetDC(), TRANSPARENT);
		SelectObject(m_DC.GetDC(), m_hFont);
	}

	void OnPaint(HDC hDC, const RECT& rcPaint)
	{
		WCHAR szBuf[CchI64ToStrBufNoRadix2];
		// 画列头
		int xStart = -GetSbPos(SB_HORZ);

		SetDCBrushColor(hDC, GetThreadCtx()->crDefBkg);
		FillRect(hDC, &rcPaint, GetStockBrush(DC_BRUSH));
		RECT rcT{ xStart + D.cxGap, 0, xStart + m_cxAddress, m_cyChar };
		DrawTextW(hDC, L"Offset", -1, &rcT,
			DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);

		const int xData = xStart + m_cxAddress;
		int x = xData;
		EckCounter(D.cCol, i)
		{
			swprintf(szBuf, L"% 2X", (UINT)i);
			TextOutW(hDC, x, 0, szBuf, 2);
			x += GetColumnWidth();
		}

		x = xData + GetDataRegionWidth();
		const int cxCharCol = GetCharColumnWidth();
		RECT rcCharCol{ x,0,x + cxCharCol,m_cyChar };
		for (auto& e : m_vCharCol)
		{
			e.xOrg = rcCharCol.left;
			rcCharCol.right -= D.cxGap;
			DrawTextW(hDC, e.rsName.Data(), e.rsName.Size(), &rcCharCol,
				DT_SINGLELINE | DT_RIGHT | DT_VCENTER | DT_NOPREFIX);
			rcCharCol.right += D.cxGap;
			OffsetRect(rcCharCol, cxCharCol, 0);
		}

		// 画数据
		if (m_pData)
		{
			WCHAR szCharW[2]{};
			const PCWSTR pszFmtAddress = (D.bHexAddress ? L"%08Ix" : L"%08Iu");
			x = xData;
			const int idxLineBeginInView = std::max(0L, (rcPaint.top - GetHeaderHeight()) / GetRowHeight());
			PCBYTE pData = m_pData + m_posFirstVisible + idxLineBeginInView * D.cCol;
			if (idxLineBeginInView + GetFirstVisibleRow() >= m_cRow)
				goto EndDrawData;
			OffsetRect(rcT, 0, GetHeaderHeight() + idxLineBeginInView * GetRowHeight());
			for (int i = idxLineBeginInView; i < m_cRow; ++i)
			{
				for (auto& e : m_vCharCol)
				{
					e.x = e.xOrg;
					e.pNeedle = pData;
				}
				const int cch = swprintf(szBuf, pszFmtAddress, (UINT)(i * D.cCol + m_posFirstVisible));
				DrawTextW(hDC, szBuf, cch, &rcT, DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_NOPREFIX);
				EckCounter(D.cCol, j)
				{
					if (pData >= m_pData + m_cbData)
						goto EndDrawData;
					swprintf(szBuf, L"%02X", (UINT)*pData);
					TextOutW(hDC, x, rcT.top, szBuf, 2);
					x += (m_cxChar * 2 + D.cxGap);

					for (auto& e : m_vCharCol)
					{
						if (e.pNeedle != pData)
							continue;
						if (e.uCp == CP_UTF16LE)
						{
							if ((e.pNeedle + 1) < m_pData + m_cbData)
							{
								szCharW[0] = *(WCHAR*)e.pNeedle;
								TextOutW(hDC, e.x, rcT.top, szCharW, 1);
							}
							e.x += (m_cxChar * 2);
							e.pNeedle += 2;
						}
						else if (e.uCp == CP_UTF16BE)
						{
							if ((e.pNeedle + 1) < m_pData + m_cbData)
							{
								szCharW[0] = ReverseInteger(*(WCHAR*)e.pNeedle);
								TextOutW(hDC, e.x, rcT.top, szCharW, 1);
							}
							e.x += (m_cxChar * 2);
							e.pNeedle += 2;
						}
						else if (e.uCp == CP_UTF8)
						{
							e.x += m_cxChar;
							e.pNeedle++;
						}
						else if (e.cbMaxChar == 1)
						{
							if (MultiByteToWideChar(e.uCp, 0, (PCSTR)e.pNeedle, 1, szCharW, 1))
							{
								if (!iswprint(szCharW[0]))
									szCharW[0] = L'.';
								TextOutW(hDC, e.x, rcT.top, szCharW, 1);
							}
							e.x += m_cxChar;
							++e.pNeedle;
						}
						else if (e.cbMaxChar == 2)
						{
							const BYTE by = *e.pNeedle;
							BOOL bDbcsLeadByte{};
							for (int i = 0; i < 10; i += 2)
							{
								if (!e.LeadByteRange[i])
									break;
								if (by >= e.LeadByteRange[i] && by < e.LeadByteRange[i + 1])
								{
									bDbcsLeadByte = TRUE;
									break;
								}
							}

							if (bDbcsLeadByte)
							{
								if ((e.pNeedle + 1) < m_pData + m_cbData)
								{
									if (MultiByteToWideChar(e.uCp, 0, (PCSTR)e.pNeedle, 2, szCharW, 1))
									{
										if (!iswprint(szCharW[0]))
											szCharW[0] = L'.';
										TextOutW(hDC, e.x, rcT.top, szCharW, 1);
									}
								}
								else
									TextOutW(hDC, e.x, rcT.top, L".", 1);
								e.x += (m_cxChar * 2);
								e.pNeedle += 2;
							}
							else
							{
								if (MultiByteToWideChar(e.uCp, 0, (PCSTR)e.pNeedle, 1, szCharW, 1))
								{
									if (!iswprint(szCharW[0]))
										szCharW[0] = L'.';
									TextOutW(hDC, e.x, rcT.top, szCharW, 1);
								}
								e.x += m_cxChar;
								++e.pNeedle;
							}
						}
					}

					++pData;
				}

				x = xData;
				OffsetRect(rcT, 0, m_cyChar + D.cyGap);

				if (rcT.top >= rcPaint.bottom)
					break;
			}
		EndDrawData:;
		}

		// 画分隔线
		// 列头分隔线
		MoveToEx(hDC, 0, m_cyChar, NULL);
		LineTo(hDC, xStart + m_cxContent, m_cyChar);
		// 偏移分隔线
		x = xStart + m_cxAddress - D.cxGap / 2;
		MoveToEx(hDC, x, 0, NULL);
		LineTo(hDC, x, m_cyClient);
		// 数据分组
		if (D.cColPerGroup)
		{
			for (int i = 0; i < D.cCol - D.cColPerGroup; i += D.cColPerGroup)
			{
				x += (m_cxChar * 2 + D.cxGap) * D.cColPerGroup;
				MoveToEx(hDC, x, 0, NULL);
				LineTo(hDC, x, m_cyClient);
			}
		}
		x = xData +
			(m_cxChar * 2 + D.cxGap) * D.cCol - D.cxGap / 2;
		MoveToEx(hDC, x, 0, NULL);
		LineTo(hDC, x, m_cyClient);
	}

	void UpdateCaretPos()
	{
		ShowCaret(HWnd);
		const int idxV = D.posCaret / D.cCol - GetSbPos(SB_VERT);
		const int idxH = D.posCaret % D.cCol;
		if(D.idxCaretCol < 0)
		{
			SetCaretPos(
				-GetSbPos(SB_HORZ) + m_cxAddress + GetColumnWidth() * idxH + (D.bCaretInFirstNum ? 0 : m_cxChar),
				GetHeaderHeight() + idxV * GetRowHeight());
		}
		else
		{
			SetCaretPos(
				-GetSbPos(SB_HORZ) + m_cxAddress + m_cxData + m_cxChar * idxH +
				GetCharColumnWidth() * D.idxCaretCol,
				GetHeaderHeight() + idxV * GetRowHeight());
		}
	}
public:
	CHexEdit()
	{
		D.crOddCol = D.crEvenCol = D.crAddressCol = CLR_DEFAULT;
		D.bShowAddress = D.bShowChar = D.bShowHeader = TRUE;
		D.cCol = 16;
		D.cColPerGroup = 4;
		D.cxGap = 20;
		D.cyGap = 10;
	}

	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = NULL) override
	{
		return IntCreate(dwExStyle, WCN_HEXEDIT, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, g_hInstance, this);
	}

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_CREATE:
		{
			m_DC.Create(hWnd);
			UpdateDCAttributes();
			UpdateSystemConfig();
		}
		break;

		case WM_PAINT:
		{
			HideCaret(hWnd);
			PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);
			OnPaint(m_DC.GetDC(), ps.rcPaint);
			BitBltPs(&ps, m_DC.GetDC());
			EndPaint(hWnd, &ps);
			ShowCaret(hWnd);
		}
		break;

		case WM_LBUTTONDOWN:
		{
			m_bLBtnDown = TRUE;
			SetFocus(hWnd);
			SetCapture(hWnd);
			HEHITTEST heht;
			heht.pt = ECK_GET_PT_LPARAM(lParam);
			const auto pos = HitTest(heht);
			if ((heht.bHitData || heht.bHitChar))
			{
				m_posDragSelStart = pos;
				D.posCaret = pos;
				D.idxCaretCol = (heht.bHitChar ? heht.idxCharCol : -1);
				D.bCaretInFirstNum = (heht.bHitData ? heht.bFirstNumber : FALSE);
				UpdateCaretPos();
			}

			

			//EckDbgPrintFmt(L"HitTest: pos = %Id, idxRow = %d, idxCol = %d, idxCharCol = %d\n"
			//	L"bHitContent = %d",
			//	pos, heht.idxRowInView, heht.idxCol, heht.idxCharCol,
			//	(BOOL)heht.bHitContent);
		}
		break;

		case WM_MOUSEMOVE:
		{
			if (m_bLBtnDown)
			{
				HEHITTEST heht;
				heht.pt = ECK_GET_PT_LPARAM(lParam);
				const auto pos = HitTest(heht);
				if ((heht.bHitData || heht.bHitChar) && heht.bHitContent)
				{

				}
			}
		}
		break;

		case WM_LBUTTONUP:
		{
			if (m_bLBtnDown)
			{
				m_bLBtnDown = FALSE;
				ReleaseCapture();
			}
		}
		break;

		case WM_CAPTURECHANGED:
		{
			if (m_bLBtnDown)
			{
				m_bLBtnDown = FALSE;
			}
		}
		break;

		case WM_MOUSEWHEEL:
		{
			if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
				goto MouseWhellH;
			const int d = -GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
			if (d)
			{
				const int cScroll = std::min(std::max(GetVisibleRowCount(), 1), m_cScrollLine);
				ScrollV(d * cScroll);
			}
		}
		return 0;

		case WM_MOUSEHWHEEL:
		{
		MouseWhellH:
			const int d = -GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
			if (d)
			{
				const int t = std::min(m_cCharPreScrollH * m_cxChar * d, m_cxClient);
				ScrollH(t);
			}
		}
		return 0;

		case WM_VSCROLL:
		{
			SCROLLINFO si;
			si.cbSize = sizeof(si);
			si.fMask = SIF_ALL;
			GetSbInfo(SB_VERT, &si);
			const int iOld = si.nPos;
			switch (LOWORD(wParam))
			{
			case SB_TOP:
				si.nPos = si.nMin;
				break;
			case SB_BOTTOM:
				si.nPos = si.nMax;
				break;
			case SB_LINEUP:
				--si.nPos;
				break;
			case SB_LINEDOWN:
				++si.nPos;
				break;
			case SB_PAGEUP:
				si.nPos -= si.nPage;
				break;
			case SB_PAGEDOWN:
				si.nPos += si.nPage;
				break;
			case SB_THUMBTRACK:
				si.nPos = si.nTrackPos;
				break;
			}

			si.fMask = SIF_POS;
			SetSbInfo(SB_VERT, &si);
			GetSbInfo(SB_VERT, &si);
			ReCalcFirstVisible(si.nPos);
			RECT rc{ 0,m_cyChar + D.cyGap,m_cxClient,m_cyClient };
			ScrollWindowEx(hWnd, 0, (iOld - si.nPos) * (m_cyChar + D.cyGap), &rc, &rc, NULL, NULL, SW_INVALIDATE);
			UpdateWindow(hWnd);
			UpdateCaretPos();
		}
		return 0;

		case WM_HSCROLL:
		{
			SCROLLINFO si;
			si.cbSize = sizeof(si);
			si.fMask = SIF_ALL;
			GetSbInfo(SB_HORZ, &si);
			const int iOld = si.nPos;
			switch (LOWORD(wParam))
			{
			case SB_TOP:
				si.nPos = si.nMin;
				break;
			case SB_BOTTOM:
				si.nPos = si.nMax;
				break;
			case SB_LINEUP:
				si.nPos -= (m_cCharPreScrollH * m_cxChar);
				break;
			case SB_LINEDOWN:
				si.nPos += (m_cCharPreScrollH * m_cxChar);
				break;
			case SB_PAGEUP:
				si.nPos -= si.nPage;
				break;
			case SB_PAGEDOWN:
				si.nPos += si.nPage;
				break;
			case SB_THUMBTRACK:
				si.nPos = si.nTrackPos;
				break;
			}

			si.fMask = SIF_POS;
			SetSbInfo(SB_HORZ, &si);
			GetSbInfo(SB_HORZ, &si);
			ScrollWindowEx(hWnd, iOld - si.nPos, 0,
				NULL, NULL, NULL, NULL, SW_INVALIDATE);
			UpdateWindow(hWnd);
			UpdateCaretPos();
		}
		return 0;

		case WM_SIZE:
		{
			ECK_GET_SIZE_LPARAM(m_cxClient, m_cyClient, lParam);
			m_DC.ReSize(hWnd, m_cxClient, m_cyClient);
			UpdateDCAttributes();
			UpdateScrollBar();
		}
		break;

		case WM_SETFOCUS:
			m_bFocus = TRUE;
			CreateCaret(hWnd, NULL, m_cxChar, m_cyChar);
			return 0;

		case WM_KILLFOCUS:
			m_bFocus = FALSE;
			DestroyCaret();
			return 0;

		case WM_SETFONT:
		{
			m_hFont = (HFONT)wParam;
			SelectObject(m_DC.GetDC(), m_hFont);
			UpdateFontMetrics();
			if (lParam)
				Redraw();
		}
		return 0;

		case WM_SETTINGCHANGE:
			UpdateSystemConfig();
			break;
		}
		return CWnd::OnMsg(hWnd, uMsg, wParam, lParam);
	}

	void SetData(PCVOID pData, SIZE_T cbData)
	{
		m_pData = (PCBYTE)pData;
		m_cbData = cbData;
		UpdateDataMetrics();
		UpdateScrollBar();
	}

	EckInline constexpr int GetVisibleRowCount() const
	{
		return std::min((m_cyClient - m_cyChar - D.cyGap) / (m_cyChar + D.cyGap), m_cRow);
	}

	EckInline constexpr int GetPartialVisibleRowCount() const
	{
		return std::min(DivUpper(m_cyClient - m_cyChar - D.cyGap, m_cyChar + D.cyGap), m_cRow);
	}

	EckInline constexpr int GetDataRegionWidth() const
	{
		return m_cxData;
	}

	EckInline constexpr int GetColumnCount() const
	{
		return D.cCol;
	}

	EckInline constexpr int GetColumnPerGroup() const
	{
		return D.cColPerGroup;
	}

	EckInline constexpr int GetColumnWidth() const
	{
		return m_cxChar * 2 + D.cxGap;
	}

	EckInline constexpr int GetCharColumnWidth() const
	{
		return (m_cxChar + 1) * D.cCol + D.cxGap;
	}

	EckInline constexpr int GetHeaderHeight() const
	{
		return m_cyChar + D.cyGap;
	}

	EckInline constexpr int GetRowHeight() const
	{
		return m_cyChar + D.cyGap;
	}

	EckInline constexpr int GetFirstVisibleRow() const
	{
		return int(m_posFirstVisible / D.cCol);
	}

	int InsertCharColumn(UINT uCodePage, int idx = -1)
	{
		const BOOL bUtf16 = (uCodePage == CP_UTF16LE || uCodePage == CP_UTF16BE);
		CPINFOEXW cpi;
		if (!bUtf16)
			if (!GetCPInfoExW(uCodePage, 0, &cpi))
				return -1;
		if (idx < 0)
			idx = (int)m_vCharCol.size();
		if (bUtf16)
			m_vCharCol.emplace(m_vCharCol.begin() + idx,
				uCodePage, CLR_DEFAULT, 0, 0, uCodePage == CP_UTF16LE ? L"UTF-16LE" : L"UTF-16BE");
		else
		{
			auto& e = *m_vCharCol.emplace(m_vCharCol.begin() + idx,
				uCodePage, CLR_DEFAULT, cpi.MaxCharSize, 0, cpi.CodePageName);
			memcpy(e.LeadByteRange, cpi.LeadByte, sizeof(cpi.LeadByte));
		}
		return idx;
	}

	void ScrollV(int d)
	{
		SCROLLINFO si;
		si.cbSize = sizeof(si);
		si.fMask = SIF_POS;
		GetSbInfo(SB_VERT, &si);
		const int iOld = si.nPos;
		si.nPos += d;
		SetSbInfo(SB_VERT, &si);
		GetSbInfo(SB_VERT, &si);
		if (iOld != si.nPos)
		{
			ReCalcFirstVisible(si.nPos);
			RECT rc{ 0,m_cyChar + D.cyGap,m_cxClient,m_cyClient };
			ScrollWindowEx(HWnd, 0, (iOld - si.nPos) * (m_cyChar + D.cyGap), &rc, &rc,
				NULL, NULL, SW_INVALIDATE);
			UpdateWindow(HWnd);
			UpdateCaretPos();
		}
	}

	void ScrollH(int d)
	{
		SCROLLINFO si;
		si.cbSize = sizeof(si);
		si.fMask = SIF_POS;
		GetSbInfo(SB_HORZ, &si);
		const int iOld = si.nPos;
		si.nPos += d;
		SetSbInfo(SB_HORZ, &si);
		GetSbInfo(SB_HORZ, &si);
		if (iOld != si.nPos)
		{
			ScrollWindowEx(HWnd, iOld - si.nPos, 0, NULL, NULL,
				NULL, NULL, SW_INVALIDATE);
			UpdateWindow(HWnd);
			UpdateCaretPos();
		}
	}

	size_t HitTest(HEHITTEST& heht) const
	{
		const int xStart = -GetSbPos(SB_HORZ);
		heht.uFlags = 0u;
		heht.idxRowInView = heht.idxCol = heht.idxCharCol = -1;

		if (heht.pt.x < 0 || heht.pt.x > std::min(xStart + m_cxContent, m_cxClient) ||
			heht.pt.y < 0 || heht.pt.y > GetPartialVisibleRowCount() * GetRowHeight() + GetHeaderHeight())
			return SizeTMax;
		if (heht.pt.y < GetHeaderHeight())
		{
			heht.bHitHeader = TRUE;
			return SizeTMax;
		}
		heht.idxRowInView = (heht.pt.y - GetHeaderHeight()) / GetRowHeight();
		int x = xStart + m_cxAddress;
		if (heht.pt.x < x)// 地址
		{
			heht.bHitAddress = TRUE;
			if (heht.pt.y < GetHeaderHeight() + heht.idxRowInView * GetRowHeight() - D.cyGap)
				heht.bHitContent = TRUE;
			return SizeTMax;
		}
		else if (heht.pt.x < x + m_cxData)// 数据
		{
			heht.bHitData = TRUE;
			heht.idxCol = (heht.pt.x - x) / GetColumnWidth();
			if (heht.pt.y < GetHeaderHeight() + (heht.idxRowInView + 1) * GetRowHeight() - D.cyGap &&
				heht.pt.x < x + GetColumnWidth() * (heht.idxCol + 1) - D.cxGap)
				heht.bHitContent = TRUE;
			const int xHalf = x + heht.idxCol * GetColumnWidth() + m_cxChar;
			heht.bFirstNumber = (heht.pt.x < xHalf);
			return m_posFirstVisible + heht.idxRowInView * D.cCol + heht.idxCol;
		}
		else if (heht.pt.x < xStart + m_cxContent)// 字符
		{
			heht.bHitChar = TRUE;
			heht.idxCharCol = (heht.pt.x - x - m_cxData) / GetCharColumnWidth();
			heht.idxCol = (heht.pt.x - x - m_cxData - heht.idxCharCol * GetCharColumnWidth()) / m_cxChar;
			if (heht.pt.y < GetHeaderHeight() + (heht.idxRowInView + 1) * GetRowHeight() - D.cyGap &&
				heht.pt.x < x + m_cxData + (heht.idxCharCol + 1) * GetCharColumnWidth() - m_cxChar - D.cxGap)
				heht.bHitContent = TRUE;
			return m_posFirstVisible + heht.idxRowInView * D.cCol + heht.idxCol;
		}
		else
			heht.idxRowInView = -1;
		return SizeTMax;
	}
};
ECK_NAMESPACE_END