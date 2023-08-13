/*
* WinEzCtrlKit Library
*
* CStatic.h �� ��׼��̬
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "CWnd.h"

ECK_NAMESPACE_BEGIN
class CStatic :public CWnd
{
public:
	EckInline HICON GetIcon()
	{
		return (HICON)SendMsg(STM_GETICON, 0, 0);
	}

	/// <summary>
	/// ȡͼ��
	/// </summary>
	/// <param name="uType">ͼ�����ͣ�IMAGE_����</param>
	/// <returns></returns>
	template<UINT uType = IMAGE_BITMAP>
	EckInline HBITMAP GetImage()
	{
		return (HBITMAP)SendMsg(STM_GETIMAGE, uType, 0);
	}


};

ECK_NAMESPACE_END