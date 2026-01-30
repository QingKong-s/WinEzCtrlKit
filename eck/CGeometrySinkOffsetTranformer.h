#pragma once
#include "IGeometrySinkTransformer.h"
#include "CUnknown.h"
#include "ComPtr.h"
#include "CTrivialBuffer.h"

ECK_NAMESPACE_BEGIN
class CGeometrySinkOffsetTranformer final : public CUnknown<CGeometrySinkOffsetTranformer,
    ID2D1SimplifiedGeometrySink,
    IGeometrySinkTransformer>
{
private:
    ComPtr<ID2D1SimplifiedGeometrySink> m_pSink{};
    float m_ox{}, m_oy{};
    BOOL m_bOffsetEnabled{};
    CTrivialBuffer<D2D1_POINT_2F> m_vTemp{};
public:
    STDMETHOD_(void, SetFillMode)(D2D1_FILL_MODE e) override { m_pSink->SetFillMode(e); }

    STDMETHOD_(void, SetSegmentFlags)(D2D1_PATH_SEGMENT e) override { m_pSink->SetSegmentFlags(e); }

    STDMETHOD_(void, BeginFigure)(D2D1_POINT_2F startPoint, D2D1_FIGURE_BEGIN figureBegin) override
    {
        startPoint.x += m_ox;
        startPoint.y += m_oy;
        m_pSink->BeginFigure(startPoint, figureBegin);
    }

    STDMETHOD_(void, AddLines)(CONST D2D1_POINT_2F* p, UINT32 c) override
    {
        if (m_bOffsetEnabled)
        {
            m_vTemp.Assign(p, c);
            for (auto& e : m_vTemp)
            {
                e.x += m_ox;
                e.y += m_oy;
            }
            m_pSink->AddLines(m_vTemp.Data(), c);
        }
        else
            m_pSink->AddLines(p, c);
    }

    STDMETHOD_(void, AddBeziers)(CONST D2D1_BEZIER_SEGMENT* p, UINT32 c) override
    {
        if (m_bOffsetEnabled)
        {
            m_vTemp.Assign((D2D1_POINT_2F*)p, c * 3);
            for (auto& e : m_vTemp)
            {
                e.x += m_ox;
                e.y += m_oy;
            }
            m_pSink->AddBeziers((D2D1_BEZIER_SEGMENT*)m_vTemp.Data(), c);
        }
        else
            m_pSink->AddBeziers(p, c);
    }

    STDMETHOD_(void, EndFigure)(D2D1_FIGURE_END figureEnd) override { m_pSink->EndFigure(figureEnd); }

    STDMETHOD(Close)() override { return m_pSink->Close(); }

    STDMETHOD(GstSetSink)(ID2D1SimplifiedGeometrySink* pSink) override
    {
        m_pSink = pSink;
        return S_OK;
    }
    STDMETHOD(GstGetSink)(ID2D1SimplifiedGeometrySink** ppSink) override
    {
        *ppSink = m_pSink.Get();
        if (*ppSink)
            (*ppSink)->AddRef();
        return S_OK;
    }

    STDMETHOD(GstEnableTransform)(BOOL b) override { return E_NOTIMPL; }
    STDMETHOD(GstIsTransformEnabled)(BOOL* pIsEnabled) override { return E_NOTIMPL; }
    STDMETHOD(GstSetMatrix)(const D2D1_MATRIX_3X2_F* pMatrix) override { return E_NOTIMPL; }

    STDMETHOD(GstEnableOffset)(BOOL b) override
    {
        m_bOffsetEnabled = b;
        return S_OK;
    }
    STDMETHOD(GstIsOffsetEnabled)(BOOL* pIsEnabled) override
    {
        *pIsEnabled = m_bOffsetEnabled;
        return S_OK;
    }
    STDMETHOD(GstSetOffset)(float dx, float dy) override
    {
        m_ox = dx;
        m_oy = dy;
        return S_OK;
    }
};
ECK_NAMESPACE_END