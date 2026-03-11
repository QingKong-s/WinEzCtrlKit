#pragma once
#include "CReferenceCounted.h"
#include "KwDef.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
class CElement;

// 对于DES_OWNER_COMP_CACHE元素，使用此对象表示图集上的范围
// 实现必须保证当引用计数减为0时，与其相关联的资源也一同被释放
class CCompCacheSurface : public CReferenceCounted
{
protected:
    D2D1_RECT_F m_rcValid{};
    ID2D1Bitmap1* m_pBitmap{};
public:
    EckInlineNdCe auto GetBitmap() const noexcept { return m_pBitmap; }
    EckInlineNdCe auto& GetValidRect() const noexcept { return m_rcValid; }
};

struct COMP_RENDER_INFO
{
    CElement* pEle;			// 当前正渲染的元素
    ID2D1DeviceContext* pDC;// 已设置适当变换使其坐标相对pEle
    ID2D1Bitmap1* pBitmap;	// 渲染完毕的位图。通常，混合器需要将此位图内容做恰当处理后呈现
    D2D1_RECT_F rcSrc;		// 相对pBitmap
    D2D1_RECT_F rcDst;		// 相对pEle
};

// 表示一个特定的混合操作，默认的实现不执行任何操作
struct ICompositor : public CReferenceCounted
{
    // 坐标相对元素
    virtual void TransformPoint(CElement* pEle,
        _Inout_ Kw::Vec2& pt, BOOL bNormalToComposited) noexcept {}

    /// <summary>
    /// 计算混合后矩形。
    /// 若IsInPlace返回TRUE，则此方法不会被调用。
    /// DUI系统自动缓存结算结果，并在尺寸改变时重新调用计算，实现无需缓存
    /// </summary>
    /// <param name="pEle">目标元素</param>
    /// <param name="rc">计算结果</param>
    /// <param name="bInClientOrParent">结果相对于客户区还是相对于父元素</param>
    virtual void CalculateCompositedRect(CElement* pEle,
        _Out_ D2D1_RECT_F& rc, BOOL bInClientOrParent) noexcept
    {
        rc = {};
    }

    // 是否原地操作
    virtual BOOL IsInPlace() const noexcept { return TRUE; }

    // 若元素设置了DES_COMP_NO_REDIRECTION，则在渲染之前调用此方法，
    // 此时pBitmap和rcDst字段无效
    virtual void PreRender(COMP_RENDER_INFO& cri) noexcept {}

    // 执行混合操作。
    // 混合元素渲染到独立的图面，当该元素连同其所有子元素都渲染完毕后调用此方法
    virtual void PostRender(COMP_RENDER_INFO& cri) noexcept
    {}

    // 分配缓存位图。
    // 仅当需要创建元素的混合重定向表面时，DUI才调用此方法。
    // 若混合器对特定效果（或其他情况）实现图集缓存，则可在此分配并返回缓存图集。
    // 注意不能使两个设置了混合器且互为父子关系的元素共享同一幅D2D位图
    virtual HRESULT CreateCacheBitmap(int cxPhy, int cyPhy,
        _Out_ CCompCacheSurface*& pSurface) noexcept
    {
        return E_NOTIMPL;
    }
};
/*
    void TransformPoint(CElement* pEle,
        _Inout_ Kw::Vec2& pt, BOOL bNormalToComposited) noexcept override
    {}
    void CalculateCompositedRect(CElement* pEle, _Out_ RECT& rc, BOOL bInClientOrParent) noexcept override
    {}
    BOOL IsInPlace() const noexcept override { return TRUE; }
    void PostRender(COMP_RENDER_INFO& cri) noexcept override
    {}
*/
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END