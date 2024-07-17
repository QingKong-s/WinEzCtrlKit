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

template<class TAlloc, class TChar, class TTraits, class TAlloc1>
EckInline CRefBinT<TAlloc>& operator<<(CRefBinT<TAlloc>& rb, const CRefStrT<TChar, TTraits, TAlloc1>& rs)
{
	rb.PushBack(rs.Data(), rs.ByteSize());
	return rb;
}

template<class TAlloc, class TAlloc1>
EckInline CRefBinT<TAlloc>& operator<<(CRefBinT<TAlloc>& rb, const CRefBinT<TAlloc1>& rb1)
{
	rb.PushBack(rb1.Data(), rb1.Size());
	return rb;
}

template<class TAlloc, class T>
EckInline CRefBinT<TAlloc>& operator<<(CRefBinT<TAlloc>& rb, const T& t)
{
	rb.PushBack(&t, sizeof(T));
	return rb;
}
ECK_NAMESPACE_END