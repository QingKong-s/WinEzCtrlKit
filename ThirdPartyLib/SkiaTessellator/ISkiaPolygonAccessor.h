#pragma once
namespace SkiaTessellator
{
	enum class PathType : BYTE
	{
		Move = 0x01,
		Line = 0x02,

		Mask = Move | Line
	};

	// 提供访问多边形数据的能力
	struct ISkiaPolygonAccessor
	{
		STDMETHOD_(BOOL, PaIsEmpty)() = 0;

		STDMETHOD_(pk::SkPathFillType, PaGetFillType)() = 0;
		/// <summary>
		/// 取多边形信息
		/// </summary>
		/// <param name="ppt">点数组</param>
		/// <param name="ppeType">类型，细分器使用PathType::Mask屏蔽其他位</param>
		/// <param name="pcPoint">点数</param>
		/// <returns>S_OK</returns>
		STDMETHOD(PaGetFlattenData)(_Out_opt_ pk::SkPoint const** ppt,
			_Out_opt_ PathType const** ppeType, _Out_ UINT* pcPoint) = 0;
		// 若失败，则细分器使用PaGetFlattenData的结果计算边界
		STDMETHOD(PaGetBounds)(_Out_ pk::SkRect* prc) = 0;
		// 添加一个三角形，每组数据为(x, y, alpha)
		STDMETHOD_(void, PaAppendTriangle)(_In_reads_(9) float* pVetAndAlpha) = 0;
		// 细分器调用此方法指示应预留多少顶点的内存
		STDMETHOD(PaReserveVertex)(UINT cVert) = 0;
		// 取当前顶点数
		STDMETHOD_(UINT, PaGetVertexCount)() = 0;
	};
}