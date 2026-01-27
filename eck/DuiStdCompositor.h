#pragma once
#include "DuiBase.h"
#include "MathHelper.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
// 四边形映射变换，必须使用CalcDistortMatrix/CalcInverseDistortMatrix计算矩阵
struct CCompositorCornerMapping : public CCompositor
{
    DirectX::XMMATRIX Mat{};
    DirectX::XMMATRIX MatR{};
    float Opacity{ 1.f };

    void PtXToX(CElem* pElem, _Inout_ POINT& pt, BOOL bNormalToComposited) noexcept
    {
        const DirectX::XMFLOAT4A v{ (float)pt.x,(float)pt.y,0.f,1.f };
        auto p = DirectX::XMVector4Transform(DirectX::XMLoadFloat4A(&v),
            bNormalToComposited ? MatR : Mat);
        const auto w = DirectX::XMVectorGetW(p);
        p = DirectX::XMVectorDivide(p, DirectX::XMVectorSet(w, w, w, w));
        pt.x = (LONG)DirectX::XMVectorGetX(p);
        pt.y = (LONG)DirectX::XMVectorGetY(p);
    }
    void PtNormalToComposited(CElem* pElem, _Inout_ POINT& pt) noexcept override
    {
        PtXToX(pElem, pt, TRUE);
    }
    void PtCompositedToNormal(CElem* pElem, _Inout_ POINT& pt) noexcept override
    {
        PtXToX(pElem, pt, FALSE);
    }

    void CalcCompositedRect(CElem* pElem, _Out_ D2D1_RECT_F& rc,
        BOOL bInClientOrParent) noexcept override
    {
        const auto cx = pElem->GetWidthF();
        const auto cy = pElem->GetHeightF();
        const D2D1_POINT_2F pt[]{ { 0,0 }, { cx,0 }, { cx,cy }, { 0,cy } };

        float l{ FLT_MAX }, t{ FLT_MAX }, r{ FLT_MIN }, b{ FLT_MIN };
        for (const auto e : pt)
        {
            const DirectX::XMFLOAT4A v{ e.x,e.y,0.f,1.f };
            auto p = DirectX::XMVector4Transform(
                DirectX::XMLoadFloat4A(&v), Mat);
            const auto w = DirectX::XMVectorGetW(p);
            p = DirectX::XMVectorDivide(p, DirectX::XMVectorSet(w, w, w, w));
            const auto x = DirectX::XMVectorGetX(p);
            const auto y = DirectX::XMVectorGetY(p);
            if (x < l) l = x;
            if (x > r) r = x;
            if (y < t) t = y;
            if (y > b) b = y;
        }
        rc = { l,t,r,b };
        pElem->ElemToClient(rc);
        if (!bInClientOrParent && pElem->GetParentElem())
            pElem->GetParentElem()->ClientToElem(rc);
    }
    BOOL IsInPlace() const noexcept override { return FALSE; }
    void PostRender(COMP_RENDER_INFO& cri) noexcept override
    {
        cri.pDC->DrawBitmap(cri.pBitmap, cri.rcDst, Opacity,
            D2D1_INTERPOLATION_MODE_LINEAR, cri.rcSrc, (D2D1_MATRIX_4X4_F*)&Mat);
    }
};

