#include "CEdit.h"
#include "DesignerDef.h"

ECK_NAMESPACE_BEGIN
HWND CEdit::Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
	int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData)
{
	if (pData)
	{
		auto pBase = (const CREATEDATA_STD*)pData;
		auto p = (const CREATEDATA_EDIT*)SkipBaseData(pData);
		if (pBase->iVer_Std != DATAVER_STD_1)
		{
			EckDbgBreak();
			return NULL;
		}

		BOOL bVisible = IsBitSet(pBase->dwStyle, WS_VISIBLE);
		dwStyle = pBase->dwStyle & ~WS_VISIBLE;

		m_hWnd = CreateWindowExW(pBase->dwExStyle, WC_EDITW, pBase->Text(), dwStyle,
			x, y, cx, cy, hParent, i32ToP<HMENU>(nID), NULL, NULL);

		switch (p->iVer)
		{
		case DATAVER_EDIT_1:
			SetPasswordChar(p->chPassword);
			SetTransformMode((TransMode)p->eTransMode);
			SetSel(p->iSelStart, p->iSelEnd);
			SetMargins(p->iLeftMargin, p->iRightMargin);
			SetCueBanner(p->CueBanner(), TRUE);
			SetLimitText(p->cchMax);
			break;
		default:
			EckDbgBreak();
			break;
		}
		if (bVisible)
			ShowWindow(m_hWnd, SW_SHOWNOACTIVATE);
	}
	else
	{
		dwStyle |= WS_CHILD;

		m_hWnd = CreateWindowExW(dwExStyle, WC_EDITW, pszText, dwStyle,
			x, y, cx, cy, hParent, i32ToP<HMENU>(nID), NULL, NULL);
	}
	return m_hWnd;
}

CRefBin CEdit::SerializeData(SIZE_T cbExtra, SIZE_T* pcbSize)
{
	SIZE_T cbBase;
	auto rsCueBanner = GetCueBanner();
	const SIZE_T cbSize = sizeof(CREATEDATA_EDIT) + rsCueBanner.ByteSize();
	auto rb = CWnd::SerializeData(cbSize + cbExtra, &cbBase);
	if (pcbSize)
		*pcbSize = cbBase + cbSize;

	CMemWriter w(rb.Data() + cbBase, cbSize);
	CREATEDATA_EDIT* p;
	w.SkipPointer(p);
	p->iVer = DATAVER_EDIT_1;

	p->chPassword = GetPasswordChar();
	p->eTransMode = (ECKENUM)GetTransformMode();
	GetSel(&p->iSelStart, &p->iSelEnd);
	GetMargins(&p->iLeftMargin, &p->iRightMargin);
	p->cchCueBanner = rsCueBanner.Size();
	p->cchMax = GetLimitText();

	w << rsCueBanner;
	return rb;
}


void CEdit::SetTransformMode(TransMode iTransformMode)
{
	DWORD dwStyle;
	switch (iTransformMode)
	{
	case TransMode::None:dwStyle = 0; break;
	case TransMode::ToLowerCase:dwStyle = ES_LOWERCASE; break;
	case TransMode::ToUpperCase:dwStyle = ES_UPPERCASE; break;
	default:assert(FALSE);
	}
	ModifyStyle(dwStyle, ES_LOWERCASE | ES_UPPERCASE);
}

CEdit::TransMode CEdit::GetTransformMode()
{
	DWORD dwStyle = GetStyle();
	if (IsBitSet(dwStyle, ES_LOWERCASE))
		return TransMode::ToLowerCase;
	else if (IsBitSet(dwStyle, ES_UPPERCASE))
		return TransMode::ToUpperCase;
	else
		return TransMode::None;
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

int CEdit::GetSelLen()
{
	DWORD dwStart, dwEnd;
	SendMsg(EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
	DWORD dwLen;
	if (dwStart > dwEnd)
		dwLen = dwStart - dwEnd;
	else
		dwLen = dwEnd - dwStart;
	return dwLen;
}

CRefStrW CEdit::GetSelText()
{
	CRefStrW rs;
	DWORD dwStart, dwEnd;
	SendMsg(EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
	DWORD dwLen;
	if (dwStart > dwEnd)
	{
		dwLen = dwStart - dwEnd;
		std::swap(dwStart, dwEnd);
	}
	else
		dwLen = dwEnd - dwStart;
	if (!dwLen)
		return rs;
	rs.ReSize(dwLen);
	auto psz = (PWSTR)_malloca((dwEnd + 1) * sizeof(WCHAR));
	GetWindowTextW(m_hWnd, psz, dwEnd + 1);
	wcscpy(rs, psz + dwStart);
	_freea(psz);
	return rs;
}

int CEdit::GetSelText(PWSTR pszBuf, int cchMax)
{
	DWORD dwStart, dwEnd;
	SendMsg(EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
	DWORD dwLen;
	if (dwStart > dwEnd)
	{
		dwLen = dwStart - dwEnd;
		std::swap(dwStart, dwEnd);
	}
	else
		dwLen = dwEnd - dwStart;
	if (!dwLen)
		return 0;
	auto psz = (PWSTR)_malloca((dwEnd + 1) * sizeof(WCHAR));
	GetWindowTextW(m_hWnd, psz, dwEnd + 1);
	wcsncpy(pszBuf, psz + dwStart, cchMax);
	_freea(psz);
	if ((int)dwLen > cchMax)
	{
		*(pszBuf + cchMax) = L'\0';
		return cchMax;
	}
	else
		return dwLen;
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
		*(WORD*)rs.Data() = cch;// 发送消息前将第一个WORD设置为缓冲区大小
		SendMsg(EM_GETLINE, iPos, (LPARAM)rs.Data());
	}
	return rs;
}


CWnd* CALLBACK Create_Edit(PCBYTE pData, ECK_CREATE_CTRL_EXTRA_ARGS)
{
	auto p = new CEdit;
	p->Create(ECK_CREATE_CTRL_EXTRA_REALARGS, pData);
	if (p->GetHWND())
		return p;
	else
	{
		delete p;
		return NULL;
	}
}
ECK_NAMESPACE_END

#ifdef ECK_CTRL_DESIGN_INTERFACE

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

static EckCtrlPropEntry s_Prop_Edit[]
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

EckCtrlDesignInfo CtInfoEdit
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
	{80,24}
};
ECK_NAMESPACE_END
#else
ECK_NAMESPACE_BEGIN
EckCtrlDesignInfo CtInfoEdit{ Create_Edit };
ECK_NAMESPACE_END
#endif // ECK_CTRL_DESIGN_INTERFACE