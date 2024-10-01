/*
* WinEzCtrlKit Library
*
* CRegion.h : 区域
*
* Copyright(C) 2023-2024 QingKong
*/
#pragma once
#include "Utility.h"

ECK_NAMESPACE_BEGIN
class CRegion
{
public:
	enum :int
	{
		InfMin = (int)0x80000000,
		InfMax = (int)0x7FFFFFFF
	};
private:
	struct YCategory
	{
		int yMin;
		int yMax;
		int idxXPairBegin;
		int cXPair;
	};
	struct XPair
	{
		int xMin;
		int xMax;
	};
	struct ComplexRegion
	{
		std::vector<YCategory> m_vYCategory{};
		std::vector<XPair> m_vXPair{};

		constexpr void Reset() { m_vYCategory.clear(); m_vXPair.clear(); }

		void AddYCategory(int yMin, int yMax, const XPair* pXPair, int cXPair)
		{
			if (m_vYCategory.empty())
			{
				m_vYCategory.emplace_back(yMin, yMax, (int)m_vXPair.size(), cXPair);
				m_vXPair.insert(m_vXPair.end(), pXPair, pXPair + cXPair);
			}
			else
			{
				auto& e = m_vYCategory.back();
				if (e.cXPair == cXPair && yMin <= e.yMax &&
					memcmp(m_vXPair.data() + e.idxXPairBegin, pXPair, cXPair * sizeof(XPair)) == 0)
				{
					e.yMin = std::min(e.yMin, yMin);
					e.yMax = std::max(e.yMax, yMax);
				}
				else
				{
					m_vYCategory.emplace_back(yMin, yMax, (int)m_vXPair.size(), cXPair);
					m_vXPair.insert(m_vXPair.end(), pXPair, pXPair + cXPair);
				}
			}
		}

		constexpr void AddYCategory(int yMin, int yMax)
		{
			m_vYCategory.emplace_back(yMin, yMax, (int)m_vXPair.size(), 0);
		}
	};

	RECT m_SimpleRc{ InfMin,InfMin,InfMin,InfMin };
	ComplexRegion* m_pComplex{};
#ifdef _DEBUG
	BOOL m_bRecordingPoint{};
#endif

	constexpr void AllocComplexRegion()
	{
		EckAssert(!m_pComplex && L"ComplexRegion has already been allocated.");
		m_pComplex = new ComplexRegion{};
	}

	constexpr void FreeComplexRegion()
	{
		EckAssert(m_pComplex && L"ComplexRegion has not been allocated.");
		delete m_pComplex;
		m_pComplex = nullptr;
	}

	constexpr static void SimplifyXPair(std::vector<XPair>& vXPair)
	{
		if (vXPair.size() <= 1)
			return;
		size_t idxDelBegin{ SizeTMax };
		size_t idxDelEnd{ SizeTMax };
		for (size_t i = vXPair.size() - 1; i > 0; --i)
		{
			if (vXPair[i].xMin == vXPair[i - 1].xMax)
			{
				if (idxDelBegin == SizeTMax)
				{
					idxDelBegin = idxDelEnd = i - 1;
					vXPair[i].xMin = vXPair[i - 1].xMin;
				}
				else
				{
					idxDelBegin = i - 1;
					vXPair[idxDelEnd + 1].xMin = vXPair[i - 1].xMin;
				}
			}
			else if (idxDelBegin != SizeTMax)
			{
				vXPair.erase(vXPair.begin() + idxDelBegin, vXPair.begin() + idxDelEnd + 1);
				idxDelBegin = idxDelEnd = SizeTMax;
			}
		}
		if (idxDelBegin != SizeTMax)
			vXPair.erase(vXPair.begin() + idxDelBegin, vXPair.begin() + idxDelEnd + 1);
	}

	constexpr static void XPairIntersect(const XPair* xPair1, const XPair* xPairEnd1,
		const XPair* xPair2, const XPair* xPairEnd2, std::vector<XPair>& vResult)
	{
		vResult.clear();
		do
		{
			if (xPair1->xMin <= xPair2->xMin)
			{
				if (xPair1->xMax > xPair2->xMin)// 相交
				{
					auto& e = vResult.emplace_back(xPair2->xMin);
					if (xPair1->xMax < xPair2->xMax)
					{
						e.xMax = xPair1->xMax;
						++xPair1;
					}
					else
					{
						e.xMax = xPair2->xMax;
						++xPair2;
					}
				}
				else
					++xPair1;
			}
			else if (xPair2->xMax > xPair1->xMin)// 相交
			{
				auto& e = vResult.emplace_back(xPair1->xMin);
				if (xPair2->xMax < xPair1->xMax)
				{
					e.xMax = xPair2->xMax;
					++xPair2;
				}
				else
				{
					e.xMax = xPair1->xMax;
					++xPair1;
				}
			}
			else
				++xPair1;
		} while (xPair1 < xPairEnd1 && xPair2 < xPairEnd2);
	}

