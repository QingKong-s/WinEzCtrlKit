/*
* WinEzCtrlKit Library
*
* StrBinInterop.h ： 字符串字节集互操作
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "CRefBin.h"
#include "CRefStr.h"

ECK_NAMESPACE_BEGIN
EckInline CRefStrW ToStr(const CRefBin& rb)
{
	CRefStrW rs((int)rb.Size() / 2);
	memcpy(rs.Data(), rb.Data(), rs.Size() * sizeof(WCHAR));
	return rs;
}
ECK_NAMESPACE_END