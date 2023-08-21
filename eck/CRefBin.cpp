#include "CRefBin.h"
#include "Utility.h"// byd循环引用了

ECK_NAMESPACE_BEGIN
CRefBin::CRefBin(SIZE_T cb)
{
	if (!cb)
		return;

	m_cb = cb;
	SIZE_T cbCapacity = TAlloc::MakeCapacity(cb);
	m_pStream = TAlloc::Alloc(cbCapacity);
	if (m_pStream)
		m_cbCapacity = cbCapacity;
}

CRefBin::CRefBin(PCVOID p, SIZE_T cb)
{
	if (!p || !cb)
		return;

	SIZE_T cbCapacity = TAlloc::MakeCapacity(cb);
	m_pStream = TAlloc::Alloc(cbCapacity);
	if (m_pStream)
	{
		m_cb = cb;
		m_cbCapacity = cbCapacity;
		memcpy(m_pStream, p, cb);
	}
}

CRefBin::CRefBin(const CRefBin& x)
{
	if (!x.m_cb || !x.m_pStream)
		return;
	SIZE_T cbCapacity = TAlloc::MakeCapacity(x.m_cb);
	m_pStream = TAlloc::Alloc(cbCapacity);
	if (m_pStream)
	{
		m_cb = x.m_cb;
		m_cbCapacity = cbCapacity;
		memcpy(m_pStream, x.m_pStream, x.m_cb);
	}
}

BYTE* CRefBin::Attach(BYTE* p, SIZE_T cbCapacity, SIZE_T cb)
{
	auto pOld = m_pStream;
	if (!p)
	{
		m_pStream = NULL;
		m_cbCapacity = 0u;
		m_cb = 0u;
		return pOld;
	}
	m_pStream = p;
	m_cbCapacity = cbCapacity;
	m_cb = cb;
	return pOld;
}

void CRefBin::DupStream(PCVOID p, SIZE_T cb)
{
	if (m_pStream)
	{
		if (m_cbCapacity < cb)
		{
			SIZE_T cbCapacity = TAlloc::MakeCapacity(cb);
			auto pTemp = TAlloc::ReAlloc(m_pStream, cbCapacity);
			if (!pTemp)
				return;
			m_pStream = pTemp;
			m_cbCapacity = cbCapacity;
		}
	}
	else
	{
		SIZE_T cbCapacity = TAlloc::MakeCapacity(cb);
		m_pStream = TAlloc::Alloc(cbCapacity);
		if (!m_pStream)
			return;
		m_cbCapacity = cbCapacity;
	}

	m_cb = cb;
	memcpy(m_pStream, p, cb);
}

void CRefBin::Reserve(SIZE_T cb)
{
	if (m_cbCapacity >= cb)
		return;

	if (m_pStream)
	{
		auto pTemp = TAlloc::ReAlloc(m_pStream, cb);
		if (pTemp)
		{
			m_pStream = pTemp;
			m_cbCapacity = cb;
		}
	}
	else
	{
		m_pStream = TAlloc::Alloc(cb);
		if (m_pStream)
			m_cbCapacity = cb;
	}
}

void CRefBin::Replace(SIZE_T posStart, SIZE_T cbReplacing, PCVOID pNew, SIZE_T cbNew)
{
	SIZE_T cbOrg = m_cb;
	ReSize(m_cb + cbNew - cbReplacing);

	memmove(m_pStream + posStart + cbNew, m_pStream + posStart + cbReplacing, cbOrg - posStart - cbReplacing);
	if (pNew)
		memcpy(m_pStream + posStart, pNew, cbNew);
}

void CRefBin::ReplaceSubBin(PCVOID pReplacedBin, SIZE_T cbReplacedBin, PCVOID pSrcBin, SIZE_T cbSrcBin, SIZE_T posStart, int cReplacing)
{
	SIZE_T pos = 0u;
	for (int c = 1;; ++c)
	{
		pos = FindBin(m_pStream, m_cb, pReplacedBin, cbReplacedBin, posStart + pos);
		if (pos == INVALID_BIN_POS)
			break;
		Replace(pos, cbReplacedBin, pSrcBin, cbSrcBin);
		pos += cbSrcBin;
		if (c == cReplacing)
			break;
	}
}

void CRefBin::MakeRepeatedBinSeq(PCVOID pBin, SIZE_T cbBin, SIZE_T cCount, SIZE_T posStart)
{
	ReSize(posStart + cCount * cbBin);
	BYTE* pCurr;
	SIZE_T i;
	for (i = 0, pCurr = m_pStream + posStart; i < cCount; ++i, pCurr += cbBin)
		memcpy(pCurr, pBin, cbBin);
}



SIZE_T FindBin(PCVOID pMain, SIZE_T cbMainSize, PCVOID pSub, SIZE_T cbSubSize, SIZE_T posStart)
{
	if (cbMainSize < cbSubSize)
		return INVALID_BIN_POS;
	for (PCBYTE pCurr = (PCBYTE)pMain + posStart, pEnd = (PCBYTE)pMain + cbMainSize - cbSubSize; pCurr <= pEnd; ++pCurr)
	{
		if (memcmp(pCurr, pSub, cbSubSize) == 0)
			return pCurr - (PCBYTE)pMain;
	}
	return INVALID_BIN_POS;
}

SIZE_T FindBinRev(PCVOID pMain, SIZE_T cbMainSize, PCVOID pSub, SIZE_T cbSubSize, SIZE_T posStart)
{
	if (cbMainSize < cbSubSize)
		return INVALID_BIN_POS;
	for (PCBYTE pCurr = (PCBYTE)pMain + cbMainSize - posStart - cbSubSize; pCurr >= pMain; --pCurr)
	{
		if (memcmp(pCurr, pSub, cbSubSize) == 0)
			return pCurr - (PCBYTE)pMain;
	}
	return INVALID_BIN_POS;
}

template<class TProcesser>
void SplitBinInt(PCVOID p, SIZE_T cbSize, PCVOID pDiv, SIZE_T cbDiv, int cBinExpected, TProcesser Processer)
{
	SIZE_T pos = FindBin(p, cbSize, pDiv, cbDiv);
	SIZE_T posPrevFirst = 0u;
	int c = 0;
	while (pos != INVALID_BIN_POS)
	{
		Processer((BYTE*)p + posPrevFirst, pos - posPrevFirst);
		++c;
		if (c == cBinExpected)
			return;
		posPrevFirst = pos + cbDiv;
		pos = FindBin(p, cbSize, pDiv, cbDiv, posPrevFirst);
	}

	Processer((BYTE*)p + posPrevFirst, pos + cbSize - posPrevFirst);
}

void SplitBin(PCVOID p, SIZE_T cbSize, PCVOID pDiv, SIZE_T cbDiv, std::vector<SPLITBININFO>& aResult, int cBinExpected)
{
	SplitBinInt(p, cbSize, pDiv, cbDiv, cBinExpected,
		[&](PCVOID p, SIZE_T cb)
		{
			aResult.push_back({ p,cb });
		});
}

void SplitBin(PCVOID p, SIZE_T cbSize, PCVOID pDiv, SIZE_T cbDiv, std::vector<CRefBin>& aResult, int cBinExpected)
{
	SplitBinInt(p, cbSize, pDiv, cbDiv, cBinExpected,
		[&](PCVOID p, SIZE_T cb)
		{
			aResult.push_back(CRefBin(p, cb));
		});
}

ECK_NAMESPACE_END