	constexpr static void XPairUnion(const XPair* xPair1, const XPair* xPairEnd1,
		const XPair* xPair2, const XPair* xPairEnd2, std::vector<XPair>& vResult)
	{
		vResult.clear();
		do
		{
			if (xPair1->xMin <= xPair2->xMin)
			{
				auto& e = vResult.emplace_back(xPair1->xMin);
				if (xPair1->xMax < xPair2->xMin)// 不相交
				{
					e.xMax = xPair1->xMax;
					++xPair1;
				}
				else// 相交
				{
					e.xMax = std::max(xPair1->xMax, xPair2->xMax);
					++xPair1;
					++xPair2;
				}
			}
			else
			{
				auto& e = vResult.emplace_back(xPair2->xMin);
				if (xPair2->xMax < xPair1->xMin)// 不相交
				{
					e.xMax = xPair2->xMax;
					++xPair2;
				}
				else// 相交
				{
					e.xMax = std::max(xPair1->xMax, xPair2->xMax);
					++xPair1;
					++xPair2;
				}
			}
		} while (xPair1 < xPairEnd1 && xPair2 < xPairEnd2);
		// 剩下的全部加入
		if (xPair1 < xPairEnd1)
			vResult.insert(vResult.end(), xPair1, xPairEnd1);
		if (xPair2 < xPairEnd2)
			vResult.insert(vResult.end(), xPair2, xPairEnd2);
		SimplifyXPair(vResult);
	}

	constexpr static void XPairDiff(const XPair* xPair1, const XPair* xPairEnd1,
		const XPair* xPair2, const XPair* xPairEnd2, std::vector<XPair>& vResult)
	{
		vResult.clear();
		int xMin1 = xPair1->xMin;
		do
		{
			if (xMin1 < xPair2->xMin)
			{
				auto& e = vResult.emplace_back(xMin1);
				if (xPair1->xMax < xPair2->xMin)
				{
					e.xMax = xPair1->xMax;
					++xPair1;
					xMin1 = xPair1->xMin;
				}
				else
				{
					e.xMax = xPair2->xMin;
					xMin1 = xPair2->xMin;
				}
			}
			else if (xPair2->xMax > xPair1->xMin)// 相交
			{
				if (xPair2->xMax > xPair1->xMax)
				{
					++xPair1;
					xMin1 = xPair1->xMin;
				}
				else
				{
					xMin1 = xPair2->xMax;
					++xPair2;
				}
			}
			else
				++xPair2;
		} while (xPair1 < xPairEnd1 && xPair2 < xPairEnd2);
		// 左边剩下的全部加入
		if (xPair1 < xPairEnd1)
		{
			vResult.emplace_back(xMin1, xPair1->xMax);// 不要忽略xMin1，可能被拉平
			++xPair1;
			if (xPair1 < xPairEnd1)
				vResult.insert(vResult.end(), xPair1, xPairEnd1);
		}
		SimplifyXPair(vResult);
	}

	constexpr static void XPairSymDiff(const XPair* xPair1, const XPair* xPairEnd1,
		const XPair* xPair2, const XPair* xPairEnd2, std::vector<XPair>& vResult)
	{
		vResult.clear();
		int xMin1 = xPair1->xMin;
		int xMin2 = xPair2->xMin;
		EckLoop()
		{
			if (xMin1 < xMin2)
			{
				auto& e = vResult.emplace_back(xMin1);
				if (xPair1->xMax <= xMin2)// 不相交
				{
					e.xMax = xPair1->xMax;
					++xPair1;
					if (xPair1 == xPairEnd1)
						break;
					xMin1 = xPair1->xMin;
				}
				else// 相交
				{
					e.xMax = xMin2;
					if (xPair1->xMax < xPair2->xMax)
					{
						xMin2 = xPair1->xMax;
						++xPair1;
						if (xPair1 == xPairEnd1)
							break;
						xMin1 = xPair1->xMin;
					}
					else if (xPair2->xMax < xPair1->xMax)
					{
						xMin1 = xPair2->xMax;
						++xPair2;
						if (xPair2 == xPairEnd2)
							break;
						xMin2 = xPair2->xMin;
					}
					else
					{
						BOOL bEnd{};
						++xPair1;
						if (xPair1 == xPairEnd1)
							bEnd = TRUE;
						else
							xMin1 = xPair1->xMin;
						++xPair2;
						if (xPair2 == xPairEnd2)
							break;
						xMin2 = xPair2->xMin;
						if (bEnd)
							break;
					}
				}
			}
			else if (xMin2 < xMin1)
			{
				auto& e = vResult.emplace_back(xMin2);
				if (xPair2->xMax <= xMin1)// 不相交
				{
					e.xMax = xPair2->xMax;
					++xPair2;
					if (xPair2 == xPairEnd2)
						break;
					xMin2 = xPair2->xMin;
				}
				else// 相交
				{
					e.xMax = xMin1;
					if (xPair2->xMax < xPair1->xMax)
					{
						xMin1 = xPair2->xMax;
						++xPair2;
						if (xPair2 == xPairEnd2)
							break;
						xMin2 = xPair2->xMin;
					}
					else if (xPair1->xMax < xPair2->xMax)
					{
						xMin2 = xPair1->xMax;
						++xPair1;
						if (xPair1 == xPairEnd1)
							break;
						xMin1 = xPair1->xMin;
					}
					else
					{
						BOOL bEnd{};
						++xPair1;
						if (xPair1 == xPairEnd1)
							bEnd = TRUE;
						else
							xMin1 = xPair1->xMin;
						++xPair2;
						if (xPair2 == xPairEnd2)
							break;
						xMin2 = xPair2->xMin;
						if (bEnd)
							break;
					}
				}
			}
			else
			{
				if (xPair1->xMax < xPair2->xMax)
				{
					xMin2 = xPair1->xMax;
					++xPair1;
					if (xPair1 == xPairEnd1)
						break;
					xMin1 = xPair1->xMin;
				}
				else if (xPair2->xMax < xPair1->xMax)
				{
					xMin1 = xPair2->xMax;
					++xPair2;
					if (xPair2 == xPairEnd2)
						break;
					xMin2 = xPair2->xMin;
				}
				else
				{
					BOOL bEnd{};
					++xPair1;
					if (xPair1 == xPairEnd1)
						bEnd = TRUE;
					else
						xMin1 = xPair1->xMin;
					++xPair2;
					if (xPair2 == xPairEnd2)
						break;
					xMin2 = xPair2->xMin;
					if (bEnd)
						break;
				}
			}
		}
		// 剩下的全部加入
		if (xPair1 < xPairEnd1)
		{
			vResult.emplace_back(xMin1, xPair1->xMax);// 不要忽略xMin1，可能被拉平
			++xPair1;
			if (xPair1 < xPairEnd1)
				vResult.insert(vResult.end(), xPair1, xPairEnd1);
		}
		if (xPair2 < xPairEnd2)
		{
			vResult.emplace_back(xMin2, xPair2->xMax);// 不要忽略xMin2，可能被拉平
			++xPair2;
			if (xPair2 < xPairEnd2)
				vResult.insert(vResult.end(), xPair2, xPairEnd2);
		}
		SimplifyXPair(vResult);
	}
public:
	class CPtRecorder
	{
		friend class CRegion;
	private:
		int m_y{ InfMin };
		ComplexRegion* m_pComplex;
		YCategory* m_pCurrYCat{};

