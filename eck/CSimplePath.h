#pragma once
#include "CUnknown.h"
#include "Utility.h"

#include "../ThirdPartyLib/SkiaTessellator/GrAATriangulator.h"
#include "../ThirdPartyLib/SkiaTessellator/ISkiaPolygonAccessor.h"

ECK_NAMESPACE_BEGIN
enum class PathType : BYTE
{
	None,
	Move = SkiaTessellator::PathType::Move,
	Line = SkiaTessellator::PathType::Line,
	Bezier = 1 << 2,

	Close = 1 << 7,
	Fill = 1 << 6,
	End = 1 << 5,

	FlagMask = Close | Fill | End,
};
ECK_ENUM_BIT_FLAGS(PathType);

class CSimplePath final : public CUnknown<CSimplePath, ID2D1SimplifiedGeometrySink>
{
public:
	using TPoint = D2D1_POINT_2F;

	struct PathFlattenData
	{
		std::vector<TPoint> vPoint{};
		std::vector<PathType> vType{};
		float fTolerance{ 0.25f };
		BYTE eFillMode{ D2D1_FILL_MODE_WINDING };
		BOOLEAN bBoundsValid{ FALSE };
		D2D1_RECT_F rcBounds{};
	};
private:

	std::vector<TPoint> m_vPoint{};
	std::vector<PathType> m_vType{};
	TPoint m_ptOffset{};
	BYTE m_eFigureOpt{};
	BYTE m_eFillMode{ D2D1_FILL_MODE_WINDING };
	BOOLEAN m_bBezier{};
	BOOLEAN m_bTranslate{};
#ifdef _DEBUG
	BOOLEAN m_bDbgInFigure{};
	BOOLEAN m_bDbgClosed{};
#endif

	static float PtLen(TPoint a) noexcept { return std::sqrt(a.x * a.x + a.y * a.y); }
	static constexpr TPoint PtAdd(TPoint a, TPoint b) noexcept { return { a.x + b.x, a.y + b.y }; }
	static constexpr TPoint PtSub(TPoint a, TPoint b) noexcept { return { a.x - b.x, a.y - b.y }; }
	static constexpr TPoint PtMul(TPoint a, float s) noexcept { return { a.x * s, a.y * s }; }

public:
	STDMETHOD_(void, SetFillMode)(D2D1_FILL_MODE eFillMode) override
	{
		m_eFillMode = (BYTE)eFillMode;
	}

	STDMETHOD_(void, SetSegmentFlags)(D2D1_PATH_SEGMENT eSegOpt) override {}

	STDMETHOD_(void, BeginFigure)(TPoint ptStart, D2D1_FIGURE_BEGIN eOpt) override
	{
#ifdef _DEBUG
		EckAssert(!m_bDbgClosed);
		if (m_bDbgInFigure)
			EckDbgBreak();
		m_bDbgInFigure = TRUE;
#endif
		if (m_bTranslate)
			ptStart = PtAdd(ptStart, m_ptOffset);
		m_vPoint.emplace_back(ptStart);
		m_vType.emplace_back(PathType::Move | (eOpt == D2D1_FIGURE_BEGIN_FILLED ?
			PathType::Fill : PathType::None));
		m_eFigureOpt = (BYTE)eOpt;
	}

	STDMETHOD_(void, AddLines)(_In_reads_(cPt) CONST TPoint* ppt, UINT32 cPt) override
	{
		EckAssert(!m_bDbgClosed);
		if (m_bTranslate)
			for (UINT32 i = 0; i < cPt; ++i)
				m_vPoint.push_back(PtAdd(ppt[i], m_ptOffset));
		else
			m_vPoint.insert(m_vPoint.end(), ppt, ppt + cPt);
		m_vType.insert(m_vType.end(), cPt, PathType::Line);
	}

	STDMETHOD_(void, AddBeziers)(_In_reads_(cBezier)
		CONST D2D1_BEZIER_SEGMENT* pBezier, UINT32 cBezier) override
	{
		EckAssert(!m_bDbgClosed);
		m_bBezier = TRUE;
		if (m_bTranslate)
		{
			const auto ppt = (TPoint*)pBezier;
			for (UINT32 i = 0; i < cBezier * 3; ++i)
				m_vPoint.push_back(PtAdd(ppt[i], m_ptOffset));
		}
		else
			m_vPoint.insert(m_vPoint.end(), (TPoint*)pBezier,
				(TPoint*)(pBezier + cBezier));
		m_vType.insert(m_vType.end(), cBezier * 3, PathType::Bezier);
	}

