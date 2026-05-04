#pragma once
#include "DuiBase.h"
#include "MathHelper.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
// 四边形映射变换，必须使用CalculateDistortMatrix/CalculateInverseDistortMatrix计算矩阵
class CCompositorCornerMapping : public CCompositor
{
private:
    DirectX::XMFLOAT4X4A m_Matrix{};
    DirectX::XMFLOAT4X4A m_MatrixR{};
    float m_Opacity{ 1.f };
public:
    void TransformPoint(_Inout_ Kw::Vec2& pt, BOOL bNormalToComposited) noexcept override
    {
        const DirectX::XMFLOAT4A v{ pt.x, pt.y, 0.f, 1.f };
        auto p = DirectX::XMVector4Transform(
            DirectX::XMLoadFloat4A(&v),
            DirectX::XMLoadFloat4x4A(bNormalToComposited ? &m_MatrixR : &m_Matrix));
        const auto w = DirectX::XMVectorGetW(p);
        p = DirectX::XMVectorDivide(p, DirectX::XMVectorSet(w, w, w, w));
        pt.x = DirectX::XMVectorGetX(p);
        pt.y = DirectX::XMVectorGetY(p);
    }

    void CalculateCompositedRect(_Out_ D2D1_RECT_F& rc, BOOL bInClientOrParent) noexcept override
    {
        const auto cx = GetElement()->GetWidth();
        const auto cy = GetElement()->GetHeight();
        const D2D1_POINT_2F pt[]{ { 0, 0 }, { cx, 0 }, { cx, cy }, { 0, cy } };

        rc = { FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX };
        const auto m = DirectX::XMLoadFloat4x4A(&m_Matrix);
        for (const auto e : pt)
        {
            const DirectX::XMFLOAT4A v{ e.x, e.y, 0.f, 1.f };
            auto p = DirectX::XMVector4Transform(DirectX::XMLoadFloat4A(&v), m);
            const auto w = DirectX::XMVectorGetW(p);
            p = DirectX::XMVectorDivide(p, DirectX::XMVectorSet(w, w, w, w));
            const auto x = DirectX::XMVectorGetX(p);
            const auto y = DirectX::XMVectorGetY(p);
            if (x < rc.left) rc.left = x;
            if (x > rc.right) rc.right = x;
            if (y < rc.top) rc.top = y;
            if (y > rc.bottom) rc.bottom = y;
        }
        GetElement()->ElementToClient(rc);
        if (!bInClientOrParent && GetElement()->EtParent())
            GetElement()->EtParent()->ClientToElement(rc);
    }

    BOOL IsInPlace() const noexcept override { return FALSE; }

    void PostRender(COMP_RENDER_INFO& cri) noexcept override
    {
        cri.pDC->DrawBitmap(cri.pBitmap, cri.rcDst, m_Opacity,
            D2D1_INTERPOLATION_MODE_LINEAR, cri.rcSrc, (D2D1_MATRIX_4X4_F*)&m_Matrix);
    }

    EckInlineNdCe auto AtMatrix() noexcept { return &m_Matrix; }
    EckInlineNdCe auto AtMatrixR() noexcept { return &m_MatrixR; }

    EckInlineCe void SetOpacity(float f) noexcept { m_Opacity = f; }
    EckInlineNdCe float GetOpacity() const noexcept { return m_Opacity; }
};

// 2D仿射变换
struct CCompositor2DAffineTransform : public CCompositor
{
private:
    D2D1::Matrix3x2F m_Matrix{};
    D2D1::Matrix3x2F m_MatrixR{};
    float m_Opacity{ 1.f };
public:
    void TransformPoint(_Inout_ Kw::Vec2& pt, BOOL bNormalToComposited) noexcept override
    {
        D2D1_POINT_2F pt0;
        if (bNormalToComposited)
            pt0 = m_MatrixR.TransformPoint({ pt.x, pt.y });
        else
            pt0 = m_Matrix.TransformPoint({ pt.x, pt.y });
        pt.x = pt0.x;
        pt.y = pt0.y;
    }

