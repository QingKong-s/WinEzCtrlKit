#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
struct Intercept_T {};
struct InterceptDelete_T {};
struct NoIntercept_T {};

struct SlotCtx
{
    template<class TIntercept, class TRet, class ...TArgs>
    friend class CEventChain;
private:
    BOOL m_bDeleting{};
    BOOL m_bProcessed{};
    void* m_pCurrNode{};

    constexpr SlotCtx(void* pCurrNode) noexcept : m_pCurrNode{ pCurrNode } {}
public:
    SlotCtx() = default;

    EckInlineNdCe BOOL IsDeleting() const noexcept { return m_bDeleting; }
    EckInlineCe void Processed(BOOL b = TRUE) noexcept { m_bProcessed = b; }
    EckInlineNdCe BOOL IsProcessed() const noexcept { return m_bProcessed; }
};

// WARNING 非线程安全
// WARNING 必须保证实例连接的所有资源都晚于实例销毁
// 
// TIntercept设为Intercept_T以指示信号可被拦截，此时在槽参数列表尾部增加SlotCtx
template<class TIntercept, class TRet, class ...TArgs>
class CEventChain
{
public:
    constexpr static bool IsRetVoid = std::is_same_v<TRet, void>;
    constexpr static bool IsIntercept = !std::is_same_v<TIntercept, NoIntercept_T>;
    constexpr static bool IsInterceptDelete = std::is_same_v<TIntercept, InterceptDelete_T>;

    using FSlotPointer = std::conditional_t<
        IsIntercept,
        TRet(*)(TArgs..., SlotCtx&),
        TRet(*)(TArgs...)
    >;

    template<class TCls>
    using FSlotMethodPointer = std::conditional_t<
        IsIntercept,
        TRet(TCls::*)(TArgs..., SlotCtx&),
        TRet(TCls::*)(TArgs...)
    >;
    template<class TCls>
    using FSlotMethodPointerConst = std::conditional_t<
        IsIntercept,
        TRet(TCls::*)(TArgs..., SlotCtx&) const,
        TRet(TCls::*)(TArgs...) const
    >;

    using FSlot = std::conditional_t<
        IsIntercept,
        std::function<TRet(TArgs..., SlotCtx&)>,
        std::function<TRet(TArgs...)>
    >;
private:
    using TReturnDefault = std::conditional_t<IsRetVoid, std::monostate, TRet>;

    struct NODE
    {
#ifdef _DEBUG
        CEventChain* pThis;
#endif
        FSlot Fn;
        USHORT uId;
        USHORT uFlags;
        int cEnter;
        NODE* pNext;
    };

    enum : USHORT
    {
        NF_DELETED = (1 << 0),
    };

    NODE* m_pHead{};

    NODE* DeleteNode(_In_ NODE* pNode, _In_opt_ NODE* pPrev) noexcept
    {
        EckAssert((pNode->uFlags & NF_DELETED) && pNode->cEnter == 0);
        if (pPrev)
            pPrev->pNext = pNode->pNext;
        else
            m_pHead = pNode->pNext;
        const auto pNext = pNode->pNext;
        if constexpr (IsInterceptDelete)
        {
            SlotCtx Ctx{};
            Ctx.m_bDeleting = TRUE;
            pNode->Fn(TArgs{}..., Ctx);
        }
        delete pNode;
        return pNext;
    }