		constexpr CPtRecorder(ComplexRegion* pComplex) :m_pComplex{ pComplex } {}
	public:
		EckInline constexpr void BeginLine(int y)
		{
			EckAssert(y > m_y && m_pComplex && m_pCurrYCat == nullptr);
			m_y = y;
			m_pComplex->AddYCategory(m_y, m_y + 1);
			m_pCurrYCat = &m_pComplex->m_vYCategory.back();
		}

		EckInline constexpr void EndLine()
		{
			EckAssert(m_y != InfMin && m_pComplex && m_pCurrYCat);
			auto& vYCat = m_pComplex->m_vYCategory;
			if (m_pCurrYCat->cXPair == 0)// 空分类，删除
				vYCat.pop_back();
			else if (vYCat.size() > 1)
			{
				auto& Prev = vYCat[vYCat.size() - 2];
				if (m_pCurrYCat->cXPair == Prev.cXPair && m_pCurrYCat->yMin <= Prev.yMax &&
					memcmp(m_pComplex->m_vXPair.data() + m_pCurrYCat->idxXPairBegin,
						m_pComplex->m_vXPair.data() + Prev.idxXPairBegin,
						m_pCurrYCat->cXPair * sizeof(XPair)) == 0)// 此分类与上一个相交/相邻分类相同，合并
				{
					Prev.yMax = m_pCurrYCat->yMax;
					vYCat.pop_back();
				}
			}
#ifdef _DEBUG
			m_y = InfMin;
			m_pCurrYCat = nullptr;
#endif
		}

		constexpr void AddPoint(int x)
		{
			EckAssert(m_y != InfMin && m_pComplex && m_pCurrYCat);
			if (m_pCurrYCat->cXPair == 0)
			{
				m_pComplex->m_vXPair.emplace_back(x, x + 1);
				++m_pCurrYCat->cXPair;
			}
			else
			{
				const auto itBegin = m_pComplex->m_vXPair.begin() + m_pCurrYCat->idxXPairBegin;
				const auto itEnd = itBegin + m_pCurrYCat->cXPair;
				for (auto it = itBegin; it != itEnd; ++it)// 检查已有相邻范围
				{
					if (x == it->xMin - 1)
					{
						--it->xMin;
						return;
					}
					else if (x == it->xMax)
					{
						++it->xMax;
						return;
					}
				}
				m_pComplex->m_vXPair.emplace_back(x, x + 1);
				++m_pCurrYCat->cXPair;
			}
		}
	};

	constexpr void SetRect(const RECT* prc, int cRc)
	{
		EckAssert(!m_bRecordingPoint);
		if (cRc == 1)
		{
			if (m_pComplex)
				FreeComplexRegion();
			m_SimpleRc = *prc;
		}
		else
		{
			if (!m_pComplex)
				AllocComplexRegion();
			m_pComplex->Reset();
			int yMin = prc[0].top;
			int yMax = prc[0].bottom;
			int idxPrevXPair = 0;
			m_pComplex->m_vYCategory.emplace_back(yMin, yMax);
			m_pComplex->m_vXPair.emplace_back(prc[0].left, prc[0].right);
			for (int i = 1; i < cRc; ++i)
			{
				if (prc[i].top != yMin || prc[i].bottom != yMax)// 新分类
				{
					auto& e = m_pComplex->m_vYCategory.back();
					e.idxXPairBegin = idxPrevXPair;
					e.cXPair = (int)m_pComplex->m_vXPair.size() - idxPrevXPair;
					idxPrevXPair = (int)m_pComplex->m_vXPair.size();
					yMin = prc[i].top;
					yMax = prc[i].bottom;
					m_pComplex->m_vYCategory.emplace_back(yMin, yMax);
				}
				m_pComplex->m_vXPair.emplace_back(prc[i].left, prc[i].right);
			}
			if (auto& e = m_pComplex->m_vYCategory.back(); e.cXPair == 0)
			{
				e.idxXPairBegin = idxPrevXPair;
				e.cXPair = (int)m_pComplex->m_vXPair.size() - idxPrevXPair;
			}
		}
	}

