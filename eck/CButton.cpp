#include "CButton.h"

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
		if (IsBitSet(dwStyle, BS_CENTER))
			return CACenter;
		else if (IsBitSet(dwStyle, BS_RIGHT))
			return CARight;
		return CALeft;
	}
	else
	{
		if (IsBitSet(dwStyle, BS_VCENTER))
			return CAVCenter;
		else if (IsBitSet(dwStyle, BS_BOTTOM))
			return CARight;
		return CALeft;
	}
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
ECK_NAMESPACE_END

#ifdef ECK_CTRL_DESIGN_INTERFACE
#include "DesignerDef.h"

ECK_NAMESPACE_BEGIN
EckPropCallBackRet CALLBACK SetProp_Button(CWnd* pWnd, int idProp, EckCtrlPropValue* pProp)
{
	auto p = (CButton*)pWnd;
	return ESPR_NONE;
}

EckPropCallBackRet CALLBACK GetProp_Button(CWnd* pWnd, int idProp, EckCtrlPropValue* pProp)
{

	return ESPR_NONE;
}

CWnd* CALLBACK Create_Button(BYTE* pData, ECK_CREATE_CTRL_EXTRA_ARGS)
{
	auto p = new CPushButton;
	p->Create(ECK_CREATE_CTRL_EXTRA_REALARGS);
	if (p->GetHWND())
		return p;
	else
	{
		delete p;
		return NULL;
	}
}

static EckCtrlPropEntry s_Prop_Button[] =
{
	{2,L"AlignH",L"�������",L"",ECPT::PickInt,ECPF_NONE,L"���\0""����\0""�ұ�\0""\0"},
	{3,L"AlignV",L"�������",L"",ECPT::PickInt,ECPF_NONE,L"�ϱ�\0""����\0""�±�\0""\0"},
	{4,L"Image",L"ͼƬ",L"",ECPT::Image},
	{5,L"ShowImageAndText",L"ͬʱ��ʾͼƬ���ı�",L"",ECPT::Bool},
	{7,L"Default",L"Ĭ��",L"",ECPT::Bool,ECPF_NONE},
	{8,L"Type",L"����",L"",ECPT::PickInt,ECPF_NONE,L"��ͨ��ť\0""��ְ�ť\0""\0"},
};

EckCtrlDesignInfo CtInfoButton =
{
	L"Button",
	L"��ť",
	L"",
	NULL,
	ARRAYSIZE(s_Prop_Button),
	s_Prop_Button,
	SetProp_Button,
	GetProp_Button,
	Create_Button,
	{140,40}
};




EckPropCallBackRet CALLBACK SetProp_CheckButton(CWnd* pWnd, int idProp, EckCtrlPropValue* pProp)
{

	return ESPR_NONE;
}

EckPropCallBackRet CALLBACK GetProp_CheckButton(CWnd* pWnd, int idProp, EckCtrlPropValue* pProp)
{

	return ESPR_NONE;
}

CWnd* CALLBACK Create_CheckButton(BYTE* pData, ECK_CREATE_CTRL_EXTRA_ARGS)
{
	auto p = new CCheckButton;
	p->Create(ECK_CREATE_CTRL_EXTRA_REALARGS);
	if (p->GetHWND())
		return p;
	else
	{
		delete p;
		return NULL;
	}
}

static EckCtrlPropEntry s_Prop_CheckButton[] =
{
	{2,L"AlignH",L"�������",L"",ECPT::PickInt,ECPF_NONE,L"���\0""����\0""�ұ�\0""\0"},
	{3,L"AlignV",L"�������",L"",ECPT::PickInt,ECPF_NONE,L"�ϱ�\0""����\0""�±�\0""\0"},
	{4,L"Image",L"ͼƬ",L"",ECPT::Image},
	{5,L"ShowImageAndText",L"ͬʱ��ʾͼƬ���ı�",L"",ECPT::Bool},
	{8,L"Type",L"����",L"",ECPT::PickInt,ECPF_NONE,L"��ѡ��\0""��ѡ��\0""��̬��ѡ��\0""\0"},
	{8,L"Checked",L"ѡ��",L"",ECPT::Bool,ECPF_NONE},
	{8,L"Flat",L"ƽ��",L"",ECPT::Bool,ECPF_NONE},
	{8,L"ButtonLike",L"��ť��ʽ",L"",ECPT::Bool,ECPF_NONE},
	{8,L"LeftText",L"�ı�����",L"",ECPT::Bool,ECPF_NONE},
};

EckCtrlDesignInfo CtInfoCheckButton =
{
	L"CheckButton",
	L"ѡ���",
	L"",
	NULL,
	ARRAYSIZE(s_Prop_CheckButton),
	s_Prop_CheckButton,
	SetProp_CheckButton,
	GetProp_CheckButton,
	Create_CheckButton,
	{140,40}
};




EckPropCallBackRet CALLBACK SetProp_CommandLink(CWnd* pWnd, int idProp, EckCtrlPropValue* pProp)
{

	return ESPR_NONE;
}

EckPropCallBackRet CALLBACK GetProp_CommandLink(CWnd* pWnd, int idProp, EckCtrlPropValue* pProp)
{

	return ESPR_NONE;
}

CWnd* CALLBACK Create_CommandLink(BYTE* pData, ECK_CREATE_CTRL_EXTRA_ARGS)
{
	auto p = new CCommandLink;
	p->Create(ECK_CREATE_CTRL_EXTRA_REALARGS);
	if (p->GetHWND())
		return p;
	else
	{
		delete p;
		return NULL;
	}
}

static EckCtrlPropEntry s_Prop_CommandLink[] =
{
	{2,L"AlignH",L"�������",L"",ECPT::PickInt,ECPF_NONE,L"���\0""����\0""�ұ�\0""\0"},
	{3,L"AlignV",L"�������",L"",ECPT::PickInt,ECPF_NONE,L"�ϱ�\0""����\0""�±�\0""\0"},
	{4,L"Image",L"ͼƬ",L"",ECPT::Image},
	{7,L"Note",L"ע���ı�",L"",ECPT::Text,ECPF_NONE},
	{8,L"ShieldIcon",L"����ͼ��",L"",ECPT::Bool,ECPF_NONE},
};

EckCtrlDesignInfo CtInfoCommandLink =
{
	L"CommandLink",
	L"��������",
	L"",
	NULL,
	ARRAYSIZE(s_Prop_CommandLink),
	s_Prop_CommandLink,
	SetProp_CommandLink,
	GetProp_CommandLink,
	Create_CommandLink,
	{140,40}
};
ECK_NAMESPACE_END

#endif // ECK_CTRL_DESIGN_INTERFACE