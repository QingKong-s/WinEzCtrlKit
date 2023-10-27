#include "CEditExt.h"

ECK_NAMESPACE_BEGIN
SUBCLASS_MGR_INIT(CEditExt, SCID_EDIT, SubclassProc)
SUBCLASS_REF_MGR_INIT(CEditExt, ObjRecorderRefPlaceholder, SCID_EDITPARENT, SubclassProc_Parent, ObjRecordRefStdDeleter)

void CEditExt::UpdateTextInfo()
{
	HFONT hFont = (HFONT)SendMsg(WM_GETFONT, 0, 0);
	HDC hDC = GetDC(m_hWnd);
	SelectObject(hDC, hFont);
	TEXTMETRICW tm;
	GetTextMetricsW(hDC, &tm);
	m_cyText = tm.tmHeight;
	ReleaseDC(m_hWnd, hDC);
}

LRESULT CALLBACK CEditExt::SubclassProc_Parent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	switch (uMsg)
	{
	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORSTATIC:
	{
		auto it = m_WndRecord.find((HWND)lParam);
		if (it != m_WndRecord.end())
		{
			auto p = it->second;
			HBRUSH hbr;
			if (p->m_hbrEditBK)
				hbr = p->m_hbrEditBK;
			else
				hbr = (HBRUSH)DefSubclassProc(hWnd, uMsg, wParam, lParam);
			if (p->m_crText != CLR_DEFAULT)
				SetTextColor((HDC)wParam, p->m_crText);
			SetBkColor((HDC)wParam, p->m_crTextBK);
			return (LRESULT)hbr;
		}
	}
	break;

	case WM_DESTROY:
		m_SMRef.DeleteRecord(hWnd);
		break;
	}

	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK CEditExt::SubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	auto p = (CEditExt*)dwRefData;
	switch (uMsg)
	{
	case WM_KEYDOWN:
		if (wParam == 'A')
			if (GetKeyState(VK_CONTROL) & 0x80000000)
				SendMessageW(hWnd, EM_SETSEL, 0, -1);// Ctrl + A全选
		break;

	case WM_CHAR:
	{
		if (p->m_iInputMode > InputMode::NeedFilterKey)
		{
			if (GetKeyState(VK_CONTROL) & 0x8000 || GetKeyState(VK_MENU) & 0x8000)
				break;

			if (wParam == VK_BACK ||
				wParam == VK_RETURN)
				break;
		}

		switch (p->m_iInputMode)
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
	return DefSubclassProc(hWnd, uMsg, wParam, lParam);

	case WM_KILLFOCUS:
	{
		constexpr SIZE_T BUFSIZE_EDITVALUE = 36;
		WCHAR szValue[BUFSIZE_EDITVALUE]{};
		LONGLONG llValue;
		double lfValue;

		PWSTR pszText;
		int cchText;

		PCWSTR pszCorrectValue = NULL;
		switch (p->m_iInputMode)
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
		return DefSubclassProc(hWnd, uMsg, wParam, lParam);
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
		return DefSubclassProc(hWnd, uMsg, wParam, lParam);
		case InputMode::DateTime:
			break;
		}

		if (pszCorrectValue)
			SetWindowTextW(hWnd, pszCorrectValue);
	}
	break;

	case WM_DESTROY:
	{
		m_SMRef.Release(p->m_hParent);
		m_SM.RemoveSubclass(hWnd);
	}
	break;

	case WM_NCCALCSIZE:
	{
		if (!p->GetMultiLine())
		{
			LRESULT lResult;
			if (wParam)
			{
				auto pnccsp = (NCCALCSIZE_PARAMS*)lParam;
				p->m_rcMargins = pnccsp->rgrc[0];// 保存非客户区尺寸
				lResult = DefSubclassProc(hWnd, uMsg, wParam, lParam);// call默认过程，计算标准边框尺寸
				// 保存边框尺寸
				p->m_rcMargins.left = pnccsp->rgrc[0].left - p->m_rcMargins.left;
				p->m_rcMargins.top = pnccsp->rgrc[0].top - p->m_rcMargins.top;
				p->m_rcMargins.right -= pnccsp->rgrc[0].right;
				p->m_rcMargins.bottom -= pnccsp->rgrc[0].bottom;
				// 上下留空
				pnccsp->rgrc[0].top += ((pnccsp->rgrc[0].bottom - pnccsp->rgrc[0].top - p->m_cyText) / 2);
				pnccsp->rgrc[0].bottom = pnccsp->rgrc[0].top + p->m_cyText;
			}
			else
			{
				auto prc = (RECT*)lParam;
				p->m_rcMargins = *prc;// 保存非客户区尺寸
				lResult = DefSubclassProc(hWnd, uMsg, wParam, lParam);// call默认过程，计算标准边框尺寸
				// 保存边框尺寸
				p->m_rcMargins.left = prc->left - p->m_rcMargins.left;
				p->m_rcMargins.top = prc->top - p->m_rcMargins.top;
				p->m_rcMargins.right -= prc->right;
				p->m_rcMargins.bottom -= prc->bottom;
				// 上下留空
				prc->top += ((prc->bottom - prc->top - p->m_cyText) / 2);
				prc->bottom = prc->top + p->m_cyText;
			}
			return lResult;
		}
	}
	break;

	case WM_NCPAINT:
	{
		DefSubclassProc(hWnd, uMsg, wParam, lParam);// 画默认边框
		if (p->GetMultiLine())
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
		rcText.top = (rcWnd.bottom - p->m_cyText) / 2;
		rcText.right = rcWnd.right;
		rcText.bottom = rcText.top + p->m_cyText;
		// 非客户区矩形减掉边框
		rcWnd.left += p->m_rcMargins.left;
		rcWnd.top += p->m_rcMargins.top;
		rcWnd.right -= p->m_rcMargins.right;
		rcWnd.bottom -= p->m_rcMargins.bottom;
		// 异或合并，剪辑
		HRGN hRgnBK = CreateRectRgnIndirect(&rcWnd);
		HRGN hRgnText = CreateRectRgnIndirect(&rcText);
		CombineRgn(hRgnBK, hRgnBK, hRgnText, RGN_XOR);
		SelectClipRgn(hDC, hRgnBK);
		DeleteObject(hRgnBK);
		DeleteObject(hRgnText);
		// 填充背景
		FillRect(hDC, &rcWnd, p->m_hbrEditBK);
		ReleaseDC(hWnd, hDC);
	}
	return 0;

	case WM_SETFONT:
	{
		auto lResult = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		if (!p->GetMultiLine())
		{
			p->UpdateTextInfo();
			p->FrameChanged();
		}
		return lResult;
	}

	case WM_NCHITTEST:
	{
		auto lResult = DefSubclassProc(hWnd, uMsg, wParam, lParam);
		if (!p->GetMultiLine())
		{
			if (lResult == HTNOWHERE)
				lResult = HTCLIENT;
		}
		return lResult;
	}
	}

	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