    void CalculateCompositedRect(_Out_ D2D1_RECT_F& rc, BOOL bInClientOrParent) noexcept override
    {
        const auto cx = GetElement()->GetWidth();
        const auto cy = GetElement()->GetHeight();
        const D2D1_POINT_2F pt[]{ { 0, 0 }, { cx, 0 }, { cx, cy }, { 0, cy } };
        rc = { FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX };
        for (const auto& e : pt)
        {
            const auto pt0 = m_Matrix.TransformPoint(e);
            if (pt0.x < rc.left) rc.left = pt0.x;
            if (pt0.x > rc.right) rc.right = pt0.x;
            if (pt0.y < rc.top) rc.top = pt0.y;
            if (pt0.y > rc.bottom) rc.bottom = pt0.y;
        }
        GetElement()->ElementToClient(rc);
        if (!bInClientOrParent && GetElement()->EtParent())
            GetElement()->EtParent()->ClientToElement(rc);
    }

    BOOL IsInPlace() const noexcept override { return FALSE; }

    void PostRender(COMP_RENDER_INFO& cri) noexcept override
    {
        D2D1::Matrix3x2F MatOld;
        cri.pDC->GetTransform(&MatOld);
        cri.pDC->SetTransform(m_Matrix * MatOld);
        cri.pDC->DrawBitmap(cri.pBitmap, cri.rcDst, m_Opacity,
            D2D1_INTERPOLATION_MODE_LINEAR, 0);// cri.rcSrc);
        cri.pDC->SetTransform(MatOld);
    }

    EckInline void SetMatrix(const D2D1::Matrix3x2F& Mat) noexcept
    {
        m_Matrix = m_MatrixR = Mat;
        m_MatrixR.Invert();
    }
    EckInline void SetMatrix(const D2D1::Matrix3x2F& Mat,
        const D2D1::Matrix3x2F& MatR) noexcept
    {
        m_Matrix = Mat;
        m_MatrixR = MatR;
    }
    EckInlineNdCe auto& GetMatrix() const noexcept { return m_Matrix; }
    EckInlineNdCe auto& GetMatrixR() const noexcept { return m_MatrixR; }

    EckInlineCe void SetOpacity(float f) noexcept { m_Opacity = f; }
    EckInlineNdCe float GetOpacity() const noexcept { return m_Opacity; }
};

// 页面切换相关动画
// HACK
class CCompositorPageAn : public CCompositor
{
public:
    enum class Type : BYTE// 动画类型
    {
        Sticker,		// 贴纸
        m_Opacity,		// 透明度
        Scale,			// 缩放
        ScaleOpacity,	// 缩放+透明度
        ScaleBlur, 		// 缩放+模糊
        Translation,	// 平移
        TranslationOpacity,	// 平移+透明度
    };

    enum class Corner : BYTE// 贴纸强调顶点
    {
        LT,
        RT,
        LB,
        RB,
    };
private:
    CCompositorCornerMapping m_CompCornerMap{};
    CCompositor2DAffineTransform m_Comp2DAffine{};

    ComPtr<ID2D1Effect> m_pFxBlur{};
    ComPtr<ID2D1Effect> m_pFxCrop{};

    float Scale{ 1.f };
    float m_Opacity{ 1.f };
    float Dx{};
    float Dy{};
    D2D1_POINT_2F Point{};
    Corner Corner{};
    Type m_eType{};
public:
    void Attach(CElement* pEle) noexcept override
    {
        __super::Attach(pEle);
        m_CompCornerMap.Attach(pEle);
        m_Comp2DAffine.Attach(pEle);
        if (!pEle)
        {
            m_pFxBlur.Clear();
            m_pFxCrop.Clear();
        }
    }

