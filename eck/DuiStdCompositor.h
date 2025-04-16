#pragma once
#include "DuiBase.h"
#include "MathHelper.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
// 四边形映射变换，必须使用CalcDistortMatrix/CalcInverseDistortMatrix计算矩阵
class CCompositorCornerMapping : public CCompositor
{
public:
	DirectX::XMMATRIX Mat{};
	DirectX::XMMATRIX MatR{};
private:
	void PtXToX(CElem* pElem, _Inout_ POINT& pt, BOOL bNormalToComposited)
	{
		auto p = DirectX::XMVector4Transform({ (float)pt.x,(float)pt.y,0.f,1.f },
			bNormalToComposited ? MatR : Mat);
		const auto w = DirectX::XMVectorGetW(p);
		p = DirectX::XMVectorDivide(p, DirectX::XMVectorSet(w, w, w, w));
		pt.x = (LONG)DirectX::XMVectorGetX(p);
		pt.y = (LONG)DirectX::XMVectorGetY(p);
	}
public:
	void PtNormalToComposited(CElem* pElem, _Inout_ POINT& pt) override
	{
		PtXToX(pElem, pt, TRUE);
	}
	void PtCompositedToNormal(CElem* pElem, _Inout_ POINT& pt) override
	{
		PtXToX(pElem, pt, FALSE);
	}
	void CalcCompositedRect(CElem* pElem, _Out_ RECT& rc, BOOL bInClientOrParent) override
	{
		const auto cx = pElem->GetWidthF();
		const auto cy = pElem->GetHeightF();
		const D2D1_POINT_2F pt[]{ { 0,0 }, { cx,0 }, { cx,cy }, { 0,cy } };

		float l{ FLT_MAX }, t{ FLT_MAX }, r{ FLT_MIN }, b{ FLT_MIN };
		for (const auto e : pt)
		{
			auto p = DirectX::XMVector4Transform({ e.x, e.y, 0.f, 1.f }, Mat);
			const auto w = DirectX::XMVectorGetW(p);
			p = DirectX::XMVectorDivide(p, DirectX::XMVectorSet(w, w, w, w));
			const auto x = DirectX::XMVectorGetX(p);
			const auto y = DirectX::XMVectorGetY(p);
			if (x < l) l = x;
			if (x > r) r = x;
			if (y < t) t = y;
			if (y > b) b = y;
		}
		rc = { (LONG)floorf(l), (LONG)floorf(t), (LONG)ceilf(r), (LONG)ceilf(b) };
		pElem->ElemToClient(rc);
		if (!bInClientOrParent && pElem->GetParentElem())
			pElem->GetParentElem()->ClientToElem(rc);
	}
	BOOL IsInPlace() const override { return FALSE; }
	void PostRender(COMP_RENDER_INFO& cri) override
	{
		cri.pDC->DrawBitmap(cri.pBitmap, cri.rcDst, 1.f,
			D2D1_INTERPOLATION_MODE_CUBIC, cri.rcSrc, (D2D1_MATRIX_4X4_F*)&Mat);
	}
};

// 2D仿射变换
class CCompositor2DAffineTransform : public CCompositor
{
public:
	D2D1::Matrix3x2F Mat{};
	D2D1::Matrix3x2F MatR{};
private:
	void PtXToX(CElem* pElem, _Inout_ POINT& pt, BOOL bNormalToComposited)
	{
		D2D1_POINT_2F pt0;
		if (bNormalToComposited)
			pt0 = MatR.TransformPoint({ (float)pt.x, (float)pt.y });
		else
			pt0 = Mat.TransformPoint({ (float)pt.x, (float)pt.y });
		pt.x = (LONG)pt0.x;
		pt.y = (LONG)pt0.y;
	}
public:
	void PtNormalToComposited(CElem* pElem, _Inout_ POINT& pt) override
	{
		PtXToX(pElem, pt, TRUE);
	}
	void PtCompositedToNormal(CElem* pElem, _Inout_ POINT& pt) override
	{
		PtXToX(pElem, pt, FALSE);
	}
	void CalcCompositedRect(CElem* pElem, _Out_ RECT& rc, BOOL bInClientOrParent) override
	{
		const auto cx = pElem->GetWidthF();
		const auto cy = pElem->GetHeightF();
		const D2D1_POINT_2F pt[]{ { 0,0 }, { cx,0 }, { cx,cy }, { 0,cy } };

		float l{ FLT_MAX }, t{ FLT_MAX }, r{ FLT_MIN }, b{ FLT_MIN };
		for (const auto e : pt)
		{
			const auto pt0 = Mat.TransformPoint(e);
			if (pt0.x < l) l = pt0.x;
			if (pt0.x > r) r = pt0.x;
			if (pt0.y < t) t = pt0.y;
			if (pt0.y > b) b = pt0.y;
		}
		rc = { (LONG)floorf(l), (LONG)floorf(t), (LONG)ceilf(r), (LONG)ceilf(b) };
		pElem->ElemToClient(rc);
		if (!bInClientOrParent && pElem->GetParentElem())
			pElem->GetParentElem()->ClientToElem(rc);
	}
	BOOL IsInPlace() const override { return FALSE; }
	void PostRender(COMP_RENDER_INFO& cri) override
	{
		D2D1::Matrix3x2F MatOld;
		cri.pDC->GetTransform(&MatOld);
		cri.pDC->SetTransform(Mat * MatOld);
		cri.pDC->DrawBitmap(cri.pBitmap, cri.rcDst, 1.f,
			D2D1_INTERPOLATION_MODE_CUBIC, cri.rcSrc);
		cri.pDC->SetTransform(MatOld);
	}

	void InverseMatrix()
	{
		MatR = Mat;
		MatR.Invert();
	}
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END