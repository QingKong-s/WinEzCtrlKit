#pragma once
#include "CWnd.h"
#include "GraphicsHelper.h"

ECK_NAMESPACE_BEGIN
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

	COLORREF crOddCol;
	COLORREF crEvenCol;
	COLORREF crAddressCol;

	UINT cCharCol;
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

	int m_oxContent{};

	int m_cRow{};

	int m_cScrollLine{ 3 };
	int m_cCharPreScrollH{ 3 };

	BITBOOL m_bLBtnDown : 1 = FALSE;

	int m_iDpi{ USER_DEFAULT_SCREEN_DPI };

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
		m_cRow = DivUpper(m_cbData, D.cCol);
		WCHAR szBuf[CchI64ToStrBufNoRadix2];
		m_cxAddress = swprintf(szBuf, (D.bHexAddress ? L"%08Ix" : L"%08Iu"), m_cbData) * m_cxChar + D.cxGap;
		m_cxData = (m_cxChar * 2 + D.cxGap) * D.cCol;
		m_cxContent = m_cxAddress + m_cxData + D.cxGap + (m_vCharCol.size() * (m_cxChar * D.cCol + D.cxGap));
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
			UpdateSystemConfig();
		}
		break;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);

			WCHAR szBuf[CchI64ToStrBufNoRadix2];
			PCBYTE pData = m_pData + m_posFirstVisible;
			// 画列头
			int xStart = m_oxContent;

			const HDC hDC = m_DC.GetDC();
			SetDCBrushColor(hDC, GetThreadCtx()->crDefBkg);
			FillRect(hDC, &ps.rcPaint, GetStockBrush(DC_BRUSH));
			RECT rcT{ xStart + D.cxGap, 0, xStart + m_cxAddress, m_cyChar };
			DrawTextW(hDC, L"Offset", -1, &rcT,
				DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);

			const int xData = xStart + m_cxAddress + D.cxGap;
			int x = xData;
			EckCounter(D.cCol, i)
			{
				swprintf(szBuf, L"% 2X", (UINT)i);
				TextOutW(hDC, x, 0, szBuf, 2);
				x += GetColumnWidth();
			}

			x = xData + GetDataRegionWidth();
			const int cxCharCol = GetCharColumnWidth();
			RECT rcCharCol{ x,0,x + cxCharCol + D.cxGap,m_cyChar };
			for (auto& e : m_vCharCol)
			{
				e.pNeedle = pData;
				e.xOrg = rcCharCol.left;
				rcCharCol.right -= D.cxGap;
				DrawTextW(hDC, e.rsName.Data(), e.rsName.Size(), &rcCharCol,
					DT_SINGLELINE | DT_RIGHT | DT_VCENTER | DT_NOPREFIX);
				rcCharCol.right += D.cxGap;
				OffsetRect(rcCharCol, cxCharCol + D.cxGap, 0);
			}

			// 画数据
			if (m_pData)
			{
				WCHAR szCharW[2]{};
				const PCWSTR pszFmtAddress = (D.bHexAddress ? L"%08Ix" : L"%08Iu");
				x = xData;
				OffsetRect(rcT, 0, m_cyChar + D.cyGap);
				EckCounter(m_cRow, i)
				{
					for (auto& e : m_vCharCol)
						e.x = e.xOrg;
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

					if (rcT.top >= ps.rcPaint.bottom)
						break;
				}
			EndDrawData:;
			}

			// 画分隔线
			// 列头分隔线
			MoveToEx(hDC, 0, m_cyChar, NULL);
			LineTo(hDC, xStart + m_cxContent, m_cyChar);
			// 偏移分隔线
			x = xStart + m_cxAddress + D.cxGap / 2;
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
				(m_cxChar * 2 + D.cxGap) * D.cCol;
			MoveToEx(hDC, x, 0, NULL);
			LineTo(hDC, x, m_cyClient);

			BitBltPs(&ps, hDC);
			EndPaint(hWnd, &ps);
		}
		break;

		case WM_LBUTTONDOWN:
		{
			m_bLBtnDown = TRUE;
			SetFocus(hWnd);
			SetCapture(hWnd);
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
			const int d = -GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
			if (d)
			{
				const int cScroll = std::min(std::max(GetVisibleRowCount(), 1), m_cScrollLine);
				SCROLLINFO si;
				si.cbSize = sizeof(si);
				si.fMask = SIF_POS;
				GetSbInfo(SB_VERT, &si);
				const int iOld = si.nPos;
				si.nPos += (d * cScroll);
				SetSbInfo(SB_VERT, &si);
				GetSbInfo(SB_VERT, &si);
				if (iOld != si.nPos)
				{
					ReCalcFirstVisible(si.nPos);
					RECT rc{ 0,m_cyChar + D.cyGap,m_cxClient,m_cyClient };
					ScrollWindowEx(hWnd, 0, (iOld - si.nPos) * (m_cyChar + D.cyGap), &rc, &rc, NULL, NULL, SW_INVALIDATE);
					UpdateWindow(hWnd);
				}
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
		}
		return 0;

		case WM_SIZE:
		{
			ECK_GET_SIZE_LPARAM(m_cxClient, m_cyClient, lParam);
			m_DC.ReSize(hWnd, m_cxClient, m_cyClient);
			UpdateScrollBar();
		}
		break;

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
		return (m_cyClient - m_cyChar - D.cyGap) / (m_cyChar + D.cyGap);
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
		return m_cxChar * D.cCol;
	}

	EckInline int InsertCharColumn(UINT uCodePage, int idx = -1)
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
};
ECK_NAMESPACE_END