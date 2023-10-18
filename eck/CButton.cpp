#include "CButton.h"
#include "DesignerDef.h"

ECK_NAMESPACE_BEGIN
void CButton::SetAlign(BOOL bHAlign, int iAlign)
{
	DWORD dwStyle = GetStyle();
	if (bHAlign)
	{
		dwStyle &= (~(BS_LEFT | BS_CENTER | BS_RIGHT));
		switch (iAlign)
		{
		case 0: dwStyle |= BS_LEFT; break;
		case 1: dwStyle |= BS_CENTER; break;
		case 2: dwStyle |= BS_RIGHT; break;
		}
	}
	else
	{
		dwStyle &= (~(BS_TOP | BS_VCENTER | BS_BOTTOM));
		switch (iAlign)
		{
		case 0: dwStyle |= BS_TOP; break;
		case 1: dwStyle |= BS_VCENTER; break;
		case 2: dwStyle |= BS_BOTTOM; break;
		}
	}
	SetStyle(dwStyle);
	Redraw();
}

int CButton::GetAlign(BOOL bHAlign)
{
	DWORD dwStyle = GetStyle();
	if (bHAlign)
	{
		if (IsBitSet(dwStyle, BS_LEFT))
			return 0;
		else if (IsBitSet(dwStyle, BS_CENTER))
			return 1;
		else if (IsBitSet(dwStyle, BS_RIGHT))
			return 2;
		else
			return 1;
	}
	else
	{
		if (IsBitSet(dwStyle, BS_TOP))
			return 0;
		else if (IsBitSet(dwStyle, BS_VCENTER))
			return 1;
		else if (IsBitSet(dwStyle, BS_BOTTOM))
			return 2;
		else
			return 1;
	}
}


HWND CPushButton::Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
	int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData)
{
	if (pData)
	{
		auto pBase = (const CREATEDATA_STD*)pData;
		auto p = (const CREATEDATA_PUSHBUTTON*)SkipBaseData(pData);
		if (pBase->iVer_Std != DATAVER_STD_1)
		{
			EckDbgBreak();
			return NULL;
		}

		m_hWnd = CreateWindowExW(pBase->dwExStyle, WC_BUTTONW, pBase->Text(), pBase->dwStyle,
			x, y, cx, cy, hParent, i32ToP<HMENU>(nID), NULL, NULL);

		switch (p->iVer)
		{
		case DATAVER_PUSHBUTTON_1:
			SetTextImageShowing(p->bShowTextAndImage);
			break;
		default:
			EckDbgBreak();
			break;
		}
	}
	else
	{
		dwStyle &= ~(BS_CHECKBOX | BS_COMMANDLINK | BS_DEFCOMMANDLINK);
		dwStyle |= WS_CHILD;
		if (!IsBitSet(dwStyle, BS_PUSHBUTTON | BS_DEFPUSHBUTTON | BS_SPLITBUTTON | BS_DEFSPLITBUTTON))
			dwStyle |= BS_PUSHBUTTON;
		m_hWnd = CreateWindowExW(dwExStyle, WC_BUTTONW, pszText, dwStyle,
			x, y, cx, cy, hParent, i32ToP<HMENU>(nID), NULL, NULL);
	}

	return m_hWnd;
}

CRefBin CPushButton::SerializeData(SIZE_T cbExtra, SIZE_T* pcbSize)
{
	SIZE_T cbBase;
	const SIZE_T cbSize = sizeof(CREATEDATA_PUSHBUTTON);
	auto rb = CWnd::SerializeData(cbSize + cbExtra, &cbBase);
	if (pcbSize)
		*pcbSize = cbBase + cbSize;

	CMemWriter w(rb.Data() + cbBase, cbSize);

	CREATEDATA_PUSHBUTTON* p;
	w.SkipPointer(p);
	p->iVer = DATAVER_PUSHBUTTON_1;
	p->bShowTextAndImage = GetTextImageShowing();

	return rb;
}

int CPushButton::GetType()
{
	DWORD dwStyle = GetStyle();
	if (IsBitSet(dwStyle, BS_SPLITBUTTON) || IsBitSet(dwStyle, BS_DEFSPLITBUTTON))
		return 1;
	else if (IsBitSet(dwStyle, BS_PUSHBUTTON) || IsBitSet(dwStyle, BS_DEFPUSHBUTTON))
		return 0;
	return -1;
}