	constexpr void GetRect(std::vector<RECT>& vRc) const
	{
		EckAssert(!m_bRecordingPoint);
		if (m_pComplex)
		{
			for (const auto& yCat : m_pComplex->m_vYCategory)
			{
				for (auto p = m_pComplex->m_vXPair.data() + yCat.idxXPairBegin;
					p < m_pComplex->m_vXPair.data() + yCat.idxXPairBegin + yCat.cXPair; ++p)
				{
					vRc.emplace_back(p->xMin, yCat.yMin, p->xMax, yCat.yMax);
				}
			}
		}
		else
			vRc.emplace_back(m_SimpleRc);
	}

	constexpr CRegion Intersect(const CRegion& Rgn) const
	{
		EckAssert(!m_bRecordingPoint);
		CRegion RgnResult{};
		if (!m_pComplex && !Rgn.m_pComplex)// optimization for simple region
		{
			IntersectRect(RgnResult.m_SimpleRc, m_SimpleRc, Rgn.m_SimpleRc);
			return RgnResult;
		}
		const XPair* xPair1;
		const XPair* xPairEnd1;
		const YCategory* yCat1;
		const YCategory* yCatEnd1;
		const XPair* xPair2;
		const XPair* xPairEnd2;
		const YCategory* yCat2;
		const YCategory* yCatEnd2;
		YCategory yCatDummy;
		XPair xPairDummy;
		if (m_pComplex)
		{
			yCat1 = m_pComplex->m_vYCategory.data();
			yCatEnd1 = yCat1 + m_pComplex->m_vYCategory.size();
			xPair1 = m_pComplex->m_vXPair.data();
			xPairEnd1 = xPair1 + yCat1->cXPair;
		}
		else
		{
			xPair1 = &xPairDummy;
			xPairEnd1 = xPair1 + 1;
			yCat1 = &yCatDummy;
			yCatEnd1 = yCat1 + 1;
			xPairDummy.xMin = m_SimpleRc.left;
			xPairDummy.xMax = m_SimpleRc.right;
			yCatDummy.yMin = m_SimpleRc.top;
			yCatDummy.yMax = m_SimpleRc.bottom;
			yCatDummy.idxXPairBegin = 0;
			yCatDummy.cXPair = 1;
		}
		if (Rgn.m_pComplex)
		{
			yCat2 = Rgn.m_pComplex->m_vYCategory.data();
			yCatEnd2 = yCat2 + Rgn.m_pComplex->m_vYCategory.size();
			xPair2 = Rgn.m_pComplex->m_vXPair.data();
			xPairEnd2 = xPair2 + yCat2->cXPair;
		}
		else
		{
			xPair2 = &xPairDummy;
			xPairEnd2 = xPair2 + 1;
			yCat2 = &yCatDummy;
			yCatEnd2 = yCat2 + 1;
			xPairDummy.xMin = Rgn.m_SimpleRc.left;
			xPairDummy.xMax = Rgn.m_SimpleRc.right;
			yCatDummy.yMin = Rgn.m_SimpleRc.top;
			yCatDummy.yMax = Rgn.m_SimpleRc.bottom;
			yCatDummy.idxXPairBegin = 0;
			yCatDummy.cXPair = 1;
		}

		RgnResult.AllocComplexRegion();
		std::vector<XPair> vXPairWork{};
		vXPairWork.reserve(std::max(xPairEnd1 - xPair1, xPairEnd2 - xPair2));
		EckLoop()
		{
			if (yCat1->yMin <= yCat2->yMin)
			{
				if (yCat1->yMax < yCat2->yMin)// 不相交，跳过yCat1
				{
					++yCat1;
					if (yCat1 == yCatEnd1)
						break;
					xPair1 += yCat1->cXPair;
					xPairEnd1 = xPair1 + yCat1->cXPair;
					continue;
				}
				// 此时上界 = yCat2->yMin
				if (yCat1->yMax < yCat2->yMax)
				{
					XPairIntersect(xPair1, xPairEnd1, xPair2, xPairEnd2, vXPairWork);
					RgnResult.m_pComplex->AddYCategory(yCat2->yMin, yCat1->yMax,
						vXPairWork.data(), (int)vXPairWork.size());
					++yCat1;
					if (yCat1 == yCatEnd1)
						break;
					xPair1 += yCat1->cXPair;
					xPairEnd1 = xPair1 + yCat1->cXPair;
				}
				else
				{
					XPairIntersect(xPair1, xPairEnd1, xPair2, xPairEnd2, vXPairWork);
					RgnResult.m_pComplex->AddYCategory(yCat2->yMin, yCat2->yMax,
						vXPairWork.data(), (int)vXPairWork.size());
					++yCat2;
					if (yCat2 == yCatEnd2)
						break;
					xPair2 += yCat2->cXPair;
					xPairEnd2 = xPair2 + yCat2->cXPair;
				}
			}
			else if (yCat2->yMax > yCat1->yMin)
			{
				// 此时上界 = yCat1->yMin
				if (yCat2->yMax < yCat1->yMax)
				{
					XPairIntersect(xPair1, xPairEnd1, xPair2, xPairEnd2, vXPairWork);
					RgnResult.m_pComplex->AddYCategory(yCat1->yMin, yCat2->yMax,
						vXPairWork.data(), (int)vXPairWork.size());
					++yCat2;
					if (yCat2 == yCatEnd2)
						break;
					xPair2 += yCat2->cXPair;
					xPairEnd2 = xPair2 + yCat2->cXPair;
				}
				else
				{
					XPairIntersect(xPair1, xPairEnd1, xPair2, xPairEnd2, vXPairWork);
					RgnResult.m_pComplex->AddYCategory(yCat1->yMin, yCat1->yMax,
						vXPairWork.data(), (int)vXPairWork.size());
					++yCat1;
					if (yCat1)
						break;
					xPair1 += yCat1->cXPair;
					xPairEnd1 = xPair1 + yCat1->cXPair;
				}
			}
			else
			{
				++yCat1;
				if (yCat1 == yCatEnd1)
					break;
				xPair1 += yCat1->cXPair;
				xPairEnd1 = xPair1 + yCat1->cXPair;
			}
		}
		return RgnResult;
	}

