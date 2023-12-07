#include "DesignerDef.h"

#include "CButton.h"

ECK_NAMESPACE_BEGIN
#ifdef ECK_CTRL_DESIGN_INTERFACE
EckPropCallBackRet CALLBACK SetProp_Common(CWnd* pWnd, int idProp, EckCtrlPropValue* pProp, BOOL* pbProcessed)
{
	*pbProcessed = FALSE;
	switch (idProp)
	{
	case CPID_NAME:
	{
		*pbProcessed = TRUE;
		pWnd->m_DDBase.rsName = pProp->Vpsz;
	}
	break;
	case CPID_LEFT:
	{
		*pbProcessed = TRUE;
		HWND hWnd = pWnd->GetHWND();
		RECT rc;
		GetWindowRect(hWnd, &rc);
		ScreenToClient(GetParent(hWnd), &rc);
		rc.left = eck::DpiScale(pProp->Vi, eck::GetDpi(pWnd->GetHWND()), USER_DEFAULT_SCREEN_DPI);
		SetWindowPos(hWnd, NULL, rc.left, rc.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	}
	break;
	case CPID_TOP:
	{
		*pbProcessed = TRUE;
		HWND hWnd = pWnd->GetHWND();
		RECT rc;
		GetWindowRect(hWnd, &rc);
		ScreenToClient(GetParent(hWnd), &rc);
		rc.top = eck::DpiScale(pProp->Vi, eck::GetDpi(pWnd->GetHWND()), USER_DEFAULT_SCREEN_DPI);
		SetWindowPos(hWnd, NULL, rc.left, rc.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	}
	break;
	case CPID_WIDTH:
	{
		*pbProcessed = TRUE;
		HWND hWnd = pWnd->GetHWND();
		RECT rc;
		GetWindowRect(hWnd, &rc);
		ScreenToClient(GetParent(hWnd), &rc);
		rc.right = eck::DpiScale(pProp->Vi, eck::GetDpi(pWnd->GetHWND()), USER_DEFAULT_SCREEN_DPI);
		rc.bottom -= rc.top;
		SetWindowPos(hWnd, NULL, 0, 0, rc.right, rc.bottom, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
	}
	break;
	case CPID_HEIGHT:
	{
		*pbProcessed = TRUE;
		HWND hWnd = pWnd->GetHWND();
		RECT rc;
		GetWindowRect(hWnd, &rc);
		ScreenToClient(GetParent(hWnd), &rc);
		rc.bottom = eck::DpiScale(pProp->Vi, eck::GetDpi(pWnd->GetHWND()), USER_DEFAULT_SCREEN_DPI);
		rc.right -= rc.left;
		SetWindowPos(hWnd, NULL, 0, 0, rc.right, rc.bottom, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
	}
	break;
	case CPID_TEXT:
		*pbProcessed = TRUE;
		pWnd->SetText(pProp->Vpsz);
		break;
	case CPID_VISIBLE:
		*pbProcessed = TRUE;
		pWnd->m_DDBase.bVisible = pProp->Vb;
		break;
	case CPID_ENABLE:
		*pbProcessed = TRUE;
		pWnd->m_DDBase.bEnable = pProp->Vb;
		break;
	case CPID_FRAMETYPE:
		*pbProcessed = TRUE;
		pWnd->SetFrameType(pProp->Vi);
		pWnd->FrameChanged();
		break;
	case CPID_SCROLLBAR:
		*pbProcessed = TRUE;
		pWnd->SetScrollBar(pProp->Vi);
		break;
	}
	return ESPR_NONE;
}

EckPropCallBackRet CALLBACK GetProp_Common(CWnd* pWnd, int idProp, EckCtrlPropValue* pProp, BOOL* pbProcessed)
{
	*pbProcessed = FALSE;
	switch (idProp)
	{
	case CPID_NAME:
	{
		auto& rsName = pWnd->m_DDBase.rsName;
		PWSTR p = (PWSTR)TDesignAlloc::Alloc(rsName.ByteSize());
		rsName.CopyTo(p);
		pProp->Vpsz = p;
	}
	return ESPR_NEEDFREE;
	case CPID_LEFT:
	{
		*pbProcessed = TRUE;
		HWND hWnd = pWnd->GetHWND();
		RECT rc;
		GetWindowRect(hWnd, &rc);
		ScreenToClient(GetParent(hWnd), &rc);
		pProp->Vi = eck::DpiScale(rc.left, USER_DEFAULT_SCREEN_DPI, eck::GetDpi(pWnd->GetHWND()));
	}
	break;
	case CPID_TOP:
	{
		*pbProcessed = TRUE;
		HWND hWnd = pWnd->GetHWND();
		RECT rc;
		GetWindowRect(hWnd, &rc);
		ScreenToClient(GetParent(hWnd), &rc);
		pProp->Vi = eck::DpiScale(rc.top, USER_DEFAULT_SCREEN_DPI, eck::GetDpi(pWnd->GetHWND()));
	}
	break;
	case CPID_WIDTH:
	{
		*pbProcessed = TRUE;
		HWND hWnd = pWnd->GetHWND();
		RECT rc;
		GetWindowRect(hWnd, &rc);
		ScreenToClient(GetParent(hWnd), &rc);
		pProp->Vi = eck::DpiScale(rc.right - rc.left, USER_DEFAULT_SCREEN_DPI, eck::GetDpi(pWnd->GetHWND()));
	}
	break;
	case CPID_HEIGHT:
	{
		*pbProcessed = TRUE;
		HWND hWnd = pWnd->GetHWND();
		RECT rc;
		GetWindowRect(hWnd, &rc);
		ScreenToClient(GetParent(hWnd), &rc);
		pProp->Vi = eck::DpiScale(rc.bottom - rc.top, USER_DEFAULT_SCREEN_DPI, eck::GetDpi(pWnd->GetHWND()));
	}
	break;
	case CPID_TEXT:
		*pbProcessed = TRUE;
		pProp->Vpsz = pWnd->GetText().Detach();
		return ESPR_NEEDFREE;
	case CPID_VISIBLE:
		*pbProcessed = TRUE;
		pProp->Vb = pWnd->m_DDBase.bVisible;
		break;
	case CPID_ENABLE:
		*pbProcessed = TRUE;
		pProp->Vb = pWnd->m_DDBase.bEnable;
		break;
	case CPID_FRAMETYPE:
		*pbProcessed = TRUE;
		pProp->Vi = pWnd->GetFrameType();
		break;
	case CPID_SCROLLBAR:
		*pbProcessed = TRUE;
		pProp->Vi = pWnd->GetScrollBar();
		break;
	}
	return ESPR_NONE;
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
		p->SetAlign(TRUE, (Align)pProp->Vi);
		break;
	case 2:
		p->SetAlign(FALSE, (Align)pProp->Vi);
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
		pProp->Vi = (int)p->GetAlign(TRUE);
		break;
	case 2:
		pProp->Vi = (int)p->GetAlign(FALSE);
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
		p->SetAlign(TRUE, (Align)pProp->Vi);
		break;
	case 2:
		p->SetAlign(FALSE, (Align)pProp->Vi);
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
		pProp->Vi = (int)p->GetAlign(TRUE);
		break;
	case 2:
		pProp->Vi = (int)p->GetAlign(FALSE);
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

#else
ECK_NAMESPACE_BEGIN
EckCtrlDesignInfo CtInfoButton{ Create_Button };
EckCtrlDesignInfo CtInfoCheckButton{ Create_CheckButton };
EckCtrlDesignInfo CtInfoCommandLink{ Create_CommandLink };
ECK_NAMESPACE_END
#endif // ECK_CTRL_DESIGN_INTERFACE


#endif // ECK_CTRL_DESIGN_INTERFACE
ECK_NAMESPACE_END