HWND CEditExt::Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
	int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData)
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
		m_hWnd = CreateWindowExW(pBase->dwExStyle, WC_EDITW, pBase->Text(), dwStyle,
			x, y, cx, cy, hParent, i32ToP<HMENU>(nID), NULL, NULL);

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

		m_hWnd = CreateWindowExW(dwExStyle, WC_EDITW, pszText, dwStyle,
			x, y, cx, cy, hParent, i32ToP<HMENU>(nID), NULL, NULL);
	}

	m_SM.AddSubclass(m_hWnd, this);
	m_hParent = hParent;
	m_SMRef.AddRef(hParent, ObjRecorderRefPlaceholderVal);

	if (!GetMultiLine())
	{
		UpdateTextInfo();
		FrameChanged();
	}
	return m_hWnd;
}

CRefBin CEditExt::SerializeData(SIZE_T cbExtra, SIZE_T* pcbSize)
{
	SIZE_T cbBase;
	const SIZE_T cbSize = sizeof(CREATEDATA_EDITEXT);
	auto rb = CEdit::SerializeData(cbSize + cbExtra, &cbBase);
	if (pcbSize)
		*pcbSize = cbBase + cbSize;

	((CREATEDATA_EDIT*)CWnd::SkipBaseData(rb.Data()))->chPassword = GetPasswordChar();

	CMemWriter w(rb.Data() + cbBase, cbSize);

	CREATEDATA_EDITEXT* p;
	w.SkipPointer(p);
	p->iVer = DATAVER_EDITEXT_1;
	p->crText = GetClr(0);
	p->crTextBK = GetClr(1);
	p->crBK = GetClr(2);
	p->iInputMode = (ECKENUM)GetInputMode();
	p->bMultiLine = GetMultiLine();
	p->bAutoWrap = GetAutoWrap();

	return rb;
}

void CEditExt::SetClr(int iType, COLORREF cr)
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


ECK_NAMESPACE_END