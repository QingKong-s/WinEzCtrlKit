#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
void* MemMem(const void* pMem, size_t nMemSize, const void* pPattern, size_t nPatternSize);
ECK_NAMESPACE_END