	constexpr CRegion Union(const CRegion& Rgn) const
	{
		EckAssert(!m_bRecordingPoint);
		CRegion RgnResult{};

		const XPair* xPair1;
		const XPair* xPairEnd1;
		const YCategory* yCat1;
		const YCategory* yCatEnd1;
		const XPair* xPair2;
		const XPair* xPairEnd2;
		const YCategory* yCat2;
		const YCategory* yCatEnd2;
		YCategory yCatDummy1;
		XPair xPairDummy1;
		YCategory yCatDummy2;
		XPair xPairDummy2;
		if (m_pComplex)
		{
			yCat1 = m_pComplex->m_vYCategory.data();
			yCatEnd1 = yCat1 + m_pComplex->m_vYCategory.size();
			xPair1 = m_pComplex->m_vXPair.data();
			xPairEnd1 = xPair1 + yCat1->cXPair;
		}
		else
		{
			xPair1 = &xPairDummy1;
			xPairEnd1 = xPair1 + 1;
			yCat1 = &yCatDummy1;
			yCatEnd1 = yCat1 + 1;
			xPairDummy1.xMin = m_SimpleRc.left;
			xPairDummy1.xMax = m_SimpleRc.right;
			yCatDummy1.yMin = m_SimpleRc.top;
			yCatDummy1.yMax = m_SimpleRc.bottom;
			yCatDummy1.idxXPairBegin = 0;
			yCatDummy1.cXPair = 1;
		}
		if (Rgn.m_pComplex)
		{
			yCat2 = Rgn.m_pComplex->m_vYCategory.data();
			yCatEnd2 = yCat2 + Rgn.m_pComplex->m_vYCategory.size();
			xPair2 = Rgn.m_pComplex->m_vXPair.data();
			xPairEnd2 = xPair2 + yCat2->cXPair;
		}
		else
		{
			xPair2 = &xPairDummy2;
			xPairEnd2 = xPair2 + 1;
			yCat2 = &yCatDummy2;
			yCatEnd2 = yCat2 + 1;
			xPairDummy2.xMin = Rgn.m_SimpleRc.left;
			xPairDummy2.xMax = Rgn.m_SimpleRc.right;
			yCatDummy2.yMin = Rgn.m_SimpleRc.top;
			yCatDummy2.yMax = Rgn.m_SimpleRc.bottom;
			yCatDummy2.idxXPairBegin = 0;
			yCatDummy2.cXPair = 1;
		}

		RgnResult.AllocComplexRegion();
		std::vector<XPair> vXPairWork{};
		vXPairWork.reserve((xPairEnd1 - xPair1) + (xPairEnd2 - xPair2));
		int yMin1 = yCat1->yMin;
		int yMin2 = yCat2->yMin;
		EckLoop()
		{
			if (yMin1 < yMin2)
			{
				if (yCat1->yMax <= yMin2)// 1不与2相交，合并1
				{
					RgnResult.m_pComplex->AddYCategory(yMin1, yCat1->yMax,
						xPair1, yCat1->cXPair);
					++yCat1;
					if (yCat1 == yCatEnd1)
						break;
					yMin1 = yCat1->yMin;
					xPair1 += yCat1->cXPair;
					xPairEnd1 = xPair1 + yCat1->cXPair;
				}
				else// 1与2相交，拉平1
				{
					RgnResult.m_pComplex->AddYCategory(yMin1, yMin2,
						xPair1, yCat1->cXPair);
					yMin1 = yMin2;// 不要递增，1、2还留有一块未处理区域
				}
			}
			else if (yMin2 < yMin1)
			{
				if (yCat2->yMax <= yMin1)// 2不与1相交，合并2
				{
					RgnResult.m_pComplex->AddYCategory(yMin2, yCat2->yMax,
						xPair2, yCat2->cXPair);
					++yCat2;
					if (yCat2 == yCatEnd2)
						break;
					yMin2 = yCat2->yMin;
					xPair2 += yCat2->cXPair;
					xPairEnd2 = xPair2 + yCat2->cXPair;
				}
				else// 2与1相交，拉平2
				{
					RgnResult.m_pComplex->AddYCategory(yMin2, yMin1,
						xPair2, yCat2->cXPair);
					yMin2 = yMin1;// 不要递增，1、2还留有一块未处理区域
				}
			}
			else// 1与2高度相同，这时可以方便地进行并运算
			{
				XPairUnion(xPair1, xPairEnd1, xPair2, xPairEnd2, vXPairWork);
				// 判断下部是否对齐
				if (yCat1->yMax < yCat2->yMax)// 1较短
				{
					RgnResult.m_pComplex->AddYCategory(yMin1, yCat1->yMax,
						vXPairWork.data(), (int)vXPairWork.size());
					yMin2 = yCat1->yMax;
					++yCat1;
					if (yCat1 == yCatEnd1)
						break;
					yMin1 = yCat1->yMin;
					xPair1 += yCat1->cXPair;
					xPairEnd1 = xPair1 + yCat1->cXPair;
				}
				else if (yCat2->yMax < yCat1->yMax)// 2较短
				{
					RgnResult.m_pComplex->AddYCategory(yMin2, yCat2->yMax,
						vXPairWork.data(), (int)vXPairWork.size());
					yMin1 = yCat2->yMax;
					++yCat2;
					if (yCat2 == yCatEnd2)
						break;
					yMin2 = yCat2->yMin;
					xPair2 += yCat2->cXPair;
					xPairEnd2 = xPair2 + yCat2->cXPair;
				}
				else// 对齐
				{
					RgnResult.m_pComplex->AddYCategory(yMin1, yCat1->yMax,
						vXPairWork.data(), (int)vXPairWork.size());
					BOOL bEnd{};
					++yCat1;
					if (yCat1 == yCatEnd1)
						bEnd = TRUE;
					else
					{
						yMin1 = yCat1->yMin;
						xPair1 += yCat1->cXPair;
						xPairEnd1 = xPair1 + yCat1->cXPair;
					}
					++yCat2;
					if (yCat2 == yCatEnd2)
						break;
					yMin2 = yCat2->yMin;
					xPair2 += yCat2->cXPair;
					xPairEnd2 = xPair2 + yCat2->cXPair;
					if (bEnd)
						break;
				}
			}
		}
		// 处理剩余部分，注意上次可能拉平了某Y分类，因此绝不能忽略yMin1和yMin2
		for (; yCat1 < yCatEnd1; ++yCat1)
		{
			RgnResult.m_pComplex->AddYCategory(yMin1, yCat1->yMax,
				xPair1, yCat1->cXPair);
			yMin1 = yCat1->yMin;
			xPair1 += yCat1->cXPair;
			xPairEnd1 = xPair1 + yCat1->cXPair;
		}
		for (; yCat2 < yCatEnd2; ++yCat2)
		{
			RgnResult.m_pComplex->AddYCategory(yMin2, yCat2->yMax,
				xPair2, yCat2->cXPair);
			yMin2 = yCat2->yMin;
			xPair2 += yCat2->cXPair;
			xPairEnd2 = xPair2 + yCat2->cXPair;
		}
		return RgnResult;
	}