// 2D仿射变换
struct CCompositor2DAffineTransform : public CCompositor
{
    D2D1::Matrix3x2F Mat{};
    D2D1::Matrix3x2F MatR{};
    float Opacity{ 1.f };

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
    void PtNormalToComposited(CElem* pElem, _Inout_ POINT& pt) noexcept override
    {
        PtXToX(pElem, pt, TRUE);
    }
    void PtCompositedToNormal(CElem* pElem, _Inout_ POINT& pt) noexcept override
    {
        PtXToX(pElem, pt, FALSE);
    }
    void CalcCompositedRect(CElem* pElem, _Out_ D2D1_RECT_F& rc,
        BOOL bInClientOrParent) noexcept override
    {
        const auto cx = pElem->GetWidthF();
        const auto cy = pElem->GetHeightF();
        D2D1_POINT_2F pt[]{ { 0,0 },{ cx,cy } };
        pt[0] = Mat.TransformPoint(pt[0]);
        pt[1] = Mat.TransformPoint(pt[1]);
        rc = { pt[0].x,pt[0].y,pt[1].x,pt[1].y };
        pElem->ElemToClient(rc);
        if (!bInClientOrParent && pElem->GetParentElem())
            pElem->GetParentElem()->ClientToElem(rc);
    }
    BOOL IsInPlace() const noexcept override { return FALSE; }
    void PostRender(COMP_RENDER_INFO& cri) noexcept override
    {
        D2D1::Matrix3x2F MatOld;
        cri.pDC->GetTransform(&MatOld);
        cri.pDC->SetTransform(Mat * MatOld);
        cri.pDC->DrawBitmap(cri.pBitmap, cri.rcDst, Opacity,
            D2D1_INTERPOLATION_MODE_LINEAR, cri.rcSrc);
        cri.pDC->SetTransform(MatOld);
    }

    void InverseMatrix() noexcept
    {
        MatR = Mat;
        MatR.Invert();
    }
};

// 页面切换相关动画
class CCompositorPageAn : public CCompositor
{
public:
    enum class Type : BYTE// 动画类型
    {
        Sticker,		// 贴纸
        Opacity,		// 透明度
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

    float Scale{ 1.f };
    float Opacity{ 1.f };
    float Dx{};
    float Dy{};
    D2D1_POINT_2F RefPoint{};
    Corner Corner{};
private:
    Type m_eType{};
    CCompositorCornerMapping* m_pCornerMap{};
    CCompositor2DAffineTransform* m_p2DAffine{};
    ID2D1Effect* m_pFxBlur{};
    ID2D1Effect* m_pFxCrop{};
    ID2D1DeviceContext* m_pDC{};
public:
    ~CCompositorPageAn()
    {
        SafeRelease(m_pCornerMap);
        SafeRelease(m_p2DAffine);
        SafeRelease(m_pFxBlur);
        SafeRelease(m_pFxCrop);
        SafeRelease(m_pDC);
    }

    void PtNormalToComposited(CElem* pElem, _Inout_ POINT& pt) noexcept override
    {
        switch (m_eType)
        {
        case Type::Sticker:
            m_pCornerMap->PtNormalToComposited(pElem, pt);
            break;
        case Type::Scale:
        case Type::ScaleOpacity:
        case Type::ScaleBlur:
            m_p2DAffine->PtNormalToComposited(pElem, pt);
            break;
        case Type::Translation:
            pt.x += (int)Dx;
            pt.y += (int)Dy;
            break;
        }
    }

    void PtCompositedToNormal(CElem* pElem, _Inout_ POINT& pt) noexcept override
    {
        switch (m_eType)
        {
        case Type::Sticker:
            m_pCornerMap->PtCompositedToNormal(pElem, pt);
            break;
        case Type::Scale:
        case Type::ScaleOpacity:
        case Type::ScaleBlur:
            m_p2DAffine->PtCompositedToNormal(pElem, pt);
            break;
        case Type::Translation:
            pt.x -= (int)Dx;
            pt.y -= (int)Dy;
            break;
        }
    }

    void CalcCompositedRect(CElem* pElem, _Out_ D2D1_RECT_F& rc,
        BOOL bInClientOrParent) noexcept override
    {
        switch (m_eType)
        {
        case Type::Sticker:
            m_pCornerMap->CalcCompositedRect(pElem, rc, bInClientOrParent);
            break;
        case Type::Scale:
        case Type::ScaleOpacity:
        case Type::ScaleBlur:
            m_p2DAffine->CalcCompositedRect(pElem, rc, bInClientOrParent);
            break;
        case Type::Translation:
        case Type::TranslationOpacity:
            rc = bInClientOrParent ? pElem->GetRectInClientF() : pElem->GetRectF();
            OffsetRect(rc, Dx, Dy);
            break;
        default:
            rc = {};
            break;
        }
    }

    BOOL IsInPlace() const noexcept override { return m_eType == Type::Opacity; }