	STDMETHOD_(void, EndFigure)(D2D1_FIGURE_END eFigureEnd) override
	{
#ifdef _DEBUG
		EckAssert(!m_bDbgClosed);
		if (!m_bDbgInFigure)
			EckDbgBreak();
		m_bDbgInFigure = FALSE;
#endif
		if (m_vPoint.size() < 2)
			return;
		const auto eType = PathType::Line | PathType::End |
			(m_eFigureOpt == D2D1_FIGURE_BEGIN_FILLED ?
				PathType::Fill : PathType::Line);
		if (eFigureEnd == D2D1_FIGURE_END_CLOSED &&
			FloatEqual(m_vPoint.front().x, m_vPoint.back().x) &&
			FloatEqual(m_vPoint.front().y, m_vPoint.back().y))
		{
			m_vPoint.push_back(m_vPoint[0]);
			m_vType.push_back(eType);
		}
		else
			m_vType.back() = eType;
	}

	STDMETHOD(Close)() override
	{
#ifdef _DEBUG
		EckAssert(!m_bDbgClosed);
		if (m_bDbgInFigure)
			EckDbgBreak();
		m_bDbgClosed = TRUE;
#endif
		return S_OK;
	}

	// 将路径内容写入D2D简单几何接收器
	void Stream(ID2D1SimplifiedGeometrySink* pSink) const noexcept
	{
		for (size_t i = 0; i < m_vPoint.size(); ++i)
		{
			auto eType = m_vType[i];
			if ((eType & PathType::Move) == PathType::Move)
			{
				pSink->BeginFigure(m_vPoint[i], ((eType & PathType::Fill) != PathType::None) ?
					D2D1_FIGURE_BEGIN_FILLED : D2D1_FIGURE_BEGIN_HOLLOW);
			}
			else
			{
				switch (eType & ~PathType::FlagMask)
				{
				case PathType::None:
					DebugBreak();
					break;
				case PathType::Line:
					pSink->AddLines(&m_vPoint[i], 1);
					break;
				case PathType::Bezier:
					pSink->AddBeziers((D2D1_BEZIER_SEGMENT*)&m_vPoint[i], 1);
					i += 2;
					eType = m_vType[i];
					break;
				}
				if ((eType & PathType::End) != PathType::None)
					pSink->EndFigure((eType & PathType::Close) == PathType::Close ?
						D2D1_FIGURE_END_CLOSED : D2D1_FIGURE_END_OPEN);
			}
		}
	}

	void Flatten(std::vector<TPoint>& vPoint, std::vector<PathType>& vType,
		float fTolerance = 0.25f) const noexcept
	{
		if (m_vPoint.empty() || m_vType.empty())
			return;
		const size_t cPt = m_vPoint.size();
		for (size_t i = 0; i < cPt; ++i)
		{
			PathType eType = m_vType[i];
			if ((eType & PathType::Bezier) == PathType::Bezier)
			{
				EckAssert(!vPoint.empty());// 没有起点
				EckAssert(i + 2 < cPt);// 曲线不完整
				TPoint P0 = m_vPoint[i - 1];// 起始
				TPoint P1 = m_vPoint[i + 0];// 控点1
				TPoint P2 = m_vPoint[i + 1];// 控点2
				TPoint P3 = m_vPoint[i + 2];// 终止
				// 计算power-basis系数a,b,c,d
				TPoint a = PtAdd(PtAdd(PtSub(P3, PtMul(P2, 3.f)), PtMul(P1, 3.f)), PtMul(P0, -1.f));
				TPoint b = PtAdd(PtAdd(PtMul(P2, 3.f), PtMul(P1, -6.f)), PtMul(P0, 3.f));
				TPoint c = PtMul(PtSub(P1, P0), 3.f);
				TPoint d = P0;
				// 二阶导上界估算段数n，Mdd = max(||2b||, ||6a+2b||)
				TPoint two_b = PtMul(b, 2.f);
				TPoint six_a = PtMul(a, 6.f);
				TPoint six_a_plus_two_b = PtAdd(six_a, two_b);
				float Mdd = std::max(PtLen(two_b), PtLen(six_a_plus_two_b));
				UINT n = 1;
				if (Mdd > 0.f)
				{
					float n_est = std::sqrt(Mdd / (8.f * fTolerance));
					n = (UINT)std::ceil(std::max(1.f, n_est));
					//n = std::min(n, MaxSeg);// 限制最大段数
				}
				// 前向差分
				float s = 1.f / (float)n;
				float s2 = s * s, s3 = s2 * s;

				TPoint p = d;
				TPoint d1 = PtAdd(PtAdd(PtMul(a, s3), PtMul(b, s2)), PtMul(c, s));
				TPoint d2 = PtAdd(PtMul(a, 6.f * s3), PtMul(b, 2.f * s2));
				TPoint d3 = PtMul(a, 6.f * s3);

				for (UINT k = 0; k < n; ++k)
				{
					p = PtAdd(p, d1);
					d1 = PtAdd(d1, d2);
					d2 = PtAdd(d2, d3);
					vPoint.emplace_back(p);
					vType.push_back(PathType::Line);
				}
				// 如果贝塞尔曲线终点是子图形终点，则复制它的标志
				if ((m_vType[i + 2] & PathType::End) != PathType::None)
					vType.back() |= (m_vType[i + 2] & PathType::FlagMask);
				i += 2;// 跳过本段使用的3个点
			}
			else
			{
				vPoint.emplace_back(m_vPoint[i]);
				vType.push_back(eType);
			}
		}
	}