void CPushButton::SetType(int iType)
{
	BOOL bDef = GetDef();
	DWORD dwStyle = GetStyle() & ~(BS_PUSHBUTTON | BS_SPLITBUTTON | BS_DEFPUSHBUTTON | BS_DEFSPLITBUTTON);

	switch (iType)
	{
	case 0:
		dwStyle |= (bDef ? BS_DEFPUSHBUTTON : BS_PUSHBUTTON);
		break;
	case 1:
		dwStyle |= (bDef ? BS_DEFSPLITBUTTON : BS_SPLITBUTTON);
		break;
	}

	SetStyle(dwStyle);
	Redraw();
}

void CPushButton::SetDef(BOOL bDef)
{
	int iType = GetType();
	DWORD dwStyle = GetStyle() & ~(BS_DEFPUSHBUTTON | BS_DEFSPLITBUTTON | BS_PUSHBUTTON | BS_SPLITBUTTON);
	if (bDef)
		if (iType)
			dwStyle |= BS_DEFSPLITBUTTON;
		else
			dwStyle |= BS_DEFPUSHBUTTON;
	else
		if (iType)
			dwStyle |= BS_SPLITBUTTON;
		else
			dwStyle |= BS_PUSHBUTTON;

	SetStyle(dwStyle);
}


HWND CCheckButton::Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
	int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData)
{
	if (pData)
	{
		auto pBase = (const CREATEDATA_STD*)pData;
		auto p = (const CREATEDATA_CHECKBUTTON*)SkipBaseData(pData);
		if (pBase->iVer_Std != DATAVER_STD_1)
		{
			EckDbgBreak();
			return NULL;
		}

		m_hWnd = CreateWindowExW(pBase->dwExStyle, WC_BUTTONW, pBase->Text(), pBase->dwStyle,
			x, y, cx, cy, hParent, i32ToP<HMENU>(nID), NULL, NULL);

		switch (p->iVer)
		{
		case DATAVER_CHECKBUTTON_1:
			SetTextImageShowing(p->bShowTextAndImage);
			SetCheckState(p->eCheckState);
			break;
		default:
			EckDbgBreak();
			break;
		}
	}
	else
	{
		dwStyle &= ~(BS_PUSHBUTTON | BS_DEFPUSHBUTTON | BS_SPLITBUTTON | BS_DEFSPLITBUTTON | BS_COMMANDLINK | BS_DEFCOMMANDLINK);
		dwStyle |= WS_CHILD;
		if (!IsBitSet(dwStyle, BS_RADIOBUTTON | BS_AUTORADIOBUTTON | BS_CHECKBOX | BS_AUTOCHECKBOX | BS_3STATE | BS_AUTO3STATE))
			dwStyle |= BS_AUTORADIOBUTTON;
		m_hWnd = CreateWindowExW(dwExStyle, WC_BUTTONW, pszText, dwStyle,
			x, y, cx, cy, hParent, i32ToP<HMENU>(nID), NULL, NULL);
	}

	/*
	* 有一个专用于单选按钮的状态叫做BST_DONTCLICK，
	* 如果未设置这个状态，那么按钮每次获得焦点都会产生BN_CLICKED，
	* 发送BM_SETDONTCLICK设置它防止事件错误生成
	*/
	SendMsg(BM_SETDONTCLICK, TRUE, 0);
	return m_hWnd;
}

CRefBin CCheckButton::SerializeData(SIZE_T cbExtra, SIZE_T* pcbSize)
{
	SIZE_T cbBase;
	const SIZE_T cbSize = sizeof(CREATEDATA_CHECKBUTTON);
	auto rb = CWnd::SerializeData(cbSize + cbExtra, &cbBase);
	if (pcbSize)
		*pcbSize = cbBase + cbSize;

	CMemWriter w(rb.Data() + cbBase, cbSize);

	CREATEDATA_CHECKBUTTON* p;
	w.SkipPointer(p);
	p->iVer = DATAVER_CHECKBUTTON_1;
	p->bShowTextAndImage = GetTextImageShowing();
	p->eCheckState = GetCheckState();

	return rb;
}

void CCheckButton::SetType(int iType)
{
	DWORD dwStyle = GetStyle() & ~(BS_AUTORADIOBUTTON | BS_AUTOCHECKBOX | BS_AUTO3STATE);
	switch (iType)
	{
	case 0:dwStyle |= BS_AUTORADIOBUTTON; break;
	case 1:dwStyle |= BS_AUTOCHECKBOX; break;
	case 2:dwStyle |= BS_AUTO3STATE; break;
	default:assert(FALSE); break;
	}
	SetStyle(dwStyle);
	Redraw();
}

