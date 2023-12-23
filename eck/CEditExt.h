﻿#pragma once
#include "CEdit.h"

ECK_NAMESPACE_BEGIN
inline constexpr int
DATAVER_EDITEXT_1 = 1;

#pragma pack(push, ECK_CTRLDATA_ALIGN)
struct CREATEDATA_EDITEXT
{
	int iVer;
	COLORREF crText;
	COLORREF crTextBK;
	COLORREF crBK;
	ECKENUM iInputMode;
	BITBOOL bMultiLine : 1;
	BITBOOL bAutoWrap : 1;
};
#pragma pack(pop)

class CEditExt :public CEdit
{
public:
	enum class InputMode
	{
		Normal = 0,
		ReadOnly = 1,
		Password = 2,
		NeedFilterKey = 2,// 仅供内部使用
		IntText = 3,
		RealText = 4,
		Byte = 5,
		Short = 6,
		Int = 7,
		LongLong = 8,
		Float = 9,
		Double = 10,
		DateTime = 11,
	};
private:
	COLORREF m_crText;			// 文本颜色
	COLORREF m_crTextBK;		// 文本背景色
	COLORREF m_crBK;			// 编辑框背景色
	InputMode m_iInputMode = InputMode::Normal;	// 输入方式
	BITBOOL m_bMultiLine : 1 = FALSE;			// 多行
	BITBOOL m_bAutoWrap : 1 = TRUE;				// 自动换行
	WCHAR m_chMask = 0;			// 掩码字符

	HBRUSH m_hbrEditBK = NULL;	// 背景画刷
	int m_cyText = 0;			// 文本高度
	RECT m_rcMargins{};			// 边距
	HWND m_hParent = NULL;		// 父窗口

	void UpdateTextInfo()
	{
		HFONT hFont = (HFONT)SendMsg(WM_GETFONT, 0, 0);
		HDC hDC = GetDC(m_hWnd);
		SelectObject(hDC, hFont);
		TEXTMETRICW tm;
		GetTextMetricsW(hDC, &tm);
		m_cyText = tm.tmHeight;
		ReleaseDC(m_hWnd, hDC);
	}
public:
	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_KEYDOWN:
			if (wParam == 'A')
				if (GetKeyState(VK_CONTROL) & 0x80000000)
					SendMessageW(hWnd, EM_SETSEL, 0, -1);// Ctrl + A全选
			break;

