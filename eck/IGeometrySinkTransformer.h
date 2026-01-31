#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
constexpr IID IID_IGeometrySinkTransformer
{ 0x6abd0903, 0x4980, 0x48a0, { 0xa7, 0xd4, 0x2a, 0x9c, 0x36, 0x36, 0x3d, 0x14 } };

// 提供控制D2D简单几何接收器转换层的能力
// 转换层在向几何接收器传递数据前对点集做指定的变换
struct __declspec(uuid("6ABD0903-4980-48A0-A7D4-2A9C36363D14"))
    IGeometrySinkTransformer : public IUnknown
{
    // 指定写入数据的目标接收器，在写入前转换层执行必要的变换
    // 可传递nullptr
    STDMETHOD(GstSetSink)(ID2D1SimplifiedGeometrySink* pSink) = 0;
    STDMETHOD(GstGetSink)(ID2D1SimplifiedGeometrySink** ppSink) = 0;

    // 下列两种变换模式互斥，若同时启用实现可能报告错误

    // 以下函数控制由3x2矩阵指定的变换，若实现不支持，则应返回E_NOTIMPL

    STDMETHOD(GstEnableTransform)(BOOL b) = 0;
    STDMETHOD(GstIsTransformEnabled)(BOOL* pIsEnabled) = 0;
    STDMETHOD(GstSetMatrix)(const D2D1_MATRIX_3X2_F* pMatrix) = 0;

    // 以下函数控制平移变换，若实现不支持，则应返回E_NOTIMPL

    STDMETHOD(GstEnableOffset)(BOOL b) = 0;
    STDMETHOD(GstIsOffsetEnabled)(BOOL* pIsEnabled) = 0;
    STDMETHOD(GstSetOffset)(float dx, float dy) = 0;
};
ECK_NAMESPACE_END