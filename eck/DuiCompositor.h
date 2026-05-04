#pragma once
#include "CReferenceCounted.h"
#include "DuiDef.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
class CElement;

struct COMP_RENDER_INFO
{
    CElement* pEle;         // 当前正渲染的元素
    ID2D1DeviceContext* pDC;// 已设置适当变换使其坐标相对pEle
    ID2D1Bitmap1* pBitmap;  // 渲染完毕的位图。通常，混合器需要将此位图内容做恰当处理后呈现
    D2D1_RECT_F rcSrc;  // 相对pBitmap
    D2D1_RECT_F rcDst;  // 相对pEle
};

// 表示一个特定的混合操作，默认的实现不执行任何操作
// NOTE 260503
// 之前的设计中，Compositor应当实现为与元素无关，在长期使用过程中
// 发现此规则没有太大必要。同时，一般只有少数元素使用Compositor，
// 为了减少CElement的字段数量，将各种混合相关的矩形移出，这样Compositor
// 成为了CElement属性的一个扩展。
// 原有接口的CElement参数已删除，同时由于Compositor与元素一对一，移除引用计数
struct CCompositor
{
private:
    CElement* m_pEle{};
    // 缓存已混合的元素矩形，至少完全包含原始元素矩形，相对客户区
    D2D1_RECT_F m_rcCompInClient{};
    D2D1_RECT_F m_rcRealCompInClient{};// 实际计算得到的混合矩形
    CBitmap m_CacheBitmap{};
public:
    // DUI系统保留此函数，应用程序不得调用
    void EleUpdateCompositedRect(const D2D1_RECT_F& rcEleInClient) noexcept
    {
        if (!IsInPlace())
        {
            CalculateCompositedRect(m_rcCompInClient, TRUE);
            m_rcRealCompInClient = m_rcCompInClient;
            UnionRect(m_rcCompInClient, m_rcCompInClient, rcEleInClient);
        }
    }

    EckInline void EleUpdateCacheBitmap(const CBitmap& Bitmap) noexcept { m_CacheBitmap = Bitmap; }
    EckInline void EleInvalidateCacheBitmap() noexcept { m_CacheBitmap.Clear(); }
    EckInlineNdCe auto& EleGetCacheBitmap() const noexcept { return m_CacheBitmap; }

    // 相对客户区
    EckInlineNdCe auto& EleGetCompositedRect() const noexcept { return m_rcCompInClient; }
    // 相对客户区
    EckInlineNdCe auto& EleGetRealCompositedRect() const noexcept { return m_rcRealCompInClient; }


    EckInlineNdCe auto GetElement() const noexcept { return m_pEle; }

    virtual void Attach(CElement* pEle) noexcept
    {
        m_pEle = pEle;
        if (!pEle)
            m_CacheBitmap.Clear();
    }

    // 坐标相对元素
    virtual void TransformPoint(_Inout_ Kw::Vec2& pt, BOOL bNormalToComposited) noexcept {}

    /// <summary>
    /// 计算混合后矩形。
    /// 若IsInPlace返回TRUE，则此方法不会被调用。
    /// DUI系统自动缓存结算结果，并在尺寸改变时重新调用计算，实现无需缓存
    /// </summary>
    /// <param name="rc">计算结果</param>
    /// <param name="bInClientOrParent">结果相对于客户区还是相对于父元素</param>
    virtual void CalculateCompositedRect(
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
    virtual void PostRender(COMP_RENDER_INFO& cri) noexcept {}

    // 分配缓存位图。
    // 仅当需要创建元素的混合重定向表面时，DUI才调用此方法。
    // 若混合器对特定效果（或其他情况）实现图集缓存，则可在此分配并返回缓存图集。
    // NOTE 不能使两个设置了混合器且互为父子关系的元素共享同一幅D2D位图
    virtual HRESULT CreateCacheBitmap(int cxPhy, int cyPhy, CBitmap& Bitmap) noexcept
    {
        return E_NOTIMPL;
    }
};
/*
    void TransformPoint(_Inout_ Kw::Vec2& pt, BOOL bNormalToComposited) noexcept override
    {}
    void CalculateCompositedRect(_Out_ RECT& rc, BOOL bInClientOrParent) noexcept override
    {}
    BOOL IsInPlace() const noexcept override { return TRUE; }
    void PostRender(COMP_RENDER_INFO& cri) noexcept override
    {}
*/
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END