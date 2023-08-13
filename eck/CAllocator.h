/*
* WinEzCtrlKit Library
*
* CAllocator.h £º ÄÚ´æ·ÖÅäÆ÷
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
template<class T, class TSize = SIZE_T>
struct CAllocator
{
	static T* Alloc(TSize c)
	{
		return (T*)HeapAlloc(GetProcessHeap(), 0, c * sizeof(T));
	}

	static T* ReAlloc(T* pOrg, TSize c)
	{
		return (T*)HeapReAlloc(GetProcessHeap(), 0, pOrg, c * sizeof(T));
	}

	static T* AllocZ(TSize c)
	{
		return (T*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, c * sizeof(T));
	}

	static T* ReAllocZ(T* pOrg, TSize c)
	{
		return (T*)HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, pOrg, c * sizeof(T));
	}

	static void Free(T* p)
	{
		HeapFree(GetProcessHeap(), 0, p);
	}

	static TSize MakeCapacity(TSize c)
	{
		return c * 2;
	}
};
ECK_NAMESPACE_END