int CCheckButton::GetType()
{
	DWORD dwStyle = GetStyle();
	if (IsBitSet(dwStyle, BS_AUTORADIOBUTTON))
		return 0;
	else if (IsBitSet(dwStyle, BS_AUTOCHECKBOX))
		return 1;
	else if (IsBitSet(dwStyle, BS_AUTO3STATE))
		return 2;
	else
		return -1;
}

void CCheckButton::SetCheckState(int iState)
{
	UINT uState;
	switch (iState)
	{
	case 0:uState = BST_UNCHECKED; break;
	case 1:uState = BST_CHECKED; break;
	case 2:uState = BST_INDETERMINATE; break;
	default:assert(FALSE); break;
	}
	SendMsg(BM_SETCHECK, uState, 0);
}

int CCheckButton::GetCheckState()
{
	UINT uState = (UINT)SendMsg(BM_GETCHECK, 0, 0);
	if (IsBitSet(uState, BST_CHECKED))
		return 1;
	else if (IsBitSet(uState, BST_INDETERMINATE))
		return 2;
	else
		return 0;
}


HWND CCommandLink::Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
	int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData)
{
	if (pData)
	{
		auto pBase = (const CREATEDATA_STD*)pData;
		auto p = (const CREATEDATA_COMMANDLINK*)SkipBaseData(pData);
		if (pBase->iVer_Std != DATAVER_STD_1)
		{
			EckDbgBreak();
			return NULL;
		}

		m_hWnd = CreateWindowExW(pBase->dwExStyle, WC_BUTTONW, pBase->Text(), pBase->dwStyle,
			x, y, cx, cy, hParent, i32ToP<HMENU>(nID), NULL, NULL);

		switch (p->iVer)
		{
		case DATAVER_COMMANDLINK_1:
			SetShieldIcon(p->bShieldIcon);
			SetNote(p->Note());
			break;
		default:
			EckDbgBreak();
			break;
		}
	}
	else
	{
		dwStyle &= ~(BS_PUSHBUTTON | BS_DEFPUSHBUTTON | BS_SPLITBUTTON | BS_DEFSPLITBUTTON | BS_CHECKBOX);
		dwStyle |= WS_CHILD;
		if (!IsBitSet(dwStyle, BS_COMMANDLINK | BS_DEFCOMMANDLINK))
			dwStyle |= BS_COMMANDLINK;
		m_hWnd = CreateWindowExW(dwExStyle, WC_BUTTONW, pszText, dwStyle,
			x, y, cx, cy, hParent, i32ToP<HMENU>(nID), NULL, NULL);
	}

	return m_hWnd;
}

CRefBin CCommandLink::SerializeData(SIZE_T cbExtra, SIZE_T* pcbSize)
{
	SIZE_T cbBase;
	auto rbNote = GetNote();
	const SIZE_T cbSize = sizeof(CREATEDATA_COMMANDLINK) + rbNote.ByteSize();
	auto rb = CWnd::SerializeData(cbSize + cbExtra, &cbBase);
	if (pcbSize)
		*pcbSize = cbBase + cbSize;

	CMemWriter w(rb.Data() + cbBase, cbSize);

	CREATEDATA_COMMANDLINK* p;
	w.SkipPointer(p);
	p->iVer = DATAVER_COMMANDLINK_1;
	p->cchNote = rbNote.Size();
	p->bShieldIcon = GetShieldIcon();

	w << rbNote;
	return rb;
}

CRefStrW CCommandLink::GetNote()
{
	CRefStrW rs;
	int cch = (int)SendMsg(BCM_GETNOTELENGTH, 0, 0);
	if (cch)
	{
		rs.ReSize(cch);
		++cch;
		SendMsg(BCM_GETNOTE, (WPARAM)&cch, (LPARAM)rs.Data());
	}
	return rs;
}

void CCommandLink::SetDef(BOOL bDef)
{
	DWORD dwStyle = GetStyle() & ~(BS_DEFPUSHBUTTON | BS_PUSHBUTTON | BS_DEFCOMMANDLINK | BS_COMMANDLINK);
	if (bDef)
		dwStyle |= BS_DEFCOMMANDLINK;
	else
		dwStyle |= BS_COMMANDLINK;

	SetStyle(dwStyle);
}

///////////////////////////////////////////////////////////////
CWnd* CALLBACK Create_Button(PCBYTE pData, ECK_CREATE_CTRL_EXTRA_ARGS)
{
	auto p = new CPushButton;
	p->Create(ECK_CREATE_CTRL_EXTRA_REALARGS, pData);
	if (p->GetHWND())
		return p;
	else
	{
		delete p;
		return NULL;
	}
}