    TRet EmitStartWith(_In_ NODE* pNode, _In_opt_ NODE* pPrev,
        SlotCtx& Ctx, TArgs... Args) noexcept
    {
        Ctx.m_bDeleting = FALSE;
        EckLoop()
        {
            if (pNode->uFlags & NF_DELETED)
            {
                if (!pNode->cEnter)
                    pNode = DeleteNode(pNode, pPrev);
                else
                    pNode = pNode->pNext;
                if (!pNode)
                    break;
                continue;
            }
            ++pNode->cEnter;
            if constexpr (IsIntercept)
            {
                Ctx.m_pCurrNode = pNode;
                Ctx.m_bProcessed = FALSE;
                if constexpr (IsRetVoid)
                {
                    pNode->Fn(Args..., Ctx);
                    --pNode->cEnter;
                    if (Ctx.IsProcessed())
                        return;
                }
                else
                {
                    const auto r{ pNode->Fn(Args..., Ctx) };
                    --pNode->cEnter;
                    if (Ctx.IsProcessed())
                        return r;
                }
            }
            else
            {
                pNode->Fn(Args...);
                --pNode->cEnter;
            }
            pPrev = pNode;
            if (!(pNode = pNode->pNext))
                break;
        }
        if constexpr (!IsRetVoid)
            return {};
    }
public:
    using HSlot = const NODE*;

    const inline static HSlot SlotTop{};
    const inline static HSlot SlotBottom{ (HSlot)(UINT_PTR(-1)) };

    ECK_DISABLE_COPY_MOVE_DEF_CONS(CEventChain);

    ~CEventChain()
    {
        NODE* pNode = m_pHead;
        while (pNode)
        {
            if (pNode->cEnter)// 严禁在槽内析构信号
            {
                EckDbgBreak();
                std::terminate();
            }
            pNode->uFlags |= NF_DELETED;
            pNode = DeleteNode(pNode, nullptr);
        }
    }

    EckInline TRet EmitWithContext(SlotCtx& Ctx, TArgs ...Args) noexcept
    {
        if (m_pHead)
            return EmitStartWith(m_pHead, nullptr, Ctx, Args...);
        if constexpr (!IsRetVoid)
            return {};
    }
    EckInline TRet EmitWithDefault(SlotCtx& Ctx, TReturnDefault&& Def, TArgs ...Args) noexcept
    {
        if (m_pHead)
            return EmitStartWith(m_pHead, nullptr, Ctx, Args...);
        if constexpr (!IsRetVoid)
            return Def;
    }
    EckInline TRet Emit(TArgs ...Args) noexcept
    {
        SlotCtx Ctx{};
        return EmitWithContext(Ctx, Args...);
    }
private:
    NODE* IntConnect(FSlot&& Fn, USHORT uId, NODE* pAfter) noexcept
    {
#ifdef _DEBUG
        const auto pNew = new NODE{ this, std::move(Fn), uId };
#else
        const auto pNew = new NODE{ std::move(Fn), uId };
#endif
        if (m_pHead)
        {
            if (pAfter == SlotTop)
            {
                pNew->pNext = m_pHead;
                m_pHead = pNew;
            }
            else if (pAfter == SlotBottom)
            {
                auto p = m_pHead;
                while (p->pNext)
                    p = p->pNext;
                p->pNext = pNew;
            }
            else
            {
                EckAssert(pAfter->pThis == this && !(pAfter->uFlags & NF_DELETED));
                pNew->pNext = pAfter->pNext;
                pAfter->pNext = pNew;
            }
        }
        else
        {
            EckAssert(pAfter == SlotTop || pAfter == SlotBottom);
            m_pHead = pNew;
        }
        return pNew;
    }
public:
    template<class TProc>
    EckInline HSlot Connect(TProc&& Fn, USHORT uId = 0, HSlot pAfter = SlotTop) noexcept
    {
        return IntConnect(std::forward<TProc>(Fn), uId, (NODE*)pAfter);
    }

    template<class TCls>
    EckInline HSlot Connect(TCls* pThis, FSlotMethodPointer<TCls> pfnMethod,
        USHORT uId = 0, HSlot pAfter = SlotTop) noexcept
    {
        return IntConnect(
            [pThis, pfnMethod](auto&&... Args)
            {
                return (pThis->*pfnMethod)(std::forward<decltype(Args)>(Args)...);
            }, uId, (NODE*)pAfter);
    }
    template<class TCls>
    EckInline HSlot Connect(TCls* pThis, FSlotMethodPointerConst<TCls> pfnMethod,
        USHORT uId = 0, HSlot pAfter = SlotTop) noexcept
    {
        return IntConnect(
            [pThis, pfnMethod](auto&&... Args)
            {
                return (pThis->*pfnMethod)(std::forward<decltype(Args)>(Args)...);
            }, uId, (NODE*)pAfter);
    }