	constexpr CRegion Difference(const CRegion& Rgn) const
	{
		EckAssert(!m_bRecordingPoint);
		CRegion RgnResult{};

		const XPair* xPair1;
		const XPair* xPairEnd1;
		const YCategory* yCat1;
		const YCategory* yCatEnd1;
		const XPair* xPair2;
		const XPair* xPairEnd2;
		const YCategory* yCat2;
		const YCategory* yCatEnd2;
		YCategory yCatDummy1;
		XPair xPairDummy1;
		YCategory yCatDummy2;
		XPair xPairDummy2;
		if (m_pComplex)
		{
			yCat1 = m_pComplex->m_vYCategory.data();
			yCatEnd1 = yCat1 + m_pComplex->m_vYCategory.size();
			xPair1 = m_pComplex->m_vXPair.data();
			xPairEnd1 = xPair1 + yCat1->cXPair;
		}
		else
		{
			xPair1 = &xPairDummy1;
			xPairEnd1 = xPair1 + 1;
			yCat1 = &yCatDummy1;
			yCatEnd1 = yCat1 + 1;
			xPairDummy1.xMin = m_SimpleRc.left;
			xPairDummy1.xMax = m_SimpleRc.right;
			yCatDummy1.yMin = m_SimpleRc.top;
			yCatDummy1.yMax = m_SimpleRc.bottom;
			yCatDummy1.idxXPairBegin = 0;
			yCatDummy1.cXPair = 1;
		}
		if (Rgn.m_pComplex)
		{
			yCat2 = Rgn.m_pComplex->m_vYCategory.data();
			yCatEnd2 = yCat2 + Rgn.m_pComplex->m_vYCategory.size();
			xPair2 = Rgn.m_pComplex->m_vXPair.data();
			xPairEnd2 = xPair2 + yCat2->cXPair;
		}
		else
		{
			xPair2 = &xPairDummy2;
			xPairEnd2 = xPair2 + 1;
			yCat2 = &yCatDummy2;
			yCatEnd2 = yCat2 + 1;
			xPairDummy2.xMin = Rgn.m_SimpleRc.left;
			xPairDummy2.xMax = Rgn.m_SimpleRc.right;
			yCatDummy2.yMin = Rgn.m_SimpleRc.top;
			yCatDummy2.yMax = Rgn.m_SimpleRc.bottom;
			yCatDummy2.idxXPairBegin = 0;
			yCatDummy2.cXPair = 1;
		}

		RgnResult.AllocComplexRegion();
		std::vector<XPair> vXPairWork{};
		vXPairWork.reserve((xPairEnd1 - xPair1) + (xPairEnd2 - xPair2));
		int yMin1 = yCat1->yMin;
		int yMin2 = yCat2->yMin;
		EckLoop()
		{
			if (yMin1 < yMin2)
			{
				if (yCat1->yMax <= yMin2)// 1不与2相交，合并1
				{
					RgnResult.m_pComplex->AddYCategory(yMin1, yCat1->yMax,
						xPair1, yCat1->cXPair);
					++yCat1;
					if (yCat1 == yCatEnd1)
						break;
					yMin1 = yCat1->yMin;
					xPair1 += yCat1->cXPair;
					xPairEnd1 = xPair1 + yCat1->cXPair;
				}
				else// 1与2相交，拉平1
				{
					RgnResult.m_pComplex->AddYCategory(yMin1, yMin2,
						xPair1, yCat1->cXPair);
					yMin1 = yMin2;// 不要递增，1、2还留有一块未处理区域
				}
			}
			else if (yMin1 < yCat2->yMax)
			{
				XPairDiff(xPair1, xPairEnd1, xPair2, xPairEnd2, vXPairWork);
				if (yCat1->yMax < yCat2->yMax)
				{
					RgnResult.m_pComplex->AddYCategory(yMin1, yCat1->yMax,
						vXPairWork.data(), (int)vXPairWork.size());
					++yCat1;
					if (yCat1 == yCatEnd1)
						break;
					yMin1 = yCat1->yMin;
					xPair1 += yCat1->cXPair;
					xPairEnd1 = xPair1 + yCat1->cXPair;
				}
				else
				{
					RgnResult.m_pComplex->AddYCategory(yMin1, yCat2->yMax,
						vXPairWork.data(), (int)vXPairWork.size());
					yMin1 = yCat2->yMax;
					++yCat2;
					if (yCat2 == yCatEnd2)
						break;
					yMin2 = yCat2->yMin;
					xPair2 += yCat2->cXPair;
					xPairEnd2 = xPair2 + yCat2->cXPair;
				}
			}
			else
			{
				++yCat1;
				if (yCat1 == yCatEnd1)
					break;
				yMin1 = yCat1->yMin;
				xPair1 += yCat1->cXPair;
				xPairEnd1 = xPair1 + yCat1->cXPair;
			}
		}
		// 处理左边剩余部分，注意上次可能拉平了某Y分类，因此绝不能忽略yMin1和yMin2
		for (; yCat1 < yCatEnd1; ++yCat1)
		{
			RgnResult.m_pComplex->AddYCategory(yMin1, yCat1->yMax,
				xPair1, yCat1->cXPair);
			yMin1 = yCat1->yMin;
			xPair1 += yCat1->cXPair;
			xPairEnd1 = xPair1 + yCat1->cXPair;
		}
		return RgnResult;
	}