	void Flatten(PathFlattenData& Data, float fTolerance = 0.25f) const noexcept
	{
		Data.vPoint.clear();
		Data.vType.clear();
		Data.fTolerance = fTolerance;
		Data.eFillMode = (D2D1_FILL_MODE)m_eFillMode;
		Data.bBoundsValid = FALSE;
		Flatten(Data.vPoint, Data.vType, fTolerance);
	}

	static void StreamFlattenResult(std::vector<TPoint>& vPoint,
		std::vector<PathType>& vType, ID2D1SimplifiedGeometrySink* pSink) noexcept
	{
		for (int i = 0; i < vPoint.size(); ++i)
		{
			if ((vType[i] & PathType::Move) != PathType::None)
			{
				const auto eBegin = (vType[i] & PathType::Fill) == PathType::Fill ?
					D2D1_FIGURE_BEGIN_FILLED : D2D1_FIGURE_BEGIN_HOLLOW;
				pSink->BeginFigure(vPoint[i], eBegin);
			}
			else
			{
				pSink->AddLines(&vPoint[i], 1);
				if ((vType[i] & PathType::End) != PathType::None)
				{
					const auto eEnd = (vType[i] & PathType::Close) == PathType::Close ?
						D2D1_FIGURE_END_CLOSED : D2D1_FIGURE_END_OPEN;
					EckDbgPrint(eEnd);
					pSink->EndFigure(eEnd);
				}
			}
		}
	}

	EckInlineCe void SetTranslate(float dx, float dy) noexcept { m_ptOffset = { dx,dy }; }
	EckInlineCe void SetTranslate(BOOLEAN bTranslate) { m_bTranslate = bTranslate; }
	EckInlineCe void SetTranslate(BOOLEAN bTranslate, float dx, float dy) noexcept
	{
		SetTranslate(bTranslate); SetTranslate(dx, dy);
	}

	EckInlineNdCe D2D1_FILL_MODE GetFillMode() const noexcept { return (D2D1_FILL_MODE)m_eFillMode; }
};

struct CSimplePathFlattenData :
	public SkiaTessellator::ISkiaPolygonAccessor,
	public CSimplePath::PathFlattenData
{
	virtual ~CSimplePathFlattenData() = default;

	STDMETHOD_(BOOL, PaIsEmpty)() override { return vPoint.empty(); }

	STDMETHOD_(pk::SkPathFillType, PaGetFillType)() override
	{
		if (eFillMode == D2D1_FILL_MODE_ALTERNATE)
			return pk::SkPathFillType::kEvenOdd;
		else
			return pk::SkPathFillType::kWinding;
	}

	STDMETHOD(PaGetFlattenData)(_Out_opt_ pk::SkPoint const** ppt,
		_Out_opt_ SkiaTessellator::PathType const** ppeType, _Out_ UINT* pcPoint) override
	{
		if (ppt) *ppt = (pk::SkPoint*)vPoint.data();
		if (ppeType) *ppeType = (SkiaTessellator::PathType*)vType.data();
		*pcPoint = (UINT)vPoint.size();
		return S_OK;
	}

	STDMETHOD(PaGetBounds)(_Out_ pk::SkRect* prc) override
	{
		if (!bBoundsValid)
		{
			rcBounds = { FLT_MAX,FLT_MAX,FLT_MIN,FLT_MIN };
			for (auto& e : vPoint)
			{
				rcBounds.left = std::min(rcBounds.left, e.x);
				rcBounds.top = std::min(rcBounds.top, e.y);
				rcBounds.right = std::max(rcBounds.right, e.x);
				rcBounds.bottom = std::max(rcBounds.bottom, e.y);
			}
			bBoundsValid = TRUE;
		}
		*prc = *(pk::SkRect*)&rcBounds;
		return S_OK;
	}

	/*
		STDMETHOD_(void, PaAppendTriangle)(_In_reads_(9) float* pVetAndAlpha) override {}
		STDMETHOD(PaReserveVertex)(UINT cVert) override {}
		STDMETHOD_(UINT, PaGetVertexCount)() override {}
	*/
};

EckInline void Tessellate(
	SkiaTessellator::ISkiaPolygonAccessor* pAccessor, BOOL bAntiAlias = TRUE)
{
	if (bAntiAlias)
		pk::GrAATriangulator::PathToAATriangles(pAccessor);
	else
	{
		bool b;
		pk::GrTriangulator::PathToTriangles(pAccessor, &b);
	}
}
ECK_NAMESPACE_END