#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
inline constexpr size_t INVALID_BIN_POS = SizeTMax;
inline constexpr size_t BinNPos = SizeTMax;

template<class TPtr>
concept ccpIsPtr = std::is_pointer_v<TPtr>;


template<ccpIsPtr TPtr, ccpIsPtr TPattern>
_Ret_maybenull_ EckInline TPtr MemMem(_In_reads_bytes_(Len) TPtr Mem, size_t Len,
	_In_reads_bytes_(SubLen) TPattern SubMem, size_t SubLen)
{
	if (Len < SubLen)
		return nullptr;
	if (SubLen == 0)
		return Mem;
	const auto pEnd = (PCBYTE)Mem + Len - SubLen + 1;
	for (auto p = (PCBYTE)Mem; p < pEnd; ++p)
	{
		const auto pFind = memchr(p, *(PCBYTE)SubMem, pEnd - p);
		if (!pFind)
			return nullptr;
		if (memcmp(pFind, SubMem, SubLen) == 0)
			return (TPtr)pFind;
	}
	return nullptr;
}

template<ccpIsPtr TPtr, ccpIsPtr TPattern>
_Ret_maybenull_ EckInline TPtr MemRMem(_In_reads_bytes_(Len) TPtr Mem, size_t Len,
	_In_reads_bytes_(SubLen) TPattern SubMem, size_t SubLen, size_t posStart = SizeTMax)
{
	if (Len < SubLen)
		return nullptr;
	if (SubLen == 0)
		return (PCBYTE)Mem + std::min(posStart, Len - SubLen);
	for (auto p = (PCBYTE)Mem + std::min(posStart, Len - SubLen); p >= (PCBYTE)Mem; --p)
	{
		if (*p == *SubStr && memcmp(p, SubMem, SubLen) == 0)
			return p;
	}
	return nullptr;
}


template<ccpIsPtr TPtr, ccpIsPtr TPattern>
EckInlineNd size_t FindBin(_In_reads_bytes_(cbMain) TPtr pMain, size_t cbMain,
	_In_reads_bytes_(cbSub) TPattern pSub, size_t cbSub, size_t posStart = 0)
{
	const auto pFind = MemMem(pMain + posStart, cbMain - posStart, pSub, cbSub);
	return pFind ? ((PCBYTE)pFind - (PCBYTE)pMain) : -1;
}
template<ccpIsPtr TPtr, ccpIsPtr TPattern>
EckInlineNd size_t FindBinRev(_In_reads_bytes_(cbMain) TPtr pMain, size_t cbMain,
	_In_reads_bytes_(cbSub) TPattern pSub, size_t cbSub, size_t posStart = SizeTMax)
{
	const auto pFind = MemRMem(pMain, cbMain, pSub, cbSub, posStart);
	return pFind ? ((PCBYTE)pFind - (PCBYTE)pMain) : -1;
}

template<class TProcesser, ccpIsPtr TPtr, ccpIsPtr TPtr2>
inline void SplitBin(TPtr pMain, size_t cbMain, TPtr2 pDiv, size_t cbDiv,
	int cSubBinExpected, TProcesser&& Processer)
{
	auto pFind = (PCBYTE)MemMem(pMain, cbMain, pDiv, cbDiv);
	auto pPrevFirst = (PCBYTE)pMain;
	int c{};
	while (pFind)
	{
		if (Processer((TPtr)pPrevFirst, pFind - pPrevFirst))
			return;
		++c;
		if (c == cSubBinExpected)
			return;
		pPrevFirst = pFind + cbDiv;
		pFind = (PCBYTE)MemMem(pPrevFirst, cbMain - (pPrevFirst - pMain), pDiv, cbDiv);
	}
	Processer((TPtr)pPrevFirst, pMain + cbMain - pPrevFirst);
}
ECK_NAMESPACE_END