/*
* WinEzCtrlKit Library
*
* CEditExt.h ： 编辑框扩展
*
* Copyright(C) 2023-2024 QingKong
*/
#pragma once
#include "CEdit.h"

ECK_NAMESPACE_BEGIN
class CEditExt :public CEdit
{
public:
	ECK_RTTI(CEditExt);

	enum class InputMode :BYTE
	{
		Normal,
		ReadOnly,
		Password,
		IntText,
		RealText,
		Byte,
		Short,
		Int,
		LongLong,
		Float,
		Double,
		DateTime,// 未实现

		Priv_NeedFilterKey = Password,// 仅供内部使用
	};
protected:
	CRefStrW m_rsCueBanner{};			// 输入提示

	COLORREF m_crText{ CLR_DEFAULT };	// 文本颜色
	COLORREF m_crTextBK{ CLR_DEFAULT };	// 文本背景色
	COLORREF m_crBK{ CLR_DEFAULT };		// 编辑框背景色

	int m_cyText{};		// 文本高度
	int m_cxWnd{};		// 客户区宽度
	int m_cyWnd{};		// 客户区高度
	RECT m_rcMargins{};	// 边距

	WCHAR m_chMask{};	// 掩码字符
	InputMode m_iInputMode{ InputMode::Normal };// 输入方式
#if ECKCXX20
	BITBOOL m_bAutoWrap : 1 = TRUE;				// [多行][延迟标志]自动换行，不加入ES_AUTOHSCROLL
	BITBOOL m_bMultiLineCueBanner : 1 = TRUE;	// [多行]显示提示
	BITBOOL m_bMultiLine : 1 = FALSE;			// [延迟标志]多行
	BITBOOL m_bChangeCreateStyle : 1 = TRUE;	// [延迟标志]指示样式是否受上面两个延迟标志影响
	BITBOOL m_bDisableColorOptions : 1 = FALSE;	// 禁用颜色选项，启用后不处理着色通知
	BITBOOL m_bCtrlASelectAll : 1 = TRUE;		// Ctrl+A全选

	BITBOOL m_bEmpty : 1 = FALSE;				// 空编辑框标志
#else// ECKCXX20
	union
	{
		struct
		{
			BITBOOL m_bAutoWrap : 1;
			BITBOOL m_bMultiLineCueBanner : 1;
			BITBOOL m_bMultiLine : 1;
			BITBOOL m_bChangeCreateStyle : 1;
			BITBOOL m_bDisableColorOptions : 1;
			BITBOOL m_bCtrlASelectAll : 1;
			BITBOOL m_bEmpty : 1;
		};
		BYTE m_byFlags = 0b0101011
	};
#endif// ECKCXX20

	void UpdateTextInfo()
	{
		const auto hDC = GetDC(HWnd);
		const auto hCDC = CreateCompatibleDC(hDC);
		SelectObject(hCDC, HFont);
		TEXTMETRICW tm;
		GetTextMetricsW(hCDC, &tm);
		m_cyText = tm.tmHeight;
		DeleteDC(hCDC);
		ReleaseDC(HWnd, hDC);
	}

	EckInline constexpr RECT GetSingleLineTextRect()
	{
		return
		{
			0,
			(m_cyWnd - m_cyText) / 2,
			m_cxWnd,
			(m_cyWnd - m_cyText) / 2 + m_cyText
		};
	}

	void InitForNewWindow()
	{
		if (!GetMultiLine())
		{
			UpdateTextInfo();
			FrameChanged();
		}
	}

