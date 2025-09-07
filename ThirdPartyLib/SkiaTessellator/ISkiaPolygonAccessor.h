#pragma once
namespace SkiaTessellator
{
	enum class PathType : BYTE
	{
		Move = 0x01,
		Line = 0x02,

		Mask = Move | Line
	};

	// �ṩ���ʶ�������ݵ�����
	struct ISkiaPolygonAccessor
	{
		STDMETHOD_(BOOL, PaIsEmpty)() = 0;

		STDMETHOD_(pk::SkPathFillType, PaGetFillType)() = 0;
		/// <summary>
		/// ȡ�������Ϣ
		/// </summary>
		/// <param name="ppt">������</param>
		/// <param name="ppeType">���ͣ�ϸ����ʹ��PathType::Mask��������λ</param>
		/// <param name="pcPoint">����</param>
		/// <returns>S_OK</returns>
		STDMETHOD(PaGetFlattenData)(_Out_opt_ pk::SkPoint const** ppt,
			_Out_opt_ PathType const** ppeType, _Out_ UINT* pcPoint) = 0;
		// ��ʧ�ܣ���ϸ����ʹ��PaGetFlattenData�Ľ������߽�
		STDMETHOD(PaGetBounds)(_Out_ pk::SkRect* prc) = 0;
		// ���һ�������Σ�ÿ������Ϊ(x, y, alpha)
		STDMETHOD_(void, PaAppendTriangle)(_In_reads_(9) float* pVetAndAlpha) = 0;
		// ϸ�������ô˷���ָʾӦԤ�����ٶ�����ڴ�
		STDMETHOD(PaReserveVertex)(UINT cVert) = 0;
		// ȡ��ǰ������
		STDMETHOD_(UINT, PaGetVertexCount)() = 0;
	};
}