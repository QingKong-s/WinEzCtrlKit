#pragma once
#include "ECK.h"
#include "WndHelper.h"
#include "CWnd.h"
#include "CAllocator.h"

ECK_NAMESPACE_BEGIN
#define ECK_CREATE_CTRL_EXTRA_ARGS PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle, \
									int x, int y, int cx, int cy, HWND hParent, UINT nID
#define ECK_CREATE_CTRL_EXTRA_REALARGS pszText, dwStyle, dwExStyle, x, y, cx, cy, hParent, nID
// 创建控件
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
	int iID;				// 属性ID，通知接口使用ID标识属性而非索引
	PCWSTR pszName;         // 名称
	PCWSTR pszChsName;      // 中文名称
	PCWSTR pszDesc;         // 描述
	EckCtrlPropType Type;	// 类型
	EckCtrlPropFlags uFlags;// 标志
	PCWSTR pszPickStr;		// 候选字符串
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

		auto& operator=(CRefBin& x)
		{
			pData = x.Data();
			cbSize = x.Size();
			return *this;
		}
	} Vbin;
};

enum EckPropCallBackRet
{
	ESPR_NONE = 0,// 无
	ESPR_RESETCTRL = 1 << 0,// 控件需要被重新创建，仅用于ECKCISetProp
	ESPR_NEEDFREE = 1 << 1// 调用方应释放返回的内存
};
// 通知某属性被修改
typedef EckPropCallBackRet(CALLBACK* ECKCISetProp)(CWnd* pWnd, int idProp, EckCtrlPropValue* pProp);
// 取某属性
typedef EckPropCallBackRet(CALLBACK* ECKCIGetProp)(CWnd* pWnd, int idProp, EckCtrlPropValue* pProp);

struct EckCtrlDesignInfo
{
	PCWSTR pszName;         // 名称
	PCWSTR pszChsName;      // 中文名称
	PCWSTR pszDesc;         // 描述
	PCWSTR pszIcon;         // 图标，可以为资源ID或文件名，设计器将使用LoadImage加载图标
	SIZE_T cProp;			// 属性数目
	EckCtrlPropEntry* pProp;// 属性条目
	ECKCISetProp pfnSetProp;// 置属性通知接口
	ECKCIGetProp pfnGetProp;// 取属性通知接口
	ECKCICreate pfnCreate;	// 创建控件接口
	SIZE sizeDef;			// 默认尺寸，不要进行DPI缩放
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
	{CPID_NAME,L"Name",L"名称",L"",ECPT::Text},
	{CPID_LEFT,L"Left",L"左边",L"",ECPT::Int},
	{CPID_TOP,L"Top",L"顶边",L"",ECPT::Int},
	{CPID_WIDTH,L"Width",L"宽度",L"",ECPT::Int},
	{CPID_HEIGHT,L"Height",L"高度",L"",ECPT::Int},
	{CPID_TEXT,L"Text",L"文本",L"",ECPT::Text},
	{CPID_VISIBLE,L"Visible",L"可视",L"",ECPT::Bool},
	{CPID_ENABLE,L"Enable",L"禁止",L"",ECPT::Bool},
	{CPID_FONT,L"Font",L"字体",L"",ECPT::Font},
	{CPID_FRAMETYPE,L"Frame",L"边框",L"",ECPT::PickInt,ECPF_NONE,L"无边框\0凹入式\0凸出式\0浅凹入式\0镜框式\0单线边框式\0\0"},
	{CPID_SCROLLBAR,L"ScrollBar",L"滚动条",L"",ECPT::PickInt,ECPF_NONE,L"无\0横向滚动条\0纵向滚动条\0横向及纵向滚动条\0\0"},
};

template<class T, class TSize = SIZE_T>
struct CAllocator1
{
	static T* Alloc(TSize c)
	{
		return (T*)HeapAlloc(GetProcessHeap(), 0, c * sizeof(T));
	}

	static void Free(T* p)
	{
		std::allocator<T>().deallocate(p, 0);
	}
};

using TDesignAlloc = CAllocator1<BYTE>;

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