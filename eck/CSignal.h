#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
struct Intercept_T {};
struct NoIntercept_T {};

struct SlotCtx
{
	template<class TIntercept, class TRet, class ...TArgs>
	friend class CSignal;
private:
	BOOL m_bDeleting{};
	BOOL m_bProcessed{};
	void* m_pCurrNode{};

	constexpr SlotCtx(void* pCurrNode) : m_pCurrNode{ pCurrNode } {}
public:
	SlotCtx() = default;

	EckInlineNdCe BOOL IsDeleting() const { return m_bDeleting; }
	EckInlineCe void Processed(BOOL b = TRUE) { m_bProcessed = b; }
	EckInlineNdCe BOOL IsProcessed() const { return m_bProcessed; }
};

/// <summary>
/// 信号。
/// 应自行管理槽所关联对象与信号的生命周期
/// </summary>
/// <typeparam name="TIntercept">设为Intercept_T以指示信号可被拦截，在槽的尾部增加bProcessed参数指示是否拦截</typeparam>
/// <typeparam name="TRet">返回值类型</typeparam>
/// <typeparam name="...TArgs">参数类型列表</typeparam>
template<class TIntercept, class TRet, class ...TArgs>
class CSignal
{
public:
	constexpr static bool IsRetVoid = std::is_same_v<TRet, void>;
	constexpr static bool IsIntercept = std::is_same_v<TIntercept, Intercept_T>;

	using FProcPtr = std::conditional_t<
		IsIntercept,
		TRet(*)(TArgs..., SlotCtx&),
		TRet(*)(TArgs...)
	>;

	template<class TCls>
	using FProcMethodPtr = std::conditional_t<
		IsIntercept,
		TRet(TCls::*)(TArgs..., SlotCtx&),
		TRet(TCls::*)(TArgs...)
	>;

	using FProc = std::conditional_t<
		IsIntercept,
		std::function<TRet(TArgs..., SlotCtx&)>,
		std::function<TRet(TArgs...)>
	>;
private:
	struct NODE
	{
#ifdef _DEBUG
		CSignal* pThis;
#endif
		UINT_PTR uId;
		FProc Fn;
		NODE* pNext;
		UINT uFlags;
		int cEnter;
	};

	enum :UINT
	{
		NF_DELETED = (1u << 0),
	};

	NODE* m_pHead{};

	NODE* DeleteNode(_In_ NODE* pNode, _In_opt_ NODE* pPrev)
	{
		EckAssert((pNode->uFlags & NF_DELETED) && pNode->cEnter == 0);
		if (pPrev)
			pPrev->pNext = pNode->pNext;
		else
			m_pHead = pNode->pNext;
		const auto pNext = pNode->pNext;
		if constexpr (IsIntercept)
		{
			SlotCtx Ctx{ nullptr };
			Ctx.m_bDeleting = TRUE;
			pNode->Fn(TArgs{}..., Ctx);
		}
		delete pNode;
		return pNext;
	}

	TRet EmitStartWith(_In_ NODE* pNode, _In_opt_ NODE* pPrev,
		SlotCtx& Ctx, TArgs... Args)
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
					if (Ctx.IsProcessed())
					{
						--pNode->cEnter;
						return;
					}
				}
				else
				{
					const auto r = pNode->Fn(Args..., Ctx);
					if (Ctx.IsProcessed())
					{
						--pNode->cEnter;
						return r;
					}
				}
			}
			else
				pNode->Fn(Args...);
			--pNode->cEnter;
			pPrev = pNode;
			if (!(pNode = pNode->pNext))
				break;
		}
		if constexpr (!IsRetVoid)
			return {};
	}
public:
	using HSlot = const NODE*;

#define ECK_SIG_TOP		nullptr
#define ECK_SIG_BOTTOM	(HSlot(UINT_PTR(-1)))

	ECK_DISABLE_COPY_MOVE_DEF_CONS(CSignal);

	~CSignal()
	{
		NODE* pNode = m_pHead;
		while (pNode)
		{
			pNode->uFlags |= NF_DELETED;
			pNode = DeleteNode(pNode, nullptr);
		}
	}

	/// <summary>
	/// 发射
	/// </summary>
	/// <param name="...Args">参数</param>
	/// <param name="bProcessed">是否被拦截</param>
	/// <returns>若某槽拦截信号，则返回该槽的返回值，否则返回返回值类型的默认构造结果</returns>
	EckInline TRet Emit2(SlotCtx& Ctx, TArgs ...Args)
	{
		if (m_pHead)
			return EmitStartWith(m_pHead, nullptr, Ctx, Args...);
		if constexpr (!IsRetVoid)
			return {};
	}

	/// <summary>
	/// 发射
	/// </summary>
	/// <param name="...Args">参数</param>
	/// <returns>若某槽拦截信号，则返回该槽的返回值，否则返回返回值类型的默认构造结果</returns>
	EckInline TRet Emit(TArgs ...Args)
	{
		SlotCtx Ctx{};
		return Emit2(Ctx, Args...);
	}
