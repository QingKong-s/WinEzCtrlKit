#pragma once
#include "DuiBase.h"
#include "MathHelper.h"

#if !ECKCXX20
#error "EckDui requires C++20"
#endif// !ECKCXX20

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
// 为4x4矩阵表示的非仿射变换提供默认点变换实现
inline void CptTransformPointNonAffine(const D2D1_MATRIX_4X4_F& Mat_,
	_Inout_ POINT& pt)
{
	const DirectX::XMFLOAT4 VecOrg{ (float)pt.x,(float)pt.y,0.f,1.f };
	const auto Vec = DirectX::XMVector4Transform(
		DirectX::XMLoadFloat4(&VecOrg), 
		DirectX::XMLoadFloat4x4((DirectX::XMFLOAT4X4*)&Mat_));
	const auto w = DirectX::XMVectorGetW(Vec);
	pt.x = int(DirectX::XMVectorGetX(Vec) / w);
	pt.y = int(DirectX::XMVectorGetY(Vec) / w);
	auto r = DirectX::XMVectorDivide(Vec, DirectX::XMVectorReplicate(w));
	r = {};
}

inline void CptTransformPointNonAffine(const D2D1_MATRIX_4X4_F& Mat_,
	const D2D1_MATRIX_4X4_F& MatR_, _Inout_ COMP_POS* pcp)
{
	if (pcp->bNormalToComp)
		CptTransformPointNonAffine(MatR_, pcp->pt);
	else
		CptTransformPointNonAffine(Mat_, pcp->pt);
}
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END