#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
#pragma pack(push, ECK_CTRLDATA_ALIGN)
inline constexpr int
DATAVER_RESTABLE_1 = 1,
DATAVER_PUSHBUTTON_1 = 1,
DATAVER_CHECKBUTTON_1 = 1,
DATAVER_COMMANDLINK_1 = 1;


// 资源表
enum class ResType
{
	Others = 0,
	Sound = 1,
	Image = 2,
	ImageList = 3,
};

struct RESTABLEHEADER
{
	int iVer;
	int cResources;
};

struct RESTABLE_INDEXENTRY
{
	int iID;
	UINT uOffset;
	UINT cbSize;
	ECKENUM eType;
};





// 窗体表
struct FORMTABLEHEADER
{
	int iVer;
	int cForms;
};

struct FORMTABLE_INDEXENTRY
{
	int iID;
	UINT uOffset;
	UINT cbSize;
};


#pragma pack(pop)
ECK_NAMESPACE_END