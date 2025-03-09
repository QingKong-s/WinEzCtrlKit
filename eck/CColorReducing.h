#pragma once
#include "CMemorySet.h"

ECK_NAMESPACE_BEGIN
class CColorReducing
{
private:
	struct NODE
	{
		UINT r, g, b;	// 颜色和
		BOOLEAN bLeaf;	// 叶节点
		BOOLEAN bCut;	// 是否可裁剪
		BYTE cChild;	// 子节点数
		UINT cColor;	// 颜色数
		NODE* pChild[8];// 子节点
		NODE* pNext;	// 同一层的后继节点，仅供常规节点使用
	};

	CMemorySet m_MemSet{};
	NODE m_Root{};
	std::array<NODE*, 8> m_apLast{ &m_Root };
	UINT m_cValid;		// 不重复的颜色数
	UINT m_cMax;		// 最大颜色数
	BOOL m_bNotInit;	// 是否非首次运行

	inline UINT ComputeColorError(NODE* p)
	{
		UINT avgR = p->r / p->cColor;
		UINT avgG = p->g / p->cColor;
		UINT avgB = p->b / p->cColor;
		UINT error = 0;
		for (auto& child : p->pChild)
		{
			if (child)
			{
				error += (child->r - avgR) * (child->r - avgR);
				error += (child->g - avgG) * (child->g - avgG);
				error += (child->b - avgB) * (child->b - avgB);
			}
		}
		return error;
	}

	void RemoveChild(NODE* p)
	{
		if (!p->bCut)
			return;
		p->bCut = FALSE;
		for (auto e : p->pChild)
		{
			if (e)
				RemoveChild(e);
		}
	}

	void ReduceColor()
	{
		NODE* pCurr{};
		UINT cMin{ 0xFFFFFFFF };

		for (int i = 7; i >= 0; --i)
		{
			auto p{ m_apLast[i] };
			while (p)
			{
				if (p->cChild && p->bCut && p->cColor < cMin)
				{
					cMin = p->cColor;
					pCurr = p;
					if (cMin == 1)
						break;
				}

				if (!(p = p->pNext))
					break;
			}
			if (pCurr)
				break;
			else
				m_apLast[i] = nullptr;
		}
		EckAssert(pCurr);
		pCurr->bLeaf = TRUE;
		m_cValid -= (pCurr->cChild - 1);
		RemoveChild(pCurr);
	}
public:
	CColorReducing()
	{
		m_MemSet.SetPageSize(4096 * 10);
		m_Root.bCut = TRUE;
	}
	CColorReducing(UINT cMaxColor) : CColorReducing{}
	{
		m_cMax = cMaxColor;
	}

	void Reset()
	{
		m_MemSet.ClearRecord();
		m_Root = {};
		m_bNotInit = TRUE;
		m_Root.bCut = TRUE;
		m_apLast = {};
	}

	void AddColor(UINT r, UINT g, UINT b)
	{
		UINT nCurrLevel{}, nShift{ 7 }, idx;
		NODE* pCurrNode{ &m_Root };
		pCurrNode->r += r;
		pCurrNode->g += g;
		pCurrNode->b += b;
		pCurrNode->cColor += 1;
		for (; nCurrLevel < 8; ++nCurrLevel, --nShift)
		{
			idx = (((r >> nShift) << 2) & 0b100) |
				(((g >> nShift) << 1) & 0b010) |
				((b >> nShift) & 0b001);
			if (!pCurrNode->pChild[idx])
			{
				const auto p = (NODE*)m_MemSet.Allocate(sizeof(NODE));
				pCurrNode->pChild[idx] = p;
				if (!m_bNotInit)
					ZeroMemory(p, sizeof(NODE));
				if (nCurrLevel == 7)
				{
					p->bLeaf = TRUE;
					++m_cValid;
					while (m_cValid > m_cMax)
						ReduceColor();
				}
				else
				{
					p->pNext = m_apLast[nCurrLevel + 1];
					m_apLast[nCurrLevel + 1] = p;
				}
				p->bCut = TRUE;

				++pCurrNode->cChild;
			}
			pCurrNode = pCurrNode->pChild[idx];
			pCurrNode->r += r;
			pCurrNode->g += g;
			pCurrNode->b += b;
			pCurrNode->cColor += 1;
			if (pCurrNode->bLeaf)
				break;
		}
	}

	BOOL GetNearestColor(UINT r, UINT g, UINT b,
		UINT& rOut, UINT& gOut, UINT& bOut)
	{
		UINT nCurrLevel{}, nShift{ 7 }, idx;
		NODE* p{ &m_Root };
		if (p->bLeaf)
		{
			rOut = p->r / p->cColor;
			gOut = p->g / p->cColor;
			bOut = p->b / p->cColor;
			return TRUE;
		}
		for (; nCurrLevel < 8; ++nCurrLevel, --nShift)
		{
			idx = (((r >> nShift) << 2) & 0b100) |
				(((g >> nShift) << 1) & 0b010) |
				((b >> nShift) & 0b001);
			p = p->pChild[idx];
			if (!p)
				break;
			if (p->bLeaf)
			{
				rOut = p->r / p->cColor;
				gOut = p->g / p->cColor;
				bOut = p->b / p->cColor;
				return TRUE;
			}
		}
		rOut = gOut = bOut = 0;
		return FALSE;
	}

	UINT GetPalette(_Out_writes_(cMaxColor) UINT* pColors, UINT cMaxColor)
	{
		std::vector<NODE*> s{};
		s.reserve(16);
		s.push_back(&m_Root);
		auto pcr{ pColors };
		while (!s.empty())
		{
			auto p = s.back();
			s.pop_back();
			if (p->bLeaf)
			{
				*pcr++ = ((BYTE(p->r / p->cColor) << 16) |
					(BYTE(p->g / p->cColor) << 8) |
					BYTE(p->b / p->cColor));
				if (p->cColor == m_cMax)
					break;
			}
			else
			{
				for (auto e : p->pChild)
				{
					if (e)
						s.push_back(e);
				}
			}
		}
		return UINT(pcr - pColors);
	}

	UINT Reduce()
	{
		while (m_cValid > m_cMax)
			ReduceColor();
		return m_cValid;
	}
};
ECK_NAMESPACE_END