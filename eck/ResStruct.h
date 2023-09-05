#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
#pragma pack(push, ECK_CTRLDATA_ALIGN)
static constexpr int
DATAVER_RESTABLE_1 = 1;


// ��Դ��
enum class ResType
{
	Others = 0,
	Sound = 1,
	Image = 2,
	ImageList = 3,
	Font = 4,
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





// �����
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

struct FTCTRLDATAHEADER
{
	int iVer;
	int cCtrls;
};

struct FTCTRLDATA
{
	UINT cbData;
	int cChildren;
	int idxInfo;
	RECT rc;// �󶥿�ߣ����������ϽǵĿؼ�
};
// BYTE byData[];


#pragma pack(pop)
ECK_NAMESPACE_END