    EckInline HSlot Connect(FSlotPointer pfn, USHORT uId = 0, HSlot pAfter = SlotTop) noexcept
    {
        return IntConnect(pfn, uId, (NODE*)pAfter);
    }

    // WARNING 调用方必须保证被连接事件链不悬空
    EckInline HSlot Connect(CEventChain& ec, USHORT uId = 0, HSlot pAfter = SlotTop) noexcept
    {
        EckAssert(&ec != this);
        if constexpr (IsIntercept)
            return IntConnect([&](TArgs ...Args, SlotCtx& Ctx)->TRet
                {
                    return ec.EmitWithContext(Ctx, Args...);
                }, uId, (NODE*)pAfter);
        else
            return IntConnect([&](TArgs ...Args)->TRet
                {
                    ec.Emit(Args...);
                }, uId, (NODE*)pAfter);
    }

    HSlot FindSlot(USHORT uId, HSlot pBegin = SlotTop) const noexcept
    {
        auto p = (NODE*)(pBegin ? pBegin->pNext : m_pHead);
        while (p)
        {
            if (p->uId == uId && !(p->uFlags & NF_DELETED))
                return p;
            p = p->pNext;
        }
        return nullptr;
    }

    void Disconnect(HSlot hSlot) noexcept
    {
        EckAssert(hSlot && hSlot->pThis == this && !(hSlot->uFlags & NF_DELETED));
        ((NODE*)hSlot)->uFlags |= NF_DELETED;
    }

    BOOL Disconnect(USHORT uId) noexcept
    {
        const auto p = FindSlot(uId);
        if (p)
        {
            Disconnect(p);
            return TRUE;
        }
        return FALSE;
    }

    // 立即删除所有进入计数为0且标记为删除的节点
    void CleanupNow() noexcept
    {
        NODE* pNode = m_pHead;
        NODE* pPrev{};
        do
        {
            if (pNode->cEnter == 0 && (pNode->uFlags & NF_DELETED))
                pNode = DeleteNode(pNode, pPrev);
            else
            {
                pPrev = pNode;
                pNode = pNode->pNext;
            }
        } while (pNode);
    }

    void Clear() noexcept
    {
        auto pNode = m_pHead;
        while (pNode)
        {
            const auto pNext = pNode->pNext;
            Disconnect(pNode);
            pNode = pNext;
        }
        CleanupNow();
    }

    // 调用下一槽。
    // 此方法将重置上下文状态，若要保留状态，则应在此方法返回后重新设置状态或复制上下文
    TRet CallNext(SlotCtx& Ctx, TArgs ...Args) noexcept
    {
        EckAssert(Ctx.m_pCurrNode && ((NODE*)Ctx.m_pCurrNode)->pThis == this);
        const auto pNext = ((NODE*)Ctx.m_pCurrNode)->pNext;
        if (pNext)
            return EmitStartWith(pNext, (NODE*)Ctx.m_pCurrNode, Ctx, Args...);
        if constexpr (!IsRetVoid)
            return {};
    }

    template<class T>
    const T* GetFunctionTarget(HSlot hSlot) const noexcept
    {
        EckAssert(hSlot && hSlot->pThis == this && !(hSlot->uFlags & NF_DELETED));
        return hSlot->Fn.template target<T>();
    }
    template<class T>
    T* GetFunctionTarget(HSlot hSlot) noexcept
    {
        EckAssert(hSlot && hSlot->pThis == this && !(hSlot->uFlags & NF_DELETED));
        return ((NODE*)hSlot)->Fn.template target<T>();
    }
};
ECK_NAMESPACE_END