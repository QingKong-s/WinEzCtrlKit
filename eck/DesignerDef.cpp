#include "DesignerDef.h"
#include "CButton.h"
#include "CEdit.h"

ECK_NAMESPACE_BEGIN
CWnd* CALLBACK Create_Button(PCBYTE pData, ECK_CREATE_CTRL_EXTRA_ARGS)
{
	auto p = new CPushButton;
	p->Create(ECK_CREATE_CTRL_EXTRA_REALARGS, pData);
	if (p->GetHWND())
		return p;
	else
	{
		delete p;
		return nullptr;
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
		return nullptr;
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
		return nullptr;
	}
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
		return nullptr;
	}
}
ECK_NAMESPACE_END
#ifdef ECK_CTRL_DESIGN_INTERFACE
ECK_NAMESPACE_BEGIN
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
	{
		*pbProcessed = TRUE;
		int dummy;
		pProp->Vpsz = pWnd->GetText().Detach(dummy, dummy);
	}
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
		pProp->Vi = (int)pWnd->GetFrameType();
		break;
	case CPID_SCROLLBAR:
		*pbProcessed = TRUE;
		pProp->Vi = (int)pWnd->GetScrollBar();
		break;
	}
	return ESPR_NONE;
}

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
		pProp->Vb = FALSE;
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
		//p->SetFlat(pProp->Vb);
		break;
	case 8:
		//p->SetPushLike(pProp->Vb);
		break;
	case 9:
		//p->SetLeftText(pProp->Vb);
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
		pProp->Vb = 0;// p->GetTextImageShowing();
		break;
	case 5:
		pProp->Vi = p->GetType();
		break;
	case 6:
		pProp->Vi = p->GetCheckState();
		break;
	case 7:
		pProp->Vb = p->StyleGetFlat();
		break;
	case 8:
		pProp->Vb = p->StyleGetPushLike();
		break;
	case 9:
		pProp->Vb = p->StyleGetAlignLeft();
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
		pProp->Vb = 0;// p->GetShieldIcon();
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
///////////////////////////////////////////////////////////////
EckPropCallBackRet CALLBACK SetProp_Edit(CWnd* pWnd, int idProp, EckCtrlPropValue* pProp)
{
	EckDCtrlDefSetProp;

	auto p = (CEdit*)pWnd;
	//switch (idProp)
	//{
	//}
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
EckCtrlDesignInfo CtInfoButton{ Create_Button };
EckCtrlDesignInfo CtInfoCheckButton{ Create_CheckButton };
EckCtrlDesignInfo CtInfoCommandLink{ Create_CommandLink };
EckCtrlDesignInfo CtInfoEdit{ Create_Edit };
ECK_NAMESPACE_END
#endif // ECK_CTRL_DESIGN_INTERFACE
