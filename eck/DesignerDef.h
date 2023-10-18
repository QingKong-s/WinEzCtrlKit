#pragma once
#include "ECK.h"
#include "WndHelper.h"
#include "CWnd.h"
#include "CAllocator.h"

ECK_NAMESPACE_BEGIN
#define ECK_CREATE_CTRL_EXTRA_ARGS PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle, \
									int x, int y, int cx, int cy, HWND hParent, UINT nID
#define ECK_CREATE_CTRL_EXTRA_REALARGS pszText, dwStyle, dwExStyle, x, y, cx, cy, hParent, nID
// �����ؼ�
typedef CWnd* (CALLBACK* ECKCICreate)(PCBYTE pData, ECK_CREATE_CTRL_EXTRA_ARGS);

ECK_NAMESPACE_END



#ifdef ECK_CTRL_DESIGN_INTERFACE
ECK_NAMESPACE_BEGIN

enum class EckCtrlPropType
{
	Int = 1,
	Float = 2,
	Double = 3,
	Bool = 4,
	DateTime = 5,
	Text = 6,
	PickInt = 7,
	PickText = 8,
	Color = 9,
	Font = 10,
	ImageList = 11,
	Customize = 12,
	Image = 13,
};
using ECPT = EckCtrlPropType;

enum EckCtrlPropFlags
{
	ECPF_NONE = 0,
	ECPF_HASINDENT = 1 << 0,
	ECPF_READONLY = 1 << 1,
	ECPF_CANNOTINIT = 1 << 2
};

struct EckCtrlPropEntry
{
	int iID;				// ����ID��֪ͨ�ӿ�ʹ��ID��ʶ���Զ�������
	PCWSTR pszName;         // ����
	PCWSTR pszChsName;      // ��������
	PCWSTR pszDesc;         // ����
	EckCtrlPropType Type;	// ����
	EckCtrlPropFlags uFlags;// ��־
	PCWSTR pszPickStr;		// ��ѡ�ַ���
};

union EckCtrlPropValue
{
	int Vi;
	float Vf;
	double Vlf;
	BOOL Vb;
	DATE Vdate;
	COLORREF Vclr;
	PWSTR Vpsz;
	struct
	{
		BYTE* pData;
		SIZE_T cbSize;

		auto& operator=(const CRefBin& x)
		{
			pData = x.m_pStream;
			cbSize = x.m_cb;
			return *this;
		}
	} Vbin;
};

enum EckPropCallBackRet
{
	ESPR_NONE = 0,// ��
	ESPR_RESETCTRL = 1 << 0,// �ؼ���Ҫ�����´�����������ECKCISetProp
	ESPR_NEEDFREE = 1 << 1// ���÷�Ӧ�ͷŷ��ص��ڴ�
};
// ֪ͨĳ���Ա��޸�
typedef EckPropCallBackRet(CALLBACK* ECKCISetProp)(CWnd* pWnd, int idProp, EckCtrlPropValue* pProp);
// ȡĳ����
typedef EckPropCallBackRet(CALLBACK* ECKCIGetProp)(CWnd* pWnd, int idProp, EckCtrlPropValue* pProp);

struct EckCtrlDesignInfo
{
	PCWSTR pszName;         // ����
	PCWSTR pszChsName;      // ��������
	PCWSTR pszDesc;         // ����
	PCWSTR pszIcon;         // ͼ�꣬����Ϊ��ԴID���ļ������������ʹ��LoadImage����ͼ��
	SIZE_T cProp;			// ������Ŀ
	EckCtrlPropEntry* pProp;// ������Ŀ
	ECKCISetProp pfnSetProp;// ������֪ͨ�ӿ�
	ECKCIGetProp pfnGetProp;// ȡ����֪ͨ�ӿ�
	ECKCICreate pfnCreate;	// �����ؼ��ӿ�
	SIZE sizeDef;			// Ĭ�ϳߴ磬��Ҫ����DPI����
};

enum COMMPROPID
{
	CPID_NAME = -1,
	CPID_LEFT = -2,
	CPID_TOP = -3,
	CPID_WIDTH = -4,
	CPID_HEIGHT = -5,
	CPID_TEXT = -6,
	CPID_VISIBLE = -7,
	CPID_ENABLE = -8,
	CPID_FONT = -9,
	CPID_FRAMETYPE = -10,
	CPID_SCROLLBAR = -11,
};

#define EckCPIDToIndex(cpid) ((-(cpid)) - 1)

static EckCtrlPropEntry s_CommProp[] =
{
	{CPID_NAME,L"Name",L"����",L"",ECPT::Text},
	{CPID_LEFT,L"Left",L"���",L"",ECPT::Int},
	{CPID_TOP,L"Top",L"����",L"",ECPT::Int},
	{CPID_WIDTH,L"Width",L"���",L"",ECPT::Int},
	{CPID_HEIGHT,L"Height",L"�߶�",L"",ECPT::Int},
	{CPID_TEXT,L"Text",L"�ı�",L"",ECPT::Text},
	{CPID_VISIBLE,L"Visible",L"����",L"",ECPT::Bool},
	{CPID_ENABLE,L"Enable",L"��ֹ",L"",ECPT::Bool},
	{CPID_FONT,L"Font",L"����",L"",ECPT::Font},
	{CPID_FRAMETYPE,L"Frame",L"�߿�",L"",ECPT::PickInt,ECPF_NONE,L"�ޱ߿�\0����ʽ\0͹��ʽ\0ǳ����ʽ\0����ʽ\0���߱߿�ʽ\0\0"},
	{CPID_SCROLLBAR,L"ScrollBar",L"������",L"",ECPT::PickInt,ECPF_NONE,L"��\0���������\0���������\0�������������\0\0"},
};



using TDesignAlloc = CAllocator<BYTE>;

EckPropCallBackRet CALLBACK SetProp_Common(CWnd* pWnd, int idProp, EckCtrlPropValue* pProp, BOOL* pbProcessed);

EckPropCallBackRet CALLBACK GetProp_Common(CWnd* pWnd, int idProp, EckCtrlPropValue* pProp, BOOL* pbProcessed);

#define EckDCtrlDefSetProp \
					BOOL bDefProcessed; \
					auto iDefRet = SetProp_Common(pWnd, idProp, pProp, &bDefProcessed); \
					if (bDefProcessed) \
						return iDefRet;
#define EckDCtrlDefGetProp \
					BOOL bDefProcessed; \
					auto iDefRet = GetProp_Common(pWnd, idProp, pProp, &bDefProcessed); \
					if (bDefProcessed) \
						return iDefRet;
ECK_NAMESPACE_END
#else
ECK_NAMESPACE_BEGIN
struct EckCtrlDesignInfo
{
	ECKCICreate pfnCreate;
};
ECK_NAMESPACE_END
#endif // ECK_CTRL_DESIGN_INTERFACE




ECK_NAMESPACE_BEGIN
#define ECK_ALL_CTRL_CLASS \
CtInfoButton \
,CtInfoCheckButton \
,CtInfoCommandLink \
,CtInfoEdit \



extern EckCtrlDesignInfo
ECK_ALL_CTRL_CLASS;

static EckCtrlDesignInfo s_EckDesignAllCtrl[] = { ECK_ALL_CTRL_CLASS };

ECK_NAMESPACE_END