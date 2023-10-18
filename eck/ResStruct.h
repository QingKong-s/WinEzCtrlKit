#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
#pragma pack(push, ECK_CTRLDATA_ALIGN)
static constexpr int
DATAVER_RESTABLE_1 = 1,
DATAVER_FORMTABLE_1 = 1;


// 资源表 资源类型
enum class ResType
{
	Others		= 0,
	Sound		= 1,
	Image		= 2,
	ImageList	= 3,
	Font		= 4,
};

// 资源表 头
struct RESTABLEHEADER
{
	int iVer;			// 版本
	int cResources;		// 资源数
};

// 资源表 索引
struct RESTABLE_INDEXENTRY
{
	int iID;			// ID
	UINT uOffset;		// 资源数据相对资源表数据起始地址的偏移
	UINT cbSize;		// 资源数据大小
	ECKENUM eType;		// 资源类型
};

// 窗体表 头
struct FORMTABLEHEADER
{
	int iVer;			// 版本
	int cForms;			// 窗体数
};

// 窗体表 索引
struct FORMTABLE_INDEXENTRY
{
	int iID;			// 窗体ID
	UINT uOffset;		// 窗体数据相对窗体表起始地址的偏移
	UINT cbSize;		// 窗体数据大小
};

// 窗体表 窗体信息 头
struct FTFORMDATAHEADER
{
	int iVer;			// 版本
	POINT xyOffset;		// x、y坐标最小的控件在客户区的坐标
	UINT cbData;		// byData的大小
	// BYTE byData[];	// CWnd或CForm的序列化数据
};

// 窗体表 控件信息 头
struct FTCTRLDATAHEADER
{
	int iVer;			// 版本
	int cCtrls;			// 控件数
};

// 窗体表 单个控件信息
struct FTCTRLDATA
{
	UINT cbData;		// byData的大小
	int cChildren;		// 字控件数
	int idxInfo;		// 类索引
	RECT rc;			// 位置，左顶宽高，相对于x、y坐标最小的控件
	// BYTE byData[];	// 控件类的序列化数据
};

/*
资源表：
--------------
RESTABLEHEADER
{
	RESTABLE_INDEXENTRY
}
{
	资源数据
}
--------------


窗体序列化数据（包含窗体上的所有控件）：
--------------
FTCTRLDATAHEADER
{
	FTCTRLDATA
	控件类序列化数据
}
--------------


窗体表（包含一个或多个窗体）：
--------------
FORMTABLEHEADER
{
	FORMTABLE_INDEXENTRY
}
{
	FTFORMDATAHEADER
	FTCTRLDATAHEADER
	{
		FTCTRLDATA
		控件类序列化数据
	}
}
--------------
*/
#pragma pack(pop)
ECK_NAMESPACE_END