	constexpr CRegion SymmetricDifference(const CRegion& Rgn) const
	{
		EckAssert(!m_bRecordingPoint);
		CRegion RgnResult{};

		const XPair* xPair1;
		const XPair* xPairEnd1;
		const YCategory* yCat1;
		const YCategory* yCatEnd1;
		const XPair* xPair2;
		const XPair* xPairEnd2;
		const YCategory* yCat2;
		const YCategory* yCatEnd2;
		YCategory yCatDummy1;
		XPair xPairDummy1;
		YCategory yCatDummy2;
		XPair xPairDummy2;
		if (m_pComplex)
		{
			yCat1 = m_pComplex->m_vYCategory.data();
			yCatEnd1 = yCat1 + m_pComplex->m_vYCategory.size();
			xPair1 = m_pComplex->m_vXPair.data();
			xPairEnd1 = xPair1 + yCat1->cXPair;
		}
		else
		{
			xPair1 = &xPairDummy1;
			xPairEnd1 = xPair1 + 1;
			yCat1 = &yCatDummy1;
			yCatEnd1 = yCat1 + 1;
			xPairDummy1.xMin = m_SimpleRc.left;
			xPairDummy1.xMax = m_SimpleRc.right;
			yCatDummy1.yMin = m_SimpleRc.top;
			yCatDummy1.yMax = m_SimpleRc.bottom;
			yCatDummy1.idxXPairBegin = 0;
			yCatDummy1.cXPair = 1;
		}
		if (Rgn.m_pComplex)
		{
			yCat2 = Rgn.m_pComplex->m_vYCategory.data();
			yCatEnd2 = yCat2 + Rgn.m_pComplex->m_vYCategory.size();
			xPair2 = Rgn.m_pComplex->m_vXPair.data();
			xPairEnd2 = xPair2 + yCat2->cXPair;
		}
		else
		{
			xPair2 = &xPairDummy2;
			xPairEnd2 = xPair2 + 1;
			yCat2 = &yCatDummy2;
			yCatEnd2 = yCat2 + 1;
			xPairDummy2.xMin = Rgn.m_SimpleRc.left;
			xPairDummy2.xMax = Rgn.m_SimpleRc.right;
			yCatDummy2.yMin = Rgn.m_SimpleRc.top;
			yCatDummy2.yMax = Rgn.m_SimpleRc.bottom;
			yCatDummy2.idxXPairBegin = 0;
			yCatDummy2.cXPair = 1;
		}

		RgnResult.AllocComplexRegion();
		std::vector<XPair> vXPairWork{};
		vXPairWork.reserve((xPairEnd1 - xPair1) + (xPairEnd2 - xPair2));
		int yMin1 = yCat1->yMin;
		int yMin2 = yCat2->yMin;
		EckLoop()
		{
			if (yMin1 < yMin2)
			{
				if (yCat1->yMax <= yMin2)// 1不与2相交，合并1
				{
					RgnResult.m_pComplex->AddYCategory(yMin1, yCat1->yMax,
						xPair1, yCat1->cXPair);
					++yCat1;
					if (yCat1 == yCatEnd1)
						break;
					yMin1 = yCat1->yMin;
					xPair1 += yCat1->cXPair;
					xPairEnd1 = xPair1 + yCat1->cXPair;
				}
				else// 1与2相交，拉平1
				{
					RgnResult.m_pComplex->AddYCategory(yMin1, yMin2,
						xPair1, yCat1->cXPair);
					yMin1 = yMin2;// 不要递增，1、2还留有一块未处理区域
				}
			}
			else if (yMin2 < yMin1)
			{
				if (yCat2->yMax <= yMin1)// 2不与1相交，合并2
				{
					RgnResult.m_pComplex->AddYCategory(yMin2, yCat2->yMax,
						xPair2, yCat2->cXPair);
					++yCat2;
					if (yCat2 == yCatEnd2)
						break;
					yMin2 = yCat2->yMin;
					xPair2 += yCat2->cXPair;
					xPairEnd2 = xPair2 + yCat2->cXPair;
				}
				else// 2与1相交，拉平2
				{
					RgnResult.m_pComplex->AddYCategory(yMin2, yMin1,
						xPair2, yCat2->cXPair);
					yMin2 = yMin1;// 不要递增，1、2还留有一块未处理区域
				}
			}
			else// 1与2高度相同，这时可以方便地进行对称差运算
			{
				XPairSymDiff(xPair1, xPairEnd1, xPair2, xPairEnd2, vXPairWork);
				// 判断下部是否对齐
				if (yCat1->yMax < yCat2->yMax)// 1较短
				{
					RgnResult.m_pComplex->AddYCategory(yMin1, yCat1->yMax,
						vXPairWork.data(), (int)vXPairWork.size());
					yMin2 = yCat1->yMax;
					++yCat1;
					if (yCat1 == yCatEnd1)
						break;
					yMin1 = yCat1->yMin;
					xPair1 += yCat1->cXPair;
					xPairEnd1 = xPair1 + yCat1->cXPair;
				}
				else if (yCat2->yMax < yCat1->yMax)// 2较短
				{
					RgnResult.m_pComplex->AddYCategory(yMin1, yCat2->yMax,
						vXPairWork.data(), (int)vXPairWork.size());
					yMin1 = yCat2->yMax;
					++yCat2;
					if (yCat2 == yCatEnd2)
						break;
					yMin2 = yCat2->yMin;
					xPair2 += yCat2->cXPair;
					xPairEnd2 = xPair2 + yCat2->cXPair;
				}
				else// 对齐
				{
					RgnResult.m_pComplex->AddYCategory(yMin1, yCat1->yMax,
						vXPairWork.data(), (int)vXPairWork.size());
					BOOL bEnd{};
					++yCat1;
					if (yCat1 == yCatEnd1)
						bEnd = TRUE;
					else
					{
						yMin1 = yCat1->yMin;
						xPair1 += yCat1->cXPair;
						xPairEnd1 = xPair1 + yCat1->cXPair;
					}
					++yCat2;
					if (yCat2 == yCatEnd2)
						break;
					yMin2 = yCat2->yMin;
					xPair2 += yCat2->cXPair;
					xPairEnd2 = xPair2 + yCat2->cXPair;
					if (bEnd)
						break;
				}
			}
		}
		// 处理剩余部分，注意上次可能拉平了某Y分类，因此绝不能忽略yMin1和yMin2
		for (; yCat1 < yCatEnd1; ++yCat1)
		{
			RgnResult.m_pComplex->AddYCategory(yMin1, yCat1->yMax,
				xPair1, yCat1->cXPair);
			yMin1 = yCat1->yMin;
			xPair1 += yCat1->cXPair;
			xPairEnd1 = xPair1 + yCat1->cXPair;
		}
		for (; yCat2 < yCatEnd2; ++yCat2)
		{
			RgnResult.m_pComplex->AddYCategory(yMin2, yCat2->yMax,
				xPair2, yCat2->cXPair);
			yMin2 = yCat2->yMin;
			xPair2 += yCat2->cXPair;
			xPairEnd2 = xPair2 + yCat2->cXPair;
		}
		return RgnResult;
	}

	/// <summary>
	/// 开始记录点。
	/// 函数清空当前区域，并返回记录器。此为点并区域的优化实现，只可自上而下记录点，但可以跨越空行
	/// </summary>
	CPtRecorder RpBegin()
	{
#ifdef _DEBUG
		EckAssert(!m_bRecordingPoint && L"Recording point already started.");
		m_bRecordingPoint = TRUE;
#endif
		if (!m_pComplex)
			AllocComplexRegion();
		m_pComplex->Reset();
		return CPtRecorder(m_pComplex);
	}

	EckInline constexpr void RpEnd([[maybe_unused]] const CPtRecorder& rpr)
	{
#ifdef _DEBUG
		EckAssert(m_bRecordingPoint && L"Recording point not started.");
		m_bRecordingPoint = FALSE;
#endif
	}
};
ECK_NAMESPACE_END