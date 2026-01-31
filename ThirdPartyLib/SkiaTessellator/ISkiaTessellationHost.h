#pragma once
namespace SkiaTessellator
{
    enum class PathType : BYTE
    {
        Begin = 1 << 7,
        Line = 1 << 6,
        Fill = 1 << 5,

        Mask = Begin | Line | Fill
    };

    // 提供访问多边形数据以及录制顶点和索引的能力，此接口由多边形数据提供者实现
    // 细分器向接口输出带索引的三角形列表，三角形绕序不确定，因此渲染时应禁用面剔除
    struct ITessellationHost
    {
        STDMETHOD_(BOOL, PaIsEmpty)() noexcept = 0;

        STDMETHOD_(pk::SkPathFillType, PaGetFillType)() noexcept = 0;

        /// <summary>
        /// 取多边形信息
        /// </summary>
        /// <param name="ppt">点数组</param>
        /// <param name="ppeType">类型，细分器使用PathType::Mask屏蔽其他位</param>
        /// <param name="pcPoint">点数</param>
        /// <returns>S_OK</returns>
        STDMETHOD(PaGetFlattenData)(
            _Out_opt_ pk::SkPoint const** ppt,
            _Out_opt_ PathType const** ppeType,
            _Out_ UINT* pcPoint) noexcept = 0;

        // 若失败，则细分器使用PaGetFlattenData的结果计算边界
        STDMETHOD(PaGetBounds)(_Out_ pk::SkRect* prc) noexcept = 0;


        // 即将追加指定数量个顶点
        STDMETHOD(TslReserveVertex)(UINT cVert) noexcept = 0;
        // 即将追加指定数量个索引
        STDMETHOD(TslReserveIndex)(UINT cIdx) noexcept = 0;

        STDMETHOD_(void, TslAppendVertex)(float x, float y, float fAlpha) noexcept = 0;
        STDMETHOD_(void, TslAppendTriangle)(UINT i0, UINT i1, UINT i2) noexcept = 0;

        STDMETHOD_(UINT, TslGetVertexCount)() noexcept = 0;
    };
}