    void PostRender(COMP_RENDER_INFO& cri) noexcept override
    {
        switch (m_eType)
        {
        case Type::Sticker:
            m_pCornerMap->PostRender(cri);
            break;
        case Type::Opacity:
            cri.pDC->DrawBitmap(cri.pBitmap, cri.rcDst, Opacity,
                D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR, cri.rcSrc);
            break;
        case Type::Scale:
        case Type::ScaleOpacity:
            m_p2DAffine->PostRender(cri);
            break;
        case Type::ScaleBlur:
        {
            D2D1::Matrix3x2F MatOld;
            cri.pDC->GetTransform(&MatOld);
            cri.pDC->SetTransform(m_p2DAffine->Mat * MatOld);
            m_pFxCrop->SetInput(0, cri.pBitmap);
            m_pFxCrop->SetValue(D2D1_CROP_PROP_RECT, cri.rcSrc);
            m_pFxBlur->SetInputEffect(0, m_pFxCrop);
            cri.pDC->DrawImage(m_pFxBlur, { cri.rcDst.left,cri.rcDst.top },
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
                m_eType == Type::Translation ? 1.f : Opacity,
                D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR, cri.rcSrc);
        }
        break;
        }
    }

    void InitAsSticker()
    {
        m_eType = Type::Sticker;
        if (!m_pCornerMap)
            m_pCornerMap = new CCompositorCornerMapping{};
    }
    void InitAsOpacity(float fOpacity = 1.f)
    {
        m_eType = Type::Opacity;
        Opacity = fOpacity;
    }
    void InitAsScale()
    {
        m_eType = Type::Scale;
        if (!m_p2DAffine)
            m_p2DAffine = new CCompositor2DAffineTransform{};
    }
    void InitAsScaleOpacity(float fOpacity = 1.f)
    {
        m_eType = Type::ScaleOpacity;
        Opacity = fOpacity;
        if (!m_p2DAffine)
            m_p2DAffine = new CCompositor2DAffineTransform{};
    }
    void InitAsScaleBlur(float fOpacity = 1.f)
    {
        m_eType = Type::ScaleBlur;
        Opacity = fOpacity;
        if (!m_p2DAffine)
            m_p2DAffine = new CCompositor2DAffineTransform{};
        if (!m_pFxBlur)
        {
            m_pDC->CreateEffect(CLSID_D2D1GaussianBlur, &m_pFxBlur);
            m_pFxBlur->SetValue(D2D1_GAUSSIANBLUR_PROP_BORDER_MODE,
                D2D1_BORDER_MODE_SOFT);
        }
        if (!m_pFxCrop)
            m_pDC->CreateEffect(CLSID_D2D1Crop, &m_pFxCrop);
    }
    void InitAsTranslation(float dx = 0, float dy = 0)
    {
        m_eType = Type::Translation;
        Dx = dx;
        Dy = dy;
    }
    void InitAsTranslationOpacity(float dx = 0, float dy = 0, float fOpacity = 1.f)
    {
        m_eType = Type::TranslationOpacity;
        Dx = dx;
        Dy = dy;
        Opacity = fOpacity;
    }

    void SetWorkDC(ID2D1DeviceContext* pDC)
    {
        std::swap(m_pDC, pDC);
        if (m_pDC)
            m_pDC->AddRef();
        if (pDC)
            pDC->Release();
        SafeRelease(m_pFxBlur);
        SafeRelease(m_pFxCrop);
    }

    void Set2DAffineTransform(const D2D1::Matrix3x2F& Mat) noexcept
    {
        if (m_p2DAffine)
        {
            m_p2DAffine->Mat = Mat;
            m_p2DAffine->InverseMatrix();
        }
    }

    void Update2DAffineTransform() noexcept
    {
        Set2DAffineTransform(D2D1::Matrix3x2F::Scale(
            Scale, Scale, RefPoint));
    }

    void SetBlurStdDeviation(float fStdDev) noexcept
    {
        if (m_pFxBlur)
            m_pFxBlur->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, fStdDev);
    }

    void UpdateOpacity() noexcept
    {
        if (m_p2DAffine)
            m_p2DAffine->Opacity = Opacity;
        if (m_pCornerMap)
            m_pCornerMap->Opacity = Opacity;
    }
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END