	void CleanupForDestroyWindow()
	{
		m_bAutoWrap = m_bMultiLineCueBanner =
			m_bChangeCreateStyle = m_bCtrlASelectAll = TRUE;
		m_bMultiLine = m_bDisableColorOptions = FALSE;
		m_iInputMode = InputMode::Normal;
		m_chMask = L'\0';
		m_crText = m_crTextBK = m_crBK = CLR_DEFAULT;
		m_rsCueBanner.Clear();
	}
public:
	void AttachNew(HWND hWnd) override
	{
		CWnd::AttachNew(hWnd);
		const auto dwStyle = GetStyle();
		m_bMultiLine = Multiline;
		m_bAutoWrap = (m_bMultiLine ? IsBitSet(dwStyle, ES_AUTOHSCROLL) : FALSE);
		m_bEmpty = !GetWindowTextLengthW(hWnd);
		InitForNewWindow();
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
		case WM_PRINTCLIENT:
		case WM_PAINT:
		{
			if (m_bMultiLineCueBanner && m_bMultiLine && m_bEmpty &&
				!m_rsCueBanner.IsEmpty())
			{
				PAINTSTRUCT ps;
				BeginPaint(hWnd, wParam, ps);

				CEdit::OnMsg(hWnd, WM_PAINT, (WPARAM)ps.hdc, 0);
				RECT rcText;
				GetRect(&rcText);

				const auto hOld = SelectObject(ps.hdc, HFont);
				const auto crOld = SetTextColor(ps.hdc, GetThreadCtx()->crGray1);
				DrawTextW(ps.hdc, m_rsCueBanner.Data(), m_rsCueBanner.Size(), &rcText,
					DT_WORDBREAK | DT_NOPREFIX | DT_EDITCONTROL);
				SetTextColor(ps.hdc, crOld);
				SelectObject(ps.hdc, hOld);

				EndPaint(hWnd, wParam, ps);
				return 0;
			}
		}
		break;

		case WM_KEYDOWN:
			if (m_bCtrlASelectAll && wParam == 'A')
				if (GetKeyState(VK_CONTROL) & 0x80000000)
					SendMessageW(hWnd, EM_SETSEL, 0, -1);// Ctrl + A全选
			break;

		case WM_CHAR:
		{
			if (m_iInputMode > InputMode::Priv_NeedFilterKey)
			{
				if (GetKeyState(VK_CONTROL) & 0x8000 || GetKeyState(VK_MENU) & 0x8000)
					break;

				if (wParam == VK_BACK ||
					wParam == VK_RETURN)
					break;
			}

			switch (m_iInputMode)
			{
			case InputMode::IntText:
			case InputMode::Short:
			case InputMode::Int:
			case InputMode::LongLong:
				if ((wParam >= L'0' && wParam <= L'9') || wParam == L'-')
					break;
				return 0;
			case InputMode::RealText:
			case InputMode::Float:
			case InputMode::Double:
				if ((wParam >= L'0' && wParam <= L'9') || wParam == L'-' ||
					wParam == L'.' || wParam == L'e' || wParam == L'E')
					break;
				return 0;
			case InputMode::Byte:
				if ((wParam >= L'0' && wParam <= L'9'))
					break;
				return 0;
			}
		}
		break;

		case WM_KILLFOCUS:
		{
			WCHAR szValue[CchI64ToStrBufNoRadix2]{};
			LONGLONG llValue;
			double lfValue;

			PWSTR pszText;
			int cchText;

			PCWSTR pszCorrectValue = nullptr;
			switch (m_iInputMode)
			{
			case InputMode::Byte:
			{
				GetText(szValue, ARRAYSIZE(szValue));
				llValue = _wtoi64(szValue);
				if (llValue < 0ll)
					pszCorrectValue = L"0";
				else if (llValue > 255ll)
					pszCorrectValue = L"255";
				else if (_swprintf(szValue, L"%hhu", (BYTE)llValue))
					SetText(szValue);
			}
			break;

			case InputMode::Short:
			{
				GetText(szValue, ARRAYSIZE(szValue));
				llValue = _wtoi64(szValue);
				if (llValue < -32768ll)
					pszCorrectValue = L"-32768";
				else if (llValue > 32767ll)
					pszCorrectValue = L"32767";
				else if (_swprintf(szValue, L"%hd", (short)llValue))
					SetText(szValue);
			}
			break;

			case InputMode::Int:
			{
				GetText(szValue, ARRAYSIZE(szValue));
				llValue = _wtoi64(szValue);
				if (llValue < -2147483648ll)
					pszCorrectValue = L"-2147483648";
				else if (llValue > 2147483647ll)
					pszCorrectValue = L"2147483647";
				else if (_swprintf(szValue, L"%d", (int)llValue))
					SetText(szValue);
			}
			break;

			case InputMode::LongLong:
			{
				GetText(szValue, ARRAYSIZE(szValue));
				llValue = _wtoi64(szValue);
				if (errno == ERANGE)
				{
					if (llValue == _I64_MIN)
						pszCorrectValue = L"-9223372036854775808";
					else if (llValue == _I64_MAX)
						pszCorrectValue = L"9223372036854775807";
				}
				else if (_swprintf(szValue, L"%lld", llValue))
					SetText(szValue);
			}
			break;

			case InputMode::Float:
			{
				cchText = GetWindowTextLengthW(hWnd) + 10;
				if (!cchText)
					break;
				pszText = (PWSTR)_malloca(Cch2CbW(cchText));
				EckCheckMem(pszText);
				GetText(pszText, cchText + 1);
				lfValue = _wtof(pszText);
				if (lfValue < -3.402823466e38)// 实际上正负值中间是有空隙的，不做判断了。。。
					pszCorrectValue = L"-3.402823466e38";
				else if (lfValue < 3.402823466e38)
					pszCorrectValue = L"3.402823466e38";
				else
				{
					swprintf_s(pszText, cchText + 1, L"%.7g", lfValue);
					SetText(pszText);
				}
				_freea(pszText);
			}
			break;

			case InputMode::Double:
			{
				cchText = GetWindowTextLengthW(hWnd) + 10;
				if (!cchText)
					break;
				pszText = (PWSTR)_malloca(Cch2CbW(cchText));
				EckCheckMem(pszText);
				GetText(pszText, cchText);
				lfValue = _wtof(pszText);
				if (*(ULONGLONG*)&lfValue == 0xFFF0000000000000)
					pszCorrectValue = L"-1.79769313486231570e308";
				else if (*(ULONGLONG*)&lfValue == 0x7FF0000000000000)
					pszCorrectValue = L"1.79769313486231570e308";
				else
				{
					swprintf_s(pszText, cchText + 1, L"%.15g", lfValue);
					SetText(pszText);
				}
				_freea(pszText);
			}
			break;

			case InputMode::DateTime:
				break;
			}

			if (pszCorrectValue)
				SetWindowTextW(hWnd, pszCorrectValue);
		}
		break;

		case EM_SETCUEBANNER:
		{
			const auto lResult = CEdit::OnMsg(hWnd, uMsg, wParam, lParam);
			if (lResult && m_bMultiLine)
				m_rsCueBanner.DupString((PCWSTR)lParam);
			return lResult;
		}

		case WM_NCCALCSIZE:
		{
			if (!GetMultiLine())
			{
				LRESULT lResult;
				if (wParam)
				{
					auto pnccsp = (NCCALCSIZE_PARAMS*)lParam;
					m_rcMargins = pnccsp->rgrc[0];// 保存非客户区尺寸
					lResult = CEdit::OnMsg(hWnd, uMsg, wParam, lParam);// call默认过程，计算标准边框尺寸
					// 保存边框尺寸
					m_rcMargins.left = pnccsp->rgrc[0].left - m_rcMargins.left;
					m_rcMargins.top = pnccsp->rgrc[0].top - m_rcMargins.top;
					m_rcMargins.right -= pnccsp->rgrc[0].right;
					m_rcMargins.bottom -= pnccsp->rgrc[0].bottom;
					// 上下留空
					pnccsp->rgrc[0].top += ((pnccsp->rgrc[0].bottom - pnccsp->rgrc[0].top - m_cyText) / 2);
					pnccsp->rgrc[0].bottom = pnccsp->rgrc[0].top + m_cyText;
				}
				else
				{
					auto prc = (RECT*)lParam;
					m_rcMargins = *prc;// 保存非客户区尺寸
					lResult = CEdit::OnMsg(hWnd, uMsg, wParam, lParam);// call默认过程，计算标准边框尺寸
					// 保存边框尺寸
					m_rcMargins.left = prc->left - m_rcMargins.left;
					m_rcMargins.top = prc->top - m_rcMargins.top;
					m_rcMargins.right -= prc->right;
					m_rcMargins.bottom -= prc->bottom;
					// 上下留空
					prc->top += ((prc->bottom - prc->top - m_cyText) / 2);
					prc->bottom = prc->top + m_cyText;
				}
				return lResult;
			}
		}
		break;

		case WM_NCPAINT:
		{
			CEdit::OnMsg(hWnd, uMsg, wParam, lParam);// 画默认边框
			if (GetMultiLine())
				return 0;
			const HDC hDC = GetWindowDC(hWnd);
			RECT rcWnd{ 0,0,m_cxWnd,m_cyWnd };
			const RECT rcText{ GetSingleLineTextRect() };
			// 非客户区矩形减掉边框
			rcWnd.left += m_rcMargins.left;
			rcWnd.top += m_rcMargins.top;
			rcWnd.right -= m_rcMargins.right;
			rcWnd.bottom -= m_rcMargins.bottom;
			// 异或合并，剪辑
			const HRGN hRgnBK = CreateRectRgnIndirect(&rcWnd);
			const HRGN hRgnText = CreateRectRgnIndirect(&rcText);
			CombineRgn(hRgnBK, hRgnBK, hRgnText, RGN_XOR);
			SelectClipRgn(hDC, hRgnBK);
			DeleteObject(hRgnBK);
			DeleteObject(hRgnText);
			// 填充背景
			SetDCBrushColor(hDC, m_crBK != CLR_DEFAULT ? m_crBK : GetThreadCtx()->crDefBkg);
			FillRect(hDC, &rcWnd, GetStockBrush(DC_BRUSH));
			ReleaseDC(hWnd, hDC);
		}
		return 0;

		case WM_SETFONT:
		{
			auto lResult = CEdit::OnMsg(hWnd, uMsg, wParam, lParam);
			if (!GetMultiLine())
			{
				UpdateTextInfo();
				FrameChanged();
			}
			return lResult;
		}

		case WM_WINDOWPOSCHANGED:
		{
			const auto* const pwp = (WINDOWPOS*)lParam;
			m_cxWnd = pwp->cx;
			m_cyWnd = pwp->cy;
		}
		break;

		case WM_NCHITTEST:
		{
			auto lResult = CEdit::OnMsg(hWnd, uMsg, wParam, lParam);
			if (!GetMultiLine())
			{
				if (lResult == HTNOWHERE)
					lResult = HTCLIENT;
				else if (lResult == HTBORDER)
					return HTCLIENT;
			}
			return lResult;
		}

		case WM_CREATE:
		{
			const auto* const pcs = (CREATESTRUCTW*)lParam;
			m_bEmpty = !pcs->lpszName || *pcs->lpszName == L'\0';
			InitForNewWindow();
		}
		break;

		case WM_DESTROY:
			CleanupForDestroyWindow();
			break;
		}