CWnd* CALLBACK Create_CheckButton(PCBYTE pData, ECK_CREATE_CTRL_EXTRA_ARGS)
{
	auto p = new CCheckButton;
	p->Create(ECK_CREATE_CTRL_EXTRA_REALARGS, pData);
	if (p->GetHWND())
		return p;
	else
	{
		delete p;
		return NULL;
	}
}

CWnd* CALLBACK Create_CommandLink(PCBYTE pData, ECK_CREATE_CTRL_EXTRA_ARGS)
{
	auto p = new CCommandLink;
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
EckPropCallBackRet CALLBACK SetProp_Button(CWnd* pWnd, int idProp, EckCtrlPropValue* pProp)
{
	EckDCtrlDefSetProp;

	auto p = (CPushButton*)pWnd;
	switch (idProp)
	{
	case 1:
		p->SetAlign(TRUE, pProp->Vi);
		break;
	case 2:
		p->SetAlign(FALSE, pProp->Vi);
		break;
	case 3:
		break;
	case 4:
		p->SetTextImageShowing(pProp->Vb);
		break;
	case 5:
		p->SetDef(pProp->Vb);
		break;
	case 6:
		p->SetType(pProp->Vi);
		break;
	}
	return ESPR_NONE;
}

EckPropCallBackRet CALLBACK GetProp_Button(CWnd* pWnd, int idProp, EckCtrlPropValue* pProp)
{
	EckDCtrlDefGetProp;

	auto p = (CPushButton*)pWnd;
	switch (idProp)
	{
	case 1:
		pProp->Vi = p->GetAlign(TRUE);
		break;
	case 2:
		pProp->Vi = p->GetAlign(FALSE);
		break;
	case 3:
		break;
	case 4:
		pProp->Vb = p->GetTextImageShowing();
		break;
	case 5:
		pProp->Vb = p->GetDef();
		break;
	case 6:
		pProp->Vi = p->GetType();
		break;
	}
	return ESPR_NONE;
}

static EckCtrlPropEntry s_Prop_Button[]
{
	{1,L"AlignH",L"横向对齐",L"",ECPT::PickInt,ECPF_NONE,L"左边\0""居中\0""右边\0""\0"},
	{2,L"AlignV",L"纵向对齐",L"",ECPT::PickInt,ECPF_NONE,L"上边\0""居中\0""下边\0""\0"},
	{3,L"Image",L"图片",L"",ECPT::Image},
	{4,L"ShowImageAndText",L"同时显示图片和文本",L"",ECPT::Bool},
	{5,L"Default",L"默认",L"",ECPT::Bool},
	{6,L"Type",L"类型",L"",ECPT::PickInt,ECPF_NONE,L"普通按钮\0""拆分按钮\0""\0"},
};

EckCtrlDesignInfo CtInfoButton
{
	L"Button",
	L"按钮",
	L"",
	NULL,
	ARRAYSIZE(s_Prop_Button),
	s_Prop_Button,
	SetProp_Button,
	GetProp_Button,
	Create_Button,
	{80,32}
};
///////////////////////////////////////////////////////////////
EckPropCallBackRet CALLBACK SetProp_CheckButton(CWnd* pWnd, int idProp, EckCtrlPropValue* pProp)
{
	EckDCtrlDefSetProp;

	auto p = (CCheckButton*)pWnd;
	switch (idProp)
	{
	case 1:
		p->SetAlign(TRUE, pProp->Vi);
		break;
	case 2:
		p->SetAlign(FALSE, pProp->Vi);
		break;
	case 3:
		break;
	case 4:
		p->SetTextImageShowing(pProp->Vb);
		break;
	case 5:
		p->SetType(pProp->Vi);
		break;
	case 6:
		p->SetCheckState(pProp->Vi);
		break;
	case 7:
		p->SetFlat(pProp->Vb);
		break;
	case 8:
		p->SetPushLike(pProp->Vb);
		break;
	case 9:
		p->SetLeftText(pProp->Vb);
		break;
	}
	return ESPR_NONE;
}

EckPropCallBackRet CALLBACK GetProp_CheckButton(CWnd* pWnd, int idProp, EckCtrlPropValue* pProp)
{
	EckDCtrlDefGetProp;

	auto p = (CCheckButton*)pWnd;
	switch (idProp)
	{
	case 1:
		pProp->Vi = p->GetAlign(TRUE);
		break;
	case 2:
		pProp->Vi = p->GetAlign(FALSE);
		break;
	case 3:
		break;
	case 4:
		pProp->Vb = p->GetTextImageShowing();
		break;
	case 5:
		pProp->Vi = p->GetType();
		break;
	case 6:
		pProp->Vi = p->GetCheckState();
		break;
	case 7:
		pProp->Vb = p->GetFlat();
		break;
	case 8:
		pProp->Vb = p->GetPushLike();
		break;
	case 9:
		pProp->Vb = p->GetLeftText();
		break;
	}
	return ESPR_NONE;
}

static EckCtrlPropEntry s_Prop_CheckButton[]
{
	{1,L"AlignH",L"横向对齐",L"",ECPT::PickInt,ECPF_NONE,L"左边\0""居中\0""右边\0""\0"},
	{2,L"AlignV",L"纵向对齐",L"",ECPT::PickInt,ECPF_NONE,L"上边\0""居中\0""下边\0""\0"},
	{3,L"Image",L"图片",L"",ECPT::Image},
	{4,L"ShowImageAndText",L"同时显示图片和文本",L"",ECPT::Bool},
	{5,L"Type",L"类型",L"",ECPT::PickInt,ECPF_NONE,L"单选框\0""复选框\0""三态复选框\0""\0"},
	{6,L"Checked",L"选中",L"",ECPT::PickInt,ECPF_NONE,L"未选中\0""选中\0""半选中\0""\0"},
	{7,L"Flat",L"平面",L"",ECPT::Bool},
	{8,L"ButtonLike",L"按钮形式",L"",ECPT::Bool},
	{9,L"LeftText",L"文本居左",L"",ECPT::Bool},
};

EckCtrlDesignInfo CtInfoCheckButton
{
	L"CheckButton",
	L"选择框",
	L"",
	NULL,
	ARRAYSIZE(s_Prop_CheckButton),
	s_Prop_CheckButton,
	SetProp_CheckButton,
	GetProp_CheckButton,
	Create_CheckButton,
	{112,24}
};
///////////////////////////////////////////////////////////////
EckPropCallBackRet CALLBACK SetProp_CommandLink(CWnd* pWnd, int idProp, EckCtrlPropValue* pProp)
{
	EckDCtrlDefSetProp;

	auto p = (CCommandLink*)pWnd;
	switch (idProp)
	{
	case 1:
		break;
	case 2:
		p->SetNote(pProp->Vpsz);
		break;
	case 3:
		p->SetShieldIcon(pProp->Vb);
		break;
	}
	return ESPR_NONE;
}

EckPropCallBackRet CALLBACK GetProp_CommandLink(CWnd* pWnd, int idProp, EckCtrlPropValue* pProp)
{
	EckDCtrlDefGetProp;

	auto p = (CCommandLink*)pWnd;
	switch (idProp)
	{
	case 1:
		break;
	case 2:
	{
		auto rs = p->GetNote();
		if (rs.Data())
		{
			pProp->Vpsz = (PWSTR)TDesignAlloc::Alloc(rs.ByteSize());
			rs.CopyTo(pProp->Vpsz);
		}
		else
		{
			pProp->Vpsz = NULL;
		}
	}
	return ESPR_NEEDFREE;
	case 3:
		pProp->Vb = p->GetShieldIcon();
		break;
	}
	return ESPR_NONE;
}

static EckCtrlPropEntry s_Prop_CommandLink[]
{
	{1,L"Image",L"图片",L"",ECPT::Image},
	{2,L"Note",L"注释文本",L"",ECPT::Text},
	{3,L"ShieldIcon",L"UAC图标",L"",ECPT::Bool},
};

EckCtrlDesignInfo CtInfoCommandLink
{
	L"CommandLink",
	L"命令链接",
	L"",
	NULL,
	ARRAYSIZE(s_Prop_CommandLink),
	s_Prop_CommandLink,
	SetProp_CommandLink,
	GetProp_CommandLink,
	Create_CommandLink,
	{192,48}
};
ECK_NAMESPACE_END
#else
ECK_NAMESPACE_BEGIN
EckCtrlDesignInfo CtInfoButton{ Create_Button };
EckCtrlDesignInfo CtInfoCheckButton{ Create_CheckButton };
EckCtrlDesignInfo CtInfoCommandLink{ Create_CommandLink };
ECK_NAMESPACE_END
#endif // ECK_CTRL_DESIGN_INTERFACE