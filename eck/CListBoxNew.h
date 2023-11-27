/*
* WinEzCtrlKit Library
*
* CListBoxNew.h ： 所有者数据模式的列表框
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "CWnd.h"
#include "GdiplusFlatDef.h"

ECK_NAMESPACE_BEGIN
class CListBoxNew :public CWnd
{
private:
	CEzCDC m_DC{};
	GpGraphics* m_pGraphics = NULL;



	ECK_DS_BEGIN(DPIS)
		ECK_DS_ENTRY_F(cxSelBorder, 1.f)
		;
	ECK_DS_END_VAR(m_DsF);
};

ECK_NAMESPACE_END