		return CEdit::OnMsg(hWnd, uMsg, wParam, lParam);
	}

	LRESULT OnNotifyMsg(HWND hParent, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bProcessed) override
	{
		switch (uMsg)
		{
		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORSTATIC:
		{
			if (!m_bDisableColorOptions)
			{
				bProcessed = TRUE;
				const auto* const ptc = GetThreadCtx();
				SetTextColor((HDC)wParam, m_crText != CLR_DEFAULT ? m_crText : ptc->crDefText);
				SetBkColor((HDC)wParam, m_crTextBK != CLR_DEFAULT ? m_crTextBK : ptc->crDefBkg);
				SetDCBrushColor((HDC)wParam, m_crBK != CLR_DEFAULT ? m_crBK : ptc->crDefBkg);
				return (LRESULT)GetStockObject(DC_BRUSH);
			}
		}
		break;

		case WM_COMMAND:
		{
			if (HIWORD(wParam) == EN_UPDATE)
			{
				if (m_bMultiLineCueBanner)
				{
					const auto bEmpty = !GetWindowTextLengthW(HWnd);
					if (m_bEmpty != bEmpty)
					{
						m_bEmpty = bEmpty;
						Redraw();
					}
				}
			}
		}
		break;
		}

		return CEdit::OnNotifyMsg(hParent, uMsg, wParam, lParam, bProcessed);
	}

	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = nullptr) override
	{
		if (m_bChangeCreateStyle)
			if (m_bMultiLine)
				dwStyle |= (ES_MULTILINE | ES_AUTOVSCROLL |
					(m_bAutoWrap ? 0 : ES_AUTOHSCROLL));
			else
				dwStyle |= ES_AUTOHSCROLL;
		if (pData)
		{
			const auto* const pBase = (CTRLDATA_WND*)pData;
			PreDeserialize(pData);
			IntCreate(pBase->dwExStyle, WC_EDITW, pBase->Text(), pBase->dwStyle,
				x, y, cx, cy, hParent, hMenu, nullptr, nullptr);
			PostDeserialize(pData);
		}
		else
		{
			IntCreate(dwExStyle, WC_EDITW, pszText, dwStyle,
				x, y, cx, cy, hParent, hMenu, nullptr, nullptr);
		}
		return m_hWnd;
	};

	void SetClr(ClrPart ePart, COLORREF cr)
	{
		switch (ePart)
		{
		case ClrPart::Text: m_crText = cr; break;
		case ClrPart::TextBk: m_crTextBK = cr; break;
		case ClrPart::Bk:
			m_crBK = cr;
			SendMsg(WM_NCPAINT, 0, 0);
			break;
		}
	}

	constexpr COLORREF GetClr(ClrPart ePart)
	{
		switch (ePart)
		{
		case ClrPart::Text: return m_crText;
		case ClrPart::TextBk: return m_crTextBK;
		case ClrPart::Bk: return m_crBK;
		default: return CLR_INVALID;
		}
	}

	EckInline constexpr void SetMultiLine(BOOL bMultiLine) { m_bMultiLine = bMultiLine; }
	EckInline constexpr BOOL GetMultiLine() const { return m_bMultiLine; }

	EckInline constexpr void SetAutoWrap(BOOL bAutoWrap) { m_bAutoWrap = bAutoWrap; }
	EckInline constexpr BOOL GetAutoWrap() const { return m_bAutoWrap; }

	void SetInputMode(InputMode iInputMode)
	{
		m_iInputMode = iInputMode;
		SendMsg(EM_SETREADONLY, iInputMode == InputMode::ReadOnly, 0);
		SendMsg(EM_SETPASSWORDCHAR, (iInputMode == InputMode::Password ? m_chMask : 0), 0);
	}
	EckInline constexpr InputMode GetInputMode() const { return m_iInputMode; }

	void SetPasswordChar(WCHAR chMask)
	{
		m_chMask = chMask;
		if (m_iInputMode == InputMode::Password)
			CEdit::SetPasswordChar(m_chMask);
	}
	EckInline constexpr WCHAR GetPasswordChar() const { return m_chMask; }

	void SetMultiLineCueBanner(BOOL bMultiLineCueBanner)
	{
		m_bMultiLineCueBanner = bMultiLineCueBanner;
		const auto bEmpty = !GetWindowTextLengthW(HWnd);
		if (m_bEmpty != bEmpty)
		{
			m_bEmpty = bEmpty;
			Redraw();
		}
	}
	EckInline constexpr BOOL GetMultiLineCueBanner() const { return m_bMultiLineCueBanner; }

	EckInline constexpr void SetChangeCreateStyle(BOOL b) { m_bChangeCreateStyle = b; }
	EckInline constexpr BOOL GetChangeCreateStyle() const { return m_bChangeCreateStyle; }

	EckInline constexpr void SetDisableColorOptions(BOOL b) { m_bDisableColorOptions = b; }
	EckInline constexpr BOOL GetDisableColorOptions() const { return m_bDisableColorOptions; }
};
ECK_RTTI_IMPL_BASE_INLINE(CEditExt, CEdit);
ECK_NAMESPACE_END