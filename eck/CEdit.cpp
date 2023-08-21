#include "CEdit.h"

ECK_NAMESPACE_BEGIN

SUBCLASS_MGR_INIT(CEdit, SCID_EDIT, CEdit::SubclassProc)
SUBCLASS_REF_MGR_INIT(CEdit, ObjRecorderRefPlaceholder, SCID_EDITPARENT, CEdit::SubclassProc_Parent, ObjRecordRefStdDeleter)

void CEdit::UpdateTextInfo()
{
	HFONT hFont = (HFONT)SendMsg(WM_GETFONT, 0, 0);
	HDC hDC = GetDC(m_hWnd);
	SelectObject(hDC, hFont);
	TEXTMETRICW tm;
	GetTextMetricsW(hDC, &tm);
	m_cyText = tm.tmHeight;
	ReleaseDC(m_hWnd, hDC);
}

LRESULT CALLBACK CEdit::SubclassProc_Parent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	switch (uMsg)
	{
	case WM_CTLCOLOREDIT:
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
			if (p->m_Info.crText != CLR_DEFAULT)
				SetTextColor((HDC)wParam, p->m_Info.crText);
			SetBkColor((HDC)wParam, p->m_Info.crTextBK);
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

LRESULT CALLBACK CEdit::SubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	auto p = (CEdit*)dwRefData;
	switch (uMsg)
	{
	case WM_KEYDOWN:
		if (wParam == 'A')
			if (GetKeyState(VK_CONTROL) & 0x80000000)
				SendMessageW(hWnd, EM_SETSEL, 0, -1);// Ctrl + A全选
		break;

	case WM_CHAR:
	{
		if (p->m_Info.iInputMode > 2)
		{
			if (GetKeyState(VK_CONTROL) & 0x8000 || GetKeyState(VK_MENU) & 0x8000)
				break;

			if (wParam == VK_BACK ||
				wParam == VK_RETURN)
				break;
		}

		switch (p->m_Info.iInputMode)
		{
		case 3:// 整数文本
		case 6:// 输入短整数
		case 7:// 输入整数
		case 8:// 输入长整数
			if ((wParam >= L'0' && wParam <= L'9') ||
				wParam == L'-')
				break;
			else
				return 0;
		case 4:// 小数文本
		case 9:// 输入小数
		case 10:// 输入双精度小数
			if ((wParam >= L'0' && wParam <= L'9') ||
				wParam == L'-' ||
				wParam == L'.' ||
				wParam == L'e' ||
				wParam == L'E')
				break;
			else
				return 0;
		case 5:// 输入字节
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
		switch (p->m_Info.iInputMode)
		{
		case 5:// 输入字节
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
		case 6:// 输入短整数
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
		case 7:// 输入整数
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
		case 8:// 输入长整数
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
		case 9:// 输入小数
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
		case 10:// 输入双精度小数
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
		case 11:// 输入日期时间
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

void CEdit::SetClr(int iType, COLORREF cr)
{
	switch (iType)
	{
	case 0:m_Info.crText = cr; break;
	case 1:m_Info.crTextBK = cr; break;
	case 2:
		if (m_hbrEditBK)
			DeleteObject(m_hbrEditBK);
		m_hbrEditBK = CreateSolidBrush(cr);
		m_Info.crBK = cr;
		SendMsg(WM_NCPAINT, 0, 0);
		break;
	}
	Redraw();
}

void CEdit::SetTransformMode(int iTransformMode)
{
	DWORD dwStyle;
	switch (iTransformMode)
	{
	case 0:dwStyle = 0; break;
	case 1:dwStyle = ES_LOWERCASE; break;
	case 2:dwStyle = ES_UPPERCASE; break;
	default:assert(FALSE);
	}
	ModifyStyle(dwStyle, ES_LOWERCASE | ES_UPPERCASE);
}

int CEdit::GetTransformMode()
{
	DWORD dwStyle = GetStyle();
	if (IsBitSet(dwStyle, ES_LOWERCASE))
		return 1;
	else if (IsBitSet(dwStyle, ES_UPPERCASE))
		return 2;
	else
		return 0;
}

void CEdit::SetSelPos(int iSelPos)
{
	DWORD dwStart, dwEnd;
	SendMsg(EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
	DWORD dwLen;
	if (dwStart > dwEnd)
		dwLen = dwStart - dwEnd;
	else
		dwLen = dwEnd - dwStart;
	SendMsg(EM_SETSEL, iSelPos, iSelPos + dwLen);
}

int CEdit::GetSelNum()
{
	DWORD dwStart, dwEnd;
	SendMsg(EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
	DWORD dwLen;
	if (dwStart > dwEnd)
		dwLen = dwStart - dwEnd;
	else
		dwLen = dwEnd - dwStart;
	return (int)dwLen;
}

CRefStrW CEdit::GetSelText()
{
	CRefStrW rs;
	DWORD dwStart, dwEnd;
	SendMsg(EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
	DWORD dwLen;
	if (dwStart > dwEnd)
		dwLen = dwStart - dwEnd;
	else
		dwLen = dwEnd - dwStart;
	if (!dwLen)
		return rs;

	rs.ReSize(dwEnd + 1);
	PWSTR psz = new WCHAR[dwEnd + 1];// UTF-16是变长的，这里按说还要多一步解析，先这样吧。。
	GetWindowTextW(m_hWnd, psz, (int)dwEnd + 1);
	wcscpy(rs, psz + dwStart);
	delete[] psz;
	return rs;
}

int CEdit::CharFromPos(POINT pt, int* piPosInLine)
{
	int iPos;
	DWORD dwRet = (DWORD)SendMsg(EM_CHARFROMPOS, 0, MAKELPARAM(pt.x, pt.y));
	USHORT usPos = LOWORD(dwRet);
	if (usPos == 65535)
		iPos = -1;
	else
		iPos = usPos;
	if (piPosInLine)
	{
		usPos = HIWORD(dwRet);
		if (usPos == 65535)
			*piPosInLine = -1;
		else
			*piPosInLine = usPos;
	}
	return iPos;
}

CRefStrW CEdit::GetLine(int iPos)
{
	CRefStrW rs;
	int cch = (int)SendMsg(EM_LINELENGTH, iPos, 0);
	if (cch)
	{
		rs.ReSize(cch);
		*(WORD*)rs.m_pszText = cch;// 发送消息前将第一个WORD设置为缓冲区大小
		SendMsg(EM_GETLINE, iPos, (LPARAM)rs.m_pszText);
		*(rs + cch) = L'\0';// 复制完成后不会添加结尾NULL
	}
	return rs;
}
ECK_NAMESPACE_END

#ifdef ECK_CTRL_DESIGN_INTERFACE
#include "DesignerDef.h"
ECK_NAMESPACE_BEGIN
EckPropCallBackRet CALLBACK SetProp_Edit(CWnd* pWnd, int idProp, EckCtrlPropValue* pProp)
{
	EckDCtrlDefSetProp;

	auto p = (CEdit*)pWnd;
	switch (idProp)
	{

	}
	return ESPR_NONE;
}

EckPropCallBackRet CALLBACK GetProp_Edit(CWnd* pWnd, int idProp, EckCtrlPropValue* pProp)
{
	EckDCtrlDefGetProp;

	return ESPR_NONE;
}

CWnd* CALLBACK Create_Edit(BYTE* pData, ECK_CREATE_CTRL_EXTRA_ARGS)
{
	auto p = new CEdit;
	p->Create(ECK_CREATE_CTRL_EXTRA_REALARGS);
	if (p->GetHWND())
		return p;
	else
	{
		delete p;
		return NULL;
	}
}

static EckCtrlPropEntry s_Prop_Edit[] =
{
	{1,L"TextColor",L"文本颜色",L"",ECPT::Color},
	{2,L"TextBKColor",L"文本背景颜色",L"",ECPT::Color},
	{3,L"BKColor",L"编辑框背景颜色",L"",ECPT::Color},
	{4,L"HideSelection",L"隐藏选择",L"",ECPT::Bool},
	{5,L"MaxLength",L"最大允许长度",L"",ECPT::Int},
	{6,L"MultiLine",L"是否多行",L"",ECPT::Bool},
	{7,L"Align",L"对齐方式",L"",ECPT::PickInt,ECPF_NONE,L"左对齐\0居中\0右对齐\0\0"},
	{8,L"InputMode",L"输入方式",L"",ECPT::PickInt,ECPF_NONE,
					L"通常\0""只读\0""密码\0""整数文本\0""小数文本\0""输入字节\0""输入短整数\0""输入整数\0"
					"输入长整数\0""输入小数\0""输入双精度小数\0""输入日期时间\0""\0"},
	{9,L"PasswordChar",L"密码遮盖字符",L"",ECPT::Text},
	{10,L"TransformMode",L"转换方式",L"",ECPT::PickInt,ECPF_NONE,L"无\0大写到小写\0小写到大写\0\0"},
	{11,L"CueBanner",L"提示文本",L"",ECPT::Text},
	{12,L"AlwaysCueBanner",L"总是显示提示文本",L"",ECPT::Bool},
	{13,L"AutoWrap",L"自动换行",L"",ECPT::Bool},
};

EckCtrlDesignInfo CtInfoEdit =
{
	L"Edit",
	L"编辑框",
	L"",
	NULL,
	ARRAYSIZE(s_Prop_Edit),
	s_Prop_Edit,
	SetProp_Edit,
	GetProp_Edit,
	Create_Edit,
	{140,40}
};
ECK_NAMESPACE_END
#endif // ECK_CTRL_DESIGN_INTERFACE