    void TransformPoint(_Inout_ Kw::Vec2& pt, BOOL bNormalToComposited) noexcept override
    {
        switch (m_eType)
        {
        case Type::Sticker:
            m_CompCornerMap.TransformPoint(pt, bNormalToComposited);
            break;
        case Type::Scale:
        case Type::ScaleOpacity:
        case Type::ScaleBlur:
            m_Comp2DAffine.TransformPoint(pt, bNormalToComposited);
            break;
        case Type::Translation:
            if (bNormalToComposited)
            {
                pt.x += Dx;
                pt.y += Dy;
            }
            else
            {
                pt.x -= Dx;
                pt.y -= Dy;
            }
            break;
        }
    }

    void CalculateCompositedRect(_Out_ D2D1_RECT_F& rc, BOOL bInClientOrParent) noexcept override
    {
        switch (m_eType)
        {
        case Type::Sticker:
            m_CompCornerMap.CalculateCompositedRect(rc, bInClientOrParent);
            break;
        case Type::Scale:
        case Type::ScaleOpacity:
        case Type::ScaleBlur:
            m_Comp2DAffine.CalculateCompositedRect(rc, bInClientOrParent);
            break;
        case Type::Translation:
        case Type::TranslationOpacity:
            *(Kw::Rect*)&rc = (bInClientOrParent ?
                GetElement()->GetRectInClient() :
                GetElement()->GetRect());
            OffsetRect(rc, Dx, Dy);
            break;
        default:
            rc = {};
            break;
        }
    }

    BOOL IsInPlace() const noexcept override { return m_eType == Type::m_Opacity; }

    void PostRender(COMP_RENDER_INFO& cri) noexcept override
    {
        switch (m_eType)
        {
        case Type::Sticker:
            m_CompCornerMap.PostRender(cri);
            break;
        case Type::m_Opacity:
            cri.pDC->DrawBitmap(cri.pBitmap, cri.rcDst, m_Opacity,
                D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR, cri.rcSrc);
            break;
        case Type::Scale:
        case Type::ScaleOpacity:
            m_Comp2DAffine.PostRender(cri);
            break;
        case Type::ScaleBlur:
        {
            D2D1::Matrix3x2F MatOld;
            cri.pDC->GetTransform(&MatOld);
            cri.pDC->SetTransform(m_Comp2DAffine.GetMatrix() * MatOld);
            m_pFxCrop->SetInput(0, cri.pBitmap);
            m_pFxCrop->SetValue(D2D1_CROP_PROP_RECT, cri.rcSrc);
            m_pFxBlur->SetInputEffect(0, m_pFxCrop.Get());
            cri.pDC->DrawImage(m_pFxBlur.Get(), { cri.rcDst.left,cri.rcDst.top },
                D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
            cri.pDC->SetTransform(MatOld);
        }
        break;
        case Type::Translation:
        case Type::TranslationOpacity:
        {
            auto rcDst = cri.rcDst;
            OffsetRect(rcDst, (float)Dx, (float)Dy);
            cri.pDC->DrawBitmap(cri.pBitmap, rcDst,
                m_eType == Type::Translation ? 1.f : m_Opacity,
                D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR, cri.rcSrc);
        }
        break;
        }
    }

    void Initialize(Type eType) noexcept
    {
        m_eType = eType;
        if (eType == Type::ScaleBlur)
        {
            if (!m_pFxBlur)
            {
                GetElement()->GetDC()->CreateEffect(CLSID_D2D1GaussianBlur, &m_pFxBlur);
                m_pFxBlur->SetValue(D2D1_GAUSSIANBLUR_PROP_BORDER_MODE,
                    D2D1_BORDER_MODE_SOFT);
            }
            if (!m_pFxCrop)
                GetElement()->GetDC()->CreateEffect(CLSID_D2D1Crop, &m_pFxCrop);
        }
    }


    void Update2DAffineTransform() noexcept
    {
        m_Comp2DAffine.SetMatrix(D2D1::Matrix3x2F::Scale(Scale, Scale, Point));
    }

    void SetBlurStdDeviation(float fStdDev) noexcept
    {
        if (m_pFxBlur)
            m_pFxBlur->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, fStdDev);
    }
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END