private:
	NODE* IntConnect(FProc&& Fn, UINT_PTR uId, NODE* pAfter)
	{
#ifdef _DEBUG
		const auto pNew = new NODE{ this,uId,std::move(Fn) };
#else
		const auto pNew = new NODE{ uId,std::move(Fn) };
#endif
		if (m_pHead)
		{
			if (pAfter == ECK_SIG_TOP)
			{
				pNew->pNext = m_pHead;
				m_pHead = pNew;
			}
			else if (pAfter == ECK_SIG_BOTTOM)
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
			EckAssert(pAfter == ECK_SIG_TOP || pAfter == ECK_SIG_BOTTOM);
			m_pHead = pNew;
		}
		return pNew;
	}

	template <class TCls, size_t... Index>
	NODE* IntConnect(TCls* pThis, FProcMethodPtr<TCls> pfnMethod,
		std::index_sequence<Index...>, UINT_PTR uId, NODE* pAfter)
	{
		return IntConnect(std::bind(pfnMethod, pThis, std::_Ph<Index + 1>{}...), uId, pAfter);
	}
public:
	template<class TProc>
	EckInline HSlot Connect(TProc&& Fn, UINT_PTR uId = 0u, HSlot pAfter = ECK_SIG_TOP)
	{
		return IntConnect(std::forward<TProc>(Fn), uId, (NODE*)pAfter);
	}

	template<class TCls>
	EckInline HSlot Connect(TCls* pThis, FProcMethodPtr<TCls> pfnMethod,
		UINT_PTR uId = 0u, HSlot pAfter = ECK_SIG_TOP)
	{
		if constexpr (IsIntercept)
			return IntConnect(pThis, pfnMethod,
				std::make_index_sequence<sizeof...(TArgs) + 1>{}, uId, (NODE*)pAfter);
		else
			return IntConnect(pThis, pfnMethod,
				std::make_index_sequence<sizeof...(TArgs)>{}, uId, (NODE*)pAfter);
	}

	EckInline HSlot Connect(FProcPtr pfn, UINT_PTR uId = 0u, HSlot pAfter = ECK_SIG_TOP)
	{
		return IntConnect(pfn, uId, (NODE*)pAfter);
	}

	EckInline HSlot Connect(CSignal& sig, UINT_PTR uId = 0u, HSlot pAfter = ECK_SIG_TOP)
	{
		EckAssert(&sig != this);
		if constexpr (IsIntercept)
			return IntConnect([&](TArgs ...Args, SlotCtx& Ctx)->TRet
				{
					if constexpr (IsRetVoid)
						sig.Emit2(Ctx, Args...);
					else
					{
						const auto r = sig.Emit2(Ctx, Args...);
						if (Ctx.IsProcessed())
							return r;
					}
				}, uId, (NODE*)pAfter);
		else
			return IntConnect([&](TArgs ...Args)->TRet
				{
					sig.Emit(Args...);
				}, uId, (NODE*)pAfter);
	}

	HSlot FindSlot(UINT_PTR uId, HSlot pBegin = ECK_SIG_TOP) const
	{
		auto p = (NODE*)(pBegin ? pBegin : m_pHead);
		while (p)
		{
			if (p->uId == uId && !(p->uFlags & NF_DELETED))
				return p;
			p = p->pNext;
		}
		return nullptr;
	}

	void Disconnect(HSlot hSlot)
	{
		EckAssert(hSlot && hSlot->pThis == this && !(hSlot->uFlags & NF_DELETED));
		((NODE*)hSlot)->uFlags |= NF_DELETED;
	}

	BOOL Disconnect(UINT_PTR uId)
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
	void CleanUpNow()
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

	// 可能返回已标记为删除的节点
	HSlot GetHead() const { return m_pHead; }

	void Clear()
	{
		auto pNode = m_pHead;
		while (pNode)
		{
			const auto pNext = pNode->pNext;
			Disconnect(pNode);
			pNode = pNext;
		}
		CleanUpNow();
	}

	// 调用下一槽。
	// 此方法将重置上下文状态，若要保留状态，则应在此方法返回后重新设置状态或复制上下文
	TRet CallNext(SlotCtx& Ctx, TArgs ...Args)
	{
		EckAssert(Ctx.m_pCurrNode && ((NODE*)Ctx.m_pCurrNode)->pThis == this);
		const auto pNext = ((NODE*)Ctx.m_pCurrNode)->pNext;
		if (pNext)
			return EmitStartWith(pNext, (NODE*)Ctx.m_pCurrNode, Ctx, Args...);
		if constexpr (!IsRetVoid)
			return {};
	}
};
ECK_NAMESPACE_END