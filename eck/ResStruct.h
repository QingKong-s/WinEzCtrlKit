#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
#pragma pack(push, ECK_CTRLDATA_ALIGN)
static constexpr int
DATAVER_RESTABLE_1 = 1,
DATAVER_FORMTABLE_1 = 1;


// ��Դ�� ��Դ����
enum class ResType
{
	Others		= 0,
	Sound		= 1,
	Image		= 2,
	ImageList	= 3,
	Font		= 4,
};

// ��Դ�� ͷ
struct RESTABLEHEADER
{
	int iVer;			// �汾
	int cResources;		// ��Դ��
};

// ��Դ�� ����
struct RESTABLE_INDEXENTRY
{
	int iID;			// ID
	UINT uOffset;		// ��Դ���������Դ��������ʼ��ַ��ƫ��
	UINT cbSize;		// ��Դ���ݴ�С
	ECKENUM eType;		// ��Դ����
};

// ����� ͷ
struct FORMTABLEHEADER
{
	int iVer;			// �汾
	int cForms;			// ������
};

// ����� ����
struct FORMTABLE_INDEXENTRY
{
	int iID;			// ����ID
	UINT uOffset;		// ����������Դ������ʼ��ַ��ƫ��
	UINT cbSize;		// �������ݴ�С
};

// ����� ������Ϣ ͷ
struct FTFORMDATAHEADER
{
	int iVer;			// �汾
	POINT xyOffset;		// x��y������С�Ŀؼ��ڿͻ���������
	UINT cbData;		// byData�Ĵ�С
	// BYTE byData[];	// CWnd��CForm�����л�����
};

// ����� �ؼ���Ϣ ͷ
struct FTCTRLDATAHEADER
{
	int iVer;			// �汾
	int cCtrls;			// �ؼ���
};

// ����� �����ؼ���Ϣ
struct FTCTRLDATA
{
	UINT cbData;		// byData�Ĵ�С
	int cChildren;		// �ֿؼ���
	int idxInfo;		// ������
	RECT rc;			// λ�ã��󶥿�ߣ������x��y������С�Ŀؼ�
	// BYTE byData[];	// �ؼ�������л�����
};

/*
��Դ��
--------------
RESTABLEHEADER
{
	RESTABLE_INDEXENTRY
}
{
	��Դ����
}
--------------


�������л����ݣ����������ϵ����пؼ�����
--------------
FTCTRLDATAHEADER
{
	FTCTRLDATA
	�ؼ������л�����
}
--------------


���������һ���������壩��
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
		�ؼ������л�����
	}
}
--------------
*/
#pragma pack(pop)
ECK_NAMESPACE_END