		case WM_CHAR:
		{
			if (m_iInputMode > InputMode::NeedFilterKey)
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
				if ((wParam >= L'0' && wParam <= L'9') ||
					wParam == L'-')
					break;
				else
					return 0;
			case InputMode::RealText:
			case InputMode::Float:
			case InputMode::Double:
				if ((wParam >= L'0' && wParam <= L'9') ||
					wParam == L'-' ||
					wParam == L'.' ||
					wParam == L'e' ||
					wParam == L'E')
					break;
				else
					return 0;
			case InputMode::Byte:
				if ((wParam >= L'0' && wParam <= L'9'))
					break;
				else
					return 0;
			}
		}
		return CEdit::OnMsg(hWnd, uMsg, wParam, lParam);

		case WM_KILLFOCUS:
		{
			constexpr SIZE_T BUFSIZE_EDITVALUE = 36;
			WCHAR szValue[BUFSIZE_EDITVALUE]{};
			LONGLONG llValue;
			double lfValue;

			PWSTR pszText;
			int cchText;

			PCWSTR pszCorrectValue = NULL;
			switch (m_iInputMode)
			{
			case InputMode::Byte:
			{
				GetWindowTextW(hWnd, szValue, BUFSIZE_EDITVALUE);
				llValue = _wtoi64(szValue);
				if (llValue < 0ll)
					pszCorrectValue = L"0";
				else if (llValue > 255ll)
					pszCorrectValue = L"255";
				else
					SetWindowTextW(hWnd, std::to_wstring(llValue).c_str());
			}
			break;
			case InputMode::Short:
			{
				GetWindowTextW(hWnd, szValue, BUFSIZE_EDITVALUE);
				llValue = _wtoi64(szValue);
				if (llValue < -32768ll)
					pszCorrectValue = L"-32768";
				else if (llValue > 32767ll)
					pszCorrectValue = L"32767";
				else
					SetWindowTextW(hWnd, std::to_wstring(llValue).c_str());
			}
			break;
			case InputMode::Int:
			{
				GetWindowTextW(hWnd, szValue, BUFSIZE_EDITVALUE);
				llValue = _wtoi64(szValue);
				if (llValue < -2147483648ll)
					pszCorrectValue = L"-2147483648";
				else if (llValue > 2147483647ll)
					pszCorrectValue = L"2147483647";
				else
					SetWindowTextW(hWnd, std::to_wstring(llValue).c_str());
			}
			break;
			case InputMode::LongLong:
			{
				GetWindowTextW(hWnd, szValue, BUFSIZE_EDITVALUE);
				llValue = _wtoi64(szValue);
				if (errno == ERANGE)
				{
					if (llValue == _I64_MIN)
						pszCorrectValue = L"-9223372036854775808";
					else if (llValue == _I64_MAX)
						pszCorrectValue = L"9223372036854775807";
				}
				else
					SetWindowTextW(hWnd, std::to_wstring(llValue).c_str());
			}
			break;
			case InputMode::Float:
			{
				cchText = GetWindowTextLengthW(hWnd);
				if (!cchText)
					break;
				pszText = new WCHAR[cchText + 1];
				GetWindowTextW(hWnd, pszText, cchText + 1);
				lfValue = _wtof(pszText);
				if (lfValue < -3.402823466e38)// 实际上正负值中间是有空隙的，不做判断了。。。
					SetWindowTextW(hWnd, L"-3.402823466e38");
				else if (lfValue < 3.402823466e38)
					SetWindowTextW(hWnd, L"3.402823466e38");
				else
					SetWindowTextW(hWnd, std::to_wstring(lfValue).c_str());
				delete[] pszText;
			}
			return CEdit::OnMsg(hWnd, uMsg, wParam, lParam);
			case InputMode::Double:
			{
				cchText = GetWindowTextLengthW(hWnd);
				if (!cchText)
					break;
				pszText = new WCHAR[cchText + 1];
				GetWindowTextW(hWnd, pszText, cchText + 1);
				lfValue = _wtof(pszText);
				if (*(ULONGLONG*)&lfValue == 0xFFF0000000000000)
					SetWindowTextW(hWnd, L"-1.79769313486231570e308");
				else if (*(ULONGLONG*)&lfValue == 0x7FF0000000000000)
					SetWindowTextW(hWnd, L"1.79769313486231570e308");
				else
					SetWindowTextW(hWnd, std::to_wstring(lfValue).c_str());
				delete[] pszText;
			}
			return CEdit::OnMsg(hWnd, uMsg, wParam, lParam);
			case InputMode::DateTime:
				break;
			}

			if (pszCorrectValue)
				SetWindowTextW(hWnd, pszCorrectValue);
		}
		break;

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

			RECT rcWnd, rcText;
			HDC hDC = GetWindowDC(hWnd);
			// 取非客户区矩形
			GetWindowRect(hWnd, &rcWnd);
			rcWnd.right -= rcWnd.left;
			rcWnd.bottom -= rcWnd.top;
			rcWnd.left = 0;
			rcWnd.top = 0;
			// 制文本矩形
			rcText.left = 0;
			rcText.top = (rcWnd.bottom - m_cyText) / 2;
			rcText.right = rcWnd.right;
			rcText.bottom = rcText.top + m_cyText;
			// 非客户区矩形减掉边框
			rcWnd.left += m_rcMargins.left;
			rcWnd.top += m_rcMargins.top;
			rcWnd.right -= m_rcMargins.right;
			rcWnd.bottom -= m_rcMargins.bottom;
			// 异或合并，剪辑
			HRGN hRgnBK = CreateRectRgnIndirect(&rcWnd);
			HRGN hRgnText = CreateRectRgnIndirect(&rcText);
			CombineRgn(hRgnBK, hRgnBK, hRgnText, RGN_XOR);
			SelectClipRgn(hDC, hRgnBK);
			DeleteObject(hRgnBK);
			DeleteObject(hRgnText);
			// 填充背景
			FillRect(hDC, &rcWnd, m_hbrEditBK);
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

		case WM_NCHITTEST:
		{
			auto lResult = CEdit::OnMsg(hWnd, uMsg, wParam, lParam);
			if (!GetMultiLine())
			{
				if (lResult == HTNOWHERE)
					lResult = HTCLIENT;
			}
			return lResult;
		}
		}

		return CEdit::OnMsg(hWnd, uMsg, wParam, lParam);
	}

	BOOL OnNotifyMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult) override
	{
		switch (uMsg)
		{
		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORSTATIC:
		{
			if ((HWND)lParam != m_hWnd)
				break;
			HBRUSH hbr;
			if (m_hbrEditBK)
				hbr = m_hbrEditBK;
			else
				hbr = (HBRUSH)DefSubclassProc(hWnd, uMsg, wParam, lParam);
			if (m_crText != CLR_DEFAULT)
				SetTextColor((HDC)wParam, m_crText);
			SetBkColor((HDC)wParam, m_crTextBK);
			lResult = (LRESULT)hbr;
			return TRUE;
		}
		break;
		}

		return CEdit::OnNotifyMsg(hWnd, uMsg, wParam, lParam, lResult);
	}

	CEditExt()
	{
		m_crTextBK = m_crBK = GetSysColor(COLOR_WINDOW);
		m_crText = CLR_DEFAULT;
	}

	~CEditExt()
	{
		DeleteObject(m_hbrEditBK);
	}

	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = NULL) override
	{
		if (pData)
	{
		auto pBase = (const CREATEDATA_STD*)pData;
		auto pEditBase = (const CREATEDATA_EDIT*)CWnd::SkipBaseData(pBase);
		auto p = (const CREATEDATA_EDITEXT*)CEdit::SkipBaseData(pEditBase);
		if (pBase->iVer_Std != DATAVER_STD_1)
		{
			EckDbgBreak();
			return NULL;
		}

		BOOL bVisible = IsBitSet(pBase->dwStyle, WS_VISIBLE);
		dwStyle = pBase->dwStyle & ~WS_VISIBLE;

		switch (p->iVer)
		{
		case DATAVER_EDITEXT_1:
			SetMultiLine(p->bMultiLine);
			SetAutoWrap(p->bAutoWrap);
			if (m_bMultiLine)
				dwStyle |= ES_MULTILINE | ES_AUTOVSCROLL | (m_bAutoWrap ? ES_AUTOHSCROLL : 0);
			else
				dwStyle |= ES_AUTOHSCROLL;
			break;
		default:
			EckDbgBreak();
			break;
		}
		m_hWnd = IntCreate(pBase->dwExStyle, WC_EDITW, pBase->Text(), dwStyle,
			x, y, cx, cy, hParent, hMenu, NULL, NULL);

		switch (p->iVer)
		{
		case DATAVER_EDITEXT_1:
			SetPasswordChar(pEditBase->chPassword);
			SetTransformMode((TransMode)pEditBase->eTransMode);
			SetSel(pEditBase->iSelStart, pEditBase->iSelEnd);
			SetMargins(pEditBase->iLeftMargin, pEditBase->iRightMargin);
			SetCueBanner(pEditBase->CueBanner(), TRUE);
			SetLimitText(pEditBase->cchMax);

			SetClr(0, p->crText);
			SetClr(1, p->crTextBK);
			SetClr(2, p->crBK);
			SetInputMode((InputMode)p->iInputMode);
			break;
		}
		if (bVisible)
			ShowWindow(m_hWnd, SW_SHOWNOACTIVATE);
	}
	else
	{
		dwStyle |= WS_CHILD;
		if (m_bMultiLine)
			dwStyle |= ES_MULTILINE | ES_AUTOVSCROLL | (m_bAutoWrap ? ES_AUTOHSCROLL : 0);
		else
			dwStyle |= ES_AUTOHSCROLL;

		m_hWnd = IntCreate(dwExStyle, WC_EDITW, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, NULL, NULL);
	}

	m_hParent = hParent;

	if (!GetMultiLine())
	{
		UpdateTextInfo();
		FrameChanged();
	}
	return m_hWnd;
	}

	void SerializeData(CRefBin& rb) override
	{
		const SIZE_T cbSize = sizeof(CREATEDATA_EDITEXT);
		CEdit::SerializeData(rb);
		((CREATEDATA_EDIT*)CWnd::SkipBaseData(rb.Data()))->chPassword = GetPasswordChar();

		CMemWriter w(rb.PushBack(cbSize), cbSize);
		CREATEDATA_EDITEXT* p;
		w.SkipPointer(p);
		p->iVer = DATAVER_EDITEXT_1;
		p->crText = GetClr(0);
		p->crTextBK = GetClr(1);
		p->crBK = GetClr(2);
		p->iInputMode = (ECKENUM)GetInputMode();
		p->bMultiLine = GetMultiLine();
		p->bAutoWrap = GetAutoWrap();
	}

	/// <summary>
	/// 置颜色
	/// </summary>
	/// <param name="iType">0 - 文本  1 - 文本背景  2 - 背景</param>
	/// <param name="cr">颜色</param>
	void SetClr(int iType, COLORREF cr)
	{
		switch (iType)
		{
		case 0:m_crText = cr; break;
		case 1:m_crTextBK = cr; break;
		case 2:
			if (m_hbrEditBK)
				DeleteObject(m_hbrEditBK);
			m_hbrEditBK = CreateSolidBrush(cr);
			m_crBK = cr;
			SendMsg(WM_NCPAINT, 0, 0);
			break;
		}
		Redraw();
	}

	/// <summary>
	/// 取颜色
	/// </summary>
	/// <param name="iType">0 - 文本  1 - 文本背景  2 - 背景</param>
	EckInline COLORREF GetClr(int iType)
	{
		switch (iType)
		{
		case 0:return m_crText;
		case 1:return m_crTextBK;
		case 2:return m_crBK;
		}
		assert(FALSE);
		return 0;
	}

	EckInline void SetMultiLine(BOOL bMultiLine)
	{
		m_bMultiLine = bMultiLine;
	}

	EckInline BOOL GetMultiLine()
	{
		return IsBitSet(GetWindowLongPtrW(m_hWnd, GWL_STYLE), ES_MULTILINE);
	}

	EckInline void SetAutoWrap(BOOL bAutoWrap)
	{
		m_bAutoWrap = bAutoWrap;
	}

	EckInline BOOL GetAutoWrap()
	{
		return m_bAutoWrap;
	}

	EckInline void SetInputMode(InputMode iInputMode)
	{
		m_iInputMode = iInputMode;
		SendMsg(EM_SETREADONLY, iInputMode == InputMode::ReadOnly, 0);
		SendMsg(EM_SETPASSWORDCHAR, (iInputMode == InputMode::Password ? m_chMask : 0), 0);
	}

	EckInline InputMode GetInputMode()
	{
		return m_iInputMode;
	}

	EckInline void SetPasswordChar(WCHAR chMask)
	{
		m_chMask = chMask;
		if (m_iInputMode == InputMode::Password)
			CEdit::SetPasswordChar(m_chMask);
	}

	EckInline WCHAR GetPasswordChar()
	{
		return m_chMask;
	}
};

ECK_NAMESPACE_END