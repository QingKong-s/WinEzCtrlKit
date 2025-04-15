#pragma once
#include "CUnknown.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
class CElem;

struct COMP_RENDER_INFO
{
	CElem* pElem;			// 当前正渲染的元素
	ID2D1DeviceContext* pDC;// 已设置适当变换使其坐标相对pElem
	ID2D1Bitmap1* pBitmap;	// 渲染完毕的位图。通常，混合器需要将此位图内容做恰当处理后呈现
	D2D1_RECT_F rcSrc;		// 相对pBitmap
	RECT rcInvalid;			// 相对客户区
};

// 表示一个特定的混合操作，默认的实现不执行任何操作，
// 通常实现为与元素无关
class CCompositor : public CRefObjSingleThread<CCompositor>
{
	ECK_DECL_CUNK_FRIENDS;
protected:
	LONG m_cRef{ 1 };
	CCompositor* m_pParent{};// 父级混合器
public:
	virtual ~CCompositor() = default;
	/// <summary>
	/// 坐标 常规到混合后
	/// </summary>
	/// <param name="pElem">目标元素</param>
	/// <param name="pt">坐标，相对pElem</param>
	/// <param name="pAncestor">pElem的第一个设置了混合器的祖元素，若pElem本身具有混合器，则为nullptr</param>
	virtual void PtNormalToComposited(CElem* pElem, _Inout_ POINT& pt, CElem* pAncestor) {}

	/// <summary>
	/// 坐标 混合后到常规
	/// </summary>
	/// <param name="pElem">目标元素</param>
	/// <param name="pt">坐标，相对pElem</param>
	/// <param name="pAncestor">pElem的第一个设置了混合器的祖元素，若pElem本身具有混合器，则为nullptr</param>
	virtual void PtCompositedToNormal(CElem* pElem, _Inout_ POINT& pt, CElem* pAncestor) {}

	/// <summary>
	/// 计算混合后矩形。
	/// 永远不会对未设置混合器的元素调用。
	/// 若IsInPlace返回TRUE，则此方法不会被调用。
	/// DUI系统自动缓存结算结果，并在尺寸改变时重新调用计算，实现无需缓存
	/// </summary>
	/// <param name="pElem">目标元素</param>
	/// <param name="rc">计算结果</param>
	/// <param name="bInClient">结果相对于客户区还是相对于父元素</param>
	virtual void CalcCompositedRect(CElem* pElem, _Out_ RECT& rc, BOOL bInClientOrParent) { rc = {}; }

	// 是否原地操作
	virtual BOOL IsInPlace() const { return TRUE; }

	/// <summary>
	/// 执行混合操作。
	/// 永远不会对未设置混合器的元素调用。
	/// 混合元素渲染到独立的图面，当该元素连同其所有
	/// 子元素都渲染完毕后调用此方法
	/// </summary>
	/// <param name="cri">渲染信息</param>
	virtual void PostRender(COMP_RENDER_INFO& cri)
	{
	}

	void SetParent(CCompositor* pParent)
	{
		std::swap(m_pParent, pParent);
		if (m_pParent)
			m_pParent->AddRef();
		if (pParent)
			pParent->Release();
	}
};
/*
	void PtNormalToComposited(CElem* pElem, _Inout_ POINT& pt, CElem* pAncestor) override
	{}
	void PtCompositedToNormal(CElem* pElem, _Inout_ POINT& pt, CElem* pAncestor) override
	{}
	void CalcCompositedRect(CElem* pElem, _Out_ RECT& rc, BOOL bInClient) override
	{}
	BOOL IsInPlace() const override { return TRUE; }
	void PostRender(COMP_RENDER_INFO& cri) override
	{}
*/
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END